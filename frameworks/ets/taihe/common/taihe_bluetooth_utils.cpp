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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_utils"
#endif

#include <algorithm>
#include <regex>

#include "taihe_bluetooth_utils.h"

#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
ani_ref TaiheGetNull(ani_env *env)
{
    ani_ref nullRef = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        HILOGE("null env");
        return nullptr;
    }

    if ((status = env->GetNull(&nullRef)) != ANI_OK) {
        HILOGE("GetNull failed, status: %{public}d", status);
        return nullptr;
    }
    return nullRef;
}

ani_ref TaiheGetUndefined(ani_env *env)
{
    ani_ref undefinedRef = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        HILOGE("null env");
        return nullptr;
    }

    if ((status = env->GetUndefined(&undefinedRef)) != ANI_OK) {
        HILOGE("GetUndefined failed, status: %{public}d", status);
        return nullptr;
    }
    return undefinedRef;
}

static ani_ref CreateBusinessError(ani_env *env, int code, const std::string &msg)
{
    ani_class cls{};
    ani_method method{};
    ani_object obj = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        HILOGE("null env");
        return nullptr;
    }

    if ((status = env->FindClass("@ohos.base.BusinessError", &cls)) != ANI_OK) {
        HILOGE("FindClass failed %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        HILOGE("Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &obj)) != ANI_OK) {
        HILOGE("Object_New failed %{public}d", status);
        return nullptr;
    }

    ani_int errCode = static_cast<ani_int>(code);
    ani_string errMsg;
    if ((status = env->String_NewUTF8(msg.c_str(), msg.size(), &errMsg)) != ANI_OK) {
        HILOGE("String_NewUTF8 failed %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_SetPropertyByName_Int(obj, "code", errCode)) != ANI_OK) {
        HILOGE("Object_SetPropertyByName_Int failed %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetPropertyByName_Ref(obj, "message", errMsg)) != ANI_OK) {
        HILOGE("Object_SetPropertyByName_Ref failed %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<ani_ref>(obj);
}

ani_ref GetCallbackErrorValue(ani_env *env, int errCode)
{
    HILOGE("errCode: %{public}d", errCode);
    ani_ref result = TaiheGetNull(env);
    if (errCode == BT_NO_ERROR) {
        return result;
    }

    std::string errMsg = GetTaiheErrMsg(errCode);
    result = CreateBusinessError(env, errCode, errMsg);
    return result;
}

bool IsValidAddress(std::string bdaddr)
{
#if defined(IOS_PLATFORM)
    const std::regex deviceIdRegex("^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$");
    return regex_match(bdaddr, deviceIdRegex);
#else
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(bdaddr, deviceIdRegex);
#endif
}

bool IsValidConnectStrategy(int strategy)
{
    return strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)
        || strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
}

bool IsValidTransport(int transport)
{
    return transport == BT_TRANSPORT_BREDR || transport == BT_TRANSPORT_BLE;
}

// This function applies to interfaces with a single address as a parameter.
bool CheckDeviceIdParam(std::string &addr)
{
    TAIHE_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    return true;
}

bool CheckSetConnectStrategyParam(std::string &addr, int32_t &strategy)
{
    TAIHE_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    TAIHE_BT_RETURN_IF(!IsValidConnectStrategy(strategy), "Invalid strategy", false);
    return true;
}

bool CheckPairCredibleDeviceParam(std::string &addr, int &transport)
{
    TAIHE_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    TAIHE_BT_RETURN_IF(!IsValidTransport(transport), "Invalid transport", false);
    return true;
}

TaiheStatus ParseUuidParams(const std::string &uuid, UUID &outUuid)
{
    if (!IsValidUuid(uuid)) {
        HILOGE("match the UUID faild.");
        return TAIHE_INVALID_ARG;
    }
    outUuid = ParcelUuid::FromString(uuid);

    return TAIHE_OK;
}
} // namespace Bluetooth
} // namespace OHOS