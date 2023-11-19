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

#ifndef NAPI_BLUETOOTH_AUDIO_MANAGER_H
#define NAPI_BLUETOOTH_AUDIO_MANAGER_H

#include <vector>
#include "bluetooth_a2dp_snk.h"
#include "napi_bluetooth_a2dp_snk_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiBluetoothAudioManager {
public:
    static void DefineSystemWearDetectionInterface(napi_env env, napi_value exports);
    static napi_value EnableWearDetection(napi_env env, napi_callback_info info);
    static napi_value DisableWearDetection(napi_env env, napi_callback_info info);
    static napi_value IsWearDetectionEnabled(napi_env env, napi_callback_info info);

    static NapiA2dpSinkObserver observer_;
    static bool isRegistered_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_AUDIO_MANAGER_H */
