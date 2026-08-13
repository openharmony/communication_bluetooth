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

#ifndef BLUETOOTH_CHANNEL_RSSI_MANAGER_H
#define BLUETOOTH_CHANNEL_RSSI_MANAGER_H

#include <cstdint>
#include <vector>

namespace OHOS {
namespace bluetooth {
constexpr int BT_ACL_RSSI_REP_SIZE = 14;
constexpr int RSSI_UNIT_SIZE = 2;

class BluetoothChannelRssiManager {
public:
    static BluetoothChannelRssiManager *GetInstance();

    void SendChannelRssiReadCommand();
    int GetChannelRssiInfo(std::vector<uint32_t> &idxInfo, std::vector<int64_t> &channelRssiInfo, int64_t &timestamp);
private:
    bool ShouldRefuseSndCmdTooFrequently(int64_t lastUpdatedTime);
    bool ShouldRefuseReadRssiOutOfDate(int64_t lastUpdatedTime);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif