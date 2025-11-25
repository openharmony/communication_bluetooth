/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_BLUETOOTH_UTILS_H_
#define TAIHE_BLUETOOTH_UTILS_H_

#include <shared_mutex>
#include <vector>

#include "bluetooth_types.h"
#include "stdexcept"
#include "taihe/array.hpp"
#include "taihe/runtime.hpp"
#include "taihe_bluetooth_error.h"

namespace OHOS {
namespace Bluetooth {
constexpr size_t ARGS_SIZE_ZERO = 0;
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr size_t ARGS_SIZE_THREE = 3;
constexpr size_t ARGS_SIZE_FOUR = 4;

ani_ref TaiheGetNull(ani_env *env);
ani_ref TaiheGetUndefined(ani_env *env);
ani_ref GetCallbackErrorValue(ani_env *env, int errCode);

struct TaiheNotifyCharacteristic {
    UUID serviceUuid;
    UUID characterUuid;
    std::vector<uint8_t> characterValue {};
    bool confirm;
};

enum TaiheStatus {
    TAIHE_OK = 0,
    TAIHE_INVALID_ARG,
    TAIHE_OBJECT_EXPECTED,
    TAIHE_STRING_EXPECTED,
    TAIHE_NAME_EXPECTED,
    TAIHE_FUNCTION_EXPECTED,
    TAIHE_NUMBER_EXPECTED,
    TAIHE_BOOLEAN_EXPECTED,
    TAIHE_ARRAY_EXPECTED,
    TAIHE_GENERIC_FAILURE,
    TAIHE_PENDING_EXCEPTION,
    TAIHE_CANCELLED,
    TAIHE_ESCAPE_CALLED_TWICE,
    TAIHE_HANDLE_SCOPE_MISMATCH,
    TAIHE_CALLBACK_SCOPE_MISMATCH,
    TAIHE_QUEUE_FULL,
    TAIHE_CLOSING,
    TAIHE_BIGINT_EXPECTED,
    TAIHE_DATE_EXPECTED,
    TAIHE_ARRAYBUFFER_EXPECTED,
    TAIHE_DETACHABLE_ARRAYBUFFER_EXPECTED,
    TAIHE_WOULD_DEADLOCK,  // unused
    TAIHE_CREATE_ARK_RUNTIME_TOO_MANY_ENVS = 22,
    TAIHE_CREATE_ARK_RUNTIME_ONLY_ONE_ENV_PER_THREAD = 23,
    TAIHE_DESTROY_ARK_RUNTIME_ENV_NOT_EXIST = 24
};

#ifndef TAIHE_BT_CALL_RETURN
#define TAIHE_BT_CALL_RETURN(func)                                          \
    do {                                                                   \
        TaiheStatus ret = (func);                                          \
        if (ret != TAIHE_OK) {                                              \
            HILOGE("api call function failed. ret:%{public}d", ret);      \
            return ret;                                                    \
        }                                                                  \
    } while (0)
#endif

#define TAIHE_BT_RETURN_IF(condition, msg, ret)              \
    do {                                                    \
        if ((condition)) {                                  \
            HILOGE(msg);                                    \
            return (ret);                                   \
        }                                                   \
    } while (0)

bool IsValidAddress(std::string bdaddr);
bool IsValidConnectStrategy(int strategy);
bool IsValidTransport(int transport);
bool CheckDeviceIdParam(std::string &addr);
bool CheckSetConnectStrategyParam(std::string &addr, int32_t &strategy);
bool CheckPairCredibleDeviceParam(std::string &addr, int &transport);
TaiheStatus ParseUuidParams(const std::string &uuid, UUID &outUuid);
template<typename T>
void ParseArrayBufferParams(const taihe::array<T>& data, std::vector<T> &outParam)
{
    for (const auto& item : data) {
        outParam.push_back(item);
    }
}
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_UTILS_H_