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

#ifndef NAPI_BLUETOOTH_A2DP_SNK_H_
#define NAPI_BLUETOOTH_A2DP_SNK_H_

#include <vector>
#include "bluetooth_a2dp_snk.h"
#include "napi_bluetooth_a2dp_snk_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiA2dpSink {
public:

    static void DefineA2dpSinkJSClass(napi_env env);
    static napi_value A2dpSinkConstructor(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    static napi_value GetConnectionDevices(napi_env env, napi_callback_info info);
    static napi_value GetDeviceState(napi_env env, napi_callback_info info);
    static napi_value getPlayingState(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);

    static std::shared_ptr<NapiA2dpSinkObserver> observer_;
    static bool isRegistered_;
};



}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_A2DP_SNK_H_ */