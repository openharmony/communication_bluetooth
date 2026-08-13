/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_BASE_H
#define BT_CHR_BASE_H
#include <cstddef>
#include <iostream>
#include <vector>
#include "securec.h"

// Event names begin
/**
 * Event Name Naming Rules:
 * 1. Start with a letter and can only consist of uppercase letters/numbers/underscores
 * 2. String length range 1~32
 */
// dft exception Event names
#define CHR_USER_DISCONNECT "BT_USER_DISCONNECT"
#define CHR_BT_EXCEPTION_NEW "BT_EXCEPTION"
#define CHR_BT_PAIR_EXCEPTION "BT_PAIR_EXCEPTION"
#define CHR_BLE_DISCONNECT "BT_BLE_DISCONNECT"
#define CHR_HID_EXCEPTION "BT_HID_EXCEPTION"
#define CHR_A2DP_CONNECT_EXCEPTION "BT_A2DP_CONNECT_EXCEPTION"
#define CHR_HFP_RFCOMM_EXCEPTION "BT_HFP_RFCOMM_EXCEPTION"
#define CHR_AVRCP_CONNECT_EXCEPTION "BT_AVRCP_CONNECT_EXCEPTION"
#define CHR_A2DP_AUDIO_EXCEPTION "BT_A2DP_AUDIO_EXCEPTION"
#define CHR_HDAP_AUDIO_EXCEPTION "BT_HDAP_AUDIO_EXCEPTION"
#define CHR_UNION_HISI_COMMON_INFO "BT_UNION_HISI_COMMON_INFO"
#define CHR_HFP_AUDIO_EXCEPTION "BT_HFP_AUDIO_EXCEPTION"
#define CHR_SNIFF_EXCEPTION "BT_SNIFF_EXCEPTION"
#define CHR_BT_SWITCH_ATOMIC_EVENT "BT_SWITCH_EXCEPTION_NOT_COLLECT"
#define CHR_BT_SWITCH_EVENT "BT_SWITCH_EXCEPTION"
#define CHR_BT_SOCKET_EXCEPTION "BT_SOCKET_EXCEPTION"
#define CHR_BT_WATCH_CONNECT "BT_WATCH_CONNECT"
#define CHR_BT_WATCH_REMOVE_PAIR "BT_WATCH_REMOVE_PAIR"
#define CHR_BT_WATCH_SPP_CONNECT "BT_WATCH_SPP_CONNECT"
#define CHR_BT_WATCH_SPP_DISCONNECT "BT_WATCH_SPP_DISCONNECT"

// dft statictics Event names
#define  CHR_BT_STATISTICS_ATOMIC_EVENT "BT_BUSINESS_STATISTICS"
#define CHR_BLE_ADV_RESOURCE_REACHING_MAX "BT_BLE_ADV_RESOURCE_REACHING_MAX"

// UE Event names
#define CHR_UE_MEDIA_RECV_CMD "MEDIA_RECV_CMD"
#define CHR_UE_STACK_CONN_SCO "STACK_CONN_SCO"
#define CHR_UE_STACK_DISCONN_SCO "STACK_DISCONN_SCO"
#define CHR_UE_HFP_RECV_CMD "HFP_RECV_CMD"
#define CHR_UE_SEND_ECHO89 "SEND_ECHO89"
#define CHR_UE_RECV_ECHO88 "RECV_ECHO88"
#define CHR_UE_TWS_DUAL_CONN_HAIR_CAPSL "TWS_DUAL_CONN_HAIR_CAPSL"
#define CHR_UE_AVRCP_VOL_CHANGE "AVRCP_VOL_CHANGE"
#define CHR_UE_HFP_VOL_CHANGE "HFP_VOL_CHANGE"
#define CHR_UE_AVRCP_CONN_CMP "AVRCP_CONN_CMP"
#define CHR_UE_AVRCP_DISCONN_CMP "AVRCP_DISCONN_CMP"
#define CHR_UE_ACL_CONN_CMP "ACL_CONN_CMP"
#define CHR_UE_ACL_DISCONN_CMP "ACL_DISCONN_CMP"
#define CHR_UE_BLE_CONN_CMP "BLE_CONN_CMP"
#define CHR_UE_BLE_DISCONN_CMP "BLE_DISCONN_CMP"
#define CHR_UE_START_VOICE_REG "START_VOICE_REG"
#define CHR_UE_STOP_VOICE_REG "STOP_VOICE_REG"
#define CHR_UE_ABS_VOL_SUPPORT "ABS_VOL_SUPPORT"
#define CHR_UE_ECHO11_SUPPORT "ECHO11_SUPPORT"
#define CHR_UE_INBAND_RING_SUPPORT "INBAND_RING_SUPPORT"
#define CHR_UE_BT_START_CONN "BT_START_CONN"
#define CHR_UE_SDP_CONN "BT_SDP_CONN"
#define CHR_UE_USER_OPER_PAIR "USER_OPER_PAIR"
#define CHR_UE_BT_ANTENNA_SWITCH "BT_ANTENNA_SWITCH"
#define CHR_UE_BLE_SCH_SKIP "BLE_SCH_SKIP"
#define CHR_UE_SET_CODEC_CONFIG "SET_CODEC_CONFIG"
#define CHR_UE_BLE_START_CONN "BLE_START_CONN"

