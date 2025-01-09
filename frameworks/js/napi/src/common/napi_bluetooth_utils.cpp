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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_utils"
#endif

#include "napi_bluetooth_utils.h"
#include <algorithm>
#include <functional>
#include <optional>
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_spp_client.h"
#include "../parser/napi_parser_utils.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    HILOGE("errCode: %{public}d", errCode);
    napi_value result = NapiGetNull(env);
    napi_value eCode = NapiGetNull(env);
    if (errCode == BT_NO_ERROR) {
        return result;
    }
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));

    std::string errMsg = GetNapiErrMsg(env, errCode);
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);
    napi_set_named_property(env, result, "message", message);
    return result;
}

std::shared_ptr<BluetoothCallbackInfo> GetCallbackInfoByType(const std::string &type)
{
    std::lock_guard<std::mutex> lock(g_observerMutex);
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[type]) {
        return nullptr;
    }
    return observers[type];
}

bool ParseString(napi_env env, string &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    if (valuetype != napi_string) {
        HILOGE("Wrong argument type(%{public}d). String expected.", valuetype);
        return false;
    }
    size_t size = 0;

    if (napi_get_value_string_utf8(env, args, nullptr, 0, &size) != napi_ok) {
        HILOGE("can not get string size");
        param = "";
        return false;
    }
    param.reserve(size + 1);
    param.resize(size);
    if (napi_get_value_string_utf8(env, args, param.data(), (size + 1), &size) != napi_ok) {
        HILOGE("can not get string value");
        param = "";
        return false;
    }
    return true;
}

bool ParseInt32(napi_env env, int32_t &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);
    if (valuetype != napi_number) {
        HILOGE("Wrong argument type(%{public}d). Int32 expected.", valuetype);
        return false;
    }
    napi_get_value_int32(env, args, &param);
    return true;
}

bool ParseBool(napi_env env, bool &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    if (valuetype != napi_boolean) {
        HILOGE("Wrong argument type(%{public}d). bool expected.", valuetype);
        return false;
    }
    napi_get_value_bool(env, args, &param);
    return true;
}


bool ParseArrayBuffer(napi_env env, uint8_t** data, size_t &size, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    if (valuetype != napi_object) {
        HILOGE("Wrong argument type(%{public}d). object expected.", valuetype);
        return false;
    }

    status = napi_get_arraybuffer_info(env, args, reinterpret_cast<void**>(data), &size);
    if (status != napi_ok) {
        HILOGE("can not get arraybuffer, error is %{public}d", status);
        return false;
    }
    HILOGI("arraybuffer size is %{public}zu", size);
    return true;
}

napi_status ConvertOppTransferInformationToJS(napi_env env, napi_value result,
    const BluetoothOppTransferInformation& transferInformation)
{
    HILOGI("ConvertOppTransferInformationToJS called");
    napi_value id;
    napi_create_int32(env, transferInformation.GetId(), &id);
    napi_set_named_property(env, result, "id", id);

    napi_value fileName;
    napi_create_string_utf8(env, transferInformation.GetFileName().c_str(), NAPI_AUTO_LENGTH, &fileName);
    napi_set_named_property(env, result, "fileName", fileName);

    napi_value filePath;
    napi_create_string_utf8(env, transferInformation.GetFilePath().c_str(), NAPI_AUTO_LENGTH, &filePath);
    napi_set_named_property(env, result, "filePath", filePath);

    napi_value mimeType;
    napi_create_string_utf8(env, transferInformation.GetMimeType().c_str(), NAPI_AUTO_LENGTH, &mimeType);
    napi_set_named_property(env, result, "mimeType", mimeType);

    napi_value deviceName;
    napi_create_string_utf8(env, transferInformation.GetDeviceName().c_str(), NAPI_AUTO_LENGTH, &deviceName);
    napi_set_named_property(env, result, "deviceName", deviceName);

    napi_value deviceAddress;
    napi_create_string_utf8(env, transferInformation.GetDeviceAddress().c_str(), NAPI_AUTO_LENGTH, &deviceAddress);
    napi_set_named_property(env, result, "deviceAddress", deviceAddress);

    napi_value direction;
    napi_create_int32(env, transferInformation.GetDirection(), &direction);
    napi_set_named_property(env, result, "direction", direction);

    napi_value status;
    napi_create_int32(env, transferInformation.GetStatus(), &status);
    napi_set_named_property(env, result, "status", status);

    napi_value failedReason;
    napi_create_int32(env, transferInformation.GetFailedReason(), &failedReason);
    napi_set_named_property(env, result, "failedReason", failedReason);

    napi_value timeStamp;
    napi_create_int64(env, transferInformation.GetTimeStamp(), &timeStamp);
    napi_set_named_property(env, result, "timeStamp", timeStamp);

    napi_value currentBytes;
    napi_create_int64(env, transferInformation.GetCurrentBytes(), &currentBytes);
    napi_set_named_property(env, result, "currentBytes", currentBytes);

    napi_value totalBytes;
    napi_create_int64(env, transferInformation.GetTotalBytes(), &totalBytes);
    napi_set_named_property(env, result, "totalBytes", totalBytes);

    return napi_ok;
}

