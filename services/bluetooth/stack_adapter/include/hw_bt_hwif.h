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
#include "raw_address.h"

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
    OHOS::bluetooth::RawAddress addr;
    BtStackStatus status;
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
    uint16_t mediaPacketHeader;
    uint8_t mPt;
    uint32_t ssrc;
    uint8_t boundaryFlag;
    uint8_t broadcastFlag;
    uint32_t codecType;
    uint16_t maxLatency;
    uint16_t scmsTEnable;
    uint32_t sampleRate;
    uint32_t encodedAudioBitrate;
    uint8_t bitsPerSample;
    uint8_t chMode;
    uint16_t aclHdl;
    uint16_t l2cRcid;
    uint16_t mtu;
    uint8_t codecSpecific0;
    uint8_t codecSpecific1;
    uint8_t codecSpecific2;
    uint8_t codecSpecific3;
    uint8_t codecSpecific4;
    uint8_t codecSpecific5;
    uint8_t codecSpecific6;
    uint8_t codecSpecific7;
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
struct BthwifCallbacks {
    size_t size;
    void (*reserved1)(void);
    void (*stackErrnoCb)(BtStackStatus status, OHOS::bluetooth::RawAddress *addr,
                           BtStackErrno state);
    void (*reserved3)(void);
    void (*hdapConnectCb)(OHOS::bluetooth::RawAddress *bdAddr, bool isConnected,
                            uint8_t featureBit, HdapConfigCallback config);
    void (*profileStateCb)(OHOS::bluetooth::RawAddress *addr, uint8_t a2dpState,
                             uint8_t hfpState, std::string targetBtDevice,
                             uint8_t a2dpServiceType);
    void (*reserved6)(void);
    void (*aclDisconnReasonCb)(OHOS::bluetooth::RawAddress *addr, int reason);
    void (*sensorhubDevInfoCb)(uint8_t *buffer, int length);
    void (*sensorhubResetCb)(uint32_t state);
    void (*hiechoIndCallCb)(uint8_t echoType, uint8_t *payload, uint16_t len,
                               uint8_t *args, uint16_t argsLen);
    void (*bluetoothConnCb)(const StackCallbackParam &param);
    void (*reserved12)(void);
    void (*reserved13)(void);
    void (*deviceCb)(OHOS::bluetooth::RawAddress *addr, int deviceType);
    void (*reserved15)(void);
    void (*a2dpOffloadStateCb)(OHOS::bluetooth::RawAddress *addr, bool isOffload);
    void (*a2dpOffloadCodecConfigCb)(OHOS::bluetooth::RawAddress *bdAddr,
                                         A2dpOffloadConfigCallback config);
    void (*hiechoDeviceAclEncryptionChangedCb)(
        const OHOS::bluetooth::RawAddress &remoteAddr);
    void (*reportA2dpAbnormalStatusCb)(
        const OHOS::bluetooth::RawAddress &remoteAddr, int32_t errCode);
#ifdef CONTEXTHUB_BLE_V3
    void (*sensorhubCollaborationCb)(const std::vector<uint8_t> &notifyValue);
#endif
    void *endMarker[1];
};

/* Closed-source vendor HW interface. All function pointers may be null when
 * the vendor HAL is unavailable; callers must null-check before use. */
