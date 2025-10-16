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
#define LOG_TAG "bt_napi_access"
#endif

#include "napi_bluetooth_access.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"

#include "napi_bluetooth_error.h"
#include "napi_bluetooth_access_observer.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_spp_client.h"
#include "parser/napi_parser_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
namespace {
std::shared_ptr<NapiBluetoothAccessObserver> g_bluetoothAccessObserver =
    std::make_shared<NapiBluetoothAccessObserver>();
}  // namespace

napi_value NapiAccess::DefineAccessJSFunction(napi_env env, napi_value exports)
{
    HILOGD("enter");
    RegisterAccessObserverToHost();
    AccessPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getState", GetState),
        DECLARE_WRITABLE_NAPI_FUNCTION("enableBluetooth", EnableBluetooth),
        DECLARE_NAPI_FUNCTION("disableBluetooth", DisableBluetooth),
        DECLARE_NAPI_FUNCTION("restrictBluetooth", RestrictBluetooth),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("factoryReset", FactoryReset),
        DECLARE_NAPI_FUNCTION("getLocalAddress", GetLocalAddress),
        DECLARE_NAPI_FUNCTION("on", RegisterAccessObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterAccessObserver),
        DECLARE_NAPI_FUNCTION("addPersistentDeviceId", AddPersistentDeviceId),
        DECLARE_NAPI_FUNCTION("deletePersistentDeviceId", DeletePersistentDeviceId),
        DECLARE_NAPI_FUNCTION("getPersistentDeviceIds", GetPersistentDeviceIds),
        DECLARE_NAPI_FUNCTION("isValidRandomDeviceId", isValidRandomDeviceId),
        DECLARE_NAPI_FUNCTION("enableBluetoothAsync", EnableBluetoothAsync),
        DECLARE_NAPI_FUNCTION("disableBluetoothAsync", DisableBluetoothAsync),
        DECLARE_NAPI_FUNCTION("notifyDialogResult", NotifyDialogResult),
        DECLARE_NAPI_FUNCTION("convertUuid", ConvertUuid),
#endif
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "access:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

void NapiAccess::RegisterAccessObserverToHost()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->RegisterObserver(g_bluetoothAccessObserver);
}

napi_value NapiAccess::EnableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    // if return value is a specified error code, sync interface does not process.
    if (ret == BT_ERR_DIALOG_FOR_USER_CONFIRM) {
        ret = BT_NO_ERROR;
    }
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::RestrictBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto func = []() {
        int32_t ret = BluetoothHost::GetDefaultHost().RestrictBluetooth();
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::DisableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->DisableBt();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::GetState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int32_t state = static_cast<int>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    napi_value result = nullptr;
    napi_create_int32(env, state, &result);
    return result;
}

napi_value NapiAccess::AccessPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value stateObj = StateChangeInit(env);
    napi_value dialogTypeObj = DialogTypeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BluetoothState", stateObj),
        DECLARE_NAPI_PROPERTY("DialogType", dialogTypeObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "access:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value NapiAccess::StateChangeInit(napi_env env)
{
    HILOGD("enter");
    napi_value state = nullptr;
    napi_create_object(env, &state);
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_OFF), "STATE_OFF");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_TURNING_ON), "STATE_TURNING_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_ON), "STATE_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_TURNING_OFF), "STATE_TURNING_OFF");
    SetNamedPropertyByInteger(
        env, state, static_cast<int>(BluetoothState::STATE_BLE_TURNING_ON), "STATE_BLE_TURNING_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_BLE_ON), "STATE_BLE_ON");
    SetNamedPropertyByInteger(
        env, state, static_cast<int>(BluetoothState::STATE_BLE_TURNING_OFF), "STATE_BLE_TURNING_OFF");
    return state;
}

napi_value NapiAccess::DialogTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value dialogType = nullptr;
    napi_create_object(env, &dialogType);
    SetNamedPropertyByInteger(env, dialogType, static_cast<int>(DialogBoxType::BLUETOOTH_SWITCH), "BLUETOOTH_SWITCH");
    return dialogType;
}

napi_value NapiAccess::RegisterAccessObserver(napi_env env, napi_callback_info info)
{
    if (g_bluetoothAccessObserver) {
        auto status = g_bluetoothAccessObserver->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiAccess::DeregisterAccessObserver(napi_env env, napi_callback_info info)
{
    if (g_bluetoothAccessObserver) {
        auto status = g_bluetoothAccessObserver->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

#ifdef BLUETOOTH_API_SINCE_10
napi_value NapiAccess::FactoryReset(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto func = []() {
        int32_t ret = BluetoothHost::GetDefaultHost().BluetoothFactoryReset();
        HILOGI("factoryReset ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::GetLocalAddress(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localAddr = INVALID_MAC_ADDRESS;
    int32_t err = host->GetLocalAddress(localAddr);
    napi_create_string_utf8(env, localAddr.c_str(), localAddr.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    return result;
}

bool CheckConvertUuid(napi_env env, napi_callback_info info, std::string &outUuid)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", false);
    NAPI_BT_RETURN_IF(NapiParseString(env, argv[0], outUuid) != napi_ok, "invalid param.", false);
    NAPI_BT_RETURN_IF((outUuid.size() != STR_LEN_OF_2_BYTES_UUID && outUuid.size() != STR_LEN_OF_4_BYTES_UUID &&
        outUuid.size() != STR_LEN_OF_16_BYTES_UUID), "invalid param.", false);
    return true;
}

napi_value NapiAccess::ConvertUuid(napi_env env, napi_callback_info info)
{
    std::string inputUuid = "";
    auto status = CheckConvertUuid(env, info, inputUuid);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status, BT_ERR_INVALID_PARAM);
    std::string completeUuid = "";
    if (inputUuid.size() == STR_LEN_OF_2_BYTES_UUID) {
        completeUuid = "0000" + inputUuid + "-0000-1000-8000-00805f9b34fb";
    } else if (inputUuid.size() == STR_LEN_OF_4_BYTES_UUID) {
        completeUuid = inputUuid + "-0000-1000-8000-00805f9b34fb";
    } else {
        completeUuid = inputUuid;
    }

    NAPI_BT_ASSERT_RETURN_UNDEF(env, IsValidUuid(completeUuid), BT_ERR_INVALID_PARAM);
    std::for_each(completeUuid.begin(), completeUuid.end(), [](char &c) { c = std::tolower(c); });

    napi_value outputUuid = nullptr;
    napi_create_string_utf8(env, completeUuid.c_str(), NAPI_AUTO_LENGTH, &outputUuid);
    return outputUuid;
}
#endif

napi_value NapiAccess::AddPersistentDeviceId(napi_env env, napi_callback_info info)
{
    std::string deviceId = "";
    auto status = CheckDeviceAddressParam(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [deviceId]() {
        bool isValid = false;
        std::vector<std::string> deviceIdVec = { deviceId };
        int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
            static_cast<int32_t>(RandomDeviceIdCommand::ADD), deviceIdVec, isValid);
        HILOGI("AddPersistentDeviceId ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::DeletePersistentDeviceId(napi_env env, napi_callback_info info)
{
    std::string deviceId = "";
    auto status = CheckDeviceAddressParam(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [deviceId]() mutable {
        bool isValid = false;
        std::vector<std::string> deviceIdVec = { deviceId };
        int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
            static_cast<int32_t>(RandomDeviceIdCommand::DELETE), deviceIdVec, isValid);
        HILOGI("DeletePersistentDeviceId ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::GetPersistentDeviceIds(napi_env env, napi_callback_info info)
{
    bool isValid = false;
    std::vector<std::string> deviceIdVec;
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::GET), deviceIdVec, isValid);
    HILOGI("GetPersistentDeviceIds ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);

    NapiNativeStringArray object(deviceIdVec);
    return object.ToNapiValue(env);
}

napi_value NapiAccess::isValidRandomDeviceId(napi_env env, napi_callback_info info)
{
    std::string deviceId = "";
    auto status = CheckDeviceAddressParam(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    bool isValid = false;
    std::vector<std::string> deviceIdVec = { deviceId };
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::IS_VALID), deviceIdVec, isValid);
    HILOGI("isValidRandomDeviceId ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);

    NapiNativeBool object(isValid);
    return object.ToNapiValue(env);
}

napi_value NapiAccess::EnableBluetoothAsync(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto func = []() {
        bool isAsync = true;
        int32_t ret = BluetoothHost::GetDefaultHost().EnableBle("", isAsync);
        HILOGI("EnableBluetoothAsync ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::DisableBluetoothAsync(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto func = []() {
        bool isAsync = true;
        int32_t ret = BluetoothHost::GetDefaultHost().DisableBt("", isAsync);
        HILOGI("DisableBluetoothAsync ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status ParseNotifyDialogResultParams(napi_env env, napi_value object,
    NapiAccess::NotifyDialogResultParams &params)
{
    HILOGD("enter");
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"dialogType", "dialogResult"}));

    uint32_t tmpDialogType = INVALID_DIALOG_TYPE;
    bool tmpDialogResult = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, object, "dialogType", tmpDialogType));
    NAPI_BT_CALL_RETURN(NapiParseObjectBoolean(env, object, "dialogResult", tmpDialogResult));

    HILOGI("dialogType: %{public}u, dialogResult: %{public}d", tmpDialogType, tmpDialogResult);

    params.dialogType = tmpDialogType;
    params.dialogResult = tmpDialogResult;
    return napi_ok;
}

napi_value NapiAccess::NotifyDialogResult(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    auto checkRes = napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRes == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, argc == ARGS_SIZE_ONE, BT_ERR_INVALID_PARAM);

    NotifyDialogResultParams params = { INVALID_DIALOG_TYPE, false};
    auto status = ParseNotifyDialogResultParams(env, argv[PARAM0], params);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    uint32_t dialogType = params.dialogType;
    bool dialogResult = params.dialogResult;
    auto func = [dialogType, dialogResult]() {
        int32_t ret = BluetoothHost::GetDefaultHost().NotifyDialogResult(dialogType, dialogResult);
        HILOGI("NotifyDialogResult err: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

}  // namespace Bluetooth
}  // namespace OHOS
