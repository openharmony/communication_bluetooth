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
#define LOG_TAG "bt_napi_connection"
#endif

#include "napi_bluetooth_connection.h"

#include <set>

#include "napi_bluetooth_connection_observer.h"
#include "napi_bluetooth_remote_device_observer.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_error.h"
#include "napi_async_work.h"
#include "napi_bluetooth_utils.h"
#include "parser/napi_parser_utils.h"
#include "hitrace_meter.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<NapiBluetoothConnectionObserver> g_connectionObserver =
    std::make_shared<NapiBluetoothConnectionObserver>();
std::shared_ptr<NapiBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    std::make_shared<NapiBluetoothRemoteDeviceObserver>();
std::mutex deviceMutex;

std::map<std::string, std::function<napi_value(napi_env env)>> g_callbackDefaultValue = {
    {REGISTER_DEVICE_FIND_TYPE,
        [](napi_env env) -> napi_value {
            napi_value result = 0;
            napi_value value = 0;
            napi_create_array(env, &result);
            napi_create_string_utf8(env, INVALID_DEVICE_ID, NAPI_AUTO_LENGTH, &value);
            napi_set_element(env, result, 0, value);
            return result;
        }},
    {REGISTER_PIN_REQUEST_TYPE,
        [](napi_env env) -> napi_value {
            napi_value result = 0;
            napi_value deviceId = nullptr;
            napi_value pinCode = nullptr;
            napi_create_object(env, &result);
            napi_create_string_utf8(env, INVALID_DEVICE_ID, NAPI_AUTO_LENGTH, &deviceId);
            napi_set_named_property(env, result, "deviceId", deviceId);
            napi_create_string_utf8(env, INVALID_PIN_CODE, NAPI_AUTO_LENGTH, &pinCode);
            napi_set_named_property(env, result, "pinCode", pinCode);
            return result;
        }},
    {REGISTER_BOND_STATE_TYPE, [](napi_env env) -> napi_value {
         napi_value result = 0;
         napi_value deviceId = nullptr;
         napi_value state = nullptr;
         napi_create_object(env, &result);
         napi_create_string_utf8(env, INVALID_DEVICE_ID, NAPI_AUTO_LENGTH, &deviceId);
         napi_set_named_property(env, result, "deviceId", deviceId);
         napi_create_int32(env, static_cast<int32_t>(BondState::BOND_STATE_INVALID), &state);
         napi_set_named_property(env, result, "state", state);
         return result;
     }}};
