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

#ifndef I_A2DP_SNK_STREAM_STATE_MACHINE_H
#define I_A2DP_SNK_STREAM_STATE_MACHINE_H

#include "a2dp_snk_message.h"

namespace OHOS {
namespace bluetooth {

// IA2dpSnkStreamStateMachine — A2DP Sink 流控状态机的公共接口
// 由 CdcStreamStateMachine 实现；A2dpSnkService 持有，
// 作为中央枢纽。所有 Notify* 方法将事件投递到专用的
// A2DP Sink 流线程，由状态机串行分发处理。
class IA2dpSnkStreamStateMachine {
public:
    virtual ~IA2dpSnkStreamStateMachine() = default;

    // 向流线程投递事件进行异步分发。
    // 供 A2dpSnkService::ProcessEvent 将 A2dp 线程上到达的事件
    // （如 Bluedroid 栈回调）转发到流线程。
    virtual void PostEvent(const A2dpSnkMessage &event) = 0;

    // AVRCP 用户意图通知（来自 AvrcpCtStateMachine AVSessionCallback）
    virtual void NotifyUserPlayIntent() = 0;
    virtual void NotifyUserPauseIntent() = 0;

    // A2DP 协议栈音频流状态。
    virtual void NotifyAudioStreamStarted() = 0;
    virtual void NotifyAudioStreamStopped() = 0;

    // 策略层通知（来自 CarPolicyAdapter）。
    virtual void NotifyPlaybackDisallowed() = 0;
    virtual void NotifyA2dpConnected(bool playbackAllowed) = 0;
    virtual void NotifyDeviceDisconnected() = 0;

    // DELAYED_PAUSE 定时器回调（设计 §5.9）。
    virtual void NotifyDelayedPause() = 0;

    // 查询接口。
    virtual bool CanPlay() const = 0;
    virtual bool IsPlaying() const = 0;

    // 生命周期。
    virtual void Init() = 0;
    virtual void Cleanup() = 0;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // I_A2DP_SNK_STREAM_STATE_MACHINE_H