napi_status ConvertStringVectorToJS(napi_env env, napi_value result, std::vector<std::string>& stringVector)
{
    HILOGI("vector size: %{public}zu", stringVector.size());
    size_t idx = 0;

    if (stringVector.empty()) {
        return napi_ok;
    }

    for (auto& str : stringVector) {
        napi_value obj = nullptr;
        NAPI_BT_CALL_RETURN(napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &obj));
        NAPI_BT_CALL_RETURN(napi_set_element(env, result, idx, obj));
        idx++;
    }
    return napi_ok;
}

void ConvertStateChangeParamToJS(napi_env env, napi_value result, const std::string &device, int state, int cause)
{
    napi_value deviceId = nullptr;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value profileState = nullptr;
    napi_create_int32(env, GetProfileConnectionState(state), &profileState);
    napi_set_named_property(env, result, "state", profileState);

    napi_value stateChangeCause = nullptr;
    napi_create_int32(env, cause, &stateChangeCause);
    napi_set_named_property(env, result, "cause", stateChangeCause);
}

void ConvertScoStateChangeParamToJS(napi_env env, napi_value result, const std::string &device, int state)
{
    napi_value deviceId = nullptr;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value profileState = nullptr;
    napi_create_int32(env, GetScoConnectionState(state), &profileState);
    napi_set_named_property(env, result, "state", profileState);
}

void ConvertUuidsVectorToJS(napi_env env, napi_value result, const std::vector<std::string> &uuids)
{
    HILOGD("enter");
    size_t idx = 0;

    if (uuids.empty()) {
        return;
    }
    HILOGI("size: %{public}zu", uuids.size());
    for (auto& uuid : uuids) {
        napi_value uuidValue = nullptr;
        napi_create_string_utf8(env, uuid.c_str(), NAPI_AUTO_LENGTH, &uuidValue);
        napi_set_element(env, result, idx, uuidValue);
        idx++;
    }
}

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

