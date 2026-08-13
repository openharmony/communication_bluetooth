/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_HEARINGAID_H
#define NAPI_BLUETOOTH_HEARINGAID_H

#include <string>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_hearingaid_observer.h"

namespace OHOS {
namespace Bluetooth {

enum DeviceSide {
    DEVICE_SIDE_LEFT = 0,
    DEVICE_SIDE_RIGHT = 1
};

enum DeviceMode {
    DEVICE_MODE_MONAURAL = 0,
    DEVICE_MODE_BINAURAL = 1
};

enum HearingAidConnectionState {
    CONNECTION_STATE_DISCONNECTED = 0,
    CONNECTION_STATE_CONNECTING = 1,
    CONNECTION_STATE_CONNECTED = 2,
    CONNECTION_STATE_DISCONNECTING = 3
};

enum HearingAidConnectionStrategy {
    CONNECTION_STRATEGY_UNSUPPORTED = 0,
    CONNECTION_STRATEGY_ALLOWED = 1,
    CONNECTION_STRATEGY_FORBIDDEN = 2
};

struct HearingAidDeviceInfo {
    DeviceSide side;
    DeviceMode mode;
    int64_t id;
};

struct HearingAidStateChangeParam {
    std::string address;
    HearingAidConnectionState state;
};

class NapiHearingAid {
public:
    static void DefineHearingAidJSClass(napi_env env, napi_value exports);
    static napi_value HearingAidConstructor(napi_env env, napi_callback_info info);
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static napi_value CreateHearingAidSrcProfile(napi_env env, napi_callback_info info);
    static napi_value PropertyValueInit(napi_env env, napi_value exports);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    static napi_value IsLocalDeviceSupportHearingAid(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value GetHearingAidDeviceInfo(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);

    static std::shared_ptr<NapiHearingAidObserver> observer_;
private:
    static napi_status CheckVolume(napi_env env, napi_callback_info info, std::string& addr, int64_t& volume);
    static napi_status CheckSetConnectionStrategy(napi_env env, napi_callback_info info,
        std::string& addr, int32_t& strategy);
    static void ConvertDeviceInfoToJs(napi_env env, napi_value &object, int32_t side, int32_t mode, int64_t id);
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif