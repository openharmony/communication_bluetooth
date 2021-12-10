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
#include "napi_bluetooth_host_observer.h"

#include <map>
#include <memory>
#include <string>

#include "bluetooth_log.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothHostObserver::OnStateChanged(const int transport, const int status)
{
    HILOGI("NapiBluetoothHostObserver::OnStateChanged called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_STATE_CHANGE_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiBluetoothHostObserver::OnStateChanged: %{public}s is registered by ability",
        REGISTER_STATE_CHANGE_TYPE.c_str());

    BluetoothState state;
    DealStateChange(transport, status, state);
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_STATE_CHANGE_TYPE];
    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackInfo->env_, &undefined);
    HILOGI("NapiBluetoothHostObserver::OnStateChanged: Status is %{public}d", state);
    napi_create_int32(callbackInfo->env_, static_cast<int32_t>(state), &result);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothHostObserver::OnDiscoveryStateChanged(int status)
{
    HILOGI("NapiBluetoothHostObserver::OnDiscoveryStateChanged called, status is %{public}d", status);
    switch (status) {
        case DISCOVERY_STARTED:
            HILOGI("NapiBluetoothHostObserver::OnDiscoveryStateChanged discover start");
            break;
        case DISCOVERYING:
            HILOGI("NapiBluetoothHostObserver::OnDiscoveryStateChanged discovering");
            break;
        case DISCOVERY_STOPED:
            HILOGI("NapiBluetoothHostObserver::OnDiscoveryStateChanged discover stop");
            break;
        default:
            HILOGI("NapiBluetoothHostObserver::OnDiscoveryStateChanged other status");
            break;
    }
}

void NapiBluetoothHostObserver::OnDiscoveryResult(const BluetoothRemoteDevice &device)
{
    HILOGI("NapiBluetoothHostObserver::OnDiscoveryResult called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_DEVICE_FIND_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnDiscoveryResult: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiBluetoothHostObserver::OnDiscoveryResult: %{public}s is registered by ability",
        REGISTER_DEVICE_FIND_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_DEVICE_FIND_TYPE];
    napi_value result = 0;
    napi_value value = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackInfo->env_, &undefined);
    HILOGI("NapiBluetoothHostObserver::OnDiscoveryResult: Status is %{public}s", device.GetDeviceAddr().c_str());
    napi_create_array(callbackInfo->env_, &result);
    napi_create_string_utf8(callbackInfo->env_, device.GetDeviceAddr().c_str(), device.GetDeviceAddr().size(), &value);
    napi_set_element(callbackInfo->env_, result, 0, value);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothHostObserver::OnPairRequested(const BluetoothRemoteDevice &device)
{
    HILOGI("NapiBluetoothHostObserver::OnPairRequested called");
    BluetoothRemoteDevice remoteDevice;
    if (device.GetTransportType() == BT_TRANSPORT_BREDR) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BREDR);
    } else if (device.GetTransportType() == BT_TRANSPORT_BLE) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BLE);
    }
    remoteDevice.PairRequestReply(true);
}

void NapiBluetoothHostObserver::OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number)
{
    HILOGI("NapiBluetoothHostObserver::OnPairConfirmed called");
    if (device.GetTransportType() == BT_TRANSPORT_BREDR) {
        DealBredrPairComfirmed(device.GetDeviceAddr(), reqType, number);
    } else if (device.GetTransportType() == BT_TRANSPORT_BLE) {
        DealBlePairComfirmed(device.GetDeviceAddr(), reqType, number);
    }
}

void NapiBluetoothHostObserver::OnScanModeChanged(int mode)
{
    HILOGI("NapiBluetoothHostObserver::OnScanModeChanged called, mode is %{public}d", mode);
}

void NapiBluetoothHostObserver::OnDeviceNameChanged(const std::string &deviceName)
{
    HILOGI("NapiBluetoothHostObserver::OnDeviceNameChanged called, name is %{public}s", deviceName.c_str());
}

void NapiBluetoothHostObserver::OnDeviceAddrChanged(const std::string &address)
{
    HILOGI("NapiBluetoothHostObserver::OnDeviceAddrChanged called, address is %{public}s", address.c_str());
}

