/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_audio_manager"
#endif

#include "a2dp_service.h"
#include "adapter_manager.h"
#include "avrcp_tg_avsession_media_loader.h"
#include "avrcp_tg_service_manager.h"
#include "hfp_ag_service.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_remote_device.h"
#include "remote_device_properties.h"
#include "bluetooth_audio_manager_info.h"
#include "log.h"
#include "common_util.h"
#include "avrc_defs.h"
#include <unordered_map>
#include <base/bind.h>
#include "bluetooth.h"
#include "hal_util.h"
#include "btif_config.h"
#include "preferences.h"
#include "preferences_errno.h"
#include "preferences_helper.h"
#include "bt_def.h"
#include "adapter_device_config.h"
#include "bluetooth_device.h"
#include "bt_chr_ue_manager.h"
#include "bluetooth_audio_framework_adapter.h"
#include "hitrace_meter.h"
#include "bluetooth_hw_interface.h"
#include "preferences_manager.h"

using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

constexpr const int USE_A2DP_STATUS_RELEASE = 1;
constexpr const int USE_A2DP_STATUS_START = 2;
const std::string DEVICE_AUDIO_MANAGER_PATH = "/data/service/el1/public/bluetooth/bluetooth_audio_manager_config.xml";

BluetoothAudioManager &BluetoothAudioManager::GetInstance()
{
    static BluetoothAudioManager instance;
    return instance;
}

void BluetoothAudioManager::WearStateHiechoProcess(const RawAddress &rawAddr, int leftState, int rightState)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    remoteDevice->SetWearDetectionSupportValue(true);
    HILOGI(" receive audio manager Echo command, leftState = %{public}d, rightState = %{public}d, rawaddr= %{public}s",
        leftState, rightState, GET_ENCRYPT_ADDR(rawAddr));
    WearingStateRecord record;
    bool found = mAudioManagerRecordMap_.Find(rawAddr.GetAddress(), record);
    if ((leftState == WEAR_STATUS_INVALID) && (rightState == WEAR_STATUS_INVALID)) {
        HILOGI(" got all side unknown");
    } else if (leftState == WEAR_STATUS_INVALID) {
        bool leftWearState = (!found ? false : record.leftIn_);
        UpdateDeviceWearState(remoteDevice, leftWearState, rightState == WEAR_STATUS_IN);
    } else if (rightState == WEAR_STATUS_INVALID) {
        bool rightWearState = (!found ? false : record.rightIn_);
        UpdateDeviceWearState(remoteDevice, leftState == WEAR_STATUS_IN, rightWearState);
    } else {
        UpdateDeviceWearState(remoteDevice, leftState == WEAR_STATUS_IN, rightState == WEAR_STATUS_IN);
    }
}

void BluetoothAudioManager::ChangeActiveForNotWearing(const std::string &addr, bool isLeftIn, bool isRightIn)
{
    RawAddress rawAddr = RawAddress(addr);
    if (isLeftIn || isRightIn) {
        HILOGI("notify wear");
        NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_WEAR);
        return;
    }

    HILOGI("notify media headSet not wear");
    NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_UNWEAR);
}

void BluetoothAudioManager::ChangeActiveDeviceIfNeeded(const std::string &addr, bool leftIn, bool rightIn,
    bool &isActiveDevWill)
{
    WearingStateRecord record;
    bool found = mAudioManagerRecordMap_.Find(addr, record);
    if (!found) {
        HILOGE(" unable to find record");
        return;
    }
    RawAddress rawAddr = RawAddress(addr);
    if ((record.leftIn_ || record.rightIn_) && (!leftIn && !rightIn)) {
        record.SetNewState(leftIn, rightIn);
        mAudioManagerRecordMap_.EnsureInsert(addr, record);
        HILOGI("notify wearstate, from wear to not wear");
        NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_UNWEAR);
    } else if (!record.leftIn_ && !record.rightIn_ && (leftIn || rightIn)) {
        record.SetNewState(leftIn, rightIn);
        mAudioManagerRecordMap_.EnsureInsert(addr, record);
        HILOGI("notify wearstate, from not wear to wear");
        NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_WEAR);
        isActiveDevWill = true;
    } else {
        record.SetNewState(leftIn, rightIn);
        mAudioManagerRecordMap_.EnsureInsert(addr, record);
    }
}

void BluetoothAudioManager::UnWearActionNotifyHdapDisConnected(const RawAddress &addr,
    const UpdateOutputStackAction &action)
{
    if (ACTION_UNWEAR == action && !IsDeviceWearing(addr)) {
        HILOGI("unwear action, hdap notify disConnected");
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        if (a2dpService != nullptr) {
            A2dpSrcCodecInfo codecInfo{};
            a2dpService->NotifyCaptureConnStateChanged(addr, static_cast<int>(BTHdapConnectState::DISCONNECTED),
                codecInfo);
        }
    }
}

void BluetoothAudioManager::NotifyWearStatus(RawAddress &addr, UpdateOutputStackAction action)
{
    HfpAgService *hfpAgService = HfpAgService::GetService();
    if (hfpAgService != nullptr) {
        int connectionState = hfpAgService->GetDeviceState(addr);
        if (connectionState == static_cast<int>(BTConnectState::CONNECTED)) {
            hfpAgService->NotifyHfpStackChanged(static_cast<int>(action), addr);
        } else {
            HILOGW("hfp profile connected: %{public}d, not notify wear status.", connectionState);
        }
    }

    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        int connectionState = a2dpService->GetDeviceState(addr);
        if (connectionState == static_cast<int>(BTConnectState::CONNECTED)) {
            a2dpService->NotifyMediaStackChanged(static_cast<int>(action), addr);
            UnWearActionNotifyHdapDisConnected(addr, action);
        } else {
            HILOGW("a2dp profile connected: %{public}d, not notify wear status.", connectionState);
        }
    }
}

int BluetoothAudioManager::GetPauseReasonNotPlaying(const std::string &addr, int action, WearingStateRecord &record)
{
    BluetoothAudioManagerInfo wdi;
    int64_t currentTime = GetTimeStamp();

    PauseRecord pauseRecord;
    bool isNoNeedReason = !mPauseRecordMap.Find(addr, pauseRecord) ||
        (currentTime > pauseRecord.pauseTime_ &&
        currentTime - pauseRecord.pauseTime_ > DOUBLE_REMOVE_CHECK_TIME &&
        BluetoothAudioFrameworkAdapter::IsMusicActive(StreamType::STREAM_MUSIC));
    if (isNoNeedReason) {
        return wdi.PAUSE_NO_NEED;
    }

    if ((pauseRecord.pauseReason_ == wdi.PAUSE_REASON_DOUBLE_REMOVE_LEFT) &&
        (!record.leftIn_ && record.rightIn_) && (action == wdi.ACTION_REMOVE_RIGHT)) {
        return wdi.PAUSE_REASON_DOUBLE_REMOVE_DOUBLE;
    } else if ((pauseRecord.pauseReason_ == wdi.PAUSE_REASON_DOUBLE_REMOVE_RIGHT) &&
        (record.leftIn_ && !record.rightIn_) && (action == wdi.ACTION_REMOVE_LEFT)) {
        return wdi.PAUSE_REASON_DOUBLE_REMOVE_DOUBLE;
    } else {
        return wdi.PAUSE_NO_NEED;
    }
}

int BluetoothAudioManager::GetPauseReason(const std::string &addr, int action, WearingStateRecord &record,
    int currentPlayState)
{
    BluetoothAudioManagerInfo wdi;
    if (currentPlayState != PlayState::PLAYING) {
        return GetPauseReasonNotPlaying(addr, action, record);
    }

    if (record.leftIn_ && !record.rightIn_ && (action == wdi.ACTION_REMOVE_LEFT)) {
        return wdi.PAUSE_REASON_LEFT_REMOVE_LEFT; // 10->00
    } else if (!record.leftIn_ && record.rightIn_ && (action == wdi.ACTION_REMOVE_RIGHT)) {
        return wdi.PAUSE_REASON_RIGHT_REMOVE_RIGHT; // 01->00
    } else if (record.leftIn_ && record.rightIn_) {
        if (action == wdi.ACTION_REMOVE_LEFT) {
            return wdi.PAUSE_REASON_DOUBLE_REMOVE_LEFT; // 11->01
        } else if (action == wdi.ACTION_REMOVE_RIGHT) {
            return wdi.PAUSE_REASON_DOUBLE_REMOVE_RIGHT; // 11->10
        } else if (action == wdi.ACTION_REMOVE_DOUBLE) {
            return wdi.PAUSE_REASON_DOUBLE_REMOVE_DOUBLE; // 11->00
        } else {
            HILOGI("GetPauseReason pause no need");
            return wdi.PAUSE_NO_NEED;
        }
    } else {
        return wdi.PAUSE_NO_NEED;
    }
}

