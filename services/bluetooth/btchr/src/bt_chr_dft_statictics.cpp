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
#define LOG_TAG "bt_chr_dft_statictics"
#endif

#include <unordered_set>
#include "bt_chr_dft_statictics.h"
#include "bt_chr_util.h"
#include "bluetooth_log.h"
#include "cJSON.h"

namespace OHOS {
namespace bluetooth {
// key：BtDurationStatisticsType, value：pair<BtStatisticsAtomicEventType, paramName>
static std::map<uint8_t, std::pair<uint8_t, std::string>> g_durationStatParamMap = {
    { DURATION_STAT_TYPE_A2DP, { EVENT_TYPE_A2DP_CALL_STATISTICS, "A2DP_DURATION" } },
    { DURATION_STAT_TYPE_CALL, { EVENT_TYPE_A2DP_CALL_STATISTICS, "CALL_DURATION" } },
    { DURATION_STAT_TYPE_OFFLOAD, { EVENT_TYPE_OFFLOAD_STATISTICS, "OFFLOAD_USED_DURATION" } },
};

constexpr double TIME_SEC_PER_MIN = 60.0;

std::mutex g_durationStatMapLock;
// key: BtDurationStatisticsType, value：start time
std::map<uint8_t, uint64_t> g_durationStatMap;

SafeMap<std::string, BleAdvReachingMaxRecord> g_btChrBleAdvReachingMaxRecords;
std::mutex g_btChrBleAdvReachingMaxPackageNamesMutex {};
std::unordered_set<std::string> g_btChrBleAdvReachingMaxPackageNames;

std::shared_ptr<BtChrDftStatictics> BtChrDftStatictics::instance_ = std::make_shared<BtChrDftStatictics>();

BtChrDftStatictics::BtChrDftStatictics()
{}

std::shared_ptr<BtChrDftStatictics> BtChrDftStatictics::GetInstance()
{
    return BtChrDftStatictics::instance_;
}

void BtChrDftStatictics::WriteCommonStatictics(uint8_t eventType, const std::vector<BtChrEventParam>& params)
{
    if (params.empty()) {
        HILOGE("params is empty, eventType: %{public}d", eventType);
        return;
    }
    std::string eventValue = GetEventValueJsonStr(params);
    BtChrStaticticsEventWrite(eventType, eventValue);
}

void BtChrDftStatictics::WriteCodecStatictics(const uint8_t codedType)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("CODEC_TYPE", codedType));
    BtChrStaticticsEventWrite(EVENT_TYPE_CODEC_STATISTICS, GetEventValueJsonStr(params));
}

void BtChrDftStatictics::WriteAutoConnectStatictics(const uint8_t result)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("AUTOCONNECT_CNT", result));
    BtChrStaticticsEventWrite(EVENT_TYPE_AUTOCONNECT_STATISTICS, GetEventValueJsonStr(params));
}

void BtChrDftStatictics::WriteDurationStatictics(uint8_t durationStatType, bool isStart)
{
    std::lock_guard<std::mutex> lock(g_durationStatMapLock);
    if (isStart) {
        auto iter = g_durationStatMap.find(durationStatType);
        if (iter != g_durationStatMap.end()) {
            g_durationStatMap.erase(durationStatType);
        }
        g_durationStatMap.insert(std::make_pair(durationStatType, GetNowTime()));
        return;
    }
    // get start time for calculating usage duration
    auto iter = g_durationStatMap.find(durationStatType);
    if (iter == g_durationStatMap.end()) {
        HILOGE("g_durationStatMap not found, durationStatType: %{public}d", durationStatType);
        return;
    }
    uint32_t duration = GetDurationInMinutes(GetNowTime() - iter->second); // unit: minutes
    if (duration == 0) {
        HILOGD("duration is zero, durationStatType: %{public}d", durationStatType);
        return;
    }

    // get EventType and paramName
    auto it = g_durationStatParamMap.find(durationStatType);
    if (it == g_durationStatParamMap.end()) {
        HILOGE("g_durationStatParamMap not found, durationStatType: %{public}d", durationStatType);
        return;
    }
    std::pair<uint8_t, std::string> stateParam = it->second;
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam(stateParam.second, duration, BTCHREVENT_UINT32));
    BtChrStaticticsEventWrite(stateParam.first, GetEventValueJsonStr(params));
    g_durationStatMap.erase(durationStatType);
}

