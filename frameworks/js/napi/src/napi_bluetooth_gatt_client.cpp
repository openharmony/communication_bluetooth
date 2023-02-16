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
#include "napi_bluetooth_gatt_client.h"
#include <unistd.h>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_host.h"
#include "napi_bluetooth_event.h"
#include "parser/napi_parser_utils.h"


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
        DECLARE_NAPI_FUNCTION("writeCharacteristicValue", WriteCharacteristicValue),
        DECLARE_NAPI_FUNCTION("writeDescriptorValue", WriteDescriptorValue),
        DECLARE_NAPI_FUNCTION("getRssiValue", GetRssiValue),
        DECLARE_NAPI_FUNCTION("setBLEMtuSize", SetBLEMtuSize),
        DECLARE_NAPI_FUNCTION("setNotifyCharacteristicChanged", SetNotifyCharacteristicChanged),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
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

    napi_wrap(
        env, thisVar, gattClient,
        [](napi_env env, void* data, void* hint) {
            NapiGattClient* client = (NapiGattClient*)data;
            if (client) {
                delete client;
                client = nullptr;
            }
        },
        nullptr,
        nullptr);

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

static GattCharacteristic *GetGattcCharacteristic(const std::shared_ptr<GattClient> &client,
    const NapiBleCharacteristic &napiCharacter)
{
    GattCharacteristic *character = GetCharacteristic(client, napiCharacter.serviceUuid,
        napiCharacter.characteristicUuid);
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

napi_status CheckGattClientOn(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);

    std::string type {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo {nullptr};

    if (type == STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE) {
        callbackInfo = std::make_shared<GattCharacteristicCallbackInfo>();
    } else {
        callbackInfo = std::make_shared<BluetoothCallbackInfo>();
    }
    callbackInfo->env_ = env;

    auto gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
    std::unique_lock<std::shared_mutex> guard(NapiGattClientCallback::g_gattClientCallbackInfosMutex);
    NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_));
    gattClient->GetCallback().SetCallbackInfo(type, callbackInfo);
    HILOGI("%{public}s is registered", type.c_str());
    return napi_ok;
}

napi_value NapiGattClient::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckGattClientOn(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

napi_status CheckGattClientOff(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};  // argv[PARAM1] is not used.
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments.", napi_invalid_arg);

    std::string type {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));

    auto gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);
    // callback_ need unref before, see napi_bluetooth_gatt_client
    std::unique_lock<std::shared_mutex> guard(NapiGattClientCallback::g_gattClientCallbackInfosMutex);
    auto &gattClientCallback = gattClient->GetCallback();
    uint32_t refCount = INVALID_REF_COUNT;
    NAPI_BT_CALL_RETURN(napi_reference_unref(env, gattClientCallback.GetCallbackInfo(type)->callback_, &refCount));
    HILOGI("decrements the refernce count, refCount: %{public}d", refCount);
    if (refCount == 0) {
        NAPI_BT_CALL_RETURN(napi_delete_reference(env, gattClientCallback.GetCallbackInfo(type)->callback_));
    }
    gattClientCallback.SetCallbackInfo(type, nullptr);
    HILOGI("%{public}s is removed", type.c_str());
    return napi_ok;
}

