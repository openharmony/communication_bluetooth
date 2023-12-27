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

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<NapiBluetoothConnectionObserver> g_connectionObserver =
    std::make_shared<NapiBluetoothConnectionObserver>();
std::shared_ptr<NapiBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    std::make_shared<NapiBluetoothRemoteDeviceObserver>();
std::mutex deviceMutex;
std::vector<std::shared_ptr<BluetoothRemoteDevice>> g_DiscoveryDevices;

std::map<std::string, std::function<napi_value(napi_env env)>> g_callbackDefaultValue = {
    {REGISTER_DEVICE_FIND_TYPE,
        [](napi_env env) -> napi_value {
            napi_value result = 0;
            napi_value value = 0;
            napi_create_array(env, &result);
            napi_create_string_utf8(env, INVALID_DEVICE_ID.c_str(), NAPI_AUTO_LENGTH, &value);
            napi_set_element(env, result, 0, value);
            return result;
        }},
    {REGISTER_PIN_REQUEST_TYPE,
        [](napi_env env) -> napi_value {
            napi_value result = 0;
            napi_value deviceId = nullptr;
            napi_value pinCode = nullptr;
            napi_create_object(env, &result);
            napi_create_string_utf8(env, INVALID_DEVICE_ID.c_str(), NAPI_AUTO_LENGTH, &deviceId);
            napi_set_named_property(env, result, "deviceId", deviceId);
            napi_create_string_utf8(env, INVALID_PIN_CODE.c_str(), NAPI_AUTO_LENGTH, &pinCode);
            napi_set_named_property(env, result, "pinCode", pinCode);
            return result;
        }},
    {REGISTER_BOND_STATE_TYPE, [](napi_env env) -> napi_value {
         napi_value result = 0;
         napi_value deviceId = nullptr;
         napi_value state = nullptr;
         napi_create_object(env, &result);
         napi_create_string_utf8(env, INVALID_DEVICE_ID.c_str(), NAPI_AUTO_LENGTH, &deviceId);
         napi_set_named_property(env, result, "deviceId", deviceId);
         napi_create_int32(env, static_cast<int32_t>(BondState::BOND_STATE_INVALID), &state);
         napi_set_named_property(env, result, "state", state);
         return result;
     }}};

napi_value DefineConnectionFunctions(napi_env env, napi_value exports)
{
    HILOGD("enter");
    RegisterObserverToHost();
    ConnectionPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getBtConnectionState", GetBtConnectionState),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("pairDevice", PairDeviceAsync),
        DECLARE_NAPI_FUNCTION("cancelPairedDevice", CancelPairedDeviceAsync),
        DECLARE_NAPI_FUNCTION("getProfileConnectionState", GetProfileConnectionStateEx),
#else
        DECLARE_NAPI_FUNCTION("pairDevice", PairDevice),
        DECLARE_NAPI_FUNCTION("cancelPairedDevice", CancelPairedDevice),
        DECLARE_NAPI_FUNCTION("getProfileConnectionState", GetProfileConnectionState),
#endif
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
#ifdef BLUETOOTH_API_SINCE_10
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
#endif
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static bool IsValidObserverType(const std::string &callbackName)
{
    if (callbackName == REGISTER_DEVICE_FIND_TYPE || callbackName == REGISTER_PIN_REQUEST_TYPE ||
        callbackName == REGISTER_BOND_STATE_TYPE) {
        return true;
    } else {
        HILOGE("not support %{public}s.", callbackName.c_str());
        return false;
    }
}

napi_status CheckRegisterObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);

    std::string callbackName;
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], callbackName));
    NAPI_BT_RETURN_IF(!IsValidObserverType(callbackName), "Invalid type", napi_invalid_arg);

    auto napiCallback = std::make_shared<NapiCallback>(env, argv[PARAM1]);
    if (callbackName == REGISTER_BOND_STATE_TYPE) {
        g_remoteDeviceObserver->RegisterCallback(callbackName, napiCallback);
    } else {
        g_connectionObserver->RegisterCallback(callbackName, napiCallback);
    }
    HILOGI("%{public}s registered", callbackName.c_str());
    return napi_ok;
}

napi_status CheckDeRegisterObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);

    std::string callbackName;
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], callbackName));
    NAPI_BT_RETURN_IF(!IsValidObserverType(callbackName), "Invalid type", napi_invalid_arg);

    if (callbackName == REGISTER_BOND_STATE_TYPE) {
        g_remoteDeviceObserver->DeRegisterCallback(callbackName);
    } else {
        g_connectionObserver->DeRegisterCallback(callbackName);
    }
    HILOGI("%{public}s unregistered", callbackName.c_str());
    return napi_ok;
}

