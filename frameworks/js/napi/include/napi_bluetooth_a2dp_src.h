/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_A2DP_SRC_H_
#define NAPI_BLUETOOTH_A2DP_SRC_H_

#include <vector>
#include "bluetooth_a2dp_src.h"
#include "napi_bluetooth_a2dp_src_observer.h"

namespace OHOS {
namespace Bluetooth {
constexpr int32_t MIN_AUTO_PLAY_DURATION_SEC = 3 * 1000;
constexpr int32_t MAX_AUTO_PLAY_DURATION_SEC = 20 * 1000;
enum CodecType {
    CODEC_TYPE_SBC = 0,
    CODEC_TYPE_AAC = 1,
    CODEC_TYPE_L2HC = 2,
    CODEC_TYPE_L2HCST = 3,
    CODEC_TYPE_LDAC = 4,
    CODEC_TYPE_INVALID = -1,
};

enum CodecBitsPerSample {
    CODEC_BITS_PER_SAMPLE_NONE = 0,
    CODEC_BITS_PER_SAMPLE_16 = 1,
    CODEC_BITS_PER_SAMPLE_24 = 2,
    CODEC_BITS_PER_SAMPLE_32 = 3,
};

enum CodecChannelMode {
    CODEC_CHANNEL_MODE_NONE = 0,
    CODEC_CHANNEL_MODE_MONO = 1,
    CODEC_CHANNEL_MODE_STEREO = 2,
};

enum CodecSampleRate {
    CODEC_SAMPLE_RATE_NONE = 0,
    CODEC_SAMPLE_RATE_44100 = 1,
    CODEC_SAMPLE_RATE_48000 = 2,
    CODEC_SAMPLE_RATE_88200 = 3,
    CODEC_SAMPLE_RATE_96000 = 4,
    CODEC_SAMPLE_RATE_176400 = 5,
    CODEC_SAMPLE_RATE_192000 = 6,
};

struct CodecInfo {
    CodecType codecType;
    CodecBitsPerSample codecBitsPerSample;
    CodecChannelMode codecChannelMode;
    CodecSampleRate codecSampleRate;
};

class NapiA2dpSource {
public:
    static napi_value DefineA2dpSourceJSClass(napi_env env, napi_value exports);
    static napi_value A2dpSourceConstructor(napi_env env, napi_callback_info info);
    static napi_value A2dpPropertyValueInit(napi_env env, napi_value exports);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetPlayingState(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value GetConnectionDevices(napi_env env, napi_callback_info info);
    static napi_value GetDeviceState(napi_env env, napi_callback_info info);
#ifdef BLUETOOTH_API_SINCE_10
    static napi_value DefineCreateProfile(napi_env env, napi_value exports);
    static napi_value CreateA2dpSrcProfile(napi_env env, napi_callback_info info);
    static napi_value SetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionStrategy(napi_env env, napi_callback_info info);
    static napi_value GetConnectionState(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDevices(napi_env env, napi_callback_info info);
    static napi_value IsAbsoluteVolumeSupported(napi_env env, napi_callback_info info);
    static napi_value IsAbsoluteVolumeEnabled(napi_env env, napi_callback_info info);
    static napi_value EnableAbsoluteVolume(napi_env env, napi_callback_info info);
    static napi_value DisableAbsoluteVolume(napi_env env, napi_callback_info info);
    static napi_value SetCurrentCodecInfo(napi_env env, napi_callback_info info);
    static napi_value GetCurrentCodecInfo(napi_env env, napi_callback_info info);
    static napi_value EnableAutoPlay(napi_env env, napi_callback_info info);
    static napi_value DisableAutoPlay(napi_env env, napi_callback_info info);
    static napi_value GetAutoPlayDisabledDuration(napi_env env, napi_callback_info info);
#endif
    static std::shared_ptr<NapiA2dpSourceObserver> observer_;
    static bool isRegistered_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_A2DP_SRC_H_ */