#ifdef BLUETOOTH_API_SINCE_10
napi_value DefineConnectionFunctions(napi_env env, napi_value exports)
{
    RegisterObserverToHost();
    ConnectionPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getBtConnectionState", GetBtConnectionState),
        DECLARE_NAPI_FUNCTION("pairDevice", PairDeviceAsync),
        DECLARE_NAPI_FUNCTION("cancelPairedDevice", CancelPairedDeviceAsync),
        DECLARE_NAPI_FUNCTION("getProfileConnectionState", GetProfileConnectionStateEx),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceName", GetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceClass", GetRemoteDeviceClass),
        DECLARE_NAPI_FUNCTION("getLocalName", GetLocalName),
        DECLARE_NAPI_FUNCTION("getPairedDevices", GetPairedDevices),
        DECLARE_NAPI_FUNCTION("getProfileConnState", GetProfileConnectionState),
        DECLARE_NAPI_FUNCTION("setDevicePairingConfirmation", SetDevicePairingConfirmation),
        DECLARE_NAPI_FUNCTION("setLocalName", SetLocalName),
        DECLARE_NAPI_FUNCTION("setBluetoothScanMode", SetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("getBluetoothScanMode", GetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("startBluetoothDiscovery", StartBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("stopBluetoothDiscovery", StopBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("setDevicePinCode", SetDevicePinCode),
        DECLARE_NAPI_FUNCTION("cancelPairingDevice", CancelPairingDevice),
        DECLARE_NAPI_FUNCTION("pairCredibleDevice", PairCredibleDevice),
        DECLARE_NAPI_FUNCTION("getLocalProfileUuids", GetLocalProfileUuids),
        DECLARE_NAPI_FUNCTION("getRemoteProfileUuids", GetRemoteProfileUuids),
        DECLARE_NAPI_FUNCTION("on", RegisterConnectionObserver),
        DECLARE_NAPI_FUNCTION("off", DeRegisterConnectionObserver),
        DECLARE_NAPI_FUNCTION("isBluetoothDiscovering", IsBluetoothDiscovering),
        DECLARE_NAPI_FUNCTION("getPairState", GetPairState),
        DECLARE_NAPI_FUNCTION("connectAllowedProfiles", ConnectAllowedProfiles),
        DECLARE_NAPI_FUNCTION("disconnectAllowedProfiles", DisconnectAllowedProfiles),
        DECLARE_NAPI_FUNCTION("getRemoteProductId", GetRemoteProductId),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceName", SetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceType", SetRemoteDeviceType),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceType", GetRemoteDeviceType),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceBatteryInfo", GetRemoteDeviceBatteryInfo),
        DECLARE_NAPI_FUNCTION("controlDeviceAction", ControlDeviceAction),
        DECLARE_NAPI_FUNCTION("getLastConnectionTime", GetRemoteDeviceConnectionTime),
        DECLARE_NAPI_FUNCTION("updateCloudBluetoothDevice", UpdateCloudBluetoothDevice),
    };

    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "connection:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
#else
napi_value DefineConnectionFunctions(napi_env env, napi_value exports)
{
    RegisterObserverToHost();
    ConnectionPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getBtConnectionState", GetBtConnectionState),
        DECLARE_NAPI_FUNCTION("pairDevice", PairDevice),
        DECLARE_NAPI_FUNCTION("cancelPairedDevice", CancelPairedDevice),
        DECLARE_NAPI_FUNCTION("getProfileConnectionState", GetProfileConnectionState),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceName", GetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceClass", GetRemoteDeviceClass),
        DECLARE_NAPI_FUNCTION("getLocalName", GetLocalName),
        DECLARE_NAPI_FUNCTION("getPairedDevices", GetPairedDevices),
        DECLARE_NAPI_FUNCTION("getProfileConnState", GetProfileConnectionState),
        DECLARE_NAPI_FUNCTION("setDevicePairingConfirmation", SetDevicePairingConfirmation),
        DECLARE_NAPI_FUNCTION("setLocalName", SetLocalName),
        DECLARE_NAPI_FUNCTION("setBluetoothScanMode", SetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("getBluetoothScanMode", GetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("startBluetoothDiscovery", StartBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("stopBluetoothDiscovery", StopBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceName", SetRemoteDeviceName),
        DECLARE_NAPI_FUNCTION("setRemoteDeviceType", SetRemoteDeviceType),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceType", GetRemoteDeviceType),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceBatteryInfo", GetRemoteDeviceBatteryInfo),
        DECLARE_NAPI_FUNCTION("controlDeviceAction", ControlDeviceAction),
        DECLARE_NAPI_FUNCTION("getLastConnectionTime", GetRemoteDeviceConnectionTime),
        DECLARE_NAPI_FUNCTION("updateCloudBluetoothDevice", UpdateCloudBluetoothDevice),
    };

    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "connection:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
#endif

using NapiBluetoothOnOffFunc = std::function<napi_status(napi_env env, napi_callback_info info)>;

static napi_status NapiConnectionOnOffExecute(napi_env env, napi_callback_info info,
    NapiBluetoothOnOffFunc connectionObserverFunc, NapiBluetoothOnOffFunc remoteDeviceObserverFunc)
{
    std::string type = "";
    NAPI_BT_CALL_RETURN(NapiGetOnOffCallbackName(env, info, type));

    napi_status status = napi_ok;
    if (type == REGISTER_DEVICE_FIND_TYPE ||
        type == REGISTER_DISCOVERY_RESULT_TYPE ||
        type == REGISTER_PIN_REQUEST_TYPE) {
        status = connectionObserverFunc(env, info);
    } else if (type == REGISTER_BOND_STATE_TYPE || type == REGISTER_BATTERY_CHANGE_TYPE) {
        status = remoteDeviceObserverFunc(env, info);
    } else {
        HILOGE("Unsupported callback: %{public}s", type.c_str());
        status = napi_invalid_arg;
    }
    return status;
}

napi_value RegisterConnectionObserver(napi_env env, napi_callback_info info)
{
    auto connectionObserverFunc = [](napi_env env, napi_callback_info info) {
        return g_connectionObserver->eventSubscribe_.Register(env, info);
    };
    auto remoteDeviceObserverFunc =  [](napi_env env, napi_callback_info info) {
        return g_remoteDeviceObserver->eventSubscribe_.Register(env, info);
    };

    auto status = NapiConnectionOnOffExecute(env, info, connectionObserverFunc, remoteDeviceObserverFunc);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

napi_value DeRegisterConnectionObserver(napi_env env, napi_callback_info info)
{
    auto connectionObserverFunc = [](napi_env env, napi_callback_info info) {
        return g_connectionObserver->eventSubscribe_.Deregister(env, info);
    };
    auto remoteDeviceObserverFunc =  [](napi_env env, napi_callback_info info) {
        return g_remoteDeviceObserver->eventSubscribe_.Deregister(env, info);
    };

    auto status = NapiConnectionOnOffExecute(env, info, connectionObserverFunc, remoteDeviceObserverFunc);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

napi_value GetBtConnectionState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    int32_t err = host->GetBtConnectionState(state);
    HILOGD("start state %{public}d", state);
    napi_value result = nullptr;
    napi_create_int32(env, GetProfileConnectionState(state), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    return result;
}

napi_value PairDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t ret = remoteDevice.StartPair();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value CancelPairedDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->RemovePair(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);

    return NapiGetBooleanTrue(env);
}

napi_value GetRemoteDeviceName(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    std::string name = INVALID_NAME;
    napi_value result = nullptr;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    napi_create_string_utf8(env, name.c_str(), name.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, checkRet == true, BT_ERR_INVALID_PARAM, result);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name);
    napi_create_string_utf8(env, name.c_str(), name.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    return result;
}

napi_value GetRemoteDeviceClass(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int tmpCod = MajorClass::MAJOR_UNCATEGORIZED;
    int tmpMajorClass = MajorClass::MAJOR_UNCATEGORIZED;
    int tmpMajorMinorClass = MajorClass::MAJOR_UNCATEGORIZED;
    int32_t err = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value majorClass = 0;
    napi_create_int32(env, tmpMajorClass, &majorClass);
    napi_set_named_property(env, result, "majorClass", majorClass);
    napi_value majorMinorClass = 0;
    napi_create_int32(env, tmpMajorMinorClass, &majorMinorClass);
    napi_set_named_property(env, result, "majorMinorClass", majorMinorClass);
    napi_value cod = 0;
    napi_create_int32(env, tmpCod, &cod);
    napi_set_named_property(env, result, "classOfDevice", cod);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    return result;
}

napi_value GetLocalName(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localName = INVALID_NAME;
    int32_t err = host->GetLocalName(localName);
    napi_create_string_utf8(env, localName.c_str(), localName.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGI("end");
    return result;
}

napi_value GetPairedDevices(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists;
    int32_t ret = host->GetPairedDevices(BT_TRANSPORT_BREDR, remoteDeviceLists);
    napi_value result = nullptr;
    int count = 0;
    napi_create_array(env, &result);
    for (auto vec : remoteDeviceLists) {
        napi_value remoteDeviceResult;
        napi_create_string_utf8(env, vec.GetDeviceAddr().c_str(), vec.GetDeviceAddr().size(), &remoteDeviceResult);
        napi_set_element(env, result, count, remoteDeviceResult);
        count++;
    }
    NAPI_BT_ASSERT_RETURN(env, ret == BT_NO_ERROR, ret, result);
    HILOGI("end");
    return result;
}

napi_value GetProfileConnectionState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int profileId = 0;
    bool checkRet = CheckProfileIdParam(env, info, profileId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    int32_t err = host->GetBtProfileConnState(GetProfileId(profileId), state);
    int status = GetProfileConnectionState(state);
    napi_value ret = nullptr;
    napi_create_int32(env, status, &ret);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, ret);
    HILOGD("status: %{public}d", status);
    return ret;
}

napi_value GetProfileConnectionStateEx(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int profileId = 0;
    size_t argSize = ARGS_SIZE_ONE;
    bool checkRet = CheckProfileIdParamEx(env, info, profileId, argSize);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    napi_value ret = nullptr;
    if (argSize == 0) {
        ret = GetBtConnectionState(env, info);
    } else {
        ret = GetProfileConnectionState(env, info);
    }
    return ret;
}

napi_value SetDevicePairingConfirmation(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    bool accept = false;
    bool checkRet = CheckSetDevicePairingConfirmationParam(env, info, remoteAddr, accept);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    HILOGI("SetDevicePairingConfirmation::accept = %{public}d", accept);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t ret = BT_NO_ERROR;
    if (accept) {
        ret = remoteDevice.SetDevicePairingConfirmation(accept);
    } else {
        ret = remoteDevice.CancelPairing();
    }
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value SetLocalName(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string localName = INVALID_NAME;
    bool checkRet = CheckLocalNameParam(env, info, localName);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetLocalName(localName);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value SetBluetoothScanMode(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int32_t mode = 0;
    int32_t duration = 0;
    bool checkRet = CheckSetBluetoothScanModeParam(env, info, mode, duration);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);
    HILOGI("mode = %{public}d,duration = %{public}d", mode, duration);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetBtScanMode(mode, duration);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    host->SetBondableMode(BT_TRANSPORT_BREDR, 1);
    return NapiGetBooleanTrue(env);
}

napi_value GetBluetoothScanMode(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t scanMode = 0;
    int32_t err = host->GetBtScanMode(scanMode);
    napi_value result = nullptr;
    napi_create_uint32(env, scanMode, &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGI("end");
    return result;
}

napi_value StartBluetoothDiscovery(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->StartBtDiscovery();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value StopBluetoothDiscovery(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->CancelBtDiscovery();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

#ifdef BLUETOOTH_API_SINCE_10
napi_status ParseSetDevicePinCodeParameters(napi_env env, napi_callback_info info,
    std::string &outRemoteAddr, std::string &outPinCode)
{
    HILOGD("enter");
    std::string remoteAddr{};
    std::string pinCode{};
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE,
        "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], remoteAddr));
    NAPI_BT_RETURN_IF(!ParseString(env, pinCode, argv[PARAM1]), "pinCode ParseString failed", napi_invalid_arg);
    outRemoteAddr = remoteAddr;
    outPinCode = pinCode;
    return napi_ok;
}

napi_value SetDevicePinCode(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = "";
    std::string pinCode = "";
    auto status = ParseSetDevicePinCodeParameters(env, info, remoteAddr, pinCode);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, pinCode]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.SetDevicePin(pinCode);
        HILOGI("SetDevicePinCode err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status CheckDeviceAsyncParam(napi_env env, napi_callback_info info, std::string &addr)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    return napi_ok;
}

napi_value PairDeviceAsync(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    auto checkRet = CheckDeviceAsyncParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.StartPair();
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value CancelPairedDeviceAsync(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeviceAsyncParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t err = host->RemovePair(remoteDevice);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value CancelPairingDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeviceAsyncParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.CancelPairing();
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status CheckPairCredibleDeviceParam(napi_env env, napi_callback_info info, std::string &addr, int &transport)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, transport, argv[PARAM1]), "ParseInt32 failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!IsValidTransport(transport), "Invalid transport", napi_invalid_arg);
    return napi_ok;
}

napi_value PairCredibleDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    int transport = BT_TRANSPORT_NONE;
    auto status = CheckPairCredibleDeviceParam(env, info, remoteAddr, transport);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, transport]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
        int32_t err = remoteDevice.StartCrediblePair();
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status CheckGetProfileUuids(napi_env env, napi_callback_info info, std::string &address)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], address));
    return napi_ok;
}

napi_value GetLocalProfileUuids(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto func = []() {
        std::vector<std::string> uuids{};
        int32_t err = BluetoothHost::GetDefaultHost().GetLocalProfileUuids(uuids);
        HILOGI("err: %{public}d", err);
        auto object = std::make_shared<NapiNativeUuidsArray>(uuids);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value GetRemoteProfileUuids(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string address;
    auto status = CheckGetProfileUuids(env, info, address);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    auto func = [address]() {
        std::vector<std::string> uuids{};
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(address);
        int32_t err = remoteDevice.GetDeviceUuids(uuids);
        HILOGI("err: %{public}d", err);
        auto object = std::make_shared<NapiNativeUuidsArray>(uuids);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value IsBluetoothDiscovering(napi_env env, napi_callback_info info)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isDiscovering = false;
    int32_t err = host->IsBtDiscovering(isDiscovering);
    napi_value result = nullptr;
    NAPI_BT_ASSERT_RETURN(env, napi_get_boolean(env, isDiscovering, &result) == napi_ok, err, result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGE("isBluetoothDiscovering :%{public}d", isDiscovering);
    return result;
}

napi_value GetPairState(napi_env env, napi_callback_info info)
{
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int state = PAIR_NONE;
    int32_t err = remoteDevice.GetPairState(state);
    int pairState = static_cast<int>(BondState::BOND_STATE_INVALID);
    DealPairStatus(state, pairState);
    napi_value result = nullptr;
    NAPI_BT_ASSERT_RETURN(env, napi_create_int32(env, pairState, &result) == napi_ok, err, result);
    NAPI_BT_ASSERT_RETURN(env, (err == BT_NO_ERROR || err == BT_ERR_INTERNAL_ERROR), err, result);
    HILOGI("getPairState :%{public}d", pairState);
    return result;
}

napi_value ConnectAllowedProfiles(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    auto checkRet = CheckDeviceAsyncParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t ret = host->ConnectAllowedProfiles(remoteAddr);
        HILOGI("ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value DisconnectAllowedProfiles(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    auto checkRet = CheckDeviceAsyncParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t ret = host->DisconnectAllowedProfiles(remoteAddr);
        HILOGI("ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value GetRemoteProductId(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    std::string productId;
    int32_t err = remoteDevice.GetDeviceProductId(productId);

    napi_value result = nullptr;
    napi_create_string_utf8(env, productId.c_str(), productId.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGI("GetRemoteProductId :%{public}s", productId.c_str());
    return result;
}

#endif

napi_status ParseSetRemoteDeviceNameParameters(napi_env env, napi_callback_info info,
    std::string &outRemoteAddr, std::string &outDeviceName)
{
    HILOGD("enter");
    std::string remoteAddr{};
    std::string deviceName{};
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], remoteAddr));
    NAPI_BT_RETURN_IF(!ParseString(env, deviceName, argv[PARAM1]), "deviceName ParseString failed", napi_invalid_arg);
    outRemoteAddr = remoteAddr;
    outDeviceName = deviceName;
    return napi_ok;
}

napi_value SetRemoteDeviceName(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = "";
    std::string deviceName = "";
    auto status = ParseSetRemoteDeviceNameParameters(env, info, remoteAddr, deviceName);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, deviceName]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.SetDeviceAlias(deviceName);
        HILOGI("SetDeviceName err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status ParseSetRemoteDeviceTypeParameters(napi_env env, napi_callback_info info,
    std::string &outRemoteAddr, int32_t &outDeviceType)
{
    HILOGD("enter");
    std::string remoteAddr{};
    int32_t deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], remoteAddr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, deviceType, argv[PARAM1]), "deviceType ParseInt32 failed", napi_invalid_arg);
    outRemoteAddr = remoteAddr;
    outDeviceType = deviceType;
    return napi_ok;
}

napi_value SetRemoteDeviceType(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    int32_t deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    auto status = ParseSetRemoteDeviceTypeParameters(env, info, remoteAddr, deviceType);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, deviceType]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.SetDeviceCustomType(deviceType);
        HILOGI("SetRemoteDeviceType err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value GetRemoteDeviceType(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);
    auto func = [remoteAddr]() {
        int32_t deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.GetDeviceCustomType(deviceType);
        HILOGI("GetRemoteDeviceType err: %{public}d", err);
        auto object = std::make_shared<NapiNativeInt>(deviceType);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value GetRemoteDeviceBatteryInfo(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    auto checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);
    auto func = [remoteAddr]() {
        DeviceBatteryInfo batteryInfo;
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.GetRemoteDeviceBatteryInfo(batteryInfo);
        HILOGI("err: %{public}d", err);
        auto object = std::make_shared<NapiNativeBatteryInfo>(batteryInfo);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value ConnectionPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value scanModeObj = ScanModeInit(env);
    napi_value bondStateObj = BondStateInit(env);
    napi_value unbondCauseObj = UnbondCauseInit(env);
#ifdef BLUETOOTH_API_SINCE_10
    napi_value bluetoothTransportObject = BluetoothTransportInit(env);
    napi_value pinTypeObject = PinTypeInit(env);
#endif
    napi_value deviceTypeObject = DeviceTypeInit(env);
    napi_value deviceChargeStateObject = DeviceChargeStateInit(env);
    napi_property_descriptor exportProperties[] = {
        DECLARE_NAPI_PROPERTY("ScanMode", scanModeObj),
        DECLARE_NAPI_PROPERTY("BondState", bondStateObj),
        DECLARE_NAPI_PROPERTY("UnbondCause", unbondCauseObj),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_PROPERTY("BluetoothTransport", bluetoothTransportObject),
        DECLARE_NAPI_PROPERTY("PinType", pinTypeObject),
#endif
        DECLARE_NAPI_PROPERTY("DeviceType", deviceTypeObject),
        DECLARE_NAPI_PROPERTY("DeviceChargeState", deviceChargeStateObject),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "connection:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportProperties) / sizeof(*exportProperties), exportProperties);
    return exports;
}

napi_value ScanModeInit(napi_env env)
{
    HILOGD("enter");
    napi_value scanMode = nullptr;
    napi_create_object(env, &scanMode);
    SetNamedPropertyByInteger(env, scanMode, static_cast<int>(ScanMode::SCAN_MODE_NONE), "SCAN_MODE_NONE");
    SetNamedPropertyByInteger(
        env, scanMode, static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE), "SCAN_MODE_CONNECTABLE");
    SetNamedPropertyByInteger(
        env, scanMode, static_cast<int>(ScanMode::SCAN_MODE_GENERAL_DISCOVERABLE), "SCAN_MODE_GENERAL_DISCOVERABLE");
    SetNamedPropertyByInteger(
        env, scanMode, static_cast<int>(ScanMode::SCAN_MODE_LIMITED_DISCOVERABLE), "SCAN_MODE_LIMITED_DISCOVERABLE");
    SetNamedPropertyByInteger(env,
        scanMode,
        static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE),
        "SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE");
    SetNamedPropertyByInteger(env,
        scanMode,
        static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE),
        "SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE");
    return scanMode;
}

napi_value BondStateInit(napi_env env)
{
    HILOGD("enter");
    napi_value bondState = nullptr;
    napi_create_object(env, &bondState);
    SetNamedPropertyByInteger(env, bondState, static_cast<int>(BondState::BOND_STATE_INVALID), "BOND_STATE_INVALID");
    SetNamedPropertyByInteger(env, bondState, static_cast<int>(BondState::BOND_STATE_BONDING), "BOND_STATE_BONDING");
    SetNamedPropertyByInteger(env, bondState, static_cast<int>(BondState::BOND_STATE_BONDED), "BOND_STATE_BONDED");
    return bondState;
}

napi_value UnbondCauseInit(napi_env env)
{
    HILOGD("enter");
    napi_value unbondCause = nullptr;
    napi_create_object(env, &unbondCause);
    SetNamedPropertyByInteger(env, unbondCause, UNBOND_CAUSE_USER_REMOVED, "USER_REMOVED");
    SetNamedPropertyByInteger(env, unbondCause, UNBOND_CAUSE_REMOTE_DEVICE_DOWN, "REMOTE_DEVICE_DOWN");
    SetNamedPropertyByInteger(env, unbondCause, UNBOND_CAUSE_AUTH_FAILURE, "AUTH_FAILURE");
    SetNamedPropertyByInteger(env, unbondCause, UNBOND_CAUSE_AUTH_REJECTED, "AUTH_REJECTED");
    SetNamedPropertyByInteger(env, unbondCause, UNBOND_CAUSE_INTERNAL_ERROR, "INTERNAL_ERROR");
    return unbondCause;
}

#ifdef BLUETOOTH_API_SINCE_10
napi_value BluetoothTransportInit(napi_env env)
{
    HILOGD("enter");
    napi_value bluetoothTransport = nullptr;
    napi_create_object(env, &bluetoothTransport);
    SetNamedPropertyByInteger(
        env, bluetoothTransport, static_cast<int>(BluetoothTransport::TRANSPORT_BR_EDR), "TRANSPORT_BR_EDR");
    SetNamedPropertyByInteger(
        env, bluetoothTransport, static_cast<int>(BluetoothTransport::TRANSPORT_LE), "TRANSPORT_LE");
    return bluetoothTransport;
}

napi_value PinTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value pinType = nullptr;
    napi_create_object(env, &pinType);
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_ENTER_PIN_CODE), "PIN_TYPE_ENTER_PIN_CODE");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_ENTER_PASSKEY), "PIN_TYPE_ENTER_PASSKEY");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_CONFIRM_PASSKEY), "PIN_TYPE_CONFIRM_PASSKEY");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_NO_PASSKEY_CONSENT), "PIN_TYPE_NO_PASSKEY_CONSENT");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_NOTIFY_PASSKEY), "PIN_TYPE_NOTIFY_PASSKEY");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_DISPLAY_PIN_CODE), "PIN_TYPE_DISPLAY_PIN_CODE");
    SetNamedPropertyByInteger(env, pinType, static_cast<int>(PinType::PIN_TYPE_OOB_CONSENT), "PIN_TYPE_OOB_CONSENT");
    SetNamedPropertyByInteger(
        env, pinType, static_cast<int>(PinType::PIN_TYPE_PIN_16_DIGITS), "PIN_TYPE_PIN_16_DIGITS");
    return pinType;
}
#endif

napi_value DeviceTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value deviceType = nullptr;
    napi_create_object(env, &deviceType);
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_DEFAULT), "DEVICE_TYPE_DEFAULT");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_CAR), "DEVICE_TYPE_CAR");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_HEADSET), "DEVICE_TYPE_HEADSET");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_HEARING), "DEVICE_TYPE_HEARING");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_GLASSES), "DEVICE_TYPE_GLASSES");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_WATCH), "DEVICE_TYPE_WATCH");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_SPEAKER), "DEVICE_TYPE_SPEAKER");
    SetNamedPropertyByInteger(
        env, deviceType, static_cast<int>(DeviceType::DEVICE_TYPE_OTHERS), "DEVICE_TYPE_OTHERS");
    return deviceType;
}

napi_value DeviceChargeStateInit(napi_env env)
{
    HILOGD("enter");
    napi_value deviceChargeState = nullptr;
    napi_create_object(env, &deviceChargeState);
    SetNamedPropertyByInteger(
        env, deviceChargeState, static_cast<int32_t>(DeviceChargeState::DEVICE_NORMAL_CHARGE_NOT_CHARGED),
        "DEVICE_NORMAL_CHARGE_NOT_CHARGED");
    SetNamedPropertyByInteger(
        env, deviceChargeState, static_cast<int32_t>(DeviceChargeState::DEVICE_NORMAL_CHARGE_IN_CHARGING),
        "DEVICE_NORMAL_CHARGE_IN_CHARGING");
    SetNamedPropertyByInteger(
        env, deviceChargeState, static_cast<int32_t>(DeviceChargeState::DEVICE_SUPER_CHARGE_NOT_CHARGED),
        "DEVICE_SUPER_CHARGE_NOT_CHARGED");
    SetNamedPropertyByInteger(
        env, deviceChargeState, static_cast<int32_t>(DeviceChargeState::DEVICE_SUPER_CHARGE_IN_CHARGING),
        "DEVICE_SUPER_CHARGE_IN_CHARGING");
    return deviceChargeState;
}

void RegisterObserverToHost()
{
    HILOGD("enter");
    BluetoothHost &host = BluetoothHost::GetDefaultHost();
    host.RegisterObserver(g_connectionObserver);
    host.RegisterRemoteDeviceObserver(g_remoteDeviceObserver);
}

void DealPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = static_cast<int>(BondState::BOND_STATE_INVALID);
            break;
        case PAIR_PAIRING:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDING);
            break;
        case PAIR_PAIRED:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
}

struct ControlDeviceActionParams {
    std::string deviceId;
    uint32_t controlType;
    uint32_t controlTypeVal;
    uint32_t controlObject;
};

napi_status ParseControlDeviceActionParams(napi_env env, napi_value object, ControlDeviceActionParams &params)
{
    HILOGD("ParseControlDeviceActionParams enter");
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object,
        {"deviceId", "type", "typeValue", "controlObject"}));
    std::string tmpDeviceId = INVALID_MAC_ADDRESS;
    NAPI_BT_CALL_RETURN(NapiParseObjectBdAddr(env, object, "deviceId", tmpDeviceId));
    if (tmpDeviceId.empty() || tmpDeviceId.length() != ADDRESS_LENGTH) {
        HILOGE("Invalid deviceId");
        return napi_invalid_arg;
    }
    uint32_t tmpControlType = INVALID_CONTROL_TYPE;
    uint32_t tmpControlTypeVal = INVALID_CONTROL_TYPE_VAL;
    uint32_t tmpControlObject = INVALID_CONTROL_OBJECT;

    NapiObject napiObject = { env, object };
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Check(napiObject, "type", tmpControlType,
        ControlType::PLAY, ControlType::ERASE));
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Check(napiObject, "typeValue", tmpControlTypeVal,
        ControlTypeVal::DISABLE, ControlTypeVal::QUERY));
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Check(napiObject, "controlObject", tmpControlObject,
        ControlObject::LEFT_EAR, ControlObject::LEFT_RIGHT_EAR));
    
    HILOGI("deviceId: %{public}s, controlType: %{public}u, controlTypeVal: %{public}u, controlObject: %{public}u",
        GetEncryptAddr(tmpDeviceId).c_str(), tmpControlType, tmpControlTypeVal, tmpControlObject);

    params.deviceId = tmpDeviceId;
    params.controlType = tmpControlType;
    params.controlTypeVal = tmpControlTypeVal;
    params.controlObject = tmpControlObject;
    return napi_ok;
}

