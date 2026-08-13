/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_metrics_reporter.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_metrics"
#endif
#include "log.h"

#include "bt_chr_ue_manager.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
namespace {
constexpr int SCENE_RECONNECT_SUCCESS = 1;
constexpr int SCENE_RECONNECT_FAIL = 2;
constexpr int SCENE_SCAN_TERMINATE = 3;
constexpr int SCENE_CALL_RECORD = 4;
constexpr int SCENE_PBAP_SYNC = 5;
constexpr int SCENE_PBAP_SYNC_FAIL = 6;
const std::string PKG_NAME = "car_policy";
}  // namespace

void CarMetricsReporter::ReportReconnectResult(const std::string &addr, bool success, int retryTimes)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportReconnectResult");
        return;
    }
    int scene = success ? SCENE_RECONNECT_SUCCESS : SCENE_RECONNECT_FAIL;
    ue->WriteCommonUe("CAR_BT_RECONNECT_RESULT", RawAddress(addr), scene, retryTimes, PKG_NAME);
}

void CarMetricsReporter::ReportScanTerminate(const std::string &reason)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportScanTerminate");
        return;
    }
    // 字段复用说明：WriteCommonUe 无独立 reason 槽位，复用 pkgName 字段传递终止原因字符串。
    // 接收端按 pkgName 字段解析为 reason 字符串（如 "BtSettingsUiActive"）。
    ue->WriteCommonUe("CAR_BT_SCAN_TERMINATE", SCENE_SCAN_TERMINATE, reason);
}

void CarMetricsReporter::ReportCallRecord(const std::unordered_map<std::string, std::string> &callReportMap,
                                          const std::vector<std::string> &scoSwitchList)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportCallRecord");
        return;
    }
    // 拼成 "INCOMING=ts;START=ts;END=ts|SCO=1:3,0:8,1:15" 字符串作为 pkgName 通道上报
    std::string callInfo;
    for (const auto &kv : callReportMap) {
        if (!callInfo.empty()) callInfo += ";";
        callInfo += kv.first + "=" + kv.second;
    }
    std::string scoInfo;
    for (const auto &s : scoSwitchList) {
        if (!scoInfo.empty()) scoInfo += ",";
        scoInfo += s;
    }
    std::string payload = callInfo + "|SCO=" + scoInfo;
    // 字段复用说明：WriteCommonUe 无独立 payload 槽位，复用 pkgName 字段传递结构化键值对字符串。
    // 格式："INCOMING=ts;START=ts;END=ts|SCO=1:3,0:8,1:15"，接收端按此格式解析。
    ue->WriteCommonUe("CAR_BT_CALL_RECORD", SCENE_CALL_RECORD, payload);
}

void CarMetricsReporter::ReportPbapSync(const std::string &addr, uint64_t syncStartMs, uint64_t syncEndMs, bool failed)
{
    auto ue = BtChrUeManager::GetInstance();
    if (ue == nullptr) {
        HILOGW("BtChrUeManager null, skip ReportPbapSync");
        return;
    }
    int scene = failed ? SCENE_PBAP_SYNC_FAIL : SCENE_PBAP_SYNC;
    uint64_t duration = 0;
    if (!failed && syncEndMs != 0 && syncEndMs > syncStartMs) {
        duration = syncEndMs - syncStartMs;
    }
    ue->WriteCommonUe("CAR_BT_PBAP_SYNC", RawAddress(addr), scene, static_cast<int>(duration), PKG_NAME);
}

}  // namespace bluetooth
}  // namespace OHOS
