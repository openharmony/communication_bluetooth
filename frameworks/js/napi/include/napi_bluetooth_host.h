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

#ifndef NAPI_BLUETOOTH_HOST_H_
#define NAPI_BLUETOOTH_HOST_H_

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {
napi_value BluetoothHostInit(napi_env env, napi_value exports);
napi_value RegisterHostObserver(napi_env env, napi_callback_info info);
napi_value DeregisterHostObserver(napi_env env, napi_callback_info info);
napi_value PropertyValueInit(napi_env env, napi_value exports);
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_H_ */
