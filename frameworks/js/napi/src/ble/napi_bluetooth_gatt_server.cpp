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
#define LOG_TAG "bt_napi_gatt_server"
#endif

#include "napi_bluetooth_gatt_server.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_ble.h"
#include "napi_bluetooth_ble_utils.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_event_subscribe_module.h"
#include "parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::vector<std::string> NapiGattServer::deviceList_;
std::mutex NapiGattServer::deviceListMutex_;
thread_local napi_ref NapiGattServer::consRef_ = nullptr;

napi_value NapiGattServer::CreateGattServer(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value result;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &result);

    return result;
}

void NapiGattServer::DefineGattServerJSClass(napi_env env)
{
    napi_property_descriptor gattserverDesc[] = {
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("notifyCharacteristicChanged", NotifyCharacteristicChangedEx),
        DECLARE_NAPI_FUNCTION("setPhy", SetPhy),
        DECLARE_NAPI_FUNCTION("readPhy", ReadPhy),
#else
        DECLARE_NAPI_FUNCTION("startAdvertising", StartAdvertising),
        DECLARE_NAPI_FUNCTION("stopAdvertising", StopAdvertising),
        DECLARE_NAPI_FUNCTION("notifyCharacteristicChanged", NotifyCharacteristicChanged),
#endif
        DECLARE_NAPI_FUNCTION("addService", AddService),
        DECLARE_NAPI_FUNCTION("removeService", RemoveGattService),
        DECLARE_NAPI_FUNCTION("getService", GetService),
        DECLARE_NAPI_FUNCTION("getServices", GetServices),
        DECLARE_NAPI_FUNCTION("close", Close),
        DECLARE_NAPI_FUNCTION("sendResponse", SendResponse),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("onBlePhyUpdate", OnBlePhyUpdate),
        DECLARE_NAPI_FUNCTION("offBlePhyUpdate", OffBlePhyUpdate),
        DECLARE_NAPI_FUNCTION("getConnectedState", GetConnectedState),
    };

    napi_value constructor = nullptr;
    napi_define_class(env, "GattServer", NAPI_AUTO_LENGTH, GattServerConstructor, nullptr,
        sizeof(gattserverDesc) / sizeof(gattserverDesc[0]), gattserverDesc, &constructor);
    napi_create_reference(env, constructor, 1, &consRef_);
}

napi_value NapiGattServer::GattServerConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    NapiGattServer* gattServer = new NapiGattServer();

    auto status = napi_wrap(
        env, thisVar, gattServer,
        [](napi_env env, void* data, void* hint) {
            NapiGattServer* server = static_cast<NapiGattServer*>(data);
            if (server) {
                delete server;
                server = nullptr;
            }
        },
        nullptr,
        nullptr);
    if (status != napi_ok) {
        HILOGE("napi_wrap failed");
        delete gattServer;
        gattServer = nullptr;
    }

    return thisVar;
}

static NapiGattServer *NapiGetGattServer(napi_env env, napi_value thisVar)
{
    NapiGattServer *gattServer = nullptr;
    auto status = napi_unwrap(env, thisVar, (void **)&gattServer);
    if (status != napi_ok) {
        return nullptr;
    }
    return gattServer;
}

static NapiGattServer *NapiGetGattServer(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr) != napi_ok) {
        return nullptr;
    }
    return NapiGetGattServer(env, thisVar);
}

