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
class NapiAccess {
public:
    static napi_value DefineAccessJSFunction(napi_env env, napi_value exports);
    static napi_value EnableBluetooth(napi_env env, napi_callback_info info);
    static napi_value DisableBluetooth(napi_env env, napi_callback_info info);
    static napi_value RestrictBluetooth(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value RegisterAccessObserver(napi_env env, napi_callback_info info);
    static napi_value DeregisterAccessObserver(napi_env env, napi_callback_info info);

#ifdef BLUETOOTH_API_SINCE_10
    static napi_value FactoryReset(napi_env env, napi_callback_info info);
    static napi_value GetLocalAddress(napi_env env, napi_callback_info info);
#endif
private:
    static napi_value AccessPropertyValueInit(napi_env env, napi_value exports);
    static napi_value StateChangeInit(napi_env env);
    static void RegisterAccessObserverToHost();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_ACCESS_H_ */
