/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_event"
#endif

#include <uv.h>
#include "bluetooth_utils.h"
#include "napi_bluetooth_ble_utils.h"
#include "napi_bluetooth_event.h"

namespace OHOS {
namespace Bluetooth {
void NapiEvent::EventNotify(AsyncEventData *asyncEvent)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(asyncEvent->env_, &loop);

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOGI("uv_work_t work is null.");
        delete asyncEvent;
        asyncEvent = nullptr;
        return;
    }
    work->data = asyncEvent;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            AsyncEventData *callbackInfo = static_cast<AsyncEventData*>(work->data);
            napi_value callback = nullptr;
            napi_status ret = napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            if (ret == napi_ok && callback != nullptr) {
                napi_value result = nullptr;
                napi_value undefined = nullptr;
                napi_value callResult = nullptr;
                if (napi_get_undefined(callbackInfo->env_, &undefined) == napi_ok) {
                    result = callbackInfo->packResult();
                }
                if (result != nullptr) {
                    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE,
                        &result, &callResult);
                }
            }
            delete callbackInfo;
            delete work;
            work = nullptr;
        }
    );
}

napi_value NapiEvent::CreateResult(const std::shared_ptr<BluetoothCallbackInfo> &cb, int value)
{
    napi_value result = nullptr;
    if (cb == nullptr) {
        HILOGE("CreateResult cb is null!");
        return result;
    }
    napi_create_object(cb->env_, &result);
    ConvertStateChangeParamToJS(cb->env_, result, cb->deviceId_, value,
        static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
    return result;
}

napi_value NapiEvent::CreateResult(const std::shared_ptr<BluetoothCallbackInfo> &cb,
    BluetoothOppTransferInformation &information)
{
    napi_value result = nullptr;
    napi_create_object(cb->env_, &result);
    ConvertOppTransferInformationToJS(cb->env_, result, information);
    return result;
}

// if callbackInfos contains specific type, new callbackInfo will cover the old.
// If exist, covered event happen, this function will clear rest reference of old callbackInfo in napi framework.
void UpdateCallbackInfo(std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> callbackInfos,
    const std::string &type)
{
    auto it = callbackInfos.find(type);
    if (it != callbackInfos.end() && it->second != nullptr) {
        HILOGD("repetition type %{public}s is register, old callbackInfo will be deleted.", type.c_str());
        // as long as the type is same, callbackInfo will be covered.
        uint32_t refCount = INVALID_REF_COUNT;
        napi_value handlerTemp = nullptr;
        napi_status status = napi_get_reference_value(it->second->env_, it->second->callback_, &handlerTemp);
        if (status != napi_ok) {
            HILOGE("napi_get_reference_value failed. napi status is %{public}d", status);
            return;
        }
        // if handlerTemp exist clear it. no exist, memory leak safe
        if (handlerTemp != nullptr) {
            HILOGD("napi_get_reference_value succeed");
            napi_reference_unref(it->second->env_, it->second->callback_, &refCount);
            HILOGD("decrements the refernce count, refCount: %{public}d", refCount);
            // other place like EventNotify before use will add refCount, happen refCount != 0,
            // ensure other place copy the prepare covered callbackInfo、add refCount and unref and delete refCount
            if (refCount == 0) {
                HILOGD("delete the reference");
                napi_delete_reference(it->second->env_, it->second->callback_);
            }
            HILOGD("old %{public}s is deleted", type.c_str());
        } else {
            HILOGI("napi_get_reference_value is nullptr");
        }
    }
}

napi_value NapiEvent::OnEvent(napi_env env, napi_callback_info info,
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> &callbackInfos)
{
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 2 argument.");
        return ret;
    }
    std::string type;
    if (!ParseString(env, type, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = std::make_shared<BluetoothCallbackInfo>();
    callbackInfo->env_ = env;

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM1], &valueType);
    if (valueType != napi_function) {
        HILOGE("Wrong argument type. Function expected.");
        return ret;
    }

    UpdateCallbackInfo(callbackInfos, type);
    napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_);
    callbackInfos[type] = callbackInfo;
    HILOGD("%{public}s is registered", type.c_str());
    return ret;
}

napi_value NapiEvent::OffEvent(napi_env env, napi_callback_info info,
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> &callbackInfos)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO) {
        HILOGE("Requires 1 or 2 argument.");
        return ret;
    }
    std::string type;
    if (!ParseString(env, type, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }
    auto it = callbackInfos.find(type);
    if (it == callbackInfos.end() || it->second == nullptr) {
        HILOGE("type %{public}s callbackInfos isn't exist.", type.c_str());
        return ret;
    }
    if (env != it->second->env_) {
        HILOGE("env doesn't match, please check.");
        return ret;
    }
    napi_delete_reference(env, it->second->callback_);
    it->second = nullptr;
    HILOGI("%{public}s is unregistered", type.c_str());
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS