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
#define LOG_TAG "bt_napi_gatt_client"
#endif

#include "napi_bluetooth_gatt_client.h"
#include <unistd.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "napi_async_callback.h"
#include "napi_bluetooth_ble_utils.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_host.h"
#include "napi_bluetooth_utils.h"
#include "napi_event_subscribe_module.h"
#include "../parser/napi_parser_utils.h"


namespace OHOS {
namespace Bluetooth {
using namespace std;

thread_local napi_ref NapiGattClient::consRef_ = nullptr;

static napi_status CheckCreateGattClientDeviceParams(napi_env env, napi_callback_info info, napi_value &outResult)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "expect 1 args", napi_invalid_arg);

    std::string deviceId {};
    if (!ParseString(env, deviceId, argv[0])) {
        HILOGE("expect string");
        return napi_string_expected;
    }
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
        return napi_invalid_arg;
    }

    napi_value constructor = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_reference_value(env, NapiGattClient::consRef_, &constructor));
    NAPI_BT_CALL_RETURN(napi_new_instance(env, constructor, argc, argv, &outResult));
    return napi_ok;
}

napi_value NapiGattClient::CreateGattClientDevice(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value result;
    auto status = CheckCreateGattClientDeviceParams(env, info, result);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return result;
}

void NapiGattClient::DefineGattClientJSClass(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("close", Close),
        DECLARE_NAPI_FUNCTION("getDeviceName", GetDeviceName),
        DECLARE_NAPI_FUNCTION("getServices", GetServices),
        DECLARE_NAPI_FUNCTION("readCharacteristicValue", ReadCharacteristicValue),
        DECLARE_NAPI_FUNCTION("readDescriptorValue", ReadDescriptorValue),
        DECLARE_NAPI_FUNCTION("getRssiValue", GetRssiValue),
        DECLARE_NAPI_FUNCTION("setBLEMtuSize", SetBLEMtuSize),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("writeCharacteristicValue", WriteCharacteristicValueEx),
        DECLARE_NAPI_FUNCTION("writeDescriptorValue", WriteDescriptorValueEx),
        DECLARE_NAPI_FUNCTION("setCharacteristicChangeNotification", setCharacteristicChangeNotification),
        DECLARE_NAPI_FUNCTION("setCharacteristicChangeIndication", setCharacteristicChangeIndication),
#else
        DECLARE_NAPI_FUNCTION("writeCharacteristicValue", WriteCharacteristicValue),
        DECLARE_NAPI_FUNCTION("writeDescriptorValue", WriteDescriptorValue),
        DECLARE_NAPI_FUNCTION("setNotifyCharacteristicChanged", SetNotifyCharacteristicChanged),
#endif
    };

    napi_value constructor = nullptr;
    napi_define_class(env, "GattClientDevice", NAPI_AUTO_LENGTH, GattClientConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &consRef_);
}

napi_value NapiGattClient::GattClientConstructor(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value thisVar = nullptr;

    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    string deviceId;
    ParseString(env, deviceId, argv[PARAM0]);
    SetGattClientDeviceId(deviceId);

    NapiGattClient *gattClient = new NapiGattClient(deviceId);

    auto status = napi_wrap(
        env, thisVar, gattClient,
        [](napi_env env, void* data, void* hint) {
            NapiGattClient* client = static_cast<NapiGattClient*>(data);
            if (client) {
                delete client;
                client = nullptr;
            }
        },
        nullptr,
        nullptr);
    if (status != napi_ok) {
        HILOGE("napi_wrap failed");
        delete gattClient;
        gattClient = nullptr;
    }

    return thisVar;
}

static NapiGattClient *NapiGetGattClient(napi_env env, napi_value thisVar)
{
    NapiGattClient *gattClient = nullptr;
    auto status = napi_unwrap(env, thisVar, (void **)&gattClient);
    if (status != napi_ok) {
        return nullptr;
    }
    return gattClient;
}

static NapiGattClient *NapiGetGattClient(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr) != napi_ok) {
        return nullptr;
    }
    return NapiGetGattClient(env, thisVar);
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const UUID &serviceUuid, const UUID &characterUuid)
{
    GattCharacteristic *character = nullptr;
    if (client) {
        auto service = client->GetService(serviceUuid);
        if (service.has_value()) {
            character = service->get().GetCharacteristic(characterUuid);
        }
    }
    return character;
}

