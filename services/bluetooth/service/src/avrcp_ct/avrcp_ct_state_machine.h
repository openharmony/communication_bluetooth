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

#ifndef AVRCP_CT_STATE_MACHINE_H
#define AVRCP_CT_STATE_MACHINE_H

#include <string>
#include <atomic>
#include <memory>
#include "base_def.h"

#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "avrcp_ct_define.h"
#include "avrcp_ct_avsession.h"
#include "avrcp_ct_message.h"
#include "avrcp_ct_player.h"
#include "avrcp_ct_media_item.h"
#include "common_util.h"
#include "thread_util.h"
#include "avrcp_ct_avsession_controller.h"
#include "avrcp_ct_cover_art_manager.h"
#include "a2dp_snk_service.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCtStateMachine;

/**
 * @brief Class for avrcp controller state machine.
 */
class AvrcpCtStateMachine : public utility::StateMachine, public std::enable_shared_from_this<AvrcpCtStateMachine> {
public:
    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string CONNECTED = "Connected";
    inline static const std::string DISCONNECTING = "Disconnecting";

    explicit AvrcpCtStateMachine(const std::string &address);
    ~AvrcpCtStateMachine() = default;
    void Init();
    void NotifyStateChanged();
    int GetDeviceState();
    int GetState() const;
    std::string GetDeviceName() const;
    static std::string GetEventName(int what);
    void CleanupAvSession();
    void RemoveFromService();

    AVSession::AVMetaData &GetAVMetaData();
    AVSession::AVPlaybackState &GetAVPlaybackState();

    void UpdatePlayerInfo(const std::string& playerName, int playerId, int playStatus);
    void UpdateCurrentTrack(std::shared_ptr<AvrcpCtMediaItem> track);
    void ProcessGetCoverArtPsm();
    void CreateAvSessionController(const std::string &sessionId);
    void SetAvsessionFilter();
    void OnMetaDataChange(const AVSession::AVMetaData &data);
    void OnPlaybackStateChange(const AVSession::AVPlaybackState &state);
    bool CheckTrackStateChanged(const AVSession::AVMetaData &data);
    bool CheckPlaybackStateChanged(const AVSession::AVPlaybackState &state);
    std::shared_ptr<AvrcpCtMediaItem> GetCurrentTrack();
    const std::vector<AvrcpCtPlayer> GetAvailablePlayers();

    class AVSessionCallbackImpl : public AVSession::AVSessionCallback {
    public:
        explicit AVSessionCallbackImpl(std::weak_ptr<AvrcpCtStateMachine> stateMachine) : sm_(stateMachine){};
        ~AVSessionCallbackImpl() = default;
        void OnPlay(const AVSession::AVControlCommand& cmd) override
        {
            auto *a2dpService = A2dpSnkService::GetService();
            if (a2dpService) {
                a2dpService->NotifyUserPlayIntent();
            }
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_PLAY);
            SendMessage(msg);
        }
        void OnPause() override
        {
            auto *a2dpService = A2dpSnkService::GetService();
            if (a2dpService) {
                a2dpService->NotifyUserPauseIntent();
            }
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_PAUSE);
            SendMessage(msg);
        }
        void OnStop() override
        {
            auto *a2dpService = A2dpSnkService::GetService();
            if (a2dpService) {
                a2dpService->NotifyUserPauseIntent();
            }
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_STOP);
            SendMessage(msg);
        }
        void OnPlayNext(const AVSession::AVControlCommand& cmd) override
        {
            auto *a2dpService = A2dpSnkService::GetService();
            if (a2dpService) {
                a2dpService->NotifyUserPlayIntent();
            }
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_FORWARD);
            SendMessage(msg);
        }
        void OnPlayPrevious(const AVSession::AVControlCommand& cmd) override
        {
            auto *a2dpService = A2dpSnkService::GetService();
            if (a2dpService) {
                a2dpService->NotifyUserPlayIntent();
            }
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_BACKWARD);
            SendMessage(msg);
        }
        void OnFastForward(int64_t time, const AVSession::AVControlCommand& cmd) override
        {
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_FF);
            SendMessage(msg);
        }
        void OnRewind(int64_t time, const AVSession::AVControlCommand& cmd) override
        {
            AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_REWIND);
            SendMessage(msg);
        }
        void OnSetLoopMode(int32_t loopMode) override
        {
            // AVSession 把 shuffle 当作 loopMode 的一种（LOOP_MODE_SHUFFLE）传过来，
            // 需分流到 AVRCP REPEAT_STATUS 或 SHUFFLE_STATUS（对齐双框架 setRepeat/setShuffle）
            if (loopMode == AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE) {
                AvrcpCtMessage msg(AVRCP_MSG_SET_SHUFFLE, loopMode);
                SendMessage(msg);
            } else {
                AvrcpCtMessage msg(AVRCP_MSG_SET_REPEAT, loopMode);
                SendMessage(msg);
            }
        };
        void OnSeek(int64_t time) override{};
        void OnSetSpeed(double speed) override{};
        void OnToggleFavorite(const std::string &mediaId) override{};
        void OnMediaKeyEvent(const OHOS::MMI::KeyEvent &keyEvent) override{};
        void OnOutputDeviceChange(
            const int32_t connectionState, const OHOS::AVSession::OutputDeviceInfo &outputDeviceInfo) override{};
        void OnCommonCommand(const std::string &commonCommand, const OHOS::AAFwk::WantParams &commandArgs) override{};
        void OnSkipToQueueItem(int32_t itemId) override{};
        void OnAVCallAnswer() override{};
        void OnAVCallHangUp() override{};
        void OnAVCallToggleCallMute() override{};
        void OnPlayFromAssetId(int64_t assetId) override{};
        void OnCastDisplayChange(const AVSession::CastDisplayInfo &castDisplayInfo) override{};

    private:
        void SendMessage(const AvrcpCtMessage &msg)
        {
            HILOGI("msg = %{public}d, cmd = %{public}d", msg.what_, msg.arg1_);
            auto smPtr = sm_.lock();
            CHECK_AND_RETURN_LOG(smPtr, "smPtr not exist!");
            DoInAvrcpCtThread([msg, smPtr]() {
                HILOGI("enter send task.");
                smPtr->ProcessMessage(msg);
            });
        }
        std::weak_ptr<AvrcpCtStateMachine> sm_;
        bool flag;
    };

private:
    void InitAvSession();
    void InitAvSessionController();
    void SyncMetaDataToAVSession();
    void SyncPlaybackStateToAVSession();

    void ConnectCoverArt();
    void HandleCoverArtDownloaded(const std::vector<uint8_t> &imageData);
    void DownloadImageIfNeeded(std::shared_ptr<AvrcpCtMediaItem> track);
    void RemoveUnusedArtwork(const std::string& uuid);
    void ProcessTrackChanged(std::shared_ptr<AvrcpCtMediaItem> track);
    void ProcessPlayStatusChanged(int playStatus);
    void ProcessPlayPosChanged(int position);
    void ProcessAddressedPlayerChanged(int playerId);
    void ProcessCoverArtPsm(int psm);
    void ProcessImageDownloaded(const std::string& uuid);
    bool IsActive() const;

    void ClearMediaInfo();
    void HandleAbsVolumeRequest(int absVol, int label);
    void SetAbsVolume(int absVol);
    int GetAbsVolume() const;
    void DisconnectCoverArt();
    void ProcessVolumeChangedNotification(int volume);
    void ProcessRegisterAbsVolumeNotification(int label, int volume);
    void ProcessPrepare();
    void CleanupUnusedArtwork();

private:
    // 统一获取 CoverArtManager，避免各处重复 GetService()->GetCoverArtManager()
    std::shared_ptr<AvrcpCtCoverArtManager> GetCoverArtManager() const;

    std::shared_ptr<AvrcpCtAvSessionController> avSessionCtrl_{ nullptr };
    bool avSessionCtrlInitialized_ = false;
    std::string address_;
    std::mutex serviceCallbackLock_;
    AVSession::AVMetaData currMateData_;
    AVSession::AVPlaybackState currPlaybackstate_;
    int coverArtPsm_{};
    int mostRecentState_ = AVRCP_CT_STATE_DISCONNECTED;
    bool remoteControlConnected_ = false;

    int volumeNotificationLabel_ = -1;
    int64_t setVolumeTime_ = -1;
    std::atomic<int> volumeChangedNotificationsToIgnore_ {0};
    bool isVolumeEventRegistered_ = false;

    // 对齐双框架 removeMessages+sendMessageDelayed(ABS_VOL_TIMEOUT, 1000ms)：
    // 每次 SetAbsVolume 重置计时器，1s 内无新的 SetAbsVolume 则强制归零 ignore 计数器，
    // 防止 TG 频繁 SetAbsVolume 但无对应 VolumeChangedNotification 时计数器只增不减。
    std::unique_ptr<utility::Timer> absVolTimer_;

    int addressedPlayerId_ = 0;
    bool flag =  false;
    AVSession::AVMetaData currentMetaData_{};
    AVSession::AVPlaybackState currentPlaybackState_{};
    std::shared_ptr<AVSessionCallbackImpl> avSessionCallbackImpl_{nullptr};
    std::shared_ptr<AvrcpCtAVSession> avSession_{nullptr};
    std::shared_ptr<AvrcpCtPlayer> player_;
    std::vector<AvrcpCtPlayer> availablePlayerList_;
    BT_DISALLOW_COPY_AND_ASSIGN(AvrcpCtStateMachine);

    friend class AvrcpCtDisconnectedState;
    friend class AvrcpCtConnectingState;
    friend class AvrcpCtConnectedState;
    friend class AvrcpCtDisconnectingState;
};

class AvrcpCtState : public utility::StateMachine::State {
public:
    AvrcpCtState(const std::string &name, utility::StateMachine &stateMachine, int stateInt)
        : State(name, stateMachine), stateInt_(stateInt),
          stateMachine_(static_cast<AvrcpCtStateMachine &>(stateMachine))
    {}

    AvrcpCtState(const std::string &name, utility::StateMachine &stateMachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, stateMachine, parent), stateInt_(stateInt),
          stateMachine_(static_cast<AvrcpCtStateMachine &>(stateMachine))
    {}
    ~AvrcpCtState() override
    {}

    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_{AVRCP_CT_STATE_DISCONNECTED};
    AvrcpCtStateMachine &stateMachine_;
};

class AvrcpCtDisconnectedState : public AvrcpCtState {
public:
    AvrcpCtDisconnectedState(const std::string &name, utility::StateMachine &stateMachine)
        : AvrcpCtState(name, stateMachine, AVRCP_CT_STATE_DISCONNECTED){};
    ~AvrcpCtDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class AvrcpCtConnectingState : public AvrcpCtState {
public:
    AvrcpCtConnectingState(const std::string &name, utility::StateMachine &stateMachine)
        : AvrcpCtState(name, stateMachine, AVRCP_CT_STATE_CONNECTING)
    {}
    ~AvrcpCtConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class AvrcpCtConnectedState : public AvrcpCtState {
public:
    AvrcpCtConnectedState(const std::string &name, utility::StateMachine &stateMachine)
        : AvrcpCtState(name, stateMachine, AVRCP_CT_STATE_CONNECTED)
    {}
    ~AvrcpCtConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    void HandleDeviceStateActive(const AvrcpCtMessage& event);
    void HandleDeviceStateInactive(const AvrcpCtMessage& event);
    void HandlePassthrough(const AvrcpCtMessage& event);
    void HandleSetRepeat(const AvrcpCtMessage& event);
    void HandleSetShuffle(const AvrcpCtMessage& event);
    void HandleCurrentAppSettings(const AvrcpCtMessage& event);
    void HandleStackDisconnect(const AvrcpCtMessage& event);
    void HandleCoverArtPsm(const AvrcpCtMessage& event);
    void HandleTrackChanged(const AvrcpCtMessage& event);
    void HandlePlayStatusChanged(const AvrcpCtMessage& event);
    void HandlePlayPosChanged(const AvrcpCtMessage& event);
    void HandleSetAbsVol(const AvrcpCtMessage& event);
    void HandleRegisterAbsVol(const AvrcpCtMessage& event);
    void HandleAddressedPlayerChanged(const AvrcpCtMessage& event);
    void HandleNoOp(const AvrcpCtMessage& event);
    void HandleImageDownloaded(const AvrcpCtMessage& event);
    void HandleVolumeChanged(const AvrcpCtMessage& event);
    void HandlePrepare(const AvrcpCtMessage& event);
    void HandleCleanup(const AvrcpCtMessage& event);

private:
    bool IsHoldableKey(int cmd);
    void PassThrough(int cmd);
    void SetRepeat(int loopMode);
    void SetShuffle(int loopMode);
    int currentHoldKey_{0};
};

class AvrcpCtDisconnectingState : public AvrcpCtState {
public:
    AvrcpCtDisconnectingState(const std::string &name, utility::StateMachine &stateMachine)
        : AvrcpCtState(name, stateMachine, AVRCP_CT_STATE_DISCONNECTING)
    {}
    ~AvrcpCtDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_CT_STATE_MACHINE_H