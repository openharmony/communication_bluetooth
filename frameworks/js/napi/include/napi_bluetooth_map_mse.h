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

#ifndef NAPI_BLUETOOTH_MAP_MSE_H_
#define NAPI_BLUETOOTH_MAP_MSE_H_

#include <vector>
#include "bluetooth_map_mse.h"
#include "napi_bluetooth_map_mse_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiMapMse {
public:
    static void DefineMapMseJSClass(napi_env env, napi_value exports);
    static napi_value MapMseConstructor(napi_env env, napi_callback_info info);
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static napi_value CreateMapMseProfile(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);

    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value SetMessageAccessAuthorization(napi_env env, napi_callback_info info);
    static napi_value GetMessageAccessAuthorization(napi_env env, napi_callback_info info);

    static std::shared_ptr<NapiMapMseObserver> observer_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_MAP_MSE_H_ */