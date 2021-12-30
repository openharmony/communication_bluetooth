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

#include "napi_bluetooth_host.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_host_observer.h"
#include "napi_bluetooth_remote_device_observer.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
namespace {
NapiBluetoothHostObserver g_bluetoothHostObserver;
NapiBluetoothRemoteDeviceObserver g_bluetoothRemoteDevice;
std::string g_RemoteDeviceAddr;
}  // namespace

napi_value BluetoothHostInit(napi_env env, napi_value exports)
{
    HILOGI("BluetoothHostInit start");
    StateChangeInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("enableBluetooth", EnableBluetooth),
        DECLARE_NAPI_FUNCTION("disableBluetooth", DisableBluetooth),
        DECLARE_NAPI_FUNCTION("setLocalName", SetLocalName),
        DECLARE_NAPI_FUNCTION("getLocalName", GetLocalName),
        DECLARE_NAPI_FUNCTION("setBluetoothScanMode", SetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("getBluetoothScanMode", GetBluetoothScanMode),
        DECLARE_NAPI_FUNCTION("startBluetoothDiscovery", StartBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("stopBluetoothDiscovery", StopBluetoothDiscovery),
        DECLARE_NAPI_FUNCTION("getState", GetState),
        DECLARE_NAPI_FUNCTION("getBtConnectionState", GetBtConnectionState),
        DECLARE_NAPI_FUNCTION("pairDevice", PairDevice),
        DECLARE_NAPI_FUNCTION("getPairedDevices", GetPairedDevices),
        DECLARE_NAPI_FUNCTION("setDevicePariringConfirmation", SetDevicePariringConfirmation),
        DECLARE_NAPI_FUNCTION("on", RegisterObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterObserver),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("BluetoothHostInit end");
    return exports;
}

napi_value EnableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("EnableBluetooth start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->RegisterObserver(g_bluetoothHostObserver);
    host->RegisterRemoteDeviceObserver(g_bluetoothRemoteDevice);
    bool enabled = host->EnableBt();
    enabled |= host->EnableBle();
    napi_value result = nullptr;
    napi_get_boolean(env, enabled, &result);
    HILOGI("EnableBluetooth end");
    return result;
}

napi_value DisableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("DisableBluetooth start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->DeregisterObserver(g_bluetoothHostObserver);
    host->DeregisterRemoteDeviceObserver(g_bluetoothRemoteDevice);
    bool enabled = host->DisableBt();
    enabled &= host->DisableBle();
    napi_value result = nullptr;
    napi_get_boolean(env, enabled, &result);
    HILOGI("DisableBluetooth end");
    return result;
}

napi_value SetLocalName(napi_env env, napi_callback_info info)
{
    HILOGI("SetLocalName start");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[PARAM0], nullptr, 0, &bufferSize);
    HILOGI("local name writeChar bufferSize = %{public}d", (int)bufferSize);
    char buffer[bufferSize + 1];
    napi_get_value_string_utf8(env, argv[PARAM0], buffer, bufferSize + 1, &strLength);
    std::string localName(buffer);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool enabled = host->SetLocalName(localName);
    napi_value result = nullptr;
    napi_get_boolean(env, enabled, &result);
    HILOGI("SetLocalName end");
    return result;
}

napi_value GetLocalName(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    HILOGI("GetLocalName start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localName = host->GetLocalName();
    napi_create_string_utf8(env, localName.c_str(), localName.size(), &result);
    HILOGI("GetLocalName end");
    return result;
}

napi_value SetBluetoothScanMode(napi_env env, napi_callback_info info)
{
    HILOGI("SetBluetoothScanMode start");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int32_t mode = 0;
    napi_get_value_int32(env, argv[PARAM0], &mode);
    HILOGI("SetBluetoothScanMode::mode = %{public}d", mode);
    NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int32_t duration = 0;
    napi_get_value_int32(env, argv[PARAM1], &duration);
    HILOGI("SetBluetoothScanMode::duration = %{public}d", duration);

    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool enabled = host->SetBtScanMode(mode, duration);
    host->SetBondableMode(BT_TRANSPORT_BREDR, 1);
    napi_value result = nullptr;
    napi_get_boolean(env, enabled, &result);
    HILOGI("SetBluetoothScanMode end");
    return result;
}

napi_value GetBluetoothScanMode(napi_env env, napi_callback_info info)
{
    HILOGI("GetBluetoothScanMode start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t scanMode = host->GetBtScanMode();
    napi_value result = nullptr;
    napi_create_uint32(env, scanMode, &result);
    HILOGI("GetBluetoothScanMode end");
    return result;
}

napi_value StartBluetoothDiscovery(napi_env env, napi_callback_info info)
{
    HILOGI("StartBluetoothDiscovery start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isStart = host->StartBtDiscovery();
    napi_value result = nullptr;
    napi_get_boolean(env, isStart, &result);
    HILOGI("StartBluetoothDiscovery end");
    return result;
}

napi_value StopBluetoothDiscovery(napi_env env, napi_callback_info info)
{
    HILOGI("StopBluetoothDiscovery start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isStart = host->CancelBtDiscovery();
    napi_value result = nullptr;
    napi_get_boolean(env, isStart, &result);
    HILOGI("StopBluetoothDiscovery end");
    return result;
}

napi_value GetState(napi_env env, napi_callback_info info)
{
    HILOGI("GetState start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t state = host->GetBtState();
    int32_t status = 0;
    switch (state) {
        case BTStateID::STATE_TURNING_ON:
            HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turning on");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_ON);
            break;
        case BTStateID::STATE_TURN_ON:
            HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turn on");
            status = static_cast<int32_t>(BluetoothState::STATE_ON);
            break;
        case BTStateID::STATE_TURNING_OFF:
            HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turning off");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_OFF);
            break;
        case BTStateID::STATE_TURN_OFF:
            HILOGI("NapiBluetoothHostObserver::OnStateChanged BREDR Turn off");
            status = static_cast<int32_t>(BluetoothState::STATE_OFF);
            break;
        default:
            break;
    }

    bool enableBle = host->IsBleEnabled();
    if (enableBle && (status == BTStateID::STATE_TURN_OFF)) {
        status = static_cast<int32_t>(BluetoothState::STATE_BLE_ON);
    }

    napi_value result = nullptr;
    HILOGI("GetState start state %{public}d", status);
    napi_create_int32(env, status, &result);
    HILOGI("GetState end");
    return result;
}

napi_value GetBtConnectionState(napi_env env, napi_callback_info info)
{
    HILOGI("GetBtConnectionState start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t state = host->GetBtConnectionState();
    HILOGI("GetBtConnectionState start state %{public}d", state);
    int32_t profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
    switch (state) {
        case static_cast<int32_t>(BTConnectState::CONNECTING):
            HILOGI("NapiBluetoothHostObserver::GetBtConnectionState BTConnectState connecting");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::CONNECTED):
            HILOGI("NapiBluetoothHostObserver::GetBtConnectionState BTConnectState connected");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTED;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTING):
            HILOGI("NapiBluetoothHostObserver::GetBtConnectionState BTConnectState disconnecting");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTED):
            HILOGI("NapiBluetoothHostObserver::GetBtConnectionState BTConnectState disconnected");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    napi_value result = nullptr;
    napi_create_int32(env, profileConnectionState, &result);
    HILOGI("GetBtConnectionState end");
    return result;
}

napi_value PairDevice(napi_env env, napi_callback_info info)
{
    HILOGI("PairDevice start");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }
    ParseString(env, g_RemoteDeviceAddr, argv[PARAM0]);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(g_RemoteDeviceAddr, BT_TRANSPORT_BREDR);
    bool isSuccess = remoteDevice.StartPair();
    napi_value result = nullptr;
    napi_get_boolean(env, isSuccess, &result);
    HILOGI("PairDevice end");
    return result;
}

napi_value GetPairedDevices(napi_env env, napi_callback_info info)
{
    HILOGI("GetPairedDevices start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists = host->GetPairedDevices(BT_TRANSPORT_BREDR);
    napi_value result = nullptr;
    int count = 0;
    napi_create_array(env, &result);
    for (auto vec : remoteDeviceLists) {
        napi_value remoteDeviceResult;
        napi_create_string_utf8(env, vec.GetDeviceAddr().c_str(), vec.GetDeviceAddr().size(), &remoteDeviceResult);
        napi_set_element(env, result, count, remoteDeviceResult);
        count++;
    }
    HILOGI("GetPairedDevices end");
    return result;
}

napi_value SetDevicePariringConfirmation(napi_env env, napi_callback_info info)
{
    HILOGI("SetDevicePariringConfirmation start");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }
    ParseString(env, g_RemoteDeviceAddr, argv[PARAM0]);
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
    bool accept = false;
    napi_get_value_bool(env, argv[PARAM1], &accept);
    HILOGI("SetDevicePariringConfirmation::accept = %{public}d", accept);
    bool isSuccess = false;
    BluetoothHost::GetDefaultHost()
        .GetRemoteDevice(g_RemoteDeviceAddr, BT_TRANSPORT_BREDR)
        .SetDevicePairingConfirmation(accept);
    napi_value result = nullptr;
    napi_get_boolean(env, isSuccess, &result);
    HILOGI("SetDevicePariringConfirmation end");
    return result;
}

static void SetCallback(const napi_env &env, const napi_ref &callbackIn, const int &errorCode, const napi_value &result)
{
    HILOGI("SetCallback enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ARGS_SIZE_TWO] = {nullptr};
    results[PARAM0] = GetCallbackErrorValue(env, errorCode);
    results[PARAM1] = result;
    NAPI_CALL_RETURN_VOID(
        env, napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &results[PARAM0], &resultout));
    HILOGI("SetCallback end");
}

static void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result)
{
    HILOGI("SetPromise enter");
    napi_resolve_deferred(env, deferred, result);
    HILOGI("SetPromise end");
}

static void PaddingCallbackPromiseInfo(
    const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise)
{
    HILOGI("PaddingCallbackPromiseInfo enter");

    if (callback) {
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
    HILOGI("PaddingCallbackPromiseInfo end");
}

static void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result)
{
    HILOGI("ReturnCallbackPromise enter");

    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result);
    } else {
        SetPromise(env, info.deferred, result);
    }
    HILOGI("ReturnCallbackPromise end");
}

static napi_value JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return NapiGetNull(env);
    } else {
        napi_value promise = nullptr;
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        SetPromise(env, deferred, NapiGetNull(env));
        return promise;
    }
}

static napi_value GetCallback(const napi_env &env, const napi_value &value, CallbackPromiseInfo &params)
{
    HILOGI("GetCallback enter");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    napi_create_reference(env, value, 1, &params.callback);
    HILOGI("GetCallback end");
    return NapiGetNull(env);
}

static napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, CallbackPromiseInfo &params)
{
    HILOGI("ParseParameters enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argv[PARAM0] != nullptr) {
        GetCallback(env, argv[PARAM0], params);
    }
    HILOGI("ParseParameters end");
    return NapiGetNull(env);
}

static void GetDeviceNameSyncWorkStart(const napi_env env, GattGetDeviceNameCallbackInfo *asynccallbackinfo)
{
    std::string deviceId = GetGattClientDeviceId();
    asynccallbackinfo->deviceId =
        BluetoothHost::GetDefaultHost().GetRemoteDevice(deviceId, BT_TRANSPORT_BLE).GetDeviceName();
    if (asynccallbackinfo->deviceId.empty()) {
        HILOGI("GetDeviceName failed.");
        asynccallbackinfo->promise.errorCode = CODE_FAILED;
        asynccallbackinfo->result = NapiGetNull(env);
    } else {
        HILOGI("GetDeviceName success.");
        napi_value result = nullptr;
        napi_create_string_utf8(env, asynccallbackinfo->deviceId.c_str(), asynccallbackinfo->deviceId.size(), &result);
        asynccallbackinfo->result = result;
        asynccallbackinfo->promise.errorCode = CODE_SUCCESS;
    }
}

