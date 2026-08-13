/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_policy_dispatcher.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_dispatcher"
#endif
#include "log.h"

#include "thread_util.h"

#include "a2dp_snk_service.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_def.h"
#include "car_policy_manager.h"
#include "common_util.h"
#include "hfp_hf_service.h"
#include "interface_profile_a2dp_snk.h"
#include "interface_profile_hfp_hf.h"
#include "raw_address.h"
#include "uri.h"

#include "adapter_manager.h"
#include "interface_adapter_classic.h"

#include <string>

namespace OHOS {
namespace bluetooth {

namespace {
const char *const SETTINGSDATA_SCT_KEY = "SMART_CALL_TRANSFER_STATUS";
const char *const SETTINGSDATA_BT_SETTINGS_UI_KEY = "IS_BT_SETTINGS_ACTIVITY";
const char *const SETTINGSDATA_FA_KEY = "FA_MEDIA_TYPE";
const char *const SETTINGSDATA_BT_MUSIC_KEY = "BT_MUSIC_STATUS";
constexpr int ACL_STATE_DISCONNECTED = 0x00;
constexpr int SCT_STATUS_INVALID = -1;

/// 字符串转 int：失败时回退到 fallback。ConvertStrToDigit 失败时不修改 ret，故显式回退。
template <typename T>
T SafeParseInt(const std::string &value, T fallback)
{
    if (value.empty()) {
        return fallback;
    }
    T result = fallback;
    if (!ConvertStrToDigit(value, result)) {
        return fallback;
    }
    return result;
}
}  // namespace

// ── Profile Observer 实现类 ──

class CarPolicyDispatcher::HfpHfObserverImpl : public HfpHfServiceObserver {
public:
    explicit HfpHfObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~HfpHfObserverImpl() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnProfileStateChanged(addr, ProfileType::HFP_HF, state, -1);
        });
    }
    void OnCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call) override
    {
        std::string addr = device.GetAddress();
        int callState = call.GetState();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, callState]() {
            mgr->OnCallChanged(addr, callState);
        });
    }
    void OnScoStateChanged(const RawAddress &device, int state) override
    {
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnScoSwitchChanged(addr, state);
        });
    }

private:
    CarPolicyManager &owner_;
};

class CarPolicyDispatcher::A2dpSnkObserverImpl : public IA2dpSnkObserver {
public:
    explicit A2dpSnkObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~A2dpSnkObserverImpl() override = default;
    void OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause) override
    {
        std::string addr = remoteAddr.GetAddress();
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, state]() {
            mgr->OnProfileStateChanged(addr, ProfileType::A2DP_SINK, state, -1);
        });
    }

private:
    CarPolicyManager &owner_;
};

// ── Classic 远端设备观察者 ──

class CarPolicyDispatcher::ClassicRemoteDeviceObserverImpl : public IClassicRemoteDeviceObserver {
public:
    explicit ClassicRemoteDeviceObserverImpl(CarPolicyManager &owner) : owner_(owner) {}
    ~ClassicRemoteDeviceObserverImpl() override = default;
    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override
    {
        if (state != ACL_STATE_DISCONNECTED) {
            return;
        }
        std::string addr = device.GetAddress();
        CarPolicyManager *mgr = &owner_;
        int rawReason = static_cast<int>(reason);
        DoInCarPolicyThread([mgr, addr, rawReason]() {
            mgr->OnAclDisconnect(addr, rawReason);
        });
    }
    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
                              const std::string &causeMessage) override
    {
        std::string addr = device.GetAddress();
        int newState = status;
        CarPolicyManager *mgr = &owner_;
        DoInCarPolicyThread([mgr, addr, newState]() {
            mgr->OnBondStateChanged(addr, newState);
        });
    }
    void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override {}
    void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override {}
    void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override {}
    void OnRemoteCodChanged(const RawAddress &device, int cod) override {}
    void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override {}
    void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) override {}
private:
    CarPolicyManager &owner_;
};

// ── CarPolicyDispatcher 实现 ──

CarPolicyDispatcher::CarPolicyDispatcher(CarPolicyManager &owner) : owner_(owner) {}

CarPolicyDispatcher::~CarPolicyDispatcher()
{
    UnregisterAll();
}

void CarPolicyDispatcher::Dispatch(std::function<void()> task)
{
    DoInCarPolicyThread(std::move(task));
}

void CarPolicyDispatcher::DispatchDelayed(std::function<void()> task, uint64_t delayMs)
{
    DoInCarPolicyThread(std::move(task), delayMs);
}

void CarPolicyDispatcher::RegisterAll()
{
    RegisterProfileObservers();
    RegisterClassicRemoteDeviceObserver();
    RegisterFaBtMusicObservers();
    RegisterSmartCallTransferObserver();
    RegisterBtSettingsUiObserver();
}

void CarPolicyDispatcher::UnregisterAll()
{
    DeregisterProfileObservers();
    DeregisterClassicRemoteDeviceObserver();
    UnregisterFaBtMusicObservers();
    UnregisterSmartCallTransferObserver();
    UnregisterBtSettingsUiObserver();
}

void CarPolicyDispatcher::RegisterProfileObservers()
{
    auto *hfpSvc = HfpHfService::GetService();
    if (hfpSvc != nullptr) {
        hfpObserver_ = std::make_unique<HfpHfObserverImpl>(owner_);
        hfpSvc->RegisterObserver(*hfpObserver_);
        HILOGI("HFP-HF observer registered");
    } else {
        HILOGE("HfpHfService null, observer not registered");
    }
    auto *a2dpSvc = A2dpSnkService::GetService();
    if (a2dpSvc != nullptr) {
        a2dpObserver_ = std::make_unique<A2dpSnkObserverImpl>(owner_);
        a2dpSvc->RegisterObserver(a2dpObserver_.get());
        HILOGI("A2DP-Sink observer registered");
    } else {
        HILOGE("A2dpSnkService null, observer not registered");
    }
}

void CarPolicyDispatcher::DeregisterProfileObservers()
{
    if (hfpObserver_ != nullptr) {
        auto *hfpSvc = HfpHfService::GetService();
        if (hfpSvc != nullptr) {
            hfpSvc->DeregisterObserver(*hfpObserver_);
        }
        hfpObserver_.reset();
    }
    if (a2dpObserver_ != nullptr) {
        auto *a2dpSvc = A2dpSnkService::GetService();
        if (a2dpSvc != nullptr) {
            a2dpSvc->DeregisterObserver(a2dpObserver_.get());
        }
        a2dpObserver_.reset();
    }
}

void CarPolicyDispatcher::RegisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer)
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGE("RegisterSettingsObserver: helper utils is null, key=%{public}s", key.c_str());
        return;
    }
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("RegisterSettingsObserver: invalid userId, skip key=%{public}s", key.c_str());
        return;
    }
    Uri uri(uriStr);
    if (!dataShareHelperUtils->RegisterObserver(uri, observer)) {
        HILOGE("RegisterSettingsObserver: RegisterObserver failed, key=%{public}s", key.c_str());
        return;
    }
    HILOGI("RegisterSettingsObserver: key=%{public}s registered", key.c_str());
}

void CarPolicyDispatcher::UnregisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer)
{
    if (observer == nullptr) {
        return;
    }
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGE("UnregisterSettingsObserver: helper utils is null, key=%{public}s", key.c_str());
        return;
    }
    std::string uriStr = BluetoothBuildSettingsUri(key);
    if (uriStr.empty()) {
        HILOGE("UnregisterSettingsObserver: invalid userId, skip key=%{public}s", key.c_str());
        return;
    }
    Uri uri(uriStr);
    dataShareHelperUtils->UnregisterObserver(uri, observer);
    HILOGI("UnregisterSettingsObserver: key=%{public}s unregistered", key.c_str());
}

void CarPolicyDispatcher::RegisterFaBtMusicObservers()
{
    if (faObserver_ == nullptr) {
        CarPolicyManager *mgr = &owner_;
        faObserver_ = new BluetoothObserver([mgr]() {
            std::string value;
            if (!BluetoothDataShareQueryForUser(SETTINGSDATA_FA_KEY, value)) {
                HILOGW("FA observer: DataShare query failed");
                return;
            }
            int newFaStatus = SafeParseInt<int>(value, 0);
            DoInCarPolicyThread([mgr, newFaStatus]() { mgr->OnFaStatusChanged(newFaStatus); });
        });
        // RegisterObserver 失败时 RegisterSettingsObserver 内部已日志告警；
        // sptr 引用计数为 1（仅 faObserver_ 持有），UnregisterFaBtMusicObservers 中 nullptr 赋值会释放。
        RegisterSettingsObserver(SETTINGSDATA_FA_KEY, faObserver_);
    }
    if (btMusicObserver_ == nullptr) {
        CarPolicyManager *mgr = &owner_;
        btMusicObserver_ = new BluetoothObserver([mgr]() {
            std::string value;
            if (!BluetoothDataShareQueryForUser(SETTINGSDATA_BT_MUSIC_KEY, value)) {
                HILOGW("BtMusic observer: DataShare query failed");
                return;
            }
            int newStatus = SafeParseInt<int>(value, 0);
            DoInCarPolicyThread([mgr, newStatus]() { mgr->OnBtMusicStatusChanged(newStatus); });
        });
        RegisterSettingsObserver(SETTINGSDATA_BT_MUSIC_KEY, btMusicObserver_);
    }
}

void CarPolicyDispatcher::UnregisterFaBtMusicObservers()
{
    UnregisterSettingsObserver(SETTINGSDATA_FA_KEY, faObserver_);
    faObserver_ = nullptr;
    UnregisterSettingsObserver(SETTINGSDATA_BT_MUSIC_KEY, btMusicObserver_);
    btMusicObserver_ = nullptr;
}

void CarPolicyDispatcher::RegisterSmartCallTransferObserver()
{
    if (sctObserver_ != nullptr) {
        return;
    }
    CarPolicyManager *mgr = &owner_;
    sctObserver_ = new BluetoothObserver([mgr]() {
        std::string value;
        if (!BluetoothDataShareQueryForUser(SETTINGSDATA_SCT_KEY, value)) {
            HILOGW("SCT observer: DataShare query failed");
            return;
        }
        // SCT 状态失败时使用 -1（STATUS_UNKNOWN 语义），区别于合法 0
        int newStatus = SafeParseInt<int>(value, SCT_STATUS_INVALID);
        DoInCarPolicyThread([mgr, newStatus]() { mgr->OnSmartCallTransferStatusChanged(newStatus); });
    });
    RegisterSettingsObserver(SETTINGSDATA_SCT_KEY, sctObserver_);
}

void CarPolicyDispatcher::UnregisterSmartCallTransferObserver()
{
    UnregisterSettingsObserver(SETTINGSDATA_SCT_KEY, sctObserver_);
    sctObserver_ = nullptr;
}

void CarPolicyDispatcher::RegisterClassicRemoteDeviceObserver()
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("ClassicAdapter null, remote device observer not registered");
        return;
    }
    classicObserver_ = std::make_unique<ClassicRemoteDeviceObserverImpl>(owner_);
    if (!classicAdapter->RegisterRemoteDeviceObserver(*classicObserver_)) {
        HILOGE("RegisterRemoteDeviceObserver failed");
        classicObserver_.reset();
    } else {
        HILOGI("Classic remote device observer registered");
    }
}

void CarPolicyDispatcher::DeregisterClassicRemoteDeviceObserver()
{
    if (classicObserver_ != nullptr) {
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter != nullptr) {
            classicAdapter->DeregisterRemoteDeviceObserver(*classicObserver_);
        }
        classicObserver_.reset();
    }
}

void CarPolicyDispatcher::RegisterBtSettingsUiObserver()
{
    if (btSettingsUiObserver_ != nullptr) {
        return;
    }
    CarPolicyManager *mgr = &owner_;
    btSettingsUiObserver_ = new BluetoothObserver([mgr]() {
        std::string value;
        if (!BluetoothDataShareQueryForUser(SETTINGSDATA_BT_SETTINGS_UI_KEY, value)) {
            HILOGW("BT Settings UI observer: DataShare query failed");
            return;
        }
        int newStatus = SafeParseInt<int>(value, 0);
        DoInCarPolicyThread([mgr, newStatus]() { mgr->OnBtSettingsUiChanged(newStatus); });
    });
    RegisterSettingsObserver(SETTINGSDATA_BT_SETTINGS_UI_KEY, btSettingsUiObserver_);
}

void CarPolicyDispatcher::UnregisterBtSettingsUiObserver()
{
    UnregisterSettingsObserver(SETTINGSDATA_BT_SETTINGS_UI_KEY, btSettingsUiObserver_);
    btSettingsUiObserver_ = nullptr;
}

void CarPolicyDispatcher::LoadFaBtMusicStatus(int &outFaStatus, int &outBtMusicStatus)
{
    constexpr int STATUS_UNKNOWN = -1;
    outFaStatus = STATUS_UNKNOWN;
    outBtMusicStatus = STATUS_UNKNOWN;
    std::string faValue;
    std::string btMusicValue;
    bool faOk = BluetoothDataShareQueryForUser(SETTINGSDATA_FA_KEY, faValue);
    bool btMusicOk = BluetoothDataShareQueryForUser(SETTINGSDATA_BT_MUSIC_KEY, btMusicValue);
    if (faOk && !faValue.empty()) {
        outFaStatus = SafeParseInt<int>(faValue, STATUS_UNKNOWN);
    } else {
        HILOGW("LoadFaBtMusicStatus: FA query failed or empty, fallback UNKNOWN");
    }
    if (btMusicOk && !btMusicValue.empty()) {
        outBtMusicStatus = SafeParseInt<int>(btMusicValue, STATUS_UNKNOWN);
    } else {
        HILOGW("LoadFaBtMusicStatus: BtMusic query failed or empty, fallback UNKNOWN");
    }
    HILOGI("LoadFaBtMusicStatus: faStatus=%{public}d, btMusicStatus=%{public}d", outFaStatus, outBtMusicStatus);
}

}  // namespace bluetooth
}  // namespace OHOS
