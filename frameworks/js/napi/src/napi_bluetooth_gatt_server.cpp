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
#include "napi_bluetooth_gatt_server.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_ble.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::vector<std::string> NapiGattServer::deviceList;
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
        DECLARE_NAPI_FUNCTION("startAdvertising", StartAdvertising),
        DECLARE_NAPI_FUNCTION("stopAdvertising", StopAdvertising),
        DECLARE_NAPI_FUNCTION("addService", AddService),
        DECLARE_NAPI_FUNCTION("removeService", RemoveGattService),
        DECLARE_NAPI_FUNCTION("close", Close),
        DECLARE_NAPI_FUNCTION("notifyCharacteristicChanged", NotifyCharacteristicChanged),
        DECLARE_NAPI_FUNCTION("sendResponse", SendResponse),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
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

    napi_wrap(
        env, thisVar, gattServer,
        [](napi_env env, void* data, void* hint) {
            NapiGattServer* server = (NapiGattServer*)data;
            if (server) {
                delete server;
                server = nullptr;
            }
        },
        nullptr,
        nullptr);

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

napi_status CheckGattsOn(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);

    std::string type {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo {nullptr};

    if (type == STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ ||
        type == STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE) {
        callbackInfo = std::make_shared<GattCharacteristicCallbackInfo>();
    } else if (type == STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE ||
        type == STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ) {
        callbackInfo = std::make_shared<GattDescriptorCallbackInfo>();
    } else {
        callbackInfo = std::make_shared<BluetoothCallbackInfo>();
    }
    callbackInfo->env_ = env;

    auto gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
    NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_));
    gattServer->GetCallback().SetCallbackInfo(type, callbackInfo);
    HILOGI("%{public}s is registered", type.c_str());
    return napi_ok;
}

napi_value NapiGattServer::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckGattsOn(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

napi_status CheckGattsOff(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};  // argv[PARAM1] is not used.
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);

    std::string type {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));

    auto gattServer = NapiGetGattServer(env, thisVar);
    NAPI_BT_RETURN_IF(gattServer == nullptr, "gattServer is nullptr.", napi_invalid_arg);
    // callback_ need unref before, see napi_bluetooth_gatt_client
    auto &gattServerCallback = gattServer->GetCallback();
    gattServerCallback.SetCallbackInfo(type, nullptr);
    HILOGI("%{public}s is removed", type.c_str());
    return napi_ok;
}

napi_value NapiGattServer::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckGattsOff(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
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

    // Build GattService
    using Permission = GattCharacteristic::Permission;
    using Propertie = GattCharacteristic::Propertie;
    int permissions = Permission::READABLE | Permission::WRITEABLE;
    int properties = Propertie::NOTIFY | Propertie::READ | Propertie::WRITE;
    GattServiceType type = napiGattService.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;

    outService = std::make_unique<GattService>(napiGattService.serviceUuid, type);
    for (const auto &napiCharacter : napiGattService.characteristics) {
        GattCharacteristic character(napiCharacter.characteristicUuid, permissions, properties);
        character.SetValue(napiCharacter.characteristicValue.data(), napiCharacter.characteristicValue.size());

        for (const auto &napiDescriptor : napiCharacter.descriptors) {
            GattDescriptor descriptor(napiDescriptor.descriptorUuid, permissions);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckGattsClose(napi_env env, napi_callback_info info, std::shared_ptr<GattServer> &outServer)
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
    auto status = CheckGattsClose(env, info, server);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    int ret = server->Close();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_SUCCESS, ret);
    return NapiGetUndefinedRet(env);
}

static napi_status CheckGattsRemoveService(napi_env env, napi_callback_info info,
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
    auto status = CheckGattsRemoveService(env, info, server, serviceUuid);
    NAPI_BT_ASSERT_RETURN_FALSE(env, (status == napi_ok && server != nullptr), BT_ERR_INVALID_PARAM);

    int ret = BT_ERR_INTERNAL_ERROR;
    auto primaryService = server->GetService(serviceUuid, true);
    if (primaryService.has_value()) {
        ret = server->RemoveGattService(*primaryService);
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    }
    auto secondService = server->GetService(serviceUuid, false);
    if (secondService.has_value()) {
        ret = server->RemoveGattService(*secondService);
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    }
    NAPI_BT_ASSERT_RETURN_FALSE(env, (primaryService.has_value() || secondService.has_value()), BT_ERR_INVALID_PARAM);
    return NapiGetBooleanRet(env, ret == BT_SUCCESS);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}

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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}
} // namespace Bluetooth
} // namespace OHOS
