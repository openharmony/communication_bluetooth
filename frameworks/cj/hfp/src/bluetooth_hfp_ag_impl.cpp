/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_cj_hfp_ag"
#endif

#include "bluetooth_hfp_ag_impl.h"

#include <vector>

#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "cj_lambda.h"
#include "napi_bluetooth_utils.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<HandsFreeUnitObserverImpl> HandsFreeAudioGatewayImpl::observer_ =
    std::make_shared<HandsFreeUnitObserverImpl>();
bool HandsFreeAudioGatewayImpl::isRegistered_ = false;

CArrString HandsFreeAudioGatewayImpl::GetConnectionDevices(int32_t* errCode)
{
    HILOGD("enter");
    HandsFreeAudioGateway* profile = HandsFreeAudioGateway::GetProfile();
    if (profile == nullptr) {
        HILOGE("profile is nullptr.");
        *errCode = BT_ERR_INTERNAL_ERROR;
        return CArrString { 0 };
    }
    std::vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetConnectedDevices(devices);
    if (errorCode != BT_NO_ERROR) {
        HILOGE("bluetooth assert failed.");
        *errCode = static_cast<int32_t>(errorCode);
        return CArrString { 0 };
    }
    if (devices.size() <= 0) {
        return CArrString { 0 };
    }
    char** deviceVector = static_cast<char**>(malloc(sizeof(char*) * devices.size()));
    if (deviceVector == nullptr) {
        HILOGE("deviceVector malloc failed.");
        *errCode = BT_ERR_INTERNAL_ERROR;
        return CArrString { 0 };
    }
    int64_t i = 0;
    for (auto& device : devices) {
        deviceVector[i] = MallocCString(device.GetDeviceAddr());
        i++;
    }
    CArrString ret = CArrString { .head = deviceVector, .size = i };
    return ret;
}

int32_t HandsFreeAudioGatewayImpl::GetConnectionState(char* device, int32_t* errCode)
{
    HILOGD("enter");
    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    std::string remoteAddr = device;
    if (!IsValidAddress(remoteAddr)) {
        HILOGE("Invalid addr");
        *errCode = static_cast<int32_t>(BT_ERR_INVALID_PARAM);
        return profileState;
    }
    HandsFreeAudioGateway* profile = HandsFreeAudioGateway::GetProfile();
    if (profile == nullptr) {
        HILOGE("profile is nullptr.");
        *errCode = BT_ERR_INTERNAL_ERROR;
        return profileState;
    }
    BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(remoteDevice, state);
    if (errorCode != BT_NO_ERROR) {
        HILOGE("bluetooth assert failed.");
        *errCode = errorCode;
        return profileState;
    }
    profileState = GetProfileConnectionState(state);
    return profileState;
}

void HandsFreeAudioGatewayImpl::On(int32_t type, int64_t id, int32_t* errCode)
{
    if (observer_) {
        auto observerFunc = CJLambda::Create(reinterpret_cast<void (*)(StateChangeParam)>(id));
        if (!observerFunc) {
            HILOGE("Register state change event failed");
            *errCode = BT_ERR_INVALID_PARAM;
            return;
        }
        observer_->RegisterStateChangeFunc(observerFunc);
    }

    if (!isRegistered_) {
        HandsFreeAudioGateway* profile = HandsFreeAudioGateway::GetProfile();
        if (profile == nullptr) {
            HILOGE("profile is nullptr.");
            *errCode = BT_ERR_INVALID_PARAM;
            return;
        }
        profile->RegisterObserver(observer_);
        isRegistered_ = true;
    }
    return;
}
} // namespace Bluetooth
} // namespace OHOS