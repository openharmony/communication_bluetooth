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

#include "profile_coordinator.h"

#include "bluetooth_def.h"
#include "log.h"

#include "dispatcher/car_policy_dispatcher.h"
#include "service/car_profile_service.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
namespace {
#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_coord"
#endif

constexpr uint64_t CONNECT_OTHER_PROFILES_DELAY_MS = 15000;
} // namespace

ProfileCoordinator::ProfileCoordinator(CarConnectionGuard &guard, DevicePriorityManager &dpm,
                                       CarPolicyDispatcher &dispatcher)
    : guard_(guard), dpm_(dpm), dispatcher_(dispatcher) {}

void ProfileCoordinator::OnProfileStateChanged(const std::string &addr, ProfileType profile,
                                                int nextState, int prevState)
{
    HILOGI("OnProfileStateChanged: addr=%{public}s, profile=%{public}d, next=%{public}d, prev=%{public}d",
           addr.c_str(), static_cast<int>(profile), nextState, prevState);
    if (profile == ProfileType::HFP_HF) {
        OnHfpHfStateChanged(addr, nextState, prevState);
    } else if (profile == ProfileType::A2DP_SINK) {
        OnA2dpSinkStateChanged(addr, nextState, prevState);
    }
}

void ProfileCoordinator::OnHfpHfStateChanged(const std::string &addr, int nextState, int prevState)
{
    if (nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        // HFP 连接成功：更新 lastDevice + SinkLimit 抢占 + 延时补连 A2DP
        // 可配对广播由 CarPolicyManager::OnProfileStateChanged 入口统一停止，此处不重复处理。
        dpm_.SetLastDevice(addr);
        guard_.DisconnectOthersForSinkLimit(addr);
        CancelPendingConnectOtherProfiles(addr);
        if (guard_.IsAllowConnA2dp()) {
            ScheduleConnectOtherProfiles(addr, CONNECT_OTHER_PROFILES_DELAY_MS);
        }
        // PBAP connect: 本代码库无 PbapPceService，no-op
    } else if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        // PBAP disconnect: no-op
        // HFP 连接失败（CONNECTING→DISCONNECTED）兜底连 A2DP：
        // HFP 连接失败时尝试连接 A2DP，确保至少一个 profile 可用。
        if (prevState == static_cast<int>(BTConnectState::CONNECTING) && guard_.IsAllowConnA2dp()) {
            HILOGI("HFP connect failed, fallback connect A2DP: %{public}s", addr.c_str());
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
        }
    }
}

void ProfileCoordinator::OnA2dpSinkStateChanged(const std::string &addr, int nextState, int prevState)
{
    if (nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        // A2DP 连接成功：更新 lastDevice + SinkLimit 抢占
        // 可配对广播由 CarPolicyManager::OnProfileStateChanged 入口统一停止，此处不重复处理。
        dpm_.SetLastDevice(addr);
        guard_.DisconnectOthersForSinkLimit(addr);

        if (!guard_.IsAllowConnA2dp()) {
            HILOGI("A2DP connected but FA/BtMusic not allowed, disconnect: %{public}s", addr.c_str());
            CarProfileService::Disconnect(ProfileType::A2DP_SINK, addr);
        }
        // 音频焦点通知：A2DP 连接成功且门控允许时，通知音频焦点管理器请求焦点。
        if (guard_.IsAllowConnA2dp()) {
            CarProfileService::NotifyA2dpConnected();
        }
        // 延时补连 HFP：A2DP 连接成功后延时补连 HFP，确保通话 profile 可用。
        // 直接 ++gen 使旧任务失效并安排新任务，无需先 Cancel 再 Schedule（旧实现连续两次自增等价于一次）
        ScheduleConnectOtherProfiles(addr, CONNECT_OTHER_PROFILES_DELAY_MS);
    } else if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        // A2DP 断开：取消 pending 的 HFP 补连。
        CancelPendingConnectOtherProfiles(addr);
    }
}

void ProfileCoordinator::ScheduleConnectOtherProfiles(const std::string &addr, uint64_t delayMs)
{
    int gen = ++pendingConnectGen_[addr];
    std::string addrCopy = addr;
    dispatcher_.DispatchDelayed(
        [this, addrCopy, gen]() {
            auto it = pendingConnectGen_.find(addrCopy);
            if (it == pendingConnectGen_.end() || it->second != gen) {
                HILOGI("Cancel pending ConnectOtherProfiles: %{public}s", addrCopy.c_str());
                return;
            }
            ConnectOtherProfiles(addrCopy);
        },
        delayMs);
}

void ProfileCoordinator::CancelPendingConnectOtherProfiles(const std::string &addr)
{
    auto it = pendingConnectGen_.find(addr);
    if (it != pendingConnectGen_.end()) {
        it->second++;
        HILOGI("Cancel pending ConnectOtherProfiles: %{public}s", addr.c_str());
    }
}

void ProfileCoordinator::ConnectOtherProfiles(const std::string &addr)
{
    // 补连逻辑：检查 HFP 和 A2DP 连接状态，哪个 profile 没连就发起连接。
    if (CarProfileService::GetHfpDeviceState(addr) == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("ConnectOtherProfiles: connect HFP_HF %{public}s", addr.c_str());
        CarProfileService::Connect(ProfileType::HFP_HF, addr);
    }
    if (CarProfileService::GetA2dpDeviceState(addr) == static_cast<int>(BTConnectState::DISCONNECTED)
        && guard_.IsAllowConnA2dp()) {
        HILOGI("ConnectOtherProfiles: connect A2DP_SINK %{public}s", addr.c_str());
        CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
    }
}

}  // namespace bluetooth
}  // namespace OHOS
