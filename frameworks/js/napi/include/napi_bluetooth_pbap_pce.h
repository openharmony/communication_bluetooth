/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_PBAP_PCE_H_
#define NAPI_BLUETOOTH_PBAP_PCE_H_

#include <vector>
#include "bluetooth_pbap_pce.h"
#include "napi_bluetooth_pbap_pce_observer.h"

namespace OHOS {
namespace Bluetooth {
class NapiPbapClient {
public:
    static void DefinePbapClientJSClass(napi_env env, napi_value exports);
    static napi_value PbapClientConstructor(napi_env env, napi_callback_info info);
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static napi_value CreatePbapClientProfile(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value OnSyncStateChange(napi_env env, napi_callback_info info);
    static napi_value OffSyncStateChange(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value GetSyncState(napi_env env, napi_callback_info info);
    static napi_value OnConnectionStateChange(napi_env env, napi_callback_info info);
    static napi_value OffConnectionStateChange(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);

    static std::shared_ptr<NapiPbapPceObserver> observer_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_PBAP_PCE_H_ */