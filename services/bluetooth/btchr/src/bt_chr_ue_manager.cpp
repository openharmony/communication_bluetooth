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
#ifndef LOG_TAG
#define LOG_TAG "bt_chr_ue_manager"
#endif

#include "bt_chr_business_event.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_dft_statictics.h"
#include "bt_chr_base.h"
#include "bt_chr_util.h"
#include "bluetooth_log.h"
#include "bt_chr_pkgname_manager.h"

namespace OHOS {
namespace bluetooth {
std::shared_ptr<BtChrUeManager> BtChrUeManager::instance_ = std::make_shared<BtChrUeManager>();

BtChrUeManager::BtChrUeManager()
{}

std::shared_ptr<BtChrUeManager> BtChrUeManager::GetInstance()
{
    return BtChrUeManager::instance_;
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
    const int subSceneCode)
{
    if (eventName == CHR_UE_BT_UI_SWITCH_CHANGE && sceneCode == UE_COMMON_SCENE_CASE0) {
        if (subSceneCode == UE_COMMON_SCENE_CASE2) {
            BtChrBusinessEvent::GetInstance().SetAbsolutVolumeParams(UE_COMMON_SCENE_CASE1, 0);
        }
        BtChrBusinessEvent::GetInstance().SetAbsolutVolumeParams(UE_COMMON_SCENE_CASE1, subSceneCode);
    }
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
    const int subSceneCode, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const std::string& deviceAddr, const int sceneCode,
    const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("DEV_ADDRESS", GetEncryptAddr(deviceAddr)));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const int sceneCode, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
    const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const RawAddress &device,
    const int sceneCode, const std::string& uuid, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    if (uuid.size() > 0) {
        params.push_back(BtChrEventParam("UUID", uuid));
    }
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommonUe(const std::string& eventName, const RawAddress &device,
    const BtChrDeviceInfo &deviceInfo, const int sceneCode, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    if (!device.GetAddress().empty()) {
        params.push_back(BtChrEventParam("DEV_ADDRESS", GET_ENCRYPT_RAWADDR(device)));
        params.push_back(BtChrEventParam("DEV_NAME", deviceInfo.btDeviceName));
        params.push_back(BtChrEventParam("DEV_TYPE", std::to_string(deviceInfo.btDeviceClass)));
    }
    
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteSetActiveDevUe(const std::string& eventName, const RawAddress &preDevice,
    const RawAddress &curDevice, const int sceneCode, const std::string& pkgName)
{
    if (eventName == CHR_UE_SET_A2DP_ACTIVE_DEV) {
        BtChrBusinessEvent::GetInstance().SetAudioDeviceParams(pkgName, sceneCode);
    }
    std::vector<BtChrEventParam> params;
    if (!preDevice.GetAddress().empty()) {
        PutPreDeviceProperties(params, preDevice, eventName);
    }
    if (!curDevice.GetAddress().empty()) {
        PutDeviceProperties(params, curDevice, eventName);
    }
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteStateChangeUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
    const int subSceneCode, const int curVal)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    params.push_back(BtChrEventParam("CUR_VAL", curVal));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteStateChangeUe(const std::string& eventName, const RawAddress &device, const int sceneCode,
    const std::string pkgName, const int curVal)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode)); // rssi
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", UE_COMMON_SCENE_CASE1)); // LE type
    params.push_back(BtChrEventParam("CUR_VAL", curVal)); // for walletservice broadcast event
    params.push_back(BtChrEventParam("DEV_TYPE", pkgName));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteCommandUe(const std::string& eventName, const RawAddress &device, const int command,
    const int sceneCode)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("COMMAND", command));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteSupportUe(const std::string& eventName, const RawAddress &device, const int support)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SUPPORT", support));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteVolChangeUe(const int sceneCode, const int volume, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("CUR_VOL_VALUE", volume));
    BtChrUeEventWrite(CHR_UE_BT_VOL_CHANGE, params);
}

void BtChrUeManager::WriteSetLocalNameUe(const std::string& localName, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("LOCAL_NAME", localName));
    BtChrUeEventWrite(CHR_UE_SET_BT_NAME, params);
}

void BtChrUeManager::WriteA2dpPlayStateUe(const RawAddress &device, const bool isPlaying, const uint8_t codecType,
    const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    if (isPlaying) {
        BtChrBusinessEvent::GetInstance().SetAudioStartTime(GetCurrentTime());
        return;
    }
    PutDeviceProperties(params, device, CHR_UE_AUDIO_STATUS_CHANGE);
    std::string savedPkgName = BtChrBusinessEvent::GetInstance().GetSavedPkgName();
    params.push_back(BtChrEventParam("PKG_NAME", (savedPkgName.size() > 0) ? savedPkgName : pkgName));
    // app type, unknown
    params.push_back(BtChrEventParam("SCENE_CODE", -1));
    params.push_back(BtChrEventParam("CODEC", codecType));
    params.push_back(BtChrEventParam("ACTIVATE_DEVICE_PKG", BtChrBusinessEvent::GetInstance().GetAudioDevicePkg()));
    params.push_back(BtChrEventParam("ACTIVATE_DEVICE_RESULT",
        BtChrBusinessEvent::GetInstance().GetActiveDeviceResult()));
    params.push_back(BtChrEventParam("OFFLOAD_FLAG", BtChrBusinessEvent::GetInstance().GetOffloadFlag()));
    params.push_back(BtChrEventParam("ABSOLUT_VOLUME", BtChrBusinessEvent::GetInstance().IsAbsolutVolume()));
    params.push_back(BtChrEventParam("ABSOLUT_VOLUME_STSTUS",
        BtChrBusinessEvent::GetInstance().GetAbsolutVolumeStatus()));
    params.push_back(BtChrEventParam("VOLUME_TYPE", BtChrBusinessEvent::GetInstance().GetVolumeType()));
    params.push_back(BtChrEventParam("VOLUME_VALUE", BtChrBusinessEvent::GetInstance().GetVolumeValue()));
    params.push_back(BtChrEventParam("AUDIO_START_TIME", BtChrBusinessEvent::GetInstance().GetAudioStartTime()));
    params.push_back(BtChrEventParam("AUDIO_STOP_TIME", GetCurrentTime()));
    params.push_back(BtChrEventParam("BT_CHIP_STATE", GetPowerLevelStatistics()));
    BtChrUeEventWrite(CHR_UE_AUDIO_STATUS_CHANGE, params);
    BtChrDftStatictics::GetInstance()->WriteDurationStatictics(DURATION_STAT_TYPE_A2DP, isPlaying);
}

void BtChrUeManager::WriteBtSwitchChangeUe(const BTTransport transport, const int sceneCode, const int subSceneCode,
    const std::string& pkgName, const int bleAppSize)
{
    if (transport == ADAPTER_BLE) {
        return;
    }
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    params.push_back(BtChrEventParam("BLE_APP_SIZE", bleAppSize));
    BtChrUeEventWrite(CHR_UE_CHANGE_BT_SWITCH, params);
}

void BtChrUeManager::WriteCodecConfigUe(const RawAddress &device, const CodecInfo &codecInfo,
    const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, CHR_UE_SERVICE_CODEC_SETCONFIG);
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("CODEC_TYPE", codecInfo.codecType));
    params.push_back(BtChrEventParam("SAMPLE_RATE", codecInfo.sampleRate));
    params.push_back(BtChrEventParam("BITS_PER_SAMPLE", codecInfo.bitsPerSample));
    params.push_back(BtChrEventParam("CHANNEL_MODE", codecInfo.channelMode));
    params.push_back(BtChrEventParam("SPECIFIC1", codecInfo.codecSpecific1));
    params.push_back(BtChrEventParam("SPECIFIC2", codecInfo.codecSpecific2));
    params.push_back(BtChrEventParam("SPECIFIC3", codecInfo.codecSpecific3));
    params.push_back(BtChrEventParam("SPECIFIC4", codecInfo.codecSpecific4));
    BtChrUeEventWrite(CHR_UE_SERVICE_CODEC_SETCONFIG, params);
}

void BtChrUeManager::WriteCustomTypeChangeUe(
    const RawAddress &device, const std::string &deviceName, const int32_t preCustomType, const int32_t curCustomType)
{
    std::vector<BtChrEventParam> params;
    if (!device.GetAddress().empty()) {
        params.push_back(BtChrEventParam("DEV_ADDRESS", GET_ENCRYPT_RAWADDR(device)));
        params.push_back(BtChrEventParam("DEV_NAME", deviceName));
    }
    params.push_back(BtChrEventParam("PRE_CUSTOM_DEVICE_TYPE", preCustomType));
    params.push_back(BtChrEventParam("CUR_CUSTOM_DEVICE_TYPE", curCustomType));
    BtChrUeEventWrite(CHR_UE_PAIRED_CUSTOM_TYPE_CHANGE, params);
}

void BtChrUeManager::PutDeviceProperties(std::vector<BtChrEventParam> &params, const RawAddress &device,
    const std::string& eventName)
{
    if (eventName == CHR_UE_SOCKET_SERVER_CONN || eventName == CHR_UE_SET_A2DP_SUSPEND) {
        // Do not need device properties
        return;
    }
    if (device.GetAddress().empty()) {
        HILOGE("invalid device, eventName: %{public}s", eventName.c_str());
        return;
    }
    params.push_back(BtChrEventParam("DEV_ADDRESS", GET_ENCRYPT_RAWADDR(device)));
    BtChrDeviceInfo deviceInfo;
    if (!GetDeviceInfoFromMap(device.GetAddress(), deviceInfo)) {
        HILOGD("device %{public}s not exist, eventName: %{public}s", GET_ENCRYPT_RAWADDR(device), eventName.c_str());
        return;
    }
    std::string deviceName = deviceInfo.btDeviceName;
    int deviceClass = deviceInfo.btDeviceClass;
    if (deviceName.empty() || deviceClass == 0) {
        HILOGI("device: %{public}s, eventName: %{public}s, btDeviceClass: %{public}d",
            GET_ENCRYPT_RAWADDR(device), eventName.c_str(), deviceClass);
    }
    params.push_back(BtChrEventParam("DEV_NAME", deviceName));
    params.push_back(BtChrEventParam("DEV_TYPE", std::to_string(deviceClass)));
}

void BtChrUeManager::PutPreDeviceProperties(std::vector<BtChrEventParam> &params, const RawAddress &preDevice,
    const std::string& eventName)
{
    if (preDevice.GetAddress().empty()) {
        HILOGE("invalid preDevice, eventName: %{public}s", eventName.c_str());
        return;
    }
    params.push_back(BtChrEventParam("PRE_ACT_DEV_ADDRESS", GET_ENCRYPT_RAWADDR(preDevice)));
    BtChrDeviceInfo deviceInfo;
    if (!GetDeviceInfoFromMap(preDevice.GetAddress(), deviceInfo)) {
        HILOGE("device %{public}s not exist, eventName: %{public}s", GET_ENCRYPT_RAWADDR(preDevice),
            eventName.c_str());
        return;
    }
    params.push_back(BtChrEventParam("PRE_ACT_DEV_NAME", deviceInfo.btDeviceName));
    params.push_back(BtChrEventParam("PRE_ACT_DEV_TYPE", std::to_string(deviceInfo.btDeviceClass)));
}

void BtChrUeManager::ReportBandWearStateUe(const std::string &eventName, const RawAddress &device, const int wearState)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("WEAR_STATE", wearState));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteConnecionStateChangeUe(const std::string& eventName, const RawAddress &device,
    const int sceneCode, const int preVal, const int curVal)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("PRE_VAL", preVal));
    params.push_back(BtChrEventParam("CUR_VAL", curVal));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::WriteStackScoStateChangeUe(const std::string &eventName, const RawAddress &device,
    const int sceneCode, const int subSceneCode, const int codec)
{
    if (eventName != CHR_UE_STACK_CONN_SCO && eventName != CHR_UE_STACK_DISCONN_SCO) {
        HILOGE("invalid eventName: %{public}s", eventName.c_str());
        return;
    }
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("CODEC", codec));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::ReportPublishProfileDisableUe(const std::string &eventName, const RawAddress &device,
    const std::string &receiverName, const std::string &pkgName, const int sceneCode)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, device, eventName);
    params.push_back(BtChrEventParam("RECEIVER_NAME", receiverName));
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    BtChrUeEventWrite(eventName, params);
}

void BtChrUeManager::ReportHdapRecordOperUe(const std::string &eventName, const RawAddress &peerAddr,
    const std::string& packageName, int32_t actionType)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, peerAddr, eventName);
    params.push_back(BtChrEventParam("PKG_NAME", packageName));
    params.push_back(BtChrEventParam("ACTION_TYPE", actionType));
    params.push_back(BtChrEventParam("ACTION_TIME", GetCurrentTime()));
    params.push_back(BtChrEventParam("RECORD_MODE", 0));
    params.push_back(BtChrEventParam("HDAP_VER", 0));
    BtChrUeEventWrite(CHR_UE_BT_HDAP_OPER_EVENT, params);
}

void BtChrUeManager::WriteSwitchDialogResultUe(const int sceneCode, const int subSceneCode, const std::string& pkgName)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("PKG_NAME", pkgName));
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    BtChrUeEventWrite(CHR_UE_CHANGE_BT_SWITCH, params);
}

void BtChrUeManager::HwChrReportFinishCloudPair(const RawAddress& peerAddr, int32_t sceneCode, int32_t subSceneCode)
{
    std::vector<BtChrEventParam> params;
    PutDeviceProperties(params, peerAddr, CHR_UE_FINISH_CLOUD_PAIR);
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("SUB_SCENE_CODE", subSceneCode));
    BtChrUeEventWrite(CHR_UE_FINISH_CLOUD_PAIR, params);
}

void BtChrUeManager::WriteSetMusicMuteUe(const std::string &eventName, const int sceneCode,
    const int streamType, const int deviceType)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("SCENE_CODE", sceneCode));
    params.push_back(BtChrEventParam("STREAM_TYPE", streamType));
    params.push_back(BtChrEventParam("MUTE_DEVICE", deviceType));
    BtChrUeEventWrite(eventName, params);
}

#ifdef CONTEXTHUB_BLE_V3
void BtChrUeManager::WriteSensorhubCollaborationUe(const std::string &eventName,
    const SensorhubCollaborationUe &shColl)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("SENSORHUB_RESET_COUNT", shColl.sensorhubResetCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_COUNT", shColl.scanCollCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_FAILED_COUNT", shColl.scanCollFailedCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_START_COUNT", shColl.scanCollStartCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_AVERAGE_COST_TIME", shColl.scanCollAvgMs, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_MAX_COST_TIME", shColl.scanCollMaxMs, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("SCAN_COLL_SH_STARTED_PARAM", shColl.shScanParam));
    params.push_back(BtChrEventParam("CONN_COLL_COUNT", shColl.connCollCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("CONN_COLL_FAILED_COUNT", shColl.connCollFailedCnt, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("CONN_COLL_AVERAGE_COST_TIME", shColl.connCollAvgMs, BTCHREVENT_UINT32));
    params.push_back(BtChrEventParam("CONN_COLL_MAX_COST_TIME", shColl.connCollMaxMs, BTCHREVENT_UINT32));
    BtChrUeEventWrite(eventName, params);
}
#endif

void BtChrUeManager::WriteAppWakeupUe(const std::vector<AppBehaviorDataInternal> &dataQueue)
{
    if (dataQueue.empty()) {
        return;
    }
    size_t queueSize = dataQueue.size();
    std::vector<std::string> appNameParams;
    std::vector<int64_t> appUidParams;
    std::vector<int64_t> appPidParams;
    std::vector<int64_t> businessTypeParams;
    std::vector<int64_t> triggerCountParams;
    for (size_t i = 0; i < queueSize; i++) {
        if (dataQueue[i].appName.empty()) {
            appNameParams.push_back("recvApp");
        } else {
            appNameParams.push_back(dataQueue[i].appName);
        }
        appUidParams.push_back(dataQueue[i].appUid);
        appPidParams.push_back(dataQueue[i].appPid);
        businessTypeParams.push_back(dataQueue[i].businessType);
        triggerCountParams.push_back(dataQueue[i].triggerCount);
    }

    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("APP_NAME", appNameParams));
    params.push_back(BtChrEventParam("APP_UID", appUidParams));
    params.push_back(BtChrEventParam("APP_PID", appPidParams));
    params.push_back(BtChrEventParam("BUSINESS_TYPE", businessTypeParams));
    params.push_back(BtChrEventParam("TRIGGER_COUNT", triggerCountParams));
    BtChrUeEventWrite(CHR_UE_BLUETOOTH_APP_WAKEUP, params);
}

}  // namespace bluetooth
}  // namespace OHOS
