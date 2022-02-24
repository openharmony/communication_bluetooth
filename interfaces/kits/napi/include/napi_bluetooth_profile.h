/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_PROFILE_H_
#define NAPI_BLUETOOTH_PROFILE_H_

#include <map>
#include <memory>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {
enum ProfileCode {
    CODE_BT_PROFILE_A2DP_SINK = 0,
    CODE_BT_PROFILE_A2DP_SOURCE,
    CODE_BT_PROFILE_AVRCP_CT,
    CODE_BT_PROFILE_AVRCP_TG,
    CODE_BT_PROFILE_HANDS_FREE_AUDIO_GATEWAY,
    CODE_BT_PROFILE_HANDS_FREE_UNIT,
    CODE_BT_PROFILE_HID_HOST,
    CODE_BT_PROFILE_PAN_NETWORK,
    CODE_BT_PROFILE_PBAP_CLIENT,
    CODE_BT_PROFILE_PBAP_SERVER,
};

void DefineProfileFunctions(napi_env env, napi_value exports);

class NapiProfile {
public:
    static std::map<ProfileCode, napi_value> profiles_;
    static void SetProfile(ProfileCode code, napi_value profile);
    static napi_value GetProfile(napi_env env, napi_callback_info info);
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_PROFILE_H_ */

