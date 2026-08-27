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

#ifndef BLUETOOTH_HW_BT_HWIF_H
#define BLUETOOTH_HW_BT_HWIF_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "hardware/bluetooth.h"
#include "hw_hcidefs.h"
#include "types/raw_address.h"

#ifndef STACK
#define STACK
#endif

/* Opaque key used to retrieve the vendor interface from the bt stack. */
#define BT_VENDER_INTERFACE_ID "bt_vender_interface"

namespace OHOS {
namespace bluetooth {

/* Audio manager package name used for CHR UE reporting. */
constexpr const char *AUDIO_MANAGER_PKG_NAME = "com.ohos.audiomanager";

/* Set connection attribution. Supported attribution see the definition of HwConnAttrType */
enum HwConnAttrType : uint8_t {
    HW_CONN_ATTR_TYPE_KEEP_BLE_SCAN_IN_CONN = 1,
    HW_CONN_ATTR_TYPE_SET_LP_DEVICE_BUFFER_SIZE_AND_COUNT = 9,
};

struct HwConnAttr {
    uint8_t type;
    bool keepBleScanInConn = false;
};

/* Stack callback parameter for socket connection observer */
struct StackCallbackParam {
    STACK::RawAddress addr;
    bt_status_t status;
    int result;
    int type;
    int psm;
    uint64_t msb;
    uint64_t lsb;
};

enum BtStackErrno : int32_t {
    BT_STACK_ERRNO_SUCCESS = 0,
};

/* Hdap (L2HC) audio session configuration */
struct HdapConfigCallback {
    uint32_t sampleRate;
    uint8_t bitWidth;
    uint8_t channel;
};

/* A2DP offload codec configuration */
struct A2dpOffloadConfigCallback {
    uint16_t media_packet_header;
    uint8_t m_pt;
    uint32_t ssrc;
    uint8_t boundary_flag;
    uint8_t broadcast_flag;
    uint32_t codec_type;
    uint16_t max_latency;
    uint16_t scms_t_enable;
    uint32_t sample_rate;
    uint32_t encoded_audio_bitrate;
    uint8_t bits_per_sample;
    uint8_t ch_mode;
    uint16_t acl_hdl;
    uint16_t l2c_rcid;
    uint16_t mtu;
    uint8_t codec_specific_0;
    uint8_t codec_specific_1;
    uint8_t codec_specific_2;
    uint8_t codec_specific_3;
    uint8_t codec_specific_4;
    uint8_t codec_specific_5;
    uint8_t codec_specific_6;
    uint8_t codec_specific_7;
};

/* A2DP stream session info */
struct HwBtA2dpSteamSessionInfo {
    int32_t sessionId;
    int32_t streamType;
    int32_t sampleRate;
    int32_t isSpatialAudio;
    bool isPlaying;
};

/* HW Bluetooth stack callbacks, dispatched from the closed-source HAL.
 * Field order must match the sBluetoothHWCallbacks initializer. */
typedef struct {
    size_t size;
    void (*reserved1)(void);
    void (*stack_errno_cb)(bt_status_t status, STACK::RawAddress *addr,
                           BtStackErrno state);
    void (*reserved3)(void);
    void (*hdap_connect_cb)(STACK::RawAddress *bdAddr, bool isConnected,
                            uint8_t featureBit, HdapConfigCallback config);
    void (*profile_state_cb)(STACK::RawAddress *addr, uint8_t a2dpState,
                             uint8_t hfpState, std::string targetBtDevice,
                             uint8_t a2dpServiceType);
    void (*reserved6)(void);
    void (*acl_disconn_reason_cb)(STACK::RawAddress *addr, int reason);
    void (*sensorhub_dev_info_cb)(uint8_t *buffer, int length);
    void (*sensorhub_reset_cb)(uint32_t state);
    void (*hiecho_ind_call_cb)(uint8_t echoType, uint8_t *payload, uint16_t len,
                               uint8_t *args, uint16_t argsLen);
    void (*bluetooth_conn_cb)(const StackCallbackParam &param);
    void (*reserved12)(void);
    void (*reserved13)(void);
    void (*device_cb)(STACK::RawAddress *addr, int deviceType);
    void (*reserved15)(void);
    void (*a2dp_offload_state_cb)(STACK::RawAddress *addr, bool isOffload);
    void (*a2dp_offload_codec_config_cb)(STACK::RawAddress *bdAddr,
                                         A2dpOffloadConfigCallback config);
    void (*hiecho_device_acl_encryption_changed_cb)(
        const STACK::RawAddress &remoteAddr);
    void (*report_a2dp_abnormal_status_cb)(
        const STACK::RawAddress &remoteAddr, int32_t errCode);
#ifdef CONTEXTHUB_BLE_V3
    void (*sensorhub_collaboration_cb)(const std::vector<uint8_t> &notifyValue);
#endif
    void *end_marker[1];
} BthwifCallbacks;

/* Closed-source vendor HW interface. All function pointers may be null when
 * the vendor HAL is unavailable; callers must null-check before use. */
typedef struct {
    size_t size;
    /* Set connection attribution. */
    void (*hwConnAttrSet)(uint8_t transport, const STACK::RawAddress &addr,
                          const HwConnAttr *attr);
    /* Send vendor private echo data. */
    void (*sendHiechoData)(uint8_t type, const uint8_t *data, int len,
                           const uint8_t *args);
    /* Query local device echo product type. */
    uint8_t (*hwGetLocalDeviceEchoType)(void);
    /* Check whether the remote device supports echo. */
    bool (*hwIsSupportEchoDevice)(const STACK::RawAddress &addr);
    /* Check whether the remote device is bonded. */
    bool (*checkRemoteDeviceBonded)(const STACK::RawAddress &addr);
    /* Interoperability feature matching. */
    bool (*interopMatch)(const uint16_t feature,
                         const STACK::RawAddress &remote_bdaddr);
    /* Interoperability feature matching (Huawei wrapper). */
    bool (*InteropMatch)(const uint16_t feature,
                         const STACK::RawAddress &remote_bdaddr);
    /* Clean the HFP SCO occupied flag. */
    void (*hwBtifHfpScoOccupiedClean)(const STACK::RawAddress &remote_bdaddr);
    /* Remove the HFP SCO queue entry. */
    void (*hwBtifHfpQueueRemove)(const STACK::RawAddress &remote_bdaddr);
    /* Remove the A2DP queue entry. */
    void (*hwBtifA2dpQueueRemove)(const STACK::RawAddress &remote_bdaddr);
    /* Get A2DP offload codec config. */
    bool (*getA2dpOffloadCodecConfig)(
        const STACK::RawAddress &remote_bdaddr,
        A2dpOffloadConfigCallback &config);
    /* Initialize the vendor HW interface. */
    bt_status_t (*Init)(BthwifCallbacks *callbacks);
    /* Set advertising key. */
    void (*hwSetAdvKey)(const STACK::RawAddress &addr, const char &irk,
                        const char &hbk, const uint8_t version,
                        const char &feature);
    /* Remove advertising key. */
    void (*hwRemoveAdvKey)(const STACK::RawAddress &addr);
    /* Get advertising IRK. */
    void (*hwGetAdvIrk)(const STACK::RawAddress &addr,
                        std::vector<uint8_t> &irk);
    /* Enable/disable Hisilicon PCM. */
    void (*hwEnableHisiPcm)(bool state);
    /* Send foreground app scene type. */
    void (*hwSendSceneType)(int type);
    /* Switch satellite/antenna mode. */
    void (*hwSatelliteModeSwitch)(int state);
    /* Set MAC id. */
    void (*hwSetMacId)(int state);
    /* Set connection reason flag. */
    void (*hwSetConnReasonFlag)(const STACK::RawAddress &addr, int connType,
                                const uint8_t *data, size_t len);
    /* Start an Hdap (L2HC) audio session. */
    void (*hdapStartSession)(uint32_t sampleRate, uint8_t bitWidth,
                             uint8_t channel);
    /* Set Hdap active device. */
    void (*setHdapActive)(const STACK::RawAddress &addr);
    /* Send params to sensorhub. */
    void (*sendParamsToSensorhub)(const char *params, int32_t type, int length);
    /* Reset sensorhub. */
    void (*resetSensorhub)(bool reset);
    /* Query sensorhub capability. */
    bool (*getSensorhubCap)(void);
    /* Get sensorhub adv handle. */
    int (*getSensorhubAdvHandle)(void);
    /* Delete sensorhub filter. */
    void (*deleteSensorhubFilter)(int idx);
    /* Set burst parameter. */
    void (*setBurstParam)(int duration, int maxExtAdvEvents, int window,
                          int interval, int advHandle);
    /* Set scan report channel for sensorhub. */
    void (*setScanReportChannelSensorHub)(int index, uint8_t reportChannel);
    /* Set BLE scan enhance mode status. */
    void (*hwSetBleScanEnhanceModeStatus)(bool enable);
    /* Send HCI disconnect. */
    void (*hwBtsndHcicDisconnect)(const STACK::RawAddress &addr);
    /* Check whether bonding or SDP in progress. */
    bool (*isBondingOrSdp)(void);
    /* Check whether incoming connection. */
    bool (*isIncomingConnection)(const STACK::RawAddress &addr);
    /* Get device info into property. */
    bool (*hwGetDeviceInfo)(const STACK::RawAddress &addr,
                            bt_property_t &prop);
    /* Check whether paired info needs sync. */
    bool (*needSyncPairedInfo)(std::vector<std::string> &pairedAddr);
    /* Fast LE connect. */
    void (*leConnectFastest)(const STACK::RawAddress &addr);
    /* Set LE connection scan to fast mode. */
    void (*hwSetLeConnectionScanToFast)(bool enable);
    /* Register connection observer. */
    void (*registerConnection)(const STACK::RawAddress &addr,
                               const ::bluetooth::Uuid &uuid);
    /* Unregister connection observer. */
    void (*unRegisterConnection)(const STACK::RawAddress &addr,
                                 const ::bluetooth::Uuid &uuid);
    /* Update CoC connection parameters. */
    void (*updateCocConnectionParams)(const STACK::RawAddress &addr,
                                      const uint16_t *params, int len);
    /* Send device selection. */
    void (*sendDeviceSelection)(const STACK::RawAddress &device, int useA2dp,
                                int useHfp, int userSelection);
    /* Check whether device supports echo. */
    bool (*isDeviceSupportEcho)(const STACK::RawAddress &device);
    /* Check whether device is a dual-conn device. */
    bool (*isHwDualConnDevice)(const STACK::RawAddress &addr);
    /* Change codec. */
    void (*changeCodec)(int preferedScene, int32_t sampleRate, bool fastFlag);
    /* A2DP offload state queries and control. */
    bool (*a2dpOffloadIsRunning)(const STACK::RawAddress &addr);
    int (*a2dpOffloadGetLatency)(void);
    void (*a2dpOffloadSetSinkStreamStart)(const STACK::RawAddress &addr);
    void (*a2dpOffloadSetSinkStreamStop)(const STACK::RawAddress &addr);
    int (*a2dpOffloadGetSinkStreamEncodingPath)(
        const STACK::RawAddress &addr,
        std::vector<HwBtA2dpSteamSessionInfo> &sessions, bool check);
    /* Spatial audio control. */
    void (*hwSetSpatialAudioModeEnabled)(bool enabled);
    void (*hwSetSpatialAudioHeadTrackingEnabled)(bool enabled);
    void (*hwSetActiveMode)(const STACK::RawAddress &addr);
    void (*hwSetAdaptiveSwitchStatus)(bool status);
    void (*hwSetAudioSourceType)(int mode);
    void (*HwSetCollAudioEnableState)(bool enabled);
    /* Audio render state and latency. */
    void (*notifyAudioRenderState)(bool isRenderActive);
    void (*sendLowLatencyStatus)(bool status);
    int (*hwGetCurrentSceneType)(void);
    void (*hwUpdateBtAudioLatency)(const STACK::RawAddress &addr,
                                   int32_t latency);
    /* Voice combine (dual-audio) control. */
    void (*hwSetLocalVoiceCombineFeatureState)(int featureState);
    void (*hwSetLocalVoiceCombineNnState)(int32_t nnState);
    int (*hwGetLocalVoiceCombineFeatureState)(void);
    bool (*hwGetRemoteVoiceCombineAbility)(const STACK::RawAddress &addr);
    void (*hwSetVoiceCombineScenarioEnabled)(const STACK::RawAddress &addr,
                                             bool isCall);
    /* Set/delete advertising indication report flag. */
    void (*hwSetAdvIndReportFlag)(int filterIndex, bool flag);
    void (*hwDeletAdvIndReportFlag)(int index);
    /* Send vendor-specific HCI command. */
    void (*hwVendorSpecificCommand)(const SpecificCmdToHisi *cmd);
    /* Apply dynamic configuration from param update events. */
    void (*hwDynamicConfig)(void);
} bthwif_interface_t;

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_BT_HWIF_H
