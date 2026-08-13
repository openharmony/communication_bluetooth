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

#ifndef A2DP_SNK_DEF_H
#define A2DP_SNK_DEF_H

namespace OHOS {
namespace bluetooth {

constexpr int STATE_FOCUS_LOST = 0;
constexpr int STATE_FOCUS_GRANTED = 1;

constexpr int ALLOWED_STREAM_PLAY = 1;
constexpr int DISALLOWED_STREAM_PLAY = 0;

// 事件延迟重投上限（状态机 IsRemoving 时重试，超过后丢弃）
constexpr int MAX_EVENT_RETRY = 3;

enum A2dpSnkConnectNum {
    A2DP_SNK_CONNECT_NUM_MAX = 6,
    A2DP_SNK_CONNECT_NUM_UN_MAX,
};

enum {
    A2DP_SNK_STATE_DISCONNECTED,
    A2DP_SNK_STATE_CONNECTING,
    A2DP_SNK_STATE_CONNECTED,
    A2DP_SNK_STATE_DISCONNECTING,
};

enum {
    // ── 连接状态机事件（路由到 A2dpSnkStateMachine）──
    // 前缀: CONN_ — 来源: Service 层 / Bluedroid 栈回调
    A2DP_SNK_CONN_EVT_BEGIN,
    A2DP_SNK_CONN_CONNECT_EVT = A2DP_SNK_CONN_EVT_BEGIN,  // Service: Connect() 主动发起连接
    A2DP_SNK_CONN_TIMEOUT_EVT,                             // 内部: 连接超时定时器触发
    A2DP_SNK_CONN_DISCONNECT_EVT,                          // Service: Disconnect() 主动发起断开
    A2DP_SNK_CONN_STACK_CONNECTING_EVT,                    // 栈回调: 正在连接
    A2DP_SNK_CONN_STACK_CONNECTED_EVT,                     // 栈回调: 已连接
    A2DP_SNK_CONN_STACK_DISCONNECTING_EVT,                 // 栈回调: 正在断开
    A2DP_SNK_CONN_STACK_DISCONNECTED_EVT,                  // 栈回调: 已断开
    A2DP_SNK_CONN_STACK_AUDIO_CONFIG_EVT,                  // 栈回调: 音频配置变更
    A2DP_SNK_CONN_EVT_END,

    // ── 流状态机事件（路由到 CdcStreamStateMachine）──
    // 前缀: STREAM_ — 来源: Service 转发 / 流状态机内部 / 策略层
    A2DP_SNK_STREAM_EVT_BEGIN, // 协议栈音频流状态（由 Service 转发）
    A2DP_SNK_STREAM_AUDIO_STARTED_EVT = A2DP_SNK_STREAM_EVT_BEGIN,  // 栈: 音频流开始
    A2DP_SNK_STREAM_AUDIO_STOPPED_EVT,                               // 栈: 音频流停止
    // 用户意图（流状态机内部，NotifyUserPlay/PauseIntent）
    A2DP_SNK_STREAM_USER_PLAY_EVT,                                   // 用户: 播放意图
    A2DP_SNK_STREAM_USER_PAUSE_EVT,                                  // 用户: 暂停意图
    // A2DP 连接通知（流状态机内部，NotifyA2dpConnected）
    A2DP_SNK_STREAM_A2DP_CONNECTED_EVT,                              // A2DP 连接已建立
    // 设备断开清理（流状态机内部，NotifyDeviceDisconnected）
    A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT,                         // 设备断开，清理流
    // 策略层通知（流状态机内部）
    A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT,                         // 策略: 禁止播放
    // 焦点变更（流状态机内部，PostFocusChangeEvent）
    A2DP_SNK_STREAM_FOCUS_CHANGE_EVT,                                // 音频焦点变更
    // 定时器（流状态机内部，NotifyDelayedPause）
    A2DP_SNK_STREAM_DELAYED_PAUSE_EVT,                               // 延迟暂停定时器触发
    A2DP_SNK_STREAM_EVT_END,
};

enum class AudioFocusState {
    AUDIO_FOCUS_NONE = 0,
    AUDIO_FOCUS_GAIN = 1,
    AUDIO_FOCUS_LOSS_TRANSIENT = 2,
    AUDIO_FOCUS_LOSS_TRANSIENT_CAN_DUCK = 3,
    AUDIO_FOCUS_LOSS = 4,
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // A2DP_SNK_DEF_H