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

#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <ctime>
#include <cstdio>
#include <securec.h>

#include "btcommon/timer_manager.h"
#include "bluetooth_log.h"

#define DEFAULT_MAX_COUNT 10
#define MILLISECOND_WIDTH 3
#define DEFAULT_TIMER_TIMEOUT_MS 5000

// 以打印该日志的文件名+行号+用户key为关键字，每5秒允许打印一次INFO级别日志，默认会打印一次DEBUG级别日志
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
#define HILOGI_TIME_LIMIT(key, fmt, ...) \
do { \
    if (OHOS::bluetooth::LogUtils::GetInstance()->IsEnableLog(key, __FUNCTION__, __LINE__)) { \
        HILOG_INFO(LOG_CORE, "[%{public}d]" fmt, GetFfrtQueueId(), ##__VA_ARGS__); \
    } \
    HILOGD(fmt, ##__VA_ARGS__); \
} while (0)
#else
#define HILOGI_TIME_LIMIT(key, fmt, ...) \
do { \
    if (OHOS::bluetooth::LogUtils::GetInstance()->IsEnableLog(key, __FUNCTION__, __LINE__)) { \
        HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__); \
    } \
    HILOGD(fmt, ##__VA_ARGS__); \
} while (0)
#endif

struct AccumulatedLogInfo {
    std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> logs;
    std::chrono::system_clock::time_point startTime;
    int count;
};

// 累积日志打印，日志打印次数超过10次或时间超过5秒时打印日志
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
#define HILOGI_ACCUMULATE(key, fmt, ...) \
do { \
    auto _result = OHOS::bluetooth::LogUtils::GetInstance()->FormatLogAndAccumulate( \
        key, fmt, ##__VA_ARGS__); \
    if (!_result.empty()) { \
        HILOG_INFO(LOG_CORE, "[%{public}d]%{public}s", GetFfrtQueueId(), _result.c_str()); \
    } \
} while (0)
#else
#define HILOGI_ACCUMULATE(key, fmt, ...) \
do { \
    auto _result = OHOS::bluetooth::LogUtils::GetInstance()->FormatAndAccumulateLog( \
        key, fmt, ##__VA_ARGS__); \
    if (!_result.empty()) { \
        HILOG_INFO(LOG_CORE, "%{public}s", _result.c_str()); \
    } \
} while (0)
#endif

namespace OHOS {
namespace bluetooth {
class LogUtils {
public:
    LogUtils();
    ~LogUtils() = default;

    static LogUtils *GetInstance(void);

    /**
     * @brief: 根据日志关键字来判断本次是否允许日志打印，目前是5s仅允许打印一次
     * @param key 日志打印关键字
     * @param file 文件名
     * @param line 行号
     * @return true 允许本次日志打印, false 拒绝本次日志打印
     */
    bool IsEnableLog(std::string key, std::string file, int line);

    /**
     * @brief: 累积日志辅助函数，返回需要打印的日志内容
     * @param key 日志打印关键字
     * @param logContent 日志内容
     * @return 需要打印的日志内容，如果不需要打印则返回空services/bluetooth/service/src/common/connect_strategy_manager.cpp字符串
     */
    std::string ProcessAccumulatedLog(std::string key, std::string logContent);

    /**
     * @brief: 格式化并累积日志的辅助函数
     * @param key 日志打印关键字
     * @param fmt 格式化字符串
     * @param ... 可变参数
     * @return 需要打印的日志内容，如果不需要打印则返回空字符串
     */
    template<typename... Args>
    inline std::string FormatLogAndAccumulate(std::string key, const char* fmt, Args... args)
    {
        char buffer[1024];
        errno_t ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, fmt, args...);
        if (ret < 0) {
            buffer[0] = '\0';
        }
        return ProcessAccumulatedLog(key, buffer);
    }

private:
    void OnTimerCallback(void);

    std::set<std::string> logKeySet_ {};
    std::mutex logKeySetMutex_ {};

    int timerTimeoutMs_ = DEFAULT_TIMER_TIMEOUT_MS;
    std::shared_ptr<utility::Timer> timer_ {nullptr};  // 5s 定时器，用于刷新日志key

    std::map<std::string, AccumulatedLogInfo> accumulatedLogMap_ {};
    std::mutex accumulatedLogMutex_ {};
};

// 辅助函数声明
namespace LogUtilsHelper {
    std::string FormatAccumulatedLog(const std::string& key, const AccumulatedLogInfo& info);
    bool ShouldFlushAccumulatedLog(const AccumulatedLogInfo& info, int timerTimeoutMs);
}

}  // namespace bluetooth
}  // namespace OHOS
#endif  // LOG_UTILS_H
