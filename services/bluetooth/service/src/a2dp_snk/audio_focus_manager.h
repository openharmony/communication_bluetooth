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

#ifndef AUDIO_FOCUS_MANAGER_H
#define AUDIO_FOCUS_MANAGER_H

#include <functional>
#include <memory>
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_interrupt_callback.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {

class AudioInterruptCallbackImpl : public AudioStandard::AudioInterruptCallback {
public:
    void OnInterrupt(const AudioStandard::InterruptEventInternal &interruptEvent) override
    {
        if (callback_) {
            callback_(interruptEvent.hintType);
        }
    }
    void SetCallback(std::function<void(AudioStandard::InterruptHint)> cb) { callback_ = std::move(cb); }
private:
    std::function<void(AudioStandard::InterruptHint)> callback_;
};

class AudioFocusManager {
public:
    AudioFocusManager();
    virtual ~AudioFocusManager();

    virtual int32_t InitOnly(std::function<void(AudioStandard::InterruptHint)> callback);
    virtual int32_t ActivateFocus();
    virtual void DeactivateFocus();
    virtual void DeactivateAndDeinit();
    virtual uint32_t GetSessionId() const;

private:
    uint32_t sessionId_{0};
    bool activated_{false};
    bool callbackRegistered_{false};
    std::shared_ptr<AudioInterruptCallbackImpl> callback_;
    AudioStandard::AudioInterrupt CreateAudioInterrupt();
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // AUDIO_FOCUS_MANAGER_H
