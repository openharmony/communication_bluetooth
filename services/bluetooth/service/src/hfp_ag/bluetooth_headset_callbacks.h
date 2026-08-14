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
#include "types/raw_address.h"

namespace bluetooth {
namespace headset {

/* Abstract callback interface dispatched from the HFP AG stack. */
class Callbacks {
public:
    virtual ~Callbacks() = default;

    virtual void ConnectionStateCallback(bthf_connection_state_t state,
                                         RawAddress *bdAddr) = 0;
    virtual void AudioStateCallback(bthf_audio_state_t state,
                                    RawAddress *bdAddr) = 0;
    virtual void VoiceRecognitionCallback(bthf_vr_state_t state,
                                          RawAddress *bdAddr) = 0;
    virtual void AnswerCallCallback(RawAddress *bdAddr) = 0;
    virtual void HangupCallCallback(RawAddress *bdAddr) = 0;
    virtual void VolumeControlCallback(bthf_volume_type_t type, int volume,
                                       RawAddress *bdAddr) = 0;
    virtual void DialCallCallback(char *number, RawAddress *bdAddr) = 0;
    virtual void DtmfCmdCallback(char dtmf, RawAddress *bdAddr) = 0;
    virtual void NoiseReductionCallback(bthf_nrec_t nrec, RawAddress *bdAddr) = 0;
    virtual void WbsCallback(bthf_wbs_config_t wbsConfig,
                             RawAddress *bdAddr) = 0;
    virtual void AtChldCallback(bthf_chld_type_t chld, RawAddress *bdAddr) = 0;
    virtual void AtCnumCallback(RawAddress *bdAddr) = 0;
    virtual void AtCindCallback(RawAddress *bdAddr) = 0;
    virtual void AtCopsCallback(RawAddress *bdAddr) = 0;
    virtual void AtClccCallback(RawAddress *bdAddr) = 0;
    virtual void UnknownAtCallback(char *atString, RawAddress *bdAddr) = 0;
    virtual void KeyPressedCallback(RawAddress *bdAddr) = 0;
    virtual void AtBindCallback(char *atString, RawAddress *bdAddr) = 0;
    virtual void AtBievCallback(bthf_hf_ind_type_t indId, int indValue,
                                RawAddress *bdAddr) = 0;
    virtual void AtBiaCallback(bool service, bool roam, bool signal,
                               bool battery, RawAddress *bdAddr) = 0;
#ifdef BLUETOOTH_SCO_NORMALIZED_FEATURE_ENABLE
    virtual void AtBccCallback(RawAddress *bdAddr) = 0;
#endif
#ifdef COMMUNICATION_L2
    virtual void AtBrsfCallback(uint32_t features, RawAddress *bdAddr) = 0;
#endif
};

}  // namespace headset
}  // namespace bluetooth

#endif  // BLUETOOTH_HEADSET_CALLBACKS_H
