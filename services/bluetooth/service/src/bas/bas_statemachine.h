/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef BAS_STATEMACHINE_H
#define BAS_STATEMACHINE_H

#include <atomic>
#include <list>
#include <memory>
#include <string>
#include "bas_defines.h"
#include "bas_message.h"
#include "bas_service_impl_interface.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "interface_adapter_classic.h"
#include "interface_profile_gatt_client.h"

namespace OHOS {
namespace bluetooth {
class BasStateMachine : public utility::StateMachine {
public:
    /**
     * @brief Construct a new BasStateMachine object.
     *
     * @param address Device address.
     */
    explicit BasStateMachine(const std::string &address, BasServiceImplInterface *basServiceImpl);

    /**
     * @brief Destroy the BasStateMachine object.
     */
    ~BasStateMachine();

    /**
     * @brief Initialize the state machine.
     */
    void Init();

    /**
     * @brief Check if current statemachine is removing.
     *
     * @return Returns <b>true</b> if the statemachine is removing; returns <b>false</b> if not.
     */
    bool IsRemoving() const;

    /**
     * @brief Mark statemachine removing.
     *
     * @param isRemoving removing mark.
     */
    void SetRemoving(bool isRemoving);

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     */
    int GetDeviceStateInt() const;

    BasServiceImplInterface* GetBasService(void);

    void ConnectionTimeout() const;
    void DisconnectionTimeout() const;
    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void StartDisconnectionTimer() const;
    void StopDisconnectionTimer() const;
    void AddDeferredMessage(const BasMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAddress();
    void NotifyStateTransitions();
    static std::string GetEventName(int what);

    void InitGatt();
    bool ConnectGatt();
    bool DisconnectGatt();
    void CloseGatt();
    bool DiscoverServicesGatt();
    bool GetBatteryLevel();
    bool EnableBasNotification();
    int GetCachedBatteryLevel() const;

    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string DISCONNECTING = "Disconnecting";
    inline static const std::string CONNECTED = "Connected";

private:
    static IProfileGattClient *GetGattClientService();
    const Characteristic* FindBatteryCharacteristic(const std::vector<Service>& services);
    const Descriptor* FindCccdDescriptor(const Characteristic& charac);

    friend class BasStateMachineFriendTest;

    void OnServicesDiscovered();
    void UpdateBatteryLevel(int batteryLevel);
    void OnBatteryLevelRead(int batteryLevel);

    class GattClientCallbackImpl;
    std::string address_;
    BasServiceImplInterface* basServiceImpl_ = nullptr;
    int batteryLevel_{BAS_INVALID_BATTERY_LEVEL};
    std::atomic<int> appId_{-1};
    std::shared_ptr<IGattClientCallback> callback_{nullptr};
    bool isRemoving_ {false};
    int preState_ {0};
    std::atomic<uint16_t> basCharacHandle_ {0};
    std::list<BasMessage> deferMsgs_ {};
    std::unique_ptr<utility::Timer> connTimer_ {nullptr};
    std::unique_ptr<utility::Timer> disconnTimer_ {nullptr};
    inline static const int CONNECTION_TIMEOUT_MS {60000};
    inline static const int DISCONNECTION_TIMEOUT_MS {60000};

    BT_DISALLOW_COPY_AND_ASSIGN(BasStateMachine);
};

class BasState : public utility::StateMachine::State {
public:
    BasState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(static_cast<BasStateMachine &>(statemachine))
    {}

    BasState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine), stateInt_(stateInt),
        stateMachine_(static_cast<BasStateMachine &>(statemachine))
    {}

    ~BasState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {BAS_STATE_DISCONNECTED};
    BasStateMachine &stateMachine_;
};

class BasDisconnectedState : public BasState {
public:
    BasDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : BasState(name, statemachine, BAS_STATE_DISCONNECTED)
    {}
    ~BasDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

class BasConnectingState : public BasState {
public:
    BasConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : BasState(name, statemachine, BAS_STATE_CONNECTING)
    {}
    ~BasConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class BasDisconnectingState : public BasState {
public:
    BasDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : BasState(name, statemachine, BAS_STATE_DISCONNECTING)
    {}
    ~BasDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class BasConnectedState : public BasState {
public:
    BasConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : BasState(name, statemachine, BAS_STATE_CONNECTED)
    {}
    ~BasConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}
#endif  // BAS_STATEMACHINE_H