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

#ifndef BT_CHR_DFT_EXCEPTION_H
#define BT_CHR_DFT_EXCEPTION_H
#include "bt_chr_base.h"
#include "bt_chr_subevent_manager.h"

#define BT_CHR_TRANSPORT_LE 0
#define BT_CHR_TRANSPORT_BR_EDR 1

#define BT_SCO_CODEC_NONE 0x0000
#define BLUETOOTH_SUB_SYSTEM 201
#define BLUETOOTH_ERRNO(module) ((((BLUETOOTH_SUB_SYSTEM) << 21) | ((module) << 16) | (0xFFFF)))

enum SoftBusSubModule {
    PUBLIC_SUB_MODULE_CODE = 1,
    CONNECT_SUB_MODULE_CODE = 2,
};

const int CHR_ACTIVE_CONNECT_FAIL_OFFSET = 10000;
const uint8_t TX_MAX_INTERVEL_MS = 100; // uint:ms
const uint8_t TX_MIN_INTERVEL_MS = 60;

/* ble connect profile type */
enum BleProfileType {
    CHR_BLE_COC = 1,
    CHR_BLE_GATT = 2,
};

/* customization auth error code */
enum CustomAuthErrCode {
    CHR_AUTH_CUSTOM_LINK_KEY_REQ_REPLY = 10000,
    CHR_AUTH_CUSTOM_LINK_KEY_NEG_REPLY = 10006,
};

// 913000004 ErrCode
enum BtExcptErrType {
    BTOPT_SERVICE = 1,
    BTOPT_A2DP,
    BTOPT_HFP,
    BTOPT_OPP,
    BTOPT_MTU_ERR_RECOVERY = 11,
    BTOPT_RESOURCE_BLE_NO_FREE = 12,
    BTOPT_GATT_RES_MAX = 13,
    BTOPT_SNIFF_RES_MAX = 14,
    BTOPT_RSND_L2CAP_CONNECT_RSP = 15,
    BTOPT_RESET_SENT_NOT_ACKED = 16,
    BTOPT_BT_RESET = 17,
    BTOPT_A2DP_REPORT_ERR = 18,
    BTOPT_SDP_UUID_LEN_ZERO = 19,
    BTOPT_L2CAP_NO_RESOURCE_RETRY = 20,
    BTOPT_L2CAP_SECURITY_BLOCK = 21,
    BTOPT_HCI_COMMAND_TIMEOUT = 22,
    BTOPT_HCI_COMMAND_TIMEOUT_BT_RESET = 23,
    BTOPT_L2CAP_AVCT_CFM_FAIL_RES = 24,
    BTOPT_ALREADY_PAIRED = 25,
    BTOPT_L2CAP_CONN_NO_RESOURCES = 26,
    BTOPT_GATT_RSP_TIMEOUT = 27,
    BTOPT_L2CAP_RSP_TIMEOUT = 28,
    BTOPT_SSP_AUTH_IO_REQ = 29,
    BTOPT_L2CAP_RESTORE_IDLE_TIMEOUT = 30,
    BTOPT_L2CAP_LINK_TYPE_ABNORMAL = 31,
    BTOPT_RFCOMM_PORT_CHECK_FAIL = 32,
    BTOPT_GATTC_NOTIF_REG_MAX = 33,
    BTOPT_HCI_COMPLETE_STATUS_ERR = 34,
    BTOPT_AUTH_COLLISION_CANCEL = 35,
    BTOPT_NO_CANCELDISCOVERY = 36,
    BTOPT_RESERVED = 37,
    BTOPT_BLE_DISCONNECT = 38,
    BTOPT_RFCOM_DM = 39,
    BTOPT_DEVICE_UUIDS_EMPTY = 40,
    BTOPT_TWS_AUDIO_FAIL = 41,
    BTOPT_MULTCONNECT_AUTOCONNECT_FAIL = 42,
    BTOPT_TWS_NOT_ALLOWED_SET_BLE_RANDOM_ADDRESS = 43,
    BTOPT_BLE_SCAN_FILTER_FAIL = 46,
    BTOPT_DENYLIST_OPERATION = 47,
    BTOPT_ALLOCATED_TWS_RECORD_EXCP = 49,
    BTOPT_QUERY_MAP_DATASHARE_FAIL = 50,
};

// Common SubErrCode
enum BtCommonSubErrCode {
    CHR_SUB_ERRCODE_CASE1 = 1,
    CHR_SUB_ERRCODE_CASE2,
    CHR_SUB_ERRCODE_CASE3,
    CHR_SUB_ERRCODE_CASE4,
    CHR_SUB_ERRCODE_CASE5,
    CHR_SUB_ERRCODE_CASE6,
    CHR_SUB_ERRCODE_CASE7,
    CHR_SUB_ERRCODE_CASE8,
    CHR_SUB_ERRCODE_CASE9,
    CHR_SUB_ERRCODE_CASE10,
    CHR_SUB_ERRCODE_CASE11,
    CHR_SUB_ERRCODE_CASE12,
    CHR_SUB_ERRCODE_CASE13,
};

// 913000019 SubErrCode
enum BtPairExcpSubErrCode {
    SUBERRCODE_FAIL_CREATEBOND_NATIVE = 0x100,
    SUBERRCODE_FAIL_CREATEBOND_NOT_ALLOW = 0x101,
    SUBERRCODE_FAIL_CREATEBOND_COMMON = 0x102,
};

/* 913000019 customization bond modetype */
enum CustomBondModeType {
    BOND_MODE_TYPE_PIN_REQ = 10,
    BOND_MODE_TYPE_SP_CFM_REQ,
};

// 913000021 ErrCode
enum ErrTypeAvdtpClose {
    ERRTYPE_SUSPEND_CFM = 3,
    ERRTYPE_RCFG_FAILED = 4,
    ERRTYPE_API_DEREGISTER = 5,
    ERRTYPE_RECONFIG = 6,
    ERRTYPE_SUSPEND_CONT = 7,
    ERRTYPE_RCFG_CFM = 8,
    ERRTYPE_AVDTP_OPEN_CLOSE_DIFF = 9,
};

// 913000027 ErrCode
enum A2dpAudioErrType {
    CHR_A2DP_ERR_TYPE_START_FAIL = 1,
    CHR_A2DP_ERR_TYPE_SUSPEND_FAIL,
    CHR_A2DP_EXCEPTION_DROP,
    CHR_A2DP_EXCEPTION_UNDERFLOW,
    CHR_A2DP_ERR_TYPE_VOLUME_LOW, // apk used
    CHR_A2DP_ERR_TYPE_AUTO_PLAY, // apk used
    CHR_A2DP_ERR_TYPE_OUT_SPEAKER, // apk used
    CHR_A2DP_ERR_TYPE_VOLUME_0, // apk used
    CHR_A2DP_EXCEPTION_TX_SLOW,
    CHR_A2DP_EXCEPTION_RESERVER, // reserve
    CHR_HDAP_RECORD_CHOPPY_EXCEPTION,
    CHR_HDAP_COMMAND_EXCEPTION,
    CHR_AVRCP_NOT_SEND_NOT_BONDED = 13,
    CHR_AVRCP_PLAY_INTERCEPT,
    CHR_AVRCP_PAUSE_INTERCEPT,
    CHR_AVDTP_CONN_FAIL,
    CHR_AVDTP_START_BIND_SUSPEND, // Mutual influence triggering
    CHR_AVDTP_ABORT_COMMON,
    CHR_AVRCP_BLACK_LIST_COMMON,
    CHR_AVDTP_HDL_SETCONFIG_REJECT = 20, // local recieve setconfig reject
    CHR_AVDTP_HDL_SETCONFIG_SND_REJECT, // local recieve setconfig send reject
    CHR_AVDTP_SND_SETCONFIG_REJECT, // local send setconfig reject
    CHR_AVRCP_PASSTHROUGH_INTERCEPT_COMMON,
    CHR_AVDTP_HDL_SETCONFIG_COMMON, // local recieve setconfig
    CHR_A2DP_ERR_TYPE_SET_A2DP_DEV = 26,
    CHR_HWTWS_ISO_EXCEPTION = 27,
    CHR_AVDTP_SKIP_SDP_INCOMMING_CONN = 28,
    CHR_AVDTP_HITWS_RECONFIG_FAIL = 29,
    CHR_A2DP_EARBUD_EXCEPTION = 31,
    CHR_A2DP_OFFLOAD_STATE_ERROR = 32,
    CHR_A2DP_SERVICE_ERR_TYPE_MAX
};

enum A2dpAudioSubErrCode {
    SUBERR_A2DP_START_FAIL_NOT_UPDATE = 0x100,
};

// 913000028 ErrCode
enum HFPServiceErrCode {
    ERRCODE_SCO_DISC_SLOW = 1,
    ERRCODE_SCO_BUILD_SLOW = 2,
    ERRCODE_SCO_BUILD_FAIL = 3,
    ERRCODE_SCO_AB_DISC = 4,
    ERRCODE_SCO_IN_CALL = 5,
    ERRCODE_HFP_BLACK_LIST_COMMON,
    ERRCODE_HFP_CONN_TIMEOUT,
    ERRCODE_SCO_DISC_BY_RMT, // peer disconnect sco reason
    ERRCODE_VOICE_RECOGNITION,
    ERRCODE_START_VIRTUAL_VOICE_CALL,
    ERRCODE_CALLTYPE_STATUS = 13,
    ERRCODE_STATEMACHINES_STATUS = 14,
    ERRCODE_HFP_ATCOMNAND_ERROR = 15,
};

/* 913000028 SubErrCode 0x01-0xff Standard, Customizing from the begin 0x100 */
enum HfpScoBuildFailSubErrCode {
    SUBERR_HCI_HOST_REJECT_RESOURCES = 0x0D,
    SUBERR_HFP_AG_SCO_FAIL_NEG_FAIL = 0x100,
    SUBERR_HFP_AG_SCO_FAIL_NO_RESOURCE_CASE1,
    SUBERR_HFP_AG_SCO_FAIL_NO_RESOURCE_CASE2,
    SUBERR_HFP_AG_SCO_FAIL_UNKNOWN_ADDR,
    SUBERR_HFP_AG_SCO_FAIL_BTM_BUSY,
    SUBERR_HFP_AG_SCO_FAIL_TWS_MUSIC,
    SUBERR_HFP_AG_SCO_FAIL_TWS_HDAP,
    SUBERR_HFP_AG_SCO_FAIL_TIMEOUT = 0x107,
};

// 913000050 SubErrCode
enum DisconnectSubErrCode {
    FIRST_UNPAIRED_BR_LINK_CHR = 665,
    MAX_BR_LINKS_CHR = 666,
    RESTRICIT_BLUETOOTH_DISCONNECT_BR_LINKS = 671,
    REMOVEPAIR_BT_DISCONNECT_ACL = 672,
    DISABLE_BLUETOOTH_DISCONNECT_BR_LINKS = 673,
};

// 913000053 ErrCode
enum SniffErrCode {
    ERRCODE_ROLE_SWITCH = 1,
    ERRCODE_SNIFF_A2DP_STREAM,
    ERRCODE_SNIFF_SCO_ACTIVE,
};

// 913000012 ErrCode
enum HidExceptionErrCode {
    ERRCODE_BLE_ADD_2_WHITE_LIST_FAIL = 1,
    ERRCODE_BLE_REMOVE_WHITE_LIST_FAIL,
    ERRCODE_BLE_PARAM_UPDATE_ERRCODEOR,
    ERRCODE_HID_HOST_REGISTER_ERRCODEOR,
    ERRCODE_HID_HOST_L2CAP_EXCEPTION,
    ERRCODE_HID_DEVICE_L2CAP_EXCEPTION,
    ERRCODE_HID_SKIP_SDP,
};

// Hci Error Code defined in hci_error_code.h
enum HciErrorCode {
    HCI_ERRCODE_SUCCESS = 0x00,
    HCI_ERRCODE_NO_CONNECTION = 0x02,
    HCI_ERRCODE_MEMORY_FULL = 0x07,
    HCI_ERRCODE_CONN_CAUSE_LOCAL_HOST = 0x16,
    HCI_ERRCODE_UNDEFINED = 0xff,
};

enum BtSwitchAtomicEventType {
    EVENT_TYPE_BT_RESET = 5,  // 该枚举类型用于上报蓝牙芯片复位的消息给CHR仓，CHR仓进行trace抓取，值固定为5，不能变动
    EVENT_TYPE_BT_ENABLE,
    EVENT_TYPE_BT_DISABLE,
    EVENT_TYPE_BT_ENABLE_SUCCESS,
    EVENT_TYPE_BT_DISABLE_SUCCESS,
    EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET,
    EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON,
};

// 913000051 ErrCode
enum BtSocketExceptionErrCode {
    CLIENT_CONNECT_SUCCESS = 0, // notify Bluetooth apk connect success
    CLIENT_CONNECT_REFUSE = 1, // Bluetooth apk use
    CLIENT_CONNECT_START_FAIL,
    CLIENT_CONNECT_SDP_FAIL,
    CLIENT_CONNECT_RFC_FAIL,
    CLIENT_SOCKET_CLOSE = 5,
    SERVER_REGISTER_FAIL,
    SERVER_ACCEPT_FAIL,
    SERVER_SOCKET_CLOSE,
    RFC_SIGNAL_FAIL,
    CLIENT_CONNECT_PENDING = 10,
    CLIENT_CONNECT_START,
    L2CAP_CLIENT_CONNECT_FAIL,
    L2CAP_SERVER_REGISTER_FAIL,
    SERVER_SOCKET_CONN_FAIL,
};