void BluetoothAudioManager::PauseMediaAndRecordIfNeeded(const std::string &addr, int pauseReason, long currentTime)
{
    HILOGI(" the pause reason is : %{public}d", pauseReason);
    BluetoothAudioManagerInfo wdi;
    switch (pauseReason) {
        case wdi.PAUSE_NO_NEED: {
            HILOGE(" pause no need");
            mPauseRecordMap.Erase(addr);
            break;
        }
        default: {
            PauseRecord pauseRecord;
            if (!mPauseRecordMap.Find(addr, pauseRecord)) {
                AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->SendKeyEventByWearDetection(AVRC_ID_PAUSE,
                    ::bluetooth::avrcp::KeyState::PUSHED);
                AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->SendKeyEventByWearDetection(AVRC_ID_PAUSE,
                    ::bluetooth::avrcp::KeyState::RELEASED);
                BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, RawAddress(addr),
                    AVRC_ID_PAUSE, UE_COMMON_SCENE_CASE2);
                HILOGI(" send pause");
            } else {
                HILOGI(" drop duplicate pause");
            }
            mPauseRecordMap.EnsureInsert(addr, PauseRecord(pauseReason, currentTime));
            break;
        }
    }
}


void BluetoothAudioManager::ResumePlayIfNeeded(const std::string &addr, int action, long currentTime)
{
    PauseRecord pauseRecord;
    if (!mPauseRecordMap.Find(addr, pauseRecord)) {
        HILOGW("do nothing for no pause record");
        return;
    }

    bool isNoNeedResume = BluetoothAudioFrameworkAdapter::IsMusicActive(StreamType::STREAM_MUSIC) &&
                          currentTime > pauseRecord.pauseTime_ &&
                          currentTime - pauseRecord.pauseTime_ > DOUBLE_REMOVE_CHECK_TIME;
    if (isNoNeedResume) {
        HILOGW("do nothing, current audio music active");
        mPauseRecordMap.Erase(addr);
        return;
    }

    switch (pauseRecord.pauseReason_) {
        case BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_LEFT:
        case BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_RIGHT:
            if (action == BluetoothAudioManagerInfo::ACTION_ADD_DOUBLE) {
                HILOGW("ACTION_ADD_DOUBLE wrong pause reason");
                mPauseRecordMap.Erase(addr);
                break;
            }
            [[fallthrough]];
        case BluetoothAudioManagerInfo::PAUSE_REASON_DOUBLE_REMOVE_DOUBLE:
        case BluetoothAudioManagerInfo::PAUSE_REASON_RIGHT_REMOVE_RIGHT:
        case BluetoothAudioManagerInfo::PAUSE_REASON_LEFT_REMOVE_LEFT: {
            isNoNeedResume =  currentTime > pauseRecord.pauseTime_ &&
                              currentTime - pauseRecord.pauseTime_ <= MAX_RESUME_PLAY_TIME_SIMPLE;
            if (isNoNeedResume) {
                AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->SendKeyEventByWearDetection(AVRC_ID_PLAY,
                    ::bluetooth::avrcp::KeyState::PUSHED);
                AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->SendKeyEventByWearDetection(AVRC_ID_PLAY,
                    ::bluetooth::avrcp::KeyState::RELEASED);
                BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD,
                    RawAddress(addr), AVRC_ID_PLAY, UE_COMMON_SCENE_CASE2);
                lastSendPlayTime_ = GetTimeStamp();
                HILOGI("send play");
            } else {
                HILOGI("not resume for time too long");
            }
            mPauseRecordMap.Erase(addr);
            break;
        }
        default:
            mPauseRecordMap.Erase(addr);
            break;
    }
}

void BluetoothAudioManager::GetAudioManagerPlayState(PlayStatus status)
{
    HILOGI("the callbak play state is: %{public}d", status.state);
    playStatus_ = status;
}

void BluetoothAudioManager::ChangePlayStateIfNeeded(const std::string &addr, int action,
    WearingStateRecord &record, bool isActiveDevNow, bool isActiveDevWill)
{
    A2dpService *A2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (A2dpService == nullptr) {
        HILOGE("ChangePlayStateIfNeeded error, null A2dpService");
        return;
    }
    BluetoothAudioManagerInfo wdi;
    HILOGI("ACTION = (%{public}s)", wdi.GetActionString(action).c_str());
    if (action == wdi.ACTION_NONE) {
        return;
    }

    int32_t currentPlayState = playStatus_.state;

    HILOGI("the current play state is: %{public}d", currentPlayState);
    int64_t currentTime = GetTimeStamp();

    switch (action) {
        case wdi.ACTION_REMOVE_DOUBLE:
        case wdi.ACTION_REMOVE_LEFT:
        case wdi.ACTION_REMOVE_RIGHT: {
            if (!isActiveDevNow) {
                HILOGI("device not active or not bluetooth output");
                return;
            }
            bool isNeedChangeState = (currentPlayState != PlayState::PLAYING) && (lastSendPlayTime_ > 0) &&
                                  currentTime > lastSendPlayTime_ &&
                                  (currentTime - lastSendPlayTime_ <= DOUBLE_REMOVE_CHECK_TIME);
            if (isNeedChangeState) {
                HILOGI("remove after send play less than 2s, fix to playing state");
                currentPlayState = PlayState::PLAYING;
            }
            int pauseReason = GetPauseReason(addr, action, record, currentPlayState);
            PauseMediaAndRecordIfNeeded(addr, pauseReason, currentTime);
            }
            break;
        case wdi.ACTION_ADD_LEFT:
        case wdi.ACTION_ADD_RIGHT:
        case wdi.ACTION_ADD_DOUBLE:
            if (!isActiveDevWill && !isActiveDevNow) {
                HILOGW("device not active, not send play");
                mPauseRecordMap.Erase(addr);
                return;
            }
            ResumePlayIfNeeded(addr, action, currentTime);
            break;
        default:
            break;
    }
}

bool BluetoothAudioManager::ShouldHandleAsWearAction(const std::string &addr, bool isLeftIn, bool isRightIn)
{
    RawAddress rawAddr = RawAddress(addr);
    if (!isLeftIn && !isRightIn) {
        HILOGI("all unwear, no need notify");
        return false;
    }
    A2dpService *A2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    HfpAgService *Headsetservice = HfpAgService::GetService();
    if ((A2dpService != nullptr && A2dpService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED))
        || (Headsetservice != nullptr && Headsetservice->GetDeviceState(rawAddr) ==
        static_cast<int>(BTConnectState::CONNECTED))) {
        WearingStateRecord record(false, false);
        mAudioManagerRecordMap_.EnsureInsert(addr, record);
        bool isActiveDevWill = false;
        ChangeActiveDeviceIfNeeded(addr, isLeftIn, isRightIn, isActiveDevWill);
        return true;
    }
    HILOGI("profile not connect, no need to notify");
    return false;
}

bool BluetoothAudioManager::IsAudioOutputToBluetooth()
{
    BtDeviceType deviceType = BluetoothAudioFrameworkAdapter::BtGetActiveOutputDevice();
    if (deviceType == BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO ||
        deviceType == BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) {
        return true;
    } else {
        return false;
    }
}

void BluetoothAudioManager::UpdateDeviceWearStateIfSupport(const std::string &addr, bool isLeftIn, bool isRightIn)
{
    WearingStateRecord record;
    bool found = mAudioManagerRecordMap_.Find(addr, record);
    if (!found) {
        HILOGI("add new WearingStateRecord");
        if (!ShouldHandleAsWearAction(addr, isLeftIn, isRightIn)) {
            record.SetNewState(isLeftIn, isRightIn);
            mAudioManagerRecordMap_.EnsureInsert(addr, record);
        }
    } else {
        if ((record.leftIn_ == isLeftIn) && (record.rightIn_ == isRightIn)) {
            HILOGI("WearState no change");
            return;
        }
        WearingStateRecord recordbak(record.leftIn_, record.rightIn_);
        HILOGI("device: %{public}s:the old left wear is: %{public}d. old right wear: %{public}d",
            GET_ENCRYPT_STR_ADDR(addr), recordbak.leftIn_, recordbak.rightIn_);
        bool isActiveDevNow = true;
        bool isMusicOutputToBluetooth = false;
        A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
        if (service != nullptr) {
            RawAddress originAddr = service->GetActiveSinkDevice();
            isMusicOutputToBluetooth = IsAudioOutputToBluetooth();
            if (addr == originAddr.GetAddress() && isMusicOutputToBluetooth) {
                isActiveDevNow = true;
            } else {
                isActiveDevNow = false;
            }
        }
        bool isActiveDevWill = false;
        BluetoothAudioManagerInfo wdi;
        AvrcpMediaInterfaceImpl::PlayStatusCallback cb = base::Bind(&BluetoothAudioManager::GetAudioManagerPlayState);
        AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->GetPlayStatus(cb);
        ChangeActiveDeviceIfNeeded(addr, isLeftIn, isRightIn, isActiveDevWill);
        ChangePlayStateIfNeeded(addr, wdi.GetWearAction(recordbak, isLeftIn, isRightIn), recordbak,
            isActiveDevNow, isActiveDevWill);
    }
}

static std::shared_ptr<NativePreferences::Preferences> GetAbsPreferences(std::string path)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t errCode = NativePreferences::E_ERROR;
    std::shared_ptr<NativePreferences::Preferences> preferences =
        NativePreferences::PreferencesHelper::GetPreferences(path, errCode);
    if (preferences == nullptr || errCode != NativePreferences::E_OK) {
        HILOGE("get preferences error, code=%{public}d", errCode);
        return nullptr;
    }
    return preferences;
}

int BluetoothAudioManager::GetAudioManagerSupport(std::string addr)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto preferences = GetAbsPreferences(DEVICE_AUDIO_MANAGER_PATH);
    if (!preferences) {
        return SUPPORT_SYSTEM_ERROR;
    }

    return preferences->GetInt(addr, SUPPORT_UNKNOWN);
}

void BluetoothAudioManager::ChangeActiveForSupportChange(std::string addr, int support)
{
    RawAddress rawAddr = RawAddress(addr);
    if (GetAudioManagerSupport(addr) == SUPPORT_ON && support == SUPPORT_OFF) {
        HILOGI("disable wear detection");
        NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_DISABLE_WEAR_DETECTION);
    } else if (GetAudioManagerSupport(addr) == SUPPORT_OFF && support == SUPPORT_ON) {
        HILOGI("enable wear detection");
        int action = IsDeviceWearing(rawAddr) ?
            UpdateOutputStackAction::ACTION_WEAR : UpdateOutputStackAction::ACTION_UNWEAR;
        NotifyWearStatus(rawAddr, UpdateOutputStackAction(action));
        NotifyWearStatus(rawAddr, UpdateOutputStackAction::ACTION_ENABLE_WEAR_DETECTION);
    } else {
        HILOGI("unsupported change");
    }
}

int32_t BluetoothAudioManager::SetAudioManagerSupport(std::string addr, bool enable)
{
    HITRACE_METER(BT_TRACE_TAG);
    int support = enable ? SUPPORT_ON : SUPPORT_OFF;
    ChangeActiveForSupportChange(addr, support);
    auto preferences = GetAbsPreferences(DEVICE_AUDIO_MANAGER_PATH);
    if (!preferences) {
        return SET_WEARDETECTION_FAIL;
    }
    int32_t errCode = preferences->PutInt(addr, support);
    if (errCode != NativePreferences::E_OK) {
        HILOGE("put int error, code=%{public}d", errCode);
        return SET_WEARDETECTION_FAIL;
    }
    preferences->Flush();
    BtChrUeManager::GetInstance()->WriteSupportUe(CHR_UE_TWS_WEAR_STATE_SUPPORT, RawAddress(addr), support);
    return SET_WEARDETECTION_SUCCESS;
}

void BluetoothAudioManager::ClearWearDetectionSupport(std::string addr)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto preferences = GetAbsPreferences(DEVICE_AUDIO_MANAGER_PATH);
    if (!preferences) {
        HILOGE(" got null SharedPreferences");
        return;
    }
    preferences->Delete(addr);
    preferences->Flush();
}

int32_t BluetoothAudioManager::GetWearState(std::string addr, int32_t &ability)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto preferences = GetAbsPreferences(DEVICE_AUDIO_MANAGER_PATH);
    if (!preferences) {
        return ENABLE_WEAR_DETECTION_UNKOWN;
    }
    ability = preferences->GetInt(addr, SUPPORT_UNKNOWN);
    return NO_ERROR;
}

bool BluetoothAudioManager::IsDeviceWearing(const RawAddress &rawAddr)
{
    WearingStateRecord record;
    bool found = mAudioManagerRecordMap_.Find(rawAddr.GetAddress(), record);
    if (!found) {
        HILOGI("no record for device");
        return false;
    }

    bool isWear = record.leftIn_ || record.rightIn_;
    HILOGI("device %{public}s now is %{public}s ", GET_ENCRYPT_ADDR(rawAddr), isWear ? "wear" : "unwear");
    return isWear;
}

void BluetoothAudioManager::RemoveDisconnectDevice(const std::string &addr)
{
    mAudioManagerRecordMap_.Erase(addr);
    mPauseRecordMap.Erase(addr);
    profileStatusMap_.Erase(addr);
}

void BluetoothAudioManager::UpdateDeviceWearState(std::shared_ptr<BluetoothDevice> device, bool leftIn, bool rightIn)
{
#ifdef BLUETOOTH_UNSUPPORT_WEAR_DETECTION
    HILOGI("unsupport wear detection");
    return;
#endif

    if (device == nullptr) {
        HILOGE("got null device");
        return;
    }
    std::string addr = device->GetAddress();
    HILOG_COMM_INFO("UpdateDeviceWearState: %{public}s:new left wear is: %{public}d, new right wear is: %{public}d",
        GET_ENCRYPT_STR_ADDR(addr), leftIn, rightIn);
    int supportValue = GetAudioManagerSupport(addr);
    if (supportValue == SUPPORT_UNKNOWN) {
        SetAudioManagerSupport(addr, SUPPORT_ON);
        WearingStateRecord record = WearingStateRecord(leftIn, rightIn);
        mAudioManagerRecordMap_.EnsureInsert(device->GetAddress(), record);
        ChangeActiveForNotWearing(addr, leftIn, rightIn);
    } else if (supportValue == SUPPORT_OFF) {
        WearingStateRecord record;
        record.SetNewState(leftIn, rightIn);
        mAudioManagerRecordMap_.EnsureInsert(device->GetAddress(), record);
    } else { // SUPPORT_ON
        UpdateDeviceWearStateIfSupport(addr, leftIn, rightIn);
    }
}

int BluetoothAudioManager::SendDeviceSelection(const RawAddress &address, int useA2dp, int useHfp, int userSelection)
{
    HILOGI("address: %{public}s, useA2dp: %{public}d, useHfp: %{public}d, userSelection: %{public}d",
        GET_ENCRYPT_ADDR(address), useA2dp, useHfp, userSelection);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return BT_ERR_INTERNAL_ERROR;
    }

    StopOffloadWhenA2dpReleased(address, useA2dp);
    ProcessAudioRenderListenerWhenUserSelect(useA2dp);
    BLUEDROID::RawAddress device = ServiceUtil::AddrToBluedroid(address);
    const bthwif_interface_t *bthwif =
        reinterpret_cast<const bthwif_interface_t*>(btInterface->get_profile_interface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("Failed to get bthwif interface handle.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!bthwif->isDeviceSupportEcho(device)) {
        HILOGD("The device does not support echo.");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothHwInterface *bluetoothHwInterface = BluetoothHwInterface::GetInstance();
    CHECK_AND_RETURN_LOG_RET(bluetoothHwInterface != nullptr, BT_ERR_INTERNAL_ERROR, "interface nullptr");
    bthwif->sendDeviceSelection(device, useA2dp, useHfp, userSelection);
    return BT_NO_ERROR;
}

int BluetoothAudioManager::IsWearDetectionSupported(const RawAddress &address, bool &isSupported)
{
#ifdef BLUETOOTH_UNSUPPORT_WEAR_DETECTION
    HILOGI("unsupport wear detection");
    isSupported = false;
    return BT_NO_ERROR;
#endif
    RawAddress device = RawAddress(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG_RET(remoteDevice != nullptr, BT_ERR_INTERNAL_ERROR, "device not exist.");
    isSupported = remoteDevice->GetWearDetectionSupportValue();
    HILOGI("device: %{public}s isSupported: %{public}d",  GET_ENCRYPT_ADDR(address), isSupported);
    return BT_NO_ERROR;
}

void BluetoothAudioManager::DealWithNewPlayingStatus()
{
    int64_t currentTime = GetTimeStamp();
    std::vector<std::string> addrs;
    mPauseRecordMap.Iterate([&addrs, &currentTime](const std::string& addr, const PauseRecord& pauseRecord) {
        bool isNeedErase = currentTime > pauseRecord.pauseTime_ &&
                           currentTime - pauseRecord.pauseTime_ > DOUBLE_REMOVE_CHECK_TIME;
        if (isNeedErase) {
            addrs.push_back(addr);
        }
    });
    for (auto &it : addrs) {
        mPauseRecordMap.Erase(it);
    }
}

void BluetoothAudioManager::SetProfileStatus(std::string addr, uint8_t a2dpState, uint8_t hfpState)
{
    profileStatusMap_.EnsureInsert(addr, ProfileStatus(a2dpState, hfpState));
}

bool BluetoothAudioManager::GetProfileStatus(std::string addr, uint8_t &a2dpState, uint8_t &hfpState)
{
    ProfileStatus profileStatus(0, 0);
    if (profileStatusMap_.Find(addr, profileStatus)) {
        a2dpState = profileStatus.a2dpState_;
        hfpState = profileStatus.hfpState_;
        return true;
    }
    return false;
}

void BluetoothAudioManager::SaveHiechoAvLatency(const RawAddress &rawAddr, int32_t avLatency)
{
    avSyncManager_.SaveHiechoAvLatency(rawAddr, avLatency);
}

void BluetoothAudioManager::ReadLatencyConfig()
{
    avSyncManager_.ReadLatencyConfig();
}

void BluetoothAudioManager::UpdateBtAudioLatency(const RawAddress &rawAddr)
{
    avSyncManager_.UpdateBtAudioLatency(rawAddr);
}

void BluetoothAudioManager::ClearLatencyInfoByAddr(const RawAddress &rawAddr)
{
    avSyncManager_.ClearLatencyInfoByAddr(rawAddr);
}

void BluetoothAudioManager::ClearLatencyInfo()
{
    avSyncManager_.ClearLatencyInfo();
}

void BluetoothAudioManager::StopOffloadWhenA2dpReleased(const RawAddress &address, int useA2dp)
{
    if (useA2dp != USE_A2DP_STATUS_RELEASE) {
        HILOGI("useA2dp status not satisfied, useA2dp = %{public}d", useA2dp);
        return;
    }
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "interface nullptr");
    a2dpService->ForceStopOffloadPlaying(address);
}

void BluetoothAudioManager::StopAudioRenderListenerWhenA2dpReleased()
{
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioFrameworkAdapterListener();
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "a2dpService nullptr");
    a2dpService->OnRenderStateChange(false);
}

void BluetoothAudioManager::ProcessAudioRenderListenerWhenUserSelect(int useA2dp)
{
    if (useA2dp == USE_A2DP_STATUS_RELEASE) {
        StopAudioRenderListenerWhenA2dpReleased();
    } else if (useA2dp == USE_A2DP_STATUS_START) {
        BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioFrameworkAdapterListener();
    }
}

void BluetoothAudioManager::SetSharedPreVirtualAutoConnSwitch(const std::string &address, bool value)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("virtual connect value:%{public}d", value);
    PreferencesManager::Save(address, value, PreferencesManagerType::VIRTUAL_AUTO_CONN_SWITCH);
}

bool BluetoothAudioManager::GetSharedPreVirtualAutoConnSwitchValue(const std::string &address)
{
    HITRACE_METER(BT_TRACE_TAG);
    bool value = false;
    value = PreferencesManager::Get(address, value, PreferencesManagerType::VIRTUAL_AUTO_CONN_SWITCH);
    return value;
}
}
}
