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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_host"
#endif

#include "bluetooth_host.h"
#include <memory>
#include <mutex>
#include <unistd.h>
#include <thread>
#include "bluetooth_ble_peripheral_observer_stub.h"
#include "bluetooth_host_load_callback.h"
#include "bluetooth_host_observer_stub.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device_observer_stub.h"
#include "bluetooth_resource_manager_observer_stub.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "bluetooth_switch_module.h"
#include "ffrt_inner.h"
#include "common_event.h"
#include "common_event_data.h"
#include "common_event_manager.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace Bluetooth {
namespace {
constexpr int32_t LOAD_SA_TIMEOUT_MS = 4000;
}

struct BluetoothHost::impl {
    impl();
    ~impl();

    bool LoadBluetoothHostService();
    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();
    int EnableBluetoothAfterFactoryReset(void);

    // host observer
    class BluetoothHostObserverImp;
    sptr<BluetoothHostObserverImp> observerImp_ = nullptr;
    sptr<BluetoothHostObserverImp> bleObserverImp_ = nullptr;

    // remote device observer
    class BluetoothRemoteDeviceObserverImp;
    sptr<BluetoothRemoteDeviceObserverImp> remoteObserverImp_ = nullptr;

    // remote device observer
    class BluetoothBlePeripheralCallbackImp;
    sptr<BluetoothBlePeripheralCallbackImp> bleRemoteObserverImp_ = nullptr;

    // bluetooth resource manager observer
    class BluetoothResourceManagerObserverImp;
    sptr<BluetoothResourceManagerObserverImp> resourceManagerObserverImp_ = nullptr;

    // user regist observers
    BluetoothObserverList<BluetoothHostObserver> observers_;

    // user regist remote observers
    BluetoothObserverList<BluetoothRemoteDeviceObserver> remoteObservers_;

    // user regist resource manager observers
    BluetoothObserverList<BluetoothResourceManagerObserver> resourceManagerObservers_;

    void SyncRandomAddrToService(void);
    int ConvertBluetoothStateToBtStateID(BluetoothState state);

    std::mutex proxyMutex_;
    std::string stagingRealAddr_;
    std::string stagingRandomAddr_;
    int32_t profileRegisterId = 0;
    std::atomic_bool isFactoryReseting_ { false };

    class BluetoothSwitchAction;
    std::mutex switchModuleMutex_ {};  // used for serial execute enableBluetoothToRestrictMode.
    std::shared_ptr<BluetoothSwitchModule> switchModule_ { nullptr };

private:
    SaManagerStatus saManagerStatus_ = SaManagerStatus::WAIT_NOTIFY;
    std::condition_variable proxyConVar_;
};

class BluetoothHost::impl::BluetoothHostObserverImp : public BluetoothHostObserverStub {
public:
    explicit BluetoothHostObserverImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothHostObserverImp() override{};

    void Register(std::shared_ptr<BluetoothHostObserver> &observer)
    {
        host_.observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<BluetoothHostObserver> &observer)
    {
        host_.observers_.Deregister(observer);
    }

    void OnStateChanged(int32_t transport, int32_t status) override
    {
        if (status == BTStateID::STATE_TURN_ON) {
            host_.SyncRandomAddrToService();
        }
        CHECK_AND_RETURN_LOG(!isNeedInterceptSwitchStatus(transport, status), "No Need transform same status");
        BluetoothProfileManager::GetInstance().NotifyBluetoothStateChange(transport, status);
        host_.observers_.ForEach([transport, status](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnStateChanged(transport, status);
        });
    }

    void OnBluetoothStateChanged(int32_t state) override
    {
        std::lock_guard<std::mutex> lock(host_.switchModuleMutex_);
        CHECK_AND_RETURN_LOG(host_.switchModule_, "switchModule is nullptr");
        if (state == bluetooth::BluetoothSwitchState::STATE_ON) {
            host_.switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_ON);
        }
        if (state == bluetooth::BluetoothSwitchState::STATE_OFF) {
            host_.switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
        }
        if (state == bluetooth::BluetoothSwitchState::STATE_HALF) {
            host_.switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_HALF);
        }
    }

    void OnDiscoveryStateChanged(int32_t status) override
    {
        HILOGD("enter, status: %{public}d", status);
        host_.observers_.ForEach(
            [status](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDiscoveryStateChanged(status); });
    }

    void OnDiscoveryResult(
        const BluetoothRawAddress &device, int rssi, const std::string deviceName, int deviceClass) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.observers_.ForEach([remoteDevice, rssi, deviceName, deviceClass](
            std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnDiscoveryResult(remoteDevice, rssi, deviceName, deviceClass);
        });
    }

    void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s",
            transport, GET_ENCRYPT_RAW_ADDR(device));
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.observers_.ForEach([remoteDevice](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnPairRequested(remoteDevice);
        });
    }

    void OnPairConfirmed(const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s, reqType: %{public}d, number: %{public}d",
            transport, GET_ENCRYPT_RAW_ADDR(device), reqType, number);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.observers_.ForEach([remoteDevice, reqType, number](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnPairConfirmed(remoteDevice, reqType, number);
        });
    }

    void OnScanModeChanged(int mode) override
    {
        HILOGI("enter, mode: %{public}d", mode);
        host_.observers_.ForEach(
            [mode](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnScanModeChanged(mode); });
    }

    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        HILOGI("enter, deviceName: %{public}s", deviceName.c_str());
        host_.observers_.ForEach([deviceName](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnDeviceNameChanged(deviceName);
        });
    }

    void OnDeviceAddrChanged(const std::string &address) override
    {
        HILOGD("enter");
        host_.observers_.ForEach(
            [address](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDeviceAddrChanged(address); });
    }

private:
    bool isNeedInterceptSwitchStatus(int32_t transport, int32_t status)
    {
        bool isBluetoothSeriviceOn = BluetoothProfileManager::GetInstance().IsBluetoothServiceOn();
        if (status == BTStateID::STATE_TURN_OFF) {
            if (transport == BTTransport::ADAPTER_BLE &&
                preBleState_ == BTStateID::STATE_TURN_OFF && !isBluetoothSeriviceOn) {
                return true;
            }
            if (transport == BTTransport::ADAPTER_BREDR &&
                preBrState_ == BTStateID::STATE_TURN_OFF && !isBluetoothSeriviceOn) {
                return true;
            }
        }
        if (transport == BTTransport::ADAPTER_BREDR) {
            preBrState_ = status;
        } else {
            preBleState_ = status;
        }
        return false;
    }
    BluetoothHost::impl &host_;
    const int32_t INVALID_STATE = -1;
    int32_t preBrState_ = INVALID_STATE;
    int32_t preBleState_ = INVALID_STATE;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHostObserverImp);
};

class BluetoothHost::impl::BluetoothRemoteDeviceObserverImp : public BluetoothRemoteDeviceObserverstub {
public:
    explicit BluetoothRemoteDeviceObserverImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothRemoteDeviceObserverImp() override = default;

    void OnAclStateChanged(const BluetoothRawAddress &device, int32_t state, uint32_t reason) override
    {
        HILOGD("enter, device: %{public}s, state: %{public}d, reason: %{public}u",
            GET_ENCRYPT_RAW_ADDR(device), state, reason);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, state, reason](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnAclStateChanged(remoteDevice, state, reason);
            });
    }

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device,
        int32_t status, int32_t cause) override
    {
        HILOGD("enter, transport: %{public}d, device: %{public}s, status: %{public}d, cause: %{public}d",
            transport, GET_ENCRYPT_RAW_ADDR(device), status, cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.remoteObservers_.ForEach(
            [remoteDevice, status, cause](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnPairStatusChanged(remoteDevice, status, cause);
            });
    }

    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) override
    {
        HILOGD("enter, device: %{public}s", GET_ENCRYPT_RAW_ADDR(device));
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, uuids](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                std::vector<ParcelUuid> parcelUuids;
                for (auto &uuid : uuids) {
                    ParcelUuid parcelUuid = UUID::ConvertFrom128Bits(uuid.ConvertTo128Bits());
                    parcelUuids.push_back(parcelUuid);
                }
                observer->OnRemoteUuidChanged(remoteDevice, parcelUuids);
            });
    }

    void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) override
    {
        HILOGD("enter, device: %{public}s, deviceName: %{public}s",
            GET_ENCRYPT_RAW_ADDR(device), deviceName.c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceName](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteNameChanged(remoteDevice, deviceName);
            });
    }

    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) override
    {
        HILOGI("enter, device: %{public}s, alias: %{public}s",
            GET_ENCRYPT_RAW_ADDR(device), alias.c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, alias](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteAliasChanged(remoteDevice, alias);
            });
    }

    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) override
    {
        HILOGD("enter, device: %{public}s, cod: %{public}d", GET_ENCRYPT_RAW_ADDR(device), cod);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        BluetoothDeviceClass deviceClass(cod);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceClass](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteCodChanged(remoteDevice, deviceClass);
            });
    }

    void OnRemoteBatteryChanged(const BluetoothRawAddress &device, const BluetoothBatteryInfo &batteryInfo) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        DeviceBatteryInfo info;
        info.deviceId_ = device.GetAddress();
        info.batteryLevel_ = batteryInfo.batteryLevel_;
        info.leftEarBatteryLevel_ = batteryInfo.leftEarBatteryLevel_;
        info.leftEarChargeState_ = static_cast<DeviceChargeState>(batteryInfo.leftEarChargeState_);
        info.rightEarBatteryLevel_ = batteryInfo.rightEarBatteryLevel_;
        info.rightEarChargeState_ = static_cast<DeviceChargeState>(batteryInfo.rightEarChargeState_);
        info.boxBatteryLevel_ = batteryInfo.boxBatteryLevel_;
        info.boxChargeState_ = static_cast<DeviceChargeState>(batteryInfo.boxChargeState_);
        host_.remoteObservers_.ForEach(
            [remoteDevice, info](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteBatteryChanged(remoteDevice, info);
            });
    }

    void OnRemoteDeviceCommonInfoReport(const BluetoothRawAddress &device, const std::vector<uint8_t> &value) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, value](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteDeviceCommonInfoReport(remoteDevice, value);
            });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothRemoteDeviceObserverImp);
};

class BluetoothHost::impl::BluetoothBlePeripheralCallbackImp : public BluetoothBlePeripheralObserverStub {
public:
    explicit BluetoothBlePeripheralCallbackImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothBlePeripheralCallbackImp() override = default;

    void OnAclStateChanged(const BluetoothRawAddress &device, int state, unsigned int reason) override
    {
        HILOGD("enter, device: %{public}s, state: %{public}d, reason: %{public}u",
            GET_ENCRYPT_RAW_ADDR(device), state, reason);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BLE);
        host_.remoteObservers_.ForEach(
            [remoteDevice, state, reason](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnAclStateChanged(remoteDevice, state, reason);
            });
    }

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int status, int cause) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s, status: %{public}d, cause: %{public}d",
            transport, GET_ENCRYPT_RAW_ADDR(device), status, cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.remoteObservers_.ForEach(
            [remoteDevice, status, cause](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnPairStatusChanged(remoteDevice, status, cause);
            });
    }

    void OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status) override
    {
        HILOGI("enter, device: %{public}s, rssi: %{public}d, status: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), rssi, status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BLE);
        host_.remoteObservers_.ForEach(
            [remoteDevice, rssi, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnReadRemoteRssiEvent(remoteDevice, rssi, status);
            });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBlePeripheralCallbackImp);
};

class BluetoothHost::impl::BluetoothResourceManagerObserverImp : public BluetoothResourceManagerObserverStub {
public:
    explicit BluetoothResourceManagerObserverImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothResourceManagerObserverImp() override = default;

    void OnSensingStateChanged(uint8_t eventId, const BluetoothSensingInfo &info) override
    {
        HILOGD("enter, eventId: %{public}d", eventId);
        SensingInfo sensingInfo;
        sensingInfo.addr_ = info.addr_;
        sensingInfo.uuid_ = info.uuid_;
        sensingInfo.resourceId_ = info.resourceId_;
        sensingInfo.pkgName_ = info.pkgName_;
        sensingInfo.isServer_ = info.isServer_;
        sensingInfo.interval_ = info.interval_;
        host_.resourceManagerObservers_.ForEach(
            [eventId, sensingInfo](std::shared_ptr<BluetoothResourceManagerObserver> observer) {
                observer->OnSensingStateChanged(eventId, sensingInfo);
            });
    }

    void OnBluetoothResourceDecision(uint8_t eventId, const BluetoothSensingInfo &info, uint32_t &result) override
    {
        HILOGD("enter, eventId: %{public}d, result: %{public}d", eventId, result);
        SensingInfo sensingInfo;
        sensingInfo.addr_ = info.addr_;
        sensingInfo.uuid_ = info.uuid_;
        sensingInfo.resourceId_ = info.resourceId_;
        sensingInfo.pkgName_ = info.pkgName_;
        sensingInfo.isServer_ = info.isServer_;
        sensingInfo.interval_ = info.interval_;
        host_.resourceManagerObservers_.ForEach(
            [eventId, sensingInfo, &result](std::shared_ptr<BluetoothResourceManagerObserver> observer) {
                observer->OnBluetoothResourceDecision(eventId, sensingInfo, result);
            });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothResourceManagerObserverImp);
};

class BluetoothHost::impl::BluetoothSwitchAction : public IBluetoothSwitchAction {
public:
    BluetoothSwitchAction() = default;
    ~BluetoothSwitchAction() override = default;

