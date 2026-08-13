/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "log_utils.h"
#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdarg>

namespace OHOS {
namespace bluetooth {
LogUtils::LogUtils()
{
    auto func = [this]() { OnTimerCallback(); };
    timer_ = std::make_shared<utility::Timer>(func);
}

LogUtils *LogUtils::GetInstance(void)
{
    static LogUtils instance;
    return &instance;
}

void LogUtils::OnTimerCallback(void)
{
    std::lock_guard<std::mutex> lock(logKeySetMutex_);
    logKeySet_.clear();
}

bool LogUtils::IsEnableLog(std::string key, std::string file, int line)
{
    std::lock_guard<std::mutex> lock(logKeySetMutex_);
    std::string logKey = key + file + std::to_string(line);
    if (logKeySet_.find(logKey) != logKeySet_.end()) {
        return false;
    }
    logKeySet_.insert(logKey);

    if (timer_) {
        timer_->Start(timerTimeoutMs_);
    }
    return true;
}

namespace LogUtilsHelper {
    static std::string GetTimeString(std::chrono::system_clock::time_point timePoint)
    {
        auto time = std::chrono::system_clock::to_time_t(timePoint);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()) % 1000;

        std::tm tm;
        localtime_r(&time, &tm);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%S");
        oss << '.' << std::setfill('0') << std::setw(MILLISECOND_WIDTH) << ms.count();

        return oss.str();
    }

    std::string FormatAccumulatedLog(const std::string& key, const AccumulatedLogInfo& info)
    {
        std::string combinedLog = key + " count:" + std::to_string(info.count) + ", ";
        for (size_t i = 0; i < info.logs.size(); ++i) {
            combinedLog += GetTimeString(info.logs[i].first) + ":{" + info.logs[i].second + "}";
            if (i < info.logs.size() - 1) {
                combinedLog += ", ";
            }
        }
        return combinedLog;
    }

    bool ShouldFlushAccumulatedLog(const AccumulatedLogInfo& info)
    {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.startTime).count();
        return info.count >= DEFAULT_MAX_COUNT || elapsed >= DEFAULT_TIMER_TIMEOUT_MS;
    }
}

std::string LogUtils::ProcessAccumulatedLog(std::string key, std::string logContent)
{
    std::lock_guard<std::mutex> lock(accumulatedLogMutex_);

    auto keyIt = accumulatedLogMap_.find(key);
    auto now = std::chrono::system_clock::now();
    if (keyIt == accumulatedLogMap_.end()) {
        std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> logs;
        AccumulatedLogInfo info {logs, now, 0};
        accumulatedLogMap_[key] = info;
        return key + ", " + logContent;
    }

    auto& info = keyIt->second;
    info.logs.push_back({now, logContent});
    info.count++;

    if (LogUtilsHelper::ShouldFlushAccumulatedLog(info)) {
        std::string combinedLog = LogUtilsHelper::FormatAccumulatedLog(key, info);
        accumulatedLogMap_.erase(keyIt);
        return combinedLog;
    }

    return "";
}
}  // namespace bluetooth
}  // namespace OHOS