#define CHR_UE_SOCKET_CLENT_CONN "SOCKET_CLENT_CONN"
#define CHR_UE_SOCKET_SERVER_CONN "SOCKET_SERVER_CONN"
#define CHR_UE_SOCKET_CALLBACK "SOCKET_CALLBACK"
#define CHR_UE_BT_VOL_CHANGE "BT_VOL_CHANGE"
#define CHR_UE_START_PAIR "START_PAIR"
#define CHR_UE_REMOVE_PAIR "REMOVE_PAIR"
#define CHR_UE_CANCEL_PAIR "CANCEL_PAIR"
#define CHR_UE_START_BT_DISCOVER "START_BT_DISCOVER"
#define CHR_UE_CANCEL_BT_DISCOVER "CANCEL_BT_DISCOVER"
#define CHR_UE_BLE_START_SCAN "BLE_START_SCAN"
#define CHR_UE_BLE_STOP_SCAN "BLE_STOP_SCAN"
#define CHR_UE_BLE_CLIENT_CONN "BLE_CLIENT_CONN"
#define CHR_UE_BLE_CLIENT_DISCONN "BLE_CLIENT_DISCONN"
#define CHR_UE_BLE_SERVER_CONN "BLE_SERVER_CONN"
#define CHR_UE_BLE_SERVER_DISCONN "BLE_SERVER_DISCONN"
#define CHR_UE_SET_BT_NAME "SET_BT_NAME"
#define CHR_UE_A2DP_START_PLAY "A2DP_START_PLAY"
#define CHR_UE_A2DP_STOP_PLAY "A2DP_STOP_PLAY"
#define CHR_UE_AUDIO_STATUS_CHANGE "AUDIO_STATUS_CHANGE"
#define CHR_UE_SET_A2DP_ACTIVE_DEV "SET_A2DP_ACTIVE_DEV"
#define CHR_UE_SET_HFP_ACTIVE_DEV "SET_HFP_ACTIVE_DEV"
#define CHR_UE_F_HID_CONN "F_HID_CONN"
#define CHR_UE_F_HID_DISCONN "F_HID_DISCONN"
#define CHR_UE_F_A2DP_CONN "F_A2DP_CONN"
#define CHR_UE_F_A2DP_DISCONN "F_A2DP_DISCONN"
#define CHR_UE_A2DP_CONN_CMP "A2DP_CONN_CMP"
#define CHR_UE_A2DP_DISCONN_CMP "A2DP_DISCONN_CMP"
#define CHR_UE_F_HFP_CONN "F_HFP_CONN"
#define CHR_UE_F_HFP_DISCONN "F_HFP_DISCONN"
#define CHR_UE_HFP_CONN_CMP "HFP_CONN_CMP"
#define CHR_UE_HFP_DISCONN_CMP "HFP_DISCONN_CMP"
#define CHR_UE_SET_A2DP_SUSPEND "SET_A2DP_SUSPEND"
#define CHR_UE_CHANGE_BT_SWITCH "CHANGE_BT_SWITCH"
#define CHR_UE_BT_UI_SWITCH_CHANGE "BT_UI_SWITCH_CHANGE"
#define CHR_UE_BRCAST_ACL_CONN "BRCAST_ACL_CONN"
#define CHR_UE_SERVICE_CODEC_SETCONFIG "SERVICE_CODEC_SETCONFIG"
#define CHR_UE_SEND_MEDIA_CMD "SEND_MEDIA_CMD"
#define CHR_UE_TWS_WEAR_STATE_SUPPORT "TWS_WEAR_STATE_SUPPORT"
#define CHR_UE_VOIP_CONN_SCO "VOIP_CONN_SCO"
#define CHR_UE_VOIP_DISCONN_SCO "VOIP_DISCONN_SCO"
#define CHR_UE_CS_CONN_SCO "CS_CONN_SCO"
#define CHR_UE_CS_DISCONN_SCO "CS_DISCONN_SCO"
#define CHR_UE_UPDATE_HFP_OUTPUT "UPDATE_HFP_OUTPUT"
#define CHR_UE_UPDATE_A2DP_OUTPUT "UPDATE_A2DP_OUTPUT"
#define CHR_UE_BT_SYNC_PAIRED_EVENT "BT_SYNC_PAIRED_EVENT"
#define GHR_UE_BAND_WEAR_STATE_CHANGE "BAND_WEAR_STATE_CHANGE"
#define CHR_UE_BT_DEL_PAIRED_EVENT "BT_DEL_PAIRED_EVENT"
#define CHR_UE_KEEP_BLE_SCAN_IN_CONNECTION "KEEP_BLE_SCAN_IN_CONNECTION"
#define CHR_UE_PAIRED_CUSTOM_TYPE_CHANGE "PAIRED_CUSTOM_TYPE_CHANGE"
#define CHR_UE_SET_PHONE_MODE "SET_PHONE_MODE"
#define CHR_UE_BRCAST_SCO_STATE_CHANGE "BRCAST_SCO_STATE_CHANGE"
#define CHR_UE_BT_HDAP_OPER_EVENT "BT_HDAP_OPER_EVENT"
#define CHR_UE_FINISH_CLOUD_PAIR "FINISH_CLOUD_PAIR"
#define CHR_UE_SEND_MEDIA_MUTE "SEND_MEDIA_MUTE"
#define CHR_UE_SENSORHUB_COLLABORATION_EVENT "SENSORHUB_COLLABORATION_EVENT"
#define CHR_UE_BLUETOOTH_APP_WAKEUP "BLUETOOTH_APP_WAKEUP"
// Event names end

