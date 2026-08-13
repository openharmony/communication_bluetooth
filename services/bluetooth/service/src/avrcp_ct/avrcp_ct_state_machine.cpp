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
#define LOG_TAG "bt_service_avrcp_ct_machine"
#endif

#include "avrcp_ct_service.h"

#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>

#include "avrcp_ct_state_machine.h"
#include "common_util.h"
#include "thread_util.h"
#include "bluetooth_audio_framework_adapter.h"

namespace OHOS {
namespace bluetooth {

AvrcpCtStateMachine::AvrcpCtStateMachine(const std::string &address) : address_(address)
{
    HILOGI("Create AvrcpCtStateMachine!");
}

namespace {
const std::unordered_map<int, const char*> EVENT_NAME_MAP = {
    {STACK_CONNECT, "STACK_CONNECT"},
    {STACK_DISCONNECT, "STACK_DISCONNECT"},
    {DEVICE_STATE_ACTIVE, "DEVICE_STATE_ACTIVE"},
    {DEVICE_STATE_INACTIVE, "DEVICE_STATE_INACTIVE"},
    {AVRCP_MSG_PASSTHRU, "AVRCP_MSG_PASSTHRU"},
    {AVRCP_MSG_SET_REPEAT, "AVRCP_MSG_SET_REPEAT"},
    {AVRCP_MSG_SET_SHUFFLE, "AVRCP_MSG_SET_SHUFFLE"},
    {AVRCP_CT_GET_COVER_ART_PSM_EVT, "AVRCP_CT_GET_COVER_ART_PSM_EVT"},
    {AVRCP_CT_MSG_PROCESS_TRACK_CHANGED, "MESSAGE_PROCESS_TRACK_CHANGED"},
    {AVRCP_CT_MSG_PROCESS_PLAY_STATUS_CHANGED, "MESSAGE_PROCESS_PLAY_STATUS_CHANGED"},
    {AVRCP_CT_MSG_PROCESS_PLAY_POS_CHANGED, "MESSAGE_PROCESS_PLAY_POS_CHANGED"},
    {AVRCP_CT_MSG_PROCESS_SET_ABS_VOL_CMD, "MESSAGE_PROCESS_SET_ABS_VOL_CMD"},
    {AVRCP_CT_MSG_PROCESS_REGISTER_ABS_VOL_NOTIFICATION, "MESSAGE_PROCESS_REGISTER_ABS_VOL_NOTIFICATION"},
    {AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED, "MESSAGE_PROCESS_ADDRESSED_PLAYER_CHANGED"},
    {AVRCP_CT_MSG_PROCESS_AVAILABLE_PLAYER_CHANGED, "MESSAGE_PROCESS_AVAILABLE_PLAYER_CHANGED"},
    {AVRCP_CT_MSG_PROCESS_RECEIVED_COVER_ART_PSM, "MESSAGE_PROCESS_RECEIVED_COVER_ART_PSM"},
    {AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED, "MESSAGE_PROCESS_IMAGE_DOWNLOADED"},
    {AVRCP_CT_MSG_PROCESS_VOLUME_CHANGED_NOTIFICATION, "MESSAGE_PROCESS_VOLUME_CHANGED_NOTIFICATION"},
    {AVRCP_CT_MSG_PROCESS_PREPARE, "MESSAGE_PROCESS_PREPARE"},
    {CLEANUP, "CLEANUP"},
};
}  // namespace

std::string AvrcpCtStateMachine::GetEventName(int what)
{
    auto it = EVENT_NAME_MAP.find(what);
    return (it != EVENT_NAME_MAP.end()) ? it->second : "Unknown";
}

void AvrcpCtStateMachine::Init()
{
    HILOGI("Init");
    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<AvrcpCtDisconnectedState>(DISCONNECTED, *this);

    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<AvrcpCtConnectingState>(CONNECTING, *this);

    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<AvrcpCtConnectedState>(CONNECTED, *this);

    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<AvrcpCtDisconnectingState>(DISCONNECTING, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(connectedState);
    Move(disconnectingState);
    InitState(DISCONNECTED);

    if (!avSessionCtrlInitialized_) {
        InitAvSessionController();
        avSessionCtrlInitialized_ = true;
    }
    
    InitAvSession();

    // 在 make_shared 之后初始化，此时 shared_from_this() 可用
    auto self = shared_from_this();
    absVolTimer_ = std::make_unique<utility::Timer>([self]() {
        self->volumeChangedNotificationsToIgnore_ = 0;
        HILOGI("[AVRCP_CT_MACHINE] ABS_VOL_TIMEOUT fired, reset ignore count");
    });
}

void AvrcpCtStateMachine::CleanupAvSession()
{
    HILOGI("CleanupAvSession");
    if (avSession_) {
        avSession_->Deactivate();
        avSession_->DeleteSupportCommands();
        avSession_->Destroy();
        avSession_ = nullptr;
    }
}

void AvrcpCtStateMachine::RemoveFromService()
{
    auto service = AvrcpCtService::GetService();
    if (service) {
        service->RemoveStateMachine(address_);
    }
}

void AvrcpCtStateMachine::InitAvSessionController()
{
    HILOGI("start create avsession controller");
    avSessionCtrl_ = std::make_shared<AvrcpCtAvSessionController>(shared_from_this());
    avSessionCtrl_->Init();
    avSessionCtrl_->RegisterSessionListener();
}

static char g_SessionTag[] = "bluetooth";
static char g_BundleName[] = "bluetooth.ohos.avsession";
static char g_AbilityName[] = "bluetooth.ability";

void AvrcpCtStateMachine::InitAvSession()
{
    HILOGI("Init AvSession");
    OHOS::AppExecFwk::ElementName elementName;
    elementName.SetBundleName(g_BundleName);
    elementName.SetAbilityName(g_AbilityName);
    avSession_ = std::make_shared<AvrcpCtAVSession>();
    avSession_->CreateAVSession(g_SessionTag, 0, elementName);
    avSessionCallbackImpl_ = std::make_shared<AVSessionCallbackImpl>(shared_from_this());
    avSession_->RegisterCallback(avSessionCallbackImpl_);
    avSession_->AddSupportCommands();
}

void AvrcpCtStateMachine::CreateAvSessionController(const std::string &sessionId)
{
    if (avSessionCtrl_) {
        avSessionCtrl_->CreateAvSessionController(sessionId);
    }
}

void AvrcpCtStateMachine::SetAvsessionFilter()
{
    if (avSessionCtrl_) {
        avSessionCtrl_->SetAvsessionFilter();
    }
}

void AvrcpCtStateMachine::OnMetaDataChange(const AVSession::AVMetaData &data)
{
    bool trackChanged = CheckTrackStateChanged(data);
    if (!trackChanged) {
        HILOGE("MetaData not changed");
        return;
    }
    HILOGI("AvSession GetAVMetaData title %{public}s, Description %{public}s", data.GetTitle().c_str(), data.GetDescription().c_str());
}

bool AvrcpCtStateMachine::CheckTrackStateChanged(const AVSession::AVMetaData &data)
{
    bool isTitleChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_TITLE) &&
        currMateData_.GetTitle() != data.GetTitle();
    if (isTitleChange) {
        return true;
    }
    return false;
}

void AvrcpCtStateMachine::OnPlaybackStateChange(const AVSession::AVPlaybackState &state)
{
    HILOGI("AvSession GetAVPlaybackState state %{public}d, loopMode %{public}d", state.GetState(), state.GetLoopMode());
    // if ((state.GetState() == AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE) && (!flag)) {
    //     AVSession::AVControlCommand cmd;
    //     avSessionCallbackImpl_->OnPlay(cmd);
    //     flag = true;
    // } else if (state.GetState() != AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE) {
    //     flag = false;
    // }
}

bool AvrcpCtStateMachine::CheckPlaybackStateChanged(const AVSession::AVPlaybackState &state)
{
    bool isStateChange = state.GetState() != currPlaybackstate_.GetState();
    if (isStateChange) {
        return true;
    }
    return false;
}

int AvrcpCtStateMachine::GetDeviceState()
{
    const AvrcpCtState *state = static_cast<const AvrcpCtState *>(utility::StateMachine::GetState());
    if (!state) {
        return AVRCP_CT_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

int AvrcpCtStateMachine::GetState() const
{
    return mostRecentState_;
}

std::string AvrcpCtStateMachine::GetDeviceName() const
{
    return address_;
}

AVSession::AVMetaData &AvrcpCtStateMachine::GetAVMetaData()
{
    return currentMetaData_;
}

AVSession::AVPlaybackState &AvrcpCtStateMachine::GetAVPlaybackState()
{
    return currentPlaybackState_;
}

void AvrcpCtStateMachine::NotifyStateChanged()
{
    int state = GetDeviceState();
    if (mostRecentState_ == state) {
        return;
    }
    HILOGI("[AVRCP_CT_MACHINE] Connection state: %{public}d -> %{public}d", mostRecentState_, state);
    mostRecentState_ = state;
}

void AvrcpCtStateMachine::UpdatePlayerInfo(const std::string &playerName, int playerId, int playStatus)
{
    if (!player_) {
        player_ = std::make_shared<AvrcpCtPlayer>();
        player_->SetDeviceAddress(address_);
    }
    player_->SetPlayerName(playerName);
    player_->SetPlayerId(playerId);
    player_->SetPlayStatus(playStatus);
    HILOGI("Updated player info: name=%{public}s, id=%{public}d, status=%{public}d", playerName.c_str(), playerId,
           playStatus);
    SyncPlaybackStateToAVSession();
}

void AvrcpCtStateMachine::UpdateCurrentTrack(std::shared_ptr<AvrcpCtMediaItem> track)
{
    if (!player_) {
        player_ = std::make_shared<AvrcpCtPlayer>();
        player_->SetDeviceAddress(address_);
    }

    player_->UpdateCurrentTrack(track);
    HILOGI("Updated current track: %{public}s - %{public}s", track->title_.c_str(), track->artistName_.c_str());

    // 对齐双框架 downloadImageIfNeeded：使用 uuid 查缓存，命中则直接填充封面数据
    std::string uuid = track->GetCoverArtUuid();
    if (!uuid.empty()) {
        auto coverArtManager = GetCoverArtManager();
        if (coverArtManager && coverArtManager->IsCoverArtCached(address_, uuid)) {
            std::vector<uint8_t> imageData;
            if (coverArtManager->GetCoverArt(address_, uuid, imageData) && !imageData.empty()) {
                track->SetCoverArtData(imageData);
                currentMetaData_ = track->ToAVMetaData();
                SyncMetaDataToAVSession();
                return;
            }
        }
    }
    currentMetaData_ = track->ToAVMetaData();
    SyncMetaDataToAVSession();
}

void AvrcpCtStateMachine::ProcessGetCoverArtPsm()
{
    HILOGI("ProcessGetCoverArtPsm: address=%{public}s, psm=%{public}d", address_.c_str(), coverArtPsm_);
    // 对齐双框架：psm<=0 视为无效
    if (coverArtPsm_ <= 0) {
        HILOGE("Invalid PSM value");
        return;
    }

    auto coverArtManager = GetCoverArtManager();
    if (!coverArtManager) {
        HILOGE("CoverArtManager is nullptr");
        return;
    }

    if (coverArtManager->IsConnected(address_)) {
        return;
    }

    // 下载回调统一由 Service 启动时注册的全局回调按 deviceAddr 路由到对应 StateMachine
    // （见 AvrcpCtService::StartUp），此处不再重复注册以避免多设备并发时互相覆盖。

    coverArtManager->Connect(address_, coverArtPsm_);
    if (!coverArtManager->IsConnected(address_)) {
        HILOGE("address_ disconnected");
        return;
    }

    // 对齐双框架：使用 uuid 发起下载（handle<->uuid 映射在 CoverArtManager 内部维护）
    if (player_ && player_->HasCoverArt() && !player_->GetCoverArtHandle().empty()) {
        std::string uuid = player_->GetCurrentTrack()->GetCoverArtUuid();
        if (!uuid.empty()) {
            coverArtManager->DownloadCoverArt(address_, uuid);
        }
    }
}

void AvrcpCtStateMachine::SyncMetaDataToAVSession()
{
    if (avSession_) {
        avSession_->UpdateMetaData(currentMetaData_);
    }
}

void AvrcpCtStateMachine::SyncPlaybackStateToAVSession()
{
    if (avSession_) {
        avSession_->UpdatePlaybackState(currentPlaybackState_);
    }
}

// -----------------------------------------------Disconnected
void AvrcpCtDisconnectedState::Entry()
{
    HILOGI("[AVRCP_CT_MACHINE][DISCONNECTED] entry");
    if (stateMachine_.mostRecentState_ != AVRCP_CT_STATE_DISCONNECTED) {
        stateMachine_.ClearMediaInfo();
    }
    stateMachine_.NotifyStateChanged();
}

bool AvrcpCtDisconnectedState::Dispatch(const utility::Message &msg)
{
    HILOGI("[AVRCP_CT_MACHINE][Disconnected] EventName=%{public}s",
           AvrcpCtStateMachine::GetEventName(msg.what_).c_str());
    switch (msg.what_) {
        case AVRCP_CT_MSG_PROCESS_RECEIVED_COVER_ART_PSM:
            stateMachine_.coverArtPsm_ = msg.arg1_;
            break;
        case STACK_CONNECT:
            Transition(AvrcpCtStateMachine::CONNECTING);
            break;
        case DEVICE_STATE_ACTIVE:
        case DEVICE_STATE_INACTIVE:
            break;
        case CLEANUP:
            stateMachine_.ClearMediaInfo();
            break;
        default:
            break;
    }
    return true;
}

void AvrcpCtDisconnectedState::Exit()
{
    HILOGI("[AVRCP_CT_MACHINE][DISCONNECTED] exit");
}

// ------------------------------------------->Connecting
void AvrcpCtConnectingState::Entry()
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTING] entry");
    stateMachine_.NotifyStateChanged();
    Transition(AvrcpCtStateMachine::CONNECTED);
}

bool AvrcpCtConnectingState::Dispatch(const utility::Message &msg)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTING] EventName=%{public}s", AvrcpCtStateMachine::GetEventName(msg.what_).c_str());
    return true;
}

void AvrcpCtConnectingState::Exit()
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTING] exit");
}

// -----------------------------------------> Connected
void AvrcpCtConnectedState::Entry()
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] entry");
    if (stateMachine_.mostRecentState_ == AVRCP_CT_STATE_CONNECTING) {
        stateMachine_.NotifyStateChanged();
        stateMachine_.ConnectCoverArt();
    }
}

namespace {
using AvrcpCtEventHandler = void (AvrcpCtConnectedState::*)(const AvrcpCtMessage&);

const std::unordered_map<int, AvrcpCtEventHandler> CONNECTED_EVENT_HANDLERS = {
    {DEVICE_STATE_ACTIVE, &AvrcpCtConnectedState::HandleDeviceStateActive},
    {DEVICE_STATE_INACTIVE, &AvrcpCtConnectedState::HandleDeviceStateInactive},
    {AVRCP_MSG_PASSTHRU, &AvrcpCtConnectedState::HandlePassthrough},
    {AVRCP_MSG_SET_REPEAT, &AvrcpCtConnectedState::HandleSetRepeat},
    {AVRCP_MSG_SET_SHUFFLE, &AvrcpCtConnectedState::HandleSetShuffle},
    {STACK_DISCONNECT, &AvrcpCtConnectedState::HandleStackDisconnect},
    {AVRCP_CT_GET_COVER_ART_PSM_EVT, &AvrcpCtConnectedState::HandleCoverArtPsm},
    {AVRCP_CT_MSG_PROCESS_TRACK_CHANGED, &AvrcpCtConnectedState::HandleTrackChanged},
    {AVRCP_CT_MSG_PROCESS_PLAY_STATUS_CHANGED, &AvrcpCtConnectedState::HandlePlayStatusChanged},
    {AVRCP_CT_MSG_PROCESS_PLAY_POS_CHANGED, &AvrcpCtConnectedState::HandlePlayPosChanged},
    {AVRCP_CT_MSG_PROCESS_SET_ABS_VOL_CMD, &AvrcpCtConnectedState::HandleSetAbsVol},
    {AVRCP_CT_MSG_PROCESS_REGISTER_ABS_VOL_NOTIFICATION, &AvrcpCtConnectedState::HandleRegisterAbsVol},
    {AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED, &AvrcpCtConnectedState::HandleAddressedPlayerChanged},
    {AVRCP_CT_MSG_PROCESS_SUPPORTED_APPLICATION_SETTINGS, &AvrcpCtConnectedState::HandleNoOp},
    {AVRCP_CT_MSG_PROCESS_CURRENT_APPLICATION_SETTINGS, &AvrcpCtConnectedState::HandleCurrentAppSettings},
    {AVRCP_CT_MSG_PROCESS_AVAILABLE_PLAYER_CHANGED, &AvrcpCtConnectedState::HandleNoOp},
    {AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED, &AvrcpCtConnectedState::HandleImageDownloaded},
    {AVRCP_CT_MSG_PROCESS_VOLUME_CHANGED_NOTIFICATION, &AvrcpCtConnectedState::HandleVolumeChanged},
    {AVRCP_CT_MSG_PROCESS_PREPARE, &AvrcpCtConnectedState::HandlePrepare},
    {CLEANUP, &AvrcpCtConnectedState::HandleCleanup},
};
}  // namespace

bool AvrcpCtConnectedState::Dispatch(const utility::Message &msg)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] EventName=%{public}s", AvrcpCtStateMachine::GetEventName(msg.what_).c_str());
    AvrcpCtMessage &event = const_cast<AvrcpCtMessage &>(static_cast<const AvrcpCtMessage &>(msg));
    auto it = CONNECTED_EVENT_HANDLERS.find(event.what_);
    if (it != CONNECTED_EVENT_HANDLERS.end()) {
        (this->*(it->second))(event);
    }
    return true;
}

void AvrcpCtConnectedState::HandleDeviceStateActive(const AvrcpCtMessage& event)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] DEVICE_STATE_ACTIVE");
    if (stateMachine_.avSession_) {
        stateMachine_.avSession_->Activate();
        stateMachine_.avSession_->UpdateMetaData(stateMachine_.GetAVMetaData());
        stateMachine_.avSession_->UpdatePlaybackState(stateMachine_.GetAVPlaybackState());
    }
}

void AvrcpCtConnectedState::HandleDeviceStateInactive(const AvrcpCtMessage& event)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] DEVICE_STATE_INACTIVE, pause remote");
    if (stateMachine_.avSession_) {
        stateMachine_.avSession_->Deactivate();
    }
    AvrcpCtService::GetService()->SendPassThroughCommand(RawAddress(stateMachine_.GetDeviceName()),
                                                         PASS_THRU_CMD_ID_PAUSE, KEY_STATE_PRESSED);
    AvrcpCtService::GetService()->SendPassThroughCommand(RawAddress(stateMachine_.GetDeviceName()),
                                                         PASS_THRU_CMD_ID_PAUSE, KEY_STATE_RELEASED);
}

