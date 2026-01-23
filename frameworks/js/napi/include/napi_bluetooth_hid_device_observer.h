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

#ifndef NAPI_BLUETOOTH_HID_DEVICE_OBSERVER_H
#define NAPI_BLUETOOTH_HID_DEVICE_OBSERVER_H

#include "bluetooth_hid_device.h"
#include "napi_bluetooth_utils.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
const char * const STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE = "appStatusChange";
const char * const STR_BT_HID_DEVICE_CONNECT_STATE_CHANGE = "connectionStateChange";
const char * const STR_BT_HID_DEVICE_OBSERVER_APP_GET_REPORT = "getReport";
const char * const STR_BT_HID_DEVICE_OBSERVER_APP_SET_REPORT = "setReport";
const char * const STR_BT_HID_DEVICE_OBSERVER_INTERRUPT_DATA_RECEIVED = "interruptDataReceived";
const char * const STR_BT_HID_DEVICE_OBSERVER_SET_PROTOCOL = "setProtocol";
const char * const STR_BT_HID_DEVICE_OBSERVER_VIRTUAL_CABLE_UNPLUG = "virtualCableUnplug";

class NapiBluetoothHidDeviceObserver : public HidDeviceObserver {
public:
    NapiBluetoothHidDeviceObserver();
    ~NapiBluetoothHidDeviceObserver() override = default;

    void OnAppStatusChanged(int state) override;
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state) override;
    void OnGetReport(int type, int id, uint16_t bufferSize) override;
    void OnSetProtocol(int protocol) override;
    void OnInterruptDataReceived(int id, const std::vector<uint8_t> &data) override;
    void OnSetReport(int type, int id, const std::vector<uint8_t> &data) override;
    void OnVirtualCableUnplug() override;

    NapiEventSubscribeModule eventSubscribe_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_HID_DEVICE_OBSERVER_H