static GattCharacteristic *FindCharacteristic(std::vector<GattService> &service,
    const NapiBleCharacteristic &napiCharacter)
{
    GattCharacteristic *character = nullptr;
    for (auto &svc : service) {
        if (svc.GetUuid().Equals(napiCharacter.serviceUuid)) {
            character = svc.GetCharacteristic(napiCharacter.characteristicValueHandle);
            if (character && character->GetUuid().Equals(napiCharacter.characteristicUuid)) {
                return character;
            }
        }
    }
    return nullptr;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const NapiBleCharacteristic &napiCharacter)
{
    if (client) {
        if (napiCharacter.characteristicValueHandle > 0) {
            std::vector<GattService> &services = client->GetService();
            return FindCharacteristic(services, napiCharacter);
        } else {
            GattCharacteristic *character = GetCharacteristic(client, napiCharacter.serviceUuid,
                napiCharacter.characteristicUuid);
            return character;
        }
    }
    return nullptr;
}

static GattCharacteristic *GetGattcCharacteristic(const std::shared_ptr<GattClient> &client,
    const NapiBleCharacteristic &napiCharacter)
{
    GattCharacteristic *character = GetCharacteristic(client, napiCharacter);
    if (character) {
        character->SetValue(napiCharacter.characteristicValue.data(), napiCharacter.characteristicValue.size());
    }
    return character;
}

static GattDescriptor *GetGattcDescriptor(const std::shared_ptr<GattClient> &client,
    const NapiBleDescriptor &napiDescriptor)
{
    GattDescriptor *descriptor = nullptr;
    if (client) {
        auto *character = GetCharacteristic(client, napiDescriptor.serviceUuid, napiDescriptor.characteristicUuid);
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return nullptr;
        }
        descriptor = character->GetDescriptor(napiDescriptor.descriptorUuid);
        if (descriptor) {
            descriptor->SetValue(napiDescriptor.descriptorValue.data(), napiDescriptor.descriptorValue.size());
        }
    }
    return descriptor;
}