    int EnableBluetooth(bool noAutoConnect) override
    {
        CHECK_AND_RETURN_LOG_RET(!BluetoothHost::GetDefaultHost().IsBtProhibitedByEdm(),
            BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited !");
        CHECK_AND_RETURN_LOG_RET(BluetoothHost::GetDefaultHost().pimpl->LoadBluetoothHostService(),
            BT_ERR_INTERNAL_ERROR, "pimpl is null or load bluetooth service failed.");

        sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
        return proxy->EnableBle(noAutoConnect);
    }

    int DisableBluetooth(void) override
    {
        sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
        return proxy->DisableBt();
    }

    int EnableBluetoothToRestrictMode(void) override
    {
        CHECK_AND_RETURN_LOG_RET(!BluetoothHost::GetDefaultHost().IsBtProhibitedByEdm(),
            BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited !");
        CHECK_AND_RETURN_LOG_RET(BluetoothHost::GetDefaultHost().pimpl->LoadBluetoothHostService(),
            BT_ERR_INTERNAL_ERROR, "pimpl is null or load bluetooth service failed.");

        sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
        return proxy->EnableBluetoothToRestrictMode();
    }
};

BluetoothHost::impl::impl()
{
    observerImp_ = new BluetoothHostObserverImp(*this);
    remoteObserverImp_ = new BluetoothRemoteDeviceObserverImp(*this);
    bleRemoteObserverImp_ = new BluetoothBlePeripheralCallbackImp(*this);
    bleObserverImp_ = new BluetoothHostObserverImp(*this);
    resourceManagerObserverImp_ = new BluetoothResourceManagerObserverImp(*this);

    auto switchActionPtr = std::make_unique<BluetoothSwitchAction>();
    switchModule_ = std::make_shared<BluetoothSwitchModule>(std::move(switchActionPtr));

    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(BLUETOOTH_HOST,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHost> proxy = iface_cast<IBluetoothHost>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
        proxy->RegisterObserver(observerImp_);
        proxy->RegisterBleAdapterObserver(bleObserverImp_);
        proxy->RegisterRemoteDeviceObserver(remoteObserverImp_);
        proxy->RegisterBlePeripheralCallback(bleRemoteObserverImp_);
        proxy->RegisterBtResourceManagerObserver(resourceManagerObserverImp_);
    });
}

BluetoothHost::impl::~impl()
{
    HILOGI("starts");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    proxy->DeregisterObserver(observerImp_);
    proxy->DeregisterBleAdapterObserver(bleObserverImp_);
    proxy->DeregisterRemoteDeviceObserver(remoteObserverImp_);
    proxy->DeregisterBlePeripheralCallback(bleRemoteObserverImp_);
    proxy->DeregisterBtResourceManagerObserver(resourceManagerObserverImp_);
}

bool BluetoothHost::impl::LoadBluetoothHostService()
{
    std::unique_lock<std::mutex> lock(proxyMutex_);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr.");
        return false;
    }
    sptr<IRemoteObject> hostRemote = BluetoothProfileManager::GetInstance().GetProfileRemote(BLUETOOTH_HOST);
    //当蓝牙服务已经起来的时候。这时的hostRemote不为空， 不需要进行后续的从sa拉起蓝牙服务的动作
    if (hostRemote != nullptr) {
        return true;
    }

    sptr<BluetoothHostLoadCallBack> loadCallback = new BluetoothHostLoadCallBack();
    if (loadCallback == nullptr) {
        HILOGE("loadCallback is nullptr.");
        return false;
    }
    int32_t ret = samgrProxy->LoadSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, loadCallback);
    if (ret != ERR_OK) {
        HILOGE("Failed to load bluetooth systemAbility");
        return false;
    }
    auto waitStatus = proxyConVar_.wait_for(
        lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS), [this]() {
            HILOGI("bluetooth_service load systemAbility finished");
            sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
            return proxy != nullptr || saManagerStatus_ == SaManagerStatus::LOAD_FAIL;
        });
    if (!waitStatus) {
        HILOGE("load bluetooth systemAbility timeout");
        return false;
    }
    if (saManagerStatus_ == SaManagerStatus::LOAD_FAIL) {
        HILOGE("load bluetooth_service fail");
        saManagerStatus_ = SaManagerStatus::WAIT_NOTIFY;
        return false;
    }
    saManagerStatus_ = SaManagerStatus::WAIT_NOTIFY;
    return true;
}

void BluetoothHost::impl::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    HILOGI("LoadSystemAbilitySuccess FinishStart SA");
    saManagerStatus_ = SaManagerStatus::LOAD_SUCCESS;
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::LoadSystemAbilityFail()
{
    HILOGI("LoadSystemAbilityFail FinishStart SA");
    saManagerStatus_ = SaManagerStatus::LOAD_FAIL;
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::SyncRandomAddrToService(void)
{
    if (!IsValidBluetoothAddr(stagingRealAddr_)) {
        HILOGD("stagingRealAddr_ is invalid.");
        return;
    }
    if (!IsValidBluetoothAddr(stagingRandomAddr_)) {
        HILOGE("stagingRandomAddr_ is invalid.");
        return;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    proxy->SyncRandomAddress(stagingRealAddr_, stagingRandomAddr_);
    stagingRealAddr_ = "";
    stagingRandomAddr_ = "";
}

int BluetoothHost::impl::ConvertBluetoothStateToBtStateID(BluetoothState state)
{
    int ret = BTStateID::STATE_TURN_OFF;
    switch (state) {
        case BluetoothState::STATE_ON: ret = BTStateID::STATE_TURN_ON; break;
        case BluetoothState::STATE_TURNING_ON: ret = BTStateID::STATE_TURNING_ON; break;
        case BluetoothState::STATE_TURNING_OFF: ret = BTStateID::STATE_TURNING_OFF; break;
        case BluetoothState::STATE_OFF: ret = BTStateID::STATE_TURN_OFF; break;
        default: break;
    }
    return ret;
}

BluetoothHost::BluetoothHost()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("fails: no pimpl");
    }
}

BluetoothHost::~BluetoothHost() {}

BluetoothHost &BluetoothHost::GetDefaultHost()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<BluetoothHost> instance;
    return *instance;
#else
    // C++11 static local variable initialization is thread-safe.
    static BluetoothHost hostAdapter;
    return hostAdapter;
#endif
}

void BluetoothHost::RegisterObserver(std::shared_ptr<BluetoothHostObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Register(observer);
}

void BluetoothHost::DeregisterObserver(std::shared_ptr<BluetoothHostObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Deregister(observer);
}

int BluetoothHost::EnableBt()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(!IsBtProhibitedByEdm(), BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    return proxy->EnableBt();
}

int BluetoothHost::DisableBt()
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->switchModule_, BT_ERR_INTERNAL_ERROR, "switchModule is nullptr");
    return pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH);
}

static void PublishBtSwitchRestrictBluetoothEvent(void)
{
    OHOS::AAFwk::Want want;
    want.SetAction("usual.event.bluetooth.BT_SWITCH_RESTRICT_BLUETOOTH");

    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);

    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions({"ohos.permission.ACCESS_BLUETOOTH"});
    bool ret = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    if (!ret) {
        HILOGE("Publish usual.event.bluetooth.BT_SWITCH_RESTRICT_BLUETOOTH event failed");
        return;
    }
}

int BluetoothHost::RestrictBluetooth()
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    PublishBtSwitchRestrictBluetoothEvent();
    CHECK_AND_RETURN_LOG_RET(pimpl->switchModule_, BT_ERR_INTERNAL_ERROR, "switchModule is nullptr");
    int ret =  pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::DISABLE_BLUETOOTH);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    ret = pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE);
    return ret;
}

int BluetoothHost::SatelliteControl(int type, int state)
{
    HILOGI("type: %{public}d, state: %{public}d", type, state);
    if (type == static_cast<int>(SATELLITE_CONTROL_MODE::ANTENNA)) {
        CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    } else if (type == static_cast<int>(SATELLITE_CONTROL_MODE::BLUETOOTH_SWITCH)) {
        pimpl->LoadBluetoothHostService();
    } else {
        HILOGE("Invalid control type: %{public}d", type);
        return BT_ERR_INVALID_PARAM;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    return proxy->SatelliteControl(type, state);
}
void BluetoothHost::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    proxy->UpdateVirtualDevice(action, address);
}

int BluetoothHost::GetBtState() const
{
    BluetoothState state = GetBluetoothState();
    return pimpl->ConvertBluetoothStateToBtStateID(state);
}

int BluetoothHost::GetBtState(int &state) const
{
    state = GetBtState();
    return BT_NO_ERROR;
}

BluetoothState BluetoothHost::GetBluetoothState(void) const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BluetoothState::STATE_OFF, "proxy is nullptr");
    int state = static_cast<int>(BluetoothState::STATE_OFF);
    proxy->GetBtState(state);
    return static_cast<BluetoothState>(state);
}

