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
#define LOG_TAG "bt_service_a2dp_audio_render"
#endif

#include "a2dp_snk_audio_render.h"

namespace OHOS {
namespace bluetooth {
using namespace AudioStandard;

std::shared_ptr<A2dpSnkAudioRender> A2dpSnkAudioRender::GetInstance()
{
    static std::shared_ptr<A2dpSnkAudioRender> instance = std::make_shared<A2dpSnkAudioRender>();
    return instance;
}

A2dpSnkAudioRender::A2dpSnkAudioRender()
{}

bool A2dpSnkAudioRender::CreateRender(int freq, int bits, int channels)
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (audioRenderer_) {
        HILOGW("audioRender has been exist");
        return true;
    }

    AudioRendererOptions option = {};
    option.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    option.streamInfo.samplingRate = static_cast<AudioSamplingRate>(freq);

    switch (bits) {
        case 16:
            option.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
            break;
        case 24:
            option.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
            break;
        case 32:
            option.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
            break;
        default:
            option.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
            break;
    }

    option.streamInfo.channels = (channels == 2) ? AudioChannel::STEREO : AudioChannel::MONO;
    option.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    option.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    option.rendererInfo.rendererFlags = 0;

    audioRenderer_ = AudioRenderer::Create(option);
    if (!audioRenderer_) {
        HILOGE("Failed to create audioRender!");
        return false;
    }
    HILOGI("Succeeded create with options!");
    return true;
}

bool A2dpSnkAudioRender::StartRender()
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (!audioRenderer_) {
        HILOGE("audioRender is nullptr");
        return false;
    }

    RendererState renderState = audioRenderer_->GetStatus();
    if (renderState != RendererState::RENDERER_PREPARED && renderState != RendererState::RENDERER_PAUSED &&
        renderState != RendererState::RENDERER_STOPPED) {
        HILOGE("Invalid renderState=%{public}d", renderState);
        return false;
    }

    if (!audioRenderer_->Start()) {
        HILOGE("Failed start!");
        return false;
    }
    HILOGI("Succeeded start!");
    return true;
}

bool A2dpSnkAudioRender::WriteStream(uint8_t *buffer, size_t bufferSize)
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (!audioRenderer_) {
        HILOGE("audioRender is nullptr");
        return false;
    }
    RendererState renderState = audioRenderer_->GetStatus();
    if (renderState != RendererState::RENDERER_RUNNING) {
        HILOGE("Invalid renderState=%{public}d", renderState);
        return false;
    }

    size_t bytesWritten = 0;
    while (bytesWritten < bufferSize) {
        ssize_t written = audioRenderer_->Write(buffer + bytesWritten, bufferSize - bytesWritten);
        if (written < 0) {
            HILOGE("Write failed with error: %{public}ld", written);
            return false;
        }
        bytesWritten += static_cast<size_t>(written);
    }
    return true;
}

bool A2dpSnkAudioRender::PauseRender()
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (!audioRenderer_) {
        HILOGE("audioRender is nullptr");
        return false;
    }

    RendererState renderState = audioRenderer_->GetStatus();
    if (renderState != RendererState::RENDERER_RUNNING) {
        HILOGE("Invalid renderState=%{public}d", renderState);
        return false;
    }

    if (!audioRenderer_->Pause()) {
        HILOGE("Failed pause!");
        return false;
    }
    HILOGI("Succeeded pause!");
    return true;
}

bool A2dpSnkAudioRender::StopRender()
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (!audioRenderer_) {
        HILOGE("audioRender is nullptr");
        return false;
    }

    RendererState renderState = audioRenderer_->GetStatus();
    if (renderState != RendererState::RENDERER_RUNNING && renderState != RendererState::RENDERER_PAUSED) {
        HILOGE("Invalid renderState=%{public}d", renderState);
        return false;
    }

    if (!audioRenderer_->Stop()) {
        HILOGE("Failed stop!");
        return false;
    }
    HILOGI("Succeeded stop!");
    return true;
}

bool A2dpSnkAudioRender::ReleaseRender()
{
    std::lock_guard<std::mutex> renderLock(audioRenderLock_);
    if (!audioRenderer_) {
        HILOGW("audioRender already released or null");
        audioRenderer_.reset();
        return true;
    }

    RendererState renderState = audioRenderer_->GetStatus();
    if (renderState != RendererState::RENDERER_RELEASED) {
        audioRenderer_->Release();
    }
    audioRenderer_.reset();
    HILOGI("Succeeded release!");
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
