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
#define LOG_TAG "bt_service_audio_focus_manager"
#endif

#include "audio_focus_manager.h"

namespace OHOS {
namespace bluetooth {

AudioFocusManager::AudioFocusManager()
{}

AudioFocusManager::~AudioFocusManager()
{
    DeactivateAndDeinit();
}

int32_t AudioFocusManager::InitOnly(std::function<void(AudioStandard::InterruptHint)> callback)
{
    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (!mgr) {
        HILOGE("AudioSystemManager is nullptr");
        return -1;
    }

    int32_t ret = mgr->GenerateSessionId(sessionId_);
    if (ret != 0 || sessionId_ == 0) {
        HILOGE("GenerateSessionId failed ret=%{public}d sessionId=%{public}u", ret, sessionId_);
        return ret;
    }

    callback_ = std::make_shared<AudioInterruptCallbackImpl>();
    callback_->SetCallback(std::move(callback));

    ret = mgr->SetAudioInterruptCallback(sessionId_, callback_, static_cast<uint32_t>(getuid()), 0);
    if (ret != 0) {
        HILOGE("SetAudioInterruptCallback failed ret=%{public}d", ret);
        sessionId_ = 0;
        callback_.reset();
        return ret;
    }

    callbackRegistered_ = true;
    HILOGI("InitOnly success sessionId=%{public}u", sessionId_);
    return 0;
}

AudioStandard::AudioInterrupt AudioFocusManager::CreateAudioInterrupt()
{
    AudioStandard::AudioInterrupt interrupt;
    interrupt.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_MUSIC;
    interrupt.contentType = AudioStandard::ContentType::CONTENT_TYPE_MUSIC;
    interrupt.audioFocusType.streamType = AudioStandard::AudioStreamType::STREAM_MUSIC;
    interrupt.audioFocusType.isPlay = true;
    interrupt.sessionId = sessionId_;
    interrupt.pid = getpid();
    interrupt.uid = getuid();
    interrupt.mode = AudioStandard::InterruptMode::SHARE_MODE;
    return interrupt;
}

int32_t AudioFocusManager::ActivateFocus()
{
    if (activated_) {
        HILOGI("Already activated, skip");
        return 0;
    }
    if (!callbackRegistered_ || sessionId_ == 0) {
        HILOGE("Not initialized, cannot activate");
        return -1;
    }

    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (!mgr) {
        HILOGE("AudioSystemManager is nullptr");
        return -1;
    }

    AudioStandard::AudioInterrupt interrupt = CreateAudioInterrupt();
    int32_t ret = mgr->ActivateAudioInterrupt(interrupt);
    if (ret != 0) {
        HILOGE("ActivateAudioInterrupt failed ret=%{public}d", ret);
        return ret;
    }

    activated_ = true;
    HILOGI("ActivateFocus success sessionId=%{public}u", sessionId_);
    return 0;
}

void AudioFocusManager::DeactivateFocus()
{
    if (!activated_) {
        HILOGI("Not activated, skip deactivate");
        return;
    }

    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (!mgr) {
        HILOGE("AudioSystemManager is nullptr");
        return;
    }

    AudioStandard::AudioInterrupt interrupt = CreateAudioInterrupt();
    int32_t ret = mgr->DeactivateAudioInterrupt(interrupt);
    if (ret != 0) {
        HILOGE("DeactivateAudioInterrupt failed ret=%{public}d", ret);
    }

    activated_ = false;
    HILOGI("DeactivateFocus success sessionId=%{public}u", sessionId_);
}

void AudioFocusManager::DeactivateAndDeinit()
{
    if (sessionId_ == 0) {
        return;
    }

    if (activated_) {
        DeactivateFocus();
    }

    if (callbackRegistered_) {
        auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
        if (mgr) {
            int32_t ret = mgr->UnsetAudioInterruptCallback(static_cast<uint32_t>(getuid()), sessionId_);
            if (ret != 0) {
                HILOGE("UnsetAudioInterruptCallback failed ret=%{public}d", ret);
            }
        }
        callbackRegistered_ = false;
    }

    callback_.reset();
    sessionId_ = 0;
    HILOGI("DeactivateAndDeinit done");
}

uint32_t AudioFocusManager::GetSessionId() const
{
    return sessionId_;
}

}  // namespace bluetooth
}  // namespace OHOS