void AvrcpCtConnectedState::HandlePassthrough(const AvrcpCtMessage& event)
{
    PassThrough(event.arg1_);
}

void AvrcpCtConnectedState::HandleSetRepeat(const AvrcpCtMessage& event)
{
    SetRepeat(event.arg1_);
}

void AvrcpCtConnectedState::HandleSetShuffle(const AvrcpCtMessage& event)
{
    SetShuffle(event.arg1_);
}

void AvrcpCtConnectedState::HandleCurrentAppSettings(const AvrcpCtMessage& event)
{
    // TG 主动切换 repeat/shuffle 时，CT 同步到 AVSession loopMode（对齐双框架 onPlayerAppSettingChanged）
    uint8_t attrId = event.appSettingAttr_;
    uint8_t attrVal = event.appSettingVal_;
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] HandleCurrentAppSettings: attr=%{public}d, val=%{public}d", attrId, attrVal);

    int32_t loopMode = AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE;
    if (attrId == AVRCP_REPEAT_STATUS) {
        switch (attrVal) {
            case AVRCP_REPEAT_SINGLE_TRACK: loopMode = AVSession::AVPlaybackState::LOOP_MODE_SINGLE; break;
            case AVRCP_REPEAT_ALL_TRACK:    loopMode = AVSession::AVPlaybackState::LOOP_MODE_LIST;   break;
            case AVRCP_REPEAT_OFF:
            default:                        loopMode = AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE; break;
        }
    } else if (attrId == AVRCP_SHUFFLE_STATUS) {
        switch (attrVal) {
            case AVRCP_SHUFFLE_ALL_TRACK:   loopMode = AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE; break;
            case AVRCP_SHUFFLE_OFF:
            default:                        loopMode = AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE; break;
        }
    } else {
        return;
    }

    stateMachine_.currentPlaybackState_.SetLoopMode(loopMode);
    if (stateMachine_.IsActive() && stateMachine_.avSession_) {
        stateMachine_.avSession_->UpdatePlaybackState(stateMachine_.currentPlaybackState_);
    }
}

void AvrcpCtConnectedState::HandleStackDisconnect(const AvrcpCtMessage& event)
{
    Transition(AvrcpCtStateMachine::DISCONNECTING);
}

void AvrcpCtConnectedState::HandleCoverArtPsm(const AvrcpCtMessage& event)
{
    HILOGI("Received Cover Art PSM: %{public}d", event.arg1_);
    stateMachine_.ProcessCoverArtPsm(event.arg1_);
}

void AvrcpCtConnectedState::HandleTrackChanged(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessTrackChanged(event.track_);
}

void AvrcpCtConnectedState::HandlePlayStatusChanged(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessPlayStatusChanged(event.arg1_);
}

void AvrcpCtConnectedState::HandlePlayPosChanged(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessPlayPosChanged(event.arg1_);
}

void AvrcpCtConnectedState::HandleSetAbsVol(const AvrcpCtMessage& event)
{
    stateMachine_.HandleAbsVolumeRequest(event.arg1_, event.eventThreeParameter_);
}

void AvrcpCtConnectedState::HandleRegisterAbsVol(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessRegisterAbsVolumeNotification(event.arg1_, event.eventThreeParameter_);
}

void AvrcpCtConnectedState::HandleAddressedPlayerChanged(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessAddressedPlayerChanged(event.arg1_);
}

void AvrcpCtConnectedState::HandleNoOp(const AvrcpCtMessage& event)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] %{public}s", AvrcpCtStateMachine::GetEventName(event.what_).c_str());
}

void AvrcpCtConnectedState::HandleImageDownloaded(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessImageDownloaded(event.uuid_);
}

void AvrcpCtConnectedState::HandleVolumeChanged(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessVolumeChangedNotification(event.arg1_);
}

void AvrcpCtConnectedState::HandlePrepare(const AvrcpCtMessage& event)
{
    stateMachine_.ProcessPrepare();
}

void AvrcpCtConnectedState::HandleCleanup(const AvrcpCtMessage& event)
{
    stateMachine_.ClearMediaInfo();
}

bool AvrcpCtConnectedState::IsHoldableKey(int cmd)
{
    return (cmd == PASS_THRU_CMD_ID_REWIND || cmd == PASS_THRU_CMD_ID_FF);
}

void AvrcpCtConnectedState::PassThrough(int cmd)
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] msgPassThrough = %{public}d", cmd);
    if (currentHoldKey_ != 0) {
        AvrcpCtService::GetService()->SendPassThroughCommand(RawAddress(stateMachine_.GetDeviceName()), currentHoldKey_,
                                                             KEY_STATE_RELEASED);
        if (currentHoldKey_ == cmd) {
            currentHoldKey_ = 0;
            return;
        } else {
            currentHoldKey_ = 0;
        }
    }
    AvrcpCtService::GetService()->SendPassThroughCommand(RawAddress(stateMachine_.GetDeviceName()), cmd,
                                                         KEY_STATE_PRESSED);

    if (IsHoldableKey(cmd)) {
        currentHoldKey_ = cmd;
    } else {
        AvrcpCtService::GetService()->SendPassThroughCommand(RawAddress(stateMachine_.GetDeviceName()), cmd,
                                                             KEY_STATE_RELEASED);
    }
}

void AvrcpCtConnectedState::SetRepeat(int loopMode)
{
    // AVSession LoopMode → AVRCP REPEAT_STATUS 值（对齐双框架 mapAvrcpPlayerSettingstoBTattribVal）
    uint8_t repeatVal = AVRCP_REPEAT_OFF;
    switch (loopMode) {
        case AVSession::AVPlaybackState::LOOP_MODE_SINGLE:
            repeatVal = AVRCP_REPEAT_SINGLE_TRACK;
            break;
        case AVSession::AVPlaybackState::LOOP_MODE_LIST:
            repeatVal = AVRCP_REPEAT_ALL_TRACK;
            break;
        case AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE:
        default:
            repeatVal = AVRCP_REPEAT_OFF;
            break;
    }
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] SetRepeat loopMode=%{public}d, repeatVal=%{public}d", loopMode, repeatVal);
    AvrcpCtService::GetService()->SetPlayerAppSettingCurrentValue(
        RawAddress(stateMachine_.GetDeviceName()), {AVRCP_REPEAT_STATUS}, {repeatVal});
}

void AvrcpCtConnectedState::SetShuffle(int loopMode)
{
    // AVSession LOOP_MODE_SHUFFLE → AVRCP SHUFFLE_STATUS 值（对齐双框架 mapAvrcpPlayerSettingstoBTattribVal）
    // 车机 UI 只有"shuffle 开/关"两种语义，映射为 ALL_TRACK_SHUFFLE
    uint8_t shuffleVal = AVRCP_SHUFFLE_ALL_TRACK;
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] SetShuffle loopMode=%{public}d, shuffleVal=%{public}d", loopMode, shuffleVal);
    AvrcpCtService::GetService()->SetPlayerAppSettingCurrentValue(
        RawAddress(stateMachine_.GetDeviceName()), {AVRCP_SHUFFLE_STATUS}, {shuffleVal});
}

void AvrcpCtConnectedState::Exit()
{
    HILOGI("[AVRCP_CT_MACHINE][CONNECTED] exit");
}

// -------------------------------> DISCONNECTING
void AvrcpCtDisconnectingState::Entry()
{
    HILOGI("[AVRCP_CT_MACHINE][DISCONNECTING] entry");
    stateMachine_.DisconnectCoverArt();
    stateMachine_.NotifyStateChanged();
    Transition(AvrcpCtStateMachine::DISCONNECTED);
}

bool AvrcpCtDisconnectingState::Dispatch(const utility::Message &msg)
{
    HILOGI("[AVRCP_CT_MACHINE][DISCONNECTING]...");
    return true;
}

void AvrcpCtDisconnectingState::Exit()
{
    HILOGI("[AVRCP_CT_MACHINE][DISCONNECTING] exit");
}

// -----------------------------------------> StateMachine private methods

void AvrcpCtStateMachine::HandleCoverArtDownloaded(const std::vector<uint8_t> &imageData)
{
    if (imageData.empty() || !player_ || !player_->GetCurrentTrack()) {
        return;
    }
    player_->GetCurrentTrack()->SetCoverArtData(imageData);
    currentMetaData_ = player_->GetCurrentTrack()->ToAVMetaData();
    if (IsActive()) {
        SyncMetaDataToAVSession();
    }
}

void AvrcpCtStateMachine::ConnectCoverArt()
{
    HILOGI("[AVRCP_CT_MACHINE] ConnectCoverArt, psm=%{public}d", coverArtPsm_);
    // 对齐双框架 mCoverArtPsm != 0：psm<=0 视为无效（0 是未初始化默认值，负数非法）
    if (coverArtPsm_ <= 0) {
        return;
    }
    auto coverArtManager = GetCoverArtManager();
    if (!coverArtManager || coverArtManager->IsConnected(address_)) {
        return;
    }
    // 下载回调统一由 Service 启动时注册的全局回调按 deviceAddr 路由到对应 StateMachine
    // （见 AvrcpCtService::StartUp），此处不再重复注册以避免多设备并发时互相覆盖。
    coverArtManager->Connect(address_, coverArtPsm_);
}

void AvrcpCtStateMachine::DownloadImageIfNeeded(std::shared_ptr<AvrcpCtMediaItem> track)
{
    auto coverArtManager = GetCoverArtManager();
    if (!track || !coverArtManager) {
        return;
    }
    // 对齐双框架 downloadImageIfNeeded：全程使用 uuid（由 Service 在 onTrackChanged 时
    // 通过 GetUuidForHandle 生成并 SetCoverArtUuid 写入 track），handle<->uuid 映射在
    // CoverArtManager 内部维护，DownloadCoverArt 内部通过 GetHandleForUuid 还原 handle。
    std::string uuid = track->GetCoverArtUuid();
    if (uuid.empty()) {
        return;
    }
    if (coverArtManager->IsCoverArtCached(address_, uuid)) {
        std::vector<uint8_t> imageData;
        if (coverArtManager->GetCoverArt(address_, uuid, imageData) && !imageData.empty()) {
            // 已缓存：直接填充封面数据，无需重新下载
            track->SetCoverArtData(imageData);
            return;
        }
    }
    // 未缓存：发起异步下载（内部 GetHandleForUuid -> GetImageProperties -> GetImage）
    coverArtManager->DownloadCoverArt(address_, uuid);
}

void AvrcpCtStateMachine::RemoveUnusedArtwork(const std::string &uuid)
{
    if (uuid.empty()) {
        return;
    }
    auto coverArtManager = GetCoverArtManager();
    if (!coverArtManager) {
        return;
    }
    std::string currentTrackUuid;
    if (player_ && player_->GetCurrentTrack()) {
        currentTrackUuid = player_->GetCurrentTrack()->GetCoverArtUuid();
    }
    HILOGI("[AVRCP_CT_MACHINE] RemoveUnusedArtwork, uuid=%{public}s, currentUuid=%{public}s",
           uuid.c_str(), currentTrackUuid.c_str());
    // 仅删除不再被当前曲使用的指定 uuid 缓存，避免整设备清空导致其他曲目缓存被误删
    if (uuid != currentTrackUuid) {
        coverArtManager->RemoveImage(address_, uuid);
    }
}

void AvrcpCtStateMachine::CleanupUnusedArtwork()
{
    HILOGI("[AVRCP_CT_MACHINE] CleanupUnusedArtwork");
    auto coverArtManager = GetCoverArtManager();
    if (!coverArtManager) {
        return;
    }
    coverArtManager->ClearDeviceCache(address_);
}

void AvrcpCtStateMachine::ProcessTrackChanged(std::shared_ptr<AvrcpCtMediaItem> track)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessTrackChanged");
    std::string previousTrackUuid;
    if (player_ && player_->GetCurrentTrack()) {
        HILOGI("[AVRCP_CT_MACHINE] ProcessTrackChanged player_ is not null");
        previousTrackUuid = player_->GetCurrentTrack()->GetCoverArtUuid();
        HILOGI("[AVRCP_CT_MACHINE] ProcessTrackChanged previousTrackUuid %{public}s", previousTrackUuid.c_str());
    }

    if (track) {
        DownloadImageIfNeeded(track);
        if (!player_) {
            player_ = std::make_shared<AvrcpCtPlayer>();
            player_->SetDeviceAddress(address_);
        }
        player_->UpdateCurrentTrack(track);
        currentMetaData_ = track->ToAVMetaData();
    } else {
        if (player_) {
            player_->UpdateCurrentTrack(nullptr);
        }
        AVSession::AVMetaData emptyMeta;
        currentMetaData_ = emptyMeta;
    }
    if (IsActive()) {
        SyncMetaDataToAVSession();
    }
    if (!previousTrackUuid.empty()) {
        RemoveUnusedArtwork(previousTrackUuid);
    }
}

void AvrcpCtStateMachine::ProcessPlayStatusChanged(int playStatus)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessPlayStatusChanged, status=%{public}d", playStatus);
    if (player_) {
        player_->SetPlayStatus(playStatus);
        if (player_->GetCurrentTrack()) {
            currentPlaybackState_ = player_->ToAVPlaybackState();
        }
    }
    if (!IsActive()) {
        AvrcpCtService::GetService()->RequestRemotePause(RawAddress(address_));
        return;
    }
    SyncPlaybackStateToAVSession();
}

void AvrcpCtStateMachine::ProcessPlayPosChanged(int position)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessPlayPosChanged, position=%{public}d", position);
    if (player_ && position >= 0) {
        player_->SetPlayTime(position);
        if (player_->GetCurrentTrack()) {
            currentPlaybackState_ = player_->ToAVPlaybackState();
        }
    }
    if (IsActive()) {
        SyncPlaybackStateToAVSession();
    }
}

void AvrcpCtStateMachine::HandleAbsVolumeRequest(int absVol, int label)
{
    HILOGI("[AVRCP_CT_MACHINE] HandleAbsVolumeRequest: absVol=%{public}d, label=%{public}d", absVol, label);
    volumeChangedNotificationsToIgnore_++;
    // 对齐双框架 removeMessages+sendMessageDelayed(ABS_VOL_TIMEOUT, 1000ms)
    absVolTimer_->Stop();
    absVolTimer_->Start(ABS_VOL_TIMEOUT_MILLIS);
    SetAbsVolume(absVol);
    setVolumeTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
    AvrcpCtService::GetService()->SendAbsVolumeResponse(RawAddress(address_), absVol, label);
}

void AvrcpCtStateMachine::ProcessRegisterAbsVolumeNotification(int label, int volume)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessRegisterAbsVolumeNotification, label=%{public}d, volume=%{public}d", label,
           volume);
    volumeNotificationLabel_ = label;
    isVolumeEventRegistered_ = true;
    AvrcpCtService::GetService()->SendRegisterAbsVolResponse(RawAddress(address_), NOTIFICATION_RSP_TYPE_INTERIM,
                                                             GetAbsVolume(), label);
}

void AvrcpCtStateMachine::ProcessAddressedPlayerChanged(int playerId)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessAddressedPlayerChanged, playerId=%{public}d", playerId);
    int oldPlayerId = addressedPlayerId_;
    addressedPlayerId_ = playerId;

    if (!player_) {
        player_ = std::make_shared<AvrcpCtPlayer>();
        player_->SetDeviceAddress(address_);
    }
    player_->SetPlayerId(playerId);

    AvrcpCtService::GetService()->RequestCurrentMetadata(RawAddress(address_));
    AvrcpCtService::GetService()->RequestPlaybackState(RawAddress(address_));

    if (IsActive()) {
        SyncMetaDataToAVSession();
        SyncPlaybackStateToAVSession();
    }
}

void AvrcpCtStateMachine::ProcessCoverArtPsm(int psm)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessCoverArtPsm, psm=%{public}d", psm);
    coverArtPsm_ = psm;
    ConnectCoverArt();
}

void AvrcpCtStateMachine::ProcessImageDownloaded(const std::string &uuid)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessImageDownloaded, uuid=%{public}s", uuid.c_str());
    auto coverArtManager = GetCoverArtManager();
    if (!coverArtManager) {
        return;
    }
    std::vector<uint8_t> imageData;
    if (!uuid.empty()) {
        coverArtManager->GetCoverArt(address_, uuid, imageData);
    }
    // 对齐双框架 mAddressedPlayer.notifyImageDownload(uuid, uri)：
    // 仅当下载的 uuid 与当前曲目的 coverArtUuid 匹配时才把封面设给当前曲，
    // 避免下载的图片属于其他曲目却错误地填充到当前曲。
    bool addedArtwork = false;
    if (player_ && player_->GetCurrentTrack() && !imageData.empty()
        && uuid == player_->GetCurrentTrack()->GetCoverArtUuid()) {
        player_->GetCurrentTrack()->SetCoverArtData(imageData);
        addedArtwork = true;
        if (IsActive()) {
            currentMetaData_ = player_->GetCurrentTrack()->ToAVMetaData();
            SyncMetaDataToAVSession();
        }
    }
    // 下载的图片未被当前曲使用则清理（单框架暂无浏览树，仅按 uuid 精细删除）
    if (!addedArtwork) {
        RemoveUnusedArtwork(uuid);
    }
}

