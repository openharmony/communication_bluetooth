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
#define LOG_TAG "avrcp_ct_player"
#endif

#include "avrcp_ct_player.h"
#include "avrcp_ct_media_item.h"
#include "bt_def.h"

namespace OHOS {
namespace bluetooth {

AvrcpCtPlayer::AvrcpCtPlayer()
{}

AVSession::AVPlaybackState AvrcpCtPlayer::ToAVPlaybackState() const
{
    AVSession::AVPlaybackState state;
    int32_t avState = AVSession::AVPlaybackState::PLAYBACK_STATE_IDLE;
    switch (playStatus_) {
        case AVRC_PLAY_STATUS_STOPPED:
            avState = AVSession::AVPlaybackState::PLAYBACK_STATE_STOP;
            break;
        case AVRC_PLAY_STATUS_PLAYING:
        case AVRC_PLAY_STATUS_FWD_SEEK:
            avState = AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY;
            break;
        case AVRC_PLAY_STATUS_PAUSED:
        case AVRC_PLAY_STATUS_REV_SEEK:
            avState = AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE;
            break;
        default:
            avState = AVSession::AVPlaybackState::PLAYBACK_STATE_IDLE;
            break;
    }
    state.SetState(avState);
    state.SetSpeed(playSpeed_);

    if (playTime_ > 0 && duration_ > 0 && playTime_ <= duration_) {
        AVSession::AVPlaybackState::Position pos;
        pos.elapsedTime_ = playTime_;
        pos.updateTime_ = playTime_;
        state.SetPosition(pos);
    }

    return state;
}

void AvrcpCtPlayer::SetDeviceAddress(const std::string& address)
{
    deviceAddr_ = address;
}

std::string AvrcpCtPlayer::GetDeviceAddress() const
{
    return deviceAddr_;
}

void AvrcpCtPlayer::SetPlayerId(int id)
{
    playerId_ = id;
}

int AvrcpCtPlayer::GetPlayerId() const
{
    return playerId_;
}

void AvrcpCtPlayer::SetPlayerName(const std::string& name)
{
    playerName_ = name;
}

std::string AvrcpCtPlayer::GetPlayerName() const
{
    return playerName_;
}

void AvrcpCtPlayer::SetPlayerType(int type)
{
    playerType_ = type;
}

int AvrcpCtPlayer::GetPlayerType() const
{
    return playerType_;
}

void AvrcpCtPlayer::SetPlayStatus(int status)
{
    playStatus_ = status;
    // 对齐双框架 AvrcpPlayer.setPlayStatus：按状态设播放速度，供 AVSession 读取
    // 车机不支持 FF/REWIND，仅处理 PLAYING/STOPPED/PAUSED；FWD/REV_SEEK 若出现按双框架语义设 3/-3
    switch (playStatus_) {
        case AVRC_PLAY_STATUS_PLAYING:
            playSpeed_ = 1.0f;
            break;
        case AVRC_PLAY_STATUS_STOPPED:
        case AVRC_PLAY_STATUS_PAUSED:
            playSpeed_ = 1.0f;
            break;
        case AVRC_PLAY_STATUS_FWD_SEEK:
            playSpeed_ = 3.0f;
            break;
        case AVRC_PLAY_STATUS_REV_SEEK:
            playSpeed_ = 1.0f;
            break;
        default:
            playSpeed_ = 1.0f;
            break;
    }
}

int AvrcpCtPlayer::GetPlayStatus() const
{
    return playStatus_;
}

void AvrcpCtPlayer::SetPlayPosition(int64_t position)
{
    playPosition_ = position;
}

int64_t AvrcpCtPlayer::GetPlayPosition() const
{
    return playPosition_;
}

void AvrcpCtPlayer::SetPlayTime(int64_t playTime)
{
    playTime_ = playTime;
    playTimeUpdate_ = 0;
}

int64_t AvrcpCtPlayer::GetPlayTime() const
{
    return playTime_;
}

void AvrcpCtPlayer::SetPlaySpeed(float speed)
{
    playSpeed_ = speed;
}

float AvrcpCtPlayer::GetPlaySpeed() const
{
    return playSpeed_;
}

void AvrcpCtPlayer::SetDuration(int64_t duration)
{
    duration_ = duration;
}

int64_t AvrcpCtPlayer::GetDuration() const
{
    return duration_;
}

void AvrcpCtPlayer::UpdateCurrentTrack(std::shared_ptr<AvrcpCtMediaItem> track)
{
    currentTrack_ = track;
}

std::shared_ptr<AvrcpCtMediaItem> AvrcpCtPlayer::GetCurrentTrack() const
{
    return currentTrack_;
}

bool AvrcpCtPlayer::HasCoverArt() const
{
    if (!currentTrack_) {
        return false;
    }
    return currentTrack_->HasCoverArt();
}

std::string AvrcpCtPlayer::GetCoverArtHandle() const
{
    if (!currentTrack_) {
        return "";
    }
    return currentTrack_->GetCoverArtHandle();
}

bool AvrcpCtPlayer::NotifyImageDownload(const std::string& uuid, const std::string& uri)
{
    if (uuid.empty() || !currentTrack_) {
        return false;
    }
    if (uuid == currentTrack_->GetCoverArtUuid()) {
        return true;
    }
    return false;
}

void AvrcpCtPlayer::SetSupportedFeatures(const std::vector<uint8_t>& features)
{
    playerFeatures_ = features;
}

bool AvrcpCtPlayer::SupportsFeature(int featureId) const
{
    if (featureId < 0 || (playerFeatures_.size() > (INT32_MAX / 8)) ||
        (featureId >= static_cast<int>(playerFeatures_.size() * 8))) {
        return false;
    }
    int byteNumber = featureId / 8;
    int bitMask = 1 << (featureId % 8);
    if (byteNumber >= static_cast<int>(playerFeatures_.size())) {
        return false;
    }
    return (playerFeatures_[byteNumber] & bitMask) != 0;
}

void AvrcpCtPlayer::SetSupportedFeature(int featureId)
{
    if (featureId < 0) {
        return;
    }
    int byteNumber = featureId / 8;
    int bitMask = 1 << (featureId % 8);
    if (byteNumber >= static_cast<int>(playerFeatures_.size())) {
        playerFeatures_.resize(byteNumber + 1, 0);
    }
    playerFeatures_[byteNumber] |= bitMask;
}

}  // namespace bluetooth
}  // namespace OHOS