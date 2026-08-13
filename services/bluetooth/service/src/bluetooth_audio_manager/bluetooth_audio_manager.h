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

#ifndef BLUETOOTH_AUDIO_MGR_H
#define BLUETOOTH_AUDIO_MGR_H

#include "avrcp/avrcp.h"
#include "av_sync_manager.h"
#include "bluetooth_audio_manager_interface.h"
#include "bluetooth_device.h"
#include <string>
#include "types/raw_address.h"
#include "hiecho.h"
#include "safe_map.h"
#include "securec.h"
#include <vector>
#include <utility>
#include <map>
#include "log.h"

namespace OHOS {
namespace bluetooth {

class BluetoothAudioManager : public IProfileAudioManager {
public:
    static BluetoothAudioManager &GetInstance();
    static inline ::bluetooth::avrcp::PlayStatus playStatus_;
    class PauseRecord {
    public:
        int pauseReason_;
        long pauseTime_;

        PauseRecord() {
        }
        PauseRecord(int reason, long time)
        {
            this->pauseReason_ = reason;
            this->pauseTime_ = time;
        }
    };

    class ProfileStatus {
    public:
        uint8_t a2dpState_;
        uint8_t hfpState_;

        ProfileStatus() {}
        ProfileStatus(uint8_t a2dpState, uint8_t hfpState)
        {
            this->a2dpState_ = a2dpState;
            this->hfpState_ = hfpState;
        }
    };
    class WearingStateRecord {
    public:
            bool leftIn_;
            bool rightIn_;
            WearingStateRecord() {
            }
            WearingStateRecord(bool left, bool right)
            {
                this->leftIn_ = left;
                this->rightIn_ = right;
            }
            void SetNewState(bool left, bool right)
            {
                this->leftIn_ = left;
                this->rightIn_ = right;
            }
    };

    SafeMap<std::string, WearingStateRecord> mAudioManagerRecordMap_;
    SafeMap<std::string, PauseRecord> mPauseRecordMap;
    SafeMap<std::string, ProfileStatus> profileStatusMap_;

    BluetoothAudioManager(const BluetoothAudioManager &) = delete;

    BluetoothAudioManager &operator = (BluetoothAudioManager &) = delete;

    void WearStateHiechoProcess(const RawAddress &addr, int leftState, int rightState);
    void UpdateDeviceWearState(std::shared_ptr<BluetoothDevice> device, bool leftIn, bool rightIn);
    void ChangeActiveForNotWearing(const std::string &addr, bool isLeftIn, bool isRightIn);
    void UpdateDeviceWearStateIfSupport(const std::string &addr, bool isLeftIn, bool isRightIn);
    bool ShouldHandleAsWearAction(const std::string &addr, bool isLeftIn, bool isRightIn);
    bool IsAudioOutputToBluetooth(bool isMusicStream);
    void ChangeActiveDeviceIfNeeded(const std::string &addr, bool leftIn, bool rightIn, bool &isActiveDevWill);
    void ChangePlayStateIfNeeded(const std::string &addr, int action,
        WearingStateRecord &record, bool isDevActive, bool isActiveDevWill);
    int GetPauseReasonNotPlaying(const std::string &addr, int action, WearingStateRecord &record);
    int GetPauseReason(const std::string &addr, int action, WearingStateRecord &record, int currentPlayState);
    void PauseMediaAndRecordIfNeeded(const std::string &addr, int pauseReason, long currentTime);
    static void GetAudioManagerPlayState(::bluetooth::avrcp::PlayStatus status);
    int32_t SetAudioManagerSupport(std::string addr, bool enable);
    int GetAudioManagerSupport(std::string addr);
    void ResumePlayIfNeeded(const std::string &addr, int action, long currentTime /* activePlayer */);
    void ChangeActiveForSupportChange(std::string addr, int support);
    bool IsAudioOutputToBluetooth();
    int32_t GetWearState(std::string addr, int32_t &ability);
    void ClearWearDetectionSupport(std::string addr);
    int SendDeviceSelection(const RawAddress &address, int useA2dp, int useHfp, int userSelection);
    int IsWearDetectionSupported(const RawAddress &address, bool &isSupported);
    bool IsDeviceWearing(const RawAddress &address);
    void RemoveDisconnectDevice(const std::string &addr);
    void DealWithNewPlayingStatus() override;
    void SetProfileStatus(std::string addr, uint8_t a2dpState, uint8_t hfpState);
    bool GetProfileStatus(std::string addr, uint8_t &a2dpState, uint8_t &hfpState);
    void SaveHiechoAvLatency(const RawAddress &rawAddr, int32_t avLatency);
    void ReadLatencyConfig();
    void UpdateBtAudioLatency(const RawAddress &rawAddr);
    void ClearLatencyInfoByAddr(const RawAddress &rawAddr);
    void ClearLatencyInfo();
    void StopOffloadWhenA2dpReleased(const RawAddress &address, int useA2dp);
    void StopAudioRenderListenerWhenA2dpReleased();
    void ProcessAudioRenderListenerWhenUserSelect(int useA2dp);
    void SetSharedPreVirtualAutoConnSwitch(const std::string &address, bool value);
    bool GetSharedPreVirtualAutoConnSwitchValue(const std::string &address);

private:
    static constexpr int WEAR_DOUBLE_EAR = 1;
    static constexpr int WEAR_RIGHT_EAR = 2;
    static constexpr int WEAR_LEFT_EAR = 3;
    static constexpr int WEAR_DOUBLE_DOWN_EAR = 4;
    static constexpr int WEAR_STATUS_IN = 1;
    static constexpr int WEAR_STATUS_INVALID = 0xFF;

    static constexpr int SUPPORT_SYSTEM_ERROR = -2;
    static constexpr int SUPPORT_UNKNOWN = -1;
    static constexpr int SUPPORT_ON = 1;
    static constexpr int SUPPORT_OFF = 0;
  
    static constexpr int WEAR_STATE_FAIL_WRONG_PARA = -3;
    static constexpr int WEAR_STATE_FAIL_SYSTEM_ERROR = -2;
    static constexpr int WEAR_STATE_UNKNOWN = -1;
    static constexpr int WEAR_STATE_IN = 1;
    static constexpr int WEAR_STATE_OUT = 0;

    static constexpr int SET_WEARDETECTION_SUCCESS = 0;
    static constexpr int SET_WEARDETECTION_FAIL = 1;
    static constexpr int ENABLE_WEAR_DETECTION_UNKOWN = -1;

    static constexpr int SIDE_IGNORE = 0;
    static constexpr int SIDE_LEFT = 1;
    static constexpr int SIDE_RIGHT = 2;

    static constexpr int DOUBLE_REMOVE_CHECK_TIME = 2000;
    static constexpr int MAX_RESUME_PLAY_TIME_SIMPLE = 180000L; // 180s

    void NotifyWearStatus(RawAddress &addr, UpdateOutputStackAction action);
    void UnWearActionNotifyHdapDisConnected(const RawAddress &addr, const UpdateOutputStackAction &action);

    int64_t lastSendPlayTime_ = -1L;
    AvSyncManager avSyncManager_ {};

    /**
     * BluetoothAudioManager constructor
     * @param context ApplicationContext
     */
    BluetoothAudioManager() {
    }
};

}
}
#endif

