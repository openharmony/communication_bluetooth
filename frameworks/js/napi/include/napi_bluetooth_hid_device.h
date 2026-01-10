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
#ifndef NAPI_BLUETOOTH_HID_DEVICE_H_
#define NAPI_BLUETOOTH_HID_DEVICE_H_

#include "bluetooth_hid_device.h"
#include "napi_bluetooth_hid_device_observer.h"

namespace OHOS {
namespace Bluetooth {

class NapiBluetoothHidDevice {
public:
    static void DefineHidDeviceJSClass(napi_env env, napi_value exports);
    static napi_value HidDeviceConstructor(napi_env env, napi_callback_info info);

    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);

    static napi_value RegisterHidDevice(napi_env env, napi_callback_info info);
    static napi_value UnregisterHidDevice(napi_env env, napi_callback_info info);

    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value SendReport(napi_env env, napi_callback_info info);
    static napi_value ReplyReport(napi_env env, napi_callback_info info);
    static napi_value ReportError(napi_env env, napi_callback_info info);

    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value OnGetReport(napi_env env, napi_callback_info info);
    static napi_value OnSetReport(napi_env env, napi_callback_info info);
    static napi_value OnInterruptDataReceived(napi_env env, napi_callback_info info);
    static napi_value OnSetProtocol(napi_env env, napi_callback_info info);
    static napi_value OnVirtualCableUnplug(napi_env env, napi_callback_info info);
    static napi_value OnConnectionStateChange(napi_env env, napi_callback_info info);

    static napi_value OffGetReport(napi_env env, napi_callback_info info);
    static napi_value OffSetReport(napi_env env, napi_callback_info info);
    static napi_value OffInterruptDataReceived(napi_env env, napi_callback_info info);
    static napi_value OffSetProtocol(napi_env env, napi_callback_info info);
    static napi_value OffVirtualCableUnplug(napi_env env, napi_callback_info info);
    static napi_value OffConnectionStateChange(napi_env env, napi_callback_info info);

    static napi_value CreateHidDeviceProfile(napi_env env, napi_callback_info info);
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);

    static std::shared_ptr<NapiBluetoothHidDeviceObserver> observer_;
    static thread_local napi_ref consRef_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // NAPI_BLUETOOTH_HID_DEVICE_H_