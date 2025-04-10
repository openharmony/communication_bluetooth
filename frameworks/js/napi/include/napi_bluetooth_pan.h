/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#ifndef NAPI_BLUETOOTH_PAN_H_
#define NAPI_BLUETOOTH_PAN_H_

#include "bluetooth_pan.h"
#include "napi_bluetooth_pan_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiBluetoothPan {
public:
    static void DefinePanJSClass(napi_env env, napi_value exports);
    static napi_value PanConstructor(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    static napi_value GetConnectionDevices(napi_env env, napi_callback_info info);
    static napi_value GetDeviceState(napi_env env, napi_callback_info info);

    static napi_value Disconnect(napi_env env, napi_callback_info info);

    static napi_value SetTethering(napi_env env, napi_callback_info info);
    static napi_value IsTetheringOn(napi_env env, napi_callback_info info);
    static napi_value CreatePanProfile(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static thread_local napi_ref consRef_;

    static std::shared_ptr<NapiBluetoothPanObserver> observer_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_PAN_H_