void NapiBluetoothHostObserver::DealStateChange(const int transport, const int status, BluetoothState &state)
{
    if (transport == BT_TRANSPORT_BREDR) {
        switch (status) {
            case BTStateID::STATE_TURNING_ON:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turning on");
                state = BluetoothState::STATE_TURNING_ON;
                break;
            case BTStateID::STATE_TURN_ON:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turn on");
                state = BluetoothState::STATE_ON;
                break;
            case BTStateID::STATE_TURNING_OFF:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turning off");
                state = BluetoothState::STATE_TURNING_OFF;
                break;
            case BTStateID::STATE_TURN_OFF:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turn off");
                state = BluetoothState::STATE_OFF;
                break;
            default:
                break;
        }
    } else if (transport == BT_TRANSPORT_BLE) {
        switch (status) {
            case BTStateID::STATE_TURNING_ON:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BLE Turning on");
                state = BluetoothState::STATE_BLE_TURNING_ON;
                break;
            case BTStateID::STATE_TURN_ON:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BLE Turn on");
                state = BluetoothState::STATE_BLE_ON;
                break;
            case BTStateID::STATE_TURNING_OFF:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BLE Turning off");
                state = BluetoothState::STATE_BLE_TURNING_OFF;
                break;
            case BTStateID::STATE_TURN_OFF:
                HILOGI("NapiBluetoothHostObserver::OnStateChanged BLE Turn off");
                state = BluetoothState::STATE_OFF;
                break;
            default:
                break;
        }
    } else {
        HILOGI("NapiBluetoothHostObserver::OnStateChanged error");
    }
}

void NapiBluetoothHostObserver::DealBredrPairComfirmed(const std::string &addr, const int reqType, const int number)
{
    switch (reqType) {
        case PAIR_CONFIRM_TYPE_PIN_CODE: {
            HILOGI("PAIR_CONFIRM_TYPE_PIN_CODE");
            break;
        }
        case PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY: {
            HILOGI("PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY");
            break;
        }
        case PAIR_CONFIRM_TYPE_PASSKEY_INPUT: {
            HILOGI("PAIR_CONFIRM_TYPE_PASSKEY_INPUT");
            break;
        }
        case PAIR_CONFIRM_TYPE_NUMERIC: {
            HILOGI("PAIR_CONFIRM_TYPE_NUMERIC, please setPairConfirm.");
            OnPairConfirmedCallBack(addr, number);
            break;
        }
        case PAIR_CONFIRM_TYPE_CONSENT: {
            HILOGI("PAIR_CONFIRM_TYPE_CONSENT, please setPairConfirm.");
            OnPairConfirmedCallBack(addr, number);
            break;
        }
        default:
            break;
    }
}

void NapiBluetoothHostObserver::DealBlePairComfirmed(const std::string &addr, const int reqType, const int number)
{
    switch (reqType) {
        case PAIR_CONFIRM_TYPE_PIN_CODE: {
            HILOGI("PAIR_CONFIRM_TYPE_PIN_CODE");
            break;
        }
        case PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY: {
            HILOGI("PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY");
            break;
        }
        case PAIR_CONFIRM_TYPE_PASSKEY_INPUT: {
            HILOGI("PAIR_CONFIRM_TYPE_PASSKEY_INPUT");
            break;
        }
        case PAIR_CONFIRM_TYPE_NUMERIC: {
            HILOGI("PAIR_CONFIRM_TYPE_NUMERIC, please setPairConfirm.");
            OnPairConfirmedCallBack(addr, number);
            break;
        }
    }
}

void NapiBluetoothHostObserver::OnPairConfirmedCallBack(const std::string &deviceAddr, const int number)
{
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_PIN_REQUEST_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnPairRequested: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiBluetoothHostObserver::OnPairRequested: %{public}s is registered by ability",
        REGISTER_PIN_REQUEST_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_PIN_REQUEST_TYPE];
    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackInfo->env_, &undefined);
    HILOGI("NapiBluetoothHostObserver::OnPairRequested: Addr is %{public}s", deviceAddr.c_str());
    napi_create_object(callbackInfo->env_, &result);
    napi_value device = 0;
    napi_create_string_utf8(callbackInfo->env_, deviceAddr.c_str(), deviceAddr.size(), &device);
    napi_set_named_property(callbackInfo->env_, result, "deviceId", device);
    napi_value pinCode = 0;
    napi_create_int32(callbackInfo->env_, number, &pinCode);
    napi_set_named_property(callbackInfo->env_, result, "pinCode", pinCode);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}
}  // namespace Bluetooth
}  // namespace OHOS