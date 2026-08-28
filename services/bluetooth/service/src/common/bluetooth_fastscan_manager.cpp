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
#include "bluetooth_fastscan_manager.h"
#undef LOG_TAG
#define LOG_TAG "bt_fastscan_manager"
#include "bt_def.h"
#include "hal_util.h"
#include "bluetooth_hw_interface.h"

#include "app_mgr_constants.h"
#include "iservice_registry.h"
#include "ability_manager_client.h"
#include "system_ability_definition.h"

#include "bt_av.h"
#include "wifi_device.h"
#include "wifi_msg.h"
#include "power_mgr_client.h"

#include "log.h"
#include "log_utils.h"
#include "thread_util.h"
#include "hitrace_meter.h"
#include "parameter.h"

#ifdef APP_MANAGER_ENABLE
#include "appgallery_service_client_appinfo_category.h"
#include "appgallery_service_client_param.h"
#endif

namespace OHOS {
namespace bluetooth {

using OHOS::EventFwk::CommonEventSupport;

static constexpr uint32_t CORE_EVENT_PRIORITY = 1;
static constexpr int32_t DEFAULT_ERROR_CODE = -1;
static constexpr int32_t ERR_OK = 0;
static std::unordered_set<std::string> BTFASTSCAN_WHITE_APP_SET = {
    "com.ohos.sceneboard"
};

const char *BLUETOOTH_SWITCH_NAME = "persist.bluetooth.switch_enable";

constexpr int32_t MAX_FOREGROUND_APP_SIZE = 100;
constexpr int32_t GAME_APP_PRIMARY_CATEGORY_ID = 2;
constexpr int32_t OTHER_APP_PRIMARY_CATEGORY_ID = 13;
constexpr int32_t VIDEO_APP_SECONDARY_CATEGORY_ID = 10000013;
constexpr int32_t MUSIC_APP_SECONDARY_CATEGORY_ID = 10000014;
constexpr const char* SERVICE_NAME = "bluetooth_service";

void BluetoothFastScanManager::Init()
{
    HILOGI("init");
    if (isInitialized_.load()) {
        HILOGI("has initialized, no need to init.");
        return;
    }

    isBluetoothEnable_ = GetIntParameter(BLUETOOTH_SWITCH_NAME, 0) == 1 ? true : false;
    btFastScanEventType_ = {
        {CommonEventSupport::COMMON_EVENT_SCREEN_ON, BtFastScanEventType::SCREEN_ON},
        {CommonEventSupport::COMMON_EVENT_SCREEN_OFF, BtFastScanEventType::SCREEN_OFF},
        {CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE, BtFastScanEventType::WIFI_POWER_STATE},
        {CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE,
            BtFastScanEventType::BLUETOOTH_HOST_STATE_UPDATE},
        {CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED, BtFastScanEventType::CALL_STATE},
    };

    RegisterCommonEvent();
    if (!isBluetoothEnable_) {
        SetFastScan(false);
        HILOGI("bluetoothSwitchState is not 1(on), no need to readStatus.");
        return;
    }

    RegisterAppObserver();

    isWifiOn_ = GetWifiStaPowerState();
    isScreenOn_ = OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    isCallStateIdle_ = true;
    isA2dpPlaying_ = IsA2dpPlaying();
    if (a2dpServiceFuncs_ != nullptr) {
        a2dpServiceFuncs_->setA2dpPlayingCb([](int state) {
            BluetoothFastScanManager::GetInstance().HandleA2dpPlayingStateChange(state);
        });
    }
    HandleForegroundAppInfoAfterBtOn();
    isInitialized_.store(true);
    UpdateFastScanMode();
    return;
}

void BluetoothFastScanManager::SetFastScanEnabled(bool enable)
{
    isFastScanEnabled_ = enable;
}

bool BluetoothFastScanManager::GetWifiStaPowerState()
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
        HILOGE("fail to get state, ret %{public}d", ret);
        return false;
    }
    HILOGD("state %{public}d", state);
    return (state == static_cast<int>(Wifi::WifiState::ENABLED)) ? true : false;
}

BluetoothFastScanManager& BluetoothFastScanManager::GetInstance()
{
    static BluetoothFastScanManager instance;
    return instance;
}

void BluetoothFastScanManager::RegisterAppObserver()
{
    appStateObserverImp_ = std::make_unique<AppStateObserver>();
    if (appStateObserverImp_ != nullptr) {
        appStateObserverImp_->SubscribeAppState();
    }
}

void BluetoothFastScanManager::DeRegisterAppObserver()
{
    if (appStateObserverImp_ == nullptr) {
        return;
    }
    appStateObserverImp_->UnSubscribeAppState();
}

void BluetoothFastScanManager::RegisterCommonEvent()
{
    if (subscriber_ != nullptr) {
        return;
    }

    HITRACE_METER(BT_TRACE_TAG);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_POWER_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    subscribeInfo.SetPriority(CORE_EVENT_PRIORITY);
    subscriber_ = std::make_shared<BluetoothFastScanCommonEventSubscriber>(subscribeInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        HILOGE("SubscribeCommonEvent fail.");
        subscriber_ = nullptr;
        return;
    }
    HILOGI("SubscribeCommonEvent successful");
}

void BluetoothFastScanManager::UnRegisterCommonEvent()
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("UnRegisterCommonEvent");
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

 sptr<AppExecFwk::IAppMgr> AppStateObserver::GetAppMgrProxy()
 {
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        HILOGE("remote object is nullptr");
        return nullptr;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if (!appMgrProxy || !appMgrProxy->AsObject()) {
        HILOGE("get appMgrProxy failed!");
        return nullptr;
    }

    return appMgrProxy;
}

bool AppStateObserver::SubscribeAppState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (appStateAwareObserver_) {
        HILOGE("appStateAwareObserver_ has register");
        return false;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = GetAppMgrProxy();
    if (appMgrProxy == nullptr) {
        HILOGE("appMgrProxy is nullptr");
        return false;
    }
    appStateAwareObserver_ = new (std::nothrow)AppStateAwareObserver();
    if (appStateAwareObserver_ == nullptr) {
        HILOGE("appStateAwareObserver_ is nullptr");
        return false;
    }
    auto err = appMgrProxy->RegisterApplicationStateObserver(appStateAwareObserver_);
    if (err != 0) {
        HILOGE("error, code = %{public}d", err);
        appStateAwareObserver_ = nullptr;
        return false;
    }
    return true;
}

bool AppStateObserver::UnSubscribeAppState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!appStateAwareObserver_) {
        HILOGE("appStateAwareObserver_ is nullptr");
        return false;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = GetAppMgrProxy();
    if (appMgrProxy) {
        appMgrProxy->UnregisterApplicationStateObserver(appStateAwareObserver_);
        appMgrProxy = nullptr;
        appStateAwareObserver_ = nullptr;
    }
    return true;
}

void AppStateObserver::AppStateAwareObserver::OnForegroundApplicationChanged(
    const AppExecFwk::AppStateData &appStateData)
{
    if (!ValidateAppStateData(appStateData)) {
        HILOGI("validate app state data failed");
        return;
    }
    if (appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND)) {
        DoInLowPriorityThread([appStateData]() {
            BluetoothFastScanManager::GetInstance().OnForegroundApplicationChanged(appStateData);
        });
    }
}

inline bool AppStateObserver::AppStateAwareObserver::ValidateAppStateData(const AppExecFwk::AppStateData &appStateData)
{
    return appStateData.uid > 0 && appStateData.bundleName.length() > 0;
}

std::vector<OHOS::AppExecFwk::AppStateData> BluetoothFastScanManager::GetForegroundApplications()
{
    std::vector<OHOS::AppExecFwk::AppStateData> apps;
    if (appStateObserverImp_ == nullptr) {
        HILOGE("get appStateObserverImp_ failed!");
        return apps;
    }
    sptr<AppExecFwk::IAppMgr> appMgrProxy = appStateObserverImp_->GetAppMgrProxy();
    if (appMgrProxy == nullptr) {
        HILOGE("get appMgrProxy failed!");
        return apps;
    }
    appMgrProxy->GetForegroundApplications(apps);
    return apps;
}

AppCategory BluetoothFastScanManager::GetAppTypeByBundleName(std::string bundleName)
{
    AppCategory type = APP_CATEGORY_UNKNOWN;
    auto it = appCategoryMap_.find(bundleName);
    if (it != appCategoryMap_.end()) {
        type = it->second;
        return type;
    } else if (appCategoryMap_.size() >= MAX_FOREGROUND_APP_SIZE) {
        HILOGI("foreground apps size is max, clear");
        appCategoryMap_.clear();
    }
    type = GetAppCategory(bundleName);
    appCategoryMap_[bundleName] = type;
    return type;
}

