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

#ifndef PROFILE_COORDINATOR_H
#define PROFILE_COORDINATOR_H

#include <string>
#include <unordered_map>

#include "guard/car_connection_guard.h"  // ProfileType
#include "reconnect/device_priority_manager.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyDispatcher;

/// Profile 协同器：协调多 profile 连接时序——HFP 先连，A2DP/PBAP 延时补连。
/// 处理 HFP/A2DP 状态变化，并在 HFP 连接成功后延时补连 A2DP/PBAP。
/// 按状态分支直接写逻辑，不用依赖图。
class ProfileCoordinator {
public:
    ProfileCoordinator(CarConnectionGuard &guard, DevicePriorityManager &dpm, CarPolicyDispatcher &dispatcher);
    ~ProfileCoordinator() = default;

    /// Profile 状态变更入口（由 CarPolicyManager::OnProfileStateChanged 转发）。
    void OnProfileStateChanged(const std::string &addr, ProfileType profile, int nextState, int prevState);

private:
    /// HFP_HF 状态变化处理：HFP CONNECTED 后记录 lastDevice、SinkLimit 抢占、
    /// 延时补连 A2DP/PBAP；HFP 连接失败（CONNECTING→DISCONNECTED）兜底连 A2DP。
    void OnHfpHfStateChanged(const std::string &addr, int nextState, int prevState);
    /// A2DP_SINK 状态变化处理：A2DP CONNECTED 后门控校验（IsAllowConnA2dp）、
    /// 音频焦点通知、延时补连 HFP；A2DP 闪连闪断第二道防线（FA/BtMusic 不允许时立即断开）。
    void OnA2dpSinkStateChanged(const std::string &addr, int nextState, int prevState);
    /// 延时补连未连接的 profile：HFP 连接成功后延时 15s，
    /// 检查 A2DP/HFP 哪个没连就连哪个。
    void ScheduleConnectOtherProfiles(const std::string &addr, uint64_t delayMs);
    /// 取消 addr 的 pending 补连延时任务。
    void CancelPendingConnectOtherProfiles(const std::string &addr);
    /// 补连实际执行：检查 HFP 和 A2DP 连接状态，哪个没连就发起连接。
    /// A2DP 补连前需检查 IsAllowConnA2dp 门控。
    void ConnectOtherProfiles(const std::string &addr);

    CarConnectionGuard &guard_;
    DevicePriorityManager &dpm_;
    CarPolicyDispatcher &dispatcher_;
    /// per-addr generation，CancelPendingConnectOtherProfiles 时自增使旧延时回调失效。
    std::unordered_map<std::string, int> pendingConnectGen_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PROFILE_COORDINATOR_H
