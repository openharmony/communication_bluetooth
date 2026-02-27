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
#define LOG_TAG "bt_napi_socket_spp_client"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "datetime_ex.h"
#include "napi_bluetooth_spp_client.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_native_object.h"
#include "securec.h"
#include <limits>
#include <unistd.h>
#include <uv.h>
#include "parser/napi_parser_utils.h"
#ifdef BLUETOOTH_KIA_ENABLE
#include "cJSON.h"
#include "ipc_skeleton.h"
#include "sg_collect_client.h"
#endif

namespace OHOS {
namespace Bluetooth {
std::map<int, std::shared_ptr<NapiSppClient>> NapiSppClient::clientMap;
int NapiSppClient::count = 0;

static napi_status CheckSppConnectParams(
    napi_env env, napi_callback_info info, std::string &deviceId, SppConnectCallbackInfo *callbackInfo)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_THREE && argc != ARGS_SIZE_THREE - CALLBACK_SIZE),
        "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, deviceId, argv[PARAM0]),
        "Wrong argument type. String expected.", napi_invalid_arg);

    callbackInfo->env_ = env;
    callbackInfo->sppOption_ = GetSppOptionFromJS(env, argv[PARAM1]);
    NAPI_BT_RETURN_IF((callbackInfo->sppOption_ == nullptr), "GetSppOptionFromJS failed.", napi_invalid_arg);
    NAPI_BT_RETURN_IF((
        (callbackInfo->sppOption_->type_ == TYPE_L2CAP || callbackInfo->sppOption_->type_ == TYPE_L2CAP_LE) &&
        callbackInfo->sppOption_->psm_ <= 0), "GetSppOptionFromJS failed", napi_invalid_arg);
    callbackInfo->deviceId_ = deviceId;

    napi_value promise = nullptr;

    if (argc == ARGS_SIZE_THREE) {
        // Callback mode
        HILOGI("callback mode");
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM2], &valueType);
        if (valueType != napi_function) {
            HILOGE("Wrong argument type. Function expected.");
            return napi_invalid_arg;
        }
        napi_create_reference(env, argv[PARAM2], 1, &callbackInfo->callback_);
        napi_get_undefined(env, &promise);
    } else {
        // Promise mode
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

std::shared_ptr<SppOption> GetSppOptionFromJS(napi_env env, napi_value object)
{
    std::shared_ptr<SppOption> sppOption = std::make_shared<SppOption>();
    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;

    napi_create_string_utf8(env, "uuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    bool isSuccess = ParseString(env, sppOption->uuid_, value);
    if (sppOption->uuid_ != "" && (!isSuccess || (!IsValidUuid(sppOption->uuid_)))) {
        HILOGE("Parse UUID faild.");
        return nullptr;
    }
    HILOGI("uuid is %{public}s", sppOption->uuid_.c_str());

    napi_create_string_utf8(env, "secure", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseBool(env, sppOption->secure_, value);
    HILOGI("secure is %{public}d", sppOption->secure_);

    int type = 0;
    napi_create_string_utf8(env, "type", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, type, value);
    sppOption->type_ = BtSocketType(type);
    HILOGI("uuid: %{public}s, secure: %{public}d, type: %{public}d",
        sppOption->uuid_.c_str(), sppOption->secure_, sppOption->type_);
    if (sppOption->type_ == TYPE_RFCOMM && sppOption->uuid_ == "") {
        HILOGE("RFCOMM type but uuid is nullptr");
        return nullptr;
    }
    int psm = -1;
    napi_create_string_utf8(env, "psm", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, psm, value);
    sppOption->psm_ = psm;
    HILOGI("uuid: %{public}s, secure: %{public}d, type: %{public}d, psm: %{public}d",
        sppOption->uuid_.c_str(), sppOption->secure_, sppOption->type_, sppOption->psm_);
    return sppOption;
}

void ExecuteSppConnect(napi_env env, SppConnectCallbackInfo* callbackInfo)
{
    callbackInfo->device_ = std::make_shared<BluetoothRemoteDevice>(callbackInfo->deviceId_, 0);
    callbackInfo->client_ = std::make_shared<ClientSocket>(*callbackInfo->device_,
        UUID::FromString(callbackInfo->sppOption_->uuid_),
        callbackInfo->sppOption_->type_, callbackInfo->sppOption_->secure_);

    callbackInfo->errorCode_ = callbackInfo->client_->Connect(callbackInfo->sppOption_->psm_);
    if (callbackInfo->errorCode_ == BtStatus::BT_SUCCESS) {
        HILOGI("SppConnect successfully");
        callbackInfo->errorCode_ = CODE_SUCCESS;
    } else if (callbackInfo->errorCode_ == BT_ERR_SPP_CONNECT_FAILED) {
        callbackInfo->errorCode_ = BT_ERR_INTERNAL_ERROR;
    } else {
        HILOGE("SppConnect failed");
    }
}

void HandleSppConnectResult(napi_env env, SppConnectCallbackInfo* callbackInfo)
{
    napi_value result[ARGS_SIZE_TWO] = {0};
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(env, &undefined);
    if (callbackInfo->errorCode_ == CODE_SUCCESS) {
        HILOGI("SppConnect execute back success");
        std::shared_ptr<NapiSppClient> client =  std::make_shared<NapiSppClient>();
        client->device_ = callbackInfo->device_;
        client->id_ = NapiSppClient::count++;
        napi_create_int32(env, client->id_, &result[PARAM1]);
        client->client_ = callbackInfo->client_;
        NapiSppClient::clientMap.insert(std::make_pair(client->id_, client));
        HILOGI("SppConnect execute back successfully");
    } else {
        napi_get_undefined(env, &result[PARAM1]);
        HILOGI("SppConnect execute back failed");
    }

    if (callbackInfo->callback_) {
        // Callback mode
        HILOGI("SppConnect execute Callback mode");
        result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
        napi_get_reference_value(env, callbackInfo->callback_, &callback);
        napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
        napi_delete_reference(env, callbackInfo->callback_);
    } else {
        if (callbackInfo->errorCode_ == CODE_SUCCESS) {
            // Promise mode
            napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
        } else {
            HILOGI("SppConnect execute Promise mode failed");
            napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
        }
    }
    napi_delete_async_work(env, callbackInfo->asyncWork_);
    delete callbackInfo;
    callbackInfo = nullptr;
}

napi_value NapiSppClient::SppConnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string deviceId;
    SppConnectCallbackInfo *callbackInfo = new SppConnectCallbackInfo();
    auto status = CheckSppConnectParams(env, info, deviceId, callbackInfo);
    if (status != napi_ok) {
        delete callbackInfo;
        callbackInfo = nullptr;
    }
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SppConnect", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("SppConnect execute");
            ExecuteSppConnect(env, static_cast<SppConnectCallbackInfo*>(data));
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("SppConnect execute back");
            HandleSppConnectResult(env, static_cast<SppConnectCallbackInfo*>(data));
        },
        static_cast<void*>(callbackInfo), &callbackInfo->asyncWork_);
    if (napi_queue_async_work(env, callbackInfo->asyncWork_) != napi_ok) {
        HILOGE("SppConnect napi_queue_async_work failed");
        delete callbackInfo;
        callbackInfo = nullptr;
    }
    return NapiGetUndefinedRet(env);
}

