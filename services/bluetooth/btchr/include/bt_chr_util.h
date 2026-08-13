/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_UTIL_H
#define BT_CHR_UTIL_H
#include <cstddef>
#include <iostream>
#include "safe_map.h"

#define EVENT_KEY_SPLICE_MARK "&"

// g_btChrEventMap
static constexpr int BT_CHR_EVENT_MAP = 0;
// g_btChrDeviceInfoMap
static constexpr int BT_CHR_DEVICE_INFO_MAP = 1;
// g_socketCallingRecordMap
static constexpr int BT_CHR_SOCKET_RECORD_MAP = 2;

namespace OHOS {
namespace bluetooth {
/*******************************************************************************
 *  Functions
 ******************************************************************************/
std::string GetEncryptAddr(std::string addr);
#define GET_ENCRYPT_ADDR(addr) (GetEncryptAddr(addr).c_str())
#define GET_ENCRYPT_RAWADDR(device) (GetEncryptAddr((device).GetAddress()).c_str())
std::string GetEncryptEventKey(std::string eventKey);
#define GET_ENCRYPT_EVENTKEY(eventKey) (GetEncryptEventKey(eventKey).c_str())
std::string GetCurrentTime();
std::string GetCurrentTimeMs();
uint64_t GetNowTimeMs();
uint64_t GetNowTime();
std::string GetLowercaseAddr(const std::string& peerAddr);
std::string GetAddrFromEventKey(const std::string& eventKey);

template <typename T>
void BtChrReleaseMap(SafeMap<std::string, T> &releaseMap, const std::vector<std::string> &notClearAddrs, int mapType)
{
    if (releaseMap.IsEmpty()) {
        return;
    }
    if (notClearAddrs.empty()) {
        releaseMap.Clear();
        return;
    }
    std::vector<std::string> releaseKeys;
    releaseMap.Iterate([&notClearAddrs, &releaseKeys](std::string key, T &value) {
        auto it = std::find(notClearAddrs.begin(), notClearAddrs.end(), GetAddrFromEventKey(key));
        if (it == notClearAddrs.end()) {
            releaseKeys.push_back(key);
        }
    });

    for (std::string key : releaseKeys) {
        releaseMap.Erase(key);
    }
}
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_UTIL_H */
