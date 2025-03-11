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
#define LOG_TAG "bt_napi_socket_spp_server"
#endif

#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_spp_server.h"
#include "bluetooth_errorcode.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
const int num_20 = 20;
using namespace std;
int NapiSppServer::count = 0;
std::map<int, std::shared_ptr<NapiSppServer>> NapiSppServer::serverMap;

void DefineSppFunctions(napi_env env, napi_value exports)
{
    SppPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sppListen", NapiSppServer::SppListen),
        DECLARE_NAPI_FUNCTION("sppAccept", NapiSppServer::SppAccept),
        DECLARE_NAPI_FUNCTION("sppConnect", NapiSppClient::SppConnect),
        DECLARE_NAPI_FUNCTION("sppCloseServerSocket", NapiSppServer::SppCloseServerSocket),
        DECLARE_NAPI_FUNCTION("sppCloseClientSocket", NapiSppClient::SppCloseClientSocket),
        DECLARE_NAPI_FUNCTION("sppWrite", NapiSppClient::SppWrite),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("on", NapiSppServer::RegisterSocketObserver),
        DECLARE_NAPI_FUNCTION("off", NapiSppServer::DeRegisterSocketObserver),
        DECLARE_NAPI_FUNCTION("sppWriteAsync", NapiSppClient::SppWriteAsync),
        DECLARE_NAPI_FUNCTION("sppReadAsync", NapiSppClient::SppReadAsync),
#endif
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "spp:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}

napi_value NapiSppServer::RegisterSocketObserver(napi_env env, napi_callback_info info)
{
    return NapiSppClient::On(env, info);
}

napi_value NapiSppServer::DeRegisterSocketObserver(napi_env env, napi_callback_info info)
{
    return NapiSppClient::Off(env, info);
}

napi_value SppTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value sppType = nullptr;
    napi_create_object(env, &sppType);
    SetNamedPropertyByInteger(env, sppType, SppType::SPP_RFCOMM, "SPP_RFCOMM");
    return sppType;
}

void SppPropertyValueInit(napi_env env, napi_value exports)
{
    napi_value sppTypeObj = SppTypeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("SppType", sppTypeObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "spp:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
}

static napi_status CheckSppListenParams(
    napi_env env, napi_callback_info info, string &name, SppListenCallbackInfo *callbackInfo)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_THREE && argc != ARGS_SIZE_THREE - CALLBACK_SIZE),
        "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseString(env, name, argv[PARAM0]),
        "Wrong argument type. String expected.", napi_invalid_arg);

    callbackInfo->env_ = env;
    callbackInfo->sppOption_ = GetSppOptionFromJS(env, argv[PARAM1]);
    NAPI_BT_RETURN_IF((callbackInfo->sppOption_ == nullptr), "GetSppOptionFromJS faild.", napi_invalid_arg);
    callbackInfo->name_ = name;

    napi_value promise = nullptr;

    if (argc == ARGS_SIZE_THREE) {
        HILOGI("callback mode");
        napi_valuetype valueType = napi_undefined;
        NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM2], &valueType));
        NAPI_BT_RETURN_IF(valueType != napi_function, "Wrong argument type. Function expected.", napi_invalid_arg);
        napi_create_reference(env, argv[PARAM2], 1, &callbackInfo->callback_);
        napi_get_undefined(env, &promise);
    } else {
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

napi_value NapiSppServer::SppListen(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    string name;
    SppListenCallbackInfo *callbackInfo = new (std::nothrow) SppListenCallbackInfo();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, callbackInfo != nullptr, BT_ERR_INVALID_PARAM);
    auto status = CheckSppListenParams(env, info, name, callbackInfo);
    if (status != napi_ok) {
        delete callbackInfo;
        callbackInfo = nullptr;
    }
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SppListen", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("SppListen execute");
            SppListenCallbackInfo* callbackInfo = static_cast<SppListenCallbackInfo*>(data);
            callbackInfo->server_ = std::make_shared<ServerSocket>(callbackInfo->name_,
                UUID::FromString(callbackInfo->sppOption_->uuid_), callbackInfo->sppOption_->type_,
                callbackInfo->sppOption_->secure_);
            int errorCode = callbackInfo->server_->Listen();
            HILOGI("SppListen ServerSocket constructor end");
            if (callbackInfo->server_ ->GetStringTag() != "") {
                HILOGI("SppListen execute listen success");
                callbackInfo->errorCode_ = CODE_SUCCESS;
            } else {
                HILOGI("SppListen execute listen failed");
                callbackInfo->errorCode_ = errorCode;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("SppListen execute back");
            SppListenCallbackInfo* callbackInfo = static_cast<SppListenCallbackInfo*>(data);
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);

            if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                HILOGI("SppListen execute back listen success");
                std::shared_ptr<NapiSppServer> server =  std::make_shared<NapiSppServer>();
                server->id_ = NapiSppServer::count++;
                napi_create_int32(env, server->id_, &result[PARAM1]);
                server->server_ = callbackInfo->server_;
                serverMap.insert(std::make_pair(server->id_, server));
            } else {
                HILOGI("SppListen execute back listen failed");
                napi_get_undefined(env, &result[PARAM1]);
            }

            if (callbackInfo->callback_) {
                HILOGI("SppListen execute back listen Callback mode success");
                result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
                napi_get_reference_value(env, callbackInfo->callback_, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
                napi_delete_reference(env, callbackInfo->callback_);
            } else {
                if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                    HILOGI("SppListen execute back listen Promise mode success");
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    HILOGI("SppListen execute back listen Promise mode failed");
                    napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                }
            }
            napi_delete_async_work(env, callbackInfo->asyncWork_);
            delete callbackInfo;
            callbackInfo = nullptr;
        },
        static_cast<void*>(callbackInfo),
        &callbackInfo->asyncWork_);

    napi_queue_async_work(env, callbackInfo->asyncWork_);

    return NapiGetUndefinedRet(env);
}

static napi_status CheckSppAcceptParams(
    napi_env env, napi_callback_info info, int32_t &serverSocketNum, SppAcceptCallbackInfo *callbackInfo)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_TWO - CALLBACK_SIZE),
        "Requires 1 or 2 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, serverSocketNum, argv[PARAM0]),
        "Wrong argument type. int expected.", napi_invalid_arg);

    std::shared_ptr<NapiSppServer> server = NapiSppServer::serverMap[serverSocketNum];
    if (!server) {
        HILOGE("server is null");
        return napi_invalid_arg;
    }
    callbackInfo->env_ = env;
    callbackInfo->server_ = server->server_;

    napi_value promise = nullptr;

    if (argc == ARGS_SIZE_TWO) {
        HILOGI("callback mode");
        napi_valuetype valueType = napi_undefined;
        NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_BT_RETURN_IF(valueType != napi_function, "Wrong argument type. Function expected.", napi_invalid_arg);
        napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_);
        napi_get_undefined(env, &promise);
    } else {
        HILOGI("promise mode");
        napi_create_promise(env, &callbackInfo->deferred_, &promise);
    }
    return napi_ok;
}

napi_value NapiSppServer::SppAccept(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int32_t serverSocketNum = -1;
    SppAcceptCallbackInfo *callbackInfo = new (std::nothrow) SppAcceptCallbackInfo();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, callbackInfo != nullptr, BT_ERR_INVALID_PARAM);
    auto status = CheckSppAcceptParams(env, info, serverSocketNum, callbackInfo);
    if (status != napi_ok) {
        delete callbackInfo;
        callbackInfo = nullptr;
    }
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SppAccept", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HILOGI("SppAccept execute");
            SppAcceptCallbackInfo* callbackInfo = static_cast<SppAcceptCallbackInfo*>(data);
            callbackInfo->client_ = callbackInfo->server_->Accept(num_20);
            if (callbackInfo->client_ != nullptr) {
                callbackInfo->errorCode_ = CODE_SUCCESS;
            } else {
                callbackInfo->errorCode_ = CODE_FAILED;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HILOGI("SppAccept execute back");
            SppAcceptCallbackInfo* callbackInfo = static_cast<SppAcceptCallbackInfo*>(data);
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);

            if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                std::shared_ptr<NapiSppClient> client =  std::make_shared<NapiSppClient>();
                client->id_ = NapiSppClient::count++;
                napi_create_int32(env, client->id_, &result[PARAM1]);
                client->client_ = callbackInfo->client_;
                NapiSppClient::clientMap.insert(std::make_pair(client->id_, client));
            } else {
                napi_get_undefined(env, &result[PARAM1]);
            }

            if (callbackInfo->callback_) {
                result[PARAM0] = GetCallbackErrorValue(callbackInfo->env_, callbackInfo->errorCode_);
                napi_get_reference_value(env, callbackInfo->callback_, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
                napi_delete_reference(env, callbackInfo->callback_);
            } else {
                if (callbackInfo->errorCode_ == CODE_SUCCESS) {
                    napi_resolve_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                } else {
                    napi_reject_deferred(env, callbackInfo->deferred_, result[PARAM1]);
                }
            }
            napi_delete_async_work(env, callbackInfo->asyncWork_);
            delete callbackInfo;
            callbackInfo = nullptr;
        },
        static_cast<void*>(callbackInfo),
        &callbackInfo->asyncWork_);

    napi_queue_async_work(env, callbackInfo->asyncWork_);

    return NapiGetUndefinedRet(env);
}

static napi_status CheckSppCloseServerSockeParams(napi_env env, napi_callback_info info, int &id)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_ONE), "Requires 1 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, id, argv[PARAM0]), "Wrong argument type. int expected.", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiSppServer::SppCloseServerSocket(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int id =  -1;
    auto status = CheckSppCloseServerSockeParams(env, info, id);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    bool isOK = false;

    std::shared_ptr<NapiSppServer> server = nullptr;
    std::shared_ptr<NapiSppClient> client = nullptr;

    if (NapiSppClient::clientMap[id] != nullptr) {
        client = NapiSppClient::clientMap[id];
        if (client->client_) {
            client->client_->Close();
            NapiSppClient::clientMap.erase(id);
        }
    } else {
        HILOGE("no such key in clientMap.");
    }

    if (serverMap[id] != nullptr) {
        server = serverMap[id];
        if (server->server_) {
            server->server_->Close();
            serverMap.erase(id);
            isOK = true;
        }
    } else {
        HILOGE("no such key in serverMap.");
    }

    return NapiGetBooleanRet(env, isOK);
}
} // namespace Bluetooth
} // namespace OHOS
