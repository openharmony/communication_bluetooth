/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "bluetooth_a2dp_impl.h"

#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_utils.h"
#include "bluetooth_errorcode.h"
#include "cj_lambda.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<A2dpSourceObserverImpl> A2dpSourceProfileImpl::observer_ = std::make_shared<A2dpSourceObserverImpl>();
bool A2dpSourceProfileImpl::isRegistered_ = false;

int32_t A2dpSourceProfileImpl::GetPlayingState(char* deviceId, int32_t* errCode)
{
    int32_t state = PlayingState::STATE_NOT_PLAYING;
    std::string remoteAddr = deviceId;
    if (IsValidAddress(remoteAddr)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return state;
    }
    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource* profile = A2dpSource::GetProfile();
    *errCode = profile->GetPlayingState(remoteDevice, state);
    return state;
}

CArrString A2dpSourceProfileImpl::GetConnectionDevices(int32_t* errCode)
{
    HILOGI("enter");
    A2dpSource* profile = A2dpSource::GetProfile();
    std::vector<int> states;
    states.push_back(1);
    std::vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetDevicesByStates(states, devices);
    if (errorCode != BT_NO_ERROR) {
        *errCode = static_cast<int32_t>(errorCode);
        return CArrString { 0 };
    }
    if (devices.size() <= 0) {
        return CArrString { 0 };
    }
    char** deviceVector = static_cast<char**>(malloc(sizeof(char*) * devices.size()));
    if (deviceVector == nullptr) {
        return CArrString { 0 };
    }
    int64_t i = 0;
    for (auto& device : devices) {
        deviceVector[i] = MallocCString(device.GetDeviceAddr());
        if (deviceVector[i] == nullptr) {
            for (int64_t j = 0; j < i; j++) {
                free(deviceVector[j]);
            }
            free(deviceVector);
            *errCode = static_cast<int32_t>(BT_ERR_BASE_SYSCAP);
            return CArrString { 0 };
        }
        i++;
    }
    CArrString ret = CArrString { .head = deviceVector, .size = i };
    return ret;
}

int32_t A2dpSourceProfileImpl::GetConnectionState(char* device, int32_t* errCode)
{
    HILOGI("enter");
    std::string deviceId = device;
    A2dpSource* profile = A2dpSource::GetProfile();
    BluetoothRemoteDevice remoteDevice(deviceId, 1);
    int btConnectState = static_cast<int>(BTConnectState::DISCONNECTED);
    int errorCode = profile->GetDeviceState(remoteDevice, btConnectState);
    if (errorCode != BT_NO_ERROR) {
        *errCode = static_cast<int32_t>(errorCode);
        return ProfileConnectionState::STATE_DISCONNECTED;
    }
    return GetProfileConnectionState(btConnectState);
}

void A2dpSourceProfileImpl::On(int32_t type, int64_t id, int32_t* errCode)
{
    if (observer_) {
        auto observerFunc = CJLambda::Create(reinterpret_cast<void (*)(StateChangeParam)>(id));
        if (!observerFunc) {
            HILOGD("Register state change event failed");
            *errCode = BT_ERR_INVALID_PARAM;
            return;
        }
        observer_->RegisterStateChangeFunc(observerFunc);
    }

    if (!isRegistered_) {
        A2dpSource* profile = A2dpSource::GetProfile();
        profile->RegisterObserver(observer_);
        isRegistered_ = true;
    }
}
} // namespace Bluetooth
} // namespace OHOS