napi_value RegisterConnectionObserver(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto status = CheckRegisterObserver(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

napi_value DeRegisterConnectionObserver(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto status = CheckDeRegisterObserver(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

napi_value GetBtConnectionState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    int32_t err = host->GetBtConnectionState(state);
    HILOGI("start state %{public}d", state);
    napi_value result = nullptr;
    napi_create_int32(env, GetProfileConnectionState(state), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGI("end");
    return result;
}

napi_value PairDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
    int deviceType = remoteDevice.GetDeviceType();
    if (deviceType == INVALID_TYPE) {
        HILOGE("device is not discovery or scan, just quick BLE pair");
        remoteDevice = BluetoothRemoteDevice(remoteAddr, BT_TRANSPORT_BLE);
    }
    if (deviceType == DEVICE_TYPE_LE) {
        remoteDevice = BluetoothRemoteDevice(remoteAddr, BT_TRANSPORT_BLE);
    }
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

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
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

    int transport = GetDeviceTransport(remoteAddr);
    int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport).GetDeviceName(name);
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

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport);
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
    HILOGI("status: %{public}d", status);
    return ret;
}

napi_value GetProfileConnectionStateEx(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int profileId = 0;
    size_t argSize = ARGS_SIZE_ONE;
    bool checkRet = CheckProfileIdParamEx(env, info, profileId, argSize);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);
    HILOGD("argSize = %{public}zu", argSize);

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
    int transport = GetDeviceTransport(remoteAddr);
    int32_t ret = BT_NO_ERROR;
    if (accept) {
        ret =
            BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport).SetDevicePairingConfirmation(accept);
    } else {
        ret = BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport).CancelPairing();
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

void AddDiscoveryDevice(std::shared_ptr<BluetoothRemoteDevice> &device)
{
    std::lock_guard<std::mutex> lock(deviceMutex);
    for (auto dev : g_DiscoveryDevices) {
        if (device->GetDeviceAddr().compare(dev->GetDeviceAddr()) == 0) {
            return;
        }
    }
    g_DiscoveryDevices.push_back(device);
}

void ClearDiscoveryDevice()
{
    std::lock_guard<std::mutex> lock(deviceMutex);
    g_DiscoveryDevices.clear();
}

napi_value StartBluetoothDiscovery(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    ClearDiscoveryDevice();
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
        int transport = GetDeviceTransport(remoteAddr);
        int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport).SetDevicePin(pinCode);
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
        int transport = GetDeviceTransport(remoteAddr);
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
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
        int transport = GetDeviceTransport(remoteAddr);
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
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
        int transport = GetDeviceTransport(remoteAddr);
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
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
    int transport = INVALID_TYPE;
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
        BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(address, BT_TRANSPORT_BREDR);
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
    NAPI_BT_ASSERT_RETURN(env, err = BT_NO_ERROR, err, result);
    HILOGE("isBluetoothDiscovering :%{public}d", isDiscovering);
    return result;
}

napi_value GetPairState(napi_env env, napi_callback_info info)
{
    std::string remoteAddr = INVALID_MAC_ADDRESS;
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int pairState = PAIR_NONE;
    int32_t err = remoteDevice.GetPairState(pairState);
    napi_value result = nullptr;
    NAPI_BT_ASSERT_RETURN(env, napi_create_int32(env, pairState, &result) == napi_ok, err, result);
    NAPI_BT_ASSERT_RETURN(env, err = BT_NO_ERROR, err, result);
    HILOGE("getPairState :%{public}d", pairState);
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

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(remoteAddr, transport);
    std::string productId;
    int32_t err = remoteDevice.GetDeviceProductId(productId);

    napi_value result = nullptr;
    napi_create_string_utf8(env, productId.c_str(), productId.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    HILOGI("GetRemoteProductId :%{public}s", productId.c_str());
    return result;
}

#endif

napi_value ConnectionPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value scanModeObj = ScanModeInit(env);
    napi_value bondStateObj = BondStateInit(env);
#ifdef BLUETOOTH_API_SINCE_10
    napi_value bluetoothTransportObject = BluetoothTransportInit(env);
    napi_value pinTypeObject = PinTypeInit(env);
#endif
    napi_property_descriptor exportProperties[] = {
        DECLARE_NAPI_PROPERTY("ScanMode", scanModeObj),
        DECLARE_NAPI_PROPERTY("BondState", bondStateObj),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_PROPERTY("BluetoothTransport", bluetoothTransportObject),
        DECLARE_NAPI_PROPERTY("PinType", pinTypeObject),
#endif
    };
    napi_define_properties(env, exports, sizeof(exportProperties) / sizeof(*exportProperties), exportProperties);
    HILOGI("end");
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

void RegisterObserverToHost()
{
    HILOGD("enter");
    BluetoothHost &host = BluetoothHost::GetDefaultHost();
    host.RegisterObserver(g_connectionObserver);
    host.RegisterRemoteDeviceObserver(g_remoteDeviceObserver);
}

int GetDeviceTransport(const std::string &device)
{
    std::lock_guard<std::mutex> lock(deviceMutex);
    for (auto dev : g_DiscoveryDevices) {
        if (device.compare(dev->GetDeviceAddr()) == 0) {
            return dev->GetTransportType();
        }
    }
    return BT_TRANSPORT_BREDR;
}
}  // namespace Bluetooth
}  // namespace OHOS