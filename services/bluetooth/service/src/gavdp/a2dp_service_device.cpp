/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_a2dp"
#endif

#include "a2dp_service_device.h"

#include <mutex>

#include "context.h"
#include "log.h"
#include "memory.h"
#include "bt_recursive_mutex.h"
#include "log_utils.h"

namespace OHOS {
namespace bluetooth {
BtRecursiveMutex g_deviceMutex {};

A2dpDeviceInfo::A2dpDeviceInfo(const RawAddress &device)
{
    currentConnectState_ = static_cast<int>(BTConnectState::DISCONNECTED);
    peerAddress_ = device;
    state_.SetDevice(device);
}

A2dpDeviceInfo::~A2dpDeviceInfo()
{
}

RawAddress A2dpDeviceInfo::GetDevice() const
{
    return peerAddress_;
}

A2dpStateManager *A2dpDeviceInfo::GetStateMachine()
{
    return &state_;
}

void A2dpDeviceInfo::SetCodecStatus(A2dpSrcCodecStatus codecStatusInfo)
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);

    codecStatus_ = codecStatusInfo;
}

A2dpSrcCodecStatus A2dpDeviceInfo::GetCodecStatus() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);
    return codecStatus_;
}

void A2dpDeviceInfo::SetPlayingState(bool state)
{
    HILOGI("[A2dpDeviceInfo] playState(%{public}d)", state);
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);

    isPlaying_ = state;
}

bool A2dpDeviceInfo::GetPlayingState() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);

    return isPlaying_;
}

void A2dpDeviceInfo::SetConnectState(int state)
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);
    HILOGI("Set state=%{public}d,  currentState=%{public}d", state, currentConnectState_);
    if (state == static_cast<int>(BTConnectState::CONNECTING) &&
        currentConnectState_ == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGW("The connecting state cannot overwrite the connected state");
        return;
    }
    preConnectState_ = currentConnectState_;
    currentConnectState_ = state;
}

int A2dpDeviceInfo::GetConnectState() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);

    HILOGI_TIME_LIMIT(__func__, "[currentConnectState_] = %{public}d", currentConnectState_);
    return currentConnectState_;
}

void A2dpDeviceInfo::SetOffloadCodecStatus(A2dpSrcOffloadCodecStatus offloadCodecStatus)
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);
    offloadCodecStatus_ = offloadCodecStatus;
}

A2dpSrcOffloadCodecStatus A2dpDeviceInfo::GetOffloadCodecStatus() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_deviceMutex);
    return offloadCodecStatus_;
}

void A2dpDeviceInfo::SetTryReconnectA2dp(bool tryToReconnect)
{
    tryToReconnect_ = tryToReconnect;
}

bool A2dpDeviceInfo::GetTryReconnectA2dp() const
{
    return tryToReconnect_;
}

int A2dpDeviceInfo::GetPreConnectState() const
{
    return preConnectState_;
}

bool A2dpDeviceInfo::IsHdapConnected() const
{
    return isHdapConnected_;
}

void A2dpDeviceInfo::SetIsHdapConnected(bool isHdapConnected)
{
    isHdapConnected_ = isHdapConnected;
}

const A2dpSrcCodecInfo &A2dpDeviceInfo::GetCodecInfo() const
{
    return codecInfo_;
}

void A2dpDeviceInfo::SetCodecInfo(const A2dpSrcCodecInfo &codecInfo)
{
    codecInfo_ = codecInfo;
}
}  // namespace bluetooth
}  // namespace OHOS