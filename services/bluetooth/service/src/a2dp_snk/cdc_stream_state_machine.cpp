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

#ifndef LOG_TAG
#define LOG_TAG "bt_service_cdc_stream_sm"
#endif

#include "log.h"
#include "cdc_stream_state_machine.h"

#include "a2dp_snk_service.h"
#include "a2dp_snk_message.h"
#include "a2dp_snk_def.h"
#include "audio_focus_manager.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_define.h"
#include "bluetooth_common_event_helper.h"
#include "hfp_hf_service.h"
#include "raw_address.h"

namespace {

// A2DP 播放状态广播值
constexpr int STATE_PLAYING = 10;
constexpr int STATE_NOT_PLAYING = 11;

// A2DPSINK_AUDIO_FOCUS 映射值
constexpr int BCAST_FOCUS_NONE = 0;
constexpr int BCAST_FOCUS_GAIN = 1;
constexpr int BCAST_FOCUS_LOSS_TRANSIENT = 2;
constexpr int BCAST_FOCUS_LOSS = 3;
}  // namespace

namespace OHOS {
namespace bluetooth {

// ============================================================
// CdcStreamStateMachine 构造与生命周期
// ============================================================
CdcStreamStateMachine::CdcStreamStateMachine()
{
    HILOGI("CdcStreamStateMachine created");
}

CdcStreamStateMachine::~CdcStreamStateMachine()
{
    Cleanup();
}

void CdcStreamStateMachine::SetFocusManager(std::unique_ptr<AudioFocusManager> focusManager)
{
    focusManager_ = std::move(focusManager);
}

void CdcStreamStateMachine::Init()
{
    if (!focusManager_) {
        focusManager_ = std::make_unique<AudioFocusManager>();
    }
    int32_t ret = focusManager_->InitOnly([this](AudioStandard::InterruptHint hint) {
        PostFocusChangeEvent(static_cast<int>(hint));
    });
    if (ret != 0) {
        HILOGE("AudioFocusManager InitOnly failed ret=%{public}d, focus events will be ignored", ret);
        focusManager_.reset();
    }

    // DELAYED_PAUSE 定时器：LOSS_TRANSIENT 后等手机自行停止的超时干预
    delayedPauseTimer_ = std::make_unique<utility::Timer>(
        [this]() {
            NotifyDelayedPause();
        });
    if (!service_) {
        service_ = A2dpSnkService::GetService();
    }
    if (service_) {
        nativeAdapter_ = service_->GetNativeAdapter();
    }

    // 注册 5 个状态
    std::unique_ptr<utility::StateMachine::State> idle =
        std::make_unique<IdleState>(IDLE, *this);
    std::unique_ptr<utility::StateMachine::State> ready =
        std::make_unique<ReadyState>(READY, *this);
    std::unique_ptr<utility::StateMachine::State> playing =
        std::make_unique<PlayingState>(PLAYING, *this);
    std::unique_ptr<utility::StateMachine::State> transientPaused =
        std::make_unique<TransientPausedState>(TRANSIENT_PAUSED, *this);
    std::unique_ptr<utility::StateMachine::State> noFocus =
        std::make_unique<NoFocusState>(NO_FOCUS, *this);
    Move(idle);
    Move(ready);
    Move(playing);
    Move(transientPaused);
    Move(noFocus);
    InitState(IDLE);
}

void CdcStreamStateMachine::Cleanup()
{
    HILOGI("Cleanup");
    StopDelayedPauseTimer();
    StopFluorideStreaming();
    if (focusManager_) {
        focusManager_->DeactivateAndDeinit();
        focusManager_.reset();
    }
    delayedPauseTimer_.reset();
    nativeAdapter_.reset();
    SetAudioFocusState(AudioFocusState::AUDIO_FOCUS_NONE);
    ResetFlags();
}

// ============================================================
// 事件投递 — 所有外部入口通过 PostEvent 投递到 A2dp 线程
// ============================================================
void CdcStreamStateMachine::PostEvent(const A2dpSnkMessage &event)
{
    auto cdcStreamSmPtr = shared_from_this();
    DoInA2dpSnkStreamThread([cdcStreamSmPtr, event]() {
        cdcStreamSmPtr->ProcessEvent(event);
    });
}

void CdcStreamStateMachine::ProcessEvent(const A2dpSnkMessage &event)
{
    HILOGI("[cdcStreamSM] ProcessEvent what_=%{public}d addr_=%{public}s arg3_=%{public}d",
        event.what_, GetEncryptAddr(event.addr_).c_str(), event.arg3_);
    ProcessMessage(event);
}

// ---- 外部 Notify 接口 ----

void CdcStreamStateMachine::NotifyUserPlayIntent()
{
    HILOGI("NotifyUserPlayIntent");
    A2dpSnkMessage event(A2DP_SNK_STREAM_USER_PLAY_EVT);
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyUserPauseIntent()
{
    HILOGI("NotifyUserPauseIntent");
    A2dpSnkMessage event(A2DP_SNK_STREAM_USER_PAUSE_EVT);
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyAudioStreamStarted()
{
    HILOGI("NotifyAudioStreamStarted");
    A2dpSnkMessage event(A2DP_SNK_STREAM_AUDIO_STARTED_EVT);
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyAudioStreamStopped()
{
    HILOGI("NotifyAudioStreamStopped");
    A2dpSnkMessage event(A2DP_SNK_STREAM_AUDIO_STOPPED_EVT);
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyPlaybackDisallowed()
{
    HILOGI("NotifyPlaybackDisallowed");
    A2dpSnkMessage event(A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT);
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyA2dpConnected(bool playbackAllowed)
{
    HILOGI("NotifyA2dpConnected playbackAllowed=%{public}d", playbackAllowed);
    A2dpSnkMessage event(A2DP_SNK_STREAM_A2DP_CONNECTED_EVT);
    event.arg3_ = playbackAllowed ? ALLOWED_STREAM_PLAY : DISALLOWED_STREAM_PLAY;
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyDeviceDisconnected()
{
    HILOGI("NotifyDeviceDisconnected");
    A2dpSnkMessage event(A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT);
    PostEvent(event);
}

// ---- 内部事件路由 ----

void CdcStreamStateMachine::PostFocusChangeEvent(int hint)
{
    A2dpSnkMessage event(A2DP_SNK_STREAM_FOCUS_CHANGE_EVT);
    event.arg3_ = hint;
    PostEvent(event);
}

void CdcStreamStateMachine::NotifyDelayedPause()
{
    A2dpSnkMessage event(A2DP_SNK_STREAM_DELAYED_PAUSE_EVT);
    PostEvent(event);
}

// ============================================================
// 查询接口
// ============================================================
bool CdcStreamStateMachine::CanPlay() const
{
    // 返回是否持有 GAIN 焦点（READY/PLAYING）
    return audioFocusState_.load() == AudioFocusState::AUDIO_FOCUS_GAIN;
}

bool CdcStreamStateMachine::IsPlaying() const
{
    auto focus = audioFocusState_.load();
    return streamAvailable_.load()
        && focus == AudioFocusState::AUDIO_FOCUS_GAIN;
}

std::string CdcStreamStateMachine::CurrentStateName() const
{
    const State *s = GetState();
    return s ? s->Name() : "";
}

// ============================================================
// 焦点管理 — 状态机是焦点的唯一管理者
// ============================================================
bool CdcStreamStateMachine::RequestAudioFocus()
{
    if (!focusManager_) {
        HILOGE("focusManager_ is nullptr");
        return false;
    }
    int32_t result = focusManager_->ActivateFocus();
    if (result != 0) {
        HILOGE("ActivateFocus failed result=%{public}d", result);
        return false;
    }
    StartFluorideStreaming();
    SetAudioFocusState(AudioFocusState::AUDIO_FOCUS_GAIN);
    HILOGI("RequestAudioFocus success, focus GAIN");
    return true;
}

void CdcStreamStateMachine::AbandonAudioFocus()
{
    HILOGI("AbandonAudioFocus");
    StopFluorideStreaming();
    StopDelayedPauseTimer();  // 幂等，无定时器时无副作用
    if (focusManager_) {
        focusManager_->DeactivateFocus();
    }
    SetAudioFocusState(AudioFocusState::AUDIO_FOCUS_NONE);
}

void CdcStreamStateMachine::SetAudioFocusState(AudioFocusState state)
{
    audioFocusState_.store(state);
}

// ============================================================
// Fluoride 音频流控制
// ============================================================
void CdcStreamStateMachine::StartFluorideStreaming()
{
    HILOGI("StartFluorideStreaming focusState: %{public}d", audioFocusState_.load());
    if (nativeAdapter_) {
        nativeAdapter_->SetAudioFocusState(STATE_FOCUS_GRANTED);
    }
}

void CdcStreamStateMachine::StopFluorideStreaming()
{
    HILOGI("StopFluorideStreaming");
    if (nativeAdapter_) {
        nativeAdapter_->SetAudioFocusState(STATE_FOCUS_LOST);
    }
}

void CdcStreamStateMachine::SetFluorideAudioTrackGain(float gain)
{
    HILOGI("SetFluorideAudioTrackGain gain=%{public}f", gain);
    if (nativeAdapter_) {
        nativeAdapter_->SetAudioTrackGain(gain);
    }
}

// ============================================================
// AVRCP 远端控制 — 仅状态机内部事件使用
// ============================================================
void CdcStreamStateMachine::RequestRemotePlay()
{
    HILOGI("RequestRemotePlay");
    if (!service_) {
        return;
    }
    std::string addr = service_->GetActiveDeviceAddr();
    if (addr.empty()) {
        return;
    }
    auto *avrcpService = AvrcpCtService::GetService();
    if (avrcpService) {
        avrcpService->RequestRemotePlay(RawAddress(addr));
    }
}

void CdcStreamStateMachine::RequestRemotePause()
{
    HILOGI("RequestRemotePause");
    if (!service_) {
        return;
    }
    std::string addr = service_->GetActiveDeviceAddr();
    if (addr.empty()) {
        return;
    }
    auto *avrcpService = AvrcpCtService::GetService();
    if (avrcpService) {
        avrcpService->RequestRemotePause(RawAddress(addr));
    }
}

// ============================================================
// 广播通知
// ============================================================
void CdcStreamStateMachine::NotifyPlayingStateBroadcast()
{
    // 广播暂不提供
}

int CdcStreamStateMachine::GetAudioFocusBroadcastValue() const
{
    // 状态到广播值的映射
    AudioFocusState focus = audioFocusState_.load();
    switch (focus) {
        case AudioFocusState::AUDIO_FOCUS_NONE:
            return BCAST_FOCUS_NONE;
        case AudioFocusState::AUDIO_FOCUS_GAIN:
            return BCAST_FOCUS_GAIN;
        case AudioFocusState::AUDIO_FOCUS_LOSS_TRANSIENT:
            return BCAST_FOCUS_LOSS_TRANSIENT;
        case AudioFocusState::AUDIO_FOCUS_LOSS:
            return BCAST_FOCUS_LOSS;
        default:
            return BCAST_FOCUS_NONE;
    }
}

// ============================================================
// 公共 helper — 多状态复用
// ============================================================

// 播放禁止：取消定时器 + 暂停远端（有流时）
// 调用方：READY/PLAYING/TRANSIENT_PAUSED
// 后续由 HwCarPolicy.connArbitrate 断开 A2DP，DISCONNECT_CLEANUP_EVT 到达后转 IDLE
void CdcStreamStateMachine::HandlePlaybackDisallowed()
{
    HILOGI("HandlePlaybackDisallowed streamAvailable=%{public}d", streamAvailable_.load());
    StopDelayedPauseTimer();
    if (streamAvailable_) {
        RequestRemotePause();
    }
}

// 设备断开：释放焦点（内部已含停定时器 + 停 Fluoride）
// 调用方：所有状态
void CdcStreamStateMachine::HandleDisconnect()
{
    HILOGI("HandleDisconnect");
    AbandonAudioFocus();  // 内部已调 StopDelayedPauseTimer + StopFluorideStreaming
}

// ============================================================
// 辅助方法
// ============================================================
bool CdcStreamStateMachine::IsInCallFromStreamingDevice() const
{
    if (!service_) {
        return false;
    }
    std::string activeAddr = service_->GetActiveDeviceAddr();
    if (activeAddr.empty()) {
        return false;
    }

    auto *hfpHfService = HfpHfService::GetService();
    if (!hfpHfService) {
        return false;
    }

    auto calls = hfpHfService->GetCurrentCallList(RawAddress(activeAddr));
    return !calls.empty();
}

void CdcStreamStateMachine::ResetFlags()
{
    autoResume_ = false;
    streamAvailable_ = false;
}

void CdcStreamStateMachine::StartDelayedPauseTimer()
{
    if (delayedPauseTimer_) {
        delayedPauseTimer_->Start(SETTLE_TIMEOUT_MS);
    }
}

void CdcStreamStateMachine::StopDelayedPauseTimer()
{
    if (delayedPauseTimer_) {
        delayedPauseTimer_->Stop();
    }
}

// ============================================================
// State: IDLE — 无焦点，A2DP 未连接
// ============================================================
void IdleState::Entry()
{
    HILOGI("[Idle] entry");
    stateMachine_.ResetFlags();
    stateMachine_.NotifyPlayingStateBroadcast();
}

void IdleState::Exit()
{
    HILOGI("[Idle] exit");
}

bool IdleState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Idle] what_=%{public}d", event.what_);
    switch (event.what_) {
        // A2DP 连接成功
        case A2DP_SNK_STREAM_A2DP_CONNECTED_EVT: {
            if (event.arg3_ == ALLOWED_STREAM_PLAY) {
                // 策略允许 → 申请焦点转 READY
                HILOGI("[Idle] A2dpConnected(allowed), requesting focus");
                if (stateMachine_.RequestAudioFocus()) {
                    Transition(CdcStreamStateMachine::READY);
                } else {
                    HILOGW("[Idle] focus request failed, stay IDLE");
                }
            } else {
                // 策略不允许 → 保持 IDLE，HwCarPolicy 随后断开 A2DP
                HILOGI("[Idle] A2dpConnected(disallowed), stay IDLE");
            }
            break;
        }

        // 用户播放意图：申请焦点
        // IDLE 处理此事件的两个场景：
        //   1. STREAM_CONNECTED 比 SINK_PLAY 后到（已连接但消息乱序）
        //   2. 之前申请焦点失败，用户再次点击播放
        case A2DP_SNK_STREAM_USER_PLAY_EVT: {
            HILOGI("[Idle] UserPlayIntent, requesting focus");
            if (stateMachine_.RequestAudioFocus()) {
                if (stateMachine_.streamAvailable_) {
                    // [5] 流先于焦点到达（passThru 可能先于 focus）
                    Transition(CdcStreamStateMachine::PLAYING);
                } else {
                    Transition(CdcStreamStateMachine::READY);
                }
            } else {
                HILOGW("[Idle] UserPlayIntent focus request failed");
            }
            break;
        }

        // 流状态标记（passThru 可能先于焦点申请到达）
        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            stateMachine_.streamAvailable_ = true;
            break;

        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            stateMachine_.streamAvailable_ = false;
            break;

        default:
            break;
    }
    return true;
}

// ============================================================
// State: READY — 持有 GAIN 焦点，无音频流（含初始就绪 + 暂停等待）
// 合并 PAUSED：READY 同时表达"等流到达"和"暂停等待"两种语义
// 区分靠 autoResume_：Entry 设 false（不主动恢复）
//   手机自发恢复 → STREAM_START → PLAYING
//   用户点播放  → SINK_PLAY（无操作，AVRCP 已 passThru，等流到达）
// ============================================================
void ReadyState::Entry()
{
    HILOGI("[Ready] entry");
    stateMachine_.autoResume_ = false;  // 不主动恢复播放
    stateMachine_.NotifyPlayingStateBroadcast();
}

void ReadyState::Exit()
{
    HILOGI("[Ready] exit");
}

bool ReadyState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Ready] event=%{public}d", event.what_);
    switch (event.what_) {
        // 音频流到达 → 开始播放（含手机自发恢复）
        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            HILOGI("[Ready] AudioStreamStarted → PLAYING");
            stateMachine_.streamAvailable_ = true;
            Transition(CdcStreamStateMachine::PLAYING);
            break;

        // 流停止标记（防御性，正常 READY 无流）
        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            stateMachine_.streamAvailable_ = false;
            break;

        // 用户播放意图：已有焦点，等流到达即可（AVRCP 已 passThru PLAY）
        case A2DP_SNK_STREAM_USER_PLAY_EVT:
            HILOGI("[Ready] UserPlayIntent: already have focus, waiting for stream");
            break;

        // 用户暂停意图：忽略（无流，AVRCP 已 passThru PAUSE，对手机无害）
        case A2DP_SNK_STREAM_USER_PAUSE_EVT:
            HILOGI("[Ready] UserPauseIntent: no stream, ignored");
            break;

        // 焦点变化
        case A2DP_SNK_STREAM_FOCUS_CHANGE_EVT: {
            int hint = event.arg3_;
            HILOGI("[Ready] FocusHint=%{public}d", hint);
            if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE)) {
                // 临时丢焦点 → 等恢复
                Transition(CdcStreamStateMachine::TRANSIENT_PAUSED);
            } else if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP)) {
                // 永久丢焦点 → 等策略重新授权
                Transition(CdcStreamStateMachine::NO_FOCUS);
            }
            break;
        }

        // 播放禁止：取消定时器 + 暂停远端，等后续 DISCONNECT→IDLE
        case A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT:
            stateMachine_.HandlePlaybackDisallowed();
            break;

        // 设备断开 → 释放焦点回初始状态
        case A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT:
            stateMachine_.HandleDisconnect();
            Transition(CdcStreamStateMachine::IDLE);
            break;

        default:
            break;
    }
    return true;
}

// ============================================================
// State: PLAYING — 持有 GAIN 焦点，音频流活跃
// ============================================================
void PlayingState::Entry()
{
    HILOGI("[Playing] entry");
    // 方案E核心：进入 PLAYING 时默认可自动恢复
    // 若用户在此期间点暂停，SINK_PAUSE 会设为 false
    // TRANSIENT_PAUSED.Entry 不重置，保留此值传递到 GAIN 恢复
    stateMachine_.autoResume_ = true;
    stateMachine_.NotifyPlayingStateBroadcast();
}

void PlayingState::Exit()
{
    HILOGI("[Playing] exit");
    stateMachine_.NotifyPlayingStateBroadcast();
}

bool PlayingState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Playing] what_=%{public}d", event.what_);
    switch (event.what_) {
        // 流开始（防御性标记，正常已在 PLAYING）
        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            stateMachine_.streamAvailable_ = true;
            break;

        // 流停止 → 转 READY（含暂停语义，autoResume_ 在 Entry 设 false）
        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            HILOGI("[Playing] AudioStreamStopped → READY");
            stateMachine_.streamAvailable_ = false;
            Transition(CdcStreamStateMachine::READY);
            break;

        // 用户暂停意图：标记不自动恢复（替代原 pauseRequested_）
        // AVRCP 已直接 passThru(PAUSE)，手机会自行停止流 → STREAM_STOP 到达后转 READY
        case A2DP_SNK_STREAM_USER_PAUSE_EVT:
            HILOGI("[Playing] UserPauseIntent: autoResume=false");
            stateMachine_.autoResume_ = false;
            break;

        // 焦点变化
        case A2DP_SNK_STREAM_FOCUS_CHANGE_EVT: {
            int hint = event.arg3_;
            HILOGI("[Playing] FocusHint=%{public}d", hint);
            if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE)) {
                // 临时丢焦点 → TRANSIENT_PAUSED
                // 不碰 autoResume_！保留上游值：
                //   用户在播放中 = true（Entry 设的）→ GAIN 后自动恢复
                //   用户刚暂停 = false（SINK_PAUSE 设的）→ GAIN 后不恢复
                Transition(CdcStreamStateMachine::TRANSIENT_PAUSED);
            } else if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP)) {
                // 永久丢焦点 → NO_FOCUS
                Transition(CdcStreamStateMachine::NO_FOCUS);
            }
            break;
        }

        // 播放禁止：暂停远端，等后续 DISCONNECT→IDLE
        case A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT:
            stateMachine_.HandlePlaybackDisallowed();
            break;

        // 设备断开 → 释放焦点回初始状态
        case A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT:
            stateMachine_.HandleDisconnect();
            Transition(CdcStreamStateMachine::IDLE);
            break;

        default:
            break;
    }
    return true;
}

// ============================================================
// State: TRANSIENT_PAUSED — 临时丢失焦点(LOSS_TRANSIENT)，等恢复
//
// autoResume_ 生命周期（方案E核心）：
//   Entry 不重置 autoResume_，保留上游意图：
//     从 PLAYING 正常来  = true  （Entry 设的）→ GAIN 后自动恢复
//     从 PLAYING 用户暂停来 = false（SINK_PAUSE 设的）→ GAIN 后不恢复
//     从 READY 来        = false（Entry 设的）→ GAIN 后不恢复
//   DELAYED_PAUSE 不碰 autoResume_，保留上游值
// ============================================================
void TransientPausedState::Entry()
{
    HILOGI("[TransientPaused] entry autoResume=%{public}d", stateMachine_.autoResume_.load());
    stateMachine_.SetAudioFocusState(AudioFocusState::AUDIO_FOCUS_LOSS_TRANSIENT);
    stateMachine_.StopFluorideStreaming();
    stateMachine_.StartDelayedPauseTimer();
    stateMachine_.NotifyPlayingStateBroadcast();
    // 不碰 autoResume_！保留上游传入的值
}

void TransientPausedState::Exit()
{
    HILOGI("[TransientPaused] exit");
}

bool TransientPausedState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[TransientPaused] what_=%{public}d", event.what_);
    switch (event.what_) {
        // 焦点恢复 GAIN
        case A2DP_SNK_STREAM_FOCUS_CHANGE_EVT: {
            int hint = event.arg3_;
            HILOGI("[TransientPaused] FocusHint=%{public}d", hint);
            if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME)) {
                HILOGI("[TransientPaused] GAIN recovered, autoResume=%{public}d stream=%{public}d",
                    stateMachine_.autoResume_.load(), stateMachine_.streamAvailable_.load());
                stateMachine_.StopDelayedPauseTimer();
                stateMachine_.StartFluorideStreaming();
                stateMachine_.SetAudioFocusState(AudioFocusState::AUDIO_FOCUS_GAIN);
                // autoResume=true 表示车机发过 pause，需要发 play 恢复
                if (stateMachine_.autoResume_) {
                    stateMachine_.RequestRemotePlay();
                }
                // 合并 PAUSED 后统一转 READY：
                //   有流 → PLAYING（流已到直接播放）
                //   无流 → READY（等流到达，Entry 设 autoResume=false）
                if (stateMachine_.streamAvailable_) {
                    Transition(CdcStreamStateMachine::PLAYING);
                } else {
                    Transition(CdcStreamStateMachine::READY);
                }
            } else if (hint == static_cast<int>(AudioStandard::InterruptHint::INTERRUPT_HINT_STOP)) {
                // 临时丢焦点升级为永久丢焦点
                HILOGI("[TransientPaused] LOSS_TRANSIENT → LOSS, transition to NO_FOCUS");
                Transition(CdcStreamStateMachine::NO_FOCUS);
            }
            break;
        }

        // DELAYED_PAUSE 超时：手机没自己停，车机主动发 pause
        // 方案E：不设 autoResume_=true！保留上游值
        //   正常播放中 = true（PLAYING.Entry 设的）→ GAIN 后自动恢复
        //   用户暂停后 = false（SINK_PAUSE 设的）→ GAIN 后不恢复
        //   duplicate pause 对手机无害
        case A2DP_SNK_STREAM_DELAYED_PAUSE_EVT: {
            HILOGI("[TransientPaused] DELAYED_PAUSE timer expired, stream=%{public}d autoResume=%{public}d",
                stateMachine_.streamAvailable_.load(), stateMachine_.autoResume_.load());
            if (stateMachine_.streamAvailable_ && !stateMachine_.IsInCallFromStreamingDevice()) {
                stateMachine_.RequestRemotePause();
            } else {
                HILOGI("[TransientPaused] skip pause: stream stopped or in call");
            }
            break;
        }

        // [2a] 用户播放意图：保持 TRANSIENT_PAUSED，设 autoResume=true，取消定时器
        // PLAY 已由 AVRCP 直接 passThru 给手机，GAIN 恢复时自动播放
        case A2DP_SNK_STREAM_USER_PLAY_EVT:
            HILOGI("[TransientPaused] UserPlayIntent: autoResume=true, timer cancelled");
            stateMachine_.StopDelayedPauseTimer();
            stateMachine_.autoResume_ = true;
            break;

        // [2b] 用户暂停意图：设 autoResume=false，取消定时器，保持 TRANSIENT_PAUSED
        // 合并 PAUSED 后不再转换状态，AVRCP 已 passThru PAUSE，等 GAIN 恢复时转 READY
        case A2DP_SNK_STREAM_USER_PAUSE_EVT:
            HILOGI("[TransientPaused] UserPauseIntent: autoResume=false, stay TRANSIENT_PAUSED");
            stateMachine_.StopDelayedPauseTimer();
            stateMachine_.autoResume_ = false;
            break;

        // 流状态变化（仅标记，不转换状态）
        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            stateMachine_.streamAvailable_ = false;
            break;

        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            stateMachine_.streamAvailable_ = true;
            break;

        // 播放禁止：取消定时器 + 暂停远端
        // 不转换，等后续 DISCONNECT→IDLE
        case A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT:
            stateMachine_.HandlePlaybackDisallowed();
            break;

        // 设备断开 → 释放焦点回初始状态
        case A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT:
            stateMachine_.HandleDisconnect();
            Transition(CdcStreamStateMachine::IDLE);
            break;

        default:
            break;
    }
    return true;
}

// ============================================================
// State: NO_FOCUS — 永久丢失焦点(LOSS)，A2DP 仍连接，等用户恢复
//
// 进入条件：FOCUS_LOSS 且 A2DP 不断开（如 C10 华为视频抢焦点，FA 不变）
// 恢复路径：用户点击播放 → SINK_PLAY → RequestAudioFocus → READY/PLAYING
// 注意：PlaybackAllowed 在此状态不可达（FA 不变不会触发）
// ============================================================
void NoFocusState::Entry()
{
    HILOGI("[NoFocus] entry");
    // 有流则暂停远端
    if (stateMachine_.streamAvailable_) {
        stateMachine_.RequestRemotePause();
    }
    stateMachine_.AbandonAudioFocus();
    stateMachine_.NotifyPlayingStateBroadcast();
}

void NoFocusState::Exit()
{
    HILOGI("[NoFocus] exit");
}

bool NoFocusState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[NoFocus] what_=%{public}d", event.what_);
    switch (event.what_) {
        // [8] 用户播放意图：主动申请焦点恢复
        // 对应 C10 恢复：华为视频抢焦点后 FA 不变，用户点播放恢复蓝牙音乐
        case A2DP_SNK_STREAM_USER_PLAY_EVT: {
            HILOGI("[NoFocus] UserPlayIntent, re-requesting focus");
            if (stateMachine_.RequestAudioFocus()) {
                if (stateMachine_.streamAvailable_) {
                    Transition(CdcStreamStateMachine::PLAYING);
                } else {
                    Transition(CdcStreamStateMachine::READY);
                }
            } else {
                HILOGW("[NoFocus] re-request focus failed, stay NO_FOCUS");
            }
            break;
        }

        // 手机自发开始播放（M01-2/M02-2 场景）
        // 仅标记流状态 + 广播，不转换状态（无焦点不能播）
        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            HILOGI("[NoFocus] AudioStreamStarted (phone self-play), mark stream + broadcast");
            stateMachine_.streamAvailable_ = true;
            stateMachine_.NotifyPlayingStateBroadcast();
            break;

        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            HILOGI("[NoFocus] AudioStreamStopped, mark stream + broadcast");
            stateMachine_.streamAvailable_ = false;
            stateMachine_.NotifyPlayingStateBroadcast();
            break;

        // 设备断开 → 释放焦点回初始状态
        case A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT:
            stateMachine_.HandleDisconnect();
            Transition(CdcStreamStateMachine::IDLE);
            break;

        case A2DP_SNK_STREAM_FOCUS_CHANGE_EVT: {
            int hint = event.arg3_;
            HILOGW("[NoFocus] unexpected FocusHint=%{public}d (STOP is permanent, ignoring)", hint);
            break;
        }

        default:
            break;
    }
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