// Event parameter names begin
#define CHR_ERRCODE "ERRCODE"
#define CHR_SUB_ERRCODE "SUBERRCODE"
// Event parameter names end

// 913000019 ErrCode
enum BtPairErrCode {
    ERRCODE_PAIR_FAILED = 1,
    ERRCODE_PAIR_SUCCESS = 2,
    ERRCODE_REMOVE_PAIR = 3,
    ERRCODE_CANCEL_PAIR = 4,
};

enum BtChrEventParamType {
    BTCHREVENT_INT32 = 6,
    BTCHREVENT_UINT32 = 7,
    BTCHREVENT_STRING = 12,
    BTCHREVENT_INT64_ARRAY = 20,
    BTCHREVENT_STRING_ARRAY = 24,
};

enum BtChrAclStatus {
    CHR_ACL_DISCONNECTED = 0,
    CHR_ACL_CONNECTION,
    CHR_ACL_CONNECTED,
};

enum BtChrInitiator {
    CHR_INITIATOR_LOCAL = 1,
    CHR_INITIATOR_PEER,
};

enum BtChrWatchRemovePairResult {
    CHR_REMOVE_PAIR_SUCCESS = 1,
    CHR_LOAD_BLUETOOTH_FAIL,
    CHR_DEVICE_NOT_PAIR,
    CHR_CANCLE_DISCOVERY_FAIL,
    CHR_OTHER_ERROR,
};

enum BtChrWatchRemovePairScene {
    CHR_WATCH_REMOVE_PAIR = 1,
    CHR_PHONE_REMOVE_PAIR_ONLINE,
    CHR_PHONE_REMOVE_PAIR_OFFLINE,
    CHR_PAIR_FAIL,
    CHR_REMOVE_PAIR_OTHER = 100,
};

enum BtChrWatchDisconnectScene {
    CHR_REMOVE_PAIR = 1,
    CHR_DISABLE_BT,
    CHR_OTHER = 100,
};

enum BtChrWatchRemovePairDisconnectResult {
    CHR_NOT_INVOLVED,
    CHR_DISCONNECT_SUCCESS,
    CHR_DISCONNECT_FAIL,
};

