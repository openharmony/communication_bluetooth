/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_event_subscribe"
#endif

#include "napi_event_subscribe_module.h"

#include <algorithm>
#include "../parser/napi_parser_utils.h"
#include "string_ex.h"

namespace OHOS {
namespace Bluetooth {
static std::string ToLogString(const std::vector<std::string> &validNameVec)
{
    std::string str = "[";
    for (const auto &s : validNameVec) {
        str += s;
        str += ", ";
    }
    str += "]";
    return str;
}

NapiEventSubscribeModule::NapiEventSubscribeModule(const char *validEventName, const char *moduleName)
    : validEventNameVec_(std::vector<std::string>{validEventName}), moduleName_(moduleName)
{}
NapiEventSubscribeModule::NapiEventSubscribeModule(std::vector<std::string> validEventNameVec, const char *moduleName)
    : validEventNameVec_(validEventNameVec), moduleName_(moduleName)
{}

napi_status NapiEventSubscribeModule::Register(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments", napi_invalid_arg);

    std::string name {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], name));
    if (!IsValidEventName(name)) {
        HILOGE("Invalid name %{public}s, valid name is %{public}s",
            name.c_str(), ToLogString(validEventNameVec_).c_str());
        return napi_invalid_arg;
    }

    napi_value callback = argv[PARAM1];
    NAPI_BT_CALL_RETURN(NapiIsFunction(env, callback));

    eventSubscribeMap_.ChangeValueByLambda(name, [this, &env, &name, &callback](auto &callbackVec) {
        if (IsNapiCallbackExist(callbackVec, callback)) {
            HILOGW("The %{public}s callback is registered, no need to re-registered", name.c_str());
            return;
        }
        auto napiCallback = std::make_shared<NapiCallback>(env, callback);
        if (napiCallback) {
            callbackVec.push_back(napiCallback);
        }
        HILOGI("Register one %{public}s callback in %{public}s module, %{public}s, %{public}zu callback left",
            name.c_str(), moduleName_.c_str(), napiCallback->ToLogString().c_str(), callbackVec.size());
    });
    return napi_ok;
}

napi_status NapiEventSubscribeModule::Deregister(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(
        argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments", napi_invalid_arg);

    std::string name {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], name));
    if (!IsValidEventName(name)) {
        HILOGE("Invalid name %{public}s, valid name is %{public}s",
            name.c_str(), ToLogString(validEventNameVec_).c_str());
        return napi_invalid_arg;
    }

    if (argc == ARGS_SIZE_ONE) {
        HILOGI("Deregister all %{public}s callback in %{public}s module", name.c_str(), moduleName_.c_str());
        eventSubscribeMap_.Erase(name);
        return napi_ok;
    }
    // The argc is ARGS_SIZE_TWO
    napi_value callback = argv[PARAM1];
    NAPI_BT_CALL_RETURN(NapiIsFunction(env, callback));
    eventSubscribeMap_.ChangeValueByLambda(name, [this, &name, &callback](auto &callbackVec) {
        auto it = std::find_if(callbackVec.begin(), callbackVec.end(),
            [&callback](auto &napiCallback) { return napiCallback->Equal(callback); });
        if (it == callbackVec.end()) {
            HILOGW("Deregister unknown %{public}s callback in %{public}s module", name.c_str(), moduleName_.c_str());
            return;
        }

        std::string callbackIdLog = (*it)->ToLogString();
        callbackVec.erase(it);
        HILOGI("Deregister one %{public}s callback in %{public}s module, %{public}s, %{public}zu callback left",
            name.c_str(), moduleName_.c_str(), callbackIdLog.c_str(), callbackVec.size());
    });
    return napi_ok;
}

bool NapiEventSubscribeModule::IsValidEventName(const std::string &eventName) const
{
    auto it = std::find(validEventNameVec_.begin(), validEventNameVec_.end(), eventName);
    return it != validEventNameVec_.end();
}

bool NapiEventSubscribeModule::IsNapiCallbackExist(
    const std::vector<std::shared_ptr<NapiCallback>> &napiCallbackVec, napi_value &callback) const
{
    auto it = std::find_if(napiCallbackVec.begin(), napiCallbackVec.end(),
        [&callback](const std::shared_ptr<NapiCallback> &napiCallback) { return napiCallback->Equal(callback); });
    return it != napiCallbackVec.end();
}

void NapiEventSubscribeModule::CallFunction(
    std::shared_ptr<NapiNativeObject> nativeObject, std::vector<std::shared_ptr<NapiCallback>> napiCallbackVec)
{
    for (const auto &callback : napiCallbackVec) {
        if (callback == nullptr) {
            continue;
        }
        auto func = [nativeObject, callback]() {
            callback->CallFunction(nativeObject);
        };
        DoInJsMainThread(callback->GetNapiEnv(), std::move(func));
    }
}

void NapiEventSubscribeModule::PublishEvent(
    std::string eventName, const std::shared_ptr<NapiNativeObject> &nativeObject)
{
    eventSubscribeMap_.Iterate([this, &eventName, &nativeObject](
        const std::string &name, std::vector<std::shared_ptr<NapiCallback>> &napiCallbackVec) {
        if (name != eventName) {
            return;
        }
        CallFunction(nativeObject, napiCallbackVec);
    });
}
}  // namespace Bluetooth
}  // namespace OHOS
