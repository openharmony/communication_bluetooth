/*
 * Copyright (C) Huawei Device Co., Ltd. 2025-2025. All rights reserved.
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

#ifndef NAPI_HA_EVENT_UTILS_H
#define NAPI_HA_EVENT_UTILS_H

#include <iostream>
#include <string>
#include "safe_map.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Bluetooth {

class NapiHaEventUtils {
public:
    NapiHaEventUtils(napi_env env, const std::string &apiName);
    ~NapiHaEventUtils();
    void WriteErrCode(const int32_t errCode);
    static void WriteErrCode(napi_env env, const int32_t errCode);

private:
    NapiHaEventUtils() = delete;
    static void GenerateProcessorId();
    static int64_t AddProcessor();
    std::string RandomTransId() const;
    int64_t GetNowTimeMs() const;
    void WriteEndEvent() const;

private:
    static int64_t processorId_;
    static std::mutex processorLock_;
    static SafeMap<napi_env, int32_t> envErrCodeMap_; // mapping between napi_env and errCode
    napi_env env_;
    std::string apiName_;
    int64_t beginTime_;
};

} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_HA_EVENT_UTILS_H */
