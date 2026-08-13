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

#ifndef RECONNECT_ENGINE_H
#define RECONNECT_ENGINE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "car_policy_config.h"
#include "guard/car_connection_guard.h"
#include "reconnect_predicates.h"

namespace OHOS {
namespace bluetooth {

class DevicePriorityManager;
class CarPolicyDispatcher;
class CarMetricsReporter;

/// 回连引擎。
///
/// 单一 HandleReconnect 方法既是入口也是重试循环，
/// 每次执行完整检查链（谓词 → Guard.Check → Connect → 计数 → 调度下次）。
/// 用 epoch_ 标识当前回连会话，过期延时回调通过 IsStale() 丢弃。
class ReconnectEngine {
public:
    ReconnectEngine(CarPolicyConfig &config, DevicePriorityManager &dpm,
                    CarPolicyDispatcher &dispatcher, CarMetricsReporter &metrics,
                    CarConnectionGuard &guard);
    ~ReconnectEngine();

    void AddPredicate(std::unique_ptr<IReconnectPredicate> predicate);

    /// 主动发起回连（蓝牙开启 / 配对成功 / 智能通话转入）。
    void StartReconnect(const std::string &addr, ReconnectReason reason);
    /// ACL 断开触发的回连（延时后执行，延时由 reason 决定）。
    void OnAclDisconnect(const std::string &addr, int rawReason);
    /// 所有 profile 断开（记录次选设备）。
    void OnAllProfilesDisconnected(const std::string &addr);

    void Terminate();
    void Reset();

    int GetRetryTimes() const { return retryTimes_; }
    bool IsActive() const { return isActive_; }
    std::string GetCurrentDevice() const { return currentDevice_; }
    ReconnectReason GetCurrentReason() const { return currentReason_; }

private:
    /// 统一回连处理。
    /// 谓词检查 → Guard.Check(+SinkLimit 抢占) → Connect → retryTimes++ → maxRetry/singleRetry → 调度下次。
    void HandleReconnect();
    /// 调度一次延时 HandleReconnect；回调执行时若 epoch 已变则丢弃。
    void ScheduleNextRetry(uint64_t delayMs);
    /// BT_ON 场景 800ms 后检查 HFP 是否仍 DISCONNECTED，是则重连一次。
    /// softbus 竞争可能导致首次 connect 请求被吞掉，此处做容错重试。
    void ScheduleHfpRetry(const std::string &addr, uint64_t delayMs);
    /// 谓词检查，命中则调 OnTerminate 并返回 true。
    bool CheckAndFireTerminate();
    /// Guard.Check + SinkLimit 抢占，返回最终是否允许连接。
    bool CheckGuardAndPreempt();
    /// 推进 epoch 使所有已排队的延时回调失效。
    /// 新会话开始或终止时调用，旧回调执行时通过 IsStale() 判定后丢弃，避免串会话连接。
    void BumpEpoch();
    /// 延时回调通过此判断是否属于已过期的回连会话。
    bool IsStale(int epoch) const { return epoch != epoch_; }
    uint64_t GetBackoffDelay(int retryTimes) const;

    CarPolicyConfig &config_;
    DevicePriorityManager &dpm_;
    CarPolicyDispatcher &dispatcher_;
    CarMetricsReporter &metrics_;
    CarConnectionGuard &guard_;
    std::vector<std::unique_ptr<IReconnectPredicate>> predicates_;

    int retryTimes_{0};
    bool isActive_{false};
    std::string currentDevice_;
    ReconnectReason currentReason_{ReconnectReason::BT_ON_AUTO_CONN};
    /// 回连会话标识：StartReconnect/Terminate/Reset 时自增，
    /// 延时回调捕获当时的 epoch_，执行时若不等则说明已有新会话，丢弃本次回调。
    int epoch_{0};
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // RECONNECT_ENGINE_H
