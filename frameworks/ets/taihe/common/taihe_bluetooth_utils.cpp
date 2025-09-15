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

#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_utils.h"

#include <algorithm>
#include <regex>

namespace OHOS {
namespace Bluetooth {
using namespace std;
constexpr int startPos = 6;
constexpr int endPos = 13;

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

// This function applies to interfaces with a single address as a parameter.
bool CheckDeviceIdParam(std::string &addr)
{
    TAIHE_BT_RETURN_IF(!IsValidAddress(addr), "Invalid addr", false);
    return true;
}

taihe_status ParseUuidParams(const std::string &uuid, UUID &outUuid)
{
    if (!IsValidUuid(uuid)) {
        HILOGE("match the UUID faild.");
        return taihe_invalid_arg;
    }
    outUuid = ParcelUuid::FromString(uuid);

    return taihe_ok;
}

std::string GetEncryptAddr(std::string addr)
{
#if defined(IOS_PLATFORM)
    const std::regex deviceIdRegex("^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$");
    if (!regex_match(addr, deviceIdRegex)) {
        return std::string("");
    }
    std::string tmp = "********-****-****-****-************";
    std::string out = addr;
    for (int i = START_POS_IOS_PLATFORM; i <= END_POS_IOS_PLATFORM; i++) {
        out[i] = tmp[i];
    }
    return out;
#else
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
        HILOGD("addr is invalid.");
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    // 00:01:**:**:**:05
    for (int i = startPos; i <= endPos; i++) {
        out[i] = tmp[i];
    }
    return out;
#endif
}
} // namespace Bluetooth
} // namespace OHOS