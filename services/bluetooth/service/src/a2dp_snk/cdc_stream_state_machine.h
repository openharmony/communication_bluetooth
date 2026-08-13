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

#ifndef CDC_STREAM_STATE_MACHINE_H
#define CDC_STREAM_STATE_MACHINE_H

#include <atomic>
#include <memory>
#include <string>

#include "a2dp_snk_message.h"
#include "a2dp_snk_def.h"
#include "bt_av.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "i_a2dp_snk_stream_state_machine.h"
#include "i_native_a2dp_adapter.h"
#include "audio_focus_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {

class A2dpSnkService;

class CdcStreamStateMachine : public IA2dpSnkStreamStateMachine,
                              public utility::StateMachine,
                              public std::enable_shared_from_this<CdcStreamStateMachine> {
public:
    static constexpr int SETTLE_TIMEOUT_MS = 400;  // DELAYED_PAUSE 定时器（§5.9）

    inline static const std::string IDLE = "Idle";
    inline static const std::string READY = "Ready";
    inline static const std::string PLAYING = "Playing";
    inline static const std::string TRANSIENT_PAUSED = "TransientPaused";
    inline static const std::string NO_FOCUS = "NoFocus";

    CdcStreamStateMachine();
    ~CdcStreamStateMachine() override;

    // 注册各状态并设置初始状态为 IDLE。由构造函数调用。
    void Init() override;


    // AVRCP 用户意图通知（来自 AvrcpCtStateMachine AVSessionCallback）。
    void NotifyUserPlayIntent() override;
    void NotifyUserPauseIntent() override;

    // A2DP 协议栈音频流状态。
    void NotifyAudioStreamStarted() override;
    void NotifyAudioStreamStopped() override;
    // 策略层通知（来自 CarPolicyAdapter）。
    void NotifyPlaybackDisallowed() override;
    void NotifyA2dpConnected(bool playbackAllowed) override;
    void NotifyDeviceDisconnected() override;
    // DELAYED_PAUSE 定时器回调（设计 §5.9）。
    void NotifyDelayedPause() override;
    // 查询接口。
    bool CanPlay() const override;
    bool IsPlaying() const override;
    // 生命周期。
    void Cleanup() override;

    // 供 A2dpSnkService::ProcessEvent 分发已入队事件时调用。
    void PostFocusChangeEvent(int hint);

    // ---- 专用 A2DP Sink 流线程上的事件循环 ----
    // 向专用流状态机线程投递事件（DoInA2dpSnkStreamThread），
    // 通过 ProcessEvent 串行分发。所有 Notify* 方法均使用此机制，
    // 使流状态机独立于连接状态机（A2dpSnkStateMachine）运行。
    void PostEvent(const A2dpSnkMessage &event) override;
    // 将单个事件分发给当前状态。在流线程上运行。
    void ProcessEvent(const A2dpSnkMessage &event);

    // 状态访问器，供测试 / 调试使用。
    std::string CurrentStateName() const;

    // 仅测试用：注入 mock AudioFocusManager。必须在 Init() 之前调用。
    void SetFocusManager(std::unique_ptr<AudioFocusManager> focusManager);

private:
    // Fluoride 流控制（§6.5）。
    void StartFluorideStreaming();
    void StopFluorideStreaming();
    // 焦点辅助方法。
    bool RequestAudioFocus();
    void AbandonAudioFocus();
    void SetAudioFocusState(AudioFocusState state);
    void SetFluorideAudioTrackGain(float gain);
    // AVRCP 远端命令 — 运行时通过 AvrcpCtService::GetService() 查询（§3.3, 仅状态机内部使用）。
    void RequestRemotePlay();
    void RequestRemotePause();
    // 广播与观察者通知（§6.1-§6.4）。
    void NotifyPlayingStateBroadcast();
    int GetAudioFocusBroadcastValue() const;  // §6.2 映射
    // 公共 helper — 多状态复用。
    void HandlePlaybackDisallowed();
    void HandleDisconnect();
    // 其他辅助方法。
    bool IsInCallFromStreamingDevice() const;
    void ResetFlags();           // 清除 playRequested/pauseRequested/autoResume/streamAvailable_
    void StartDelayedPauseTimer();
    void StopDelayedPauseTimer();

    // 依赖。
    A2dpSnkService *service_{nullptr};
    std::unique_ptr<AudioFocusManager> focusManager_{nullptr};
    std::shared_ptr<INativeA2dpAdapter> nativeAdapter_{nullptr};

    // DELAYED_PAUSE 定时器（§5.9）。回调向 service 投递 CDC_MSG_DELAYED_PAUSE。
    std::unique_ptr<utility::Timer> delayedPauseTimer_{nullptr};

    // 焦点状态（原子变量，支持 CanPlay/IsPlaying 跨线程读取）。
    std::atomic<AudioFocusState> audioFocusState_{AudioFocusState::AUDIO_FOCUS_NONE};

    std::atomic<bool> autoResume_{false};       // 车端在 TRANSIENT_PAUSED 期间发送暂停；GAIN 时随 play 恢复

    std::atomic<bool> streamAvailable_{false};

    // 状态类需要访问私有成员。
    friend class IdleState;
    friend class ReadyState;
    friend class PlayingState;
    friend class TransientPausedState;
    friend class NoFocusState;

    BT_DISALLOW_COPY_AND_ASSIGN(CdcStreamStateMachine);
};

