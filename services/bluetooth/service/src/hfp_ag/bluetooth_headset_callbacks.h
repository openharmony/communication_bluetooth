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

#ifndef BLUETOOTH_HEADSET_CALLBACKS_H
#define BLUETOOTH_HEADSET_CALLBACKS_H

#include "hardware/bt_hf.h"
#include "raw_address.h"

namespace bluetooth {
namespace headset {

/* Abstract callback interface dispatched from the HFP AG stack. */
class Callbacks {
public:
    virtual ~Callbacks() = default;

    virtual void ConnectionStateCallback(BthfConnectionState state,
                                         OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AudioStateCallback(BthfAudioState state,
                                    OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void VoiceRecognitionCallback(BthfVrState state,
                                          OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AnswerCallCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void HangupCallCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void VolumeControlCallback(BthfVolumeType type, int volume,
                                       OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void DialCallCallback(char *number, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void DtmfCmdCallback(char dtmf, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void NoiseReductionCallback(BthfNrec nrec, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void WbsCallback(BthfWbsConfig wbsConfig,
                             OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtChldCallback(BthfChldType chld, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtCnumCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtCindCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtCopsCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtClccCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void UnknownAtCallback(char *atString, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void KeyPressedCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtBindCallback(char *atString, OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtBievCallback(BthfHfIndType indId, int indValue,
                                OHOS::bluetooth::RawAddress *bdAddr) = 0;
    virtual void AtBiaCallback(bool service, bool roam, bool signal,
                               bool battery, OHOS::bluetooth::RawAddress *bdAddr) = 0;
#ifdef BLUETOOTH_SCO_NORMALIZED_FEATURE_ENABLE
    virtual void AtBccCallback(OHOS::bluetooth::RawAddress *bdAddr) = 0;
#endif
#ifdef COMMUNICATION_L2
    virtual void AtBrsfCallback(uint32_t features, OHOS::bluetooth::RawAddress *bdAddr) = 0;
#endif
};

}  // namespace headset
}  // namespace bluetooth

#endif  // BLUETOOTH_HEADSET_CALLBACKS_H
