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

#ifndef CAR_PROFILE_SERVICE_H
#define CAR_PROFILE_SERVICE_H

#include <string>

#include "guard/car_connection_guard.h"  // ProfileType

namespace OHOS {
namespace bluetooth {

/// Profile Service 操作封装（集中 HFP/A2DP 的 Connect/Disconnect/查询）。
/// 消除各模块重复的 lambda 包装，CarPolicyManager::Init 不再需要 service 操作回调。
class CarProfileService {
public:
    /// 连接指定 Profile（HFP_HF → HfpHfService::Connect，A2DP_SINK → A2dpSnkService::Connect）
    static void Connect(ProfileType profile, const std::string &addr);
    /// 断开指定 Profile
    static void Disconnect(ProfileType profile, const std::string &addr);
    /// 通知 A2DP 已连接（请求音频焦点）
    static void NotifyA2dpConnected();
    /// 通知 A2DP 策略不允许播放
    static void NotifyPlaybackDisallowed();
    /// 是否有 HFP 处于 CONNECTING 或 CONNECTED 状态
    static bool HasHfpConnectingOrConnected();
    /// 获取指定设备的 HFP 状态（BTConnectState 值，service 为空返回 DISCONNECTED=3）
    static int GetHfpDeviceState(const std::string &addr);
    /// 获取指定设备的 A2DP 状态（BTConnectState 值，service 为空返回 DISCONNECTED=3）
    static int GetA2dpDeviceState(const std::string &addr);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_PROFILE_SERVICE_H