void AvrcpCtStateMachine::ProcessPrepare()
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessPrepare");
    if (player_ && player_->GetCurrentTrack()) {
        currentMetaData_ = player_->GetCurrentTrack()->ToAVMetaData();
        if (avSession_) {
            avSession_->UpdateMetaData(currentMetaData_);
        }
    }
    if (avSession_) {
        avSession_->UpdatePlaybackState(currentPlaybackState_);
    }
}

bool AvrcpCtStateMachine::IsActive() const
{
    auto service = AvrcpCtService::GetService();
    if (!service) {
        return false;
    }
    return address_ == service->GetActiveDevice();
}

std::shared_ptr<AvrcpCtCoverArtManager> AvrcpCtStateMachine::GetCoverArtManager() const
{
    auto service = AvrcpCtService::GetService();
    return service ? service->GetCoverArtManager() : nullptr;
}

void AvrcpCtStateMachine::ClearMediaInfo()
{
    HILOGI("[AVRCP_CT_MACHINE] ClearMediaInfo");
    if (player_) {
        player_->SetPlayStatus(AVRC_PLAY_STATUS_PAUSED);
        if (player_->GetCurrentTrack()) {
            currentPlaybackState_ = player_->ToAVPlaybackState();
            SyncPlaybackStateToAVSession();
            std::string previousTrackUuid = player_->GetCurrentTrack()->GetCoverArtUuid();
            if (!previousTrackUuid.empty()) {
                RemoveUnusedArtwork(previousTrackUuid);
            }
        }
        player_->UpdateCurrentTrack(nullptr);
    }
    currentMetaData_ = AVSession::AVMetaData{};
    SyncMetaDataToAVSession();
    if (avSession_) {
        avSession_->Deactivate();
        avSession_->DeleteSupportCommands();
        avSession_->Destroy();
        avSession_ = nullptr;
    }
}

void AvrcpCtStateMachine::SetAbsVolume(int absVol)
{
    HILOGI("[AVRCP_CT_MACHINE] SetAbsVolume: absVol=%{public}d", absVol);
    int maxVolume = BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType::VOLUME_MUSIC,
                                                                   BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
    int curVolume = BluetoothAudioFrameworkAdapter::GetStreamVolume(STREAM_MUSIC);
    int reqVolume = (maxVolume * absVol) / ABS_VOL_BASE;
    HILOGI("[AVRCP_CT_MACHINE] SetAbsVolume: reqVolume=%{public}d, curVolume=%{public}d, maxVolume=%{public}d",
           reqVolume, curVolume, maxVolume);
    if (reqVolume != curVolume) {
        BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(address_, reqVolume, true);
    }
    setVolumeTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
}

int AvrcpCtStateMachine::GetAbsVolume() const
{
    int maxVolume = BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType::VOLUME_MUSIC,
                                                                   BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
    int curVolume = BluetoothAudioFrameworkAdapter::GetStreamVolume(STREAM_MUSIC);
    int absVolume = (curVolume * ABS_VOL_BASE) / maxVolume;
    HILOGI("[AVRCP_CT_MACHINE] GetAbsVolume: absVolume=%{public}d, curVolume=%{public}d, maxVolume=%{public}d",
           absVolume, curVolume, maxVolume);
    return absVolume;
}

void AvrcpCtStateMachine::DisconnectCoverArt()
{
    HILOGI("[AVRCP_CT_MACHINE] DisconnectCoverArt");
    auto coverArtManager = GetCoverArtManager();
    if (coverArtManager) {
        coverArtManager->Disconnect(address_);
    }
}

void AvrcpCtStateMachine::ProcessVolumeChangedNotification(int volume)
{
    HILOGI("[AVRCP_CT_MACHINE] ProcessVolumeChangedNotification, volume=%{public}d", volume);
    auto currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
    if (setVolumeTime_ != -1 && (currentTime - setVolumeTime_ < VOLUME_CHANGE_IGNORED_TIMEDIFF_MS)) {
        HILOGI("[AVRCP_CT_MACHINE] Ignore volume change notification within timeout");
        return;
    }
    if (volumeChangedNotificationsToIgnore_ > 0) {
        volumeChangedNotificationsToIgnore_--;
        HILOGI("[AVRCP_CT_MACHINE] Ignored vol change, remaining=%{public}d", volumeChangedNotificationsToIgnore_.load());
        return;
    }
    if (isVolumeEventRegistered_) {
        AvrcpCtService::GetService()->SendRegisterAbsVolResponse(RawAddress(address_), NOTIFICATION_RSP_TYPE_CHANGED,
                                                                 GetAbsVolume(), volumeNotificationLabel_);
        isVolumeEventRegistered_ = false;
    }
}

std::shared_ptr<AvrcpCtMediaItem> AvrcpCtStateMachine::GetCurrentTrack()
{
    if (player_) {
        return player_->GetCurrentTrack();
    }
    return nullptr;
}

const std::vector<AvrcpCtPlayer> AvrcpCtStateMachine::GetAvailablePlayers()
{
    return availablePlayerList_;
}

}  // namespace bluetooth
}  // namespace OHOS
