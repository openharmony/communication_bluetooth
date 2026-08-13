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

#ifndef BT_CHR_UE_MANAGER_H
#define BT_CHR_UE_MANAGER_H
#include "bt_chr_base.h"
#include "bt_chr_subevent_manager.h"
#include "raw_address.h"
#include "bt_def.h"
#include "a2dp_parcel_codec.h"
#include "bt_chr_app_behavior_reporter.h"

namespace OHOS {
namespace bluetooth {
enum UeCommonSceneCode {
    UE_SCENE_CODE_SUCCESS = 1,
    UE_SCENE_CODE_FAIL = 2,
};

constexpr int UE_FAIL_VIRCALL_SCO_NOT_ACCEPT = 0; // for chr UE dot VOIP_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_VOICE_REC_STARTED = 1; // for chr UE dot VOIP_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_AUDIO_MODE_NOT_IDLE = 2; // for chr UE dot VOIP_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_AUDIO_ON = 3; // for chr UE dot VOIP_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_NO_ACTIVE_HFP_DEVICE = 4; // for chr UE dot VOIP_CONN_SCO scene code fail-sub scene code.

constexpr int UE_FAIL_VIR_CALL_NOT_STARTED = 1; // for chr UE dot VOIP_DISCONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_DISCONN_SINGLE_SCO = 2; // for chr UE dot VOIP_DISCONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_SCO_DISCONNECTED = 3; // for chr UE dot VOIP_DISCONN_SCO scene code fail-sub scene code.

constexpr int UE_SUCCESS_SCO_NOT_DISCONN = 2; // for chr UE dot CS_CONN_SCO scene code success-sub scene code.
constexpr int UE_FAIL_SCO_NOT_ACCEPT = 1; // for chr UE dot CS_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_NULL_STATE_MACHINE = 2; // for chr UE dot CS_CONN_SCO scene code fail-sub scene code.
constexpr int UE_FAIL_HFP_NOT_CONN = 5; // for chr UE dot CS_CONN_SCO scene code fail-sub scene code.

constexpr int UE_FAIL_SCO_DISCONN = 1; // for chr UE dot CS_DISCONN_SCO scene code fail-sub scene code.

constexpr int UE_HFP_PEER_CONN = 0x0100; // for chr UE dot F_HFP_CONN scene code.

enum UeCommonSceneCase {
    UE_COMMON_SCENE_CASE0 = 0,
    UE_COMMON_SCENE_CASE1 = 1,
    UE_COMMON_SCENE_CASE2,
    UE_COMMON_SCENE_CASE3,
    UE_COMMON_SCENE_CASE4,
    UE_COMMON_SCENE_CASE5,
    UE_COMMON_SCENE_CASE6,
    UE_COMMON_SCENE_CASE7,
    UE_COMMON_SCENE_CASE8,
    UE_COMMON_SCENE_CASE9,
    UE_COMMON_SCENE_CASE10,
};

enum UeSetA2dpActiveDevSceneCase {
    UE_FAIL_SET_A2DP_ACTIVE_DEV_NULL = 1,
    UE_FAIL_SET_A2DP_ACTIVE_DEV_NO_CHANGE = 2,
    UE_SUCCESS_SET_A2DP_ACTIVE_DEV_GENERAL = 3,
    UE_FAIL_SET_A2DP_ACTIVE_DEV_SHARE_SNIFF = 4,
    UE_FAIL_SET_A2DP_ACTIVE_DEV_NULL_STATE_MAC,
    UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN,
    UE_FAIL_SET_A2DP_ACTIVE_DEV_NATIVE,
};

enum UeSetHfpActiveDevSceneCase {
    UE_SUCCESS_SET_HFP_ACTIVE_DEV_NULL = 1,
    UE_SUCCESS_SET_HFP_ACTIVE_DEV_NO_CHANGE = 2,
    UE_SUCCESS_SET_HFP_ACTIVE_DEV_GENERAL = 3,
    UE_FAIL_SET_HFP_ACTIVE_DEV_PRE_SCO_NOT_DISCONN = 4,
    UE_FAIL_SET_HFP_ACTIVE_DEV_NULL_NATIVE,
    UE_FAIL_SET_HFP_ACTIVE_DEV_NOT_CONN,
    UE_FAIL_SET_HFP_ACTIVE_DEV_NATIVE,
};

enum UeBandWearState {
    UE_OUTBAND = 0,
    UE_INBAND = 1,
};

enum UeSetPhoneModeSceneCase {
};

enum class ChrUeHdapOper {
    STOP_RECORD = 0,
    START_RECORD,
    SUSPEND_RECORD,
};

enum class UeCloudPairSceneCode {
    ADD_DOWN_CLOUD_DEV = 1,
    UPDATE_DOWN_CLOUD_DEV,
    CONNECT_DOWN_CLOUD_DEV,
};

enum class UeAddCloudPairSubSceneCode {
    ADD_DOWN_CLOUD_DEV_CMP = 0,
    DOWN_CLOUD_DEV_NAME_EMPTY,
    DOWN_CLOUD_DEV_MAC_EMPTY,
    DOWN_CLOUD_DEV_UUID_EMPTY,
    DOWN_CLOUD_DEV_TOKEN_INVALID,
    DOWN_CLOUD_DEV_IRK_EMPTY,
};

enum class UeUpdateCloudPairSubSceneCode {
    UPDATE_DEV_NAME = 1,
    UPDATE_DEV_UUID,
    UPDATE_DEV_TOKEN,
    UPDATE_DEV_IRK,
    RESET_DEV_CLOUD_PAIR_STATE,
    DELETE_CLOUD_DEV,
};

enum class UeConnCloudPairSubSceneCode {
    PAIR_SUCCESS = 0,
    BEGIN_CLOUD_PAIR,
    ACL_CONN_TIMEOUT,
    BONDING_FAIL,
    CREATE_BOND_FAIL,
};

#ifdef CONTEXTHUB_BLE_V3
struct SensorhubCollaborationUe {
    uint32_t sensorhubResetCnt;

    uint32_t scanCollCnt;
    uint32_t scanCollFailedCnt;
    uint32_t scanCollStartCnt;
    uint32_t scanCollAvgMs;
    uint32_t scanCollMaxMs;
    std::string shScanParam;

    uint32_t connCollCnt;
    uint32_t connCollFailedCnt;
    uint32_t connCollAvgMs;
    uint32_t connCollMaxMs;
};
#endif

class BtChrUeManager {
public:
    BtChrUeManager();
    static std::shared_ptr<BtChrUeManager> GetInstance();
    void WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const int subSceneCode);
    void WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const int subSceneCode, const std::string& pkgName);
    void WriteCommonUe(const std::string& eventName, const std::string& deviceAddr, const int sceneCode,
        const std::string& pkgName);
    void WriteCommonUe(const std::string& eventName, const int sceneCode, const std::string& pkgName);
    void WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const std::string& pkgName);
    void WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const std::string& uuid, const std::string& pkgName);
    void WriteCommonUe(const std::string& eventName, const RawAddress &device, const BtChrDeviceInfo &deviceInfo,
        const int sceneCode, const std::string& pkgName);
    void WriteSetActiveDevUe(const std::string& eventName, const RawAddress &preDevice, const RawAddress &curDevice,
        const int sceneCode, const std::string& pkgName);
    void WriteStateChangeUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const int subSceneCode, const int curVal);
    void WriteStateChangeUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
        const std::string pkgName, const int curVal);
    void WriteCommandUe(const std::string& eventName, const RawAddress &device, const int command,
        const int sceneCode);
    void WriteSupportUe(const std::string& eventName, const RawAddress &device, const int support);
    void WriteVolChangeUe(const int sceneCode, const int volume, const std::string& pkgName);
    void WriteSetLocalNameUe(const std::string& localName, const std::string& pkgName);
    void WriteA2dpPlayStateUe(const RawAddress &device, const bool isPlaying, const uint8_t codecType,
        const std::string& pkgName);
    void WriteBtSwitchChangeUe(const BTTransport transport, const int sceneCode, const int subSceneCode,
        const std::string& pkgName, const int bleAppSize);
    void WriteCodecConfigUe(const RawAddress &device, const CodecInfo &codecInfo, const std::string& pkgName);
    void ReportBandWearStateUe(const std::string& eventName, const RawAddress &device, const int wearState);
    void WriteCustomTypeChangeUe(
    const RawAddress &device, const std::string &deviceName, const int32_t preCustomType, const int32_t curCustomType);
    void WriteConnecionStateChangeUe(const std::string& eventName, const RawAddress &device,
        const int sceneCode, const int preVal, const int curVal);
    void WriteStackScoStateChangeUe(const std::string &eventName, const RawAddress &device,
        const int sceneCode, const int subSceneCode, const int codec = 0);
    void ReportPublishProfileDisableUe(const std::string &eventName, const RawAddress &device,
        const std::string &receiverName, const std::string &pkgName, const int sceneCode);
    void ReportHdapRecordOperUe(const std::string &eventName, const RawAddress &peerAddr,
        const std::string& packageName, int32_t actionType);
    void WriteSwitchDialogResultUe(const int sceneCode, const int subSceneCode, const std::string& pkgName);
    void HwChrReportFinishCloudPair(const RawAddress& peerAddr, int32_t sceneCode, int32_t subSceneCode);
    void WriteSetMusicMuteUe(
        const std::string &eventName, const int sceneCode, const int streamType, const int deviceType);
#ifdef CONTEXTHUB_BLE_V3
    void WriteSensorhubCollaborationUe(const std::string &eventName, const SensorhubCollaborationUe &shColl);
#endif
    void WriteAppWakeupUe(const std::vector<AppBehaviorDataInternal> &dataQueue);

private:
    static std::shared_ptr<BtChrUeManager> instance_;

    void PutDeviceProperties(std::vector<BtChrEventParam> &params, const RawAddress &device,
        const std::string& eventName);
    void PutPreDeviceProperties(std::vector<BtChrEventParam> &params, const RawAddress &preDevice,
        const std::string& eventName);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_UE_MANAGER_H */