// CdcStreamStateMachine 各状态类的基类。通过 stateMachine_
// 提供对所属状态机的类型化访问。
class CdcStreamState : public utility::StateMachine::State {
public:
    CdcStreamState(const std::string &name, utility::StateMachine &stateMachine)
        : State(name, stateMachine), stateMachine_(static_cast<CdcStreamStateMachine &>(stateMachine))
    {}
    ~CdcStreamState() override = default;

protected:
    CdcStreamStateMachine &stateMachine_;
    BT_DISALLOW_COPY_AND_ASSIGN(CdcStreamState);
};

// IDLE: 无焦点，无流。（§2.1）
class IdleState : public CdcStreamState {
public:
    IdleState(const std::string &name, utility::StateMachine &stateMachine) : CdcStreamState(name, stateMachine) {}
    ~IdleState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    BT_DISALLOW_COPY_AND_ASSIGN(IdleState);
};

// READY: 焦点 GAIN，无流。等待 AudioStreamStarted。（§2.1）
class ReadyState : public CdcStreamState {
public:
    ReadyState(const std::string &name, utility::StateMachine &stateMachine) : CdcStreamState(name, stateMachine) {}
    ~ReadyState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    BT_DISALLOW_COPY_AND_ASSIGN(ReadyState);
};

// PLAYING: 焦点 GAIN，流活跃。（§2.1）
class PlayingState : public CdcStreamState {
public:
    PlayingState(const std::string &name, utility::StateMachine &stateMachine) : CdcStreamState(name, stateMachine) {}
    ~PlayingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    BT_DISALLOW_COPY_AND_ASSIGN(PlayingState);
};

// TRANSIENT_PAUSED: 焦点 LOSS_TRANSIENT。等待 GAIN；DELAYED_PAUSE 待处理。（§2.1, §5）
class TransientPausedState : public CdcStreamState {
public:
    TransientPausedState(const std::string &name, utility::StateMachine &stateMachine)
        : CdcStreamState(name, stateMachine) {}
    ~TransientPausedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    BT_DISALLOW_COPY_AND_ASSIGN(TransientPausedState);
};

// NO_FOCUS: 焦点 LOSS（永久）。等待 PlaybackAllowed 以重新请求焦点。（§2.1）
class NoFocusState : public CdcStreamState {
public:
    NoFocusState(const std::string &name, utility::StateMachine &stateMachine) : CdcStreamState(name, stateMachine) {}
    ~NoFocusState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    BT_DISALLOW_COPY_AND_ASSIGN(NoFocusState);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CDC_STREAM_STATE_MACHINE_H
