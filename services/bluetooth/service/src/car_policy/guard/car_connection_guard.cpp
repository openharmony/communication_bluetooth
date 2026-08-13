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

#include "car_connection_guard.h"

#include <set>

#include "a2dp_snk_service.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_def.h"
#include "car_profile_service.h"
#include "hfp_hf_service.h"
#include "log.h"
#include "raw_address.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_guard"
#endif

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

CheckResult CarConnectionGuard::Check(const std::string &addr, ProfileType profile) const
{
    // 1) SinkLimit：A2dpSnk + HfpHf 合计 ≤ CAR_MAX_CONN
    if (!IsConnectionAllowedBySinkLimit(addr)) {
        return CheckResult::Deny(DenyReason::SINK_LIMIT);
    }
    // 2) A2dpGate：仅 A2DP_SINK 受 FA/BtMusic 门控
    if (profile == ProfileType::A2DP_SINK && !IsAllowConnA2dp()) {
        return CheckResult::Deny(DenyReason::A2DP_GATE);
    }
    // CarPlay 互斥由 CarplayMutexPredicate 处理（终止语义），不在 Guard.Check 中重复检查
    return CheckResult::Allow();
}

std::vector<std::string> CarConnectionGuard::GetA2dpConnectedAddrs() const
{
    std::vector<std::string> result;
    auto *svc = A2dpSnkService::GetService();
    if (svc == nullptr) {
        return result;
    }
    std::vector<int32_t> states = {static_cast<int32_t>(BTConnectState::CONNECTING),
                                    static_cast<int32_t>(BTConnectState::CONNECTED)};
    for (const auto &d : svc->GetDevicesByStates(states)) {
        result.push_back(d.GetAddress());
    }
    return result;
}

std::vector<std::string> CarConnectionGuard::GetHfpConnectedAddrs() const
{
    std::vector<std::string> result;
    auto *svc = HfpHfService::GetService();
    if (svc == nullptr) {
        return result;
    }
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTING),
                                static_cast<int>(BTConnectState::CONNECTED)};
    for (const auto &d : svc->GetDevicesByStates(states)) {
        result.push_back(d.GetAddress());
    }
    return result;
}

bool CarConnectionGuard::IsConnectionAllowedBySinkLimit(const std::string &addr) const
{
    // 收集当前 CONNECTING/CONNECTED 的设备地址（A2DP + HFP 合并去重）
    std::set<std::string> connectedSet;
    for (const auto &a : GetA2dpConnectedAddrs()) {
        connectedSet.insert(a);
    }
    for (const auto &a : GetHfpConnectedAddrs()) {
        connectedSet.insert(a);
    }
    // addr 已在连接集中：说明该设备已有 profile 在连（如 A2DP 已连），同设备的其他 profile
    // 不受 SinkLimit 限制（SinkLimit 限制的是不同设备数，不是 profile 数）。
    // 主要服务于 ProfileCoordinator dep4（A2DP 已连 → 补连 HFP）和配对成功后 Check 场景。
    // 在 ReconnectEngine 回连路径中不会命中（引擎在任一 CONNECTED 时已 Terminate）。
    if (connectedSet.count(addr) > 0) {
        return true;
    }
    // addr 不在连接集中：检查已连接的不同设备数是否已达上限
    return static_cast<int>(connectedSet.size()) < CAR_MAX_CONN;
}

void CarConnectionGuard::DisconnectOthersForSinkLimit(const std::string &addr) const
{
    std::vector<std::string> toDisconnect;
    for (const auto &a : GetA2dpConnectedAddrs()) {
        if (a != addr) {
            toDisconnect.push_back(a);
        }
    }
    for (const auto &a : GetHfpConnectedAddrs()) {
        if (a != addr && std::find(toDisconnect.begin(), toDisconnect.end(), a) == toDisconnect.end()) {
            toDisconnect.push_back(a);
        }
    }
    for (const auto &a : toDisconnect) {
        HILOGI("DisconnectOthersForSinkLimit: disconnect %{public}s", a.c_str());
        CarProfileService::Disconnect(ProfileType::A2DP_SINK, a);
        CarProfileService::Disconnect(ProfileType::HFP_HF, a);
    }
}

bool CarConnectionGuard::IsAllowConnA2dp() const
{
    // fail-open：任一状态未初始化（UNKNOWN）时允许连接。
    // DataShare 读取可能在 BT ON 后短暂返回 UNKNOWN，此时不应阻塞连接，故 fail-open。
    // FA==BT_ON 或 BtMusic==ON 或任一==UNKNOWN → 允许 A2DP 连接。
    if (faStatus_ == FA_STATUS_BT_ON || btMusicStatus_ == BT_MUSIC_STATUS_ON) {
        return true;
    }
    if (faStatus_ == STATUS_UNKNOWN) {
        return true;
    }
    return false;
}

void CarConnectionGuard::SetFaStatus(int status)
{
    HILOGI("FA status: %{public}d -> %{public}d", faStatus_, status);
    faStatus_ = status;
    // FA 切走时清除 BtMusic 状态：用户切换到非蓝牙媒体源时，BtMusic 应重置为 UNKNOWN，
    // 下次 A2DP 连接需重新检查。仅在状态为实际值（非 UNKNOWN）时触发清除。
    if (faStatus_ != FA_STATUS_BT_ON && faStatus_ != STATUS_UNKNOWN &&
        btMusicStatus_ == BT_MUSIC_STATUS_ON) {
        btMusicStatus_ = BT_MUSIC_STATUS_OFF;
    }
}

void CarConnectionGuard::SetBtMusicStatus(int status)
{
    HILOGI("BtMusic status: %{public}d -> %{public}d", btMusicStatus_, status);
    btMusicStatus_ = status;
}

bool CarConnectionGuard::IsCarPlayMutex() const
{
    // TODO: 后续通过 DataShare 读取 last_connect_device 实现 CarPlay 互斥判定，当前先默认返回 false
    return false;
}

}  // namespace bluetooth
}  // namespace OHOS
