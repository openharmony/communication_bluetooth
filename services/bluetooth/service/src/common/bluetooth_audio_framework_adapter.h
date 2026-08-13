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
#ifndef BLUETOOTH_AUDIO_FRAMEWORK_ADAPTER_H
#define BLUETOOTH_AUDIO_FRAMEWORK_ADAPTER_H

#include <memory>
#include <string>

#ifdef AUDIO_FRAMEWORK
#include "audio_volume_client_manager.h"
#include "audio_devices_client_manager.h"
#include "audio_system_client_engine_manager.h"
#include "audio_routing_client_manager.h"
#include "audio_spatialization_manager.h"
#include "audio_combine_denoising_manager.h"
#include "audio_collaborative_manager.h"
#include "audio_engine_client_manager.h"
#include "audio_output_pipe_types.h"
#include "audio_stream_enum.h"
#include "audio_stream_client_manager.h"
#include "audio_system_client_policy_manager.h"
#include "raw_address.h"
#endif

namespace OHOS {
namespace bluetooth {
constexpr int32_t ERROR_CODE = -1;
enum StreamType {
    /**
     * Indicates audio streams default.
     */
    STREAM_DEFAULT = -1,
    /**
     * Indicates audio streams of voices in calls.
     */
    STREAM_VOICE_CALL = 0,
    /**
     * Indicates audio streams for music playback.
     */
    STREAM_MUSIC = 1,
    /**
     * Indicates audio streams for ringtones.
     */
    STREAM_RING = 2,
    /**
     * Indicates audio streams media.
     */
    STREAM_MEDIA = 3,
    /**
     * Indicates Audio streams for voice assistant
     */
    STREAM_VOICE_ASSISTANT = 4,
    /**
     * Indicates audio streams for system sounds.
     */
    STREAM_SYSTEM = 5,
    /**
     * Indicates audio streams for alarms.
     */
    STREAM_ALARM = 6,
    /**
     * Indicates audio streams for notifications.
     */
    STREAM_NOTIFICATION = 7,
    /**
     * Indicates audio streams for voice calls routed through a connected Bluetooth device.
     */
    STREAM_BLUETOOTH_SCO = 8,
    /**
     * Indicates audio streams for enforced audible.
     */
    STREAM_ENFORCED_AUDIBLE = 9,
    /**
     * Indicates audio streams for dual-tone multi-frequency (DTMF) tones.
     */
    STREAM_DTMF = 10,
    /**
     * Indicates audio streams exclusively transmitted through the speaker (text-to-speech) of a device.
     */
    STREAM_TTS =  11,
    /**
     * Indicates audio streams used for prompts in terms of accessibility.
     */
    STREAM_ACCESSIBILITY = 12,
    /**
     * Indicates special scene used for recording.
     */
    STREAM_RECORDING = 13
};

enum BtDeviceType {
    /**
     * Indicates device type none.
     */
    DEVICE_TYPE_NONE = -1,
    /**
     * Indicates invalid device
     */
    DEVICE_TYPE_INVALID = 0,
    /**
     * Indicates a built-in earpiece device
     */
    DEVICE_TYPE_EARPIECE = 1,
    /**
     * Indicates a speaker built in a device.
     */
    CUSTOM_DEVICE_TYPE_SPEAKER = 2,
    /**
     * Indicates a headset, which is the combination of a pair of headphones and a microphone.
     */
    DEVICE_TYPE_WIRED_HEADSET = 3,
    /**
     * Indicates a pair of wired headphones.
     */
    DEVICE_TYPE_WIRED_HEADPHONES = 4,
    /**
     * Indicates a Bluetooth device used for telephony.
     */
    DEVICE_TYPE_BLUETOOTH_SCO = 7,
    /**
     * Indicates a Bluetooth device supporting the Advanced Audio Distribution Profile (A2DP).
     */
    DEVICE_TYPE_BLUETOOTH_A2DP = 8,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_MIC = 15,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_WAKEUP = 16,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_USB_HEADSET = 22,
    /**
     * Indicates a usb-arm device.
     */
    DEVICE_TYPE_USB_ARM_HEADSET = 23,
    /**
     * Indicates a debug sink device
     */
    DEVICE_TYPE_FILE_SINK = 50,
    /**
     * Indicates a debug source device
     */
    DEVICE_TYPE_FILE_SOURCE = 51,
    /**
     * Indicates any headset/headphone for disconnect
     */
    DEVICE_TYPE_EXTERN_CABLE = 100,
    /**
     * Indicates default device
     */
    CUSTOM_DEVICE_TYPE_DEFAULT = 1000,
    /**
     * Indicates device type max count.
     */
    DEVICE_TYPE_MAX
};

enum BtVolumeType {
    /**
     * Indicates audio streams default.
     */
    VOLUME_DEFAULT = -1,
    /**
     * Indicates audio streams of voices in calls.
     */
    VOLUME_VOICE_CALL = 0,
    /**
     * Indicates audio streams for music playback.
     */
    VOLUME_MUSIC = 1,
    /**
     * Indicates audio streams for ringtones.
     */
    VOLUME_RING = 2,
    /**
     * Indicates audio streams media.
     */
    VOLUME_MEDIA = 3,
    /**
     * Indicates Audio streams for voice assistant
     */
    VOLUME_VOICE_ASSISTANT = 4,
    /**
     * Indicates audio streams for system sounds.
     */
    VOLUME_SYSTEM = 5,
    /**
     * Indicates audio streams for alarms.
     */
    VOLUME_ALARM = 6,
    /**
     * Indicates audio streams for notifications.
     */
    VOLUME_NOTIFICATION = 7,
    /**
     * Indicates audio streams for voice calls routed through a connected Bluetooth device.
     */
    VOLUME_BLUETOOTH_SCO = 8,
    /**
     * Indicates audio streams for enforced audible.
     */
    VOLUME_ENFORCED_AUDIBLE = 9,
    /**
     * Indicates audio streams for dual-tone multi-frequency (DTMF) tones.
     */
    VOLUME_DTMF = 10,
    /**
     * Indicates audio streams exclusively transmitted through the speaker (text-to-speech) of a device.
     */
    VOLUME_TTS =  11,
    /**
     * Indicates audio streams used for prompts in terms of accessibility.
     */
    VOLUME_ACCESSIBILITY = 12,
    /**
     * Indicates special scene used for recording.
     */
    VOLUME_RECORDING = 13,
    /**
     * Indicates audio streams used for only one volume bar of a device.
     */
    VOLUME_ALL = 100
};

enum BtAudioScene : int32_t {
    /**
     * Invalid
     */
    AUDIO_SCENE_INVALID = -1,
    /**
     * Default audio scene
     */
    AUDIO_SCENE_DEFAULT,
    /**
     * Ringing audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_RINGING,
    /**
     * Phone call audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_PHONE_CALL,
    /**
     * Voice chat audio scene
     */
    AUDIO_SCENE_PHONE_CHAT,
    /**
     * AvSession set call start flag
     */
    AUDIO_SCENE_CALL_START,
    /**
     * AvSession set call end flag
     */
    AUDIO_SCENE_CALL_END,
    /**
     * Voice ringing audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_VOICE_RINGING,
    /**
     * Max
     */
    AUDIO_SCENE_MAX,
};

struct A2dpRendererState {
    bool isRenderActive = false; // all pipes, update renderer state
    bool gameRunning = false; // game renderer state
    bool musicRunning = false;
    bool navigationRunning = false;
};

struct StreamInfo {
    uint32_t routeFlag;
    uint32_t streamId;
    uint32_t streamUsage;
    AudioStandard::RendererState status;
    std::string bundleName;
};

class BluetoothAudioFrameworkAdapter {
public:
    explicit BluetoothAudioFrameworkAdapter() {}
    ~BluetoothAudioFrameworkAdapter() {}
    static BluetoothAudioFrameworkAdapter &GetInstance();

#ifdef AUDIO_FRAMEWORK
    class BluetoothAudioVolumeListener : public AudioStandard::VolumeKeyEventCallback {
    public:
        explicit BluetoothAudioVolumeListener() {};
        ~BluetoothAudioVolumeListener() {};

    private:
        void OnVolumeKeyEvent(AudioStandard::VolumeEvent volumeEvent) override;
    };

    class BluetoothAudioOutputPipeListener : public AudioStandard::AudioOutputPipeCallback {
    public:
        explicit BluetoothAudioOutputPipeListener() {};
        ~BluetoothAudioOutputPipeListener() {};

    private:
        void OnOutputPipeChange(const AudioStandard::AudioPipeChangeType changeType,
            const std::shared_ptr<AudioStandard::AudioOutputPipeInfo> &changedPipeInfo) override;
        bool IsA2dpPipe(const AudioStandard::AudioPipeChangeType changeType, uint32_t routeFlag,
            const std::vector<AudioStandard::DeviceType> &deviceTypeVec, AudioStandard::HdiAdapterType adapterType);

        std::mutex a2dpDeviceChangeMutex_;
        bool isA2dpDevice_ = false;
    };

    class BluetoothAudioRendererDataTransferListener :
        public AudioStandard::AudioRendererDataTransferStateChangeCallback {
    public:
        explicit BluetoothAudioRendererDataTransferListener() {};
        ~BluetoothAudioRendererDataTransferListener() {};

    private:
        void OnDataTransferStateChange(const AudioStandard::AudioRendererDataTransferStateChangeInfo &info) override;
        void OnMuteStateChange(const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override;
    };

    class BluetoothSpatialAudioModeChangeListener : public AudioStandard::AudioSpatializationEnabledChangeCallback {
    public:
        explicit BluetoothSpatialAudioModeChangeListener() {};
        ~BluetoothSpatialAudioModeChangeListener() {};

    private:
        void OnSpatializationEnabledChange(const bool &enabled) override {};
        void OnSpatializationEnabledChangeForAnyDevice(const std::shared_ptr<AudioStandard::AudioDeviceDescriptor>
            &deviceDescriptor, const bool &enabled) override;
    };

    class BluetoothSpatialAudioHeadTrackingChangeListener :
        public AudioStandard::AudioHeadTrackingEnabledChangeCallback {
    public:
        explicit BluetoothSpatialAudioHeadTrackingChangeListener() {};
        ~BluetoothSpatialAudioHeadTrackingChangeListener() {};

    private:
        void OnHeadTrackingEnabledChange(const bool &enabled) override {};
        void OnHeadTrackingEnabledChangeForAnyDevice(const std::shared_ptr<AudioStandard::AudioDeviceDescriptor>
            &deviceDescriptor, const bool &enabled) override;
    };

    class BluetoothAudioPreferredOutPutDeviceChangeListener
        : public AudioStandard::AudioPreferredOutputDeviceChangeCallback {
    public:
        explicit BluetoothAudioPreferredOutPutDeviceChangeListener() {};
        ~BluetoothAudioPreferredOutPutDeviceChangeListener() {};
#ifdef AUDIO_FRAMEWORK
        AudioStandard::DeviceType outPutDeviceType_ = AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT;
#endif
 
    private:
        void OnPreferredOutputDeviceUpdated(
            const std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> &deviceDescriptor) override;
    };

    class BluetoothAudioNnStateChangeListener : public AudioStandard::AudioNnStateChangeCallback {
    public:
        explicit BluetoothAudioNnStateChangeListener() {};
        ~BluetoothAudioNnStateChangeListener() {};

    private:
        void OnNnStateChange(const int32_t &nnState) override;
    };

    class BluetoothColAudioEnableListener
        : public AudioStandard::AudioCollaborationEnabledChangeForCurrentDeviceCallback  {
    public:
        explicit BluetoothColAudioEnableListener() {};
        ~BluetoothColAudioEnableListener() {};

    private:
        void OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled) override;
    };

    class BluetoothSpatialAudioAudioSourceTypeChangeListener
        : public AudioStandard::AudioSpatialAudioSourceTypeChangeCallback {
    public:
        explicit BluetoothSpatialAudioAudioSourceTypeChangeListener() {};
        ~BluetoothSpatialAudioAudioSourceTypeChangeListener() {};
    private:
        void OnSpatialAudioSourceTypeChange(const AudioStandard::SpatialAudioSourceType &mode) override;
    };

    class BluetoothAdaptiveSpatialRenderingEnabledChangeListener
        : public AudioStandard::AudioAdaptiveSpatialRenderingEnabledChangeCallback {
    public:
        explicit BluetoothAdaptiveSpatialRenderingEnabledChangeListener() {};
        ~BluetoothAdaptiveSpatialRenderingEnabledChangeListener() {};
        static bool GetAdaptiveSwitchStatus();
        static void SetAdaptiveSwitchStatus(bool status);
    private:
        void OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(
            const std::shared_ptr<AudioStandard::AudioDeviceDescriptor> &deviceDescriptor,
            const bool &enabled) override;
        static bool adaptiveSwitchStatus_;
    };
#endif
public:
    void RegisterAudioVolumeEvent();
    void RegisterAudioFrameworkAdapterListener();
    void UnregisterAudioFrameworkAdapterListener();
    void RegisterOutputPipeChangeListener();
    void UnregisterOutputPipeChangeListener();
    void RegisterRendererDataTransferListener();
    void UnregisterRendererDataTransferListener();
    void GetCurrentOutputPipeInfos();
    void UpdateRouteFlagList(uint32_t flag,
        const std::map<uint32_t, AudioStandard::RendererStreamInfo> &rendererStreams);
    void AddMemberRouteFlagList(const StreamInfo &newInfo);
    void RemoveMemberRouteFlagList(const uint32_t &flag);
    std::vector<StreamInfo> GetRouteFlagList();
    void ClearRouteFlagList();
    bool isCallStreamType(AudioStandard::StreamUsage streamUsage);
    void UpdateRendererState(const std::shared_ptr<AudioStandard::AudioOutputPipeInfo> &changedPipeInfo);
    A2dpRendererState GetA2dpRendererState();
    void UpdateMuteRendererState(const RawAddress &device, const uint32_t streamId, bool isExist);
    void SetA2dpRendererState(A2dpRendererState &state);
    void RegisterSpatialAudioListener();
    void RegisterAudioPreferredOutPutDeviceChangeListener();
    void UnregisterAudioPreferredOutPutDeviceChangeListener();
    void RegisterAudioNnStateEventListener();
    void RegisterCollaborativeAudioListener();
    void UnRegisterCollaborativeAudioListener();
    int32_t SetMusicMuteWhenA2dpRelease();
    int32_t SetMusicUnmute();
    static bool IsMusicActive(StreamType streamType);
    static int32_t BtSetDeviceAbsVolumeSupported(std::string macAddr, bool supported, int32_t volume = VOLUME_MIN);
    static int32_t BtSetA2dpDeviceVolume(std::string macAddr, int32_t storedVolume, bool supported);
    static BtDeviceType BtGetActiveOutputDevice();
    static int32_t BtGetMaxVolume(BtVolumeType volumeType, BtDeviceType deviceType);
    static void HfpSetAudioParameters(const std::string &key, const std::string &value);
    static void HfpSetExtraAudioParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs);
    static void HfpGetExtraAudioParameters(const std::string &mainKey,
        const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result);
    static void SetStreamVolume(int streamType, int volume, int flag);
    static int GetStreamVolume(int streamType);
    static int32_t SetAudioMicrophoneMute(bool isMute);
    static bool IsSpatialAudioModeEnabled(const std::string &macAddr);
    static bool IsSpatialAudioHeadTrackingEnabled(const std::string &macAddr);
    static bool IsAudioOutputToBluetoothA2dp();
    static bool IsAudioOutputToBluetoothSco();
    static void UpdateLocalVoiceCombineFeatureState();
    static void SetActiveDeviceVoiceCombineAbility(const bool &ability);
    static bool IsSpatialAudioAdaptiveSwitchEnabled(const std::string &macAddr);
    static BtAudioScene GetAudioScene();

private:
#ifdef AUDIO_FRAMEWORK
    static constexpr int32_t ALL_UID = -1; // 被监听应用的uid，-1 表示监听所有应用
    static constexpr int32_t MUTE_MAP = (1 << OHOS::AudioStandard::BadDataTransferType::NO_DATA_TRANS) |
        (1 << OHOS::AudioStandard::BadDataTransferType::SILENCE_DATA_TRANS); // 检测静音流，数据为0/音量为0
    static constexpr int64_t CHECK_TIME_INTERVAL = 3000000000; // 静音流检测3s
    static constexpr int32_t BAD_FRAMES_RATIO = 100; // 检测异常占比，当数据全是0才认为是静音流

    std::shared_ptr<AudioStandard::VolumeKeyEventCallback> volumeCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioOutputPipeCallback> audioOutputPipeCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioSpatializationEnabledChangeCallback> spatialAudioModeCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioHeadTrackingEnabledChangeCallback> spatialAudioHeadTrackingCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioPreferredOutputDeviceChangeCallback> audioPreferredOutPutDeviceCallback_ =
        nullptr;
    std::shared_ptr<AudioStandard::AudioNnStateChangeCallback> audioNnStateEventCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioRendererDataTransferStateChangeCallback> audioDataTransferCallback_ = nullptr;
    static const int32_t VOLUME_MIN = 0;
    std::shared_ptr<AudioStandard::AudioCollaborationEnabledChangeForCurrentDeviceCallback>
        collaborativeAudioModeCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioSpatialAudioSourceTypeChangeCallback> spatialAudioSourceTypeCallback_ = nullptr;
    std::shared_ptr<AudioStandard::AudioAdaptiveSpatialRenderingEnabledChangeCallback>
        adaptiveSwitchStatusChangeCallback_ = nullptr;
    std::mutex routeFlagListMutex_;
    std::vector<StreamInfo> routeFlagList_{};
    std::mutex stateMutex_;
    A2dpRendererState rendererState_;
#endif
};
} // namespace bluetooth
} // namespace OHOS

#endif // BLUETOOTH_AUDIO_FRAMEWORK_ADAPTER_H