napi_value NapiGattClient::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckGattClientOff(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
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

    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client->Connect(gattClient->GetCallback(), true, GATT_TRANSPORT_TYPE_LE);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status ParseGattClientReadCharacteristicValue(napi_env env, napi_callback_info info,
    ReadCharacteristicValueCallbackInfo **outCallbackInfo, napi_value &promise)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 1 or 2 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    ReadCharacteristicValueCallbackInfo* callbackInfo = new ReadCharacteristicValueCallbackInfo();
    gattClient->readCharacteristicValueCallbackInfo_ = callbackInfo;
    callbackInfo->env_ = env;
    callbackInfo->client_ = gattClient->GetClient();

    NapiBleCharacteristic napiCharacter;
    NAPI_BT_CALL_RETURN(NapiParseGattCharacteristic(env, argv[PARAM0], napiCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), napiCharacter);
    NAPI_BT_RETURN_IF(character == nullptr, "Not found character", napi_invalid_arg);
    callbackInfo->inputCharacteristic_ = character;

    *outCallbackInfo = callbackInfo;

    if (argc == expectedArgsCount) {
        // Callback mode
        HILOGI("callback mode");
        NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
        NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_));
        napi_get_undefined(env, &promise);
    } else {
        // Promise mode
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

napi_value NapiGattClient::ReadCharacteristicValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    ReadCharacteristicValueCallbackInfo *callbackInfo = nullptr;
    napi_value promise = nullptr;

    auto status = ParseGattClientReadCharacteristicValue(env, info, &callbackInfo, promise);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "readCharacteristicValue", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            HILOGI("ReadCharacteristicValue(): execute");
            ReadCharacteristicValueCallbackInfo* callbackInfo = (ReadCharacteristicValueCallbackInfo*)data;
            callbackInfo->asyncState_ = ASYNC_START;
            int status = -1;
            if (callbackInfo->inputCharacteristic_ != nullptr) {
                HILOGI("ReadCharacteristicValue(): Client read characteristic");
                status = callbackInfo->client_->ReadCharacteristic(*(callbackInfo->inputCharacteristic_));
            }

            if (status == BT_SUCCESS) {
                HILOGI("ReadCharacteristicValue(): successful");
                callbackInfo->errorCode_ = CODE_SUCCESS;
                int tryTime = 100;
                while (callbackInfo->asyncState_ == ASYNC_START && tryTime > 0) {
                    usleep(SLEEP_TIME);
                    if (callbackInfo->asyncState_ == ASYNC_DONE) {
                        break;
                    }
                    tryTime--;
                }
                if (callbackInfo->asyncState_ != ASYNC_DONE) {
                    HILOGE("ReadCharacteristicValue(): failed, async not done");
                    callbackInfo->errorCode_ = CODE_FAILED;
                }
            } else {
                HILOGE("ReadCharacteristicValue(): failed, status: %{public}d", status);
                callbackInfo->errorCode_ = CODE_FAILED;
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOGI("ReadCharacteristicValue(): execute back");
            ReadCharacteristicValueCallbackInfo* callbackInfo = (ReadCharacteristicValueCallbackInfo*)data;
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);

            HILOGI("ReadCharacteristicValue(): errorCode: %{public}d", callbackInfo->errorCode_);
            if (callbackInfo->errorCode_ == BT_SUCCESS) {
                napi_create_object(env, &result[PARAM1]);
                ConvertBLECharacteristicToJS(env, result[PARAM1],
                    const_cast<GattCharacteristic&>(*(callbackInfo->outputCharacteristic_)));
            } else {
                napi_get_undefined(env, &result[PARAM1]);
            }

            if (callbackInfo->callback_) {
                // Callback mode
                result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
                napi_get_reference_value(env, callbackInfo->callback_, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
                napi_delete_reference(env, callbackInfo->callback_);
            } else {
                if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                // Promise mode
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                }
            }
            napi_delete_async_work(env, callbackInfo->asyncWork_);
            delete callbackInfo;
            callbackInfo = nullptr;
        },
        (void*)callbackInfo,
        &callbackInfo->asyncWork_);
    napi_queue_async_work(env, callbackInfo->asyncWork_);
    return promise;
}

static napi_status ParseGattClientReadDescriptorValue(napi_env env, napi_callback_info info,
    ReadDescriptorValueCallbackInfo **outCallbackInfo, napi_value &promise)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 1 or 2 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);

    ReadDescriptorValueCallbackInfo *callbackInfo = new ReadDescriptorValueCallbackInfo();
    gattClient->readDescriptorValueCallbackInfo_ = callbackInfo;
    callbackInfo->env_ = env;
    callbackInfo->client_ = gattClient->GetClient();

    NapiBleDescriptor napiDescriptor;
    NAPI_BT_CALL_RETURN(NapiParseGattDescriptor(env, argv[PARAM0], napiDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), napiDescriptor);
    NAPI_BT_RETURN_IF(descriptor == nullptr, "Not found Descriptor", napi_invalid_arg);
    callbackInfo->inputDescriptor_ = descriptor;

    *outCallbackInfo = callbackInfo;

    if (argc == expectedArgsCount) {
        // Callback mode
        HILOGI("callback mode");
        NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
        NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_));
        napi_get_undefined(env, &promise);
    } else {
        // Promise mode
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

napi_value NapiGattClient::ReadDescriptorValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    ReadDescriptorValueCallbackInfo *callbackInfo = nullptr;
    napi_value promise = nullptr;

    auto status = ParseGattClientReadDescriptorValue(env, info, &callbackInfo, promise);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "readDescriptorValue", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("ReadDescriptorValue(): execute");
            ReadDescriptorValueCallbackInfo* callbackInfo = (ReadDescriptorValueCallbackInfo*)data;
            callbackInfo->asyncState_ = ASYNC_START;
            int status = -1;
            if (callbackInfo->inputDescriptor_ != nullptr) {
                HILOGI("ReadDescriptorValue(): Client read descriptor");
                status = callbackInfo->client_->ReadDescriptor(*(callbackInfo->inputDescriptor_));
            } else {
                HILOGI("callbackInfo->inputDescriptor_ is nullptr");
            }

            if (status == GattStatus::GATT_SUCCESS) {
                HILOGI("ReadDescriptorValue(): successful");
                callbackInfo->errorCode_ = CODE_SUCCESS;
                int tryTime = 100;
                while (callbackInfo->asyncState_ == ASYNC_START && tryTime > 0) {
                    usleep(SLEEP_TIME);
                    if (callbackInfo->asyncState_ == ASYNC_DONE) {
                        break;
                    }
                    tryTime--;
                }
                if (callbackInfo->asyncState_ != ASYNC_DONE) {
                    HILOGE("ReadDescriptorValue(): failed, async not done");
                    callbackInfo->errorCode_ = CODE_FAILED;
                }
            } else {
                HILOGE("ReadDescriptorValue(): failed, status: %{public}d", status);
                callbackInfo->errorCode_ = CODE_FAILED;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("ReadDescriptorValue(): execute back");
            ReadDescriptorValueCallbackInfo* callbackInfo = (ReadDescriptorValueCallbackInfo*)data;
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);

            HILOGI("ReadDescriptorValue(): errorCode: %{public}d", callbackInfo->errorCode_);
            if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                napi_create_object(env, &result[PARAM1]);
                ConvertBLEDescriptorToJS(env, result[PARAM1],
                    const_cast<GattDescriptor&>(*(callbackInfo->outputDescriptor_)));
            } else {
                napi_get_undefined(env, &result[PARAM1]);
            }

            if (callbackInfo->callback_) {
                // Callback mode
                result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
                napi_get_reference_value(env, callbackInfo->callback_, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
                napi_delete_reference(env, callbackInfo->callback_);
            } else {
                if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                // Promise mode
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                }
            }
            napi_delete_async_work(env, callbackInfo->asyncWork_);
            delete callbackInfo;
            callbackInfo = nullptr;
        },
        (void*)callbackInfo,
        &callbackInfo->asyncWork_);

    napi_queue_async_work(env, callbackInfo->asyncWork_);
    return promise;
}