struct BthwifInterface {
    size_t size;
    /* Set connection attribution. */
    void (*hwConnAttrSet)(uint8_t transport, const OHOS::bluetooth::RawAddress &addr,
                          const HwConnAttr *attr);
    /* Send vendor private echo data. */
    void (*sendHiechoData)(uint8_t type, const uint8_t *data, int len,
                           const uint8_t *args);
    /* Query local device echo product type. */
    uint8_t (*hwGetLocalDeviceEchoType)(void);
    /* Check whether the remote device supports echo. */
    bool (*hwIsSupportEchoDevice)(const OHOS::bluetooth::RawAddress &addr);
    /* Check whether the remote device is bonded. */
    bool (*checkRemoteDeviceBonded)(const OHOS::bluetooth::RawAddress &addr);
    /* Interoperability feature matching. */
    bool (*interopMatch)(const uint16_t feature,
                         const OHOS::bluetooth::RawAddress &remoteBdAddr);
    /* Interoperability feature matching (Huawei wrapper). */
    bool (*hwInteropMatch)(const uint16_t feature,
                           const OHOS::bluetooth::RawAddress &remoteBdAddr);
    /* Clean the HFP SCO occupied flag. */
    void (*hwBtHfpScoOccupiedClean)(const OHOS::bluetooth::RawAddress &remoteBdAddr);
    /* Remove the HFP SCO queue entry. */
    void (*hwBtHfpQueueRemove)(const OHOS::bluetooth::RawAddress &remoteBdAddr);
    /* Remove the A2DP queue entry. */
    void (*hwBtA2dpQueueRemove)(const OHOS::bluetooth::RawAddress &remoteBdAddr);
    /* Get A2DP offload codec config. */
    bool (*getA2dpOffloadCodecConfig)(
        const OHOS::bluetooth::RawAddress &remoteBdAddr,
        A2dpOffloadConfigCallback &config);
    /* Initialize the vendor HW interface. */
    BtStackStatus (*init)(BthwifCallbacks *callbacks);
    /* Set advertising key. */
    void (*hwSetAdvKey)(const OHOS::bluetooth::RawAddress &addr, const char &irk,
                        const char &hbk, const uint8_t version,
                        const char &feature);
    /* Remove advertising key. */
    void (*hwRemoveAdvKey)(const OHOS::bluetooth::RawAddress &addr);
    /* Get advertising IRK. */
    void (*hwGetAdvIrk)(const OHOS::bluetooth::RawAddress &addr,
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
    void (*hwSetConnReasonFlag)(const OHOS::bluetooth::RawAddress &addr, int connType,
                                const uint8_t *data, size_t len);
    /* Start an Hdap (L2HC) audio session. */
    void (*hdapStartSession)(uint32_t sampleRate, uint8_t bitWidth,
                             uint8_t channel);
    /* Set Hdap active device. */
    void (*setHdapActive)(const OHOS::bluetooth::RawAddress &addr);
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
    void (*hwBtsndHcicDisconnect)(const OHOS::bluetooth::RawAddress &addr);
    /* Check whether bonding or SDP in progress. */
    bool (*isBondingOrSdp)(void);
    /* Check whether incoming connection. */
    bool (*isIncomingConnection)(const OHOS::bluetooth::RawAddress &addr);
    /* Get device info into property. */
    bool (*hwGetDeviceInfo)(const OHOS::bluetooth::RawAddress &addr,
                            BtProperty &prop);
    /* Check whether paired info needs sync. */
    bool (*needSyncPairedInfo)(std::vector<std::string> &pairedAddr);
    /* Fast LE connect. */
    void (*leConnectFastest)(const OHOS::bluetooth::RawAddress &addr);
    /* Set LE connection scan to fast mode. */
    void (*hwSetLeConnectionScanToFast)(bool enable);
    /* Register connection observer. */
    void (*registerConnection)(const OHOS::bluetooth::RawAddress &addr,
                               const OHOS::bluetooth::Uuid &uuid);
    /* Unregister connection observer. */
    void (*unRegisterConnection)(const OHOS::bluetooth::RawAddress &addr,
                                 const OHOS::bluetooth::Uuid &uuid);
    /* Update CoC connection parameters. */
    void (*updateCocConnectionParams)(const OHOS::bluetooth::RawAddress &addr,
                                      const uint16_t *params, int len);
    /* Send device selection. */
    void (*sendDeviceSelection)(const OHOS::bluetooth::RawAddress &device, int useA2dp,
                                int useHfp, int userSelection);
    /* Check whether device supports echo. */
    bool (*isDeviceSupportEcho)(const OHOS::bluetooth::RawAddress &device);
    /* Check whether device is a dual-conn device. */
    bool (*isHwDualConnDevice)(const OHOS::bluetooth::RawAddress &addr);
    /* Change codec. */
    void (*changeCodec)(int preferedScene, int32_t sampleRate, bool fastFlag);
    /* A2DP offload state queries and control. */
    bool (*a2dpOffloadIsRunning)(const OHOS::bluetooth::RawAddress &addr);
    int (*a2dpOffloadGetLatency)(void);
    void (*a2dpOffloadSetSinkStreamStart)(const OHOS::bluetooth::RawAddress &addr);
    void (*a2dpOffloadSetSinkStreamStop)(const OHOS::bluetooth::RawAddress &addr);
    int (*a2dpOffloadGetSinkStreamEncodingPath)(
        const OHOS::bluetooth::RawAddress &addr,
        std::vector<HwBtA2dpSteamSessionInfo> &sessions, bool check);
    /* Spatial audio control. */
    void (*hwSetSpatialAudioModeEnabled)(bool enabled);
    void (*hwSetSpatialAudioHeadTrackingEnabled)(bool enabled);
    void (*hwSetActiveMode)(const OHOS::bluetooth::RawAddress &addr);
    void (*hwSetAdaptiveSwitchStatus)(bool status);
    void (*hwSetAudioSourceType)(int mode);
    void (*hwSetCollAudioEnableState)(bool enabled);
    /* Audio render state and latency. */
    void (*notifyAudioRenderState)(bool isRenderActive);
    void (*sendLowLatencyStatus)(bool status);
    int (*hwGetCurrentSceneType)(void);
    void (*hwUpdateBtAudioLatency)(const OHOS::bluetooth::RawAddress &addr,
                                   int32_t latency);
    /* Voice combine (dual-audio) control. */
    void (*hwSetLocalVoiceCombineFeatureState)(int featureState);
    void (*hwSetLocalVoiceCombineNnState)(int32_t nnState);
    int (*hwGetLocalVoiceCombineFeatureState)(void);
    bool (*hwGetRemoteVoiceCombineAbility)(const OHOS::bluetooth::RawAddress &addr);
    void (*hwSetVoiceCombineScenarioEnabled)(const OHOS::bluetooth::RawAddress &addr,
                                             bool isCall);
    /* Set/delete advertising indication report flag. */
    void (*hwSetAdvIndReportFlag)(int filterIndex, bool flag);
    void (*hwDeletAdvIndReportFlag)(int index);
    /* Send vendor-specific HCI command. */
    void (*hwVendorSpecificCommand)(const SpecificCmdToHisi *cmd);
    /* Apply dynamic configuration from param update events. */
    void (*hwDynamicConfig)(void);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_BT_HWIF_H
