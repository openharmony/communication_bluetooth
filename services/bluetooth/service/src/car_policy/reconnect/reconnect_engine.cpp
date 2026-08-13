/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "reconnect_engine.h"

#include "bluetooth_def.h"
#include "device_priority_manager.h"
#include "dispatcher/car_policy_dispatcher.h"
#include "log.h"
#include "metrics/car_metrics_reporter.h"
#include "service/car_profile_service.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

namespace {
// ACL 断开原因：链路超时通常可自愈，是唯一会触发回连的 reason
constexpr int ACL_REASON_TIMEOUT = 8;
}  // namespace

ReconnectEngine::ReconnectEngine(CarPolicyConfig &config, DevicePriorityManager &dpm,
                                  CarPolicyDispatcher &dispatcher, CarMetricsReporter &metrics,
                                  CarConnectionGuard &guard)
    : config_(config), dpm_(dpm), dispatcher_(dispatcher), metrics_(metrics),
      guard_(guard) {}

ReconnectEngine::~ReconnectEngine() = default;

void ReconnectEngine::AddPredicate(std::unique_ptr<IReconnectPredicate> predicate)
{
    if (predicate) {
        predicates_.push_back(std::move(predicate));
    }
}

void ReconnectEngine::BumpEpoch()
{
    epoch_++;
}

void ReconnectEngine::StartReconnect(const std::string &addr, ReconnectReason reason)
{
    // 显式 BumpEpoch 使旧会话延时回调失效；不调用 Terminate 以避免 isActive_=false 后立即被覆盖的语义混淆
    BumpEpoch();
    currentDevice_ = addr;
    currentReason_ = reason;
    retryTimes_ = 0;
    isActive_ = true;
    HandleReconnect();
}

void ReconnectEngine::OnAclDisconnect(const std::string &addr, int rawReason)
{
    // 仅处理 reason=8 (ACL_CONNECTION_TIMEOUT)：链路超时通常可自愈，需要回连。
    // 其他 reason（如 6=PIN_OR_KEY_MISSING、14=CONNECTION_REJECTED_BY_SEC）不会自愈，回连无意义。
    if (rawReason != ACL_REASON_TIMEOUT) {
        HILOGI("OnAclDisconnect: rawReason=%{public}d != 8, skip reconnect", rawReason);
        return;
    }
    // 同 StartReconnect：显式 BumpEpoch 替代 Terminate，语义更清晰
    BumpEpoch();
    currentReason_ = ReconnectReason::ACL_CONNECTION_TIMEOUT;
    currentDevice_ = addr;
    retryTimes_ = 0;
    isActive_ = true;

    // ACL 断开后延时 1s 再发起回连，给底层链路释放与对端恢复留出窗口，避免立即重连撞上正在清理的资源。
    ScheduleNextRetry(config_.GetAclTimeoutRetryDelayMs());
}

void ReconnectEngine::OnAllProfilesDisconnected(const std::string &addr)
{
    dpm_.SetPenultimateDevice(addr);
}

void ReconnectEngine::Terminate()
{
    isActive_ = false;
    BumpEpoch();
}

void ReconnectEngine::Reset()
{
    isActive_ = false;
    retryTimes_ = 0;
    currentDevice_.clear();
    BumpEpoch();
}

void ReconnectEngine::ScheduleNextRetry(uint64_t delayMs)
{
    int snapshot = epoch_;
    dispatcher_.DispatchDelayed([this, snapshot]() {
        if (IsStale(snapshot) || !isActive_) {
            return;
        }
        HandleReconnect();
    }, delayMs);
}

void ReconnectEngine::ScheduleHfpRetry(const std::string &addr, uint64_t delayMs)
{
    int snapshot = epoch_;
    std::string addrCopy = addr;
    dispatcher_.DispatchDelayed([this, snapshot, addrCopy]() {
        if (IsStale(snapshot) || !isActive_) {
            return;
        }
        // HFP 仍 DISCONNECTED 则再 connect 一次：softbus 竞争可能导致首次 connect 请求被丢失
        if (CarProfileService::GetHfpDeviceState(addrCopy) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            HILOGI("HfpRetry: %{public}s still DISCONNECTED after timeout, retry connect", addrCopy.c_str());
            CarProfileService::Connect(ProfileType::HFP_HF, addrCopy);
        }
    }, delayMs);
}

