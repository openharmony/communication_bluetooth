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
#include "parser/napi_parser_utils.h"
#include "securec.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include <iomanip>
#include <set>
#include <sstream>

namespace OHOS {
namespace Bluetooth {
using namespace std;

constexpr int OOB_C_SIZE = 16; // size of confirmationHash of OobData
constexpr int OOB_R_SIZE = 16; // size of randomizerHash of OobData
constexpr int OOB_NAME_MAX_SIZE = 256; // size limit of deviceName of OobData
constexpr int ADDRESS_BYTE_LEN = 6;

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
    napi_set_named_property(env, result, "remoteDeviceName", deviceName);

    napi_value deviceAddress;
    napi_create_string_utf8(env, transferInformation.GetDeviceAddress().c_str(), NAPI_AUTO_LENGTH, &deviceAddress);
    napi_set_named_property(env, result, "remoteDeviceId", deviceAddress);

    napi_value direction;
    napi_create_int32(env, transferInformation.GetDirection(), &direction);
    napi_set_named_property(env, result, "direction", direction);

    napi_value status;
    napi_create_int32(env, transferInformation.GetStatus(), &status);
    napi_set_named_property(env, result, "status", status);

    napi_value results;
    napi_create_int32(env, transferInformation.GetResult(), &results);
    napi_set_named_property(env, result, "result", results);

    napi_value timeStamp;
    napi_create_int64(env, transferInformation.GetTimeStamp(), &timeStamp);
    napi_set_named_property(env, result, "timeStamp", timeStamp);

    napi_value currentBytes;
    napi_create_int64(env, transferInformation.GetCurrentBytes(), &currentBytes);
    napi_set_named_property(env, result, "currentBytes", currentBytes);

    napi_value totalBytes;
    napi_create_int64(env, transferInformation.GetTotalBytes(), &totalBytes);
    napi_set_named_property(env, result, "totalBytes", totalBytes);

    napi_value currentCount;
    napi_create_int32(env, transferInformation.GetCurrentCount(), &currentCount);
    napi_set_named_property(env, result, "currentCount", currentCount);

    napi_value totalCount;
    napi_create_int32(env, transferInformation.GetTotalCount(), &totalCount);
    napi_set_named_property(env, result, "totalCount", totalCount);

    return napi_ok;
}

napi_status ConvertStringVectorToJS(napi_env env, napi_value result, const std::vector<std::string>& stringVector)
{
    HILOGI("vector size: %{public}zu", stringVector.size());
    size_t idx = 0;

    if (stringVector.empty()) {
        return napi_ok;
    }

    for (const auto& str : stringVector) {
        napi_value obj = nullptr;
        NAPI_BT_CALL_RETURN(napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &obj));
        NAPI_BT_CALL_RETURN(napi_set_element(env, result, idx, obj));
        idx++;
    }
    return napi_ok;
}