static napi_status CheckSppCloseClientSocketParams(napi_env env, napi_callback_info info, int &id)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_ONE), "Requires 1 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM0]), "Wrong argument type. int expected.", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiSppClient::SppCloseClientSocket(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<NapiSppClient> client = nullptr;
    int id =  -1;
    bool isOK = false;
    auto status = CheckSppCloseClientSocketParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    if (clientMap.count(id) == KEY_FOUND && clientMap[id] != nullptr) {
        client = clientMap[id];
    } else {
        HILOGE("no such key in map.");
        return NapiGetUndefinedRet(env);
    }

    if (client->client_) {
        client->client_->Close();
        isOK = true;
    }
    clientMap.erase(id);
    NAPI_BT_RETURN_IF(napi_release_threadsafe_function(client->sppReadThreadSafeFunc_, napi_tsfn_abort),
        "inner error", nullptr);
    return NapiGetBooleanRet(env, isOK);
}

static napi_status CheckSppWriteParams(
    napi_env env, napi_callback_info info, int &id, uint8_t** totalBuf, size_t &totalSize)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_TWO), "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM0]), "Wrong argument type. int expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseArrayBuffer(env, totalBuf, totalSize, argv[PARAM1]),
        "ParseArrayBuffer failed.", napi_invalid_arg);
    return napi_ok;
}

