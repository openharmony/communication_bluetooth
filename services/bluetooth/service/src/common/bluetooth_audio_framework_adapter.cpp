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
#define LOG_TAG "bt_service_audio_fwk_adapter"
#endif

#include "bluetooth_audio_framework_adapter.h"
#include "ipc_skeleton.h"
#include "interface_profile_avrcp_tg.h"
#include "interface_profile_hfp_ag.h"
#include "interface_profile_hfp_hf.h"
#include "interface_profile_manager.h"
#include "interface_profile_a2dp_src.h"
#include "log.h"
#include <memory>
#include "hfp_ag_defines.h"
#include "hfp_hf_defines.h"
#include "hfp_ag_system_interface.h"
#include "hitrace_meter.h"
#include "bluetooth_hw_interface.h"
#include "a2dp_service.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_pkgname_manager.h"
#include "service_util.h"
#include "log_utils.h"

namespace OHOS {
namespace bluetooth {
const char *const EMPTY_ADDRESS = "00:00:00:00:00:00";
const char *const NULL_ADDRESS = "";
const int32_t RENDERER_MAX_COUNT = 1000;
bool BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::adaptiveSwitchStatus_ =
    false;
BluetoothAudioFrameworkAdapter &BluetoothAudioFrameworkAdapter::GetInstance()
{
    static BluetoothAudioFrameworkAdapter bluetoothAudioframeworkAdapter;
    return bluetoothAudioframeworkAdapter;
}

void BluetoothAudioFrameworkAdapter::RegisterAudioVolumeEvent()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    volumeCallback_ = std::make_shared<BluetoothAudioVolumeListener>();
    const int32_t clientPid = IPCSkeleton::GetCallingPid();
    AudioStandard::AudioVolumeClientManager::GetInstance().RegisterVolumeKeyEventCallback(clientPid, volumeCallback_);
#endif
}

void BluetoothAudioFrameworkAdapter::RegisterAudioFrameworkAdapterListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    RegisterOutputPipeChangeListener();
    RegisterRendererDataTransferListener();
#endif
}

void BluetoothAudioFrameworkAdapter::UnregisterAudioFrameworkAdapterListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    UnregisterOutputPipeChangeListener();
    UnregisterRendererDataTransferListener();
#endif
}

void BluetoothAudioFrameworkAdapter::RegisterOutputPipeChangeListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    if (audioOutputPipeCallback_ == nullptr) {
        audioOutputPipeCallback_ = std::make_shared<BluetoothAudioOutputPipeListener>();
    }
    auto ret = DelayedSingleton<AudioStandard::AudioEngineClientManager>::GetInstance()->
        RegisterOutputPipeChangeCallback(audioOutputPipeCallback_);
    if (ret == SUCCESS) {
        GetCurrentOutputPipeInfos();
    }
    HILOGI("ret:%{public}d", ret);
#endif
}

void BluetoothAudioFrameworkAdapter::UnregisterOutputPipeChangeListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    CHECK_AND_RETURN_LOG(audioOutputPipeCallback_ != nullptr, "audioOutputPipeCallback_ is nullptr");
    auto ret = DelayedSingleton<AudioStandard::AudioEngineClientManager>::GetInstance()->
        UnregisterOutputPipeChangeCallback(audioOutputPipeCallback_);
    HILOGI("ret:%{public}d", ret);
    audioOutputPipeCallback_ = nullptr;
    ClearRouteFlagList();
#endif
}

void BluetoothAudioFrameworkAdapter::GetCurrentOutputPipeInfos()
{
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> outputPipeInfos;
    auto ptr = DelayedSingleton<AudioStandard::AudioEngineClientManager>::GetInstance()->
        GetCurrentOutputPipeChangeInfos(outputPipeInfos);
    for (const auto& pipeInfo : outputPipeInfos) {
        if (pipeInfo == nullptr) {
            HILOGD("Get pipeInfo nullptr!");
            continue;
        }
        AudioStandard::HdiAdapterType adapterType = pipeInfo->GetAdapter();
        if (adapterType != AudioStandard::HDI_ADAPTER_TYPE_A2DP &&
            adapterType != AudioStandard::HDI_ADAPTER_TYPE_PRIMARY) {
            HILOGD("Not bluetooth device!");
            continue;
        }
        uint32_t routeFlag = pipeInfo->GetRouteFlag();
        auto rendererStreams = pipeInfo->GetStreams();
        UpdateRouteFlagList(routeFlag, rendererStreams);
    }
}

void BluetoothAudioFrameworkAdapter::RegisterRendererDataTransferListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    AudioStandard::DataTransferMonitorParam param;
    param.clientUID = ALL_UID;
    param.badDataTransferTypeBitMap = MUTE_MAP;
    param.timeInterval = CHECK_TIME_INTERVAL;
    param.badFramesRatio = BAD_FRAMES_RATIO;
    if (audioDataTransferCallback_ == nullptr) {
        audioDataTransferCallback_ = std::make_shared<BluetoothAudioRendererDataTransferListener>();
    }
    auto ret = AudioStandard::AudioStreamClientManager::GetInstance().RegisterRendererDataTransferCallback(
        param, audioDataTransferCallback_);
    HILOGI("ret:%{public}d", ret);
#endif
}

void BluetoothAudioFrameworkAdapter::UnregisterRendererDataTransferListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    CHECK_AND_RETURN_LOG(audioDataTransferCallback_ != nullptr, "audioDataTransferCallback_ is nullptr");
    auto ret = AudioStandard::AudioStreamClientManager::GetInstance().UnregisterRendererDataTransferCallback(
        audioDataTransferCallback_);
    audioDataTransferCallback_ = nullptr;
    HILOGI("ret:%{public}d", ret);
#endif
}

void BluetoothAudioFrameworkAdapter::RegisterAudioPreferredOutPutDeviceChangeListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    if (audioPreferredOutPutDeviceCallback_ != nullptr) {
        return;
    }
    audioPreferredOutPutDeviceCallback_ = std::make_shared<BluetoothAudioPreferredOutPutDeviceChangeListener>();
    AudioStandard::AudioRendererInfo rendererInfo;
    rendererInfo.contentType = AudioStandard::ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_MUSIC;
    int32_t ret = AudioStandard::AudioRoutingClientManager::GetInstance().SetPreferredOutputDeviceChangeCallback(
        rendererInfo, audioPreferredOutPutDeviceCallback_);
    if (ret != 0) {
        HILOGE("SetPreferredOutputDeviceChangeCallback fail");
    }
#endif
}
 
void BluetoothAudioFrameworkAdapter::UnregisterAudioPreferredOutPutDeviceChangeListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    CHECK_AND_RETURN_LOG(
        audioPreferredOutPutDeviceCallback_ != nullptr, "audioPreferredOutPutDeviceCallback_ is nullptr");
    auto ret = AudioStandard::AudioRoutingClientManager::GetInstance().UnsetPreferredOutputDeviceChangeCallback();
    if (ret != 0) {
        HILOGE("UnsetPreferredOutputDeviceChangeCallback fail");
    }
    audioPreferredOutPutDeviceCallback_ = nullptr;
#endif
}

void BluetoothAudioFrameworkAdapter::RegisterAudioNnStateEventListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
#ifdef BLUETOOTH_VOICE_COMBINE_FEATURE_ENABLE
    HILOGW("RegisterAudioNnStateEventListener");
    audioNnStateEventCallback_ = std::make_shared<BluetoothAudioNnStateChangeListener>();
    AudioStandard::AudioCombineDenoisingManager::GetInstance()->
        RegisterNnStateEventListener(audioNnStateEventCallback_);
#endif
#endif
}

int32_t BluetoothAudioFrameworkAdapter::SetMusicMuteWhenA2dpRelease()
{
    HITRACE_METER(BT_TRACE_TAG);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG_RET(service != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR, "a2dp service nullptr");
    RawAddress device = service->GetActiveSinkDevice();
    if (device.GetAddress() == EMPTY_ADDRESS || device.GetAddress() == NULL_ADDRESS) {
        HILOGI("activeDevice is empty");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
 
    auto &statusMap = BluetoothAudioManager::GetInstance().profileStatusMap_;
    if (statusMap.Size() == 0) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    BluetoothAudioManager::ProfileStatus val = statusMap.ReadVal(device.GetAddress());
    HILOGI("macAddr=%{public}s,a2dpState=%{public}d, hfpState=%{public}d",
        GET_ENCRYPT_ADDR(device), val.a2dpState_, val.hfpState_);

    if (val.a2dpState_ != static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE)) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
#ifdef AUDIO_FRAMEWORK
    if (AudioStandard::AudioVolumeClientManager::GetInstance().
        GetVolume(AudioStandard::AudioVolumeType::STREAM_MUSIC) == 0) {
        HILOGI("no need mute, before spk volume is zero.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    auto audioGroupManager = AudioStandard::AudioVolumeClientManager::GetInstance().GetGroupManager(
        OHOS::AudioStandard::DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_LOG_RET(audioGroupManager != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR, "audioGroup nullptr");
    if (audioGroupManager->SetMute(
        AudioStandard::AudioVolumeType::STREAM_MUSIC, true, AudioStandard::DEVICE_TYPE_SPEAKER) != 0) {
        HILOGE("set mute failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    BtChrUeManager::GetInstance()->WriteSetMusicMuteUe(CHR_UE_SEND_MEDIA_MUTE,
        UE_COMMON_SCENE_CASE1,
        static_cast<int32_t>(AudioStandard::AudioVolumeType::STREAM_MUSIC),
        static_cast<int32_t>(AudioStandard::DEVICE_TYPE_SPEAKER));
#endif
    return Bluetooth::BT_NO_ERROR;
}
 
int32_t BluetoothAudioFrameworkAdapter::SetMusicUnmute()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    auto audioGroupManager = AudioStandard::AudioVolumeClientManager::GetInstance().GetGroupManager(
        OHOS::AudioStandard::DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_LOG_RET(audioGroupManager != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR, "audioGroup nullptr");
    auto ret = audioGroupManager->SetMute(
        AudioStandard::AudioVolumeType::STREAM_MUSIC, false, AudioStandard::DEVICE_TYPE_SPEAKER);
    if (ret != 0) {
        HILOGE("set unmute failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    BtChrUeManager::GetInstance()->WriteSetMusicMuteUe(CHR_UE_SEND_MEDIA_MUTE,
        UE_COMMON_SCENE_CASE0,
        static_cast<int32_t>(AudioStandard::AudioVolumeType::STREAM_MUSIC),
        static_cast<int32_t>(AudioStandard::DEVICE_TYPE_SPEAKER));
#endif
    return Bluetooth::BT_NO_ERROR;
}

int32_t BluetoothAudioFrameworkAdapter::SetAudioMicrophoneMute(bool isMute)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    auto ret = AudioStandard::AudioVolumeClientManager::GetInstance().SetMicrophoneMute(isMute);
    if (ret != 0) {
        HILOGE("set MicrophoneMute failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
#endif
    return Bluetooth::BT_NO_ERROR;
}

#ifdef AUDIO_FRAMEWORK
void BluetoothAudioFrameworkAdapter::BluetoothAudioVolumeListener::OnVolumeKeyEvent(
    AudioStandard::VolumeEvent volumeEvent)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t streamType = volumeEvent.volumeType;
    int32_t volume = volumeEvent.volume;
    HILOGD("streamType = %{public}d, volume = %{public}d\n", streamType, volume);
    IProfileAvrcpTg *avrcpTgService_ =
        static_cast<IProfileAvrcpTg *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_AVRCP_TG));
    if (avrcpTgService_ != nullptr) {
        if (streamType == AudioStandard::AudioStreamType::STREAM_MUSIC) {
            avrcpTgService_->NotifyAudioVolumeEvent(streamType, volume);
        }
    }
    IProfileHfpAg *hfpAgService_ =
        static_cast<IProfileHfpAg *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_AG));
    if (hfpAgService_ != nullptr) {
        if (streamType == AudioStandard::AudioStreamType::STREAM_VOICE_CALL ||
            streamType == AudioStandard::AudioStreamType::STREAM_BLUETOOTH_SCO) {
            hfpAgService_->NotifyAudioVolumeEvent(streamType, volume);
        }
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    IProfileHfpHf *hfpHfService_ =
        static_cast<IProfileHfpHf *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_HF));
    if (hfpHfService_ != nullptr) {
        if (streamType == AudioStandard::AudioStreamType::STREAM_VOICE_CALL ||
            streamType == AudioStandard::AudioStreamType::STREAM_BLUETOOTH_SCO) {
            int volumeType = HFP_HF_VOLUME_TYPE_SPK;
            hfpHfService_->SetHfVolume(volume, volumeType);
        }
    }
#endif
}

bool IsNeedSetActiveMode(int streamUsage)
{
    static std::vector<AudioStandard::StreamUsage> streamUsageTable {
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT,
        AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM,
        AudioStandard::StreamUsage::STREAM_USAGE_ALARM,
        AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION,
        AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION
    };
    auto iter = std::find(streamUsageTable.begin(), streamUsageTable.end(), streamUsage);
    if (iter != streamUsageTable.end()) {
        return true;
    }
    return false;
}

BtChrBusinessType GetAudioBusinessType(int streamUsage)
{
    const static std::vector<AudioStandard::StreamUsage> streamUsageTable {
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION,
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT,
        AudioStandard::StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION,
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_RINGTONE,
        AudioStandard::StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT
    };
    auto iter = std::find(streamUsageTable.begin(), streamUsageTable.end(), streamUsage);
    if (iter != streamUsageTable.end()) {
        return BUSINESS_TYPE_CALL;
    }
    return BUSINESS_TYPE_MEDIA;
}

void ChrUpdateRenderPkgName(const StreamInfo &streamInfo)
{
    std::string bundleName = streamInfo.bundleName;
    bool isRendererRunning = (streamInfo.status == AudioStandard::RendererState::RENDERER_RUNNING);
    BtChrBusinessType businessType = GetAudioBusinessType(streamInfo.streamUsage);
    BtChrPkgNameManager::GetInstance().UpdateRenderPkgName(
        bundleName, isRendererRunning, businessType, streamInfo.streamId);
}

A2dpRendererState BluetoothAudioFrameworkAdapter::GetA2dpRendererState()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return rendererState_;
}

void BluetoothAudioFrameworkAdapter::SetA2dpRendererState(A2dpRendererState &state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    rendererState_ = state;
}

std::vector<StreamInfo> BluetoothAudioFrameworkAdapter::GetRouteFlagList()
{
    std::lock_guard<std::mutex> lock(routeFlagListMutex_);
    return routeFlagList_;
}

void BluetoothAudioFrameworkAdapter::AddMemberRouteFlagList(const StreamInfo &newInfo)
{
    std::lock_guard<std::mutex> lock(routeFlagListMutex_);
    routeFlagList_.push_back(newInfo);
}

void BluetoothAudioFrameworkAdapter::RemoveMemberRouteFlagList(const uint32_t &flag)
{
    std::lock_guard<std::mutex> lock(routeFlagListMutex_);
    routeFlagList_.erase(std::remove_if(routeFlagList_.begin(), routeFlagList_.end(),
        [flag](const StreamInfo &item) { return item.routeFlag == flag; }), routeFlagList_.end());
}

// Update the stream information in all A2DP pipes.
void BluetoothAudioFrameworkAdapter::UpdateRouteFlagList(
    uint32_t flag, const std::map<uint32_t, AudioStandard::RendererStreamInfo> &rendererStreams)
{
    // Delete all streams in the pipe, traverse the pipe and add streams.
    RemoveMemberRouteFlagList(flag);
    for (const auto &rendererStream : rendererStreams) {
        auto info = rendererStream.second;
        if (GetRouteFlagList().size() > RENDERER_MAX_COUNT) {
            HILOGE("[BluetoothAudioFrameworkAdapter] err size up to limit");
            return;
        }
        HILOGI("rendererStream state=%{public}d, usage=%{public}d, streamId=%{public}d", info.state_,
            info.usage_, info.streamId_);
        if (isCallStreamType(info.usage_)) {
            continue;
        }
        StreamInfo newInfo = {
            .routeFlag = flag,
            .streamId = info.streamId_,
            .streamUsage = info.usage_,
            .status = info.state_,
            .bundleName = info.bundleName_,
        };
        AddMemberRouteFlagList(newInfo);
    }
}

bool BluetoothAudioFrameworkAdapter::isCallStreamType(AudioStandard::StreamUsage streamUsage)
{
    switch (streamUsage) {
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_RINGTONE:
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_ASSISTANT:
        case OHOS::AudioStandard::STREAM_USAGE_RINGTONE:
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_CALL_ASSISTANT:
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::STREAM_USAGE_VIDEO_COMMUNICATION:
            return true;
        default:
            break;
    }
    return false;
}

void BluetoothAudioFrameworkAdapter::ClearRouteFlagList()
{
    HILOGD("[BluetoothAudioFrameworkAdapter] enter");
    std::lock_guard<std::mutex> lock(routeFlagListMutex_);
    routeFlagList_.clear();
    routeFlagList_.shrink_to_fit();
}

static bool IsPipeDeviceTypeA2dp(const std::vector<AudioStandard::DeviceType> &deviceTypeVec)
{
    auto iter = std::find(deviceTypeVec.begin(), deviceTypeVec.end(), DEVICE_TYPE_BLUETOOTH_A2DP);
    if (iter != deviceTypeVec.end()) {
        HILOGI("have A2DP");
        return true;
    }
    return false;
}

// Traverse streams in all pipes, determine game status, music status, and navigation status.
void BluetoothAudioFrameworkAdapter::UpdateRendererState(
    const std::shared_ptr<AudioStandard::AudioOutputPipeInfo> &changedPipeInfo)
{
    CHECK_AND_RETURN_LOG(changedPipeInfo != nullptr, "changedPipeInfo null");
    auto rendererStreams = changedPipeInfo->GetStreams();
    CHECK_AND_RETURN_LOG(rendererStreams.size() < RENDERER_MAX_COUNT, "rendererStreams is over limit");

    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr, "serviceMgr null");
    IProfileA2dpSrc *a2dpSrcService = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
    CHECK_AND_RETURN_LOG(a2dpSrcService != nullptr, "a2dpSrcService null");
    bool isRendererRunning = false;
    A2dpRendererState state;
    std::vector<StreamInfo> streamInfoVec = GetRouteFlagList();
    for (const auto &streamInfo : streamInfoVec) {
        ChrUpdateRenderPkgName(streamInfo);
        HILOGI("[BluetoothAudioFrameworkAdapter] rendererState=%{public}d, streamUsage=%{public}d, streamId=%{public}d",
            streamInfo.status, streamInfo.streamUsage, streamInfo.streamId);
        isRendererRunning = (streamInfo.status == AudioStandard::RendererState::RENDERER_RUNNING);
        state.isRenderActive = state.isRenderActive || isRendererRunning; // only one renderer active, set true
        if (streamInfo.status == AudioStandard::RendererState::RENDERER_PREPARED &&
            IsNeedSetActiveMode(streamInfo.streamUsage) &&
            BtGetActiveOutputDevice() == BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) {
            a2dpSrcService->HwSetActiveMode();
        }
        if (streamInfo.streamUsage == OHOS::AudioStandard::STREAM_USAGE_GAME &&
            streamInfo.routeFlag == AUDIO_OUTPUT_FLAG_FAST) {
            if (isRendererRunning) {
                state.gameRunning = true;
            }
        } else if (streamInfo.streamUsage == OHOS::AudioStandard::STREAM_USAGE_MUSIC &&
            streamInfo.routeFlag == AUDIO_OUTPUT_FLAG_NORMAL) {
            if (isRendererRunning) {
                state.musicRunning = true;
            }
        } else if (streamInfo.streamUsage == OHOS::AudioStandard::STREAM_USAGE_NAVIGATION &&
            streamInfo.routeFlag == AUDIO_OUTPUT_FLAG_NORMAL) {
            if (isRendererRunning) {
                state.navigationRunning = true;
            }
        }
    }
    SetA2dpRendererState(state);
}

bool BluetoothAudioFrameworkAdapter::BluetoothAudioOutputPipeListener::IsA2dpPipe(
    const AudioStandard::AudioPipeChangeType changeType, uint32_t routeFlag,
    const std::vector<AudioStandard::DeviceType> &deviceTypeVec,
    AudioStandard::HdiAdapterType adapterType)
{
    // filter A2DP : 1. only a2dp device type 2. a2dp adapter
    std::lock_guard<std::mutex> lock(a2dpDeviceChangeMutex_);
    if (changeType == AudioStandard::PIPE_CHANGE_TYPE_PIPE_DEVICE && routeFlag == AUDIO_OUTPUT_FLAG_NORMAL) {
        isA2dpDevice_ = IsPipeDeviceTypeA2dp(deviceTypeVec);
    }
    return (isA2dpDevice_ || adapterType == AudioStandard::HDI_ADAPTER_TYPE_A2DP);
}

void BluetoothAudioFrameworkAdapter::BluetoothAudioOutputPipeListener::OnOutputPipeChange(
    const AudioStandard::AudioPipeChangeType changeType,
    const std::shared_ptr<AudioStandard::AudioOutputPipeInfo> &changedPipeInfo)
{
    HITRACE_METER(BT_TRACE_TAG);
    CHECK_AND_RETURN_LOG(changedPipeInfo != nullptr, "changedPipeInfo null");
    AudioStandard::HdiAdapterType adapterType = changedPipeInfo->GetAdapter();
    AudioStandard::AudioPipeStatus pipeStatus = changedPipeInfo->GetStatus();
    std::vector<AudioStandard::DeviceType> deviceTypeVec = changedPipeInfo->GetDevices();
    uint32_t routeFlag = changedPipeInfo->GetRouteFlag();
    auto rendererStreams = changedPipeInfo->GetStreams();
    std::string logKey = std::to_string(changeType) + std::to_string(adapterType) +
        std::to_string(pipeStatus) + std::to_string(routeFlag);
    HILOGI_TIME_LIMIT(logKey, "OnOutputPipeChange: change: %{public}d, adapter: %{public}d, status: %{public}d, "
        "routeFlag: %{public}d", changeType, adapterType, pipeStatus, routeFlag);
    CHECK_AND_RETURN_LOG(IsA2dpPipe(changeType, routeFlag, deviceTypeVec, adapterType), "pipe not a2dp");
    // Update stream list and stream status
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateRouteFlagList(routeFlag, rendererStreams);
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateRendererState(changedPipeInfo);

    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr, "serviceMgr null");
    IProfileA2dpSrc *a2dpSrcService = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
    CHECK_AND_RETURN_LOG(a2dpSrcService != nullptr, "a2dpSrcService null");

    A2dpRendererState state = BluetoothAudioFrameworkAdapter::GetInstance().GetA2dpRendererState();
    a2dpSrcService->OnRenderStateChange(state.isRenderActive);
    HILOGI_TIME_LIMIT(std::to_string(state.isRenderActive), "OnOutputPipeChange: game: %{public}d, music: %{public}d, "
        "navigation: %{public}d, isActive: %{public}d", state.gameRunning, state.musicRunning,
        state.navigationRunning, state.isRenderActive);
    a2dpSrcService->OnRenderSceneChange(state.isRenderActive, state.gameRunning, state.musicRunning);
    a2dpSrcService->SetNavigationMode(state.navigationRunning);
}


void BluetoothAudioFrameworkAdapter::UpdateMuteRendererState(
    const RawAddress &device, const uint32_t streamId, bool isExist)
{
    HILOGI("mute renderer update: device = %{public}s, streamId = %{public}d, isExist = %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), streamId, isExist);
    bool isRenderActive = false; // play status
    bool isRendererRunning = false;
    bool isMuteStreamA2dp = false;
    std::vector<StreamInfo> streamInfoVec = GetRouteFlagList();
    for (const auto &streamInfo : streamInfoVec) {
        // Check the status of all a2dp streams except the mute stream.
        if (streamInfo.streamId == streamId) {
            isMuteStreamA2dp = true;
            continue;
        }
        isRendererRunning = (streamInfo.status == AudioStandard::RendererState::RENDERER_RUNNING);
        isRenderActive = isRenderActive || isRendererRunning; // only one renderer active, set true
    }
    if (!isMuteStreamA2dp) {
        HILOGD("mute stream is not a2dp stream");
        return;
    }
    if (!isExist) {
        // mute stream resume
        isRenderActive = true;
    }
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr, "serviceMgr null");
    IProfileA2dpSrc *a2dpSrcService = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
    CHECK_AND_RETURN_LOG(a2dpSrcService != nullptr, "a2dpSrcService null");
    a2dpSrcService->OnRenderStateChange(isRenderActive);
}

// Mute stream: data is 0
void BluetoothAudioFrameworkAdapter::BluetoothAudioRendererDataTransferListener::OnDataTransferStateChange(
    const AudioStandard::AudioRendererDataTransferStateChangeInfo &info)
{
    HITRACE_METER(BT_TRACE_TAG);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnDataTransferStateChange: null a2dpService");
    auto device = a2dpService->GetActiveSinkDevice();
    STACK::RawAddress addr = ServiceUtil::AddrToStack(device);
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    bool isHwDevice = bluetoothHwSrcInterface->isHwDualConnDevice(addr);
    if (!isHwDevice) {
        HILOGI("not hw headphone device");
        return;
    }
    
    // no need to consider record
    CHECK_AND_RETURN_LOG((info.audioMode != AudioStandard::AUDIO_MODE_RECORD), "audioMode is record!");
    AudioStandard::DataTransferStateChangeType rendererState = info.stateChangeType;
    switch (rendererState) {
        case AudioStandard::DataTransferStateChangeType::AUDIO_STREAM_START:
        case AudioStandard::DataTransferStateChangeType::AUDIO_STREAM_STOP:
        case AudioStandard::DataTransferStateChangeType::AUDIO_STREAM_PAUSE:
            break;
        // mute stream resume
        case AudioStandard::DataTransferStateChangeType::DATA_TRANS_RESUME:
            HILOGI("DATA_TRANS_RESUME");
            BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, info.sessionId, false);
            break;
        // mute stream trigger
        case AudioStandard::DataTransferStateChangeType::DATA_TRANS_STOP:
            HILOGI("DATA_TRANS_STOP");
            BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, info.sessionId, true);
            break;
        default:
            return;
    }
}

// Mute stream: volume is 0
void BluetoothAudioFrameworkAdapter::BluetoothAudioRendererDataTransferListener::OnMuteStateChange(const int32_t &uid,
    const uint32_t &streamId, const bool &isMuted)
{
    HITRACE_METER(BT_TRACE_TAG);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnMuteStateChange: null a2dpService");
    auto device = a2dpService->GetActiveSinkDevice();
    STACK::RawAddress addr = ServiceUtil::AddrToStack(device);
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    bool isHwDevice = bluetoothHwSrcInterface->isHwDualConnDevice(addr);
    if (!isHwDevice) {
        HILOGI("not hw headphone device");
        return;
    }
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateMuteRendererState(device, streamId, isMuted);
}

#endif

#ifdef AUDIO_FRAMEWORK
void BluetoothAudioFrameworkAdapter::BluetoothAudioPreferredOutPutDeviceChangeListener::OnPreferredOutputDeviceUpdated(
    const std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> &deviceDescriptor)
{
#ifdef MEDIA_AND_CALL_VOLUME_INTEGRATION
    HILOGI("Media and call volume integration");
    return;
#endif
    if (deviceDescriptor.size() == 0) {
        HILOGE("deviceDescriptor size is zero");
        return;
    }
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnPreferredOutputDeviceUpdated: null a2dpService");
 
    switch (deviceDescriptor[0]->deviceType_) {
        case AudioStandard::DEVICE_TYPE_SPEAKER:
            HILOGI("networkid is %{public}s", deviceDescriptor[0]->networkId_.c_str());
            if (deviceDescriptor[0]->networkId_ == "LocalDevice" &&
                outPutDeviceType_ == AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) { // localdevice 表示切到扬声器
                a2dpService->SetCurrentDeviceMute();
            }
            break;
        case AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            HILOGI("networkid is %{public}s", deviceDescriptor[0]->networkId_.c_str());
            BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioFrameworkAdapterListener();
            break;
        default:
            break;
    }
    outPutDeviceType_ = deviceDescriptor[0]->deviceType_;
}
#endif

#ifdef AUDIO_FRAMEWORK
AudioStandard::AudioStreamType ConvertBtStreamTypeToAudio(StreamType streamType)
{
    static std::map<StreamType, AudioStandard::AudioStreamType> streamTypeTable {
        {StreamType::STREAM_DEFAULT, AudioStandard::AudioStreamType::STREAM_DEFAULT},
        {StreamType::STREAM_VOICE_CALL, AudioStandard::AudioStreamType::STREAM_VOICE_CALL},
        {StreamType::STREAM_MUSIC, AudioStandard::AudioStreamType::STREAM_MUSIC},
        {StreamType::STREAM_RING, AudioStandard::AudioStreamType::STREAM_RING},
        {StreamType::STREAM_MEDIA, AudioStandard::AudioStreamType::STREAM_MEDIA},
        {StreamType::STREAM_VOICE_ASSISTANT, AudioStandard::AudioStreamType::STREAM_VOICE_ASSISTANT},
        {StreamType::STREAM_SYSTEM, AudioStandard::AudioStreamType::STREAM_SYSTEM},
        {StreamType::STREAM_ALARM, AudioStandard::AudioStreamType::STREAM_ALARM},
        {StreamType::STREAM_NOTIFICATION, AudioStandard::AudioStreamType::STREAM_NOTIFICATION},
        {StreamType::STREAM_BLUETOOTH_SCO, AudioStandard::AudioStreamType::STREAM_BLUETOOTH_SCO},
        {StreamType::STREAM_ENFORCED_AUDIBLE, AudioStandard::AudioStreamType::STREAM_ENFORCED_AUDIBLE},
        {StreamType::STREAM_DTMF, AudioStandard::AudioStreamType::STREAM_DTMF},
        {StreamType::STREAM_TTS, AudioStandard::AudioStreamType::STREAM_TTS},
        {StreamType::STREAM_ACCESSIBILITY, AudioStandard::AudioStreamType::STREAM_ACCESSIBILITY},
        {StreamType::STREAM_RECORDING, AudioStandard::AudioStreamType::STREAM_RECORDING}
    };

    auto iter = streamTypeTable.find(streamType);
    if (iter != streamTypeTable.end()) {
        return iter->second;
    }
    return AudioStandard::AudioStreamType::STREAM_DEFAULT;
}

BtDeviceType ConvertAudioDeviceTypeToBt(AudioStandard::DeviceType deviceType)
{
    static std::map<AudioStandard::DeviceType, BtDeviceType> audioDeviceTypeTable {
        {AudioStandard::DeviceType::DEVICE_TYPE_INVALID, BtDeviceType::DEVICE_TYPE_INVALID},
        {AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE, BtDeviceType::DEVICE_TYPE_EARPIECE},
        {AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER, BtDeviceType::CUSTOM_DEVICE_TYPE_SPEAKER},
        {AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADSET, BtDeviceType::DEVICE_TYPE_WIRED_HEADSET},
        {AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADPHONES, BtDeviceType::DEVICE_TYPE_WIRED_HEADPHONES},
        {AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO},
        {AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP},
        {AudioStandard::DeviceType::DEVICE_TYPE_MIC, BtDeviceType::DEVICE_TYPE_MIC},
        {AudioStandard::DeviceType::DEVICE_TYPE_WAKEUP, BtDeviceType::DEVICE_TYPE_WAKEUP},
        {AudioStandard::DeviceType::DEVICE_TYPE_USB_HEADSET, BtDeviceType::DEVICE_TYPE_USB_HEADSET},
        {AudioStandard::DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, BtDeviceType::DEVICE_TYPE_USB_ARM_HEADSET},
        {AudioStandard::DeviceType::DEVICE_TYPE_FILE_SINK, BtDeviceType::DEVICE_TYPE_FILE_SINK},
        {AudioStandard::DeviceType::DEVICE_TYPE_FILE_SOURCE, BtDeviceType::DEVICE_TYPE_FILE_SOURCE},
        {AudioStandard::DeviceType::DEVICE_TYPE_EXTERN_CABLE, BtDeviceType::DEVICE_TYPE_EXTERN_CABLE},
        {AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT, BtDeviceType::CUSTOM_DEVICE_TYPE_DEFAULT},
        {AudioStandard::DeviceType::DEVICE_TYPE_MAX, BtDeviceType::DEVICE_TYPE_MAX},
        {AudioStandard::DeviceType::DEVICE_TYPE_NONE, BtDeviceType::DEVICE_TYPE_NONE}
    };

    auto iter = audioDeviceTypeTable.find(deviceType);
    if (iter != audioDeviceTypeTable.end()) {
        return iter->second;
    }
    return BtDeviceType::DEVICE_TYPE_NONE;
}

AudioStandard::DeviceType ConvertBtDeviceTypeToAudio(BtDeviceType deviceType) {
     static std::map<BtDeviceType, AudioStandard::DeviceType> btDeviceTypeTable {
         {BtDeviceType::DEVICE_TYPE_INVALID, AudioStandard::DeviceType::DEVICE_TYPE_INVALID},
         {BtDeviceType::DEVICE_TYPE_EARPIECE, AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE},
         {BtDeviceType::CUSTOM_DEVICE_TYPE_SPEAKER, AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER},
         {BtDeviceType::DEVICE_TYPE_WIRED_HEADSET, AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADSET},
         {BtDeviceType::DEVICE_TYPE_WIRED_HEADPHONES, AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADPHONES},
         {BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO, AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO},
         {BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP},
         {BtDeviceType::DEVICE_TYPE_MIC, AudioStandard::DeviceType::DEVICE_TYPE_MIC},
         {BtDeviceType::DEVICE_TYPE_WAKEUP, AudioStandard::DeviceType::DEVICE_TYPE_WAKEUP},
         {BtDeviceType::DEVICE_TYPE_USB_HEADSET, AudioStandard::DeviceType::DEVICE_TYPE_USB_HEADSET},
         {BtDeviceType::DEVICE_TYPE_USB_ARM_HEADSET, AudioStandard::DeviceType::DEVICE_TYPE_USB_ARM_HEADSET},
         {BtDeviceType::DEVICE_TYPE_FILE_SINK, AudioStandard::DeviceType::DEVICE_TYPE_FILE_SINK},
         {BtDeviceType::DEVICE_TYPE_FILE_SOURCE, AudioStandard::DeviceType::DEVICE_TYPE_FILE_SOURCE},
         {BtDeviceType::DEVICE_TYPE_EXTERN_CABLE, AudioStandard::DeviceType::DEVICE_TYPE_EXTERN_CABLE},
         {BtDeviceType::CUSTOM_DEVICE_TYPE_DEFAULT, AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT},
         {BtDeviceType::DEVICE_TYPE_MAX, AudioStandard::DeviceType::DEVICE_TYPE_MAX},
         {BtDeviceType::DEVICE_TYPE_NONE, AudioStandard::DeviceType::DEVICE_TYPE_NONE}
     };
     auto iter = btDeviceTypeTable.find(deviceType);
     if (iter != btDeviceTypeTable.end()) {
         return iter->second;
     }
     return AudioStandard::DeviceType::DEVICE_TYPE_NONE;
}

AudioStandard::AudioVolumeType ConvertBtVolumeTypeToAudio(BtVolumeType type)
{
    static std::map<BtVolumeType, AudioStandard::AudioVolumeType> btVolumeTypeTable {
        {BtVolumeType::VOLUME_DEFAULT, AudioStandard::AudioVolumeType::STREAM_DEFAULT},
        {BtVolumeType::VOLUME_VOICE_CALL, AudioStandard::AudioVolumeType::STREAM_VOICE_CALL},
        {BtVolumeType::VOLUME_MUSIC, AudioStandard::AudioVolumeType::STREAM_MUSIC},
        {BtVolumeType::VOLUME_RING, AudioStandard::AudioVolumeType::STREAM_RING},
        {BtVolumeType::VOLUME_MEDIA, AudioStandard::AudioVolumeType::STREAM_MEDIA},
        {BtVolumeType::VOLUME_VOICE_ASSISTANT, AudioStandard::AudioVolumeType::STREAM_VOICE_ASSISTANT},
        {BtVolumeType::VOLUME_SYSTEM, AudioStandard::AudioVolumeType::STREAM_SYSTEM},
        {BtVolumeType::VOLUME_ALARM, AudioStandard::AudioVolumeType::STREAM_ALARM},
        {BtVolumeType::VOLUME_NOTIFICATION, AudioStandard::AudioVolumeType::STREAM_NOTIFICATION},
        {BtVolumeType::VOLUME_BLUETOOTH_SCO, AudioStandard::AudioVolumeType::STREAM_BLUETOOTH_SCO},
        {BtVolumeType::VOLUME_ENFORCED_AUDIBLE, AudioStandard::AudioVolumeType::STREAM_ENFORCED_AUDIBLE},
        {BtVolumeType::VOLUME_DTMF, AudioStandard::AudioVolumeType::STREAM_DTMF},
        {BtVolumeType::VOLUME_TTS, AudioStandard::AudioVolumeType::STREAM_TTS},
        {BtVolumeType::VOLUME_ACCESSIBILITY, AudioStandard::AudioVolumeType::STREAM_ACCESSIBILITY},
        {BtVolumeType::VOLUME_RECORDING, AudioStandard::AudioVolumeType::STREAM_RECORDING},
        {BtVolumeType::VOLUME_ALL, AudioStandard::AudioVolumeType::STREAM_ALL}
    };

    auto iter = btVolumeTypeTable.find(type);
    if (iter != btVolumeTypeTable.end()) {
        return iter->second;
    }
    return AudioStandard::AudioVolumeType::STREAM_DEFAULT;
}

BtAudioScene ConvertAudioSceneToBt(AudioStandard::AudioScene scene)
{
    static std::map<AudioStandard::AudioScene, BtAudioScene> audioSceneTable {
        {AudioStandard::AudioScene::AUDIO_SCENE_INVALID, BtAudioScene::AUDIO_SCENE_INVALID},
        {AudioStandard::AudioScene::AUDIO_SCENE_DEFAULT, BtAudioScene::AUDIO_SCENE_DEFAULT},
        {AudioStandard::AudioScene::AUDIO_SCENE_RINGING, BtAudioScene::AUDIO_SCENE_RINGING},
        {AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CALL, BtAudioScene::AUDIO_SCENE_PHONE_CALL},
        {AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CHAT, BtAudioScene::AUDIO_SCENE_PHONE_CHAT},
        {AudioStandard::AudioScene::AUDIO_SCENE_CALL_START, BtAudioScene::AUDIO_SCENE_CALL_START},
        {AudioStandard::AudioScene::AUDIO_SCENE_CALL_END, BtAudioScene::AUDIO_SCENE_CALL_END},
        {AudioStandard::AudioScene::AUDIO_SCENE_VOICE_RINGING, BtAudioScene::AUDIO_SCENE_VOICE_RINGING},
        {AudioStandard::AudioScene::AUDIO_SCENE_MAX, BtAudioScene::AUDIO_SCENE_MAX}
    };

    auto iter = audioSceneTable.find(scene);
    if (iter != audioSceneTable.end()) {
        return iter->second;
    }
    return BtAudioScene::AUDIO_SCENE_INVALID;
}

std::shared_ptr<AudioStandard::AudioDeviceDescriptor> GetAudioDeviceDescriptor(const std::string &macAddr)
{
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev =
        std::make_shared<AudioStandard::AudioDeviceDescriptor>();
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, nullptr, "Device err: null audioDevDescriptor");
    audioDev->macAddress_ = macAddr;
    return audioDev;
}

bool IsSpatialAudioModeSupported(const std::string &macAddr)
{
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(macAddr);
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, false, "Device err: null audioDevDescriptor");
    return AudioStandard::AudioSpatializationManager::GetInstance()->IsSpatializationSupportedForDevice(audioDev);
}

bool IsSpatialAudioHeadTrackingSupported(const std::string &macAddr)
{
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(macAddr);
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, false, "Device err: null audioDevDescriptor");
    return AudioStandard::AudioSpatializationManager::GetInstance()->IsHeadTrackingSupportedForDevice(audioDev);
}
#endif

bool BluetoothAudioFrameworkAdapter::IsMusicActive(StreamType streamType)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    AudioStandard::AudioStreamType audioStreamtype = ConvertBtStreamTypeToAudio(streamType);
    return AudioStandard::AudioVolumeClientManager::GetInstance().IsStreamActive(audioStreamtype);
#else
    return false;
#endif
}

int32_t BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(
    std::string macAddr, bool supported, int32_t volume)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t ret = ERROR_CODE;
#ifdef AUDIO_FRAMEWORK
    if (supported) {
        ret = AudioStandard::AudioVolumeClientManager::GetInstance().SetDeviceAbsVolumeSupported(macAddr,
            supported, volume);
    } else {
        ret = AudioStandard::AudioVolumeClientManager::GetInstance().SetDeviceAbsVolumeSupported(macAddr, supported);
    }
#endif
    return ret;
}

int32_t BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(std::string macAddr, int32_t storedVolume, bool supported)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t ret = ERROR_CODE;
#ifdef AUDIO_FRAMEWORK
    ret = AudioStandard::AudioVolumeClientManager::GetInstance().SetA2dpDeviceVolume(macAddr, storedVolume, supported);
#endif
    return ret;
}

BtDeviceType BluetoothAudioFrameworkAdapter::BtGetActiveOutputDevice()
{
    HITRACE_METER(BT_TRACE_TAG);
    BtDeviceType btDeviceType = BtDeviceType::DEVICE_TYPE_NONE;
#ifdef AUDIO_FRAMEWORK
    AudioStandard::DeviceType deviceType = AudioStandard::AudioDevicesClientManager::GetInstance().
        GetActiveOutputDevice();
    btDeviceType = ConvertAudioDeviceTypeToBt(deviceType);
#endif
    return btDeviceType;
}

int32_t BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType volumeType, BtDeviceType deviceType)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t volume = -1;
#ifdef AUDIO_FRAMEWORK
    AudioStandard::AudioVolumeType audioVolumeType = ConvertBtVolumeTypeToAudio(volumeType);
    AudioStandard::DeviceType audioDeviceType = ConvertBtDeviceTypeToAudio(deviceType);
    volume = AudioStandard::AudioVolumeClientManager::GetInstance().GetDeviceMaxVolume(audioVolumeType,
        audioDeviceType);
#endif
    return volume;
}

void BluetoothAudioFrameworkAdapter::HfpSetAudioParameters(const std::string &key, const std::string &value)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    HILOGI("[HFP_SYSTEM_INTERFACE]SetAudioParameter to audio");
    AudioStandard::AudioSystemClientEngineManager::GetInstance().SetAudioParameter(key, value);
#endif
}

void BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    HILOGI("[HFP_SYSTEM_INTERFACE]SetExtraParameters to audio");
    AudioStandard::AudioSystemClientEngineManager::GetInstance().SetExtraParameters(key, kvpairs);
#endif
}

void BluetoothAudioFrameworkAdapter::HfpGetExtraAudioParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    AudioStandard::AudioSystemClientEngineManager::GetInstance().GetExtraParameters(mainKey, subKeys, result);
#endif
}

void BluetoothAudioFrameworkAdapter::SetStreamVolume(int streamType, int volume, int flag)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    if (streamType == STREAM_VOICE_CALL) {
        AudioStandard::AudioStreamType volumeType = AudioStandard::AudioStreamType::STREAM_VOICE_CALL;
        AudioStandard::AudioVolumeClientManager::GetInstance().SetVolumeWithDevice(volumeType, volume,
            AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    } else if (streamType == STREAM_MUSIC) {
        AudioStandard::AudioStreamType volumeType = AudioStandard::AudioStreamType::STREAM_MUSIC;
        AudioStandard::AudioVolumeClientManager::GetInstance().SetVolumeWithDevice(volumeType, volume,
            AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
    } else {
        HILOGW("[HFP_SYSTEM_INTERFACE]do not handle this streamType=%{public}d", streamType);
    }
#endif
}

int BluetoothAudioFrameworkAdapter::GetStreamVolume(int streamType)
{
    HITRACE_METER(BT_TRACE_TAG);
    int volume = 0;
#ifdef AUDIO_FRAMEWORK
    if (streamType == STREAM_VOICE_CALL) {
        AudioStandard::AudioStreamType volumeType = AudioStandard::AudioStreamType::STREAM_VOICE_CALL;
        volume = AudioStandard::AudioVolumeClientManager::GetInstance().GetVolume(volumeType);
    } else if (streamType == STREAM_MUSIC) {
        AudioStandard::AudioStreamType volumeType = AudioStandard::AudioStreamType::STREAM_MUSIC;
        volume = AudioStandard::AudioVolumeClientManager::GetInstance().GetVolume(volumeType);
    } else {
        HILOGW("[HFP_SYSTEM_INTERFACE]do not handle this streamType=%{public}d", streamType);
    }
#endif
    return volume;
}

#ifdef AUDIO_FRAMEWORK
void BluetoothAudioFrameworkAdapter::BluetoothSpatialAudioModeChangeListener::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioStandard::AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    HITRACE_METER(BT_TRACE_TAG);

    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is null");
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnSpatializationEnabledChangeForAnyDevice: null a2dpService");

    auto device = a2dpService->GetActiveSinkDevice();
    if (deviceDescriptor->macAddress_ != device.GetAddress()) {
        HILOGI("remote device %{public}s is not active device, do not handle",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str());
        return;
    }
    if (!IsSpatialAudioModeSupported(device.GetAddress())) {
        HILOGI("remote device %{public}s not support spatial audio mode",
            GetEncryptAddr(device.GetAddress()).c_str());
        return;
    }
    HILOGI("OnSpatializationEnabledChangeForAnyDevice: device=%{public}s, enable=%{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), enabled);
    bluetoothHwSrcInterface->hwSetSpatialAudioModeEnabled(enabled);
}

void BluetoothAudioFrameworkAdapter::BluetoothSpatialAudioHeadTrackingChangeListener::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioStandard::AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    HITRACE_METER(BT_TRACE_TAG);

    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is null");
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnHeadTrackingEnabledChangeForAnyDevice: null a2dpService");

    auto device = a2dpService->GetActiveSinkDevice();
    if (deviceDescriptor->macAddress_ != device.GetAddress()) {
        HILOGI("remote device %{public}s is not active device, do not handle",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str());
        return;
    }
    if (!IsSpatialAudioHeadTrackingSupported(device.GetAddress())) {
        HILOGI("remote device %{public}s not support spatial audio head tracking",
            GetEncryptAddr(device.GetAddress()).c_str());
        return;
    }
    HILOGI("OnHeadTrackingEnabledChangeForAnyDevice: device=%{public}s, enable=%{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), enabled);
    bluetoothHwSrcInterface->hwSetSpatialAudioHeadTrackingEnabled(enabled);
}

void BluetoothAudioFrameworkAdapter::BluetoothColAudioEnableListener::OnCollaborationEnabledChangeForCurrentDevice(
    const bool &enabled)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnCollaborationEnabledChangeForCurrentDevice: null a2dpService");

    auto device = a2dpService->GetActiveSinkDevice();
    HILOGI("OnCollaborationEnabledChangeForCurrentDevice: device=%{public}s, enable=%{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), enabled);
    bluetoothHwSrcInterface->HwSetCollAudioEnableState(enabled);
#endif
}

void BluetoothAudioFrameworkAdapter::BluetoothAudioNnStateChangeListener::OnNnStateChange(const int32_t &nnState)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_VOICE_COMBINE_FEATURE_ENABLE
    HILOGI("nnState=%{public}d", nnState);

    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    bluetoothHwSrcInterface->hwSetLocalVoiceCombineNnState(nnState);
#endif
}

void BluetoothAudioFrameworkAdapter::BluetoothSpatialAudioAudioSourceTypeChangeListener::OnSpatialAudioSourceTypeChange(
    const AudioStandard::SpatialAudioSourceType &mode)
{
    HITRACE_METER(BT_TRACE_TAG);
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "OnSpatialAudioSourceTypeChange: null a2dpService");

    auto device = a2dpService->GetActiveSinkDevice();
    HILOGI("OnSpatialAudioSourceTypeChange: device=%{public}s, mode=%{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), mode);
    bluetoothHwSrcInterface->hwSetAudioSourceType(mode);
}

void BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
    OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioStandard::AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    HITRACE_METER(BT_TRACE_TAG);
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr,
        "OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice: null a2dpService");

    auto device = a2dpService->GetActiveSinkDevice();
    HILOGI("OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice: device=%{public}s, enabled=%{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), enabled);
    if (device.GetAddress() == NULL_ADDRESS || device.GetAddress() == EMPTY_ADDRESS) {
        return;
    }
    adaptiveSwitchStatus_ = enabled;
    bluetoothHwSrcInterface->hwSetAdaptiveSwitchStatus(enabled);
    if (enabled) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE1);
    } else {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE0);
    }
}
#endif

void BluetoothAudioFrameworkAdapter::RegisterSpatialAudioListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    spatialAudioModeCallback_ = std::make_shared<BluetoothSpatialAudioModeChangeListener>();
    spatialAudioHeadTrackingCallback_ = std::make_shared<BluetoothSpatialAudioHeadTrackingChangeListener>();
    spatialAudioSourceTypeCallback_ = std::make_shared<BluetoothSpatialAudioAudioSourceTypeChangeListener>();
    adaptiveSwitchStatusChangeCallback_ = std::make_shared<BluetoothAdaptiveSpatialRenderingEnabledChangeListener>();
    AudioStandard::AudioSpatializationManager::GetInstance()->
        RegisterSpatializationEnabledEventListener(spatialAudioModeCallback_);
    AudioStandard::AudioSpatializationManager::GetInstance()->
        RegisterHeadTrackingEnabledEventListener(spatialAudioHeadTrackingCallback_);
    AudioStandard::AudioSpatializationManager::GetInstance()->
        RegisterSpatialAudioSourceTypeEventListener(spatialAudioSourceTypeCallback_);
    AudioStandard::AudioSpatializationManager::GetInstance()->
        RegisterAdaptiveSpatialRenderingEnabledEventListener(adaptiveSwitchStatusChangeCallback_);
#endif
}

void BluetoothAudioFrameworkAdapter::RegisterCollaborativeAudioListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#if defined(AUDIO_FRAMEWORK) && defined(BLUETOOTH_AUDIO_COLLABORATION_SUPPORT)
    collaborativeAudioModeCallback_ = std::make_shared<BluetoothColAudioEnableListener>();
    AudioStandard::AudioCollaborativeManager::GetInstance()->
        RegisterCollaborationEnabledForCurrentDeviceEventListener(collaborativeAudioModeCallback_);
#endif
}

void BluetoothAudioFrameworkAdapter::UnRegisterCollaborativeAudioListener()
{
    HITRACE_METER(BT_TRACE_TAG);
#if defined(AUDIO_FRAMEWORK) && defined(BLUETOOTH_AUDIO_COLLABORATION_SUPPORT)
    CHECK_AND_RETURN_LOG(collaborativeAudioModeCallback_ != nullptr, "collaborativeAudioModeCallback_ is nullptr");
    auto ret = AudioStandard::AudioCollaborativeManager::
        GetInstance()->UnregisterCollaborationEnabledForCurrentDeviceEventListener();
    HILOGI("ret:%{public}d", ret);
    collaborativeAudioModeCallback_ = nullptr;
#endif
}

bool BluetoothAudioFrameworkAdapter::IsSpatialAudioModeEnabled(const std::string &macAddr)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    if (!IsSpatialAudioModeSupported(macAddr)) {
        return false;
    }
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(macAddr);
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, false, "Device err: null audioDevDescriptor");
    return AudioStandard::AudioSpatializationManager::GetInstance()->IsSpatializationEnabled(audioDev);
#else
    return false;
#endif
}

bool BluetoothAudioFrameworkAdapter::IsSpatialAudioHeadTrackingEnabled(const std::string &macAddr)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    if (!IsSpatialAudioHeadTrackingSupported(macAddr)) {
        return false;
    }
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(macAddr);
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, false, "Device err: null audioDevDescriptor");
    return AudioStandard::AudioSpatializationManager::GetInstance()->IsHeadTrackingEnabled(audioDev);
#else
    return false;
#endif
}


bool BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp()
{
    BtDeviceType deviceType = BtGetActiveOutputDevice();
    if (deviceType == BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) {
        return true;
    }
    return false;
}

bool BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothSco()
{
    BtDeviceType deviceType = BtGetActiveOutputDevice();
    if (deviceType == BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        return true;
    }
    return false;
}

void BluetoothAudioFrameworkAdapter::UpdateLocalVoiceCombineFeatureState()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
#ifdef BLUETOOTH_VOICE_COMBINE_FEATURE_ENABLE
    HILOGI("%{public}s", __func__);
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");

    std::string mainKey = HFP_AG_SERVER_AUDIO_EFFECT_NAME;
    std::vector<std::string> subKeys = {HFP_AG_SERVER_AUDIO_VOICE_COMBINE_NAME};
    std::vector<std::pair<std::string, std::string>> params;
    BluetoothAudioFrameworkAdapter::GetInstance().HfpGetExtraAudioParameters(mainKey, subKeys, params);

    for (auto &it : params) {
        if (it.first == HFP_AG_SERVER_AUDIO_VOICE_COMBINE_NAME) {
            int featureState = 0;
            if (it.second == HFP_VOICE_COMBINE_ABILITY_ON_NN_SWITCH_OFF) {
                featureState = HW_VOICE_COMBINE_STATE_ABILITY_ON_NN_SWITCH_OFF;
            } else if (it.second == HFP_VOICE_COMBINE_ABILITY_ON_NN_SWITCH_ON) {
                featureState = HW_VOICE_COMBINE_STATE_ABILITY_ON_NN_SWITCH_ON;
            }
            bluetoothHwSrcInterface->hwSetLocalVoiceCombineFeatureState(featureState);
            break;
        }
    }
#endif
#endif
}

void BluetoothAudioFrameworkAdapter::SetActiveDeviceVoiceCombineAbility(const bool &ability)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
#ifdef BLUETOOTH_VOICE_COMBINE_FEATURE_ENABLE
    HILOGI("ability: %{public}d", ability);
    const std::string mainKey = "bt_extra";
    std::string voiceCombineAbilityStr = ability ? "true" : "false";
    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs;
    audioParametersKVPairs.push_back(std::make_pair(HFG_AG_SERVER_BT_COMBINE_NR_NAME, voiceCombineAbilityStr));
    BluetoothAudioFrameworkAdapter::GetInstance().HfpSetExtraAudioParameters(mainKey, audioParametersKVPairs);
#endif
#endif
}

bool BluetoothAudioFrameworkAdapter::IsSpatialAudioAdaptiveSwitchEnabled(const std::string &macAddr)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef AUDIO_FRAMEWORK
    std::shared_ptr<AudioStandard::AudioDeviceDescriptor> audioDev = GetAudioDeviceDescriptor(macAddr);
    CHECK_AND_RETURN_LOG_RET(audioDev != nullptr, false, "Device err: null audioDevDescriptor");
    return AudioStandard::AudioSpatializationManager::GetInstance()->IsAdaptiveSpatialRenderingEnabled(audioDev);
#else
    return false;
#endif
}

BtAudioScene BluetoothAudioFrameworkAdapter::GetAudioScene()
{
#ifdef AUDIO_FRAMEWORK
    AudioStandard::AudioScene audioScene =
        AudioStandard::AudioSystemClientPolicyManager::GetInstance().GetAudioScene();
    return ConvertAudioSceneToBt(audioScene);
#else
    return BtAudioScene::AUDIO_SCENE_INVALID;
#endif
}

void BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
    SetAdaptiveSwitchStatus(bool status)
{
    HILOGI("adaptive switch status: %{public}d", status);
    adaptiveSwitchStatus_ = status;
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "bthwInterface_ is null");
    bluetoothHwSrcInterface->hwSetAdaptiveSwitchStatus(status);
}

bool BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::GetAdaptiveSwitchStatus()
{
    return adaptiveSwitchStatus_;
}
} // namespace bluetooth
} // namespace OHOS
