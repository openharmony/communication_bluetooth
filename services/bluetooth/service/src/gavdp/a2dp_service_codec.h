/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef A2DP_SERVICE_CODEC_H
#define A2DP_SERVICE_CODEC_H

#include "bt_av.h"
#include "interface_profile_a2dp_src.h"

namespace OHOS {
namespace bluetooth {

bool ConvertAudioConfigCodec(btav_a2dp_codec_config_t codec_config, A2dpSrcCodecInfo &codecInfo);
bool ConvertL2hcV2Bitrate(btav_a2dp_codec_config_t &codec_config, const A2dpSrcCodecInfo &codecInfo);
bool ConvertL2hcStBitrate(btav_a2dp_codec_config_t &codec_config, const A2dpSrcCodecInfo &codecInfo);
}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SERVICE_CODEC_H