napi_value GetDeviceName(napi_env env, napi_callback_info info)
{
    HILOGI("GetDeviceName start");
    napi_ref callback = nullptr;
    GattGetDeviceNameCallbackInfo *asynccallbackinfo =
        new (std::nothrow)GattGetDeviceNameCallbackInfo{.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }
    ParseParameters(env, info, asynccallbackinfo->promise);
    napi_value promise = nullptr;
    PaddingCallbackPromiseInfo(env, asynccallbackinfo->promise.callback, asynccallbackinfo->promise, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getDeviceName", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOGI("GetDeviceName napi_create_async_work start");
            GattGetDeviceNameCallbackInfo *asynccallbackinfo = (GattGetDeviceNameCallbackInfo *)data;
            GetDeviceNameSyncWorkStart(env, asynccallbackinfo);
        },
        [](napi_env env, napi_status status, void *data) {
            HILOGI("GetDeviceName napi_create_async_work complete start");
            GattGetDeviceNameCallbackInfo *asynccallbackinfo = (GattGetDeviceNameCallbackInfo *)data;
            ReturnCallbackPromise(env, asynccallbackinfo->promise, asynccallbackinfo->result);
            if (asynccallbackinfo->promise.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->promise.callback);
            }
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
            HILOGI("GetDeviceName napi_create_async_work complete end");
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));
    if (asynccallbackinfo->promise.isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
    HILOGI("GetDeviceName end");
}

static void GetRssiValueSyncWorkStart(const napi_env env, GattGetRssiValueCallbackInfo *asynccallbackinfo)
{
    std::string deviceId = GetGattClientDeviceId();
    bool isResult = BluetoothHost::GetDefaultHost().GetRemoteDevice(deviceId, BT_TRANSPORT_BLE).ReadRemoteRssiValue();
    if (!isResult) {
        asynccallbackinfo->promise.errorCode = CODE_FAILED;
        asynccallbackinfo->result = NapiGetNull(env);
    } else {
        std::unique_lock<std::mutex> lock(asynccallbackinfo->mutexRssi);
        asynccallbackinfo->env = env;
        std::shared_ptr<GattGetRssiValueCallbackInfo> callbackInfo(asynccallbackinfo);
        SetRssiValueCallbackInfo(callbackInfo);
        if (asynccallbackinfo->cvfull.wait_for(lock, std::chrono::seconds(THREAD_WAIT_TIMEOUT)) ==
            std::cv_status::timeout) {
            HILOGI("GetRssiValue ReadRemoteRssi timeout!");
        }
    }
}