napi_value NapiGattClient::On(napi_env env, napi_callback_info info)
{
    NapiGattClient *napiGattClient = NapiGetGattClient(env, info);
    if (napiGattClient && napiGattClient->GetCallback()) {
        auto status = napiGattClient->GetCallback()->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiGattClient::Off(napi_env env, napi_callback_info info)
{
    NapiGattClient *napiGattClient = NapiGetGattClient(env, info);
    if (napiGattClient && napiGattClient->GetCallback()) {
        auto status = napiGattClient->GetCallback()->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

static napi_status CheckGattClientNoArgc(napi_env env, napi_callback_info info, NapiGattClient **outGattClient)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != 0, "No need arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    *outGattClient = gattClient;
    return napi_ok;
}

napi_value NapiGattClient::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient *gattClient = nullptr;
    auto status = CheckGattClientNoArgc(env, info, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, gattClient->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->Connect(gattClient->GetCallback(), false, GATT_TRANSPORT_TYPE_LE);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value NapiGattClient::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient* gattClient = nullptr;
    auto status = CheckGattClientNoArgc(env, info, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->Disconnect();
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status ParseGattClientReadCharacteristicValue(napi_env env, napi_callback_info info,
    NapiGattClient **outGattClient, GattCharacteristic **outCharacter)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 1 or 2 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr || outCharacter == nullptr, "Not found character", napi_invalid_arg);

    *outGattClient = gattClient;
    *outCharacter = character;
    return napi_ok;
}

napi_value NapiGattClient::ReadCharacteristicValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient *client = nullptr;
    GattCharacteristic *character = nullptr;
    auto status = ParseGattClientReadCharacteristicValue(env, info, &client, &character);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && client && character, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client->GetClient(), character]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadCharacteristic(*character);
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(NapiAsyncType::GATT_CLIENT_READ_CHARACTER, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status ParseGattClientReadDescriptorValue(napi_env env, napi_callback_info info,
    NapiGattClient **outGattClient, GattDescriptor **outDescriptor)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 1 or 2 arguments.", napi_invalid_arg);

    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(outGattClient == nullptr || gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleDescriptor napiDescriptor;
    NAPI_BT_CALL_RETURN(NapiParseGattDescriptor(env, argv[PARAM0], napiDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), napiDescriptor);
    NAPI_BT_RETURN_IF(outDescriptor == nullptr || descriptor == nullptr, "Not found Descriptor", napi_invalid_arg);

    *outGattClient = gattClient;
    *outDescriptor = descriptor;
    return napi_ok;
}

napi_value NapiGattClient::ReadDescriptorValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient *client = nullptr;
    GattDescriptor *descriptor = nullptr;
    auto status = ParseGattClientReadDescriptorValue(env, info, &client, &descriptor);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && client && descriptor, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client->GetClient(), descriptor]() {
        if (descriptor == nullptr) {
            HILOGE("descriptor is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadDescriptor(*descriptor);
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(NapiAsyncType::GATT_CLIENT_READ_DESCRIPTOR, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status ParseGattClientGetServices(napi_env env, napi_callback_info info, NapiGattClient **outGattClient)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ZERO && argc != ARGS_SIZE_ONE, "Requires 0 or 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    *outGattClient = gattClient;
    return napi_ok;
}

napi_value NapiGattClient::GetServices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient *client = nullptr;
    auto status = ParseGattClientGetServices(env, info, &client);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && client, BT_ERR_INVALID_PARAM);

    auto func = [gattClient = client->GetClient()]() {
        if (gattClient == nullptr) {
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }

        HILOGI("start discover services");
        std::shared_ptr<NapiNativeObject> object {nullptr};
        int ret = gattClient->DiscoverServices();
        if (ret == BT_NO_ERROR) {
            HILOGI("start get services");
            object = std::make_shared<NapiNativeGattServiceArray>(gattClient->GetService());
        }
        return NapiAsyncWorkRet(ret, object);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiGattClient::Close(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient* gattClient = nullptr;
    auto status = CheckGattClientNoArgc(env, info, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->Close();
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckSetBLEMtuSize(napi_env env, napi_callback_info info,
    int32_t &mtuSize, NapiGattClient **outGattClient)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiParseInt32(env, argv[PARAM0], mtuSize));
    *outGattClient = gattClient;

    return napi_ok;
}

napi_value NapiGattClient::SetBLEMtuSize(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient* gattClient = nullptr;
    int32_t mtuSize = 0;

    auto status = CheckSetBLEMtuSize(env, info, mtuSize, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->RequestBleMtuSize(mtuSize);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status ParseGattClientReadRssiValue(napi_env env, napi_callback_info info, NapiGattClient **outGattClient)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 0 or 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(outGattClient == nullptr || gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);
    *outGattClient = gattClient;
    return napi_ok;
}

napi_value NapiGattClient::GetRssiValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient *napiGattClient = nullptr;
    auto status = ParseGattClientReadRssiValue(env, info, &napiGattClient);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && napiGattClient, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiGattClient->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = napiGattClient->GetClient()] {
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadRemoteRssiValue();
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = napiGattClient->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_READ_REMOTE_RSSI_VALUE, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status CheckGattClientGetDeviceName(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ZERO && argc != ARGS_SIZE_ONE, "Requires 0 or 1 arguments.", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiGattClient::GetDeviceName(napi_env env, napi_callback_info info)
{
    HILOGD("start");

    auto status = CheckGattClientGetDeviceName(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = []() {
        std::string deviceName = "";
        std::string deviceAddr = GetGattClientDeviceId();
        int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(
            deviceAddr, BT_TRANSPORT_BLE).GetDeviceName(deviceName);

        HILOGI("err: %{public}d", err);
        auto object = std::make_shared<NapiNativeString>(deviceName);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

#ifdef BLUETOOTH_API_SINCE_10

static napi_status CheckWriteCharacteristicValueEx(napi_env env, napi_callback_info info,
    GattCharacteristic **outCharacteristic, NapiGattClient **outGattClient, std::vector<uint8_t> &outValue)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr || outCharacteristic == nullptr, "Not found character", napi_invalid_arg);

    int writeType = GattCharacteristic::WriteType::DEFAULT;
    NAPI_BT_CALL_RETURN(NapiParseGattWriteType(env, argv[PARAM1], writeType));
    character->SetWriteType(writeType);

    outValue = std::move(napiCharacter.characteristicValue);
    *outGattClient = gattClient;
    *outCharacteristic = character;

    return napi_ok;
}

napi_value NapiGattClient::WriteCharacteristicValueEx(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    GattCharacteristic* character = nullptr;
    NapiGattClient* client = nullptr;

    std::vector<uint8_t> value {};
    auto status = CheckWriteCharacteristicValueEx(env, info, &character, &client, value);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok && character && client, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client->GetClient(), character]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->WriteCharacteristic(*character);
        }
        return NapiAsyncWorkRet(ret);
    };

    bool isNeedCallback = character->GetWriteType() == GattCharacteristic::WriteType::DEFAULT;
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(
        env, info, func, isNeedCallback ? ASYNC_WORK_NEED_CALLBACK : ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    // GattCharacteristic write need callback, write no response is not needed.
    if (isNeedCallback) {
        bool success = client->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_WRITE_CHARACTER, asyncWork);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);
    }

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status CheckWriteDescriptorValueEx(napi_env env, napi_callback_info info,
    GattDescriptor **outDescriptor, NapiGattClient **outGattClient)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleDescriptor napiDescriptor;
    NAPI_BT_CALL_RETURN(NapiParseGattDescriptor(env, argv[PARAM0], napiDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), napiDescriptor);
    NAPI_BT_RETURN_IF(descriptor == nullptr, "Not found Descriptor", napi_invalid_arg);

    *outGattClient = gattClient;
    *outDescriptor = descriptor;
    return napi_ok;
}

napi_value NapiGattClient::WriteDescriptorValueEx(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NapiGattClient* client = nullptr;
    GattDescriptor* descriptor = nullptr;
    auto status = CheckWriteDescriptorValueEx(env, info, &descriptor, &client);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && client && descriptor, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client->GetClient(), descriptor]() {
        if (descriptor == nullptr) {
            HILOGE("descriptor is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->WriteDescriptor(*descriptor);
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_WRITE_DESCRIPTOR, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status CheckSetCharacteristicChange(napi_env env, napi_callback_info info,
    GattCharacteristic **outCharacteristic, bool &enable, NapiGattClient **outGattClient)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr || outCharacteristic == nullptr, "Not found character", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiParseBoolean(env, argv[PARAM1], enable));
    *outGattClient = gattClient;
    *outCharacteristic = character;
    return napi_ok;
}

static napi_value setCharacteristicChangeInner(napi_env env, napi_callback_info info, bool isNotify)
{
    GattCharacteristic *character = nullptr;
    bool enable = false;
    NapiGattClient *client = nullptr;

    auto status = CheckSetCharacteristicChange(env, info, &character, enable, &client);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok && client && character, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->GetCallback() != nullptr, BT_ERR_INTERNAL_ERROR);

    auto func = [gattClient = client->GetClient(), character, enable, isNotify]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            if (isNotify) {
                ret = gattClient->SetNotifyCharacteristic(*character, enable);
            } else {
                ret = gattClient->SetIndicateCharacteristic(*character, enable);
            }
        }
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_ENABLE_CHARACTER_CHANGED, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiGattClient::setCharacteristicChangeNotification(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    return setCharacteristicChangeInner(env, info, true);
}

napi_value NapiGattClient::setCharacteristicChangeIndication(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    return setCharacteristicChangeInner(env, info, false);
}

#else  // ! BLUETOOTH_API_SINCE_10

static napi_status CheckWriteCharacteristicValue(napi_env env, napi_callback_info info,
    GattCharacteristic **outCharacteristic, NapiGattClient **outGattClient, std::vector<uint8_t> &outValue)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr, "Not found character", napi_invalid_arg);

    outValue = std::move(napiCharacter.characteristicValue);
    *outGattClient = gattClient;
    *outCharacteristic = character;

    return napi_ok;
}

napi_value NapiGattClient::WriteCharacteristicValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    GattCharacteristic* characteristic = nullptr;
    NapiGattClient* gattClient = nullptr;

    std::vector<uint8_t> value {};
    auto status = CheckWriteCharacteristicValue(env, info, &characteristic, &gattClient, value);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = client->WriteCharacteristic(*characteristic, std::move(value));
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckWriteDescriptorValue(napi_env env, napi_callback_info info,
    GattDescriptor **outDescriptor, NapiGattClient **outGattClient)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleDescriptor napiDescriptor;
    NAPI_BT_CALL_RETURN(NapiParseGattDescriptor(env, argv[PARAM0], napiDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), napiDescriptor);
    NAPI_BT_RETURN_IF(descriptor == nullptr, "Not found Descriptor", napi_invalid_arg);

    *outGattClient = gattClient;
    *outDescriptor = descriptor;
    return napi_ok;
}

napi_value NapiGattClient::WriteDescriptorValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    GattDescriptor* descriptor = nullptr;
    NapiGattClient* gattClient = nullptr;

    auto status = CheckWriteDescriptorValue(env, info, &descriptor, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->WriteDescriptor(*descriptor);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckSetNotifyCharacteristicChanged(napi_env env, napi_callback_info info,
    GattCharacteristic **outCharacteristic, bool &enableNotify, NapiGattClient **outGattClient)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 2 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr, "Not found character", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiParseBoolean(env, argv[PARAM1], enableNotify));
    *outGattClient = gattClient;
    *outCharacteristic = character;
    return napi_ok;
}

napi_value NapiGattClient::SetNotifyCharacteristicChanged(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    GattCharacteristic* characteristic = nullptr;
    bool enableNotify = false;
    NapiGattClient* gattClient = nullptr;

    auto status = CheckSetNotifyCharacteristicChanged(env, info, &characteristic, enableNotify, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->SetNotifyCharacteristic(*characteristic, enableNotify);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

#endif

} // namespace Bluetooth
} // namespace OHOS