napi_value ControlDeviceAction(napi_env env, napi_callback_info info)
{
    HILOGD("ControlDeviceAction enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    auto checkRes = napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRes == napi_ok, BT_ERR_INVALID_PARAM);

    ControlDeviceActionParams params = { INVALID_MAC_ADDRESS, INVALID_CONTROL_TYPE,
        INVALID_CONTROL_TYPE_VAL, INVALID_CONTROL_OBJECT };
    auto status = ParseControlDeviceActionParams(env, argv[PARAM0], params);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    std::string deviceId = params.deviceId;
    uint32_t controlType = params.controlType;
    uint32_t controlTypeVal = params.controlTypeVal;
    uint32_t controlObject = params.controlObject;
    auto func = [deviceId, controlType, controlTypeVal, controlObject]() {
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
        int32_t err = remoteDevice.ControlDeviceAction(controlType, controlTypeVal, controlObject);
        HILOGI("ControlDeviceAction err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value GetRemoteDeviceConnectionTime(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);
    auto func = [remoteAddr]() {
        int64_t connectionTime = 0;
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
        int32_t err = remoteDevice.GetLastConnectionTime(connectionTime);
        HILOGI("GetRemoteDeviceConnectionTime GetLastConnectionTime err: %{public}d", err);
        auto object = std::make_shared<NapiNativeInt64>(connectionTime);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value UpdateCloudBluetoothDevice(napi_env env, napi_callback_info info)
{
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    auto checkRes = napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRes == napi_ok, BT_ERR_INVALID_PARAM);
    std::vector<TrustPairDeviceParam> trustPairs {};
    auto status = NapiParseTrustPairDevice(env, argv[PARAM0], trustPairs);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    auto func = [trustPairs]() {
        int32_t err = BluetoothHost::GetDefaultHost().UpdateCloudBluetoothDevice(trustPairs);
        HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}
}  // namespace Bluetooth
}  // namespace OHOS