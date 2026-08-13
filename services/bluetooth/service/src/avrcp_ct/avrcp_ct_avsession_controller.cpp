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
#define LOG_TAG "avrcp_ct_avsession_controller"
#endif

#include "avrcp_ct_avsession_controller.h"
#include "avrcp_ct_state_machine.h"
#include "avsession_errors.h"

namespace OHOS {
namespace bluetooth {

void AvrcpCtAvSessionController::Init()
{
    avControllerObserver_ = std::make_shared<AVControllerObserverImpl>();
    avSessionObserver_ = std::make_shared<AVSessionObserverImpl>(weak_from_this());
}

void AvrcpCtAvSessionController::RegisterSessionListener()
{
    auto res = AVSession::AVSessionManager::GetInstance().RegisterSessionListener(
        std::static_pointer_cast<AVSession::SessionListener>(avSessionObserver_));
    if (res != AVSession::AVSESSION_SUCCESS) {
        HILOGE("RegisterSessionListener fail");
    } else {
        HILOGI("RegisterSessionListener success");
    }
}

void AvrcpCtAvSessionController::OnSessionCreate(const AVSession::AVSessionDescriptor &descriptor)
{
    HILOGI("[AvrcpCtAvSessionController] OnSessionCreate, sessionId:%{public}s, isTopSession_:%{public}d",
        descriptor.sessionId_.c_str(), descriptor.isTopSession_);
    if (isControllerCreated_) {
        HILOGI("Controller already created, skip");
        return;
    }
    CreateAvSessionController(descriptor.sessionId_);
}

void AvrcpCtAvSessionController::OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor)
{
    HILOGI("[AvrcpCtAvSessionController] OnTopSessionChange, sessionId:%{public}s", descriptor.sessionId_.c_str());
    if (isControllerCreated_) {
        HILOGI("Controller already created, skip");
        return;
    }
    CreateAvSessionController(descriptor.sessionId_);
}

void AvrcpCtAvSessionController::CreateAvSessionController(const std::string &sessionId)
{
    HILOGI("CreateAvSessionController, sessionId:%{public}s", sessionId.c_str());
    if (isControllerCreated_) {
        HILOGI("Controller already created, skip");
        return;
    }
    auto ret = AVSession::AVSessionManager::GetInstance().CreateController(sessionId, avSessionController_);
    if ((ret != AVSession::AVSESSION_SUCCESS && ret != AVSession::ERR_CONTROLLER_IS_EXIST) ||
            avSessionController_ == nullptr) {
        HILOGE("AvSession CreateController failed, ret(%{public}d)", ret);
        return;
    }
    isControllerCreated_ = true;
    avSessionController_->RegisterCallback(avControllerObserver_);
    avControllerObserver_->SetMetaDataCallback([this](const AVSession::AVMetaData &data) {
        OnMetaDataChange(data);
    });
    avControllerObserver_->SetPlaybackStateCallback([this](const AVSession::AVPlaybackState &state) {
        OnPlaybackStateChange(state);
    });
    SetAvsessionFilter();
    HILOGW("CreateAvSessionController ok.");
}

void AvrcpCtAvSessionController::SetAvsessionFilter()
{
    HILOGI("SetAvsessionFilter");
    AVSession::AVPlaybackState::PlaybackStateMaskType filter;
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_STATE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_SPEED);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_POSITION);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_BUFFERED_TIME);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_LOOP_MODE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_ACTIVE_ITEM_ID);
    avSessionController_->SetPlaybackFilter(filter);
    AVSession::AVMetaData::MetaMaskType metaFilter;
    metaFilter.set(AVSession::AVMetaData::META_KEY_ASSET_ID);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ARTIST);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ALBUM);
    metaFilter.set(AVSession::AVMetaData::META_KEY_TITLE);
    metaFilter.set(AVSession::AVMetaData::META_KEY_SINGLE_LYRIC_TEXT);
    metaFilter.set(AVSession::AVMetaData::META_KEY_MEDIA_IMAGE);
    avSessionController_->SetMetaFilter(metaFilter);
}

void AvrcpCtAvSessionController::OnMetaDataChange(const AVSession::AVMetaData &data)
{
    auto stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        HILOGE("[AvrcpCtAvSessionController] stateMachine is null in OnMetaDataChange");
        return;
    }
    stateMachine->OnMetaDataChange(data);
}

void AvrcpCtAvSessionController::OnPlaybackStateChange(const AVSession::AVPlaybackState &state)
{
    auto stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        HILOGE("[AvrcpCtAvSessionController] stateMachine is null in OnPlaybackStateChange");
        return;
    }
    stateMachine->OnPlaybackStateChange(state);
}

void AvrcpCtAvSessionController::CheckTrackStateChanged(
    const AVSession::AVMetaData &mateData, const AVSession::AVPlaybackState &playbackState)
{
    if (playbackState.GetState() != AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        return;
    }
    HILOGI("[AvrcpCtAvSessionController] CheckTrackStateChanged");
}

void AvrcpCtAvSessionController::CheckPlaybackStateChanged(const AVSession::AVPlaybackState &playbackState)
{
    if (playbackState.GetState() != AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        HILOGI("[AvrcpCtAvSessionController] CheckPlaybackStateChanged, not playing");
        return;
    }
    HILOGI("[AvrcpCtAvSessionController] CheckPlaybackStateChanged");
}

void AvrcpCtAvSessionController::Destroy()
{
    if (avSessionController_ != nullptr) {
        avSessionController_->Destroy();
        avSessionController_ = nullptr;
    }
    isControllerCreated_ = false;
}

} // namespace bluetooth
} // namespace OHOS