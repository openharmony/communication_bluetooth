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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_base_profile"
#endif

#include "napi_bluetooth_base_profile.h"

#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
napi_value NapiBaseProfile::DefineBaseProfileJSFunction(napi_env env, napi_value exports)
{
    HILOGD("start");
    BaseProfilePropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("end");
    return exports;
}

napi_value NapiBaseProfile::BaseProfilePropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("start");
    napi_value strategyObj = ConnectionStrategyInit(env);
    napi_value disconnectCauseObj = DisconnectCauseInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ConnectionStrategy", strategyObj),
        DECLARE_NAPI_PROPERTY("DisconnectCause", disconnectCauseObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "baseprofile:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    HILOGI("end");
    return exports;
}

napi_value NapiBaseProfile::ConnectionStrategyInit(napi_env env)
{
    HILOGI("enter");
    napi_value strategyObj = nullptr;
    napi_create_object(env, &strategyObj);
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_UNKNOWN,
        "CONNECTION_STRATEGY_UNSUPPORTED");
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_ALLOWED,
        "CONNECTION_STRATEGY_ALLOWED");
    SetNamedPropertyByInteger(env, strategyObj, ConnectionStrategy::CONNECTION_FORBIDDEN,
        "CONNECTION_STRATEGY_FORBIDDEN");
    return strategyObj;
}

napi_value NapiBaseProfile::DisconnectCauseInit(napi_env env)
{
    HILOGI("enter");
    napi_value disconnectCauseObj = nullptr;
    napi_create_object(env, &disconnectCauseObj);
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_USER_DISCONNECT), "USER_DISCONNECT");
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_KEYBOARD), "CONNECT_FROM_KEYBOARD");
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_MOUSE), "CONNECT_FROM_MOUSE");
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_CAR), "CONNECT_FROM_CAR");
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES), "TOO_MANY_CONNECTED_DEVICES");
    SetNamedPropertyByInteger(env, disconnectCauseObj,
        static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL), "CONNECT_FAIL_INTERNAL");
    return disconnectCauseObj;
}
}  // namespace Bluetooth
}  // namespace OHOS