// 913000051 SubErrCode
enum SocketConnectSubErrCode {
    SOCKET_SUBERROR_TOO_FREQUENTLY = 1,
    SOCKET_SUBERROR_PROFILE_CONNECTING = 2,
    SOCKET_SUBERROR_LINK_LIMIT = 3,
};

enum PublicErrCode {
    PUBLIC_ERR_BASE = BLUETOOTH_ERRNO(PUBLIC_SUB_MODULE_CODE),
    SOCKET_CONNECT_ERRCODE = -(PUBLIC_ERR_BASE + 1),
    CONNECT_TOO_FREQUENTLY = -(PUBLIC_ERR_BASE + 2),
    CONNECT_CONNECTING_LINK_LIMIT = -(PUBLIC_ERR_BASE + 3),
    SOCKET_INTERFACE_INVALID = -(PUBLIC_ERR_BASE + 4),
    ADDR_ERROR = -(PUBLIC_ERR_BASE + 6),
};

enum SocketConnectErrCode {
    CONNECT_ERR_BASE = BLUETOOTH_ERRNO(CONNECT_SUB_MODULE_CODE),
};

enum HdapRecordErrCode {
    ERRCODE_HDAP_RECORD_INIT_FAIL = 1,
    ERRCODE_HDAP_RECORD_START_FAIL = 2,
    ERRCODE_HDAP_RECORD_WRITE_DATA_FAIL = 3,
    ERRCODE_HDAP_RECORD_STOP_FAIL = 4,
    ERRCODE_HDAP_RECORD_CLEAN_FAIL = 5,
};

enum HdapRecordInitFailSubErrCode {
    SUBERR_HDAP_RECORD_PARAM_EXCEP = 1,
    SUBERR_HDAP_RECORD_INIT_DECODER_FAIL = 2,
    SUBERR_HDAP_RECORD_START_SESSION_FAIL = 3,
};

namespace OHOS {
namespace bluetooth {
/*******************************************************************************
 *  Functions
 ******************************************************************************/
void BtChrDftEventWriteInt(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    int32_t paramValue);
void BtChrDftEventWriteTime(const std::string& eventName, const std::string& peerAddr, const std::string& paramName);

/* CHR_BT_EXCEPTION_NEW begin*/
void BtChrBtExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode);
/* CHR_BT_EXCEPTION_NEW end*/

/* CHR_BT_PAIR_EXCEPTION begin*/
void BtChrRemovePairHandle(const std::string& peerAddr, int32_t bondedCnt);
void BtChrRemoveAllPairHandle(std::vector<std::string> removeAddrs);
/* CHR_BT_PAIR_EXCEPTION end*/

/* CHR_A2DP_AUDIO_EXCEPTION begin*/
void BtChrA2dpAudioExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode);
void BtChrSetAudioVolume(const std::string& peerAddr, int32_t audioVolume);
void BtChrAudioStateStart(const std::string& peerAddr);
/* CHR_A2DP_AUDIO_EXCEPTION begin*/

/* CHR_AVRCP_CONNECT_EXCEPTION begin*/
void BtChrAvrcpConnectExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode);
/* CHR_AVRCP_CONNECT_EXCEPTION end*/

/* CHR_HFP_AUDIO_EXCEPTION begin*/
void BtChrCallExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode, int32_t curCodec);
/* CHR_HFP_AUDIO_EXCEPTION end*/

/* CHR_HID_EXCEPTION begin*/
void BtChrHidExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode);
/* CHR_HID_EXCEPTION end*/

/* CHR_BT_SOCKET_EXCEPTION begin*/
void BtChrAddSocketCallingRecord(const std::string& peerAddr, const std::string& uuid, const std::string& packageName);
std::string BtChrSocketExcpEvent(const std::string& peerAddr, const std::string& uuid, int32_t errCode,
    int32_t subErrCode);
void BtChrSocketExcpEvent(const std::string& peerAddr, const std::string& uuid, int32_t errCode,
    int32_t subErrCode, std::string packageName);
/* CHR_BT_SOCKET_EXCEPTION end*/

/* CHR_BLE_DISCONNECT begin*/
void BtChrBleConnData(const std::string &peerAddr, int type, const std::string &callingName);

void BtChrReleaseByAddrs(const std::vector<std::string> &notClearAddrs);

void BtChrResetExcpEvent(const std::string& eventValue);

/* CHR_HDAP_AUDIO_EXCEPTION begin*/
void BtChrHdapAudioExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode);
/* CHR_HDAP_AUDIO_EXCEPTION end*/

}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_DFT_EXCEPTION_H */