AppCategory BluetoothFastScanManager::GetForegroundAppType(const std::string &bundleName,
    const std::vector<OHOS::AppExecFwk::AppStateData> &apps)
{
    AppCategory appType = GetAppTypeByBundleName(bundleName);
    if (appType == APP_CATEGORY_GAME) {
        return appType;
    }

    if (apps.empty()) {
        return appType;
    }
    HILOGI_TIME_LIMIT(bundleName.c_str(), "app:%{public}s, type:%{public}d", bundleName.c_str(),
        static_cast<int>(appType));
    std::string preferredAppName = bundleName;

    for (auto it = apps.rbegin(); it != apps.rend(); ++it) {
        auto curApp = *it;
        AppCategory curType = GetAppTypeByBundleName(curApp.bundleName);
        // 如果前台的所有应用中存在游戏，优先取“游戏”作为当前音频场景
        if (curType == APP_CATEGORY_GAME) {
            HILOGI("find app:%{public}s in foreground apps, change type to GAME", curApp.bundleName.c_str());
            return curType;
        }
        // 如果前台的所有应用中存在视频且无游戏，则遍历完成后，取“视频”作为当前音频场景
        if (curType == APP_CATEGORY_VIDEO) {
            preferredAppName = curApp.bundleName;
            appType = curType;
        }
        // 如果前台的所有应用不存在游戏和视频，但存在音乐，则以音乐作为当前音频场景
        if (curType == APP_CATEGORY_MUSIC && appType != APP_CATEGORY_VIDEO) {
            preferredAppName = curApp.bundleName;
            appType = curType;
        }
    }
    if (preferredAppName != bundleName) {
        HILOGI("preferred foreground app changed to:%{public}s, change type to %{public}d",
            preferredAppName.c_str(), appType);
    }
    return appType;
}

void BluetoothFastScanManager::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    std::string bundleName = appStateData.bundleName;
    bool isWhiteApp = BTFASTSCAN_WHITE_APP_SET.find(bundleName) != BTFASTSCAN_WHITE_APP_SET.end();
    std::vector<OHOS::AppExecFwk::AppStateData> apps = GetForegroundApplications();
    AppCategory type = GetForegroundAppType(bundleName, apps);
    if (!isWhiteApp && type == APP_CATEGORY_MUSIC) {
        isWhiteApp = true;
    }
    isWhiteApp_ = isWhiteApp;
    UpdateFastScanMode();

    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status || btInterface == nullptr) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return;
    }
    const BthwifInterface *bthwif =
        reinterpret_cast<const BthwifInterface*>(btInterface->getProfileInterface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("bthwif is nullptr");
        return;
    }
    HILOGD("foreground app type update, type:%{public}d", static_cast<int>(type));
    bthwif->hwSendSceneType(static_cast<int>(type));
    if (lastTopAppType_.load() != type) {
        lastTopAppType_ = type;
        if (type != APP_CATEGORY_UNKNOWN) {
            hfpAgServiceFuncs_ ? hfpAgServiceFuncs_->sendAtCmdFunc(type) : HILOGD("hfpAgServiceFuncs_ is null");
        }
    }
}

void BluetoothFastScanManager::HandleForegroundAppInfoAfterBtOn()
{
    std::vector<OHOS::AppExecFwk::AppStateData> apps = GetForegroundApplications();
    if (apps.empty()) {
        HILOGE("no foreground app");
        return;
    }

    for (auto it = apps.rbegin(); it != apps.rend(); ++it) {
        auto curApp = *it;
        if (BTFASTSCAN_WHITE_APP_SET.find(curApp.bundleName) != BTFASTSCAN_WHITE_APP_SET.end()) {
            HILOGI("curApp, name = %{public}s.", curApp.bundleName.c_str());
            isWhiteApp_ = true;
            break;
        }
    }

    AppCategory type = GetForegroundAppType(apps[0].bundleName, apps);
    HILOGE("foreground app:%{public}s type:%{public}d", apps[0].bundleName.c_str(), static_cast<int>(type));
    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status || btInterface == nullptr) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return;
    }
    const BthwifInterface *bthwif =
        reinterpret_cast<const BthwifInterface*>(btInterface->getProfileInterface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("bthwif is nullptr");
        return;
    }
    bthwif->hwSendSceneType(static_cast<int>(type));
}

AppCategory BluetoothFastScanManager::GetAppCategory(std::string bundleName) const
{
#ifdef APP_MANAGER_ENABLE
    HITRACE_METER(BT_TRACE_TAG);
    std::vector<AppGalleryServiceClient::AppCategoryInfo> resultArray;
    std::vector<std::string> bundleNames;
    bundleNames.push_back(bundleName);

    HILOGI("bundleName: %{public}s", bundleName.c_str());
    std::string tag(SERVICE_NAME);
    int32_t ret = AppGalleryServiceClient::CategoryManager::GetCategoryFromSystem(bundleNames, resultArray, tag);
    if (ret != 0) {
        HILOGI("Get category failed, ret %{public}d", ret);
        return APP_CATEGORY_UNKNOWN;
    }

    for (AppGalleryServiceClient::AppCategoryInfo &item : resultArray) {
        if (item.primaryCategoryId == GAME_APP_PRIMARY_CATEGORY_ID) {
            return APP_CATEGORY_GAME;
        } else if (item.primaryCategoryId == OTHER_APP_PRIMARY_CATEGORY_ID) {
            if (item.secondaryCategoryId == VIDEO_APP_SECONDARY_CATEGORY_ID) {
                return APP_CATEGORY_VIDEO;
            } else if (item.secondaryCategoryId == MUSIC_APP_SECONDARY_CATEGORY_ID) {
                return APP_CATEGORY_MUSIC;
            }
        }
    }
    return APP_CATEGORY_UNKNOWN;
#else
    return APP_CATEGORY_UNKNOWN;
#endif
}

void BluetoothFastScanCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    DoInLowPriorityThread([data]() {
        BluetoothFastScanManager::GetInstance().HandleCommEvent(data);
    });
}

BtFastScanEventType BluetoothFastScanManager::GetEventType(std::string &event) const
{
    auto iter = btFastScanEventType_.find(event);
    if (iter == btFastScanEventType_.end()) {
        return BtFastScanEventType::UNKNOWN;
    }
    return iter->second;
}

void BluetoothFastScanManager::HandleCommEvent(const OHOS::EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    switch (GetEventType(action)) {
        case BtFastScanEventType::WIFI_POWER_STATE:
            HandleWifiStateChange(data.GetCode());
            break;
        case BtFastScanEventType::SCREEN_ON:
            HandleScreenStateChange(true);
            break;
        case BtFastScanEventType::SCREEN_OFF:
            HandleScreenStateChange(false);
            break;
        case BtFastScanEventType::BLUETOOTH_HOST_STATE_UPDATE:
            HandleBluetoothStateChange(data.GetCode());
            break;
        case BtFastScanEventType::CALL_STATE:
            HandleCallStateChange(want.GetIntParam("state", DEFAULT_CALL_STATE));
            break;
        default:
            HILOGE("unhandled event");
            break;
    }
    return;
}

void BluetoothFastScanManager::HandleScreenStateChange(bool isScreenOn)
{
    if (!isBluetoothEnable_) {
        return;
    }

    isScreenOn_ = isScreenOn;
    UpdateFastScanMode();
}

void BluetoothFastScanManager::HandleWifiStateChange(const int staState)
{
    if (!isBluetoothEnable_) {
        return;
    }

    bool isWifiOn = (staState == static_cast<int>(Wifi::WifiState::ENABLED)) ? true : false;
    isWifiOn_ = isWifiOn;
    UpdateFastScanMode();
}

void BluetoothFastScanManager::HandleCallStateChange(const int callState)
{
    HILOGI("enter HandleCallStateChange callstate: %{public}d", callState);
    if (!isBluetoothEnable_) {
        return;
    }
    bool isCallStateIdle = false;
    if (callState == CALL_STATUS_DISCONNECTED) {
        isCallStateIdle = true;
    }
    isCallStateIdle_ = isCallStateIdle;
    UpdateFastScanMode();
}

void BluetoothFastScanManager::HandleA2dpPlayingStateChange(const int a2dpPlayingState)
{
    if (!isBluetoothEnable_) {
        return;
    }
    bool isA2dpPlaying = false;
    if (a2dpPlayingState == static_cast<int>(BTAV_AUDIO_STATE_STARTED)) {
        isA2dpPlaying = true;
    }
    isA2dpPlaying_ = isA2dpPlaying;
    UpdateFastScanMode();
}

void BluetoothFastScanManager::HandleBluetoothStateChange(const int bluetoothState)
{
    bool isBluetoothEnable = (bluetoothState == bluetooth::BTStateID::STATE_TURN_ON) ? true : false;
    if (isBluetoothEnable_ == isBluetoothEnable) {
        HILOGE("No change, ignore");
        return;
    }

    isBluetoothEnable_ = isBluetoothEnable;

    if (!isBluetoothEnable_) {
        SetFastScan(false);
        DeRegisterAppObserver();
        return;
    }

    RegisterAppObserver();
    isWifiOn_ = GetWifiStaPowerState();
    isScreenOn_ = OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    isCallStateIdle_ = true;
    isA2dpPlaying_ = IsA2dpPlaying();
    HandleForegroundAppInfoAfterBtOn();
    UpdateFastScanMode();
    return;
}

void BluetoothFastScanManager::SetFastScan(bool isEnable)
{
    if (isEnable == isFastScanEnabled_) {
        HILOGD("No change, ignore");
        return;
    }

    HILOGD("isEnable: %{public}d. ", isEnable);
    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return;
    }
    if (btInterface == nullptr) {
        HILOGE("get btInterface failed!");
        return;
    }
    btInterface->enableFastScan(isEnable);
    isFastScanEnabled_ = isEnable;
}

bool BluetoothFastScanManager::IsA2dpPlaying()
{
    if (a2dpServiceFuncs_ == nullptr) {
        return false;
    }
    return a2dpServiceFuncs_->isA2dpPlayingCb();
}

bool BluetoothFastScanManager::HasA2dpConnectedDevicesForCurrentMobile()
{
    if (a2dpServiceFuncs_ == nullptr) {
        return false;
    }
    return a2dpServiceFuncs_->hasA2dpConnectedDevicesForCurrentMobilefunc();
}

bool BluetoothFastScanManager::HasHfpConnectedDevicesForCurrentMobile()
{
    if (hfpAgServiceFuncs_ == nullptr) {
        return false;
    }
    return hfpAgServiceFuncs_->hasHfpConnectedDevicesForCurrentMobilefunc();
}

void BluetoothFastScanManager::UpdateFastScanMode()
{
    bool isFastScanScenes = IsAbleToFastScan();
    HILOGD("isFastScanScenes: %{public}d", isFastScanScenes);
    SetFastScan(isFastScanScenes);
}

bool BluetoothFastScanManager::IsAbleToFastScan()
{
    if (!isBluetoothEnable_) {
        HILOGI_TIME_LIMIT(__func__, "Bluetooth is off");
        return false;
    }

    if (!isScreenOn_) {
        HILOGI_TIME_LIMIT(__func__, "Screen is off");
        return false;
    }

    if (isA2dpPlaying_.load()) {
        HILOGI_TIME_LIMIT(__func__, "a2dp is playing");
        return false;
    }

    if (!isWifiOn_) {
        HILOGI_TIME_LIMIT(__func__, "Wifi is off");
        return true;
    }

    if (!isCallStateIdle_) {
        HILOGI_TIME_LIMIT(__func__, "call state is not idle");
        return true;
    }

    if (!isWhiteApp_) {
        HILOGI_TIME_LIMIT(__func__, "Not is whiteApp");
        return false;
    }
    return true;
}

BluetoothFastScanManager::~BluetoothFastScanManager()
{
    HILOGI("deinit");
    DeRegisterAppObserver();
    UnRegisterCommonEvent();
    isInitialized_.store(false);
}

extern "C" {
void InitFastScan(void)
{
    BluetoothFastScanManager::GetInstance().Init();
    return;
}

void InitFastScanAndSetCallback(HfpAgServiceFuncs* hfpCallback, A2dpServiceFuncs* a2dpCallback)
{
    BluetoothFastScanManager::GetInstance().RegisterHfpAgServiceCb(hfpCallback);
    BluetoothFastScanManager::GetInstance().RegisterA2dpServiceCb(a2dpCallback);
    BluetoothFastScanManager::GetInstance().Init();
    return;
}

void SetFastScanEnabled(bool enable)
{
    BluetoothFastScanManager::GetInstance().SetFastScanEnabled(enable);
}
}
} // namespace bluetooth
} // namespace OHOS
