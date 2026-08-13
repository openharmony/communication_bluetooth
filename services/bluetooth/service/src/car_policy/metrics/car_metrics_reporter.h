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

#ifndef CAR_METRICS_REPORTER_H
#define CAR_METRICS_REPORTER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// 数据上报门面：封装通话记录、扫描终止、PBAP 同步等指标上报，通过 BtChrUeManager 写入 UE 事件。
/// 薄包装 BtChrUeManager，提供语义化 API 供 ReconnectEngine / SmartCallTransferController 等调用。
class CarMetricsReporter {
public:
    CarMetricsReporter() = default;
    ~CarMetricsReporter() = default;

    /// 回连结果上报。
    /// @param addr 设备地址
    /// @param success 是否成功
    /// @param retryTimes 累计重试次数
    void ReportReconnectResult(const std::string &addr, bool success, int retryTimes);

    /// 扫描被回连终止上报。
    /// @param reason 终止原因（如 "BtSettingsUiActive"）
    void ReportScanTerminate(const std::string &reason);

    /// 通话记录上报：上报来电时间、通话开始时间、通话结束时间、SCO 切换记录。
    /// @param callReportMap 键值：CALL_INCOMING/CALL_START/CALL_END -> 时间戳字符串
    /// @param scoSwitchList SCO 切换记录列表（"1:秒数" 或 "0:秒数"）
    void ReportCallRecord(const std::unordered_map<std::string, std::string> &callReportMap,
                          const std::vector<std::string> &scoSwitchList);

    /// PBAP 同步结果上报：上报 PBAP 同步开始时间、结束时间、是否失败。
    /// @param addr 设备地址
    /// @param syncStartMs 同步开始时间戳
    /// @param syncEndMs 同步结束时间戳（0 表示失败）
    /// @param failed 是否失败
    void ReportPbapSync(const std::string &addr, uint64_t syncStartMs, uint64_t syncEndMs, bool failed);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_METRICS_REPORTER_H
