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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_highpower"
#endif

#include "adapter_manager.h"
#include "bluetooth_highpower_manager.h"
#include "bluetooth_log.h"
#include "bt_av.h"
#include "interface_profile_manager.h"
#include "hfp_ag_defines.h"
#include "hfp_ag_service.h"
#include "power_mgr_client.h"
#include "wifi_device.h"
#include "wifi_hotspot.h"
#include "wifi_msg.h"

#include "log.h"
#include "common_util.h"
#include "syspara/parameters.h"
#include "hitrace_meter.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {

using OHOS::EventFwk::CommonEventSupport;

constexpr uint32_t CORE_EVENT_PRIORITY = 1;
constexpr int32_t BAND_5_GHZ_START_FREQ_MHZ = 5160;
constexpr int32_t BAND_5_GHZ_END_FREQ_MHZ = 5885;
constexpr int32_t DEFAULT_ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;

IProfileHfpAg *BluetoothHighPowerManager::GetHfpAgService()
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr == nullptr) {
        HILOGE("failed to get service manager");
        return nullptr;
    }
    IProfileHfpAg *hfpAgService = static_cast<IProfileHfpAg *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    return hfpAgService;
}

IProfileA2dp *BluetoothHighPowerManager::GetA2dpService()
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr == nullptr) {
        HILOGE("failed to get service manager");
        return nullptr;
    }
    IProfileA2dp *a2dpService = static_cast<IProfileA2dp *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    return a2dpService;
}

void BluetoothHighPowerManager::OnScoStateChanged(const RawAddress& device, int state, int reason)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    IProfileHfpAg *hfpAgService = GetHfpAgService();
    if (hfpAgService == nullptr) {
        return;
    }
    auto currActiveDevice = hfpAgService->GetActiveDevice();
    if (currActiveDevice != device.GetAddress()) {
        return;
    }
    hfpAudioOn_ = (state == HFP_AG_AUDIO_STATE_CONNECTING || state == HFP_AG_AUDIO_STATE_CONNECTED) ? true : false;
    BluetoothScenario newScenario = GetBtScenario();
    if (newScenario != currentBtScenario_) {
        currentBtScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void HfpAgObserver::OnScoStateChanged(const RawAddress& device, int state, int reason)
{
    DoInAdapterManagerThread(std::bind(&BluetoothHighPowerManager::OnScoStateChanged,
        &BluetoothHighPowerManager::GetInstance(), device, state, reason));
}

void BluetoothHighPowerManager::OnPlayingStatusChaned(const RawAddress& device, int state, int error)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    IProfileA2dp *a2dpService = GetA2dpService();
    if (a2dpService == nullptr) {
        return;
    }
    auto currActiveDevice = a2dpService->GetActiveSinkDevice();
    if (device.GetAddress() != currActiveDevice.GetAddress()) {
        return;
    }
    a2dpPlaying_ = (state >= BTAV_AUDIO_STATE_STARTED) ? true : false;
    BluetoothScenario newScenario = GetBtScenario();
    if (newScenario != currentBtScenario_) {
        currentBtScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void A2dpObserver::OnPlayingStatusChaned(const RawAddress &device, int playingState, int error)
{
    DoInAdapterManagerThread(std::bind(&BluetoothHighPowerManager::OnPlayingStatusChaned,
        &BluetoothHighPowerManager::GetInstance(), device, playingState, error));
}

BluetoothScenario BluetoothHighPowerManager::GetBtCallingScenario()
{
    HfpAgService *hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return BluetoothScenario::BT_INACTIVE;
    }
    if (!hfpAgService->IsVirtualCallStarted()) {
        return BluetoothScenario::BT_CELLULAR_CALLING;
    } else {
        return BluetoothScenario::BT_VOIP_CALLING;
    }
}

BluetoothScenario BluetoothHighPowerManager::GetBtScenario()
{
    if (hfpAudioOn_) {
        return GetBtCallingScenario();
    }
    if (a2dpPlaying_) {
        return BluetoothScenario::BT_PLAYING;
    }
    return BluetoothScenario::BT_INACTIVE;
}

BluetoothHighPowerManager::~BluetoothHighPowerManager()
{
    if (isDisabledByCCM_) {
        return;
    }
    HILOGI("deinit");
    DeRegisterA2dpObserver();
    DeRegisterHfpAgObserver();
    UnRegisterCommonEvent();
    isInitialized_ = false;
}

bool BluetoothHighPowerManager::IsSupportHighPowerV1()
{
    if (OHOS::system::GetParameter("const.bluetooth.supports_bluetooth_high_pwr", "false") != "true") {
        HILOGI("highpowerv1 is not support.");
        return false;
    } else {
        HILOGI("highpowerv1 is support.");
        return true;
    }
}

void BluetoothHighPowerManager::Init()
{
    HILOGI("init");
    isDisabledByCCM_ = !IsSupportHighPowerV1();
    if (isDisabledByCCM_) {
        return;
    }
    if (isInitialized_) {
        HILOGI("has initialized, no need to init.");
        return;
    }

    isBluetoothEnable_ = true;
    btHighPowerEventType_ = {
        {CommonEventSupport::COMMON_EVENT_SCREEN_ON, BtHighPowerEventType::SCREEN_ON},
        {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, BtHighPowerEventType::SCREEN_OFF},
        {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, BtHighPowerEventType::WIFI_POWER_STATE},
        {CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, BtHighPowerEventType::WIFI_CONN_STATE},
        {CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE, BtHighPowerEventType::WIFI_HOTSPOT_STATE},
        {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN, BtHighPowerEventType::WIFI_AP_STA_JOIN},
        {CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE, BtHighPowerEventType::WIFI_AP_STA_LEAVE},
        {CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE,
            BtHighPowerEventType::BLUETOOTH_HOST_STATE_UPDATE},
    };

    RegisterA2dpObserver();
    RegisterHfpAgObserver();
    RegisterCommonEvent();
    // init status
    isWifiEnable_ = GetWifiStaPowerState();
    isHotSpotEnable_ = GetWifiHotSpotPowerState();
    isScreenOnNow_ = OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    isInitialized_ = true;
    UpdateWifiState();
    UpdateSoftApState();
    currentWifiScenario_ = GetWifiScenario();
    UpdateHighPowerMode();
    return;
}

BluetoothHighPowerManager& BluetoothHighPowerManager::GetInstance()
{
    static BluetoothHighPowerManager instance;
    return instance;
}

void BluetoothHighPowerCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    DoInAdapterManagerThread(std::bind(&BluetoothHighPowerManager::handleCommEvent,
        &BluetoothHighPowerManager::GetInstance(), data));
}

void BluetoothHighPowerManager::RegisterCommonEvent()
{
    if (isDisabledByCCM_ || subscriber_ != nullptr) {
        return;
    }
    HITRACE_METER(BT_TRACE_TAG);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_HOTSPOT_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_JOIN);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_AP_STA_LEAVE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    subscribeInfo.SetPriority(CORE_EVENT_PRIORITY);
    subscriber_ = std::make_shared<BluetoothHighPowerCommonEventSubscriber>(subscribeInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        HILOGE("SubscribeCommonEvent fail.");
        subscriber_ = nullptr;
        return;
    }
    HILOGI("SubscribeCommonEvent successful");
}

void BluetoothHighPowerManager::UnRegisterCommonEvent()
{
    if (isDisabledByCCM_) {
        return;
    }
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("UnRegisterCommonEvent");
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

void BluetoothHighPowerManager::RegisterA2dpObserver()
{
    if (isDisabledByCCM_ || a2dpObserverImp_ != nullptr) {
        return;
    }
    IProfileA2dp *a2dpService = GetA2dpService();
    if (a2dpService == nullptr) {
        return;
    }
    a2dpObserverImp_ = std::make_unique<A2dpObserver>();
    if (a2dpObserverImp_ != nullptr) {
        a2dpService->RegisterObserver(reinterpret_cast<IA2dpObserver *>(a2dpObserverImp_.get()));
    }
}

void BluetoothHighPowerManager::DeRegisterA2dpObserver()
{
    if (isDisabledByCCM_ || a2dpObserverImp_ == nullptr) {
        return;
    }
    IProfileA2dp *a2dpService = GetA2dpService();
    if (a2dpService != nullptr && a2dpObserverImp_ != nullptr) {
        a2dpService->DeregisterObserver(reinterpret_cast<IA2dpObserver *>(a2dpObserverImp_.get()));
        a2dpObserverImp_ = nullptr;
    }
}

void BluetoothHighPowerManager::RegisterHfpAgObserver()
{
    if (isDisabledByCCM_ || hfpObserverImp_ != nullptr) {
        return;
    }
    IProfileHfpAg *hfpAgService = GetHfpAgService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpObserverImp_ = std::make_unique<HfpAgObserver>();
    if (hfpObserverImp_ != nullptr) {
        hfpAgService->RegisterObserver(*(reinterpret_cast<HfpAgServiceObserver *>(hfpObserverImp_.get())));
    }
}

void BluetoothHighPowerManager::DeRegisterHfpAgObserver()
{
    if (isDisabledByCCM_ || hfpObserverImp_ == nullptr) {
        return;
    }
    IProfileHfpAg *hfpAgService = GetHfpAgService();
    if (hfpAgService != nullptr && hfpObserverImp_ != nullptr) {
        hfpAgService->DeregisterObserver(*(reinterpret_cast<HfpAgServiceObserver *>(hfpObserverImp_.get())));
        hfpObserverImp_ = nullptr;
    }
}

bool BluetoothHighPowerManager::GetWifiStaPowerState()
{
    HITRACE_METER(BT_TRACE_TAG);
    if (wifiDevice_ == nullptr) {
        wifiDevice_ = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_SYS_ABILITY_ID);
    }
    int state = static_cast<int>(Wifi::WifiState::UNKNOWN);
    int ret = DEFAULT_ERROR_CODE;
    if (wifiDevice_ != nullptr) {
        ret = wifiDevice_->GetWifiState(state);
    }
    if (ret != ERR_OK) {
        HILOGE("fail to get wifi sta state. ret %{public}d", ret);
        return false;
    }
    HILOGD("get wifi sta state %{public}d", state);
    return (state == static_cast<int>(Wifi::WifiState::ENABLED)) ? true : false;
}

bool BluetoothHighPowerManager::GetWifiHotSpotPowerState()
{
    HITRACE_METER(BT_TRACE_TAG);
    if (wifiHotSpot_ == nullptr) {
        wifiHotSpot_ = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_SYS_ABILITY_ID);
    }
    int state = static_cast<int>(Wifi::ApState::AP_STATE_NONE);
    int ret = DEFAULT_ERROR_CODE;
    if (wifiHotSpot_ != nullptr) {
        ret = wifiHotSpot_->GetHotspotState(state);
    }
    if (ret != ERR_OK) {
        HILOGE("fail to get wifi hotSpot state.");
        return false;
    }
    HILOGD("get wifi hotspot state %{public}d", state);
    return (state == static_cast<int>(Wifi::ApState::AP_STATE_STARTED)) ? true : false;
}

void BluetoothHighPowerManager::InitWifiState()
{
    wifiSta5GhzActive_ = false;
    wifiSta2GhzActive_ = false;
}

void BluetoothHighPowerManager::UpdateWifiState()
{
    HITRACE_METER(BT_TRACE_TAG);
    InitWifiState();
    if (!isWifiEnable_) {
        HILOGE("wifi sta is disable no need to update");
        return;
    }
    if (wifiDevice_ == nullptr) {
        wifiDevice_ = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_SYS_ABILITY_ID);
    }

    Wifi::WifiLinkedInfo info;
    int ret = DEFAULT_ERROR_CODE;
    if (wifiDevice_ != nullptr) {
        ret = wifiDevice_->GetLinkedInfo(info);
    } else {
        HILOGE("get wifi device fail");
        return;
    }
    if (ret != ERR_OK || info.connState != Wifi::ConnState::CONNECTED) {
        HILOGE("failed to get wifi link info ret = %{public}d or is not connected %{public}d", ret, info.connState);
        return;
    }
    HILOGD("get wifi link info req = %{public}d", info.frequency);
    if (info.frequency >= BAND_5_GHZ_START_FREQ_MHZ && info.frequency <= BAND_5_GHZ_END_FREQ_MHZ) {
        wifiSta5GhzActive_ = true;
    } else {
        wifiSta2GhzActive_ = true;
    }
}

void BluetoothHighPowerManager::InitSoftApState()
{
    softAp2GhzActive_ = false;
    softAp5GhzActive_ = false;
}

void BluetoothHighPowerManager::UpdateSoftApState()
{
    HITRACE_METER(BT_TRACE_TAG);
    InitSoftApState();
    if (!isHotSpotEnable_) {
        HILOGE("wifi hotspot is disable");
        return;
    }
    if (wifiHotSpot_ == nullptr) {
        wifiHotSpot_ = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_SYS_ABILITY_ID);
    }
    std::vector<Wifi::StationInfo> staList;
    int ret = DEFAULT_ERROR_CODE;
    if (wifiHotSpot_ != nullptr) {
        ret = wifiHotSpot_->GetStationList(staList);
    } else {
        HILOGE("get wifi hotspot device fail");
        return;
    }
    if (ret != ERR_OK || staList.empty()) {
        HILOGE("wifi hotspot is not connect ret %{public}d, size %{public}u", ret, staList.size());
        return;
    }
    HILOGI("hotspot client size = %{public}u.", staList.size());
    Wifi::HotspotConfig config;
    if (wifiHotSpot_ != nullptr) {
        ret = wifiHotSpot_->GetHotspotConfig(config);
    }
    if (ret != ERR_OK) {
        HILOGE("get hotspot config failed");
        return;
    }
    if (config.GetBand() == Wifi::BandType::BAND_2GHZ) {
        softAp2GhzActive_ = true;
        HILOGI("hotspot band 2G.");
    } else {
        softAp5GhzActive_ = true;
        HILOGI("hotspot band 5G.");
    }
}

WifiScenario BluetoothHighPowerManager::GetWifiScenario()
{
    if (wifiSta2GhzActive_ || softAp2GhzActive_) {
        return WifiScenario::WIFI_2GHZ_ACTIVE;
    }
    if (wifiSta5GhzActive_ || softAp5GhzActive_) {
        return WifiScenario::WIFI_5GZ_ACTIVE;
    }
    return WifiScenario::WIFI_INACTIVE;
}

void BluetoothHighPowerManager::UpdateHighPowerModeOnBtWorking()
{
    switch (currentWifiScenario_) {
        case WifiScenario::WIFI_2GHZ_ACTIVE:
            SetHighPowerModeEnable(true);
            break;
        case WifiScenario::WIFI_INACTIVE:
            SetHighPowerModeEnable(true);
            break;
        case WifiScenario::WIFI_5GZ_ACTIVE:
            if (!isScreenOnNow_) {
                SetHighPowerModeEnable(true);
            } else { // can not get foreground app type, disable highpower when screen on in wifi 5G mode
                SetHighPowerModeEnable(false);
            }
            break;
        default:
            SetHighPowerModeEnable(true);
            break;
    }
}

void BluetoothHighPowerManager::UpdateHighPowerMode()
{
    if (isDisabledByCCM_) {
        return;
    }
    HILOGI("isScreenOn = %{public}d, btScenario_ = %{public}u, wifiScenario = %{public}u",
        isScreenOnNow_,
        static_cast<std::uint32_t>(currentBtScenario_),
        static_cast<std::uint32_t>(currentWifiScenario_));
    switch (currentBtScenario_) {
        case BluetoothScenario::BT_INACTIVE:
            SetHighPowerModeEnable(false);
            break;
        case BluetoothScenario::BT_PLAYING:
        case BluetoothScenario::BT_VOIP_CALLING:
            UpdateHighPowerModeOnBtWorking();
            break;
        case BluetoothScenario::BT_CELLULAR_CALLING:
            SetHighPowerModeEnable(true);
            break;
        default:
            SetHighPowerModeEnable(false);
            break;
    }
}

void BluetoothHighPowerManager::HandleBluetoothStateChange(const int bluetoothState)
{
    if (isDisabledByCCM_) {
        return;
    }
    isBluetoothEnable_ = (bluetoothState == bluetooth::BTStateID::STATE_TURN_ON) ? true : false;
    currentBtScenario_ = BluetoothScenario::BT_INACTIVE;
    if (!isBluetoothEnable_) {
        SetHighPowerModeEnable(false);
        DeRegisterHfpAgObserver();
        DeRegisterA2dpObserver();
        return;
    }
    RegisterHfpAgObserver();
    RegisterA2dpObserver();
    UpdateWifiState();
    UpdateSoftApState();
    currentWifiScenario_ = GetWifiScenario();
    UpdateHighPowerMode();
    return;
}

void BluetoothHighPowerManager::HandleScreenStateChange(bool isScreenOn)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    if (isScreenOnNow_ != isScreenOn) {
        isScreenOnNow_ = isScreenOn;
        UpdateHighPowerMode();
    }
}

BtHighPowerEventType BluetoothHighPowerManager::GetEventType(std::string &event) const
{
    auto iter = btHighPowerEventType_.find(event);
    if (iter == btHighPowerEventType_.end()) {
        return BtHighPowerEventType::UNKNOWN;
    }
    return iter->second;
}

void BluetoothHighPowerManager::HandleWifiStateChange(const int staState)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    isWifiEnable_ = (staState == static_cast<int>(Wifi::WifiState::ENABLED)) ? true : false;
    HILOGI("staState %{public}d ", staState);
    UpdateWifiState();
    UpdateSoftApState();
    WifiScenario newScenario = GetWifiScenario();
    if (currentWifiScenario_ != newScenario) {
        currentWifiScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void BluetoothHighPowerManager::HandleWifiConnectStateChange(const int wifiConnState)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    isWifiConnected_ = (wifiConnState == OHOS::Wifi::ConnState::CONNECTED) ? true : false;
    UpdateWifiState();
    WifiScenario newScenario = GetWifiScenario();
    if (currentWifiScenario_ != newScenario) {
        currentWifiScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void BluetoothHighPowerManager::HandleHotSpotStateChange(const int hotSpotState)
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    isHotSpotEnable_ = (hotSpotState == static_cast<int>(Wifi::ApState::AP_STATE_STARTED)) ? true : false;
    HILOGI("get hotspot power state %{public}d", hotSpotState);
    UpdateSoftApState();
    WifiScenario newScenario = GetWifiScenario();
    if (currentWifiScenario_ != newScenario) {
        currentWifiScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void BluetoothHighPowerManager::HandleHotSpotConnectStateChange()
{
    if (isDisabledByCCM_ || !isBluetoothEnable_) {
        return;
    }
    UpdateSoftApState();
    WifiScenario newScenario = GetWifiScenario();
    if (currentWifiScenario_ != newScenario) {
        currentWifiScenario_ = newScenario;
        UpdateHighPowerMode();
    }
}

void BluetoothHighPowerManager::handleCommEvent(const OHOS::EventFwk::CommonEventData &data)
{
    if (isDisabledByCCM_) {
        return;
    }
    auto want = data.GetWant();
    std::string action = want.GetAction();
    switch (GetEventType(action)) {
        case BtHighPowerEventType::WIFI_POWER_STATE:
            HandleWifiStateChange(data.GetCode());
            break;
        case BtHighPowerEventType::WIFI_CONN_STATE:
            HandleWifiConnectStateChange(data.GetCode());
            break;
        case BtHighPowerEventType::WIFI_HOTSPOT_STATE:
            HandleHotSpotStateChange(data.GetCode());
            break;
        case BtHighPowerEventType::WIFI_AP_STA_JOIN:
        case BtHighPowerEventType::WIFI_AP_STA_LEAVE:
            HandleHotSpotConnectStateChange();
            break;
        case BtHighPowerEventType::SCREEN_ON:
            HandleScreenStateChange(true);
            break;
        case BtHighPowerEventType::SCREEN_OFF:
            HandleScreenStateChange(false);
            break;
        case BtHighPowerEventType::BLUETOOTH_HOST_STATE_UPDATE:
            HandleBluetoothStateChange(data.GetCode());
            break;
        default:
            HILOGE("unhandled event");
            break;
    }
    return;
}

void BluetoothHighPowerManager::SetHighPowerModeEnable(bool enable)
{
    if (enable == lastHighPowerMode_) {
        HILOGD("No change, ignore");
        return;
    }
    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0 || btInterface == nullptr) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d.", status);
        return;
    }
    if (btInterface->enableBluetoothHighpower != nullptr) {
        lastHighPowerMode_ = enable;
        HILOGI("SetHighPowerModeEnable = %{public}d.", enable);
        btInterface->enableBluetoothHighpower(enable);
    }
}
} //namespace bluetooth
} //namespace OHOS
