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

#ifndef NAPI_BLUETOOTH_BASE_PROFILE_H_
#define NAPI_BLUETOOTH_BASE_PROFILE_H_

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {
class NapiBaseProfile {
public:
    static napi_value DefineBaseProfileJSFunction(napi_env env, napi_value exports);
private:
    static napi_value BaseProfilePropertyValueInit(napi_env env, napi_value exports);
    static napi_value ConnectionStrategyInit(napi_env env);
    static napi_value DisconnectCauseInit(napi_env env);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_BASE_PROFILE_H_ */
