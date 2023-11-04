/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_A2DP_SRC_H_
#define NAPI_BLUETOOTH_A2DP_SRC_H_

#include <vector>
#include "bluetooth_a2dp_src.h"
#include "napi_bluetooth_a2dp_src_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiA2dpSource {
public:
    static napi_value DefineA2dpSourceJSClass(napi_env env, napi_value exports);
    static napi_value A2dpSourceConstructor(napi_env env, napi_callback_info info);
    static napi_value A2dpPropertyValueInit(napi_env env, napi_value exports);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetPlayingState(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value GetConnectionDevices(napi_env env, napi_callback_info info);
    static napi_value GetDeviceState(napi_env env, napi_callback_info info);
#ifdef BLUETOOTH_API_SINCE_10
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static napi_value CreateA2dpSrcProfile(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value IsAbsoluteVolumeSupport(napi_env env, napi_callback_info info);
    static napi_value IsAbsoluteVolumeEnabled(napi_env env, napi_callback_info info);
    static napi_value EnableAbsoluteVolume(napi_env env, napi_callback_info info);
    static napi_value DisableAbsoluteVolume(napi_env env, napi_callback_info info);
    static napi_value SetCurrentCodecInfo(napi_env env, napi_callback_info info);
    static napi_value GetCurrentCodecInfo(napi_env env, napi_callback_info info);
#endif
    static NapiA2dpSourceObserver observer_;
    static bool isRegistered_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_A2DP_SRC_H_ */