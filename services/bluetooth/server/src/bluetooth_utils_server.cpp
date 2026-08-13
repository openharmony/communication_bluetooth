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
#define LOG_TAG "bt_server_utils"
#endif

#include "bluetooth_utils_server.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "permission_manager.h"

using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int startPos = 6;
constexpr int endPos = 13;
std::string GetEncryptAddr(std::string addr)
{
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
        HILOGE("addr is invalid.");
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    // 00:01:**:**:**:05
    for (int i = startPos; i <= endPos; i++) {
        out[i] = tmp[i];
    }
    return out;
}

std::string GetScanModeName(int mode)
{
    switch (mode) {
        case SCAN_MODE_LOW_POWER:
            return "SCAN_MODE_LOW_POWER(0)";
        case SCAN_MODE_BALANCED:
            return "SCAN_MODE_BALANCED(1)";
        case SCAN_MODE_LOW_LATENCY:
            return "SCAN_MODE_LOW_LATENCY(2)";
        case SCAN_MODE_OP_P2_60_3000:
            return "SCAN_MODE_OP_P2_60_3000(3)";
        case SCAN_MODE_OP_P10_60_600:
            return "SCAN_MODE_OP_P10_60_600(4)";
        case SCAN_MODE_OP_P25_60_240:
            return "SCAN_MODE_OP_P25_60_240(5)";
        case SCAN_MODE_OP_P100_1000_1000:
            return "SCAN_MODE_OP_P100_1000_1000(6)";
        case SCAN_MODE_OP_P50_100_200:
            return "SCAN_MODE_OP_P50_100_200(7)";
        case SCAN_MODE_OP_P10_30_300:
            return "SCAN_MODE_OP_P10_30_300(8)";
        case SCAN_MODE_OP_P2_30_1500:
            return "SCAN_MODE_OP_P2_30_1500(9)";
        case SCAN_MODE_OP_P75_30_40:
            return "SCAN_MODE_OP_P75_30_40(A)";
        case SCAN_MODE_OP_P50_30_60:
            return "SCAN_MODE_OP_P50_30_60(B)";
        case SCAN_MODE_OP_P10_25_250:
            return "SCAN_MODE_OP_P10_25_250(C)";
        default:
            return "Unknown";
    }
}

bool IsWalletService(const std::string& callingName, const uint32_t& tokenId)
{
    if (callingName != walletServiceBundleName) {
        return false;
    }
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return false;
    }
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (accessRet != 0) {
        HILOGE("not hap application");
        return false;
    }
    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    std::string appId = GetHapAppIdentifier(userId, bundleName);
    if (appId.empty()) {
        HILOGE("can not get appIdentifier.");
        return false;
    }
    if (appId != walletServiceAppId) {
        HILOGE("Error appId");
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS