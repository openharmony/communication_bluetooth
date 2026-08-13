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
#define LOG_TAG "bt_service_avrcp_ct_avsession"
#endif

#include "avrcp_ct_avsession.h"
#include "avsession_errors.h"
namespace OHOS {
namespace bluetooth {

AvrcpCtAVSession::AvrcpCtAVSession() {}

void AvrcpCtAVSession::CreateAVSession(const std::string &tag, int32_t type, const AppExecFwk::ElementName &elementName)
{
    HILOGI("enter");
    if (avSession_) {
        return;
    }

    avSession_ = AVSession::AVSessionManager::GetInstance().CreateSession(tag, type, elementName);
    if (!avSession_) {
        HILOGE("avsession create failed!");
        return;
    }
    HILOGI("avsession create success!");
    return;
}

void AvrcpCtAVSession::Destroy()
{
    if (!avSession_) {
        return;
    }
    avSession_->Destroy();
    avSession_ = nullptr;
    isActive_ = false;
}

void AvrcpCtAVSession::RegisterCallback(std::shared_ptr<AVSession::AVSessionCallback> callback)
{
    CHECK_AND_RETURN_LOG(avSession_, "avsession is nullptr");
    if (avSession_->RegisterCallback(callback) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("Registercallback failed");
        return;
    }
    HILOGI("RegisterCallback success!");
}

void AvrcpCtAVSession::AddSupportCommands()
{
    CHECK_AND_RETURN_LOG(avSession_, "avsession is nullptr");
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PAUSE);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_STOP);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY_NEXT);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY_PREVIOUS);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_FAST_FORWARD);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_REWIND);
    avSession_->AddSupportCommand(AVSession::AVControlCommand::SESSION_CMD_SET_LOOP_MODE);
    HILOGI("AddSupportCommands success!");
}

void AvrcpCtAVSession::DeleteSupportCommands()
{
    CHECK_AND_RETURN_LOG(avSession_, "avsession is nullptr");
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PAUSE);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_STOP);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY_NEXT);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_PLAY_PREVIOUS);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_FAST_FORWARD);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_REWIND);
    avSession_->DeleteSupportCommand(AVSession::AVControlCommand::SESSION_CMD_SET_LOOP_MODE);
    HILOGI("DeleteSupportCommands success!");
}

void AvrcpCtAVSession::Activate()
{
    if (!isActive_) {
        CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
        if (avSession_->Activate() != AVSession::AVSESSION_SUCCESS) {
            HILOGE("Activate failed");
            return;
        }
        HILOGI("active avsession success!");
        isActive_ = true;
    }
}

void AvrcpCtAVSession::Deactivate()
{
    if (!isActive_) {
        return;
    }
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
    if (avSession_->Deactivate() != AVSession::AVSESSION_SUCCESS) {
        HILOGE("Deactivate failed");
        return;
    }
    HILOGI("deactivate avsession success!");
    isActive_ = false;
}

void AvrcpCtAVSession::UpdateMetaData(const AVSession::AVMetaData &meta)
{
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
    if (avSession_->SetAVMetaData(meta) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("UpdateMetaData failed");
        return;
    }
    HILOGI("UpdateMetaData success!");
}

void AvrcpCtAVSession::UpdatePlaybackState(const AVSession::AVPlaybackState &state)
{
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
    if (avSession_->SetAVPlaybackState(state) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("UpdatePlaybackState failed");
        return;
    }
    HILOGI("UpdatePlaybackState success!");
}

void AvrcpCtAVSession::UpdatePlayingQueueItems(const std::vector<AVSession::AVQueueItem> &items)
{
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
    if (avSession_->SetAVQueueItems(items) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("UpdatePlayingQueueItems failed");
        return;
    }
    HILOGI("UpdatePlayingQueueItems success!");
}

void AvrcpCtAVSession::ClearPlayingQueueItems()
{
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");
    std::vector<AVSession::AVQueueItem> items;
    if (avSession_->SetAVQueueItems(items) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("ClearPlayingQueueItems failed");
        return;
    }
    HILOGI("ClearPlayingQueueItems success!");
}

void AvrcpCtAVSession::UpdateCoverArt(const std::vector<uint8_t> &imageData)
{
    CHECK_AND_RETURN_LOG(avSession_, "avSession is invalid!");

    if (imageData.empty()) {
        HILOGW("imageData is empty, skip updating cover art");
        return;
    }

    AVSession::AVMetaData currentMeta;
    if (avSession_->GetAVMetaData(currentMeta) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("Failed to get current metadata");
        return;
    }

    auto pixelMap = std::make_shared<AVSession::AVSessionPixelMap>();
    pixelMap->SetInnerImgBuffer(imageData);
    currentMeta.SetMediaImage(pixelMap);

    if (avSession_->SetAVMetaData(currentMeta) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("UpdateCoverArt failed");
        return;
    }
    HILOGI("UpdateCoverArt success! image size: %{public}zu", imageData.size());
}

}  // namespace bluetooth
}  // namespace OHOS