void SetNamedPropertyByString(napi_env env, napi_value dstObj, const std::string &strValue, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, strValue.c_str(), NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value NapiGetBooleanFalse(napi_env env)
{
    napi_value result = nullptr;
    napi_get_boolean(env, false, &result);
    return result;
}

napi_value NapiGetBooleanTrue(napi_env env)
{
    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value NapiGetBooleanRet(napi_env env, bool ret)
{
    napi_value result = nullptr;
    napi_get_boolean(env, ret, &result);
    return result;
}

napi_value NapiGetUndefinedRet(napi_env env)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

napi_value NapiGetInt32Ret(napi_env env, int32_t res)
{
    napi_value ret = nullptr;
    napi_create_int32(env, res, &ret);
    return ret;
}

std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> GetObserver()
{
    return g_Observer;
}

const sysBLEMap &GetSysBLEObserver()
{
    return g_sysBLEObserver;
}

int GetProfileConnectionState(int state)
{
    int32_t profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
    switch (state) {
        case static_cast<int32_t>(BTConnectState::CONNECTING):
            HILOGD("STATE_CONNECTING(1)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::CONNECTED):
            HILOGD("STATE_CONNECTED(2)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTED;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTING):
            HILOGD("STATE_DISCONNECTING(3)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTED):
            HILOGD("STATE_DISCONNECTED(0)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    return profileConnectionState;
}

uint32_t GetProfileId(int profile)
{
    uint32_t profileId = 0;
    switch (profile) {
        case static_cast<int32_t>(ProfileId::PROFILE_A2DP_SINK):
            HILOGD("PROFILE_ID_A2DP_SINK");
            profileId = PROFILE_ID_A2DP_SINK;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_A2DP_SOURCE):
            HILOGD("PROFILE_ID_A2DP_SRC");
            profileId = PROFILE_ID_A2DP_SRC;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_AVRCP_CT):
            HILOGD("PROFILE_ID_AVRCP_CT");
            profileId = PROFILE_ID_AVRCP_CT;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_AVRCP_TG):
            HILOGD("PROFILE_ID_AVRCP_TG");
            profileId = PROFILE_ID_AVRCP_TG;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_HANDS_FREE_AUDIO_GATEWAY):
            HILOGD("PROFILE_ID_HFP_AG");
            profileId = PROFILE_ID_HFP_AG;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_HANDS_FREE_UNIT):
            HILOGD("PROFILE_ID_HFP_HF");
            profileId = PROFILE_ID_HFP_HF;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_PBAP_CLIENT):
            HILOGD("PROFILE_ID_PBAP_PCE");
            profileId = PROFILE_ID_PBAP_PCE;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_PBAP_SERVER):
            HILOGD("PROFILE_ID_PBAP_PSE");
            profileId = PROFILE_ID_PBAP_PSE;
            break;
        case static_cast<int32_t>(ProfileId::PROFILE_HID_HOST):
            HILOGD("PROFILE_HID_HOST");
            profileId = PROFILE_ID_HID_HOST;
            break;
        default:
            break;
    }
    return profileId;
}

int GetScoConnectionState(int state)
{
    int32_t scoState = ScoState::SCO_DISCONNECTED;
    switch (state) {
        case static_cast<int32_t>(HfpScoConnectState::SCO_CONNECTING):
            HILOGD("SCO_CONNECTING(1)");
            scoState = ScoState::SCO_CONNECTING;
            break;
        case static_cast<int32_t>(HfpScoConnectState::SCO_CONNECTED):
            HILOGD("SCO_CONNECTED(3)");
            scoState = ScoState::SCO_CONNECTED;
            break;
        case static_cast<int32_t>(HfpScoConnectState::SCO_DISCONNECTING):
            HILOGD("SCO_DISCONNECTING(2)");
            scoState = ScoState::SCO_DISCONNECTING;
            break;
        case static_cast<int32_t>(HfpScoConnectState::SCO_DISCONNECTED):
            HILOGD("SCO_DISCONNECTED(0)");
            scoState = ScoState::SCO_DISCONNECTED;
            break;
        default:
            break;
    }
    return scoState;
}

void RegisterSysBLEObserver(
    const std::shared_ptr<BluetoothCallbackInfo> &info, int32_t callbackIndex, const std::string &type)
{
    if (callbackIndex >= static_cast<int32_t>(ARGS_SIZE_THREE)) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_sysBLEObserverMutex);
    HILOGI("type: %{public}s, index: %{public}d", type.c_str(), callbackIndex);
    g_sysBLEObserver[type][callbackIndex] = info;
}