napi_value NapiGattServer::On(napi_env env, napi_callback_info info)
{
    NapiGattServer *napiGattServer = NapiGetGattServer(env, info);
    if (napiGattServer && napiGattServer->GetCallback()) {
        auto status = napiGattServer->GetCallback()->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiGattServer::Off(napi_env env, napi_callback_info info)
{
    NapiGattServer *napiGattServer = NapiGetGattServer(env, info);
    if (napiGattServer && napiGattServer->GetCallback()) {
        auto status = napiGattServer->GetCallback()->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiGattServer::OnBlePhyUpdate(napi_env env, napi_callback_info info)
{
    NapiGattServer *napiGattServer = NapiGetGattServer(env, info);
    if (napiGattServer && napiGattServer->GetCallback()) {
        auto status = napiGattServer->GetCallback()->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiGattServer::OffBlePhyUpdate(napi_env env, napi_callback_info info)
{
    NapiGattServer *napiGattServer = NapiGetGattServer(env, info);
    if (napiGattServer && napiGattServer->GetCallback()) {
        auto status = napiGattServer->GetCallback()->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_GATT_SERVER_CALLBACK_BLE_PHY_UPDATE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

static napi_status CheckGattsAddService(napi_env env, napi_callback_info info, std::shared_ptr<GattServer> &outServer,
    std::unique_ptr<GattService> &outService)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 arguments.", napi_invalid_arg);

    // std::unique_ptr<GattService> service {nullptr};
    NapiGattService napiGattService;
    NAPI_BT_CALL_RETURN(NapiParseGattService(env, argv[PARAM0], napiGattService));

    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);
    outServer = gattServer->GetServer();

    GattServiceType type = napiGattService.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    outService = std::make_unique<GattService>(napiGattService.serviceUuid, type);
    for (const auto &napiCharacter : napiGattService.characteristics) {
        int charPermissions = napiCharacter.permissions;
        int charProperties = napiCharacter.properties;
        GattCharacteristic character(napiCharacter.characteristicUuid, charPermissions, charProperties);
        character.SetValue(napiCharacter.characteristicValue.data(), napiCharacter.characteristicValue.size());

        for (const auto &napiDescriptor : napiCharacter.descriptors) {
            GattDescriptor descriptor(napiDescriptor.descriptorUuid, napiDescriptor.permissions);
            descriptor.SetValue(napiDescriptor.descriptorValue.data(), napiDescriptor.descriptorValue.size());
            character.AddDescriptor(descriptor);
        }
        outService->AddCharacteristic(character);
    }

    return napi_ok;
}

napi_value NapiGattServer::AddService(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    std::unique_ptr<GattService> gattService {nullptr};
    auto status = CheckGattsAddService(env, info, server, gattService);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    int ret = server->AddService(*gattService);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckGattsEmptyParam(napi_env env, napi_callback_info info, std::shared_ptr<GattServer> &outServer)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, nullptr, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc > 0, "no needed arguments.", napi_invalid_arg);
    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);

    outServer = gattServer->GetServer();
    return napi_ok;
}

napi_value NapiGattServer::Close(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    auto status = CheckGattsEmptyParam(env, info, server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    int ret = server->Close();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);
    return NapiGetUndefinedRet(env);
}

static napi_status CheckGattsServiceUuid(napi_env env, napi_callback_info info,
    std::shared_ptr<GattServer> &outServer, UUID &outUuid)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != 1, "Requires 1 arguments.", napi_invalid_arg);

    std::string uuid {};
    NAPI_BT_CALL_RETURN(NapiParseUuid(env, argv[0], uuid));

    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);
    outServer = gattServer->GetServer();
    outUuid = UUID::FromString(uuid);
    return napi_ok;
}

napi_value NapiGattServer::RemoveGattService(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    UUID serviceUuid;
    auto status = CheckGattsServiceUuid(env, info, server, serviceUuid);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    int ret = BT_ERR_INTERNAL_ERROR;
    auto primaryService = server->GetService(serviceUuid, true);
    if (primaryService.has_value()) {
        ret = server->RemoveGattService(*primaryService);
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    }
    auto secondService = server->GetService(serviceUuid, false);
    if (secondService.has_value()) {
        ret = server->RemoveGattService(*secondService);
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    }
    NAPI_BT_ASSERT_RETURN_FALSE(env, (primaryService.has_value() || secondService.has_value()), BT_ERR_INVALID_PARAM);
    return NapiGetBooleanRet(env, ret == BT_NO_ERROR);
}

napi_value NapiGattServer::GetService(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    UUID serviceUuid;
    auto status = CheckGattsServiceUuid(env, info, server, serviceUuid);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    UUID fakeUuid = UUID::FromString("00000000-0000-0000-0000-000000000000");
    GattService outService(fakeUuid, GattServiceType::PRIMARY);
    int ret = server->GetService(serviceUuid, true, outService);
    if (ret != BT_NO_ERROR) {
        ret = server->GetService(serviceUuid, false, outService);
    }
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    ConvertGattServiceToJS(env, obj, outService);
    return obj;
}

napi_value NapiGattServer::GetServices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    auto status = CheckGattsEmptyParam(env, info, server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    std::list<GattService> serviceList;
    int ret = server->GetServices(serviceList);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);

    std::vector<GattService> serviceVec(serviceList.begin(), serviceList.end());
    napi_value obj = nullptr;
    napi_create_array(env, &obj);
    ConvertGattServiceVectorToJS(env, obj, serviceVec);
    return obj;
}

static napi_status CheckGattsSendRsp(napi_env env, napi_callback_info info, std::shared_ptr<GattServer> &outServer,
    NapiGattsServerResponse &outRsp)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != 1, "Requires 1 arguments.", napi_invalid_arg);

    NapiGattsServerResponse rsp;
    NAPI_BT_CALL_RETURN(NapiParseGattsServerResponse(env, argv[0], rsp));

    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);
    outServer = gattServer->GetServer();
    outRsp = std::move(rsp);
    return napi_ok;
}

napi_value NapiGattServer::SendResponse(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    NapiGattsServerResponse rsp;
    auto status = CheckGattsSendRsp(env, info, server, rsp);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    BluetoothRemoteDevice remoteDevice(rsp.deviceId, BTTransport::ADAPTER_BLE);
    HILOGI("Remote device address: %{public}s", GET_ENCRYPT_ADDR(remoteDevice));
    int ret = server->SendResponse(remoteDevice, rsp.transId, rsp.status, rsp.offset, rsp.value.data(),
        static_cast<int>(rsp.value.size()));
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status ParseGattServerGetConnectState(
    napi_env env, napi_callback_info info, std::string &deviceId, NapiGattServer **outGattServer)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", napi_invalid_arg);
    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr || outGattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], deviceId));
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId");
        return napi_invalid_arg;
    }
    *outGattServer = gattServer;
    return napi_ok;
}

napi_value NapiGattServer::GetConnectedState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string deviceId;
    NapiGattServer *server = nullptr;

    auto status = ParseGattServerGetConnectState(env, info, deviceId, &server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattServer> gattServer = server->GetServer();
    int state = -1;
    auto checkStatus = gattServer->GetConnectedState(deviceId, state);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkStatus == BT_NO_ERROR, checkStatus);
    int profileState = GetProfileConnectionState(state);
    napi_value result = nullptr;
    napi_create_int32(env, profileState, &result);
    return result;
}

static GattCharacteristic *GetGattCharacteristic(const std::shared_ptr<GattServer> &server, const UUID &serviceUuid,
    const UUID &characterUuid)
{
    auto service = server->GetService(serviceUuid, true);
    if (!service.has_value()) {
        service = server->GetService(serviceUuid, false);
    }
    if (!service.has_value()) {
        HILOGE("not found service uuid: %{public}s", serviceUuid.ToString().c_str());
        return nullptr;
    }
    GattCharacteristic *character = service.value().get().GetCharacteristic(characterUuid);
    return character;
}

#ifdef BLUETOOTH_API_SINCE_10
static napi_status CheckNotifyCharacteristicChangedEx(napi_env env, napi_callback_info info,
    NapiGattServer **outServer, std::string &outDeviceId, NapiNotifyCharacteristic &outCharacter)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);

    std::string deviceId {};
    NapiNotifyCharacteristic character;
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], deviceId));
    NAPI_BT_CALL_RETURN(NapiParseNotifyCharacteristic(env, argv[PARAM1], character));

    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr || outServer ==nullptr, "gattServer is nullptr.", napi_invalid_arg);
    *outServer = gattServer;
    outDeviceId = std::move(deviceId);
    outCharacter = std::move(character);
    return napi_ok;
}