bool ReconnectEngine::CheckAndFireTerminate()
{
    for (const auto &pred : predicates_) {
        if (pred->ShouldTerminate(currentDevice_, retryTimes_)) {
            pred->OnTerminate();
            return true;
        }
    }
    return false;
}

bool ReconnectEngine::CheckGuardAndPreempt()
{
    CheckResult result = guard_.Check(currentDevice_, ProfileType::HFP_HF);
    if (result.allowed) {
        return true;
    }
    // SinkLimit：当前设备被 sink 数限制时，主动断开其他低优先级设备腾出 slot，
    // 然后继续 connect 本设备，保证当前回连目标总能进入连接。
    if (result.reason == DenyReason::SINK_LIMIT) {
        guard_.DisconnectOthersForSinkLimit(currentDevice_);
        return true;
    }
    // A2DP_GATE：本次跳过，延时后重试
    HILOGI("Guard deny connect: %{public}s, skip this retry", DenyReasonName(result.reason));
    return false;
}

void ReconnectEngine::HandleReconnect()
{
    // ── 1. 谓词检查：ShouldTerminate 必须在发起连接之前执行，避免无意义连接 ──
    if (CheckAndFireTerminate()) {
        metrics_.ReportReconnectResult(currentDevice_, false, retryTimes_);
        isActive_ = false;
        return;
    }
    // ── 2. Guard.Check + SinkLimit 抢占：先查连接守卫是否允许，被 SinkLimit 拒绝时抢占断开其他设备 ──
    if (!CheckGuardAndPreempt()) {
        // A2DP_GATE 拒绝：本次跳过，不计重试次数，延时后重试
        ScheduleNextRetry(GetBackoffDelay(retryTimes_));
        return;
    }
    // ── 3. 执行连接 ──
    CarProfileService::Connect(ProfileType::HFP_HF, currentDevice_);
    // ── 3a. BT_ON 场景 800ms 后 HFP 重试容错：softbus 竞争可能导致首次 connect 请求丢失 ──
    if (currentReason_ == ReconnectReason::BT_ON_AUTO_CONN) {
        ScheduleHfpRetry(currentDevice_, config_.GetRetryHfpTimeoutMs());
    }
    // ── 4. 连接后自增重试计数 ──
    retryTimes_++;
    // ── 5. 达到最大重试次数则终止（自增后使用 >= 判断，避免实际重试次数比 MAX_RETRY 多 1） ──
    if (retryTimes_ >= config_.GetMaxRetry()) {
        metrics_.ReportReconnectResult(currentDevice_, false, retryTimes_);
        isActive_ = false;
        return;
    }
    // ── 6. BT_ON 场景每次重试轮询下一个设备：遍历优先级列表，给每个候选设备一次连接机会 ──
    if (currentReason_ == ReconnectReason::BT_ON_AUTO_CONN) {
        std::string nextDevice = dpm_.GetNextDevice(currentDevice_);
        if (!nextDevice.empty()) {
            currentDevice_ = nextDevice;
        }
    }
    // ── 7. 调度下次重试 ──
    ScheduleNextRetry(GetBackoffDelay(retryTimes_));
}

uint64_t ReconnectEngine::GetBackoffDelay(int retryTimes) const
{
    if (retryTimes < 0) {
        return config_.GetBackoffTier1Ms();
    }
    if (retryTimes >= config_.GetBackoffTier3Threshold()) {
        return config_.GetBackoffTier3Ms();
    }
    if (retryTimes >= config_.GetBackoffTier2Threshold()) {
        return config_.GetBackoffTier2Ms();
    }
    return config_.GetBackoffTier1Ms();
}

}  // namespace bluetooth
}  // namespace OHOS
