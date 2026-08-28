/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer HFP AG profile interface (bt_hf.h).
 */

#ifndef BT_HF_H
#define BT_HF_H

#include <cstddef>

#include "bt_types.h"

enum BthfConnectionState {
    BTHF_CONNECTION_STATE_DISCONNECTED = 0,
    BTHF_CONNECTION_STATE_CONNECTING,
    BTHF_CONNECTION_STATE_CONNECTED,
    BTHF_CONNECTION_STATE_SLC_CONNECTED,
    BTHF_CONNECTION_STATE_DISCONNECTING,
};

enum BthfAudioState {
    BTHF_AUDIO_STATE_DISCONNECTED = 0,
    BTHF_AUDIO_STATE_CONNECTING,
    BTHF_AUDIO_STATE_CONNECTED,
    BTHF_AUDIO_STATE_DISCONNECTING,
};

enum BthfVrState {
    BTHF_VR_STATE_STOPPED = 0,
    BTHF_VR_STATE_STARTED,
};

enum BthfNrec {
    BTHF_NREC_STOP = 0,
    BTHF_NREC_START,
};

enum BthfVolumeType {
    BTHF_VOLUME_TYPE_SPK = 0,
    BTHF_VOLUME_TYPE_MIC,
};

enum BthfWbsConfig {
    BTHF_WBS_NO = 0,
    BTHF_WBS_YES,
    BTHF_WBS_SUPER,
};

enum BthfChldType {
    BTHF_CALL_ACTION_CHLD_0 = 0,
    BTHF_CALL_ACTION_CHLD_1,
    BTHF_CALL_ACTION_CHLD_2,
    BTHF_CALL_ACTION_CHLD_3,
    BTHF_CALL_ACTION_CHLD_4,
    BTHF_CALL_ACTION_CHLD_1X,
    BTHF_CALL_ACTION_CHLD_2X,
};

enum BthfCallState {
    BTHF_CALL_STATE_ACTIVE = 0,
    BTHF_CALL_STATE_HELD,
    BTHF_CALL_STATE_DIALING,
    BTHF_CALL_STATE_ALERTING,
    BTHF_CALL_STATE_INCOMING,
    BTHF_CALL_STATE_WAITING,
    BTHF_CALL_STATE_IDLE,
    BTHF_CALL_STATE_DISCONNECTED,
};

enum BthfAtResponse {
    BTHF_AT_RESPONSE_ERROR = 0,
    BTHF_AT_RESPONSE_OK,
};

enum BthfCallDirection {
    BTHF_CALL_DIR_INCOMING = 0,
    BTHF_CALL_DIR_OUTGOING,
};

enum BthfCallMode {
    BTHF_CALL_TYPE_VOICE = 0,
    BTHF_CALL_TYPE_DATA,
    BTHF_CALL_TYPE_FAX,
};

enum BthfCallMptyType {
    BTHF_CALL_MPTY_TYPE_SINGLE = 0,
    BTHF_CALL_MPTY_TYPE_MULTI,
};

enum BthfCallAddrtype {
    BTHF_CALL_ADDRTYPE_UNKNOWN = 0,
    BTHF_CALL_ADDRTYPE_INTERNATIONAL,
};

enum BthfNetworkState {
    BTHF_NETWORK_STATE_NOT_AVAILABLE = 0,
    BTHF_NETWORK_STATE_AVAILABLE,
};

enum BthfServiceType {
    BTHF_SERVICE_TYPE_HOME = 0,
    BTHF_SERVICE_TYPE_ROAMING,
};

enum BthfHfIndType {
    BTHF_HF_IND_ENHANCED_DRIVER_SAFETY = 0,
    BTHF_HF_IND_BATTERY_LEVEL_STATUS,
};

struct BthfCallbacks {
    size_t size;
    BtStackStatus (*connectionStateCb)(BthfConnectionState state,
                                       const RawAddress *bdAddr);
    BtStackStatus (*audioStateCb)(BthfAudioState state,
                                  const RawAddress *bdAddr);
    BtStackStatus (*vrCb)(BthfVrState state, const RawAddress *bdAddr);
    BtStackStatus (*answerCallCb)(const RawAddress *bdAddr);
    BtStackStatus (*hangupCallCb)(const RawAddress *bdAddr);
    BtStackStatus (*volumeCb)(BthfVolumeType type, int volume,
                             const RawAddress *bdAddr);
    BtStackStatus (*dialCallCb)(char *number, const RawAddress *bdAddr);
    BtStackStatus (*sendDtmfCb)(char tone, const RawAddress *bdAddr);
    BtStackStatus (*noiseReductionCb)(BthfNrec nrec,
                                      const RawAddress *bdAddr);
    BtStackStatus (*atResponseCb)(const RawAddress *bdAddr);
    BtStackStatus (*cwCb)(BthfCallState state, const RawAddress *bdAddr);
    BtStackStatus (*callIndCb)(BthfCallState state,
                               const RawAddress *bdAddr);
    BtStackStatus (*callSetupIndCb)(BthfCallState state,
                                     const RawAddress *bdAddr);
    BtStackStatus (*callHeldIndCb)(BthfCallState state,
                                    const RawAddress *bdAddr);
    BtStackStatus (*netStateCb)(BthfNetworkState state,
                                const RawAddress *bdAddr);
    BtStackStatus (*callIndNetCb)(const RawAddress *bdAddr);
    BtStackStatus (*signalCb)(int signal, const RawAddress *bdAddr);
    BtStackStatus (*roamingCb)(int roaming, const RawAddress *bdAddr);
    BtStackStatus (*batteryCb)(int battery, const RawAddress *bdAddr);
    BtStackStatus (*callVolumeCb)(BthfVolumeType type, int volume,
                                  const RawAddress *bdAddr);
};

