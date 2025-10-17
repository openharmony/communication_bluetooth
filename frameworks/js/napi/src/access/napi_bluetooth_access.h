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

#ifndef NAPI_BLUETOOTH_ACCESS_H_
#define NAPI_BLUETOOTH_ACCESS_H_

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {

constexpr int STR_LEN_OF_2_BYTES_UUID = 4; // e.g. 180f
constexpr int STR_LEN_OF_4_BYTES_UUID = 8; // e.g. 0000180f
constexpr int STR_LEN_OF_16_BYTES_UUID = 36; // e.g. 0000180f-0000-1000-8000-00805f9b34fb

class NapiAccess {
public:
    static napi_value DefineAccessJSFunction(napi_env env, napi_value exports);
    static napi_value EnableBluetooth(napi_env env, napi_callback_info info);
    static napi_value DisableBluetooth(napi_env env, napi_callback_info info);
    static napi_value RestrictBluetooth(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value RegisterAccessObserver(napi_env env, napi_callback_info info);
    static napi_value DeregisterAccessObserver(napi_env env, napi_callback_info info);
    static napi_value EnableBluetoothAsync(napi_env env, napi_callback_info info);
    static napi_value DisableBluetoothAsync(napi_env env, napi_callback_info info);
    static napi_value NotifyDialogResult(napi_env env, napi_callback_info info);
#ifdef BLUETOOTH_API_SINCE_10
    static napi_value FactoryReset(napi_env env, napi_callback_info info);
    static napi_value GetLocalAddress(napi_env env, napi_callback_info info);
    static napi_value ConvertUuid(napi_env env, napi_callback_info info);
#endif

    static napi_value AddPersistentDeviceId(napi_env env, napi_callback_info info);
    static napi_value DeletePersistentDeviceId(napi_env env, napi_callback_info info);
    static napi_value GetPersistentDeviceIds(napi_env env, napi_callback_info info);
    static napi_value isValidRandomDeviceId(napi_env env, napi_callback_info info);
    struct NotifyDialogResultParams {
        uint32_t dialogType;
        bool dialogResult;
    };
private:
    static napi_value AccessPropertyValueInit(napi_env env, napi_value exports);
    static napi_value StateChangeInit(napi_env env);
    static napi_value DialogTypeInit(napi_env env);
    static void RegisterAccessObserverToHost();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_ACCESS_H_ */
