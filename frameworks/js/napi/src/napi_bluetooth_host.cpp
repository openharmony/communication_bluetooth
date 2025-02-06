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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_host"
#endif

#include "napi_bluetooth_host.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "napi_async_work.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "parser/napi_parser_utils.h"
#include "access/napi_bluetooth_access.h"
#include "connection/napi_bluetooth_connection.h"
#include "napi_bluetooth_spp_server.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {

napi_value BluetoothHostInit(napi_env env, napi_value exports)
{
    HILOGD("start");
    PropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", RegisterHostObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterHostObserver),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "host:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

napi_value RegisterHostObserver(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string type;
    auto status = NapiParseString(env, argv[PARAM0], type);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    if (type == REGISTER_STATE_CHANGE_TYPE) {
        return NapiAccess::RegisterAccessObserver(env, info);
    } else if (type == REGISTER_DEVICE_FIND_TYPE || type == REGISTER_PIN_REQUEST_TYPE ||
               type == REGISTER_BOND_STATE_TYPE) {
        return RegisterConnectionObserver(env, info);
    } else if (type == REGISTER_SPP_READ_TYPE) {
        return NapiSppServer::RegisterSocketObserver(env, info);
    } else {
        NAPI_BT_ASSERT_RETURN_UNDEF(env, false, BT_ERR_INVALID_PARAM);
    }
}

napi_value DeregisterHostObserver(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string type;
    auto status = NapiParseString(env, argv[PARAM0], type);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    if (type == REGISTER_STATE_CHANGE_TYPE) {
        return NapiAccess::DeregisterAccessObserver(env, info);
    } else if (type == REGISTER_DEVICE_FIND_TYPE || type == REGISTER_PIN_REQUEST_TYPE ||
               type == REGISTER_BOND_STATE_TYPE) {
        return DeRegisterConnectionObserver(env, info);
    } else if (type == REGISTER_SPP_READ_TYPE) {
        return NapiSppServer::DeRegisterSocketObserver(env, info);
    } else {
        NAPI_BT_ASSERT_RETURN_UNDEF(env, false, BT_ERR_INVALID_PARAM);
    }
}

napi_value PropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("start");
    napi_value scanDutyObject = ScanDutyInit(env);
    napi_value matchModeObject = MatchModeInit(env);

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ScanDuty", scanDutyObject),
        DECLARE_NAPI_PROPERTY("MatchMode", matchModeObject),

    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "host:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}
}  // namespace Bluetooth
}  // namespace OHOS
