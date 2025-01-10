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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_connection_observer"
#endif

#include "napi_bluetooth_connection_observer.h"

#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_connection.h"
#include "napi_event_subscribe_module.h"

#include <uv.h>

namespace OHOS {
namespace Bluetooth {
NapiBluetoothConnectionObserver::NapiBluetoothConnectionObserver()
    : eventSubscribe_({REGISTER_DEVICE_FIND_TYPE,
        REGISTER_DISCOVERY_RESULT_TYPE,
        REGISTER_PIN_REQUEST_TYPE},
        BT_MODULE_NAME)
{}

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

void NapiBluetoothConnectionObserver::OnDiscoveryResult(
    const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass)
{
    OnDiscoveryResultCallBack(device);
    OnDiscoveryResultCallBack(device, rssi, deviceName, deviceClass);
}

void NapiBluetoothConnectionObserver::OnPairRequested(const BluetoothRemoteDevice &device)
{
    HILOGD("start");
    BluetoothRemoteDevice remoteDevice;
    if (device.GetTransportType() == BT_TRANSPORT_BREDR) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BREDR);
    } else if (device.GetTransportType() == BT_TRANSPORT_BLE) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BLE);
    }
    remoteDevice.PairRequestReply(true);
}

void NapiBluetoothConnectionObserver::OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number)
{
    HILOGD("OnPairConfirmed");
    std::shared_ptr<PairConfirmedCallBackInfo> pairConfirmInfo =
        std::make_shared<PairConfirmedCallBackInfo>(number, reqType, device.GetDeviceAddr());
    OnPairConfirmedCallBack(pairConfirmInfo);
}

void NapiBluetoothConnectionObserver::OnScanModeChanged(int mode)
{
    HILOGI("mode is %{public}d", mode);
}

void NapiBluetoothConnectionObserver::OnDeviceNameChanged(const std::string &deviceName)
{}

void NapiBluetoothConnectionObserver::OnDeviceAddrChanged(const std::string &address)
{
    HILOGI("address is %{public}s", GetEncryptAddr(address).c_str());
}

void NapiBluetoothConnectionObserver::OnPairConfirmedCallBack(
    const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo)
{
    CHECK_AND_RETURN_LOG(pairConfirmInfo, "pairConfirmInfo is nullptr");
    HILOGI("Addr: %{public}s", GetEncryptAddr(pairConfirmInfo->deviceAddr).c_str());

    auto nativeObject = std::make_shared<NapiNativePinRequiredParam>(pairConfirmInfo);
    eventSubscribe_.PublishEvent(REGISTER_PIN_REQUEST_TYPE, nativeObject);
}

void NapiBluetoothConnectionObserver::OnDiscoveryResultCallBack(const BluetoothRemoteDevice &device)
{
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    auto nativeObject = std::make_shared<NapiNativeDiscoveryResultArray>(remoteDevice);
    eventSubscribe_.PublishEvent(REGISTER_DEVICE_FIND_TYPE, nativeObject);
}

void NapiBluetoothConnectionObserver::OnDiscoveryResultCallBack(
    const BluetoothRemoteDevice &device, int rssi, const std::string &deviceName, int deviceClass)
{
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    auto nativeObject =
        std::make_shared<NapiNativeDiscoveryInfoResultArray>(remoteDevice, rssi, deviceName, deviceClass);
    eventSubscribe_.PublishEvent(REGISTER_DISCOVERY_RESULT_TYPE, nativeObject);
}
}  // namespace Bluetooth
}  // namespace OHOS