#ifdef BLUETOOTH_KIA_ENABLE
static void ReportRefuseInfo(int32_t pid, int64_t refuseTime)
{
    cJSON *outJson = cJSON_CreateObject();
    if (outJson == nullptr) {
        HILOGE("ReportRefuseInfo json object created error");
        return;
    }
    cJSON_AddNumberToObject(outJson, "timestamp", refuseTime);
    cJSON_AddStringToObject(outJson, "type", "bluetooth_send");
    cJSON_AddStringToObject(outJson, "ftype", "1");
    cJSON_AddNumberToObject(outJson, "process_pid", pid);
    char *jsonContent = cJSON_PrintUnformatted(outJson);
    if (jsonContent == nullptr) {
        HILOGE("ReportRefuseInfo print json unformatted error");
        cJSON_Delete(outJson);
        outJson = nullptr;
        return;
    }
    std::string content = std::string(jsonContent);
    cJSON_free(jsonContent);
    cJSON_Delete(outJson);
    outJson = nullptr;
    std::shared_ptr<Security::SecurityGuard::EventInfo> eventInfo =
        std::make_shared<Security::SecurityGuard::EventInfo>(0x01C00000A, "1.0", content);
    int ret = Security::SecurityGuard::NativeDataCollectKit::ReportSecurityInfo(eventInfo);
    HILOGI("report pid:%{public}d, refuseTime:%{public}lu, ret:%{public}d", pid, refuseTime, ret);
}

static bool ShouldRefuseConnect(int32_t pid)
{
    constexpr int64_t msPerSecond = 1000;
    constexpr int64_t nsPerMs = 1000000;
    struct timespec times;
    if (clock_gettime(CLOCK_MONOTONIC, &times) < 0) {
        HILOGE("Failed clock_gettime:%{public}s, ShouldRefuseConnect:false", strerror(errno));
        return false;
    }
    int64_t bootTime = ((times.tv_sec * msPerSecond) + (times.tv_nsec / nsPerMs));
    HILOGI("bootTime:%{public}lu", bootTime);
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    auto prohibitedTime = host->GetRefusePolicyProhibitedTime();
    if (bootTime < prohibitedTime) {
        ReportRefuseInfo(pid, bootTime);
        HILOGI("ShouldRefuseConnect");
        return true;
    }
    return false;
}
#endif

napi_value NapiSppClient::SppWrite(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    uint8_t* totalBuf = nullptr;
    size_t totalSize = 0;
    bool isOK = false;
    int id = -1;

#ifdef BLUETOOTH_KIA_ENABLE
    if (ShouldRefuseConnect(IPCSkeleton::GetCallingPid())) {
        HILOGE("socket refuse because of Refuse Policy");
        NAPI_BT_ASSERT_RETURN_FALSE(env, false, BT_ERR_INVALID_PARAM);
    }
#endif

    auto status = CheckSppWriteParams(env, info, id, &totalBuf, totalSize);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_FALSE(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INTERNAL_ERROR);
    NAPI_BT_ASSERT_RETURN_FALSE(env, clientMap[id] != nullptr, BT_ERR_INTERNAL_ERROR);
    std::shared_ptr<OutputStream> outputStream = clientMap[id]->client_->GetOutputStream();
    while (totalSize) {
        int result = outputStream->Write(totalBuf, totalSize);
        NAPI_BT_ASSERT_RETURN_FALSE(env, result > 0, BT_ERR_SPP_IO);
        totalSize = totalSize - static_cast<size_t>(result);
        totalBuf += static_cast<size_t>(result);
        isOK = true;
    }
    return NapiGetBooleanRet(env, isOK);
}

static void NapiThreadSafeFuncCallJs(napi_env, napi_value jsCallback, void *context, void *data)
{
    if (jsCallback == nullptr) {
        HILOGE("delete safeFunc, jsCallback is nullptr");
        return;
    }
    BufferCallbackInfo *callbackInfo = static_cast<BufferCallbackInfo *>(data);
    std::shared_ptr<SppCallbackBuffer> buffer = callbackInfo->PopData();
    if (buffer == nullptr) {
        HILOGE("callbackInfo->PopData return nullptr");
        return;
    }
    if (buffer->len_ < 0 || buffer->len_ > L2CAP_SOCKET_BUFFER_SIZE) {
        HILOGE("buffer->len_ invalid");
        return;
    }

    napi_value result = nullptr;
    uint8_t *bufferData = nullptr;
    napi_create_arraybuffer(callbackInfo->env_, buffer->len_, (void **)&bufferData, &result);
    if (memcpy_s(bufferData, buffer->len_, buffer->data_.data(), buffer->len_) != EOK) {
        HILOGE("memcpy_s failed!");
        return;
    }

    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_call_function(callbackInfo->env_, undefined, jsCallback, ARGS_SIZE_ONE, &result, &callResult);
}

static napi_status NapiSppCreateThreadSafeFunc(const std::shared_ptr<NapiSppClient> &client)
{
    napi_value name;
    napi_threadsafe_function tsfn;
    const size_t maxQueueSize = 0;  // 0 means no limited
    const size_t initialThreadCount = 1;
    napi_value callback = nullptr;
    auto callbackInfo = client->callbackInfos_[REGISTER_SPP_READ_TYPE];
    NAPI_BT_CALL_RETURN(napi_create_string_utf8(callbackInfo->env_, "SppRead", NAPI_AUTO_LENGTH, &name));
    NAPI_BT_CALL_RETURN(napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback));
    NAPI_BT_CALL_RETURN(napi_create_threadsafe_function(callbackInfo->env_, callback, nullptr,
        name, maxQueueSize, initialThreadCount, nullptr, nullptr, nullptr, NapiThreadSafeFuncCallJs, &tsfn));
    if (client->sppReadThreadSafeFunc_ != nullptr) {
        NAPI_BT_RETURN_IF(napi_release_threadsafe_function(client->sppReadThreadSafeFunc_, napi_tsfn_abort),
            "inner error", napi_invalid_arg);
    }
    client->sppReadThreadSafeFunc_ = tsfn;
    return napi_ok;
}

napi_status CheckSppClientOn(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    int id = -1;
    std::string type;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_THREE), "Requires 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, type, argv[PARAM0]),
        "Wrong argument type. String expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(type != REGISTER_SPP_READ_TYPE, "Invalid type.", napi_invalid_arg);

    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = std::make_shared<BufferCallbackInfo>();
    callbackInfo->env_ = env;

    napi_valuetype valueType1 = napi_undefined;
    napi_valuetype valueType2 = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM1], &valueType1));
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM2], &valueType2));
    NAPI_BT_RETURN_IF(valueType1 != napi_number && valueType2 != napi_function,
        "Wrong argument type. Function expected.", napi_invalid_arg);

    napi_create_reference(env, argv[PARAM2], 1, &callbackInfo->callback_);

    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM1]), "Wrong argument type. Int expected.", napi_invalid_arg);

    NAPI_BT_RETURN_IF(NapiSppClient::clientMap.count(id) == KEY_NOT_FOUND, "client is nullptr.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(NapiSppClient::clientMap[id] == nullptr, "client is nullptr.", napi_invalid_arg);
    std::shared_ptr<NapiSppClient> client = NapiSppClient::clientMap[id];
    NAPI_BT_RETURN_IF(client->sppReadFlag, "client is reading... please off first", napi_invalid_arg);
    client->sppReadFlag = true;
    client->callbackInfos_[type] = callbackInfo;
    NAPI_BT_RETURN_IF(NapiSppCreateThreadSafeFunc(client) != napi_ok, "inner error", napi_invalid_arg);
    HILOGI("sppRead begin");
    client->thread_ = std::make_shared<std::thread>(NapiSppClient::SppRead, id);
    client->thread_->detach();
    return napi_ok;
}

napi_value NapiSppClient::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckSppClientOn(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

napi_status CheckSppClientOff(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    int id = -1;
    std::string type;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(
        (argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE), "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, type, argv[PARAM0]),
                      "Wrong argument type. String expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(type != REGISTER_SPP_READ_TYPE, "Invalid type.", napi_invalid_arg);

    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM1]), "Wrong argument type. Int expected.", napi_invalid_arg);
 
    NAPI_BT_RETURN_IF(NapiSppClient::clientMap.count(id) == KEY_NOT_FOUND, "client is nullptr.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(NapiSppClient::clientMap[id] == nullptr, "client is nullptr.", napi_invalid_arg);
    std::shared_ptr<NapiSppClient> client = NapiSppClient::clientMap[id];
    NAPI_BT_RETURN_IF(napi_release_threadsafe_function(client->sppReadThreadSafeFunc_, napi_tsfn_abort),
        "innner error",
        napi_invalid_arg);
    client->sppReadThreadSafeFunc_ = nullptr;
    client->callbackInfos_[type] = nullptr;
    client->sppReadFlag = false;
    return napi_ok;
}