napi_value GetRssiValue(napi_env env, napi_callback_info info)
{
    HILOGI("GetRssiValue start");
    napi_ref callback = nullptr;
    GattGetRssiValueCallbackInfo *asynccallbackinfo =
        new (std::nothrow)GattGetRssiValueCallbackInfo{.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }
    ParseParameters(env, info, asynccallbackinfo->promise);
    napi_value promise = nullptr;
    PaddingCallbackPromiseInfo(env, asynccallbackinfo->promise.callback, asynccallbackinfo->promise, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getRssiValue", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOGI("GetRssiValue napi_create_async_work start");
            GattGetRssiValueCallbackInfo *asynccallbackinfo = (GattGetRssiValueCallbackInfo *)data;
            GetRssiValueSyncWorkStart(env, asynccallbackinfo);
        },
        [](napi_env env, napi_status status, void *data) {
            HILOGI("GetRssiValue napi_create_async_work complete start");
            GattGetRssiValueCallbackInfo *asynccallbackinfo = (GattGetRssiValueCallbackInfo *)data;
            ReturnCallbackPromise(env, asynccallbackinfo->promise, asynccallbackinfo->result);

            if (asynccallbackinfo->promise.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->promise.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
            HILOGI("GetRssiValue napi_create_async_work complete end");
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));
    if (asynccallbackinfo->promise.isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
    HILOGI("GetRssiValue end");
}

napi_value StateChangeInit(napi_env env, napi_value exports)
{
    HILOGI("StateChangeInit start");

    napi_value stateObj = nullptr;
    napi_value profileStateObj = nullptr;
    napi_value scanModeObj = nullptr;
    napi_create_object(env, &stateObj);
    napi_create_object(env, &profileStateObj);
    napi_create_object(env, &scanModeObj);

    SetNamedPropertyByInteger(env, stateObj, static_cast<int>(BluetoothState::STATE_OFF), "STATE_OFF");
    SetNamedPropertyByInteger(env, stateObj, static_cast<int>(BluetoothState::STATE_TURNING_ON), "STATE_TURNING_ON");
    SetNamedPropertyByInteger(env, stateObj, static_cast<int>(BluetoothState::STATE_ON), "STATE_ON");
    SetNamedPropertyByInteger(env, stateObj, static_cast<int>(BluetoothState::STATE_TURNING_OFF), "STATE_TURNING_OFF");
    SetNamedPropertyByInteger(
        env, stateObj, static_cast<int>(BluetoothState::STATE_BLE_TURNING_ON), "STATE_BLE_TURNING_ON");
    SetNamedPropertyByInteger(env, stateObj, static_cast<int>(BluetoothState::STATE_BLE_ON), "STATE_BLE_ON");
    SetNamedPropertyByInteger(
        env, stateObj, static_cast<int>(BluetoothState::STATE_BLE_TURNING_OFF), "STATE_BLE_TURNING_OFF");

    SetNamedPropertyByInteger(env, profileStateObj, ProfileConnectionState::STATE_DISCONNECTED, "STATE_DISCONNECTED");
    SetNamedPropertyByInteger(env, profileStateObj, ProfileConnectionState::STATE_CONNECTING, "STATE_CONNECTING");
    SetNamedPropertyByInteger(env, profileStateObj, ProfileConnectionState::STATE_CONNECTED, "STATE_CONNECTED");
    SetNamedPropertyByInteger(env, profileStateObj, ProfileConnectionState::STATE_DISCONNECTING, "STATE_DISCONNECTING");

    SetNamedPropertyByInteger(env, scanModeObj, static_cast<int>(ScanMode::SCAN_MODE_NONE), "SCAN_MODE_NONE");
    SetNamedPropertyByInteger(
        env, scanModeObj, static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE), "SCAN_MODE_CONNECTABLE");
    SetNamedPropertyByInteger(
        env, scanModeObj, static_cast<int>(ScanMode::SCAN_MODE_GENERAL_DISCOVERABLE), "SCAN_MODE_GENERAL_DISCOVERABLE");
    SetNamedPropertyByInteger(env,
        scanModeObj,
        static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE),
        "SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE");
    SetNamedPropertyByInteger(env,
        scanModeObj,
        static_cast<int>(ScanMode::SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE),
        "SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BluetoothState", stateObj),
        DECLARE_NAPI_PROPERTY("ProfileConnectionState", profileStateObj),
        DECLARE_NAPI_PROPERTY("ScanMode", scanModeObj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    HILOGI("StateChangeInit end");
    return exports;
}
}  // namespace Bluetooth
}  // namespace OHOS