void UnregisterSysBLEObserver(const std::string &type)
{
    std::lock_guard<std::mutex> lock(g_sysBLEObserverMutex);
    auto itor = g_sysBLEObserver.find(type);
    if (itor != g_sysBLEObserver.end()) {
        g_sysBLEObserver.erase(itor);
    }
}

int DoInJsMainThread(napi_env env, std::function<void(void)> func)
{
    if (napi_send_event(env, func, napi_eprio_high) != napi_ok) {
        HILOGE("Failed to SendEvent");
        return -1;
    }
    return 0;
}

bool IsValidAddress(std::string bdaddr)
{
#if defined(IOS_PLATFORM)
    const std::regex deviceIdRegex("^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$");
    return regex_match(bdaddr, deviceIdRegex);
#else
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(bdaddr, deviceIdRegex);
#endif
}

bool IsValidTransport(int transport)
{
    return transport == BT_TRANSPORT_BREDR || transport == BT_TRANSPORT_BLE;
}

bool IsValidConnectStrategy(int strategy)
{
    return strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)
        || strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
}

napi_status NapiIsBoolean(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    NAPI_BT_RETURN_IF(valuetype != napi_boolean, "Wrong argument type. Boolean expected.", napi_boolean_expected);
    return napi_ok;
}

napi_status NapiIsNumber(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    NAPI_BT_RETURN_IF(valuetype != napi_number, "Wrong argument type. Number expected.", napi_number_expected);
    return napi_ok;
}

napi_status NapiIsString(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    NAPI_BT_RETURN_IF(valuetype != napi_string, "Wrong argument type. String expected.", napi_string_expected);
    return napi_ok;
}

napi_status NapiIsFunction(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    if (valuetype != napi_function) {
        HILOGD("Wrong argument type. Function expected.");
        return napi_function_expected;
    }
    NAPI_BT_RETURN_IF(valuetype != napi_function, "Wrong argument type. Function expected.", napi_function_expected);
    return napi_ok;
}

napi_status NapiIsArrayBuffer(napi_env env, napi_value value)
{
    bool isArrayBuffer = false;
    NAPI_BT_CALL_RETURN(napi_is_arraybuffer(env, value, &isArrayBuffer));
    NAPI_BT_RETURN_IF(!isArrayBuffer, "Expected arraybuffer type", napi_arraybuffer_expected);
    return napi_ok;
}

napi_status NapiIsArray(napi_env env, napi_value value)
{
    bool isArray = false;
    NAPI_BT_CALL_RETURN(napi_is_array(env, value, &isArray));
    NAPI_BT_RETURN_IF(!isArray, "Expected array type", napi_array_expected);
    return napi_ok;
}

napi_status NapiIsObject(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    NAPI_BT_RETURN_IF(valuetype != napi_object, "Wrong argument type. Object expected.", napi_object_expected);
    return napi_ok;
}

napi_status ParseNumberParams(napi_env env, napi_value object, const char *name, bool &outExist,
    napi_value &outParam)
{
    bool hasProperty = false;
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &hasProperty));
    if (hasProperty) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        napi_valuetype valuetype;
        NAPI_BT_CALL_RETURN(napi_typeof(env, property, &valuetype));
        NAPI_BT_RETURN_IF(valuetype != napi_number, "Wrong argument type, number expected", napi_number_expected);
        outParam = property;
    }
    outExist = hasProperty;
    return napi_ok;
}

napi_status ParseInt32Params(napi_env env, napi_value object, const char *name, bool &outExist,
    int32_t &outParam)
{
    bool exist = false;
    napi_value param;
    NAPI_BT_CALL_RETURN(ParseNumberParams(env, object, name, exist, param));
    if (exist) {
        int32_t num = 0;
        NAPI_BT_CALL_RETURN(napi_get_value_int32(env, param, &num));
        outParam = num;
    }
    outExist = exist;
    return napi_ok;
}