uint32_t BtChrDftStatictics::GetDurationInMinutes(uint32_t durationInSeconds)
{
    return static_cast<uint32_t>(std::round(durationInSeconds / TIME_SEC_PER_MIN));
}

std::string BtChrDftStatictics::GetEventValueJsonStr(const std::vector<BtChrEventParam>& params)
{
    cJSON* paramObj = cJSON_CreateObject();
    for (auto iter = params.begin(); iter != params.end(); iter++) {
        BtChrEventParamType type = iter->type;
        if (type == BTCHREVENT_INT32) {
            cJSON_AddNumberToObject(paramObj, iter->name.c_str(), iter->i32Value);
        } else if (type == BTCHREVENT_UINT32) {
            cJSON_AddNumberToObject(paramObj, iter->name.c_str(), iter->ui32Value);
        } else if (type == BTCHREVENT_STRING) {
            cJSON_AddStringToObject(paramObj, iter->name.c_str(), const_cast<char*>(iter->strValue.data()));
        } else {
            HILOGE("not support param type, paramName: %{public}s, type: %{public}d", iter->name.c_str(), type);
            continue;
        }
    }
    char* chrData = cJSON_PrintUnformatted(paramObj);
    if (chrData == nullptr) {
        cJSON_Delete(paramObj);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(paramObj);
    return result;
}

void BtChrDftStatictics::BleAdvReachingMaxStatisticsEvent()
{
    bool upload = false;
    {
        std::lock_guard<std::mutex> lock(g_btChrBleAdvReachingMaxPackageNamesMutex);
        g_btChrBleAdvReachingMaxRecords.Iterate([&upload](std::string pkgName, BleAdvReachingMaxRecord &record) {
            if (upload) {
                return;
            }
            if (g_btChrBleAdvReachingMaxPackageNames.find(pkgName) == g_btChrBleAdvReachingMaxPackageNames.end()) {
                upload = true;
            }
        });
    }
    if (upload) {
        const std::string addr = "00:00:00:00:00:00";
        BtChrEventWriteStr(CHR_BLE_ADV_RESOURCE_REACHING_MAX, addr, "REACHING_MAX_PACKAGE_ARRAY",
            GetBleAdvReachingMaxRecordsJsonStr());
    } else {
        HILOGD("No new records");
    }
    g_btChrBleAdvReachingMaxRecords.Clear();
}

void BtChrDftStatictics::SetBleAdvReachingMaxRecord(const std::string &packageName, int32_t advNum,
    uint32_t maxHoldingTime)
{
    g_btChrBleAdvReachingMaxRecords.Insert(packageName, BleAdvReachingMaxRecord(advNum, maxHoldingTime));
}

std::string BtChrDftStatictics::GetBleAdvReachingMaxRecordsJsonStr()
{
    cJSON *array = cJSON_CreateArray();
    std::unordered_set<std::string> currPackageName;

    g_btChrBleAdvReachingMaxRecords.Iterate(
        [&array, &currPackageName](std::string pkgName, BleAdvReachingMaxRecord &record) {
        cJSON *paramObj = cJSON_CreateObject();
        if (paramObj == nullptr) {
            HILOGE("cjson create paramObj is nullptr.");
            return;
        }
        cJSON_AddItemToObject(paramObj, "packageName", cJSON_CreateString(pkgName.c_str()));
        currPackageName.insert(pkgName);
        cJSON_AddItemToObject(paramObj, "advNum", cJSON_CreateNumber(record.advNum));
        cJSON_AddItemToObject(paramObj, "maxHoldingTime", cJSON_CreateNumber(record.maxHoldingTime));
        cJSON_AddItemToArray(array, paramObj);
    });
    {
        std::lock_guard<std::mutex> lock(g_btChrBleAdvReachingMaxPackageNamesMutex);
        g_btChrBleAdvReachingMaxPackageNames = std::move(currPackageName);
    }
    char* chrData = cJSON_PrintUnformatted(array);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(array);
        return "";
    }
    std::string result = std::string(chrData);
    cJSON_free(chrData);
    cJSON_Delete(array);
    return result;
}
}  // namespace bluetooth
}  // namespace OHOS