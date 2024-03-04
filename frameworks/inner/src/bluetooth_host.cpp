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

#include "bluetooth_host.h"
#include <memory>
#include <mutex>
#include <unistd.h>
#include "bluetooth_ble_peripheral_observer_stub.h"
#include "bluetooth_host_load_callback.h"
#include "bluetooth_host_observer_stub.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device_observer_stub.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
namespace {
constexpr int32_t LOAD_SA_TIMEOUT_MS = 5000;
}

struct BluetoothHost::impl {
    impl();
    ~impl();

    bool LoadBluetoothHostService();
    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();

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

    // user regist observers
    BluetoothObserverList<BluetoothHostObserver> observers_;

    // user regist remote observers
    BluetoothObserverList<BluetoothRemoteDeviceObserver> remoteObservers_;

    void SyncRandomAddrToService(void);

    std::mutex proxyMutex_;
    std::string stagingRealAddr_;
    std::string stagingRandomAddr_;
    int32_t profileRegisterId = 0;

private:
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
        if (status == BTStateID::STATE_TURN_OFF) {
            if (transport == BTTransport::ADAPTER_BLE && preBleState_ == BTStateID::STATE_TURN_OFF) {
                return;
            }
            if (transport == BTTransport::ADAPTER_BREDR && preBrState_ == BTStateID::STATE_TURN_OFF) {
                return;
            }
        }
        if (transport == BTTransport::ADAPTER_BREDR) {
            preBrState_ = status;
        } else {
            preBleState_ = status;
        }
        DelayedSingleton<BluetoothProfileManager>::GetInstance()->NotifyBluetoothStateChange(transport, status);
        host_.observers_.ForEach([transport, status](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnStateChanged(transport, status);
        });
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
            transport, GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.observers_.ForEach([remoteDevice](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnPairRequested(remoteDevice);
        });
    }

    void OnPairConfirmed(const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s, reqType: %{public}d, number: %{public}d",
            transport, GetEncryptAddr((device).GetAddress()).c_str(), reqType, number);
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
            GetEncryptAddr((device).GetAddress()).c_str(), state, reason);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, state, reason](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnAclStateChanged(remoteDevice, state, reason);
            });
    }

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s, status: %{public}d",
            transport, GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.remoteObservers_.ForEach(
            [remoteDevice, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnPairStatusChanged(remoteDevice, status);
            });
    }

    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) override
    {
        HILOGD("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
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
            GetEncryptAddr((device).GetAddress()).c_str(), deviceName.c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceName](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteNameChanged(remoteDevice, deviceName);
            });
    }

    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) override
    {
        HILOGI("enter, device: %{public}s, alias: %{public}s",
            GetEncryptAddr((device).GetAddress()).c_str(), alias.c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, alias](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteAliasChanged(remoteDevice, alias);
            });
    }

    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) override
    {
        HILOGD("enter, device: %{public}s, cod: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), cod);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        BluetoothDeviceClass deviceClass(cod);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceClass](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteCodChanged(remoteDevice, deviceClass);
            });
    }

    void OnRemoteBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        HILOGI("enter, device: %{public}s, batteryLevel: %{public}d",
            GetEncryptAddr((device).GetAddress()).c_str(), batteryLevel);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, batteryLevel](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteBatteryLevelChanged(remoteDevice, batteryLevel);
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
            GetEncryptAddr((device).GetAddress()).c_str(), state, reason);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BLE);
        host_.remoteObservers_.ForEach(
            [remoteDevice, state, reason](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnAclStateChanged(remoteDevice, state, reason);
            });
    }

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int status) override
    {
        HILOGI("enter, transport: %{public}d, device: %{public}s, status: %{public}d",
            transport, GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.remoteObservers_.ForEach(
            [remoteDevice, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnPairStatusChanged(remoteDevice, status);
            });
    }

    void OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status) override
    {
        HILOGI("enter, device: %{public}s, rssi: %{public}d, status: %{public}d",
            GetEncryptAddr((device).GetAddress()).c_str(), rssi, status);
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

BluetoothHost::impl::impl()
{
    observerImp_ = new BluetoothHostObserverImp(*this);
    remoteObserverImp_ = new BluetoothRemoteDeviceObserverImp(*this);
    bleRemoteObserverImp_ = new BluetoothBlePeripheralCallbackImp(*this);
    bleObserverImp_ = new BluetoothHostObserverImp(*this);

    profileRegisterId = DelayedSingleton<BluetoothProfileManager>::GetInstance()->RegisterFunc(BLUETOOTH_HOST,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHost> proxy = iface_cast<IBluetoothHost>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
        proxy->RegisterObserver(observerImp_);
        proxy->RegisterBleAdapterObserver(bleObserverImp_);
        proxy->RegisterRemoteDeviceObserver(remoteObserverImp_);
        proxy->RegisterBlePeripheralCallback(bleRemoteObserverImp_);
    });
}

BluetoothHost::impl::~impl()
{
    HILOGI("starts");
    DelayedSingleton<BluetoothProfileManager>::GetInstance()->DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    proxy->DeregisterObserver(observerImp_);
    proxy->DeregisterBleAdapterObserver(bleObserverImp_);
    proxy->DeregisterRemoteDeviceObserver(remoteObserverImp_);
    proxy->DeregisterBlePeripheralCallback(bleRemoteObserverImp_);
}

bool BluetoothHost::impl::LoadBluetoothHostService()
{
    std::unique_lock<std::mutex> lock(proxyMutex_);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr.");
        return false;
    }
   sptr<IRemoteObject> hostRemote = DelayedSingleton<BluetoothProfileManager>::GetInstance()->GetProfileRemote(BLUETOOTH_HOST);
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
        lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS), []() {
            sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
            HILOGI("bluetooth_service has found, proxy not null");
            return proxy != nullptr;
        });
    if (!waitStatus) {
        HILOGE("load bluetooth systemAbility timeout");
        return false;
    }
    return true;
}

void BluetoothHost::impl::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    HILOGI("LoadSystemAbilitySuccess FinishStart SA");
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::LoadSystemAbilityFail()
{
    HILOGI("LoadSystemAbilityFail FinishStart SA");
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::SyncRandomAddrToService(void)
{
    if (!IsValidBluetoothAddr(stagingRealAddr_)) {
        HILOGE("stagingRealAddr_ is invalid.");
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
    // C++11 static local variable initialization is thread-safe.
    static BluetoothHost hostAdapter;
    return hostAdapter;
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

int BluetoothHost::CountEnableTimes(bool enable)
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    return proxy->CountEnableTimes(enable);
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
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    CountEnableTimes(false);
    return proxy->DisableBt();
}

int BluetoothHost::GetBtState() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BTStateID::STATE_TURN_OFF, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BTStateID::STATE_TURN_OFF, "proxy is nullptr");

    int state = BTStateID::STATE_TURN_OFF;
    proxy->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtState(int &state) const
{
    HILOGD("enter");
    state = BTStateID::STATE_TURN_OFF;
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_NO_ERROR, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "proxy is nullptr");

    int ret = proxy->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

int BluetoothHost::BluetoothFactoryReset()
{
    HILOGD("enter");
    constexpr const char* BLUETOOTH_FACTORY_RESET_KEY = "persist.bluetooth.factoryreset";
    int ret = SetParameter(BLUETOOTH_FACTORY_RESET_KEY, "true");
    CHECK_AND_RETURN_LOG_RET(ret == 0, BT_ERR_INTERNAL_ERROR, "SetParameter failed");

    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_NO_ERROR, "bluetooth is off.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "proxy is nullptr");

    return proxy->BluetoothFactoryReset();
}

bool BluetoothHost::IsValidBluetoothAddr(const std::string &addr)
{
    CHECK_AND_RETURN_LOG_RET(addr.length() == ADDRESS_LENGTH, false, "invalid address len.");

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
}

BluetoothRemoteDevice BluetoothHost::GetRemoteDevice(const std::string &addr, int transport) const
{
    BluetoothRemoteDevice remoteDevice(addr, transport);
    return remoteDevice;
}

int BluetoothHost::EnableBle()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(!IsBtProhibitedByEdm(), BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited !");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->LoadBluetoothHostService(), BT_ERR_INTERNAL_ERROR,
        "pimpl is null or load bluetooth service failed.");

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    CountEnableTimes(true);
    return proxy->EnableBle();
}

int BluetoothHost::DisableBle()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    return proxy->DisableBle();
}

bool BluetoothHost::IsBrEnabled() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "proxy is nullptr");

    return proxy->IsBrEnabled();
}

bool BluetoothHost::IsBleEnabled() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    if (proxy == nullptr) {
        HILOGD("proxy is nullptr");
        return false;
    }

    return proxy->IsBleEnabled();
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
    HILOGI("mode: %{public}d, duration: %{public}d", mode, duration);
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
    HILOGI("transport: %{public}d, mode: %{public}d", transport, mode);
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

void BluetoothHost::OnRemoveBluetoothSystemAbility()
{
    // Notify the upper layer that bluetooth is disabled.
    if (pimpl->observerImp_ != nullptr && pimpl->bleObserverImp_ != nullptr) {
        HILOGD("bluetooth_servi died and send state off to app");
        pimpl->observerImp_->OnStateChanged(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
        pimpl->bleObserverImp_->OnStateChanged(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    }
}
} // namespace Bluetooth
} // namespace OHOS
