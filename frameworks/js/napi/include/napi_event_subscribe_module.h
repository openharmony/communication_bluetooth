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

#ifndef NAPI_EVENT_SUBSCRIBE_MODULE_H
#define NAPI_EVENT_SUBSCRIBE_MODULE_H

#include <memory>
#include <string>
#include <vector>

#include "napi_async_callback.h"
#include "napi_native_object.h"
#include "safe_map.h"

#ifndef BT_MODULE_NAME
#define BT_MODULE_NAME "default"
#endif

namespace OHOS {
namespace Bluetooth {
class NapiEventSubscribeModule {
public:
    NapiEventSubscribeModule(const char *validEventName, const char *moduleName);
    NapiEventSubscribeModule(std::vector<std::string> validEventNameVec, const char *moduleName);
    ~NapiEventSubscribeModule() = default;

    napi_status Register(napi_env env, napi_callback_info info);
    napi_status Deregister(napi_env env, napi_callback_info info);
    void PublishEvent(std::string eventName, const std::shared_ptr<NapiNativeObject> &nativeObject);

private:
    bool IsValidEventName(const std::string &eventName) const;
    bool IsNapiCallbackExist(
        const std::vector<std::shared_ptr<NapiCallback>> &napiCallbackVec, napi_value &callback) const;
    void CallFunction(
        std::shared_ptr<NapiNativeObject> nativeObject, std::vector<std::shared_ptr<NapiCallback>> napiCallbackVec);

    std::vector<std::string> validEventNameVec_ {};
    std::string moduleName_ = "";
    SafeMap<std::string, std::vector<std::shared_ptr<NapiCallback>>> eventSubscribeMap_ {};
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_EVENT_SUBSCRIBE_MODULE_H
