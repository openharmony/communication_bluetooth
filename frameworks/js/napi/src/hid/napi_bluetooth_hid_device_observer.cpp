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
#define LOG_TAG "bt_napi_hid_device_observer"
#endif

#include "napi_bluetooth_hid_device_observer.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_async_callback.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include <uv.h>

namespace OHOS {
namespace Bluetooth {
NapiBluetoothHidDeviceObserver::NapiBluetoothHidDeviceObserver()
    : eventSubscribe_({STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE,
        STR_BT_HID_DEVICE_CONNECT_STATE_CHANGE,
        STR_BT_HID_DEVICE_OBSERVER_APP_GET_REPORT,
        STR_BT_HID_DEVICE_OBSERVER_APP_SET_REPORT,
        STR_BT_HID_DEVICE_OBSERVER_INTERRUPT_DATA_RECEIVED,
        STR_BT_HID_DEVICE_OBSERVER_SET_PROTOCOL,
        STR_BT_HID_DEVICE_OBSERVER_VIRTUAL_CABLE_UNPLUG},
        BT_MODULE_NAME)
{}

void NapiBluetoothHidDeviceObserver::OnAppStatusChanged(int state)
{
    auto nativeObject = std::make_shared<NapiNativeBool>(state);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE, nativeObject);
    if (state == HID_DEVICE_UNREGISTERED) {
        eventSubscribe_.DeregisterAllCallback(STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE);
    }
}

void NapiBluetoothHidDeviceObserver::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
{
    auto nativeObject = std::make_shared<NapiNativeStateChangeParam>(device.GetDeviceAddr(), state);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_CONNECT_STATE_CHANGE, nativeObject);
}

void NapiBluetoothHidDeviceObserver::OnGetReport(int type, int id, uint16_t bufferSize)
{
    auto nativeObject = std::make_shared<NapiNativeHIDGetReportData>(type, id, bufferSize);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_APP_GET_REPORT, nativeObject);
}

void NapiBluetoothHidDeviceObserver::OnSetProtocol(int protocol)
{
    auto nativeObject = std::make_shared<NapiNativeHIDProtocolData>(protocol);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_SET_PROTOCOL, nativeObject);
}

void NapiBluetoothHidDeviceObserver::OnInterruptDataReceived(int id, const std::vector<uint8_t> &data)
{
    auto nativeObject = std::make_shared<NapiNativeHIDInterruptData>(id, data);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_INTERRUPT_DATA_RECEIVED, nativeObject);
}

void NapiBluetoothHidDeviceObserver::OnSetReport(int type, int id, const std::vector<uint8_t> &data)
{
    auto nativeObject = std::make_shared<NapiNativeHIDSetReportData>(type, id, data);
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_APP_SET_REPORT, nativeObject);
}

void NapiBluetoothHidDeviceObserver::OnVirtualCableUnplug()
{
    auto nativeObject = std::make_shared<NapiNativeEmpty>();
    eventSubscribe_.PublishEvent(STR_BT_HID_DEVICE_OBSERVER_VIRTUAL_CABLE_UNPLUG, nativeObject);
}
} // namespace Bluetooth
} // namespace OHOS