void ConvertStateChangeParamToJS(napi_env env, napi_value result, const ConnStateChangeParam &stateChangeParam)
{
    napi_value deviceId = nullptr;
    napi_create_string_utf8(env, stateChangeParam.device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value profileState = nullptr;
    napi_create_int32(env, GetProfileConnectionState(stateChangeParam.state), &profileState);
    napi_set_named_property(env, result, "state", profileState);

    napi_value disconnectReason = nullptr;
    if (stateChangeParam.isDisconnected) {
        napi_create_int32(env, stateChangeParam.reason, &disconnectReason);
        napi_set_named_property(env, result, "reason", disconnectReason);
    }

    napi_value stateChangeCause = nullptr;
    napi_create_int32(env, stateChangeParam.cause, &stateChangeCause);
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

napi_status ConvertServiceUuidsToJS(const napi_env env, napi_value &uuidsNapi,
    const std::vector<UUID> &uuids)
{
    size_t idx = 0;
    if (uuids.empty()) {
        return napi_ok;
    }
    for (const auto &uuid : uuids) {
        napi_value obj = nullptr;
        NAPI_BT_CALL_RETURN(napi_create_string_utf8(env, uuid.ToString().c_str(), NAPI_AUTO_LENGTH, &obj));
        NAPI_BT_CALL_RETURN(napi_set_element(env, uuidsNapi, idx, obj));
        idx++;
    }
    return napi_ok;
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
        case static_cast<int32_t>(ProfileId::PROFILE_HID_DEVICE):
            HILOGD("PROFILE_HID_DEVICE");
            profileId = PROFILE_ID_HID_DEVICE;
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

void UnregisterSysBLEObserver(napi_env env, const std::string &type)
{
    std::lock_guard<std::mutex> lock(g_sysBLEObserverMutex);
    auto iter = g_sysBLEObserver.find(type);
    if (iter != g_sysBLEObserver.end()) {
        std::array<std::shared_ptr<BluetoothCallbackInfo>, ARGS_SIZE_THREE> &callbackArray = iter->second;
        for (auto &callback: callbackArray) {
            if (callback != nullptr && env == callback->env_) {
                napi_delete_reference(env, callback->callback_);
                callback->callback_ = nullptr;
                HILOGI("delete ref success");
            }
        }
        g_sysBLEObserver.erase(iter);
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

bool IsRandomStaticAddress(std::string address)
{
    // RANDOM STATIC: (addr & 0xC0) == 0xC0
    // RANDOM RESOLVABLE: (addr & 0xC0) == 0x40
    // RANDOM non-RESOLVABLE: (addr & 0xC0) == 0x00
    if (!IsValidAddress(address)) {
        return false;
    }
    // 获取第一个字节
    std::string firstByteStr = address.substr(0, 2);
    uint8_t firstByteUint = 0;
    // 将第一个字节从十六进制转换为整数
    if (!ConvertStrToDigit(firstByteStr, firstByteUint, 16)) { // 16 means input string is hexadecimal
        return false;
    }
    // 检查第一个字符的高两位是否为11(即0xC0)
    return (firstByteUint & 0xC0) == 0xC0;
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

napi_status NapiIsNull(napi_env env, napi_value value)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, value, &valuetype));
    NAPI_BT_RETURN_IF(valuetype != napi_null, "Wrong argument type. Null expected.", napi_invalid_arg);
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

napi_status ParseUint8ArrayParam(napi_env env, napi_value array, std::vector<uint8_t> &outParam)
{
    void *data = nullptr;
    size_t dataLen;
    size_t offset;
    napi_value arrayBuffer = nullptr;
    napi_typedarray_type type;
    NAPI_BT_CALL_RETURN(napi_get_typedarray_info(env, array, &type, &dataLen, &data, &arrayBuffer, &offset));
    NAPI_BT_RETURN_IF(type != napi_typedarray_type::napi_uint8_array, "Wrong argument type, uint8array expected",
        napi_invalid_arg);
    uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data) + offset;
    std::vector<uint8_t> initDataStr(dataPtr, dataPtr + dataLen);
    outParam.assign(initDataStr.begin(), initDataStr.end());
    return napi_ok;
}

napi_status NapiParseObjectUint8Array(napi_env env, napi_value object, const char *name, bool &outExist,
    std::vector<uint8_t> &outParam)
{
    bool hasProperty = false;
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &hasProperty));
    if (hasProperty) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        std::vector<uint8_t> vec {};
        NAPI_BT_CALL_RETURN(ParseUint8ArrayParam(env, property, vec));
        outParam = std::move(vec);
    }
    outExist = hasProperty;
    return napi_ok;
}

napi_status ParseAddressInfoParam(napi_env env, napi_value object, AddressInfo &addressInfo)
{
    HILOGD("enter");
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"address", "addressType", "rawAddressType"}));
    bool exist = false;

    // address is not optional
    std::string address {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, object, "address", exist, address));
    if (!IsValidAddress(address)) {
        HILOGE("Invalid address: %{public}s", address.c_str());
        return napi_invalid_arg;
    }
    addressInfo.SetAddress(address);

    // addressType is not optional
    int32_t addressType = AddressType::UNSET_ADDRESS;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, object, "addressType", exist, addressType));
    bool isRealAddr = (addressType == AddressType::REAL_ADDRESS);
    bool isVirtualAddr = (addressType == AddressType::VIRTUAL_ADDRESS);
    NAPI_BT_RETURN_IF(!isRealAddr && !isVirtualAddr, "Invalid addressType", napi_invalid_arg);
    addressInfo.SetAddressType(static_cast<uint8_t>(addressType));

    // rawAddressType is optional
    int32_t rawAddressType = RawAddressType::UNSET_RAW_ADDRESS;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, object, "rawAddressType", exist, rawAddressType));
    if (exist) {
        bool isPublicAddr = (rawAddressType == RawAddressType::PUBLIC_ADDRESS);
        bool isRandomAddr = (rawAddressType == RawAddressType::RANDOM_ADDRESS);
        NAPI_BT_RETURN_IF(!isPublicAddr && !isRandomAddr, "Invalid rawAddressType", napi_invalid_arg);
        addressInfo.SetRawAddressType(static_cast<uint8_t>(rawAddressType));
    }
    return napi_ok;
}

