/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef REFUSE_PLAY_HELPER_H
#define REFUSE_PLAY_HELPER_H

#include "safe_map.h"
#include "safe_vector.h"

namespace OHOS {
namespace bluetooth {
constexpr int AUTO_PLAY_UNKNOWN = -1;
constexpr int AUTO_PLAY_NOT_ALLLOW = 0;
constexpr int AUTO_PLAY_ALLLOW = 1;
constexpr int DEFAULT_VALUE = -1;
constexpr int64_t DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS = 6 * 1000;
typedef struct {
    /** 存储最后一次AVRCP连接的时间 */
    int64_t lastAvrcpConnectTime;
    /** 存储最后一次挂断电话的时间 */
    int64_t lastHangUpTime;
    /** 存储上一次暂停的时间 */
    int64_t lastPauseTime;
} TimeRecord;

class RefusePlayHelper {
public:
    static RefusePlayHelper* GetInstance();
    /**
     * 设置最后一次AVRCP连接的时间
     */
    void SetLastAvrcpConnectTime(const std::string &device);
    /**
     * 获取最后一次AVRCP连接的时间
     */
    int64_t GetLastAvrcpConnectTime(const std::string &device);
    /**
     * play指令拦截策略
     */
    bool IsRefusePlayAfterConnect(const std::string &device);
    /**
     * 是否需要发送play指令
     */
    bool IsNeedToSendPlay(const std::string &device);

    void ClearRecord(const std::string &device);
    void ClearAuthingDevice(const std::string &device);
    bool IsShowCapsuleAfterHangUp(const std::string &device);
    int64_t GetLastHangUpTime(const std::string &device);
    void SetLastHangUpTime(const std::string &device);
    bool NeedIgnoreSetActive();
    bool NeedDropDueScoEnd(const std::string &device, bool isPushed);
    void ProcessPushed(const std::string &device, bool isPushed);
    bool IsPauseAllowedByWhiteList();
    void SavePauseTime(const std::string &device);
    void JudgeScoPause(const std::string &device);
private:
    bool IsAutoPlay(const std::string &device, int64_t currentTime);
    // 时间记录
    SafeMap<std::string, TimeRecord> mTimeRecordMap_;
    // 已弹窗等待用户授权的设备
    SafeMap<std::string, int64_t> mWatingForAuthDevices_;
    SafeMap<std::string, bool> mKeyPushedMap_;
    SafeMap<std::string, bool> mScoPauseMap_;
};
} // namespace bluetooth
} // namespace OHOS
#endif