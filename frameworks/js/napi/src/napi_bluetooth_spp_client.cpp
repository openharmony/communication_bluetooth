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
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_spp_client.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "securec.h"
#include <limits>
#include <unistd.h>
#include <uv.h>

namespace OHOS {
namespace Bluetooth {
const int sleepTime = 5;
std::map<int, std::shared_ptr<NapiSppClient>> NapiSppClient::clientMap;
int NapiSppClient::count = 0;
const int SOCKET_BUFFER_SIZE = 1024;

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
    NAPI_BT_RETURN_IF((callbackInfo->sppOption_ == nullptr), "GetSppOptionFromJS faild.", napi_invalid_arg);
    callbackInfo->deviceId_ = deviceId;

    napi_value promise = nullptr;

    if (argc == ARGS_SIZE_THREE) {
        // Callback mode
        HILOGI("callback mode");
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM2], &valueType);
        if (valueType != napi_function) {
            HILOGE("Wrong argument type. Function expected.");
            delete callbackInfo;
            callbackInfo = nullptr;
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

napi_value NapiSppClient::SppConnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string deviceId;
    SppConnectCallbackInfo *callbackInfo = new SppConnectCallbackInfo();
    auto status = CheckSppConnectParams(env, info, deviceId, callbackInfo);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SppConnect", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("SppConnect execute");
            SppConnectCallbackInfo* callbackInfo = (SppConnectCallbackInfo*)data;
            callbackInfo->device_ = std::make_shared<BluetoothRemoteDevice>(callbackInfo->deviceId_, 0);
            callbackInfo->client_ = std::make_shared<SppClientSocket>(*callbackInfo->device_,
                UUID::FromString(callbackInfo->sppOption_->uuid_),
                callbackInfo->sppOption_->type_, callbackInfo->sppOption_->secure_);
            HILOGI("SppConnect client_ constructed");
            if (callbackInfo->client_->Connect() == BtStatus::BT_SUCC) {
                HILOGI("SppConnect successfully");
                callbackInfo->errorCode_ = CODE_SUCCESS;
            } else {
                HILOGI("SppConnect failed");
                callbackInfo->errorCode_ = CODE_FAILED;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("SppConnect execute back");
            SppConnectCallbackInfo* callbackInfo = (SppConnectCallbackInfo*)data;
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
                clientMap.insert(std::make_pair(client->id_, client));
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
                    HILOGI("SppConnect execute Promise mode successfully");
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    HILOGI("SppConnect execute Promise mode failed");
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

    if (clientMap[id]) {
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
    return NapiGetBooleanRet(env, isOK);
}

static napi_status CheckSppWriteParams(
    napi_env env, napi_callback_info info, int &id, char** totalBuf, size_t &totalSize)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_TWO), "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM0]), "Wrong argument type. int expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseArrayBuffer(env, (uint8_t**)(totalBuf), totalSize, argv[PARAM1]),
        "ParseArrayBuffer failed.", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiSppClient::SppWrite(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    char* totalBuf = nullptr;
    size_t totalSize = 0;
    bool isOK = false;
    int id = -1;

    auto status = CheckSppWriteParams(env, info, id, &totalBuf, totalSize);
    NAPI_BT_ASSERT_RETURN_FALSE(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_FALSE(env, clientMap[id] > 0, BT_ERR_INTERNAL_ERROR);
    OutputStream outputStream = clientMap[id]->client_->GetOutputStream();
    while (totalSize) {
        int result = outputStream.Write(totalBuf, totalSize);
        NAPI_BT_ASSERT_RETURN_FALSE(env, result > 0, BT_ERR_SPP_IO);
        totalSize = totalSize - result;
        totalBuf += result;
        isOK = true;
    }
    return NapiGetBooleanRet(env, isOK);
}

napi_status CheckSppClientOn(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisVar = nullptr;
    int id = -1;
    std::string type;
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_THREE), "Requires 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, type, argv[PARAM0]),
        "Wrong argument type. String expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(type.c_str() != STR_BT_SPP_READ, "Invalid type.", napi_invalid_arg);

    callbackInfo = std::make_shared<BufferCallbackInfo>();
    callbackInfo->env_ = env;

    napi_valuetype valueType1 = napi_undefined;
    napi_valuetype valueType2 = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM1], &valueType1));
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM2], &valueType2));
    NAPI_BT_RETURN_IF(valueType1 != napi_number && valueType2 != napi_function,
        "Wrong argument type. Function expected.", napi_invalid_arg);

    napi_create_reference(env, argv[PARAM2], 1, &callbackInfo->callback_);

    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM1]), "Wrong argument type. Int expected.", napi_invalid_arg);

    std::shared_ptr<NapiSppClient> client = NapiSppClient::clientMap[id];
    NAPI_BT_RETURN_IF(!client, "client is nullptr.", napi_invalid_arg);
    client->sppReadFlag = true;
    client->callbackInfos_[type] = callbackInfo;
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
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_THREE), "Requires 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, type, argv[PARAM0]),
        "Wrong argument type. String expected.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(type.c_str() != STR_BT_SPP_READ, "Invalid type.", napi_invalid_arg);

    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM1]), "Wrong argument type. Int expected.", napi_invalid_arg);

    std::shared_ptr<NapiSppClient> client = NapiSppClient::clientMap[id];
    NAPI_BT_RETURN_IF(!client, "client is nullptr.", napi_invalid_arg);
    client->callbackInfos_[type] = nullptr;
    client->sppReadFlag = false;
    sleep(sleepTime);
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
    if (clientMap[id] == nullptr || !clientMap[id]->sppReadFlag ||
        clientMap[id]->callbackInfos_[STR_BT_SPP_READ] == nullptr) {
        HILOGE("thread start failed.");
        return;
    }
    InputStream inputStream = clientMap[id]->client_->GetInputStream();
    char buf[SOCKET_BUFFER_SIZE];
    int ret = 0;

    while (true) {
        HILOGI("thread start.");
        (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
        HILOGI("inputStream.Read start");
        ret = inputStream.Read(buf, sizeof(buf));
        HILOGI("inputStream.Read end");
        if (ret <= 0) {
            HILOGI("inputStream.Read failed, ret = %{public}d", ret);
            return;
        } else {
            HILOGI("callback read data to jshap begin");
            if (clientMap[id] == nullptr || !clientMap[id]->sppReadFlag ||
                clientMap[id]->callbackInfos_[STR_BT_SPP_READ] == nullptr) {
                HILOGE("failed");
                return;
            }
            std::shared_ptr<BufferCallbackInfo> callbackInfo =
                std::static_pointer_cast<BufferCallbackInfo>(clientMap[id]->callbackInfos_[STR_BT_SPP_READ]);

            callbackInfo->info_ = ret;
            if (memcpy_s(callbackInfo->buffer_, sizeof(callbackInfo->buffer_), buf, ret) != EOK) {
                HILOGE("memcpy_s failed!");
                return;
            }

            uv_loop_s *loop = nullptr;
            napi_get_uv_event_loop(callbackInfo->env_, &loop);
            uv_work_t *work = new uv_work_t;
            work->data = (void*)callbackInfo.get();

            uv_queue_work(
                loop,
                work,
                [](uv_work_t *work) {},
                [](uv_work_t *work, int status) {
                    BufferCallbackInfo *callbackInfo = (BufferCallbackInfo *)work->data;
                    int size = callbackInfo->info_;
                    if (size < 0 || size > SOCKET_BUFFER_SIZE) {
                        HILOGE("malloc size error");
                        delete work;
                        work = nullptr;
                        return;
                    }
                    uint8_t* totalBuf = (uint8_t*) malloc(size);
                    if (totalBuf == nullptr) {
                        HILOGE("malloc failed");
                        delete work;
                        work = nullptr;
                        return;
                    }
                    if (memcpy_s(totalBuf, size, callbackInfo->buffer_, size) != EOK) {
                        HILOGE("memcpy_s failed!");
                    }
                    napi_value result = nullptr;
                    uint8_t* bufferData = nullptr;
                    napi_create_arraybuffer(callbackInfo->env_, size, (void**)&bufferData, &result);
                    if (memcpy_s(bufferData, size, totalBuf, size) != EOK) {
                        HILOGE("memcpy_s failed!");
                    }
                    free(totalBuf);

                    napi_value callback = nullptr;
                    napi_value undefined = nullptr;
                    napi_value callResult = nullptr;
                    napi_get_undefined(callbackInfo->env_, &undefined);
                    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
                    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
                    delete work;
                    work = nullptr;
                }
            );
        }
    }
    return;
}
} // namespace Bluetooth
} // namespace OHOS