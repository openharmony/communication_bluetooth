/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef PC_POWER_MANAGER_H
#define PC_POWER_MANAGER_H

#include <cstdint>
#include <mutex>
#include <memory>
#include "log.h"
#include "raw_address.h"
#include "hw_hcidefs.h"
#include "ble_service_data.h"
#include "bluetooth_common_event_subscriber.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include "iservice_registry.h"
#include "interface_profile_hid_host.h"
#include "interface_profile_manager.h"
#include "btcommon/timer_manager.h"
#include "interface_adapter_manager.h"

namespace OHOS {
namespace bluetooth {

class BluetoothCommonEventSubscriber;
#define COD_MASK 0x07FF
constexpr const char *COMMON_EVENT_POWER_MANAGER_STATE_CHANGED = "usual.event.POWER_MANAGER_STATE_CHANGED";
enum class PowerEvent : uint8_t {
    STATE_POWER_START = 0x00,
    STATE_SWITCH_ON = 0x10,  // 16
    STATE_SWITCH_OFF = 0x11,
    STATE_INSERT_SCREEN = 0x20,  // 32
    STATE_REMOVEING_SCREEN = 0x21,
    STATE_ENTER_FORCESLEEP = 0x30,  // 48
    STATE_EXIT_FORCESLEEP = 0x31,
};

struct CurrentPowerState {
    PowerEvent switchState;
    PowerEvent extrernalScreenState;
    PowerEvent forcesleepState;
    CurrentPowerState()
    {
        switchState = PowerEvent::STATE_SWITCH_ON;
        extrernalScreenState = PowerEvent::STATE_REMOVEING_SCREEN;
        forcesleepState = PowerEvent::STATE_EXIT_FORCESLEEP;
    }
};

class IPowerState {
public:
    IPowerState() = default;
    virtual ~IPowerState() = default;

    static IPowerState *GetInstance()
    {
        static IPowerState instance;
        return &instance;
    }

    virtual PowerEvent State();
    virtual void HandleEvent(const std::string &eventData);

    void AddEvent(const uint8_t event);
    bool CheckCod(uint32_t remoteCod, const uint32_t &cod);
    void CmdFilterHdiToHisi(const uint8_t &enabled);

protected:
    bool ExistExternalScreen(const std::string &eventData);

public:
    static CurrentPowerState state_;
};

class SwitchEvent : public IPowerState {
public:
    SwitchEvent();
    ~SwitchEvent() override;
    static SwitchEvent *GetInstance()
    {
        static SwitchEvent instance;
        return &instance;
    }

    PowerEvent State() override;
    void HandleEvent(const std::string &eventData) override;
};

class ExternalScreenEvent : public IPowerState {
public:
    ExternalScreenEvent();
    ~ExternalScreenEvent() override;
    static ExternalScreenEvent *GetInstance()
    {
        static ExternalScreenEvent instance;
        return &instance;
    }

    PowerEvent State() override;
    void HandleEvent(const std::string &eventData) override;
};

class ForceSleep : public IPowerState {
public:
    ForceSleep();
    ~ForceSleep() override;
    static ForceSleep *GetInstance()
    {
        static ForceSleep instance;
        return &instance;
    }

    PowerEvent State() override;
    void HandleEvent(const std::string &eventData) override;

    void DisconnectExceptMouseAndKeyboard();
    void ForceSleepStopBtScan();
    void ForceSleepStartBtScan();
    void ForceSleepAutoBtConnect();
    void HandleEnterForceSleepEvent();
    void StartHandleEnterForceSleepEventTimer();
    void StopHandleEnterForceSleepEventTimer();
    bool IsHandleEnterForceSleepEventTimerStarted();
    void SetLeConnectionScanToFast(bool enable);
    bool IsPairedMouseOrKeyboard();

private:
    inline static constexpr int TIMEOUT_HANDLE_ENTER_FORCE_SLEEP_EVENT {5000};
    std::atomic_bool isForceSleep_ = false;
    std::mutex handleEnterForceSleepEventTimeoutMutex_;
    std::shared_ptr<utility::Timer> handleEnterForceSleepEventTimeout_ = nullptr;
};

class HidHostCallback : public bluetooth::IHidHostObserver {
public:
    HidHostCallback();
    ~HidHostCallback();
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override;
};

class AdapterStateCallback : public bluetooth::IAdapterStateObserver {
public:
    explicit AdapterStateCallback(int priority = PRIORITY_LOW): priority_(priority) {}
    ~AdapterStateCallback();
    void OnStateChange(const BTTransport transport, const BTStateID state) override {}
    void OnBluetoothStateChanged(int state) override;
private:
    int priority_;
};

class PowerStateFactory {
public:
    static PowerStateFactory *Instance()
    {
        static PowerStateFactory instance;
        return &instance;
    }

    IPowerState *CreatePowerState(const uint8_t state)
    {
        switch (state) {
            case static_cast<uint8_t>(PowerEvent::STATE_SWITCH_ON):
            case static_cast<uint8_t>(PowerEvent::STATE_SWITCH_OFF):
                return SwitchEvent::GetInstance();
            case static_cast<uint8_t>(PowerEvent::STATE_POWER_START):
                return IPowerState::GetInstance();
            case static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP):
            case static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP):
                return ForceSleep::GetInstance();
            case static_cast<uint8_t>(PowerEvent::STATE_INSERT_SCREEN):
            case static_cast<uint8_t>(PowerEvent::STATE_REMOVEING_SCREEN):
                return ExternalScreenEvent::GetInstance();
            default:
                return nullptr;
        }

        return nullptr;
    }
};

class PowerMgrHelper {
public:
    ~PowerMgrHelper();
    static PowerMgrHelper *GetInstance()
    {
        static PowerMgrHelper instance;
        return &instance;
    }

    void RegisterPowermgrCommonEvent();
    void UnRegisterPowermgrCommonEvent();
    void OnPowerStateChanged(const uint8_t &state, const std::string &eventData);
    void OnPowerConnectStateChanged(const std::string &action);
    void OnScreenStateChanged(const std::string &action);
    bool CheckIfNeedAutoConnect();
    void SetBleSettings(const BleScanSettingsImpl &settings)
    {
        settings_ = settings;
    }
    BleScanSettingsImpl GetBleSettings()
    {
        return settings_;
    }
    void SetNeedReconnectActiveDevice()
    {
        needReconnectActiveDevice_.store(true);
    }
    bool GetNeedReconnectActiveDevice()
    {
        return needReconnectActiveDevice_.load();
    }
    std::atomic_bool isFastScan_ = true;
    std::atomic_bool isPowerOff_ = true;
private:
    PowerMgrHelper();

    std::shared_ptr<BluetoothCommonEventSubscriber> subscriber_ = nullptr;
    std::shared_ptr<BluetoothCommonEventSubscriber> screenSubscriber_ = nullptr;
    BleScanSettingsImpl settings_;
    IProfileHidHost *hidHostService_ = nullptr;
    HidHostCallback *hidHostCallback_ = nullptr;
    AdapterStateCallback *adapterStateCallback_ = nullptr;
    std::atomic_bool needReconnectActiveDevice_ = false;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PC_POWER_MANAGER_H