napi_value NapiSppClient::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckSppClientOff(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

void NapiSppClient::SppRead(int id)
{
    if (clientMap.count(id) == KEY_NOT_FOUND) {
        HILOGE("thread start failed.");
        return;
    }
    auto client = clientMap[id];
    if (client == nullptr || !client->sppReadFlag || client->callbackInfos_[REGISTER_SPP_READ_TYPE] == nullptr) {
        HILOGE("thread start failed.");
        return;
    }
    std::shared_ptr<InputStream> inputStream = client->client_->GetInputStream();
    BtSocketType type = client->client_->GetType();
    std::vector<uint8_t> buf;
    if (type == TYPE_RFCOMM) {
        buf = std::vector<uint8_t>(RFCOMM_SOCKET_BUFFER_SIZE);
    } else {
        uint32_t bufferSize = client->client_->GetMaxReceivePacketSize();
        buf = std::vector<uint8_t>(bufferSize);
    }

    while (true) {
        HILOGI("thread start.");
        std::fill(buf.begin(), buf.end(), 0);
        HILOGI("inputStream.Read start");
        int ret = inputStream->Read(buf.data(), buf.size());
        HILOGI("inputStream.Read end");
        if (ret <= 0) {
            HILOGI("inputStream.Read failed, ret = %{public}d", ret);
            return;
        } else {
            HILOGI("callback read data to jshap begin");
            if (client == nullptr || !client->sppReadFlag || !client->callbackInfos_[REGISTER_SPP_READ_TYPE]) {
                HILOGE("failed");
                return;
            }
            std::shared_ptr<BufferCallbackInfo> callbackInfo =
                std::static_pointer_cast<BufferCallbackInfo>(client->callbackInfos_[REGISTER_SPP_READ_TYPE]);
            if (callbackInfo == nullptr) {
                HILOGE("callbackInfo nullptr");
                return;
            }

            std::shared_ptr<SppCallbackBuffer> buffer = std::make_shared<SppCallbackBuffer>(ret);
            if (memcpy_s(buffer->data_.data(), buffer->len_, buf.data(), ret) != EOK) {
                HILOGE("memcpy_s failed!");
                return;
            }
            callbackInfo->PushData(buffer);

            auto status = napi_acquire_threadsafe_function(client->sppReadThreadSafeFunc_);
            if (status != napi_ok) {
                HILOGE("napi_acquire_threadsafe_function failed, status: %{public}d", status);
                return;
            }

            status = napi_call_threadsafe_function(
                client->sppReadThreadSafeFunc_, static_cast<void *>(callbackInfo.get()), napi_tsfn_blocking);
            if (status != napi_ok) {
                HILOGE("napi_call_threadsafe_function failed, status: %{public}d", status);
                return;
            }

            status = napi_release_threadsafe_function(client->sppReadThreadSafeFunc_, napi_tsfn_release);
            if (status != napi_ok) {
                HILOGE("napi_release_threadsafe_function failed, status: %{public}d", status);
                return;
            }
        }
    }
    return;
}

static int WriteDataLoop(std::shared_ptr<OutputStream> outputStream, uint8_t* totalBuf, size_t totalSize)
{
    while (totalSize) {
        int result = outputStream->Write(totalBuf, totalSize);
        if (result <= 0) {
            HILOGE("Write failed");
            return BT_ERR_SPP_IO;
        }
        totalSize = totalSize - static_cast<size_t>(result);
        totalBuf += static_cast<size_t>(result);
    }
    return BT_NO_ERROR;
}

napi_value NapiSppClient::SppWriteAsync(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    uint8_t* totalBuf = nullptr;
    size_t totalSize = 0;
    int id = -1;

#ifdef BLUETOOTH_KIA_ENABLE
    if (ShouldRefuseConnect(IPCSkeleton::GetCallingPid())) {
        HILOGE("socket refuse because of Refuse Policy");
        NAPI_BT_ASSERT_RETURN_FALSE(env, false, BT_ERR_INVALID_PARAM);
    }
#endif

    auto status = CheckSppWriteParams(env, info, id, &totalBuf, totalSize);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INTERNAL_ERROR);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap[id] != nullptr, BT_ERR_INTERNAL_ERROR);
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client != nullptr, BT_ERR_INVALID_PARAM);
    std::shared_ptr<OutputStream> outputStream = client->client_->GetOutputStream();

    auto func = [outputStream, totalBuf, totalSize]() {
        int err = 0;
        err = WriteDataLoop(outputStream, totalBuf, totalSize);
        return NapiAsyncWorkRet(err);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status CheckSppReadParams(napi_env env, napi_callback_info info, int &id)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_ONE), "Requires 1 arguments.", napi_invalid_arg);
    napi_valuetype valueType = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM0], &valueType));
    NAPI_BT_RETURN_IF(valueType != napi_number, "Wrong argument type. Function expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM0]), "Wrong argument type. int expected.", napi_invalid_arg);

    return napi_ok;
}