napi_value NapiGattServer::NotifyCharacteristicChangedEx(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattServer* napiServer = nullptr;
    std::string deviceId {};
    NapiNotifyCharacteristic notifyCharacter;
    auto status = CheckNotifyCharacteristicChangedEx(env, info, &napiServer, deviceId, notifyCharacter);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && napiServer && napiServer->GetServer()),
        BT_ERR_INVALID_PARAM);

    auto func = [server = napiServer->GetServer(), notifyCharacter, deviceId]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        auto character = GetGattCharacteristic(server, notifyCharacter.serviceUuid, notifyCharacter.characterUuid);
        if (character == nullptr) {
            HILOGI("character is null!");
            return NapiAsyncWorkRet(ret);
        }
        character->SetValue(notifyCharacter.characterValue.data(), notifyCharacter.characterValue.size());
        BluetoothRemoteDevice remoteDevice(deviceId, BTTransport::ADAPTER_BLE);
        ret = server->NotifyCharacteristicChanged(remoteDevice, *character, notifyCharacter.confirm);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);

    bool success = napiServer->GetCallback()->asyncWorkMap_.TryPush(NapiAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC,
        asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status ParseSetPhyValue(napi_env env, napi_callback_info info, std::string &outDeviceId,
    BlePhyInfo &outPhyValue, NapiGattServer **outGattServer)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr || outGattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);

    std::string deviceId {};
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], deviceId));
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId");
        return napi_invalid_arg;
    }
    NAPI_BT_CALL_RETURN(NapiParseSetPhyValue(env, argv[PARAM1], outPhyValue));

    *outGattServer = gattServer;
    outDeviceId = std::move(deviceId);
    return napi_ok;
}


napi_value NapiGattServer::SetPhy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string deviceId;
    BlePhyInfo phyValue(static_cast<int32_t>(BLE_PHY_1M), static_cast<int32_t>(BLE_PHY_1M),
        BLE_PHY_CODED_NO_PREFERRED);
    NapiGattServer *server = nullptr;

    auto status = ParseSetPhyValue(env, info, deviceId, phyValue, &server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [gattServer = server->GetServer(), deviceId, phyValue]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattServer) {
            ret = gattServer->SetPhy(deviceId, phyValue.txPhy, phyValue.rxPhy, phyValue.phyOptions);
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = server->GetCallback()->asyncWorkMap_.TryPush(
        NapiAsyncType::GATT_SERVER_SET_PHY, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status ParseReadPhyValue(napi_env env, napi_callback_info info,
    std::string &outDeviceId, NapiGattServer **outGattServer)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", napi_invalid_arg);
    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr || outGattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);

    std::string deviceId {};
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], deviceId));
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId");
        return napi_invalid_arg;
    }
    *outGattServer = gattServer;
    outDeviceId = std::move(deviceId);
    return napi_ok;
}

napi_value NapiGattServer::ReadPhy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string deviceId;
    NapiGattServer *server = nullptr;

    auto status = ParseReadPhyValue(env, info, deviceId, &server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [gattServer = server->GetServer(), deviceId]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattServer) {
            ret = gattServer->ReadPhy(deviceId);
        }
        return NapiAsyncWorkRet(ret);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = server->GetCallback()->asyncWorkMap_.TryPush(
        NapiAsyncType::GATT_SERVER_READ_PHY, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}
#else
static napi_status CheckGattsNotify(napi_env env, napi_callback_info info, std::shared_ptr<GattServer> &outServer,
    std::string &outDeviceId, NapiNotifyCharacteristic &outCharacter)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);

    std::string deviceId {};
    NapiNotifyCharacteristic character;
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], deviceId));
    NAPI_BT_CALL_RETURN(NapiParseNotifyCharacteristic(env, argv[PARAM1], character));

    NapiGattServer *gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);
    outServer = gattServer->GetServer();
    outDeviceId = std::move(deviceId);
    outCharacter = std::move(character);
    return napi_ok;
}

napi_value NapiGattServer::NotifyCharacteristicChanged(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<GattServer> server {nullptr};
    std::string deviceId {};
    NapiNotifyCharacteristic notifyCharacter;
    auto status = CheckGattsNotify(env, info, server, deviceId, notifyCharacter);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    auto character = GetGattCharacteristic(server, notifyCharacter.serviceUuid, notifyCharacter.characterUuid);
    NAPI_BT_ASSERT_RETURN_FALSE(env, character != nullptr, BT_ERR_INVALID_PARAM);
    character->SetValue(notifyCharacter.characterValue.data(), notifyCharacter.characterValue.size());

    BluetoothRemoteDevice remoteDevice(deviceId, BTTransport::ADAPTER_BLE);
    int ret = server->NotifyCharacteristicChanged(remoteDevice, *character, notifyCharacter.confirm);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}
#endif
} // namespace Bluetooth
} // namespace OHOS
