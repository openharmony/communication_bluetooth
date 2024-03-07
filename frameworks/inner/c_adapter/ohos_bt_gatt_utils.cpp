/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "ohos_bt_gatt_utils.h"
#include "bluetooth_log.h"
#include <map>
#include <queue>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {

#define MAX_ADV_ADDR_MAP_SIZE 128
#define ADV_ADDR_TIMEOUT (60 * 60 * 1000) // 1 hour
#define MS_PER_SECOND 1000
#define NS_PER_MS 1000000

struct CaseInsensitiveCompare {
    bool operator()(const string& s1, const string& s2) const
    {
        return lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(),
            [](char c1, char c2) { return tolower(c1) < tolower(c2); });
    }
};
static mutex g_mapQueMutex;
static map<string, uint64_t, CaseInsensitiveCompare> g_advAddrMap; // map<addr, time>
static queue<pair<string, uint64_t>> g_advTimeQueue; // pair<addr, time>

static uint64_t GetBootMillis()
{
    struct timespec ts = {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * MS_PER_SECOND + ts.tv_nsec / NS_PER_MS;
}

/*
 * The method is only available for {@link BleStartAdvWithAddr}.
 * Because there cannot be duplicate adv addresses within one hour,
 * this method will delete adv addresses after one hour.
 */
void RemoveTimeoutAdvAddr()
{
    lock_guard<mutex> lock(g_mapQueMutex);
    uint64_t currentMillis = GetBootMillis();
    while (!g_advTimeQueue.empty() && currentMillis >= g_advTimeQueue.front().second + ADV_ADDR_TIMEOUT) {
        g_advAddrMap.erase(g_advTimeQueue.front().first);
        g_advTimeQueue.pop();
    }
}

/*
 * This method is only available for {@link BleStartAdvWithAddr}.
 * Duplicate addresses within 15 minutes are allowed to be broadcast,
 * and duplicate addresses after 15 minutes are not allowed to be broadcast.
 * There is no limit on non-duplicate addresses.
 */
bool CanStartAdv(const string& addrStr)
{
    lock_guard<mutex> lock(g_mapQueMutex);
    uint64_t currentMillis = GetBootMillis();
    auto addrTime = g_advAddrMap.find(addrStr);
    if (addrTime != g_advAddrMap.end()) {
        if (currentMillis >= addrTime->second + ADV_ADDR_TIME_THRESHOLD) {
            HILOGW("has the same adv addr in [15mins, 60mins]");
            return false;
        } else {
            return true;
        }
    }
    if (g_advTimeQueue.size() >= MAX_ADV_ADDR_MAP_SIZE) {
        g_advAddrMap.erase(g_advTimeQueue.front().first);
        g_advTimeQueue.pop();
    }
    g_advTimeQueue.push(pair<string, uint64_t>(addrStr, currentMillis));
    g_advAddrMap.insert(make_pair(addrStr, currentMillis));
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif