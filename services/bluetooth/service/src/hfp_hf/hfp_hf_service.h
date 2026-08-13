/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
 * @since 12
 */

/**
 * @file hfp_hf_service.h
 *
 * @brief Declares HFP AG role service functions, including basic and observer functions.
 *
 * @since 12
 */

#ifndef HFP_HF_SERVICE_H
#define HFP_HF_SERVICE_H

#include <optional>
#include <atomic>

#include "context.h"
#include "interface_profile_hfp_hf.h"
#include "btcommon/message.h"
#include "raw_address.h"
#include "bt_hf_client.h"
#include "service_util.h"
#include "hfp_hf_statemachine.h"
#include "hfp_hf_message.h"
#include "util/safe_vector.h"
#include "bt_recursive_mutex.h"
#include "base_observer_list.h"
#include "i_call_status_callback.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief This HFP class provides a set of methods that is called by Framework API, and manage
 *        the state machine and the message queue.
 *
 * @since 12
 */
class HfpHfService : public IProfileHfpHf, public utility::Context {
public:
    /**
     * @brief Get the instance of the HfpHfService object.
     *
     * @return Returns the instance of the HfpHfService object.
     * @since 12
     */
    static HfpHfService *GetService();

    /**
     * @brief Get the cause of state.
     *
     * @return Returns the cause of connection changed.
     * @since 12
     */
    static ConnChangeCause ConvertStateToCause(int state);

    /**
     * @brief Construct a new HfpHfService object.
     *
     * @since 12
     */
    HfpHfService();

    /**
     * @brief Destroy the HfpHfService object.
     *
     * @since 12
     */
    ~HfpHfService() override;

    /**
     * @brief Get the instance of the HfpAgService object.
     *
     * @return Returns the instance of the HfpAgService object.
     * @since 12
     */
    utility::Context *GetContext() override;

    /**
     * @brief Enable the target service.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    void Enable() override;

    /**
     * @brief Disable the target service.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    void Disable() override;

    /**
     * @brief Finish the shut down service.
     *
     * @since 12
     */
    void ShutDownDone(bool isAllDisconnected);

    /**
     * @brief Establish the service level connection.
     *
     * @param device The remote device.
     * @return Returns the connect result code.
     * @since 12
     */
    int Connect(const RawAddress &device) override;

    /**
     * @brief Release the service level connection.
     *
     * @param device The remote device.
     * @return Returns the disconnect result code.
     * @since 12
     */
    int Disconnect(const RawAddress &device) override;

    /**
     * @brief Get the connected devices list.
     *
     * @return Returns the connected devices list.
     * @since 12
     */
    std::list<RawAddress> GetConnectDevices() override;

    /**
     * @brief Get the connection state.
     *
     * @return Returns the connection state.
     * @since 12
     */
    int GetConnectState() override;

    /**
     * @brief Get the maximum number of connected devices.
     *
     * @return Returns the max connected devices number.
     * @since 12
     */
    int GetMaxConnectNum() override;

    /**
     * @brief Establish the audio connection.
     *
     * @param device The remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool ConnectSco(const RawAddress &device) override;

    /**
     * @brief Release the audio connection.
     *
     * @param device The remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool DisconnectSco(const RawAddress &device) override;

    /**
     * @brief Get the devices list of the specified states.
     *
     * @param states The specified states.
     * @return Returns the remote devices list of the specified states.
     * @since 12
     */
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) override;

    /**
     * @brief Get the state by the device.
     *
     * @param device: The remote device.
     * @return Returns the connection state.
     * @since 12
     */
    int GetDeviceState(const RawAddress &device) override;

    /**
     * @brief Get the audio state of the specified device.
     *
     * @param device The remote device.
     * @return Returns the audio state.
     * @since 12
     */
    int GetScoState(const RawAddress &device) override;

    /**
     * @brief Send DTMF tone code to remote AG device.
     *
     * @param device Remote device object.
     * @param code DTMF tone code.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool SendDTMFTone(const RawAddress &device, uint8_t code) override;

    /**
     * @brief Get a list of current all existing calls.
     *
     * @param device Remote device object.
     * @return The list of all existing calls.
     * @since 12
     */
    std::vector<HandsFreeUnitCalls> GetCurrentCallList(const RawAddress &device) override;

    /**
     * @brief Accept an incoming call.
     *
     * @param device Remote device object.
     * @param flag Types of calls accepted.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool AcceptIncomingCall(const RawAddress &device, int flag) override;

    /**
     * @brief Hold an active call.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool HoldActiveCall(const RawAddress &device) override;

    /**
     * @brief Reject an incoming call.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool RejectIncomingCall(const RawAddress &device) override;

    /**
     * @brief Send key preesed event.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool SendKeyPressed(const RawAddress &device) override;

    /**
     * @brief handle an incoming call.
     *
     * @param device Remote device object.
     * @param flag Types of incoming calls handle.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool HandleIncomingCall(const RawAddress &device, int flag) override;

    /**
     * @brief handle multi call.
     *
     * @param device Remote device object.
     * @param flag Types of incoming calls handle.
     * @param index call index
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool HandleMultiCall(const RawAddress &device, int flag, int index) override;

    /**
     * @brief Dial last number.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool DialLastNumber(const RawAddress &device) override;

    /**
     * @brief Dial memory.
     *
     * @param device Remote device object.
     * @param index Memory index.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool DialMemory(const RawAddress &device, int index) override;

    /**
     * @brief Send voice tag.
     *
     * @param device Remote device object.
     * @param index voice tag index.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool SendVoiceTag(const RawAddress &device, int index) override;

    /**
     * @brief Finish a specified active call.
     *
     * @param device Remote device object.
     * @param call Call object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool FinishActiveCall(const RawAddress &device, const HandsFreeUnitCalls &call) override;

    /**
     * @brief Start dial with specified call number.
     *
     * @param device Remote device object.
     * @param number Call's number to dial.
     * @return The object of dial out call or nullopt.
     * @since 12
     */
    std::optional<HandsFreeUnitCalls> StartDial(const RawAddress &device, const std::string &number) override;

    /**
     * @brief Register HfpHfServiceObserver instance.
     *
     * @param observer HfpHfServiceObserver instance.
     * @since 12
     */
    void RegisterObserver(HfpHfServiceObserver &observer) override;

    /**
     * @brief Deregister HfpHfServiceObserver instance.
     *
     * @param observer HfpHfServiceObserver instance.
     * @since 12
     */
    void DeregisterObserver(HfpHfServiceObserver &observer) override;

    /**
     * @brief Set the Hf Volume.
     *
     * @param volume The value of the volume.
     * @param type The type of the volume.
     * @since 12
     */
    void SetHfVolume(int volume, int type) override;

    /**
     * @brief Open voice recognition.
     *
     * @param device The remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool OpenVoiceRecognition(const RawAddress &device) override;

    /**
     * @brief Close voice recognition.
     *
     * @param device The remote device.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool CloseVoiceRecognition(const RawAddress &device) override;

    /**
     * @brief Notify the HF battery level change.
     *
     * @param batteryLevel The value of the battery level.
     * @since 12
     */
    void BatteryLevelChanged(int batteryLevel);

    /**
     * @brief Notify the HF enhanced driver safety change.
     *
     * @param state The state of the enhanced driver safety
     * @since 12
     */
    void EnhancedDriverSafety(int state);

    /**
     * @brief Notify the connection state change.
     *
     * @param device The remote device.
     * @param state The state of the connection.
     * @since 12
     */
    void NotifyStateChanged(const RawAddress &device, int state);

    /**
     * @brief Notify the audio connection state change.
     *
     * @param device The remote device.
     * @param state The state of the audio connection.
     * @since 12
     */
    void NotifyScoStateChanged(const RawAddress &device, int state);

    /**
     * @brief Notify the call change.
     *
     * @param device The remote device.
     * @param call The changed call.
     * @since 12
     */
    void NotifyCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call);

    /**
     * @brief Notify the battery level change.
     *
     * @param device The remote device.
     * @param batteryLevel The battery level.
     * @since 12
     */
    void NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel);

    /**
     * @brief Notify the signal strength change.
     *
     * @param device The remote device.
     * @param signal The signal strength.
     * @since 12
     */
    void NotifySignalStrengthChanged(const RawAddress &device, int signal);

    /**
     * @brief Notify the registration status change.
     *
     * @param device The remote device.
     * @param status The registration status.
     * @since 12
     */
    void NotifyRegistrationStatusChanged(const RawAddress &device, int status);

    /**
     * @brief Notify the roam status change.
     *
     * @param device The remote device.
     * @param status The roam state.
     * @since 12
     */
    void NotifyRoamingStatusChanged(const RawAddress &device, int status);

    /**
     * @brief : Post the events.
     *
     * @param event : The event of the hf
     * @since 12
     */
    void PostEvent(const HfpHfMessage &event);

    /**
     * @brief Process the events.
     *
     * @param event : The event of the hf
     * @since 12
     */
    void ProcessEvent(const HfpHfMessage &event);

    /**
     * @brief Notify the network operator name change.
     *
     * @param device The remote device.
     * @param name The network operator name.
     * @since 12
     */
    void NotifyOperatorSelectionChanged(const RawAddress &device, const std::string &name);

    /**
     * @brief Notify the subscriber number change.
     *
     * @param device : The remote device.
     * @param number The subscriber number.
     * @since 12
     */
    void NotifySubscriberNumberChanged(const RawAddress &device, const std::string &number);

    /**
     * @brief Notify the voice recognition state change.
     *
     * @param device : The remote device.
     * @param status : The voice recognition state.
     * @since 12
     */
    void NotifyVoiceRecognitionStatusChanged(const RawAddress &device, int status);

    /**
     * @brief Notify the inBand ring tone.
     *
     * @param device : The remote device.
     * @param status : The status of the inBand ring.
     * @since 12
     */
    void NotifyInBandRingTone(const RawAddress &device, int status);

    /**
     * @brief Send the AT clcc.
     * @param device : The remote device.
     * @param delayTime : The delayTime.
     * @since 12
     */
    void QueryCall(const RawAddress &device, int64_t delayTime);

    /**
     * @brief Process the events.
     *
     * @param event : The event of the hf
     * @param delayTime : delayTime
     * @since 12
     */
    void PostDelayEvent(const HfpHfMessage &event, int64_t delayTime);

    /**
     * @brief Process remove state machine.
     *
     * @param address : device address
     * @since 12
     */
    void ProcessRemoveStateMachine(const std::string &address);

    /**
     * @brief Set audio parameters.
     *
     * @since 12
     */
    void SetAudioParameters();

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

    bthf_client_interface_t* GetBluetoothHfInterface() const;

private:
    /**
     * @brief Service startup.
     *
     * @since 12
     */
    void StartUp();

    /**
     * @brief Service shutdown.
     *
     * @since 12
     */
    void ShutDown();
    void ProcessConnectEvent(const HfpHfMessage &event);
    void SendEventToEachStateMachine(const HfpHfMessage &event);
    void ProcessDefaultEvent(const HfpHfMessage &event);
    int GetConnectedDeviceNum();
    bool IsConnected(const std::string &address);
    void ReportCallInfo(const HandsFreeUnitCalls &call, const RawAddress &device);
    bool IsRemoteHfpSupported(const RawAddress &device);
    bthf_client_interface_t* bluetoothHfInterface = nullptr;

    bool isStarted_ {false};
    bool isShuttingDown_ {false};
    // The map of the device and sate machine.
    std::map<const std::string, std::unique_ptr<HfpHfStateMachine>> stateMachines_ {};
    // The mutex variable
    BtRecursiveMutex mutex_ {};
    // The list of the observer pointer.
    BaseObserverList<HfpHfServiceObserver> observers_ {};
    // The maximum default number of connection devices.
    inline static constexpr int MAX_DEFAULT_HF_CONNECTIONS_NUM {2};
    // The maximum number of connected devices.
    std::atomic<int> maxConnectedNum_ {MAX_DEFAULT_HF_CONNECTIONS_NUM};
    // callback of call state report.
    sptr<Telephony::ICallStatusCallback> btCallCallbackPtr_;

    // const state map
    const std::map<const int, const int> stateMap_ = {
        {HFP_HF_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {HFP_HF_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {HFP_HF_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {HFP_HF_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
    BT_DISALLOW_COPY_AND_ASSIGN(HfpHfService);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // HFP_HF_SERVICE_H