int BluetoothHost::impl::EnableBluetoothAfterFactoryReset(void)
{
    HILOGI("Attempt to enable bluetooth after factory reset");
    isFactoryReseting_ = false;
    SetParameter("persist.bluetooth.switch_enable", "2");  // 2 means bluetooth auto enter restricted mode
    return BT_NO_ERROR;
}

int BluetoothHost::BluetoothFactoryReset()
{
    HILOGI("enter");
    constexpr const char* BLUETOOTH_FACTORY_RESET_KEY = "persist.bluetooth.factoryreset";
    int ret = SetParameter(BLUETOOTH_FACTORY_RESET_KEY, "true");
    CHECK_AND_RETURN_LOG_RET(ret == 0, BT_ERR_INTERNAL_ERROR, "SetParameter failed");

    pimpl->isFactoryReseting_ = true;

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    if (proxy == nullptr) {
        return pimpl->EnableBluetoothAfterFactoryReset();
    }
    CHECK_AND_RETURN_LOG_RET(IS_BLE_ENABLED(), BT_NO_ERROR, "bluetooth is off.");
    return proxy->BluetoothFactoryReset();
}

bool BluetoothHost::IsValidBluetoothAddr(const std::string &addr)
{
#if defined(IOS_PLATFORM)
    const std::regex deviceIdRegex("^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$");
    return regex_match(addr, deviceIdRegex);
#elif defined(ANDROID_PLATFORM)
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(addr, deviceIdRegex);
#else
    if (addr.length() != ADDRESS_LENGTH) {
        HILOGD("invalid address len.");
        return false;
    }

    for (int i = 0; i < ADDRESS_LENGTH; i++) {
        char c = addr[i];
        switch (i % ADDRESS_SEPARATOR_UNIT) {
            case 0:
            case 1:
                if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
                    break;
                }
                return false;
            case ADDRESS_COLON_INDEX:
            default:
                if (c == ':') {
                    break;
                }
                return false;
        }
    }
    return true;
#endif
}

BluetoothRemoteDevice BluetoothHost::GetRemoteDevice(const std::string &addr, int transport) const
{
    BluetoothRemoteDevice remoteDevice(addr, transport);
    return remoteDevice;
}

int BluetoothHost::EnableBle()
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->switchModule_, BT_ERR_INTERNAL_ERROR, "switchModule is nullptr");
    return pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH);
}

int BluetoothHost::EnableBluetoothNoAutoConnect()
{
    HILOGI("enter");
    pimpl->switchModule_->SetNoAutoConnect(true);
    return EnableBle();
}

int BluetoothHost::DisableBle()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    return proxy->DisableBle();
}

int BluetoothHost::EnableBluetoothToRestrictMode(void)
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->switchModule_, BT_ERR_INTERNAL_ERROR, "switchModule is nullptr");
    return pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE);
}

bool BluetoothHost::IsBrEnabled() const
{
    BluetoothState state = GetBluetoothState();
    return state == BluetoothState::STATE_ON;
}

bool BluetoothHost::IsBleEnabled() const
{
    BluetoothState state = GetBluetoothState();
    return (state == BluetoothState::STATE_ON ||
        state == BluetoothState::STATE_BLE_ON ||
        state == BluetoothState::STATE_TURNING_ON ||
        state == BluetoothState::STATE_TURNING_OFF);
}

int BluetoothHost::GetLocalAddress(std::string &addr) const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->GetLocalAddress(addr);
}

std::vector<uint32_t> BluetoothHost::GetProfileList() const
{
    HILOGD("enter");
    std::vector<uint32_t> profileList;
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, profileList, "proxy is nullptr");

    profileList = proxy->GetProfileList();
    return profileList;
}

int BluetoothHost::GetMaxNumConnectedAudioDevices() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "proxy is nullptr");

    return proxy->GetMaxNumConnectedAudioDevices();
}

int BluetoothHost::GetBtConnectionState() const
{
    HILOGD("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), state, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, state, "proxy is nullptr");

    proxy->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtConnectionState(int &state) const
{
    HILOGD("enter");
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    int ret = proxy->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

int BluetoothHost::GetBtProfileConnState(uint32_t profileId, int &state) const
{
    HILOGI("profileId: %{public}d", profileId);
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->GetBtProfileConnState(profileId, state);
}

void BluetoothHost::GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids)
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    std::vector<std::string> stringUuids;
    proxy->GetLocalSupportedUuids(stringUuids);

    for (auto uuid : stringUuids) {
        uuids.push_back(UUID::FromString(uuid));
    }
}

bool BluetoothHost::Start()
{
    // / This function only used for passthrough, so this is empty.
    return true;
}

void BluetoothHost::Stop()
{
    // / This function only used for passthrough, so this is empty.
}

BluetoothDeviceClass BluetoothHost::GetLocalDeviceClass() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BluetoothDeviceClass(0), "proxy is nullptr");

    int LocalDeviceClass = proxy->GetLocalDeviceClass();
    return BluetoothDeviceClass(LocalDeviceClass);
}

bool BluetoothHost::SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass)
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "proxy is nullptr");

    int cod = deviceClass.GetClassOfDevice();
    return proxy->SetLocalDeviceClass(cod);
}

std::string BluetoothHost::GetLocalName() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::string(), "proxy is nullptr");

    std::string name = INVALID_NAME;
    proxy->GetLocalName(name);
    return name;
}

int BluetoothHost::GetLocalName(std::string &name) const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->GetLocalName(name);
}

int BluetoothHost::SetLocalName(const std::string &name)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->SetLocalName(name);
}

int BluetoothHost::GetBtScanMode(int32_t &scanMode) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->GetBtScanMode(scanMode);
}

int BluetoothHost::SetBtScanMode(int mode, int duration)
{
    HILOGD("mode: %{public}d, duration: %{public}d", mode, duration);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->SetBtScanMode(mode, duration);
}

int BluetoothHost::GetBondableMode(int transport) const
{
    HILOGI("transport: %{public}d", transport);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "proxy is nullptr");

    return proxy->GetBondableMode(transport);
}

bool BluetoothHost::SetBondableMode(int transport, int mode)
{
    HILOGD("transport: %{public}d, mode: %{public}d", transport, mode);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "proxy is nullptr");

    return proxy->SetBondableMode(transport, mode);
}

int BluetoothHost::StartBtDiscovery()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->StartBtDiscovery();
}

int BluetoothHost::CancelBtDiscovery()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->CancelBtDiscovery();
}

int32_t BluetoothHost::IsBtDiscovering(bool &isDiscovering, int transport) const
{
    HILOGI("transport: %{public}d", transport);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "proxy is nullptr");

    return proxy->IsBtDiscovering(isDiscovering, transport);
}

long BluetoothHost::GetBtDiscoveryEndMillis() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), 0, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, 0, "proxy is nullptr");

    return proxy->GetBtDiscoveryEndMillis();
}

int32_t BluetoothHost::GetPairedDevices(int transport, std::vector<BluetoothRemoteDevice> &pairedDevices) const
{
    HILOGI("transport: %{public}d", transport);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    std::vector<BluetoothRawAddress> pairedAddr;
    int32_t ret = proxy->GetPairedDevices(pairedAddr);

    for (auto it = pairedAddr.begin(); it != pairedAddr.end(); it++) {
        BluetoothRemoteDevice device((*it).GetAddress(), transport);
        pairedDevices.emplace_back(device);
    }
    return ret;
}

int32_t BluetoothHost::RemovePair(const BluetoothRemoteDevice &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(device.GetDeviceAddr());
    return proxy->RemovePair(device.GetTransportType(), rawAddrSptr);
}

bool BluetoothHost::RemoveAllPairs()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "proxy is nullptr");

    return proxy->RemoveAllPairs();
}

void BluetoothHost::RegisterRemoteDeviceObserver(std::shared_ptr<BluetoothRemoteDeviceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");

    pimpl->remoteObservers_.Register(observer);
}

void BluetoothHost::DeregisterRemoteDeviceObserver(std::shared_ptr<BluetoothRemoteDeviceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");

    pimpl->remoteObservers_.Deregister(observer);
}

int BluetoothHost::GetBleMaxAdvertisingDataLength() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "proxy is nullptr");

    return proxy->GetBleMaxAdvertisingDataLength();
}

void BluetoothHost::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    CHECK_AND_RETURN_LOG(pimpl, "pimpl is null.");

    pimpl->LoadSystemAbilitySuccess(remoteObject);
}

void BluetoothHost::LoadSystemAbilityFail()
{
    CHECK_AND_RETURN_LOG(pimpl, "pimpl is null.");

    pimpl->LoadSystemAbilityFail();
}

int32_t BluetoothHost::GetLocalProfileUuids(std::vector<std::string> &uuids)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INTERNAL_ERROR, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->GetLocalProfileUuids(uuids);
}

int BluetoothHost::SetFastScan(bool isEnable)
{
    HILOGI("isEnable: %{public}d", isEnable);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");

    return proxy->SetFastScan(isEnable);
}

int BluetoothHost::GetRandomAddress(const std::string &realAddr, std::string &randomAddr) const
{
    randomAddr = realAddr;
    return BT_NO_ERROR;
}

int BluetoothHost::ConnectAllowedProfiles(const std::string &remoteAddr) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");

    return proxy->ConnectAllowedProfiles(remoteAddr);
}

int BluetoothHost::DisconnectAllowedProfiles(const std::string &remoteAddr) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");

    return proxy->DisconnectAllowedProfiles(remoteAddr);
}

void BluetoothHost::RegisterBtResourceManagerObserver(std::shared_ptr<BluetoothResourceManagerObserver> observer)
{
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");

    pimpl->resourceManagerObservers_.Register(observer);
}

void BluetoothHost::DeregisterBtResourceManagerObserver(std::shared_ptr<BluetoothResourceManagerObserver> observer)
{
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");

    pimpl->resourceManagerObservers_.Deregister(observer);
}

int BluetoothHost::SetFastScanLevel(int level)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");
    return proxy->SetFastScanLevel(level);
}

bool BluetoothHost::IsBtProhibitedByEdm(void)
{
    constexpr const char* BLUETOOTH_EDM_KEY = "persist.edm.prohibit_bluetooth";
    constexpr const uint32_t PARAM_TRUE_LEN = 4; // "true" 4bytes
    constexpr const uint32_t PARAM_FALSE_LEN = 5; // "false" 5bytes
    constexpr const char* PARAM_TRUE = "true";
    constexpr const char* PARAM_FALSE = "false";

    char result[PARAM_FALSE_LEN + 1] = {0};
    //  Returns the number of bytes of the system parameter if the operation is successful.
    int len = GetParameter(BLUETOOTH_EDM_KEY, PARAM_FALSE, result, PARAM_FALSE_LEN + 1);
    CHECK_AND_RETURN_LOG_RET(len == PARAM_FALSE_LEN || len == PARAM_TRUE_LEN, false, "GetParameter len is invalid.");

    if (strncmp(result, PARAM_TRUE, PARAM_TRUE_LEN) == 0) {
        HILOGW("bluetooth is prohibited by EDM. You won't be able to turn on bluetooth !");
        return true;
    }
    return false;
}

static bool IsBluetoothSystemAbilityOn(void)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET(samgrProxy != nullptr, false, "samgrProxy is nullptr");
    auto object = samgrProxy->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    return object != nullptr;
}

void BluetoothHost::OnRemoveBluetoothSystemAbility()
{
    // Notify the upper layer that bluetooth is disabled.
    bool isBluetoothSystemAbilityOn = IsBluetoothSystemAbilityOn();
    if (isBluetoothSystemAbilityOn) {
        HILOGW("Bluetooth SA is started, the hap application may be freezed by rss");
        // Notify profile manager bluetooth off once.
        BluetoothProfileManager::GetInstance().NotifyBluetoothStateChange(
            BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    } else if (pimpl->observerImp_ && pimpl->bleObserverImp_) {
        HILOGD("bluetooth_servi died and send state off to app");
        pimpl->observerImp_->OnStateChanged(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
        pimpl->bleObserverImp_->OnStateChanged(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    }
    if (pimpl->isFactoryReseting_.load()) {
        pimpl->EnableBluetoothAfterFactoryReset();
    }
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    if (pimpl->switchModule_) {
        pimpl->switchModule_->ProcessBluetoothSwitchEvent(BluetoothSwitchEvent::BLUETOOTH_OFF);
    }
}

void BluetoothHost::Close(void)
{
    std::lock_guard<std::mutex> lock(pimpl->switchModuleMutex_);
    pimpl->switchModule_ = nullptr;
}

int32_t BluetoothHost::UpdateCloudBluetoothDevice(const std::vector<TrustPairDeviceParam> &cloudDevices)
{
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");
    std::vector<BluetoothTrustPairDevice> cloudDevicesVec {};
    for (auto &devParam : cloudDevices) {
        BluetoothTrustPairDevice trustPairDevice;
        trustPairDevice.SetMacAddress(devParam.macAddress_);
        trustPairDevice.SetDeviceName(devParam.deviceName_);
        trustPairDevice.SetUuid(devParam.uuids_);
        trustPairDevice.SetBluetoothClass(devParam.bluetoothClass_);
        trustPairDevice.SetToken(devParam.token_);
        trustPairDevice.SetSecureAdvertisingInfo(devParam.secureAdvertisingInfo_);
        HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice add device: %{public}s",
            GetEncryptAddr(trustPairDevice.GetMacAddress()).c_str());
        cloudDevicesVec.emplace_back(trustPairDevice);
    }
    return proxy->UpdateCloudBluetoothDevice(cloudDevicesVec);
}
} // namespace Bluetooth
} // namespace OHOS
