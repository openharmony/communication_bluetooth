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

#ifndef A2DP_SNK_AUDIO_RENDER_H
#define A2DP_SNK_AUDIO_RENDER_H

#include "btif/a2dp_audio_render_interface.h"
#include "audio_renderer.h"
#include "log.h"
#include <memory>

namespace OHOS {
namespace bluetooth {

class A2dpSnkAudioRender : public IA2dpAudioRender {
public:
    static std::shared_ptr<A2dpSnkAudioRender> GetInstance();
    A2dpSnkAudioRender();
    ~A2dpSnkAudioRender() override = default;

    bool CreateRender(int freq, int bits, int channels) override;
    bool StartRender() override;
    bool StopRender() override;
    bool PauseRender() override;
    bool ReleaseRender() override;
    bool WriteStream(uint8_t* buffer, size_t size) override;

private:
    std::mutex audioRenderLock_;
    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer_{nullptr};
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SNK_AUDIO_RENDER_H
