/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_connection_common"
#endif

#include "bluetooth_connection_common.h"

#include "bluetooth_def.h"
#include "bluetooth_log.h"

#include <regex>

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {

using Bluetooth::PAIR_NONE;
using Bluetooth::PAIR_PAIRING;
using Bluetooth::PAIR_PAIRED;
using Bluetooth::PIN_TYPE_CONFIRM_PASSKEY;
using Bluetooth::PIN_TYPE_NOTIFY_PASSKEY;

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

std::string GetFormatPinCode(uint32_t pinType, uint32_t pinCode)
{
    std::string pinCodeStr = std::to_string(pinCode);
    if (pinType != PIN_TYPE_CONFIRM_PASSKEY && pinType != PIN_TYPE_NOTIFY_PASSKEY) {
        return pinCodeStr;
    }

    const uint32_t FORMAT_PINCODE_LENGTH = 6;
    while (pinCodeStr.length() < FORMAT_PINCODE_LENGTH) {
        pinCodeStr = "0" + pinCodeStr;
    }
    return pinCodeStr;
}

char* MallocCString(const std::string& origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto length = origin.length() + 1;
    char* res = static_cast<char*>(malloc(sizeof(char) * length));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), length);
}

CArrString Convert2CArrString(std::vector<std::string> &tids)
{
    CArrString res{0};
    if (tids.empty()) {
        return res;
    }

    size_t size = tids.size();
    if (size == 0 ||
        size > std::numeric_limits<size_t>::max() / sizeof(char *)) {
        return res;
    }
    res.head = static_cast<char **>(malloc(sizeof(char *) * size));
    if (!res.head) {
        return res;
    }

    size_t i = 0;
    for (; i < size; ++i) {
        res.head[i] = MallocCString(tids[i]);
    }
    res.size = static_cast<int64_t>(i);

    return res;
}

void DealPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = static_cast<int>(BondState::BOND_STATE_INVALID);
            break;
        case PAIR_PAIRING:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDING);
            break;
        case PAIR_PAIRED:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
}

} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS