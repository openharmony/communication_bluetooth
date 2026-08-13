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
#ifndef BLUETOOTH_FASTSCAN_MANAGER_H
#define BLUETOOTH_FASTSCAN_MANAGER_H
#define DELAY_PAGE_TIME 10000
#define SCAN_LEVEL_FASTEST 2
#define SCAN_LEVEL_STANDARD 0

#include <mutex>
#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "iremote_object.h"
#include "bluetooth_types.h"


#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

#include "wifi_device.h"
#include <string>
#include "bluetooth_hw_interface_hook.h"
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {

enum class BtFastScanEventType : std::uint32_t {
    SCREEN_ON,
    SCREEN_OFF,
    WIFI_POWER_STATE,
    BLUETOOTH_HOST_STATE_UPDATE,
    CALL_STATE,
    UNKNOWN,
 };

enum AppCategory {
    APP_CATEGORY_UNKNOWN = -1,
    APP_CATEGORY_INSTANT = 0,
    AAPP_CATEGORY_CLOCK = 5,
    APP_CATEGORY_READER = 6,
    APP_CATEGORY_MUSIC = 7,
    APP_CATEGORY_VIDEO = 8,
    APP_CATEGORY_GAME = 9,
    AAPP_CATEGORY_LIVE = 21,
    AAPP_CATEGORY_NEWS = 26,
    APP_CATEGORY_SMS = 27,
    APP_CATEGORY_CALL = 32,
    APP_CATEGORY_OTHER = 255,
};

class AppStateObserver {
public:
    AppStateObserver() = default;
    bool SubscribeAppState();
    bool UnSubscribeAppState();
    sptr<AppExecFwk::IAppMgr> GetAppMgrProxy();

private:
    class AppStateAwareObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;
    private:
        inline bool ValidateAppStateData(const AppExecFwk::AppStateData &appStateData);
    };

    std::mutex mutex_{};
    sptr<AppStateAwareObserver> appStateAwareObserver_;

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AppStateObserver);
};

class BluetoothFastScanCommonEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit BluetoothFastScanCommonEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~BluetoothFastScanCommonEventSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class BluetoothFastScanManager {
public:
    static BluetoothFastScanManager& GetInstance();
    void Init();
    void SetFastScanEnabled(bool enable);
    std::vector<OHOS::AppExecFwk::AppStateData> GetForegroundApplications();
    void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData);
    void HandleA2dpPlayingStateChange(const int a2dpPlayingState);
    void HandleCommEvent(const OHOS::EventFwk::CommonEventData &data);
    AppCategory GetAppCategory(std::string bundleName) const;
    void RegisterHfpAgServiceCb(HfpAgServiceFuncs* hfpCallback) {
        hfpAgServiceFuncs_ = hfpCallback;
    }
    void RegisterA2dpServiceCb(A2dpServiceFuncs* a2dpCallback) {
        a2dpServiceFuncs_ = a2dpCallback;
    }
private:
    BluetoothFastScanManager() = default;
    ~BluetoothFastScanManager();
    bool GetWifiStaPowerState();
    void RegisterAppObserver();
    void DeRegisterAppObserver();
    void RegisterCommonEvent();
    void UnRegisterCommonEvent();

    void HandleScreenStateChange(bool isScreenOn);
    void HandleWifiStateChange(const int staState);
    void HandleBluetoothStateChange(const int bluetoothState);
    void HandleCallStateChange(const int callState);
    void SetFastScan(bool isEnable);
    void UpdateFastScanMode();
    bool IsAbleToFastScan();
    BtFastScanEventType GetEventType(std::string &event) const;
    void HandleForegroundAppInfoAfterBtOn();
    AppCategory GetAppTypeByBundleName(std::string bundleName);
    AppCategory GetForegroundAppType(const std::string &bundleName,
        const std::vector<OHOS::AppExecFwk::AppStateData> &apps);
    bool IsA2dpPlaying();
    bool HasHfpConnectedDevicesForCurrentMobile();
    bool HasA2dpConnectedDevicesForCurrentMobile();

    std::shared_ptr<BluetoothFastScanCommonEventSubscriber> subscriber_ = nullptr;
    std::unique_ptr<AppStateObserver> appStateObserverImp_ = nullptr;
    std::atomic<bool> isInitialized_ = false;
    bool isBluetoothEnable_ = false;
    bool isFastScanEnabled_ = false;
    bool isScreenOn_ = false;
    bool isWifiOn_ = false;
    bool isWhiteApp_ = false;
    bool isCallStateIdle_ = false;
    bool isFastestScanEnable_ = false;
    std::atomic<bool> isA2dpPlaying_ = false;
    std::shared_ptr<Wifi::WifiDevice> wifiDevice_ = nullptr;
    std::map<std::string, AppCategory> appCategoryMap_ {};
    std::atomic<AppCategory> lastTopAppType_ = APP_CATEGORY_UNKNOWN;
    HfpAgServiceFuncs* hfpAgServiceFuncs_ = nullptr;
    A2dpServiceFuncs* a2dpServiceFuncs_ = nullptr;
    const int CALL_STATUS_DISCONNECTED = 6;
    const int CALL_STATUS_DIALING = 2;
    const int CALL_STATUS_INCOMING = 4;
    const int DEFAULT_CALL_STATE = -1;
    std::shared_ptr<utility::Timer> delayFastScanTimer_ = nullptr;

    std::unordered_map<std::string, BtFastScanEventType> btFastScanEventType_ = {};
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothFastScanManager);
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_FASTSCAN_MANAGER_H