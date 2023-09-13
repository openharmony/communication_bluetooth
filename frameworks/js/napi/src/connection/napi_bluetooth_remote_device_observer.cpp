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
#include <map>
#include <memory>
#include <string>

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_remote_device_observer.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_connection.h"

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged(
    uv_work_t *work, std::pair<std::string, int> &data)
{
    HILOGI("uv_work_t start");
    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothRemoteDeviceObserver,
        decltype(&NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged),
        std::pair<std::string, int>> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }

    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;

    napi_get_undefined(callbackData->env, &undefined);
    HILOGI("Status is %{public}d", callbackData->data.second);
    napi_create_object(callbackData->env, &result);
    napi_value device = 0;
    napi_create_string_utf8(
        callbackData->env, callbackData->data.first.data(), callbackData->data.first.size(), &device);
    napi_set_named_property(callbackData->env, result, "deviceId", device);
    napi_value bondState = 0;
    napi_create_int32(callbackData->env, callbackData->data.second, &bondState);
    napi_set_named_property(callbackData->env, result, "state", bondState);
    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothRemoteDeviceObserver::OnAclStateChanged(
    const BluetoothRemoteDevice &device, int state, unsigned int reason)
{}

void NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device, int status)
{
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = GetCallback(REGISTER_BOND_STATE_TYPE);
    if (callbackInfo == nullptr) {
        HILOGD("This callback is not registered by ability.");
        return;
    }
    HILOGI("addr:%{public}s, status:%{public}d", GET_ENCRYPT_ADDR(device), status);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    int bondStatus = 0;
    DealPairStatus(status, bondStatus);

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothRemoteDeviceObserver,
        decltype(&NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged),
        std::pair<std::string, int>>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    std::string deviceAddr = device.GetDeviceAddr();
    callbackData->object = this;
    callbackData->function = &NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged;
    callbackData->env = callbackInfo->env_;
    callbackData->callback = callbackInfo->callback_;
    callbackData->data = std::pair<std::string, int>(deviceAddr, bondStatus);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }

    work->data = static_cast<void *>(callbackData);

    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteUuidChanged(
    const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids)
{
    HILOGD("called");
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteNameChanged(
    const BluetoothRemoteDevice &device, const std::string &deviceName)
{
    HILOGD("addr:%{public}s, deviceName:%{public}s", GET_ENCRYPT_ADDR(device), deviceName.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteAliasChanged(
    const BluetoothRemoteDevice &device, const std::string &alias)
{
    HILOGD("addr:%{public}s, alias:%{public}s", GET_ENCRYPT_ADDR(device), alias.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteCodChanged(
    const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod)
{
    HILOGD("addr:%{public}s, cod:%{public}d", GET_ENCRYPT_ADDR(device), cod.GetClassOfDevice());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteBatteryLevelChanged(
    const BluetoothRemoteDevice &device, int batteryLevel)
{
    HILOGD("addr:%{public}s, batteryLevel:%{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
}

void NapiBluetoothRemoteDeviceObserver ::OnReadRemoteRssiEvent(
    const BluetoothRemoteDevice &device, int rssi, int status)
{
    HILOGD("addr:%{public}s, rssi:%{public}d, status is %{public}d", GET_ENCRYPT_ADDR(device), rssi, status);
}

void NapiBluetoothRemoteDeviceObserver::RegisterCallback(
    const std::string &callbackName, const std::shared_ptr<BluetoothCallbackInfo> &callback)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_[callbackName] = callback;
}

void NapiBluetoothRemoteDeviceObserver::DeRegisterCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_.erase(callbackName);
}

std::shared_ptr<BluetoothCallbackInfo> NapiBluetoothRemoteDeviceObserver::GetCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    if (callbacks_.find(callbackName) != callbacks_.end()) {
        return callbacks_[callbackName];
    }
    return nullptr;
}

void NapiBluetoothRemoteDeviceObserver::DealPairStatus(const int &status, int &bondStatus)
{
    HILOGI("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = static_cast<int>(BondState::BOND_STATE_INVALID);
            break;
        case PAIR_PAIRING:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDING);
            break;
        case PAIR_PAIRED:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS