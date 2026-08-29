/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic bluetooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 * @since 6
 */

/**
 * @file hfp_ag_service.h
 *
 * @brief Declares HFP AG role service functions, including basic and observer functions.
 *
 * @since 6
 */

#ifndef HFP_AG_SERVICE_H
#define HFP_AG_SERVICE_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "base_def.h"
#include "base_observer_list.h"
#include "bluetooth.h"
#include "bluetooth_headset_callbacks.h"
#include "bluetooth_headset_interface.h"
#include "bluetooth_phone_state.h"
#include "bt_config.h"
#include "bt_hf.h"
#include "context.h"
#include "hfp_ag_defines.h"
#include "hfp_ag_message.h"
#include "hfp_ag_statemachine.h"
#include "interface_profile_hfp_ag.h"
#include "btcommon/message.h"
#include "raw_address.h"
#include "service_util.h"
#include "btcommon/timer_manager.h"
#include "call_statemachine.h"
#include "bt_recursive_mutex.h"
#include "util/safe_vector.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief This HFP class provides a set of methods that is called by Framework API, and manage
 *        the state machine and the message queue.
 *
 * @since 6
 */
class HfpAgService : public IProfileHfpAg, public utility::Context {
public:
    /**
     * @brief Construct a new HfpAgService object.
     *
     * @since 6
     */
    HfpAgService();

    /**
     * @brief Destroy the HfpAgService object.
     *
     * @since 6
     */
    ~HfpAgService() override;

    /**
     * @brief Get the instance of the HfpAgService object.
     *
     * @return Returns the instance of the HfpAgService object.
     * @since 6
     */
    static HfpAgService *GetService();

    /**
     * @brief Get the Context object.
     *
     * @return Returns the Context object.
     * @since 6
     */
    utility::Context *GetContext() override;

    /**
     * @brief Enable the target service.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    void Enable() override;

    /**
     * @brief Disable the target service.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    void Disable() override;

    /**
     * @brief Finish the shut down service.
     *
     * @since 6
     */
    void ShutDownDone(bool isAllDisconnected);

    /**
     * @brief Stop and null dialingOutTimeout_ pointer.
     *
     * @since 6
     */
    void StopDialingTimer();

    /**
     * @brief Establish the service level connection.
     *
     * @param device The remote device
     * @return Returns the connect result code.
     * @since 6
     */
    int Connect(const RawAddress &device) override;

    /**
     * @brief Release the service level connection.
     *
     * @param device The remote device
     * @return Returns the disconnect result code.
     * @since 6
     */
    int Disconnect(const RawAddress &device) override;

    /**
     * @brief Get the connected devices list.
     *
     * @return Returns the connected devices list.
     * @since 6
     */
    std::list<RawAddress> GetConnectDevices() override;

    /**
     * @brief Get the connection state.
     *
     * @return Returns the connection state.
     * @since 6
     */
    int GetConnectState() override;

    /**
     * @brief Get the state by the device.
     *
     * @param device The remote device.
     * @return Returns the connection state.
     * @since 6
     */
    int GetDeviceState(const RawAddress &device) override;

    /**
     * @brief Get the maximum number of connected devices.
     *
     * @return Returns the max connected devices number.
     * @since 6
     */
    int GetMaxConnectNum() override;

    /**
     * @brief Check whether device is connected.
     *
     * @param address: The remote device address.
     * @return Returns <b>true</b> if the is connected; returns <b>false</b> if not connected.
     * @since 6
     */
    bool IsConnected(const std::string &address);

    /**
     * @brief Get the devices list of the specified states.
     *
     * @param states The specified states.
     * @return Returns the remote devices list of the specified states.
     * @since 6
     */
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) override;

    /**
     * @brief Initiate the establishment of an audio connection to remote active HF device.
     *
     * @param callType the type of the call, Refer to enum BTCallType.
     * @return Returns <b>0</b> if the operation is successful; returns <b>other</b> if the operation fails.
     * @since 10
     */
    int32_t ConnectSco(uint8_t callType) override;

    /**
     * @brief Release the audio connection from remote active HF device.
     *
     * @return Returns <b>0</b> if the operation is successful; returns <b>other</b> if the operation fails.
     * @since 10
     */
    int32_t DisconnectSco(uint8_t callType) override;

    /**
     * @brief Establish the audio connection.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool ConnectSco() override;

    /**
     * @brief Release the audio connection.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool DisconnectSco() override;

    /**
     * @brief Get the audio state of the specified device.
     *
     * @param device The remote device.
     * @return Returns the audio state.
     * @since 6
     */
    int GetScoState(const RawAddress &device) override;

    /**
     * @brief This function used to Update changed phone call information.
     *
     * @param phoneState Bluetooth phone state.
     * @since 6
     */
    void PhoneStateChanged(Bluetooth::BluetoothPhoneState &phoneState, bool isVirtualCall = false) override;

    /**
     * @brief Send response for querying standard list current calls by remote Hf device.
     *
     * @param index Index of the current call.
     * @param direction Direction of the current call.
     * @param status Status of the current call.
     * @param mode Source Mode of the current call.
     * @param mpty Is this call a member of a conference call.
     * @param number Phone call number.
     * @param type Type of phone call number.
     * @since 6
     */
    void ClccResponse(
        int index, int direction, int status, int mode, bool mpty, const std::string &number, int type) override;

    /**
     * @brief Open the voice recognition.
     *
     * @param device The Remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool OpenVoiceRecognition(const RawAddress &device) override;

    /**
     * @brief Close the voice recognition.
     *
     * @param device The Remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool CloseVoiceRecognition(const RawAddress &device) override;

    bool IsAudioConnected() override;

    /**
     * @brief Set the active device for audio connection.
     *
     * @param device The remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetActiveDevice(const RawAddress &device) override;

    bool IntoMock(int state) override;
    bool SendNoCarrier(const RawAddress &device) override;

    /**
     * @brief Get the active device.
     *
     * @param device The remote active device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    std::string GetActiveDevice() override;

    /**
     * @brief Notify the slc state change.
     *
     * @param device The remote device.
     * @param toState The current state.
     * @since 6
     */
    void NotifySlcStateChanged(const RawAddress &device, int toState);

    /**
     * @brief Notify the audio state change.
     *
     * @param device The remote device.
     * @param toState The current state.
     * @param reason Reason for change status.
     * @since 6
     */
    void NotifyAudioStateChanged(const RawAddress &device, int toState, int reason = 0);

    /**
     * @brief Notify the active device change.
     *
     * @param device The active device.
     * @since 6
     */
    void NotifyCurrentActiveDevice(const RawAddress &device);

    /**
     * @brief Notify the value of the HF enhanced driver safety.
     *
     * @param device The bluetooth device address.
     * @param indValue The value of the HF enhanced driver safety.
     * @since 6
     */
    void NotifyHfEnhancedDriverSafety(const RawAddress &device, int indValue);

    /**
     * @brief Register HfpAgServiceObserver instance.
     *
     * @param observer HfpAgServiceObserver instance.
     * @since 6
     */
    void RegisterObserver(HfpAgServiceObserver &observer) override;

    /**
     * @brief Deregister HfpAgServiceObserver instance.
     *
     * @param observer HfpAgServiceObserver instance.
     * @since 6
     */
    void DeregisterObserver(HfpAgServiceObserver &observer) override;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 10.0
     */
    int SetConnectStrategy(const RawAddress &device, int strategy) override;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 10.0
     */
    int GetConnectStrategy(const RawAddress &device) override;

    /**
     * @brief Notify audio volume info.
     * @param[in] streamType The stream type.
     * @param[in] volume The device volume.
     */
    void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume) override;

    /**
     * @brief Send the event of the HFP AG role.
     *
     * @param event The event of the HFP AG role.
     * @since 6
     */
    void PostEvent(const HfpAgMessage &event);

    /**
     * @brief Process the event of the HFP AG role.
     *
     * @param event The event of the HFP AG role.
     * @since 6
     */
    void ProcessEvent(const HfpAgMessage &event);

    /**
     * @brief Notify the ag indicator state changed.
     *
     * @param what The name of the event.
     * @param state the service state of the ag indicator
     * @since 6
     */
    void NotifyAgIndicatorStateChanged(int what, const HfpAgTransferData &data);

    /**
     * @brief Dial out call by HF.
     *
     * @param address The remote device address.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool DialOutCallByHf(const std::string &address);

    /**
     * @brief Dialing out call time out.
     *
     * @since 6
     */
    void DialOutCallTimeOut();

    /**
     * @brief Open the voice recognition by HF.
     *
     * @param address The remote device address.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool OpenVoiceRecognitionByHf(const std::string &address);

    /**
     * @brief Close the voice recognition by HF.
     *
     * @param address The remote device address.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool CloseVoiceRecognitionByHf(const std::string &address);

    /**
     * @brief Voice recognition time out.
     *
     * @since 6
     */
    void VoiceRecognitionTimeOut();

    /**
     * @brief Responsing CLCC time out.
     *
     * @since 6
     */
    void ResponseClccTimeOut();

    /**
     * @brief Process response CLCC time out event of the HFP AG role.
     *
     * @param event The response CLCC time out event.
     * @since 6
     */
    void ProcessResponseClccTimeOut(const HfpAgMessage &event);

    /**
     * @brief Process response dial time out event of the HFP AG role.
     *
     * @since 6
     */
    void ProcessDialOutTimeOut();

    /**
     * @brief Open the voice recognition internal
     *
     * @param device The Remote device address.
     * @param isRequestByHf true if VR request by hf, else false.
     * @since 6
     */
    void SendOpenVoiceEvent(const std::string &address, bool isRequestByHf);

    /**
     * @brief Close the voice recognition internal
     *
     * @param device The Remote device address.
     * @since 6
     */
    void SendCloseVoiceEvent(const std::string &address, bool disconnectSco = true);

    /**
     * @brief Service level connection status change.
     *
     * @param address The remote device address.
     * @param toState The current state.
     * @since 6
     */
    void SlcStateChanged(const std::string &address, int toState);

    /**
     * @brief Audio State change.
     *
     * @param address The remote device address.
     * @param fromState The previous state.
     * @param toState The current state.
     * @since 6
     */
    void ScoStateChanged(const std::string &address, int toState);

    /**
     * @brief Set in-band ring.
     * @param action If <b>true</b> set in-band ring enabled, else disabled.
     * @since 6
     */
    void SetInbandRing(bool action);

    /**
     * @brief Process remove state machine event of the HFP AG role.
     *
     * @param address The remote device address.
     * @since 6
     */
    void ProcessRemoveStateMachine(const std::string &address);

    /**
     * @brief query is Sco should acceptable when state from conneected to audio connected.
     *
     * @param address The remote device address.
     * @since 6
     */
    bool IsScoAcceptableExceptADevice(const std::string &address);

    void ProcessConnectEvent(const HfpAgMessage &event);

    ::bluetooth::headset::Interface* getBluetoothHfpInterface() const;

    bool IsAudioConnected(const std::string &address);

    bool IsVirtualCallStarted();

    /**
     * @brief Check whether in-band-ringing is enabled
     *
     * @param isEnabled whether in-band-ringing is enabled
     * @return Returns {@link BT_NO_ERROR} if the operation fails.
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 11
     */
    bool IsInbandRingingEnabled() override;

    int IsVgsSupported(const RawAddress &device, bool &isSupported) override;

    /**
     * @brief Notify the hfp stack changed.
     *
     * @since 10
     */
    void NotifyHfpStackChanged(int action, const RawAddress &device);

    /**
     * Enable or disable the bluetooth log.
     *
     * @param state true: Enable the log. false: Disable the log.
     * @since 11
     */
    void EnableBtCallLog(bool state) override;

    bool GetRemoteDeviceConnectStatus(const RawAddress &device, int &state);

    void NotifyWearDetectionActionAfterConnected(const RawAddress &device);

    void NotifyProfileState(const RawAddress &device);

    void CallDetailsChanged(int callId, int callState) override;

    int GetVirtualCallActiveNum(void);
    int GetVirtualCallState(void);
    int GetConnectedDeviceNum();
    void ClearCallStateMachine(void);
    void StopA2dpOffload();

    void NotifyPhonebookAuthResult(const RawAddress& device, int32_t accessAuthorization,
        int32_t pbapLoadFlag) override;

    bool ShouldCallAudioBeActive();
    void HandleA2dpStateAfterCall(int callState, bool shouldActiveBefore);
    void HandleCallStateChange(OHOS::Bluetooth::BluetoothPhoneState &phoneState, bool isVirtualCall);
    void HandleBccBlockTimer(bool shouldStart);
    void SendCallStateEvent(bool isDelay, int64_t delayTime, const HfpAgMessage &curEvent,
        const std::string &taskName);
    void HandleRealCallStateChange(int numActive, int callState, bool isDelayCiev, bool isDelayCallState,
        const HfpAgMessage &curEvent);
    void ProcessHwDeviceInfoCmd(const std::string &addr, const std::string &commandStr);
    void SendBsirValue();
    void SendBsirToLastRemainingDevice();
    std::list<RawAddress> GetConnectingOrConnectionDevices();
    int GetDevicePreState(const RawAddress &device) const;

    bool IsNoNeedConnectSco();
    void SetResumeDevice(std::string activeAddr);
    std::string GetResumeDevice();
    void CheckAndResumeSco(int numActive, int numHeld, int callState);

    void UpdateVirtualDevice(int32_t action, const std::string &address) override;
    void GetVirtualDeviceList(std::vector<std::string> &devices) override;
    bool IsInActualcall();
    void ProcessChrAgConnectionState(const RawAddress rawAddr, const int connectState);

    bool IsInCloudBondingState(const RawAddress &device) const;
    bool ProcCloudDeviceConnect(const RawAddress &device);
    void SetAudioParameters() const;
    int GetCurrentCallType(int &callType) override;
    int IsVoiceRecognitionSupported(const RawAddress &device, bool &isSupported) override;
    void SendAtCmdByTopApptype(AppCategory topApptype);
    bool IsBccBlockTimerActive() const;
private:

    class HfpAgServiceCallbacks : ::bluetooth::headset::Callbacks {
    public:
        void ConnectionStateCallback(::bluetooth::headset::BthfConnectionState state,
            OHOS::bluetooth::RawAddress* bdAddr) override;
        void AudioStateCallback(::bluetooth::headset::BthfAudioState state,
            OHOS::bluetooth::RawAddress* bdAddr) override;
        void VoiceRecognitionCallback(::bluetooth::headset::BthfVrState state,
            OHOS::bluetooth::RawAddress* bdAddr) override;
        void AnswerCallCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void HangupCallCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void VolumeControlCallback(::bluetooth::headset::BthfVolumeType type,
            int volume, OHOS::bluetooth::RawAddress* bdAddr) override;
        void DialCallCallback(char* number, OHOS::bluetooth::RawAddress* bdAddr) override;
        void DtmfCmdCallback(char dtmf, OHOS::bluetooth::RawAddress* bdAddr) override;
        void NoiseReductionCallback(::bluetooth::headset::BthfNrec nrec, OHOS::bluetooth::RawAddress* bdAddr) override;
        void WbsCallback(::bluetooth::headset::BthfWbsConfig wbsConfig, OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtChldCallback(::bluetooth::headset::BthfChldType chld, OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtCnumCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtCindCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtCopsCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtClccCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void UnknownAtCallback(char* atString, OHOS::bluetooth::RawAddress* bdAddr) override;
        void KeyPressedCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtBindCallback(char* atString, OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtBievCallback(::bluetooth::headset::BthfHfIndType indId, int indValue,
            OHOS::bluetooth::RawAddress* bdAddr) override;
        void AtBiaCallback(bool service, bool roam, bool signal, bool battery,
            OHOS::bluetooth::RawAddress* bdAddr) override;
#ifdef BLUETOOTH_SCO_NORMALIZED_FEATURE_ENABLE
        void AtBccCallback(OHOS::bluetooth::RawAddress* bdAddr) override;
#endif
#ifdef COMMUNICATION_L2
        void AtBrsfCallback(uint32_t features, OHOS::bluetooth::RawAddress* bdAddr) override;
#endif
    };

    /**
     * @brief Service startup.
     *
     * @since 6
     */
    void StartUp();
    void ShutDown();
    int GetMaxConnectionDevicesNum() const;
    void ProcessDefaultEvent(const HfpAgMessage &event);
    int GetProcessingDeviceNum();
    static int CovertConnectStateFromStack(::bluetooth::headset::BthfConnectionState state);
    static int CovertAudioStateFromStack(::bluetooth::headset::BthfAudioState state);
    static int CovertVRStateFromStack(::bluetooth::headset::BthfVrState state);
    static int CovertVolumeControlTypeFromStack(::bluetooth::headset::BthfVolumeType type);
    static bool CovertNoiseReductionFromStack(::bluetooth::headset::BthfNrec nrec);
    static int CovertBievValueFromStack(::bluetooth::headset::BthfHfIndType indId);
    static int ConvetWbsConfigFromStack(::bluetooth::headset::BthfWbsConfig wbsConfig);
    void SendEventToEachStateMachine(const HfpAgMessage &event);
    void postHfpConnectEvent(bool isReachMaxConnect, const std::string address);
    bool IsActiveDevice(const std::string &address);
    bool IsAudioIdle() const;
    bool IsIncall();
    bool DisconnectSingleSco(const std::string &address);
    void ClearActiveDevice();
    bool UpdateActiveDevice(const std::string &newAddress);
    bool SetActiveDeviceToStack(const RawAddress &device);
    void RegisterAudioVolumeCallBack();
    void ConnectionStateCallbackInner(RawAddress rawAddr, int state);
    /**
     * @brief check is accept connection
     *
     * @return returns true if accept connection
     */
    bool IsAcceptConnection(RawAddress &rawAddr, int state);

    /**
     * @brief Check if voice recognition available.
     *
     * @param address Open voice recognition device address.
     * @return Returns if voice recognition should to be open.
     * @since 6
     */
    bool IsVoiceRecognitionAvailable(const std::string &address);
    /**
     * @brief Process voice recognition time out event of the HFP AG role.
     *
     * @param event The voice recognition time out event.
     * @since 6
     */
    void ProcessVoiceRecognitionTimeOut(const HfpAgMessage &event);

    bool IsRemoteHfpSupported(const RawAddress &device);

    int32_t StartCellularCallSco();

    int32_t IsAcceptCellularCall();

    int32_t StopCellularCallSco();

    bool DisconnectCsCallSco(std::string address);

    int32_t StartVirtualCallSco();

    int32_t IsAcceptVirtualCall();

    int32_t StopVirtualCallSco();

    void ConnectVirtualCallSco();

    void UpdateVirtualDeviceByHfpConnectState(const RawAddress& device, int state);

    void NotifyConnStateChangedInner(const RawAddress &device, int state, int cause);

    void BluetoothNotifyHfpStackChanged(int action, const RawAddress &device);

    int WakeUpVoiceRecognition(const std::string &address);

    void WaitForStopVirtualCall();

    void WaitForScoDisconnected();

    void UpdateLocalVoiceCombineStateForScoStateChanged(const std::string &address, const bool isCall);

    void ClearVirtualCallInfo(const RawAddress &device);

    void StopVoiceRecognitionToStack(const std::string &address);

    void ReportStackScoStateChangeUeEvent(const RawAddress &device, int reason);

    // service status
    bool isStarted_ {false};
    // service status
    bool isShuttingDown_ {false};
    // The maximum default number of connection devices.
    inline static constexpr int MAX_DEFAULT_CONNECTIONS_NUM {BLUETOOTH_MAX_DEFAULT_CONNECTIONS_NUM};
    // The maximum number of connected devices.
    int maxConnectedNum_ {MAX_DEFAULT_CONNECTIONS_NUM};

    // The time of dialing out timeout
    inline static constexpr int DIALING_OUT_TIMEOUT_TIME {10000};

    // The time of voice recognition timeout
    inline static constexpr int VOICE_RECOGNITION_TIMEOUT_TIME {5000};

    // The empty address
    inline static const std::string NULL_ADDRESS {""};

    inline static const std::string EMPTY_ADDRESS {"00:00:00:00:00:00"};

    // The mutex variable
    mutable BtRecursiveMutex mutex_ {};

    // The flag for Voice recognition opened.
    bool isVrOpened_ {false};

    bool isVirtualCallStarted_ {false};

    // virtual device address list
    utility::SafeVector<std::string> virtualDevices_ = {};

    // The map of the device and sate machine.
    std::map<const std::string, std::unique_ptr<HfpAgStateMachine>> stateMachines_ {};

    // The list of the observer.
    BaseObserverList<HfpAgServiceObserver> observers_ {};

    // dialing out time out.
    std::unique_ptr<utility::Timer> dialingOutTimeout_ {nullptr};

    // voice recognition time out.
    std::unique_ptr<utility::Timer> voiceRecognitionTimeout_ {nullptr};

    // BCC block timer after call connected
    std::unique_ptr<utility::Timer> bccBlockTimer_ {nullptr};
    std::atomic_bool isBccBlockTimerActive_ {false};
    void BccBlockTimerTimeout();
    inline static constexpr int BCC_BLOCK_TIME = 3500;

    ::bluetooth::headset::Interface* bluetoothHfpInterface = nullptr;

    uint32_t localFeatures {HFP_AG_FEATURES_DEFAULT};

    std::unique_ptr<HfpAgServiceCallbacks> hfpAgServiceCallback_ = nullptr;

    std::unique_ptr<CallStateMachine> callStateMachine_ {nullptr};

    // The status of the inband ringing.
    bool isInbandRinging_ {false};

    // active device address
    std::string activeAddr_ {};

    std::string resumeDevice_ {};

    std::mutex resumeScoMutex_ {};

    std::mutex syncMutex_ {};

    std::mutex callstatemachineMutex_ {};

    std::condition_variable condition_;

    std::atomic_bool isAllowConnectSco_ {false};

    std::mutex scoSyncMutex_ {};

    mutable BtRecursiveMutex bccBlockTimerMutex_ {};

    std::condition_variable scoCondition_;

    int currentCallType_ {-1};

    // const state map
    const std::map<const int, const int> stateMap_ = {
        {HFP_AG_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {HFP_AG_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {HFP_AG_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {HFP_AG_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };

    BT_DISALLOW_COPY_AND_ASSIGN(HfpAgService);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // HFP_AG_SERVICE_H