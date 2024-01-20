/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_LOG_H
#define BLUETOOTH_LOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"

#ifdef HILOGF
#undef HILOGF
#endif

#ifdef HILOGE
#undef HILOGE
#endif

#ifdef HILOGW
#undef HILOGW
#endif

#ifdef HILOGI
#undef HILOGI
#endif

#ifdef HILOGD
#undef HILOGD
#endif

#ifndef BT_LOG_DOMAIN
#define BT_LOG_DOMAIN 0xD000101
#endif

#ifndef BT_LOG_TAG
#define BT_LOG_TAG "Bluetooth"
#endif

#ifdef LOG_LABEL
#undef LOG_LABEL
#endif

static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, BT_LOG_DOMAIN, BT_LOG_TAG};

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define HILOGD(fmt, ...)                 \
    HILOG_DEBUG(LOG_CORE, "[%{public}s(%{public}s:%{public}d)]" fmt,    \
        FILENAME_SHORT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOGI(fmt, ...)                \
    HILOG_INFO(LOG_CORE, "[%{public}s(%{public}s:%{public}d)]" fmt,    \
        FILENAME_SHORT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOGW(fmt, ...)                \
    HILOG_WARN(LOG_CORE, "[%{public}s(%{public}s:%{public}d)]" fmt,    \
        FILENAME_SHORT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOGE(fmt, ...)                 \
    HILOG_ERROR(LOG_CORE, "[%{public}s(%{public}s:%{public}d)]" fmt,    \
        FILENAME_SHORT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOGF(fmt, ...)                 \
    HILOG_FATAL(LOG_CORE, "[%{public}s(%{public}s:%{public}d)]" fmt,    \
        FILENAME_SHORT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

#define HILOGF(...)
#define HILOGE(...)
#define HILOGW(...)
#define HILOGI(...)
#define HILOGD(...)
#endif  // CONFIG_HILOG

#define CHECK_AND_RETURN_LOG(cond, fmt, ...)        \
    do {                                            \
        if (!(cond)) {                              \
            HILOGE(fmt, ##__VA_ARGS__);             \
            return;                                 \
        }                                           \
    } while (0)

#define CHECK_AND_RETURN_LOG_RET(cond, ret, fmt, ...)               \
    do {                                                            \
        if (!(cond)) {                                              \
            HILOGE(fmt, ##__VA_ARGS__);    \
            return ret;                                             \
        }                                                           \
    } while (0)

#endif  // BLUETOOTH_LOG_H
