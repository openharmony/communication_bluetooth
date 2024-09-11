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
#ifndef NAPI_BLUETOOTH_ERROR_H_
#define NAPI_BLUETOOTH_ERROR_H_


#include <cstdint>
#include <string>
#include "bluetooth_log.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Bluetooth {

#ifdef ENABLE_NAPI_BLUETOOTH_MANAGER
#ifndef NAPI_BT_ASSERT_RETURN
#define NAPI_BT_ASSERT_RETURN(env, cond, errCode, retObj) \
do {                                                      \
    if (!(cond)) {                                        \
        HandleSyncErr((env), (errCode));                  \
        HILOGE("bluetoothManager napi assert failed.");                    \
        return (retObj);                                  \
    }                                                     \
} while (0)
#endif
#else // ENABLE_NAPI_BLUETOOTH_MANAGER
#ifndef NAPI_BT_ASSERT_RETURN
#define NAPI_BT_ASSERT_RETURN(env, cond, errCode, retObj) \
do {                                                      \
    if (!(cond)) {                                        \
        HILOGE("bluetooth napi assert failed.");                    \
        return (retObj);                                  \
    }                                                     \
} while (0)
#endif
#endif

#define NAPI_BT_ASSERT_RETURN_UNDEF(env, cond, errCode)  \
do {                                                     \
    napi_value res = nullptr;                            \
    napi_get_undefined((env), &res);                     \
    NAPI_BT_ASSERT_RETURN((env), (cond), (errCode), res); \
} while (0)

#define NAPI_BT_ASSERT_RETURN_FALSE(env, cond, errCode)  \
do {                                                     \
    napi_value res = nullptr;                            \
    napi_get_boolean((env), false, &res);                \
    NAPI_BT_ASSERT_RETURN((env), (cond), (errCode), res); \
} while (0)

std::string GetNapiErrMsg(napi_env env, int32_t errCode);
void HandleSyncErr(napi_env env, int32_t errCode);
}
}
#endif