enum BtChrUeStartVoiceRegScene {
    UE_REMOTE_START_VR_RECV_REQ = 0x0000,
    UE_REMOTE_START_VR_REQ_ACCEPT,
    UE_REMOTE_START_VR_RECV_BVRA_REQ,
    UE_REMOTE_START_VR_RECV_TBSR_REQ,
    UE_REMOTE_START_VR_FAIL_VR_IS_OPENED,
    UE_REMOTE_START_VR_FAIL_DEV_ADDR_NULL,
    UE_REMOTE_START_VR_FAIL_AUDIO_NOT_IDLE,
    UE_REMOTE_START_VR_FAIL_AUDIO_CONNECTED,
    UE_REMOTE_START_VR_FAIL_VR_IS_OPENING,
    UE_REMOTE_START_VR_FAIL_WAKEUP_VR,
    UE_REMOTE_START_VR_FAIL_SCO_CONNECTED,
    // 0x000B~0x00FF reversed
    UE_LOCAL_START_VR_RECV_OPEN = 0x0100,
    UE_LOCAL_START_VR_HF_REQ_ACCEPT,
    UE_LOCAL_START_VR_REQ_ACCEPT,
    UE_LOCAL_START_VR_FAIL_VR_IS_OPENED,
    UE_LOCAL_START_VR_FAIL_AUDIO_NOT_IDLE,
    UE_LOCAL_START_VR_FAIL_AUDIO_CONNECTED,
    UE_LOCAL_START_VR_FAIL_AG_NOT_CONNECTED,
    UE_LOCAL_START_VR_FAIL_SCO_CONNECTED,
};

enum BtChrUeStopVoiceRegScene {
    UE_REMOTE_STOP_VR_RECV_REQ = 0x0000,
    UE_REMOTE_STOP_VR_REQ_ACCEPT,
    UE_REMOTE_STOP_VR_RECV_BVRA_REQ,
    UE_REMOTE_STOP_VR_FAIL_NOT_ACTIVE_DEV,
    UE_REMOTE_STOP_VR_FAIL_VR_NOT_OPENED,
    UE_REMOTE_STOP_VR_FAIL_DISCONN_SCO,
    // 0x0006~0x00FF reversed
    UE_LOCAL_STOP_VR_RECV_REQ = 0x0100,
    UE_LOCAL_STOP_VR_FAIL_VR_NOT_OPENED,
    UE_LOCAL_STOP_VR_FAIL_AG_NOT_CONNECTED,
    UE_LOCAL_STOP_VR_FAIL_NOT_ACTIVE_DEV,
    UE_LOCAL_STOP_VR_FAIL_DISCONN_SCO,
};


struct BtChrEventParam {
    std::string name {};
    BtChrEventParamType type = BTCHREVENT_INT32;
    int32_t i32Value = 0;
    uint32_t ui32Value = 0;
    std::string strValue {};
    std::vector<int64_t> i64ArrayValue {};
    std::vector<std::string> strArrayValue {};

    BtChrEventParam(std::string paramName, int32_t paramValue)
    {
        name = paramName;
        type = BTCHREVENT_INT32;
        i32Value = paramValue;
    }

    BtChrEventParam(std::string paramName, uint32_t paramValue, uint8_t paramType)
    {
        name = paramName;
        type = BTCHREVENT_UINT32;
        ui32Value = paramValue;
    }

    BtChrEventParam(std::string paramName, std::string paramValue)
    {
        name = paramName;
        type = BTCHREVENT_STRING;
        strValue = paramValue;
    }

    BtChrEventParam(std::string paramName, const std::vector<int64_t> &paramValue)
    {
        name = paramName;
        type = BTCHREVENT_INT64_ARRAY;
        i64ArrayValue = paramValue;
    }

    BtChrEventParam(std::string paramName, const std::vector<std::string> &paramValue)
    {
        name = paramName;
        type = BTCHREVENT_STRING_ARRAY;
        strArrayValue = paramValue;
    }

    bool operator ==(const BtChrEventParam &eventParam) const
    {
        return name == eventParam.name;
    }
};

namespace OHOS {
namespace bluetooth {
/*******************************************************************************
 *  Functions
 ******************************************************************************/
void BtChrEventWriteInt(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    int32_t paramValue);
void BtChrEventWriteStr(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    const std::string& paramValue);
void BtChrEventWriteTime(const std::string& eventName, const std::string& peerAddr, const std::string& paramName);
void BtChrBaseReleaseAll(const std::vector<std::string> &notClearAddrs);
void BtChrUeEventWrite(const std::string& eventName, std::vector<BtChrEventParam>& params);
void BtChrStaticticsEventWrite(uint8_t eventType, const std::string& eventValue);
void BtChrResetEventWrite(uint8_t eventType, const std::string& eventValue);
void BtChrEventWriteParams(const std::string& eventName, const std::string& peerAddr,
    std::vector<BtChrEventParam> params);
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_BASE_H */
