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

enum taihe_status {
    taihe_ok = 0,
    taihe_invalid_arg,
    taihe_object_expected,
    taihe_string_expected,
    taihe_name_expected,
    taihe_function_expected,
    taihe_number_expected,
    taihe_boolean_expected,
    taihe_array_expected,
    taihe_generic_failure,
    taihe_pending_exception,
    taihe_cancelled,
    taihe_escape_called_twice,
    taihe_handle_scope_mismatch,
    taihe_callback_scope_mismatch,
    taihe_queue_full,
    taihe_closing,
    taihe_bigint_expected,
    taihe_date_expected,
    taihe_arraybuffer_expected,
    taihe_detachable_arraybuffer_expected,
    taihe_would_deadlock,  // unused
    taihe_create_ark_runtime_too_many_envs = 22,
    taihe_create_ark_runtime_only_one_env_per_thread = 23,
    taihe_destroy_ark_runtime_env_not_exist = 24
};

#ifndef TAIHE_BT_CALL_RETURN
#define TAIHE_BT_CALL_RETURN(func)                                          \
    do {                                                                   \
        taihe_status ret = (func);                                          \
        if (ret != taihe_ok) {                                              \
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
taihe_status ParseUuidParams(const std::string &uuid, UUID &outUuid);
template<typename T>
void ParseArrayBufferParams(const taihe::array<T>& data, std::vector<T> &outParam)
{
    for (const auto item : data) {
        outParam.push_back(item);
    }
}
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_UTILS_H_