/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "napi_bluetooth_remote_device_observer.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged(
    uv_work_t *work, std::pair<std::string, int> &data)
{
    HILOGI("OnPairStatusChanged uv_work_t start");

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
    HILOGD("UvQueueWorkOnPairStatusChanged Status is %{public}d", callbackData->data.second);
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

void NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device, int status)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_BONE_STATE_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnPairStatusChanged: This callback is not registered by ability.");
        return;
    }
    HILOGD("NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged: %{public}s is registered by ability",
        REGISTER_BONE_STATE_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_BONE_STATE_TYPE];
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    int bondStatus = 0;
    DealPairStatus(status, bondStatus);

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothRemoteDeviceObserver,
        decltype(&NapiBluetoothRemoteDeviceObserver::UvQueueWorkOnPairStatusChanged), std::pair<std::string, int>>();
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

    work->data = (void *)callbackData;

    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
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
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnRemoteUuidChanged called");
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteNameChanged(
    const BluetoothRemoteDevice &device, const std::string &deviceName)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnRemoteNameChanged called, address is %{public}s, deviceName is "
           "%{public}s",
        device.GetDeviceAddr().c_str(),
        deviceName.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteAliasChanged(
    const BluetoothRemoteDevice &device, const std::string &alias)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnRemoteAliasChanged called, address is %{public}s, alias is "
           "%{public}s",
        device.GetDeviceAddr().c_str(),
        alias.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteCodChanged(
    const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnRemoteCodChanged called, address is %{public}s, cod is "
           "%{public}d",
        device.GetDeviceAddr().c_str(),
        cod.GetClassOfDevice());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteBatteryLevelChanged(
    const BluetoothRemoteDevice &device, int batteryLevel)
{
    HILOGI(
        "NapiBluetoothRemoteDeviceObserver::OnRemoteBatteryLevelChanged called, address is %{public}s, batteryLevel is "
        "%{public}d",
        device.GetDeviceAddr().c_str(),
        batteryLevel);
}

void NapiBluetoothRemoteDeviceObserver ::OnReadRemoteRssiEvent(
    const BluetoothRemoteDevice &device, int rssi, int status)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::OnReadRemoteRssiEvent called, address is %{public}s, rssi is "
           "%{public}d, status is %{public}d",
        device.GetDeviceAddr().c_str(),
        rssi,
        status);
    std::shared_ptr<GattGetRssiValueCallbackInfo> callbackInfo = GetRssiValueCallbackInfo();
    std::unique_lock<std::mutex> lock(callbackInfo->mutexRssi);
    if (status == 0) {
        callbackInfo->promise.errorCode = CODE_SUCCESS;
        callbackInfo->rssi = rssi;
        napi_value result = nullptr;
        napi_create_int32(callbackInfo->env, callbackInfo->rssi, &result);
        callbackInfo->result = result;
    } else {
        callbackInfo->promise.errorCode = CODE_FAILED;
    }
    callbackInfo->cvfull.notify_all();
}

void NapiBluetoothRemoteDeviceObserver::DealPairStatus(const int &status, int &boneStatus)
{
    HILOGI("NapiBluetoothRemoteDeviceObserver::DealPairStatus called, status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            boneStatus = BondState::BOND_STATE_INVALID;
            break;
        case PAIR_PAIRING:
            boneStatus = BondState::BOND_STATE_BONDING;
            break;
        case PAIR_PAIRED:
            boneStatus = BondState::BOND_STATE_BONDED;
            break;
        default:
            break;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS