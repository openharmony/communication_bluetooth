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
 * Stub of the removed stack layer HFP HF profile interface (bt_hf_client.h).
 * Interface table layout and signatures mirror bluedroid
 * system/include/hardware/bt_hf_client.h.
 */

#ifndef BT_HF_CLIENT_H
#define BT_HF_CLIENT_H

#include <cstddef>

#include "bt_types.h"

enum BthfClientConnectionState {
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTING,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING,
};

enum BthfClientAudioState {
    BTHF_CLIENT_AUDIO_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_AUDIO_STATE_CONNECTING,
    BTHF_CLIENT_AUDIO_STATE_CONNECTED,
    BTHF_CLIENT_AUDIO_STATE_DISCONNECTING,
};

enum BthfClientVrState {
    BTHF_CLIENT_VR_STATE_STOPPED = 0,
    BTHF_CLIENT_VR_STATE_STARTED,
};

enum BthfClientCallState {
    BTHF_CLIENT_CALL_STATE_ACTIVE = 0,
    BTHF_CLIENT_CALL_STATE_HELD,
    BTHF_CLIENT_CALL_STATE_DIALING,
    BTHF_CLIENT_CALL_STATE_ALERTING,
    BTHF_CLIENT_CALL_STATE_INCOMING,
    BTHF_CLIENT_CALL_STATE_WAITING,
    BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD,
};

enum BthfClientCallAction {
    BTHF_CLIENT_CALL_ACTION_CHLD_0 = 0,
    BTHF_CLIENT_CALL_ACTION_CHLD_1,
    BTHF_CLIENT_CALL_ACTION_CHLD_2,
    BTHF_CLIENT_CALL_ACTION_CHLD_3,
    BTHF_CLIENT_CALL_ACTION_ATA,
    BTHF_CLIENT_CALL_ACTION_CHUP,
    BTHF_CLIENT_CALL_ACTION_BTRH_0,
    BTHF_CLIENT_CALL_ACTION_BTRH_1,
    BTHF_CLIENT_CALL_ACTION_BTRH_2,
};

enum BthfClientCmdComplete {
    BTHF_CLIENT_CMD_COMPLETE_OK = 0,
    BTHF_CLIENT_CMD_COMPLETE_ERROR,
    BTHF_CLIENT_CMD_COMPLETE_ERROR_CME,
};

enum BthfClientNetworkState {
    BTHF_CLIENT_NETWORK_STATE_NOT_AVAILABLE = 0,
    BTHF_CLIENT_NETWORK_STATE_AVAILABLE,
};

enum BthfClientServiceType {
    BTHF_CLIENT_SERVICE_TYPE_HOME = 0,
    BTHF_CLIENT_SERVICE_TYPE_ROAMING,
};

enum BthfClientCall {
    BTHF_CLIENT_CALL_NONE = 0,
    BTHF_CLIENT_CALL_ACTIVE,
    BTHF_CLIENT_CALL_HELD,
    BTHF_CLIENT_CALL_DIALING,
    BTHF_CLIENT_CALL_ALERTING,
    BTHF_CLIENT_CALL_INCOMING,
    BTHF_CLIENT_CALL_WAITING,
    BTHF_CLIENT_CALL_HELD_BY_RESP_HOLD,
};

enum BthfClientCallsetup {
    BTHF_CLIENT_CALLSETUP_NONE = 0,
    BTHF_CLIENT_CALLSETUP_INCOMING,
    BTHF_CLIENT_CALLSETUP_DIALING,
    BTHF_CLIENT_CALLSETUP_ALERTING,
};

enum BthfClientCallheld {
    BTHF_CLIENT_CALLHELD_NONE = 0,
    BTHF_CLIENT_CALLHELD_HOLD_AND_ACTIVE,
    BTHF_CLIENT_CALLHELD_HOLD,
};

enum BthfClientRespAndHold {
    BTHF_CLIENT_RESP_AND_HOLD_HELD = 0,
    BTHF_CLIENT_RESP_AND_HOLD_ACCEPT,
    BTHF_CLIENT_RESP_AND_HOLD_REJECT,
};

enum BthfClientCallDirection {
    BTHF_CLIENT_CALL_DIR_INCOMING = 0,
    BTHF_CLIENT_CALL_DIR_OUTGOING,
};

enum BthfClientCallMptyType {
    BTHF_CLIENT_CALL_MPTY_TYPE_SINGLE = 0,
    BTHF_CLIENT_CALL_MPTY_TYPE_MULTI,
};

enum BthfClientVolumeType {
    BTHF_CLIENT_VOLUME_TYPE_SPK = 0,
    BTHF_CLIENT_VOLUME_TYPE_MIC,
};

enum BthfClientSubscriberServiceType {
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_UNKNOWN = 0,
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_VOICE,
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_FAX,
};

enum BthfClientInBandRingState {
    BTHF_CLIENT_IN_BAND_RING_NOT_PROVIDED = 0,
    BTHF_CLIENT_IN_BAND_RING_PROVIDED,
};

struct BthfClientCallbacks {
    size_t size;
    void (*connectionStateCb)(const OHOS::bluetooth::RawAddress *bdAddr,
                                BthfClientConnectionState state,
                                unsigned int peerFeat, unsigned int chldFeat);
    void (*audioStateCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientAudioState state);
    void (*vrCmdCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientVrState state);
    void (*networkStateCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientNetworkState state);
    void (*networkRoamingCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientServiceType type);
    void (*networkSignalCb)(const OHOS::bluetooth::RawAddress *bdAddr, int signal);
    void (*batteryLevelCb)(const OHOS::bluetooth::RawAddress *bdAddr, int level);
    void (*currentOperatorCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *name);
    void (*callCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientCall call);
    void (*callsetupCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientCallsetup callsetup);
    void (*callheldCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientCallheld callheld);
    void (*respAndHoldCb)(const OHOS::bluetooth::RawAddress *bdAddr,
                             BthfClientRespAndHold respAndHold);
    void (*clipCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *number);
    void (*callWaitingCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *number);
    void (*currentCallsCb)(const OHOS::bluetooth::RawAddress *bdAddr, int index,
                             BthfClientCallDirection dir,
                             BthfClientCallState state,
                             BthfClientCallMptyType mpty, const char *number);
    void (*volumeChangeCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientVolumeType type,
                             int volume);
    void (*cmdCompleteCb)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientCmdComplete type,
                            int cme);
    void (*subscriberInfoCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *name,
                               BthfClientSubscriberServiceType type);
    void (*inBandRingCb)(const OHOS::bluetooth::RawAddress *bdAddr,
                            BthfClientInBandRingState inBand);
    void (*lastVoiceTagNumberCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *number);
    void (*ringIndicationCb)(const OHOS::bluetooth::RawAddress *bdAddr);
    void (*unknownEventCb)(const OHOS::bluetooth::RawAddress *bdAddr, const char *eventString);
};

struct BthfClientInterface {
    size_t size;
    BtStackStatus (*init)(BthfClientCallbacks *callbacks);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*connectAudio)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*disconnectAudio)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*startVoiceRecognition)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*stopVoiceRecognition)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*volumeControl)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientVolumeType type,
        int volume);
    BtStackStatus (*dial)(const OHOS::bluetooth::RawAddress *bdAddr, const char *number);
    BtStackStatus (*dialMemory)(const OHOS::bluetooth::RawAddress *bdAddr, int location);
    BtStackStatus (*handleCallAction)(const OHOS::bluetooth::RawAddress *bdAddr, BthfClientCallAction action,
        int idx);
    BtStackStatus (*queryCurrentCalls)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*queryCurrentOperatorName)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*retrieveSubscriberInfo)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*sendDtmf)(const OHOS::bluetooth::RawAddress *bdAddr, char code);
    BtStackStatus (*requestLastVoiceTagNumber)(const OHOS::bluetooth::RawAddress *bdAddr);
    void (*cleanup)(void);
    BtStackStatus (*sendAtCmd)(const OHOS::bluetooth::RawAddress *bdAddr, int cmd, int val1, int val2,
        const char *arg);
    BtStackStatus (*sendAndroidAt)(const OHOS::bluetooth::RawAddress *bdAddr, const char *arg);
};

#endif  // BT_HF_CLIENT_H