static napi_status ParseGattClientAsyncCallback(napi_env env, napi_callback_info info,
    GetServiceCallbackInfo *callbackInfo, napi_value &promise)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount && argc != expectedArgsCount - CALLBACK_SIZE,
        "Requires 0 or 1 arguments.", napi_invalid_arg);
    NapiGattClient *gattClient = NapiGetGattClient(env, thisVar);
    NAPI_BT_RETURN_IF(gattClient == nullptr, "gattClient is nullptr.", napi_invalid_arg);
    callbackInfo->client_ = gattClient->GetClient();

    if (argc == expectedArgsCount) {
        // Callback mode
        HILOGI("callback mode");
        NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM0]));
        napi_create_reference(env, argv[PARAM0], 1, &callbackInfo->callback_);
        napi_get_undefined(env, &promise);
    } else {
        // Promise mode
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

napi_value NapiGattClient::GetServices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    GetServiceCallbackInfo *callbackInfo = new GetServiceCallbackInfo();
    callbackInfo->env_ = env;
    napi_value promise = nullptr;

    auto status = ParseGattClientAsyncCallback(env, info, callbackInfo, promise);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "getServices", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("GetServices(): execute");
            GetServiceCallbackInfo* callbackInfo = (GetServiceCallbackInfo*)data;
            int status = callbackInfo->client_->DiscoverServices();
            if (status != BT_SUCCESS) {
                HILOGE("GetServices(): failed, status: %{public}d", status);
                callbackInfo->errorCode_ = CODE_FAILED;
            } else {
                HILOGI("GetServices(): successful");
                callbackInfo->services_ = callbackInfo->client_->GetService();
                callbackInfo->errorCode_ = CODE_SUCCESS;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("GetServices(): execute back");
            GetServiceCallbackInfo* callbackInfo = (GetServiceCallbackInfo*)data;
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);

            HILOGI("GetServices(): errorCode: %{public}d", callbackInfo->errorCode_);
            if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                napi_create_array(env, &result[PARAM1]);
                ConvertGattServiceVectorToJS(env, result[PARAM1], callbackInfo->services_);
            } else {
                napi_get_undefined(env, &result[PARAM1]);
            }

            if (callbackInfo->callback_) {
                // Callback mode
                result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
                napi_get_reference_value(env, callbackInfo->callback_, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
                napi_delete_reference(env, callbackInfo->callback_);
            } else {
                if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                // Promise mode
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                }
            }
            napi_delete_async_work(env, callbackInfo->asyncWork_);
            delete callbackInfo;
            callbackInfo = nullptr;
        },
        (void *)callbackInfo,
        &callbackInfo->asyncWork_);

    napi_queue_async_work(env, callbackInfo->asyncWork_);
    return promise;
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}

static napi_status CheckWriteCharacteristicValue(napi_env env, napi_callback_info info,
    GattCharacteristic **outCharacteristic, NapiGattClient **outGattClient)
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

    *outGattClient = gattClient;
    *outCharacteristic = character;

    return napi_ok;
}

napi_value NapiGattClient::WriteCharacteristicValue(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    GattCharacteristic* characteristic = nullptr;
    NapiGattClient* gattClient = nullptr;

    auto status = CheckWriteCharacteristicValue(env, info, &characteristic, &gattClient);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    std::shared_ptr<GattClient> client = gattClient->GetClient();
    NAPI_BT_ASSERT_RETURN_FALSE(env, client != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = client->WriteCharacteristic(*characteristic);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
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
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);
    return NapiGetBooleanTrue(env);
}
} // namespace Bluetooth
} // namespace OHOS
