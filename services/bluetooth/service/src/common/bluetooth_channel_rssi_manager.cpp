/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bluetooth_channel_rssi_manager"
#endif

#include "bluetooth_channel_rssi_manager.h"

#include "bluetooth_hw_interface.h"
#include "bluetooth_connection_manager.h"
#include "datetime_ex.h"

namespace OHOS {
namespace bluetooth {
constexpr int64_t EMPTY_TIMESTAMP = 0;
constexpr int64_t MIN_RSSI_REFRESH_TIME = 60; // 60s
constexpr int64_t MAX_RSSI_VALID_TIME = 60; // 60s

BluetoothChannelRssiManager *BluetoothChannelRssiManager::GetInstance()
{
    static BluetoothChannelRssiManager instance;
    return &instance;
}

bool BluetoothChannelRssiManager::ShouldRefuseSndCmdTooFrequently(int64_t lastUpdatedTime)
{
    int64_t currentTime = GetSecondsSince1970ToNow();
    if (currentTime - lastUpdatedTime < MIN_RSSI_REFRESH_TIME) {
        HILOGD("Read rssi too frequently, lastUpdatedTime: %{public}d, currentTime: %{public}d.", lastUpdatedTime,
            currentTime);
        return true;
    }
    return false;
}

bool BluetoothChannelRssiManager::ShouldRefuseReadRssiOutOfDate(int64_t lastUpdatedTime)
{
    int64_t currentTime = GetSecondsSince1970ToNow();
    if (currentTime - lastUpdatedTime > MAX_RSSI_VALID_TIME) {
        HILOGD("Do not read rssi out-of-date, lastUpdatedTime: %{public}d, currentTime: %{public}d.", lastUpdatedTime,
            currentTime);
        return true;
    }
    return false;
}

void BluetoothChannelRssiManager::SendChannelRssiReadCommand()
{
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }

    std::vector<uint32_t> rssiIdx(BT_ACL_RSSI_REP_SIZE);
    std::vector<int64_t> actualRssiValue(BT_ACL_RSSI_REP_SIZE);
    int64_t timestamp = 0;
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface()->
        hwGetUnionHisiJamChrInfo(rssiIdx, actualRssiValue, timestamp) != BT_STATUS_SUCCESS ||
        ShouldRefuseSndCmdTooFrequently(timestamp)) {
        HILOGD("Failed to send channel rssi read command.");
        return;
    }

    // Currently, only brlink in central role channel noise rssi can be obtained
    std::vector<std::string> brLinks = BluetoothConnectionManager::GetInstance()->brLinks_.GetVector();
    for (auto it = brLinks.begin(); it != brLinks.end(); it++) {
        int ret = BluetoothHwInterface::GetInstance()->GetBtHwInterface()->
            hwSendChannelRssiReadCmd(ServiceUtil::AddrToStack(RawAddress(*it)), BT_TRANSPORT_BR_EDR);
        if (ret == BT_STATUS_SUCCESS) {
            HILOGI("Successfully sent rssi read cmd with brlink addr: %{public}s.", GetEncryptAddr(*it).c_str());
            return;
        }
    }
}

int BluetoothChannelRssiManager::GetChannelRssiInfo(std::vector<uint32_t> &idxInfo,
    std::vector<int64_t> &channelRssiInfo, int64_t &timestamp)
{
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return BT_STATUS_FAIL;
    }

    std::vector<uint32_t> rssiIdx(BT_ACL_RSSI_REP_SIZE);
    std::vector<int64_t> actualRssiValue(BT_ACL_RSSI_REP_SIZE);
    int64_t currentTime = 0;
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface()->
        hwGetUnionHisiJamChrInfo(rssiIdx, actualRssiValue, currentTime) != BT_STATUS_SUCCESS) {
        HILOGD("Failed to get channel rssi info.");
        return BT_STATUS_FAIL;
    }
    if (currentTime == EMPTY_TIMESTAMP || ShouldRefuseReadRssiOutOfDate(currentTime)) {
        HILOGD("Ignore invalid or out-of-date timestamp.");
        return BT_STATUS_FAIL;
    }

    idxInfo = rssiIdx;
    channelRssiInfo = actualRssiValue;
    timestamp = currentTime;
    return BT_STATUS_SUCCESS;
}
}
}