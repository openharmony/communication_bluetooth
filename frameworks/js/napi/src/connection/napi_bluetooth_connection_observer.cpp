/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "napi_bluetooth_connection_observer.h"

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_connection.h"

#include <uv.h>

namespace {
constexpr uint32_t FORMAT_PINCODE_LENGTH = 6;
}  // namespace

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothConnectionObserver::OnDiscoveryStateChanged(int status)
{
    switch (status) {
        case DISCOVERY_STARTED:
            HILOGD("DISCOVERY_STARTED(1)");
            break;
        case DISCOVERYING:
            HILOGD("DISCOVERYING(2)");
            break;
        case DISCOVERY_STOPED:
            HILOGD("DISCOVERY_STOPED(3)");
            break;
        default:
            HILOGE("invaild status is %{public}d", status);
            break;
    }
}

void NapiBluetoothConnectionObserver::UvQueueWorkOnDiscoveryResult(
    uv_work_t *work, std::shared_ptr<BluetoothRemoteDevice> &device)
{
    HILOGD("start");

    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothConnectionObserver,
        decltype(&NapiBluetoothConnectionObserver::UvQueueWorkOnDiscoveryResult),
        std::shared_ptr<BluetoothRemoteDevice>> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }

    napi_value result = 0;
    napi_value value = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackData->env, &undefined);
    HILOGD("deviceId is %{public}s", GetEncryptAddr(device->GetDeviceAddr()).c_str());
    napi_create_array(callbackData->env, &result);
    napi_create_string_utf8(callbackData->env, device->GetDeviceAddr().c_str(), device->GetDeviceAddr().size(), &value);
    napi_set_element(callbackData->env, result, 0, value);

    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothConnectionObserver::OnDiscoveryResult(const BluetoothRemoteDevice &device)
{
    HILOGD("start");
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = GetCallback(REGISTER_DEVICE_FIND_TYPE);
    if (callbackInfo == nullptr) {
        HILOGD("This callback is not registered by ability.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothConnectionObserver,
        decltype(&NapiBluetoothConnectionObserver::UvQueueWorkOnDiscoveryResult),
        std::shared_ptr<BluetoothRemoteDevice>>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    callbackData->object = this;
    callbackData->function = &NapiBluetoothConnectionObserver::UvQueueWorkOnDiscoveryResult;
    callbackData->env = callbackInfo->env_;
    callbackData->callback = callbackInfo->callback_;
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    callbackData->data = remoteDevice;

    AddDiscoveryDevice(remoteDevice);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }

    work->data = (void *)callbackData;

    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
}

void NapiBluetoothConnectionObserver::OnPairRequested(const BluetoothRemoteDevice &device)
{
    HILOGI("start");
    BluetoothRemoteDevice remoteDevice;
    if (device.GetTransportType() == BT_TRANSPORT_BREDR) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BREDR);
    } else if (device.GetTransportType() == BT_TRANSPORT_BLE) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BLE);
    }
    remoteDevice.PairRequestReply(true);
}

void NapiBluetoothConnectionObserver::OnPairConfirmed(const BluetoothRemoteDevice &device, int pinType, int number)
{
    HILOGI("OnPairConfirmed");
    std::shared_ptr<PairConfirmedCallBackInfo> pairConfirmInfo =
        std::make_shared<PairConfirmedCallBackInfo>(number, pinType, device.GetDeviceAddr());
    OnPairConfirmedCallBack(pairConfirmInfo);
}

void NapiBluetoothConnectionObserver::OnScanModeChanged(int mode)
{
    HILOGI("mode is %{public}d", mode);
}

void NapiBluetoothConnectionObserver::OnDeviceNameChanged(const std::string &deviceName)
{
    HILOGI("name is %{public}s", deviceName.c_str());
}

void NapiBluetoothConnectionObserver::OnDeviceAddrChanged(const std::string &address)
{
    HILOGI("address is %{public}s", GetEncryptAddr(address).c_str());
}

void NapiBluetoothConnectionObserver::RegisterCallback(
    const std::string &callbackName, const std::shared_ptr<BluetoothCallbackInfo> &callback)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_[callbackName] = callback;
}

void NapiBluetoothConnectionObserver::DeRegisterCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_.erase(callbackName);
}

std::shared_ptr<BluetoothCallbackInfo> NapiBluetoothConnectionObserver::GetCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    if (callbacks_.find(callbackName) != callbacks_.end()) {
        return callbacks_[callbackName];
    }
    return nullptr;
}

static std::string GetFormatPinCode(const uint32_t pinType, const uint32_t pinCode)
{
    std::string pinCodeStr = std::to_string(pinCode);
    if (pinType != PIN_TYPE_CONFIRM_PASSKEY && pinType != PIN_TYPE_NOTIFY_PASSKEY) {
        return pinCodeStr;
    }
    while (pinCodeStr.length() < FORMAT_PINCODE_LENGTH) {
        pinCodeStr = "0" + pinCodeStr;
    }
    return pinCodeStr;
}

void NapiBluetoothConnectionObserver::UvQueueWorkOnPairConfirmedCallBack(
    uv_work_t *work, const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo)
{
    HILOGI("start");
    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothConnectionObserver,
        decltype(&NapiBluetoothConnectionObserver::UvQueueWorkOnPairConfirmedCallBack),
        std::shared_ptr<PairConfirmedCallBackInfo>> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }

    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackData->env, &undefined);
    HILOGI("Addr is %{public}s", GetEncryptAddr(callbackData->data->deviceAddr).c_str());
    napi_create_object(callbackData->env, &result);
    napi_value device = 0;
    napi_create_string_utf8(
        callbackData->env, callbackData->data->deviceAddr.c_str(), callbackData->data->deviceAddr.size(), &device);
    napi_set_named_property(callbackData->env, result, "deviceId", device);
    napi_value pinCode = 0;
    std::string pinCodeStr = GetFormatPinCode(callbackData->data->pinType, callbackData->data->number);
    napi_create_string_utf8(callbackData->env, pinCodeStr.c_str(), pinCodeStr.size(), &pinCode);
    napi_set_named_property(callbackData->env, result, "pinCode", pinCode);
    napi_value pinType = 0;
    napi_create_int32(callbackData->env, callbackData->data->pinType, &pinType);
    napi_set_named_property(callbackData->env, result, "pinType", pinType);

    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothConnectionObserver::OnPairConfirmedCallBack(
    const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo)
{
    HILOGI("start");
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = GetCallback(REGISTER_PIN_REQUEST_TYPE);
    if (callbackInfo == nullptr) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothConnectionObserver,
        decltype(&NapiBluetoothConnectionObserver::UvQueueWorkOnPairConfirmedCallBack),
        std::shared_ptr<PairConfirmedCallBackInfo>>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    callbackData->object = this;
    callbackData->function = &NapiBluetoothConnectionObserver::UvQueueWorkOnPairConfirmedCallBack;
    callbackData->env = callbackInfo->env_;
    callbackData->callback = callbackInfo->callback_;
    callbackData->data = pairConfirmInfo;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }

    work->data = (void *)callbackData;

    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
