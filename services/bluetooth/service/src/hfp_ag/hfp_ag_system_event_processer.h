/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef HFP_AG_SYSTEM_EVENT_PROCESS_H
#define HFP_AG_SYSTEM_EVENT_PROCESS_H

#include <string>
#include <set>

#include "hfp_ag_message.h"
#include "hfp_ag_system_interface.h"
#include "hfp_ag_at_phonebook.h"
#include "btcommon/timer_manager.h"

enum AppCategory {
    APP_CATEGORY_UNKNOWN = -1,
    APP_CATEGORY_MUSIC = 7,
    APP_CATEGORY_VIDEO = 8,
    APP_CATEGORY_GAME = 9,
};

namespace OHOS {
namespace bluetooth {
/**
 * @brief This class provides a set of methods that is used to process system event.
 */
class HfpAgSystemEventProcesser {
public:
    /**
     * @brief Construct a new HfpAgSystemEventProcesser object.
     *
     * @param profile AG profile instance.
     * @param address Remote device address.
     */
    explicit HfpAgSystemEventProcesser(const std::string &address);

    /**
     * @brief Destroy the HfpAgSystemEventProcesser object.
     */
    ~HfpAgSystemEventProcesser() = default;

    /**
     * @brief Process system event.
     *
     * @param event The system event.
     */
    void ExecuteEventProcess(const HfpAgMessage &event);

    /**
     * @brief Start the system event processor.
     */
    static void Start();

    /**
     * @brief Stop the system event processor.
     */
    static void Stop();

    void ProcessPhoneStateChange(const HfpAgPhoneState &phoneState);

    void ProcessAtResponseCodeEvent(int responseCode, int errorCode) const;

    void NotifyDeviceStatusChangedEvent();

    void ProfcessDialingOutResultEvent(int result);

    void SendBSIRValueEvent(int action);

    void ProcessScoVolumeChangedEvent(int volumeType, int volumeValue);

    void ProcessScoVolumeChangedWithoutSendAt(int volumeType, int volumeValue);

    void RecoverScoVolume();

    void ProcessClccResponseCmd(HfpAgCallList callList) const;

    void ProcessA2dpSuspendStateEvent(bool a2dpSuspendState) const;

    void ProcessPhonebookAuthResultEvent(const HfpAgMessage &event);

    bool ProcessVendorSpecificAt(const std::string &atCommand, const std::string &address);

    void SetAudioParameters(const std::string &key);

    void SetAudioExtraParametersForSwbOn();

    void UpdateDeviceNameAndTimestamp();

    void WaitScoState();
    void WaitScoStateTimeOut();

    bool IsDeviceSupportHwAt(const std::string& addr);
    void SendAtCmdOnApptypeChange(AppCategory topApptype);
    void RemoveTask();

    using HfpAgSystemEvent_ = void (HfpAgSystemEventProcesser::*)();
    std::map<std::string, HfpAgSystemEvent_> bandFeatures_ = {
        {HW_TALKBAND_BAND_MODE, &HfpAgSystemEventProcesser::DoHwTalkbandBandMode},
        {HW_TALKBAND_HEADSET_MODE, &HfpAgSystemEventProcesser::DoHwTalkbandHandSetMode},
        {TAKE_PHOTOS_AT, &HfpAgSystemEventProcesser::DoHwCameraTakePicture},
        {SEARCH_PHONE_AT, &HfpAgSystemEventProcesser::DoFindPhoneAT},
        {HW_MUTE_PHONE_AT, &HfpAgSystemEventProcesser::DoHwMutePhoneAT},
        {HW_ACTIVE_ASSISTANT, &HfpAgSystemEventProcesser::DoHwActiveAssistant},
        {HW_TALKBAND_DRIVE_MODE, &HfpAgSystemEventProcesser::DoHwTalkbandDeiveMode},
        {HW_VR_DEVICE_OUT_MODE, &HfpAgSystemEventProcesser::DoHwVrDeviceOutMode},
        {HW_VR_DEVICE_IN_MODE, &HfpAgSystemEventProcesser::DoHwVrDeviceInMode},
        {HW_PHONE_HOLDER, &HfpAgSystemEventProcesser::DoHwPhoneHolder}};

    std::unique_ptr<utility::Timer> ResponseClccTimeout_ {nullptr};

private:
    /**
     * @brief Band work mode. 1,0: band mode.
     */
    void DoHwTalkbandBandMode();

    /**
     * @brief Band work mode. 1,1: headset mode.
     */
    void DoHwTalkbandHandSetMode();

    /**
     * @brief Status of the photographing function. 3,2: photographing enabled.
     */
    void DoHwCameraTakePicture();

    /**
     * @brief Mobile phone search result. 4,0: The mobile phone is found.
     */
    void DoFindPhoneAT();

    /**
     * @brief Incoming calls, phone ringtone, and band vibration notification; Phone stops ringing/vibrating. 5,0.
     */
    void DoHwMutePhoneAT();

    /**
     * @brief Wakes up a specific voice assistant. 6,0: Hw voice assistant.
     */
    void DoHwActiveAssistant();

    /**
     * @brief Wake App. 9,0: Driving Mode.
     */
    void DoHwTalkbandDeiveMode();

    /**
     * @brief VR and vehicle-mounted applications. 10,0: Disable the driving scenario.
     */
    void DoHwVrDeviceOutMode();

    /**
     * @brief Wake App. 9,0: Driving Mode. 10,1: The driving scenario is enabled.
     */
    void DoHwVrDeviceInMode();

    /**
     * @brief Wake App. 9,0: Driving Mode. 10,2: Switch the device to PHONE_HOLDER and lower the audio output priority.
     */
    void DoHwPhoneHolder();

    /**
     * @brief Process the event of answering call.
     */
    void ProcessAnswerCallEvent() const;

    /**
     * @brief Process the event of hanging up call.
     */
    void ProcessHangupCallEvent() const;

    /**
     * @brief Process the event of rejecting call.
     */
    void ProcessRejectCallEvent();

    /**
     * @briefProcess the event of the hf volume changed.
     *
     * @param type The volume type.
     * @param volume The volume valume.
     */
    void ProcessHfVolumeChangedEvent(int type, int volume);

    /**
     * @brief Process the event of dialing out call.
     *
     * @param number The number of the call.
     */
    void ProcessDialOutCallEvent(const std::string &number);

    /**
     * @brief Process the event of sending the DTMF.
     *
     * @param dtmf The code of the DTMF.
     */
    void ProcessSendDtmfEvent(int dtmf) const;

    /**
     * @brief Process the event of noise reduction.
     *
     * @param flag Enable/disable flag.
     */
    void ProcessNoiseReductionEvent(int flag);

    /**
     * @brief Process the event of the wide band speech.
     *
     * @param codec The code of the wide band speech.
     */
    void ProcessWideBandSpeechEvent(int codec);

    /**
     * @brief Process the event of holding the call.
     *
     * @param chld The value of the CHLD command.
     */
    void ProcessHoldCallEvent(int chld) const;

    /**
     * @brief Get the Subscriber Number of the phone.
     */
    void GetSubscriberNumber() const;

    /**
     * @brief Get the Ag indicator.
     */
    void GetAgIndicator();

    /**
     * @brief Get the network operator of the phone.
     */
    void GetNetworkOperator() const;

    /**
     * @brief ProcessClccEvent.
     */
    void ProcessClccEvent();

    /**
     * @brief Process the event of the AT unknown command.
     *
     * @param atString The string of the command.
     */
    void ProcessAtUnknownEvent(const std::string &atString, const std::string &address);

    /**
     * @brief Process the event of the key pressed.
     */
    void ProcessKeyPressedEvent() const;

    /**
     * @brief Process the event of the AT bind.
     *
     * @param atString The string of the command.
     */
    void ProcessATBindEvent(const std::string &atString) const;

    /**
     * @brief Process the event of sinding the hf indicator.
     *
     * @param indId The id of the indicator.
     * @param indValue The value of the indicator.
     */
    void SendHfIndicator(int indId, int indValue) const;

    /**
     * @brief Process the event of the AT BIA.
     *
     * @param data The BIA event.
     */
    void ProcessAtBiaEvent(const HfpAgMessage &event);

    /**
     * @brief process the event of querying AG indicator.
     */
    void QueryAgIndicator();

    /**
     * @brief The status of the voice recognition change.
     *
     * @param status The status of the voice recognition.
     */
    void VoiceRecognitionStateChanged(int status);

    /**
     * @brief Get the control other modules event type.
     *
     * @param type Event type number.
     * @return Returns the event type string name.
     */
    static std::string GetEventType(int type);

    /**
     * @brief Get the control other modules event type.
     *
     * @param type Event type number.
     * @return Returns the event type string name.
     */
    static std::string GetOtherEventType(int type);

    /**
     * @brief Process the event of the phone AT command.
     *
     * @param atCommand The string of the command.
     */
    bool ProcessPhoneATCommand(const std::string atCommand) const;

    void ProcessAtCindCmdEvent(int call, int callsetup, int callheld);

    void ProcessAtResponseStringEvent(const std::string &response) const;

    int CovertCallStateToBluetdroid(int callState);

    std::string GetCallState(int state);

    void ExecuteOthersEvent(const HfpAgMessage &event);

    int GetPhoneNumberType(std::string phoneNumber) const;

    HfpAgCallList CreateCallList(int index, int state, std::string phoneNumber, int type) const;

    void ProcessRemoteBatteryLevel(const std::string &arg) const;

    std::vector<int> StringSplit(const std::string& str, char delim) const;

    void ProcessBccEvent();

    void ProcessBrsfEvent(int features);

    bool ProcessAtPhonebookCommand(const std::string &atCommand);

    bool ProcessHwPrivateCommand(const std::string &atCommandPara, const std::string &atCommandValue,
        const std::string &address);
    bool ProcessTalkBandSetFeatureCommand(const std::string &atArgument) const;
    bool ProcessTalkBandSetRequestCommand(const std::string &atArgument, const std::string &address);
    void ProcessWearDetection(const std::string &atArgument, const std::string &address) const;
    void UpdateTalkbandHandSetMode();
    bool ProcessBatteryAtCommand(const std::string &atCommand) const;
    bool ProcessHwBatteryAtCommand(const std::string &atCommand) const;
    bool IsSupportedAtCommand(const std::string &atCommand) const;
    bool ProcessXaplAtCommand(const std::string &atCommand) const;
    bool ProcessHwDeviceInfo(const std::string &atArgument);
    void StartResponseClccTimer();
    void ResponseClccTimeOut();
    void NotifyStopVirtualCall() const;
    void SetScoVolume();
    void UpdateHwDeviceInfo(const std::string &deviceName, const std::string &deviceTimeStamp);
    void AddEnableHwAtAddr(const std::string addr);

    inline int32_t GetDefaultVolume()
    {
        if (hfpMaxVolume_ < HFP_DEFAULT_VOLUME) {
            return HFP_DEFAULT_VOLUME;
        }
        return hfpMaxVolume_ / HFP_DEFAULT_VOLUME_DIVISOR;
    }

    std::map<std::string, std::string> audioParameterMap {};

    // The quote of the HfpAgSystemInterface class.
    HfpAgSystemInterface &systemInterface_ {HfpAgSystemInterface::GetInstance()};

    // The address of the bluetooth device.
    std::string address_ {""};
    std::string deviceTimestamp_ {""};

    int32_t hfpMaxVolume_;
    int32_t speakerVolume_ {HFP_INVALID_VOLUME};
    HfpAgAtPhonebook atPhonebook_;

    std::shared_ptr<utility::Timer> handSetDelayTimer_ {};
    std::shared_ptr<utility::Timer> waitScoStateTimeOut_ {};

    // The time of response clcc timeout
    inline static constexpr int RESPONSE_CLCC_TIMEOUT_TIME {5000};
    std::set<std::string> enableHwAtAddrSet_ {};
};
}  // namespace bluetooth
}
#endif // HFP_AG_SYSTEM_EVENT_PROCESS_H
