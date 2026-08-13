/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_play_helper"
#endif

#include "play_helper.h"

#include "log.h"
#include <audio_info.h>
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
using namespace AudioStandard;
constexpr const char* LOCAL_SILENT_AUDIO_FILE = "/etc/bluetooth/silent.wav";
constexpr const uint16_t READ_SIZE = 1;
constexpr const uint16_t MIN_BYTES = 4;
constexpr const size_t MAX_BUFFER_LEN = 10 * 1024;
void PlayHelper::PlaySilenceSound(void)
{
    HITRACE_METER(BT_TRACE_TAG);
    // init audio render
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(AudioStreamType::STREAM_MUSIC);
    CHECK_AND_RETURN_LOG(InitAudioRender(audioRenderer),  "init audio render error");

    size_t bufferLen = 0;
    size_t bytesToWrite = 0;
    size_t bytesWritten = 0;
    // get minimum buffer size for rendering
    CHECK_AND_RETURN_LOG(audioRenderer->GetBufferSize(bufferLen) == 0, "get render buffer error");
    CHECK_AND_RETURN_LOG((bufferLen > 0 && bufferLen < MAX_BUFFER_LEN), "buffer length out of range");

    std::vector<uint8_t> buffer(bufferLen + bufferLen);
    FILE *wavFile = fopen(LOCAL_SILENT_AUDIO_FILE, "rb");
    CHECK_AND_RETURN_LOG(wavFile != nullptr, "open silence wav failed");
    while (!feof(wavFile)) {
        // bufferLen is 3528, file length is 3918
        bytesToWrite = fread(&buffer[0], READ_SIZE, bufferLen, wavFile);
        bytesWritten = 0;
        while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > MIN_BYTES)) {
            bytesWritten += static_cast<size_t>(
                audioRenderer->Write(&buffer[0] + bytesWritten, bytesToWrite - bytesWritten));
        }
    }
    // release audio render
    audioRenderer->Flush();
    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();
    audioRenderer = nullptr;
    (void)fclose(wavFile);
}

bool PlayHelper::InitAudioRender(const std::unique_ptr<AudioRenderer> &audioRenderer)
{
    CHECK_AND_RETURN_LOG_RET(audioRenderer != nullptr, false, "audio render is null");
    // set audio render params
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = AudioSampleFormat::SAMPLE_S16LE;
    rendererParams.sampleRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererParams.channelCount = AudioChannel::STEREO;
    rendererParams.encodingType = AudioEncodingType::ENCODING_PCM;
    audioRenderer->SetSilentModeAndMixWithOthers(true);
    CHECK_AND_RETURN_LOG_RET(audioRenderer->SetParams(rendererParams) == 0, false, "audio renderer set params error");
    // start audio rendering
    CHECK_AND_RETURN_LOG_RET(audioRenderer->Start(), false, "audio renderer set params error");
    return true;
}
}
}