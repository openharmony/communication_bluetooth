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
#ifndef LOG_TAG
#define LOG_TAG "bt_chr_util"
#endif

#include <string>
#include <sys/time.h>
#include "securec.h"
#include "bt_chr_util.h"

using namespace std;
namespace OHOS {
namespace bluetooth {
#define RAW_ADDR_MASK_START_POS 6
#define RAW_ADDR_MASK_END_POS 13
#define RAW_ADDR_STR_LEN 17
#define TIME_STMP_STR_LEN 32
#define TIME_MS_PER_SECOND 1000
#define TIME_NS_PER_MS 1000000

std::string GetEncryptAddr(std::string addr)
{
    if (addr.empty() || addr.length() != RAW_ADDR_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    // 00:01:**:**:**:05
    for (int i = RAW_ADDR_MASK_START_POS; i <= RAW_ADDR_MASK_END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

std::string GetEncryptEventKey(std::string eventKey)
{
    auto pos = eventKey.find(EVENT_KEY_SPLICE_MARK);
    if (pos == std::string::npos) {
        return std::string("");
    }
    std::string addr = eventKey.substr(pos + 1, eventKey.size());
    std::string result(eventKey.substr(0, pos + 1));
    result.append(GetEncryptAddr(addr));
    return result;
}

std::string GetCurrentTime()
{
    time_t now = time(nullptr);
    char currentTime[TIME_STMP_STR_LEN];
    struct tm *localTime = localtime(&now);
    if (localTime == nullptr) {
        return "";
    }
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", localTime);
    std::string result;
    return result.append(currentTime);
}

std::string GetCurrentTimeMs()
{
    struct timeval tv = {};
    if (gettimeofday(&tv, nullptr) != 0) {
        return "";
    }
    char currentTime[TIME_STMP_STR_LEN];
    struct tm localTime = {};
    if (localtime_r(&tv.tv_sec, &localTime) == nullptr) {
        return "";
    }
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", &localTime);
    std::string result;
    result.append(currentTime);
    result.append(".");
    char msStr[4];
    snprintf_s(msStr, sizeof(msStr), sizeof(msStr), "%03ld", tv.tv_usec / TIME_MS_PER_SECOND);
    result.append(msStr);
    return result;
}

uint64_t GetNowTimeMs()
{
    struct timespec ts = {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * TIME_MS_PER_SECOND + ts.tv_nsec / TIME_NS_PER_MS;
}

uint64_t GetNowTime()
{
    return GetNowTimeMs() / TIME_MS_PER_SECOND;
}

std::string GetLowercaseAddr(const std::string& peerAddr)
{
    std::string addr(peerAddr);
    // Because of Service and bluedroid have inconsistencies in case, converted to lowercase here
    std::transform(addr.begin(), addr.end(), addr.begin(), ::tolower);
    return addr;
}

std::string GetAddrFromEventKey(const std::string& eventKey)
{
    auto pos = eventKey.find(EVENT_KEY_SPLICE_MARK);
    if (pos == std::string::npos) {
        return eventKey;
    }
    return eventKey.substr(pos + 1, eventKey.size());
}
}  // namespace bluetooth
}  // namespace OHOS