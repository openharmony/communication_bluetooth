/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
 *
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

#ifndef HFP_HF_STATEMACHINE_H
#define HFP_HF_STATEMACHINE_H

#include <list>
#include <memory>
#include <string>
#include <queue>

#include "hfp_hf_call_manager.h"
#include "hfp_hf_defines.h"
#include "hfp_hf_message.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "bt_hf_client.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Class for HF state machine.
 */
class HfpHfStateMachine : public utility::StateMachine {
public:
    /**
     * @brief Construct a new HfpHfStateMachine object.
     *
     * @param address Device address.
     * @since 12
     */
    explicit HfpHfStateMachine(const std::string &address);

    /**
     * @brief Destroy the HfpHfStateMachine object.
     * @since 12
     */
    ~HfpHfStateMachine() = default;

    /**
     * @brief Initialise the state machine.
     * @since 12
     */
    void Init();

    /**
     * @brief Get the IsInbandRing flag.
     *
     * @return Returns isInBandRing flag.
     * @since 12
     */
    bool IsInbandRing() const;

    /**
     * @brief Get the volume stored in state machine.
     *
     * @return Returns the volume stored in state machine.
     * @since 12
     */
    int GetVolume() const;

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     * @since 12
     */
    int GetDeviceState() const;

    /**
     * @brief Get the current call list.
     *
     * @return Returns the current call list.
     * @since 12
     */
    std::vector<HandsFreeUnitCalls> GetCurrentCallList();

    /**
     * @brief Process interactive event from profile.
     *
     * @param event The event from profile.
     * @since 12
     */
    void ProcessInteractiveEvent(const HfpHfMessage &event);

    /**
     * @brief Process the result(ok) of AT command.
     *
     * @param event The event from profile.
     * @since 12
     */
    void ProcessAtCommandResult(const HfpHfMessage &event);

    /**
     * @brief Process accept call event from service.
     *
     * @param flag Accept call flag.
     * @since 12
     */
    void ProcessAcceptCall(int flag);

    /**
     * @brief Process hold call event from service.
     * @since 12
     */
    void ProcessHoldCall();

    /**
     * @brief Process reject call event from service.
     * @since 12
     */
    void ProcessRejectCall();

    /**
     * @brief Process send key pressed event.
     * @since 12
     */
    void PrecessSendKeyPressed();

    /**
     * @brief Process finish active call event from service.
     *
     * @param event The finish call event.
     * @since 12
     */
    void ProcessFinishActiveCall(const HfpHfMessage &event);

    /**
     * @brief Process dial call event from service.
     *
     * @param event The dial call event.
     * @since 12
     */
    void ProcessDialCall(const HfpHfMessage &event);

    /**
     * @brief Process open voice recognition event from service.
     * @since 12
     */
    void ProcessOpenVoiceRecognition();

    /**
     * @brief Process close voice recognition event from service.
     * @since 12
     */
    void ProcessCloseVoiceRecognition();

    /**
     * @brief Get the voice recognition state.
     *
     * @return Returns the voice recognition state.
     * @since 12
     */
    int GetVoiceRecognitionState() const;

    /**
     * @brief Add the deferred message.
     *
     * @param msg The message.
     * @since 12
     */
    void AddDeferredMessage(const HfpHfMessage &msg);

    /**
     * @brief Process the deferred message.
     * @since 12
     */
    void ProcessDeferredMessage();

    /**
     * @brief Get the service event name.
     *
     * @param what Service event number.
     * @return Returns service event string name.
     * @since 12
     */
    static std::string GetEventName(int what);

    /**
     * @brief Get the service event name.
     *
     * @param what Service event number.
     * @return Returns service event string name.
     * @since 12
     */
    static std::string GetConnectEventName(int what);

    /**
     * @brief Notify the state change.
     * @since 12
     */
    void NotifyStateTransitions();

    /**
     * @brief Notify the child state change to parent state.
     * @since 12
     */
    void NotifyChildStateToParentState(int fromState, int toState);

    /**
     * @brief Process audio disconnected event.
     * @since 12
     */
    void ProcessAudioDisconnected();

    /**
     * @brief Start the connection timer.
     * @since 12
     */
    void StartConnectionTimer() const;

    /**
     * @brief Stop the connection timer.
     * @since 12
     */
    void StopConnectionTimer() const;

    /**
     * @brief Connection timeout handler.
     * @since 12
     */
    void ConnectionTimeout() const;

    /**
     * @brief Check if current statemachine is removing.
     *
     * @return Returns <b>true</b> if the statemachine is removing; returns <b>false</b> if not.
     * @since 12
     */
    bool IsRemoving() const;

    /**
     * @brief Mark statemachine removing.
     *
     * @param isRemoving removing mark.
     * @since 12
     */
    void SetRemoving(bool isRemoving);

    /**
     * @brief Process Call indicator change event.
     *
     * @param event Call indicator change event.
     * @since 12
     */
    void ProcessCallIndicatorEvent(const HfpHfMessage &event);

    /**
     * @brief Set is audio wbs.
     *
     * @param isAudioWbs is audio wbs.
     * @since 12
     */
    void SetAudioWbs(bool isAudioWbs);

    /**
     * @brief Get the device address.
     *
     * @return Returns the device address.
     * @since 12
     */
    std::string GetDeviceAddr() const;

    /**
     * @brief Process connect event.
     *
     * @return <b>true</b> if the connect success; returns <b>false</b> if not.
     * @since 12
     */
    bool ProcessConnectEvent();

    /**
     * @brief Process disconnect event.
     *
     * @return <b>true</b> if the disconnect success; returns <b>false</b> if not.
     * @since 12
     */
    bool ProcessDisconnectEvent();

    /**
     * @brief Process connect sco event.
     *
     * @return <b>true</b> if the connect sco success; returns <b>false</b> if not.
     * @since 12
     */
    bool ProcessConnectScoEvent();

    /**
     * @brief Process disconnect sco event.
     *
     * @return <b>true</b> if the disconnect sco success; returns <b>false</b> if not.
     * @since 12
     */
    bool ProcessDisconnectScoEvent();

    /**
     * @brief Whether allow ag connected.
     *
     * @param address device address.
     * @return <b>true</b> if allow; returns <b>false</b> if not.
     * @since 12
     */
    bool IsAllowAgConnected(const std::string &address);

    /**
     * @brief Process set hf volume.
     *
     * @param volume volume value.
     * @param type volume type.
     * @since 12
     */
    void ProcessSetHfVolume(int volume, int type);

    /**
     * @brief Process slc established event.
     *
     * @param event slc established event.
     * @since 12
     */
    void ProcessSlcEstablished(const HfpHfMessage &event);

    /**
     * @brief Process battery level update.
     *
     * @param batteryLevel battery level.
     * @since 12
     */
    void ProcessBatteryLevelUpdate(int batteryLevel);

    /**
     * @brief Process send dtmf command.
     *
     * @param flag dtmf flag.
     * @since 12
     */
    void PrecessSendDtmf(int flag);

/**
     * @brief Set audio parameters.
     *
     * @since 12
     */
    void SetAudioConnectedParameters();

#ifdef BLUETOOTH_WATCH_ENABLE
    /**
     * @brief watch dial call.
     *
     * @since 12
     */
    void WatchDialCall();
#endif

private:
    std::string address_;
    std::string operatorName_ {""};
    std::string subscriberNumber_ {""};
    std::list<HfpHfMessage> deferMsgs_ {};
    std::unique_ptr<utility::Timer> connTimer_ {nullptr};
    std::unique_ptr<HfpHfCallManager> calls_ {nullptr};

    int networkState_ {0};
    int networkTRoam_ {0};
    int networkSignal_ {0};
    int batteryLevel_ {0};
    int volume_ {0};
    int peerFeatures_ {0};
    int chldFeatures_ {0};
    int preState_ {0};
    int voiceRecognitionStatus_ {HFP_HF_VR_STATE_CLOSED};

    bool isInBandRing_ {false};
    bool isRemoving_ {false};
    bool isScoConnected_ {false};
    bool isAudioWbs_{false};
    bool isOnClcc_{false};

    std::queue<int> queuedActions;
    bthf_client_interface_t* bluetoothHfInterface = nullptr;

    void ProcessCurrentCallEvent(const HfpHfMessage &event) const;
    void ProcessNetworkStateEvent(const HfpHfMessage &event);
    void ProcessNetworkRoamEvent(const HfpHfMessage &event);
    void ProcessNetworkSignalEvent(const HfpHfMessage &event);
    void ProcessBatteryLevelEvent(const HfpHfMessage &event);
    void ProcessOperatorEvent(const HfpHfMessage &event);
    void ProcessSetVolumeEvent(const HfpHfMessage &event);
    void ProcessSubscriberNumberEvent(const HfpHfMessage &event);
    void ProcessInbandRingEvent(const HfpHfMessage &event);
    void ProcessOpenVoiceRecognitionResult(const HfpHfMessage &event);
    void ProcessCloseVoiceRecognitionResult(const HfpHfMessage &event);
    void NotifyVoiceRecognitionStatusChanged(int status);
    void SetAudioConnectionState(bool isScoConnected);
    void SetAudioParameter(int audioStatus);
    void HandleCallAction(int action, int event);
    bool HandleVoipCall(const HfpHfMessage &event);
    void HandleCurrentCall(const HfpHfMessage &event);
    void SetAudioExtraParameters(const std::string &key, const std::string &value);
    std::string GetInteractiveEventName(int type) const;
    void GetBluetoothHfInterface();

    BT_DISALLOW_COPY_AND_ASSIGN(HfpHfStateMachine);
};

class HfpHfState : public utility::StateMachine::State {
public:
    HfpHfState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent),
        stateInt_(stateInt), stateMachine_(static_cast<HfpHfStateMachine &>(statemachine)) {}

    HfpHfState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine),
        stateInt_(stateInt), stateMachine_(static_cast<HfpHfStateMachine &>(statemachine)) {}

    virtual ~HfpHfState()
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {HFP_HF_STATE_DISCONNECTED};
    HfpHfStateMachine &stateMachine_;
};

class HfpHfDisconnected : public HfpHfState {
public:
    HfpHfDisconnected(const std::string &name, utility::StateMachine &statemachine)
        : HfpHfState(name, statemachine, HFP_HF_STATE_DISCONNECTED)
    {}
    ~HfpHfDisconnected() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

class HfpHfConnecting : public HfpHfState {
public:
    HfpHfConnecting(const std::string &name, utility::StateMachine &statemachine)
        : HfpHfState(name, statemachine, HFP_HF_STATE_CONNECTING)
    {}
    ~HfpHfConnecting() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class HfpHfConnected : public HfpHfState {
public:
    HfpHfConnected(const std::string &name, utility::StateMachine &statemachine)
        : HfpHfState(name, statemachine, HFP_HF_STATE_CONNECTED)
    {}
    ~HfpHfConnected() override = default;
    void Entry() override;
    void Exit() override
    {}
    bool Dispatch(const utility::Message &msg) override;
    void DispatchOthers(const utility::Message &msg);
};

class HfpHfAudioConnected : public HfpHfState {
public:
    HfpHfAudioConnected(const std::string &name, utility::StateMachine &statemachine,
        utility::StateMachine::State &parent)
        : HfpHfState(name, statemachine, HFP_HF_AUDIO_STATE_CONNECTED, parent)
    {}
    ~HfpHfAudioConnected() override = default;
    void Entry() override;
    void Exit() override
    {}
    bool Dispatch(const utility::Message &msg) override;

private:
    void ProcessDisconnect(const HfpHfMessage &event);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // HFP_HF_STATEMACHINE_H