static int ReadData(std::shared_ptr<InputStream> inputStream, int bufSize, SppCallbackBuffer &sppBuffer)
{
    (void)memset_s(sppBuffer.data_.data(), bufSize, 0, bufSize);
    int result = inputStream->Read(reinterpret_cast<uint8_t*>(sppBuffer.data_.data()), bufSize);
    if (result <= 0) {
        HILOGE("Read faild");
        return BT_ERR_SPP_IO;
    }
    sppBuffer.len_ = result;
    return BT_NO_ERROR;
}

napi_value NapiSppClient::SppReadAsync(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int id = -1;
    auto status = CheckSppReadParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap[id] != nullptr, BT_ERR_INVALID_PARAM);
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, !client->sppReadFlag, BT_ERR_INVALID_PARAM);
    client->sppReadFlag = true;
    std::shared_ptr<InputStream> inputStream = client->client_->GetInputStream();
    BtSocketType type = client->client_->GetType();
    uint32_t bufferSize = client->client_->GetMaxReceivePacketSize();
    auto func = [inputStream, id, type, bufferSize] {
        int err = 0;
        SppCallbackBuffer buffer;
        if (type == TYPE_RFCOMM) {
            buffer = SppCallbackBuffer(RFCOMM_SOCKET_BUFFER_SIZE);
            err = ReadData(inputStream, RFCOMM_SOCKET_BUFFER_SIZE, buffer);
        } else {
            buffer = SppCallbackBuffer(bufferSize);
            err = ReadData(inputStream, bufferSize, buffer);
        }
        auto object = std::make_shared<NapiNativeArrayBuffer>(buffer);
        if (clientMap.count(id) == KEY_NOT_FOUND || clientMap[id] == nullptr) {
            HILOGI("client is nullptr");
            return NapiAsyncWorkRet(BT_ERR_SPP_IO, object);
        }
        auto client = clientMap[id];
        client->sppReadFlag = false;
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

static napi_status CheckSppFdParams(napi_env env, napi_callback_info info, int &id)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseInt32(env, argv[PARAM0], id));
    return napi_ok;
}

napi_value NapiSppClient::GetDeviceId(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int id = -1;
    auto status = CheckSppFdParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap[id] != nullptr, BT_ERR_INVALID_PARAM);
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->client_ != nullptr, BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice = client->client_->GetRemoteDevice();
    std::string addr = remoteDevice.GetDeviceAddr();
    napi_value result = nullptr;
    napi_create_string_utf8(env, addr.c_str(), addr.size(), &result);
    return result;
}

napi_value NapiSppClient::IsConnected(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    bool isConnected = false;
    int id = -1;
    auto status = CheckSppFdParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    napi_value result = nullptr;
    if (clientMap.count(id) == KEY_NOT_FOUND) {
        napi_get_boolean(env, isConnected, &result);
        return result;
    }
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->client_ != nullptr, BT_ERR_INVALID_PARAM);
    isConnected = client->client_->IsConnected();
    napi_get_boolean(env, isConnected, &result);
    return result;
}

napi_value NapiSppClient::GetMaxReceiveDataSize(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int id = -1;
    auto status = CheckSppFdParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap[id] != nullptr, BT_ERR_INVALID_PARAM);
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->client_ != nullptr, BT_ERR_INVALID_PARAM);
    int32_t maxReceiveDataSize = static_cast<int32_t>(client->client_->GetMaxReceivePacketSize());
    napi_value result = nullptr;
    napi_create_int32(env, maxReceiveDataSize, &result);
    return result;
}

napi_value NapiSppClient::GetMaxTransmitDataSize(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int id = -1;
    auto status = CheckSppFdParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap.count(id) == KEY_FOUND, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, clientMap[id] != nullptr, BT_ERR_INVALID_PARAM);
    auto client = clientMap[id];
    NAPI_BT_ASSERT_RETURN_UNDEF(env, client->client_ != nullptr, BT_ERR_INVALID_PARAM);
    int32_t maxTransmitDataSize = static_cast<int32_t>(client->client_->GetMaxTransmitPacketSize());
    napi_value result = nullptr;
    napi_create_int32(env, maxTransmitDataSize, &result);
    return result;
}
} // namespace Bluetooth
} // namespace OHOS