napi_status ParseUint32Params(napi_env env, napi_value object, const char *name, bool &outExist,
    uint32_t &outParam)
{
    bool exist = false;
    napi_value param;
    NAPI_BT_CALL_RETURN(ParseNumberParams(env, object, name, exist, param));
    if (exist) {
        uint32_t num = 0;
        NAPI_BT_CALL_RETURN(napi_get_value_uint32(env, param, &num));
        outParam = num;
    }
    outExist = exist;
    return napi_ok;
}

napi_status ParseBooleanParams(napi_env env, napi_value object, const char *name, bool &outExist, bool &outParam)
{
    bool hasProperty = false;
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &hasProperty));
    if (hasProperty) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        napi_valuetype valuetype;
        NAPI_BT_CALL_RETURN(napi_typeof(env, property, &valuetype));
        NAPI_BT_RETURN_IF(valuetype != napi_boolean, "Wrong argument type, boolean expected", napi_boolean_expected);

        bool param = false;
        NAPI_BT_CALL_RETURN(napi_get_value_bool(env, property, &param));
        outParam = param;
    }
    outExist = hasProperty;
    return napi_ok;
}

// Only used for optional paramters
napi_status ParseStringParams(napi_env env, napi_value object, const char *name, bool &outExist,
    std::string &outParam)
{
    bool hasProperty = false;
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &hasProperty));
    if (hasProperty) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        napi_valuetype valuetype;
        NAPI_BT_CALL_RETURN(napi_typeof(env, property, &valuetype));
        NAPI_BT_RETURN_IF(valuetype != napi_string, "Wrong argument type, string expected", napi_string_expected);

        std::string param {};
        bool isSuccess = ParseString(env, param, property);
        if (!isSuccess) {
            return napi_invalid_arg;
        }
        outParam = std::move(param);
    }
    outExist = hasProperty;
    return napi_ok;
}

napi_status ParseArrayBufferParams(napi_env env, napi_value object, const char *name, bool &outExist,
    std::vector<uint8_t> &outParam)
{
    bool hasProperty = false;
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &hasProperty));
    if (hasProperty) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        bool isArrayBuffer = false;
        NAPI_BT_CALL_RETURN(napi_is_arraybuffer(env, property, &isArrayBuffer));
        NAPI_BT_RETURN_IF(!isArrayBuffer, "Wrong argument type, arraybuffer expected", napi_arraybuffer_expected);

        uint8_t *data = nullptr;
        size_t size = 0;
        bool isSuccess = ParseArrayBuffer(env, &data, size, property);
        if (!isSuccess) {
            HILOGE("ParseArrayBuffer faild.");
            return napi_invalid_arg;
        }
        outParam = std::vector<uint8_t>(data, data + size);
    }
    outExist = hasProperty;
    return napi_ok;
}

napi_status ParseUuidParams(napi_env env, napi_value object, const char *name, bool &outExist, UUID &outUuid)
{
    bool exist = false;
    std::string uuid {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, object, name, exist, uuid));
    if (exist) {
        if (!IsValidUuid(uuid)) {
            HILOGE("match the UUID faild.");
            return napi_invalid_arg;
        }
        outUuid = ParcelUuid::FromString(uuid);
    }
    outExist = exist;
    return napi_ok;
}

// This function applies to interfaces with a single address as a parameter.
bool CheckDeivceIdParam(napi_env env, napi_callback_info info, std::string &addr)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Wrong argument type", false);
    NAPI_BT_RETURN_IF(!ParseString(env, addr, argv[PARAM0]), "ParseString failed", false);
    NAPI_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    return true;
}

bool CheckProfileIdParam(napi_env env, napi_callback_info info, int &profileId)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Wrong argument type", false);
    NAPI_BT_RETURN_IF(!ParseInt32(env, profileId, argv[PARAM0]), "ParseInt32 failed", false);
    return true;
}