struct BthfInterface {
    size_t size;
    BtStackStatus (*init)(BthfCallbacks *callbacks);
    void (*cleanup)(void);
    BtStackStatus (*connect)(const RawAddress *bdAddr);
    BtStackStatus (*disconnect)(const RawAddress *bdAddr);
    BtStackStatus (*audioConnect)(const RawAddress *bdAddr);
    BtStackStatus (*audioDisconnect)(const RawAddress *bdAddr);
    BtStackStatus (*startVoiceRecognition)(const RawAddress *bdAddr);
    BtStackStatus (*stopVoiceRecognition)(const RawAddress *bdAddr);
    BtStackStatus (*volumeControl)(const RawAddress *bdAddr,
                                  BthfVolumeType type, int volume);
    BtStackStatus (*notifyBatteryLevel)(const RawAddress *bdAddr, int level);
    BtStackStatus (*setScoAllowed)(const RawAddress *bdAddr, bool value);
};

/*
 * The service layer refers to the HFP AG types with the qualified name
 * ::bluetooth::headset::*, so re-export the global types into that
 * namespace with using declarations.
 */
namespace bluetooth {
namespace headset {

using ::BthfConnectionState;
using ::BTHF_CONNECTION_STATE_DISCONNECTED;
using ::BTHF_CONNECTION_STATE_CONNECTING;
using ::BTHF_CONNECTION_STATE_CONNECTED;
using ::BTHF_CONNECTION_STATE_SLC_CONNECTED;
using ::BTHF_CONNECTION_STATE_DISCONNECTING;

using ::BthfAudioState;
using ::BTHF_AUDIO_STATE_DISCONNECTED;
using ::BTHF_AUDIO_STATE_CONNECTING;
using ::BTHF_AUDIO_STATE_CONNECTED;
using ::BTHF_AUDIO_STATE_DISCONNECTING;

using ::BthfVrState;
using ::BTHF_VR_STATE_STOPPED;
using ::BTHF_VR_STATE_STARTED;

using ::BthfNrec;
using ::BTHF_NREC_STOP;
using ::BTHF_NREC_START;

using ::BthfVolumeType;
using ::BTHF_VOLUME_TYPE_SPK;
using ::BTHF_VOLUME_TYPE_MIC;

using ::BthfWbsConfig;
using ::BTHF_WBS_NO;
using ::BTHF_WBS_YES;
using ::BTHF_WBS_SUPER;

using ::BthfChldType;
using ::BTHF_CALL_ACTION_CHLD_0;
using ::BTHF_CALL_ACTION_CHLD_1;
using ::BTHF_CALL_ACTION_CHLD_2;
using ::BTHF_CALL_ACTION_CHLD_3;
using ::BTHF_CALL_ACTION_CHLD_4;
using ::BTHF_CALL_ACTION_CHLD_1X;
using ::BTHF_CALL_ACTION_CHLD_2X;

using ::BthfCallState;
using ::BTHF_CALL_STATE_ACTIVE;
using ::BTHF_CALL_STATE_HELD;
using ::BTHF_CALL_STATE_DIALING;
using ::BTHF_CALL_STATE_ALERTING;
using ::BTHF_CALL_STATE_INCOMING;
using ::BTHF_CALL_STATE_WAITING;
using ::BTHF_CALL_STATE_IDLE;
using ::BTHF_CALL_STATE_DISCONNECTED;

using ::BthfAtResponse;
using ::BTHF_AT_RESPONSE_ERROR;
using ::BTHF_AT_RESPONSE_OK;

using ::BthfCallDirection;
using ::BTHF_CALL_DIR_INCOMING;
using ::BTHF_CALL_DIR_OUTGOING;

using ::BthfCallMode;
using ::BTHF_CALL_TYPE_VOICE;
using ::BTHF_CALL_TYPE_DATA;
using ::BTHF_CALL_TYPE_FAX;

using ::BthfCallMptyType;
using ::BTHF_CALL_MPTY_TYPE_SINGLE;
using ::BTHF_CALL_MPTY_TYPE_MULTI;

using ::BthfCallAddrtype;
using ::BTHF_CALL_ADDRTYPE_UNKNOWN;
using ::BTHF_CALL_ADDRTYPE_INTERNATIONAL;

using ::BthfNetworkState;
using ::BTHF_NETWORK_STATE_NOT_AVAILABLE;
using ::BTHF_NETWORK_STATE_AVAILABLE;

using ::BthfServiceType;
using ::BTHF_SERVICE_TYPE_HOME;
using ::BTHF_SERVICE_TYPE_ROAMING;

using ::BthfHfIndType;
using ::BTHF_HF_IND_ENHANCED_DRIVER_SAFETY;
using ::BTHF_HF_IND_BATTERY_LEVEL_STATUS;

}  // namespace headset
}  // namespace bluetooth

#endif  // BT_HF_H
