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
#ifndef BLUETOOTH_HIGHPOWER_MANAGER_H
#define BLUETOOTH_HIGHPOWER_MANAGER_H

#include <string>
#include "bluetooth_types.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "interface_profile_hfp_ag.h"
#include "interface_profile_a2dp_src.h"
#include "wifi_device.h"
#include "wifi_hotspot.h"

namespace OHOS {
namespace bluetooth {

/**
* enum to indentify bluetooth scenario
*/
enum class BluetoothScenario : std::uint32_t {
    BT_INACTIVE,
    BT_PLAYING,
    BT_CELLULAR_CALLING,
    BT_VOIP_CALLING
};

/**
    * enum to indentify wifi scenario
    */
enum class WifiScenario : std::uint32_t {
    WIFI_INACTIVE,
    WIFI_2GHZ_ACTIVE,
    WIFI_5GZ_ACTIVE
};

enum class BtHighPowerEventType : std::uint32_t {
    SCREEN_ON,
    SCREEN_OFF,
    WIFI_POWER_STATE,
    WIFI_CONN_STATE,
    WIFI_HOTSPOT_STATE,
    WIFI_AP_STA_JOIN,
    WIFI_AP_STA_LEAVE,
    BLUETOOTH_HOST_STATE_UPDATE,
    UNKNOWN,
 };

class HfpAgObserver : public HfpAgServiceObserver {
public:
    HfpAgObserver() = default;
    ~HfpAgObserver() override {}
    void OnScoStateChanged(const RawAddress& device, int state, int reason) override;
private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfpAgObserver);
};

class A2dpObserver : public IA2dpObserver {
public:
    A2dpObserver() = default;
    ~A2dpObserver() override {}
    void OnPlayingStatusChaned(const RawAddress &device, int playingState, int error) override;
private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(A2dpObserver);
};

class BluetoothHighPowerCommonEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit BluetoothHighPowerCommonEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~BluetoothHighPowerCommonEventSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class BluetoothHighPowerManager {
public:
    static BluetoothHighPowerManager& GetInstance();
    void Init();
    void handleCommEvent(const OHOS::EventFwk::CommonEventData &data);
    void OnScoStateChanged(const RawAddress& device, int state, int reason);
    void OnPlayingStatusChaned(const RawAddress &device, int playingState, int error);

private:
    BluetoothHighPowerManager() = default;
    ~BluetoothHighPowerManager();

    bool GetWifiStaPowerState();
    bool GetWifiHotSpotPowerState();
    bool IsSupportHighPowerV1();
    void UpdateWifiState();
    void UpdateSoftApState();
    void SetHighPowerModeEnable(bool enable);
    void RegisterA2dpObserver();
    void DeRegisterA2dpObserver();
    void RegisterHfpAgObserver();
    void DeRegisterHfpAgObserver();
    void RegisterCommonEvent();
    void UnRegisterCommonEvent();
    void HandleBluetoothStateChange(const int bluetoothState);
    void UpdateHighPowerModeOnBtWorking();
    WifiScenario GetWifiScenario();
    BluetoothScenario GetBtScenario();
    BluetoothScenario GetBtCallingScenario();
    void UpdateHighPowerMode();
    void HandleScreenStateChange(bool isScreenOn);
    BtHighPowerEventType GetEventType(std::string &event) const;
    void HandleWifiStateChange(const int staState);
    void HandleWifiConnectStateChange(const int wifiConnState);
    void HandleHotSpotStateChange(const int hotSpotState);
    void HandleHotSpotConnectStateChange();
    void InitWifiState();
    void InitSoftApState();
    IProfileHfpAg *GetHfpAgService();
    IProfileA2dp *GetA2dpService();

    std::shared_ptr<BluetoothHighPowerCommonEventSubscriber> subscriber_ = nullptr;
    std::unique_ptr<HfpAgObserver> hfpObserverImp_ = nullptr;
    std::unique_ptr<A2dpObserver> a2dpObserverImp_ = nullptr;
    bool isBluetoothEnable_ = false;
    bool isDisabledByCCM_ = false;
    bool isWifiEnable_ = false;
    bool isWifiConnected_ = false;
    bool isHotSpotEnable_ = false;
    bool lastHighPowerMode_ = false;
    bool wifiSta2GhzActive_ = false;
    bool wifiSta5GhzActive_ = false;
    bool softAp2GhzActive_ = false;
    bool softAp5GhzActive_ = false;
    bool isScreenOnNow_ = false;
    bool hfpAudioOn_ = false;
    bool a2dpPlaying_ = false;
    bool isInitialized_ = false;
    WifiScenario currentWifiScenario_ = WifiScenario::WIFI_INACTIVE;
    BluetoothScenario currentBtScenario_ = BluetoothScenario::BT_INACTIVE;
    std::unordered_map<std::string, BtHighPowerEventType> btHighPowerEventType_ = {};
    std::shared_ptr<Wifi::WifiDevice> wifiDevice_ = nullptr;
    std::shared_ptr<Wifi::WifiHotspot> wifiHotSpot_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHighPowerManager);
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_HIGHPOWER_MANAGER_H