bool CheckProfileIdParamEx(napi_env env, napi_callback_info info, int &profileId, size_t &argc)
{
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc > ARGS_SIZE_ONE, "Wrong argument type", false);
    if (argc == ARGS_SIZE_ONE) {
        NAPI_BT_RETURN_IF(!ParseInt32(env, profileId, argv[PARAM0]), "ParseInt32 failed", false);
    }
    return true;
}

bool CheckSetDevicePairingConfirmationParam(napi_env env, napi_callback_info info, std::string &addr, bool &accept)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Wrong argument type", false);
    NAPI_BT_RETURN_IF(!ParseString(env, addr, argv[PARAM0]), "ParseString failed", false);
    NAPI_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    NAPI_BT_RETURN_IF(!ParseBool(env, accept, argv[PARAM1]), "ParseBool failed", false);
    return true;
}

bool CheckLocalNameParam(napi_env env, napi_callback_info info, std::string &name)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Wrong argument type", false);
    NAPI_BT_RETURN_IF(!ParseString(env, name, argv[PARAM0]), "ParseString failed", false);
    return true;
}

bool CheckSetBluetoothScanModeParam(napi_env env, napi_callback_info info, int32_t &mode, int32_t &duration)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Wrong argument type", false);
    NAPI_BT_RETURN_IF(!ParseInt32(env, mode, argv[PARAM0]), "ParseInt32 failed", false);
    NAPI_BT_RETURN_IF(!ParseInt32(env, duration, argv[PARAM1]), "ParseInt32 failed", false);
    return true;
}

napi_status CheckEmptyParam(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ZERO;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ZERO, "Requires 0 argument.", napi_invalid_arg);
    return napi_ok;
}

napi_status NapiCheckObjectPropertiesName(napi_env env, napi_value object, const std::vector<std::string> &names)
{
    uint32_t len = 0;
    napi_value properties;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(napi_get_property_names(env, object, &properties));
    NAPI_BT_CALL_RETURN(napi_get_array_length(env, properties, &len));
    for (uint32_t i = 0; i < len; ++i) {
        std::string name {};
        napi_value actualName;
        NAPI_BT_CALL_RETURN(napi_get_element(env, properties, i, &actualName));
        NAPI_BT_CALL_RETURN(NapiParseString(env, actualName, name));
        if (std::find(names.begin(), names.end(), name) == names.end()) {
            HILOGE("Unexpect object property name: \"%{public}s\"", name.c_str());
            return napi_invalid_arg;
        }
    }
    return napi_ok;
}

napi_status CheckSetConnectStrategyParam(napi_env env, napi_callback_info info, std::string &addr, int32_t &strategy)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, strategy, argv[PARAM1]), "ParseInt failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!IsValidConnectStrategy(strategy), "Invalid strategy", napi_invalid_arg);
    return napi_ok;
}

napi_status CheckDeviceAddressParam(napi_env env, napi_callback_info info, std::string &addr)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    return napi_ok;
}

bool IsAccessAuthorizationValid(int32_t accessAuthorization)
{
    return accessAuthorization == static_cast<int32_t>(AccessAuthorization::UNKNOWN) ||
        accessAuthorization == static_cast<int32_t>(AccessAuthorization::ALLOWED) ||
        accessAuthorization == static_cast<int32_t>(AccessAuthorization::REJECTED);
}

napi_status CheckAccessAuthorizationParam(napi_env env, napi_callback_info info, std::string &addr,
    int32_t &accessAuthorization)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, accessAuthorization, argv[PARAM1]), "ParseInt failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!IsAccessAuthorizationValid(accessAuthorization),
        "Invalid accessAuthorization", napi_invalid_arg);
    return napi_ok;
}

napi_status NapiGetOnOffCallbackName(napi_env env, napi_callback_info info, std::string &name)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc == ARGS_SIZE_ZERO, "Requires at least 1 arguments", napi_invalid_arg);

    std::string type {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));

    name = type;
    return napi_ok;
}
}  // namespace Bluetooth
}  // namespace OHOS