void GetAddressWithType(const std::string &address, const uint8_t rawAddressType, std::vector<uint8_t> &addressWithType)
{
    std::string hexAddrStr = std::regex_replace(address, std::regex(":"), "");
    for (size_t i = 0; i < hexAddrStr.length(); i += 2) { // 2 char is one byte
        std::string byteStr = hexAddrStr.substr(i, 2);  // 2 char is one byte
        uint8_t byte;
        ConvertStrToDigit(byteStr, byte, 16); // 16 means input string is hexadecimal
        addressWithType.push_back(byte);
    }
    addressWithType.push_back(rawAddressType);
    return;
}

napi_status ParseOobDataParam(napi_env env, napi_value object, const int32_t transport, AddressInfo &addressInfo,
    OobData &oobData)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"deviceId", "confirmationHash", "randomizerHash",
        "deviceName", "deviceRole"}));
    bool exist = false;

    napi_value property;
    NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, "deviceId", &property));
    NAPI_BT_CALL_RETURN(NapiIsObject(env, property));
    NAPI_BT_CALL_RETURN(ParseAddressInfoParam(env, property, addressInfo));
    NAPI_BT_RETURN_IF(!addressInfo.HasRawAddressType(), "rawAddressType must be given for OOB", napi_invalid_arg);

    // addressWithType is not optional
    bool isBRTransport = (transport == BT_TRANSPORT_BREDR);
    bool isPublicAddr = (addressInfo.GetRawAddressType() == RawAddressType::PUBLIC_ADDRESS);
    NAPI_BT_RETURN_IF(isBRTransport && !isPublicAddr, "Invalid transport and rawAddressType", napi_invalid_arg);
    std::vector<uint8_t> addressWithType {};
    GetAddressWithType(addressInfo.GetAddress(), addressInfo.GetRawAddressType(), addressWithType);
    oobData.SetAddressWithType(addressWithType);

    // confirmationHash is not optional
    std::vector<uint8_t> confirmHash {};
    NAPI_BT_CALL_RETURN(NapiParseObjectUint8Array(env, object, "confirmationHash", exist, confirmHash));
    NAPI_BT_RETURN_IF(confirmHash.size() != OOB_C_SIZE, "size of confirmationHash should be 16", napi_invalid_arg);
    oobData.SetConfirmationHash(confirmHash);

    // randomizerHash is optional
    std::vector<uint8_t> randomHash {};
    NAPI_BT_CALL_RETURN(NapiParseObjectUint8Array(env, object, "randomizerHash", exist, randomHash));
    if (exist) {
        NAPI_BT_RETURN_IF(randomHash.size() != OOB_R_SIZE, "size of randomizerHash should be 16", napi_invalid_arg);
        oobData.SetRandomizerHash(randomHash);
    }

    // deviceName is optional
    std::string deviceName {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, object, "deviceName", exist, deviceName));
    if (exist) {
        NAPI_BT_RETURN_IF(deviceName.empty() || deviceName.length() > OOB_NAME_MAX_SIZE,
            "size of deviceName should between 0 and 256", napi_invalid_arg);
        oobData.SetDeviceName(deviceName);
    }

    // deviceRole is optional
    int32_t deviceRole = DEVICE_ROLE_PERIPHERAL_ONLY;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, object, "deviceRole", exist, deviceRole));
    if (exist) {
        NAPI_BT_RETURN_IF((deviceRole < DEVICE_ROLE_PERIPHERAL_ONLY) || (deviceRole > DEVICE_ROLE_BOTH_PREFER_CENTRAL),
            "invalid deviceRole", napi_invalid_arg);
        oobData.SetDeviceRole(static_cast<uint8_t>(deviceRole));
    }
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

napi_status NapiParseSetPhyValue(napi_env env, napi_value object, BlePhyInfo &phyInfo)
{
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"txPhy", "rxPhy", "phyMode"}));
    if (NapiIsObjectPropertyExist(env, object, "phyMode")) {
        NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "phyMode", phyInfo.phyOptions));
        if (phyInfo.phyOptions < static_cast<int32_t>(BLE_PHY_CODED_NO_PREFERRED) ||
            phyInfo.phyOptions > static_cast<int32_t>(BLE_PHY_CODED_S8)) {
            return napi_invalid_arg;
        }
    }
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "txPhy", phyInfo.txPhy));
    if (phyInfo.txPhy < static_cast<int32_t>(BLE_PHY_1M) || phyInfo.txPhy > static_cast<int32_t>(BLE_PHY_CODED)) {
        return napi_invalid_arg;
    }

    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "rxPhy", phyInfo.rxPhy));
    if (phyInfo.rxPhy < static_cast<int32_t>(BLE_PHY_1M) || phyInfo.rxPhy > static_cast<int32_t>(BLE_PHY_CODED)) {
        return napi_invalid_arg;
    }
    return napi_ok;
}

int GetCurrentSdkVersion(void)
{
    int version = SDK_VERSION_20;  // default sdk version is api 20

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr.");
        return version;
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy.");
        return version;
    }
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        HILOGE("Failed to get bundle manager proxy.");
        return version;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = iBundleMgr->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        HILOGE("GetBundleInfoForSelf: get fail.");
        return version;
    }

    version = static_cast<int>(bundleInfo.targetVersion % 100); // %100 to get the real version
    return version;
}

int GetSDKAdaptedStatusCode(int status)
{
    std::set<int> statusCodeForNewSdk = {
        BT_ERR_MAX_RESOURCES,
        BT_ERR_OPERATION_BUSY,
        BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
        BT_ERR_GATT_CONNECTION_CONGESTED,
        BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED,
        BT_ERR_BLE_ADV_DATA_EXCEED_LIMIT,
        BT_ERR_BLE_INVALID_ADV_ID,
    };
    bool isNewStatusCode = false;
    if (statusCodeForNewSdk.find(status) != statusCodeForNewSdk.end()) {
        isNewStatusCode = true;
    }
    // If sdk version is lower than SDK_VERSION_20, the status code is changed to old version.
    if (GetCurrentSdkVersion() < SDK_VERSION_20 && isNewStatusCode) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return status;
}

NapiMap CreateNapiMap(napi_env env)
{
    NapiMap res = {nullptr, nullptr};
    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok || !global) {
        return res;
    }
    napi_value constructor = nullptr;
    if (napi_get_named_property(env, global, "Map", &constructor) != napi_ok || !constructor) {
        return res;
    }
    if (NapiIsFunction(env, constructor) != napi_ok) {
        return res;
    }
    napi_value map_instance = nullptr;
    if (napi_new_instance(env, constructor, 0, nullptr, &map_instance) != napi_ok || !map_instance) {
        return res;
    }
    napi_value map_set = nullptr;
    if (napi_get_named_property(env, map_instance, "set", &map_set) != napi_ok || !map_set) {
        return res;
    }
    if (NapiIsFunction(env, map_set) != napi_ok) {
        return res;
    }
    res.instance = map_instance;
    res.set_function = map_set;
    return res;
}

std::string UpperStr(const string &str)
{
    std::string upperString = str;
    std::transform(upperString.begin(), upperString.end(), upperString.begin(), ::toupper);
    return upperString;
}

std::string HexToString(uint8_t value)
{
    std::stringstream strStream;
    // 用0补齐，避免单字节
    strStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value); // 2 表示强制两位十六进制
    return strStream.str();
}

std::string BuildAddressString(const std::vector<uint8_t> &data)
{
    std::string addr = "";
    if (data.size() != ADDRESS_BYTE_LEN) {
        HILOGE("Invalid address length");
        return addr;
    }
    for (size_t i = 0; i < ADDRESS_BYTE_LEN; i++) {
        uint8_t temp = data[i];
        addr += HexToString(temp);
        if (i < ADDRESS_BYTE_LEN - 1) { // 判断是否为地址的最后一个字节
            addr += ":";
        }
    }
    return UpperStr(addr);
}
}  // namespace Bluetooth
}  // namespace OHOS
