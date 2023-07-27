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
#include "bluetooth_load_system_ability.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device_observer_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bluetooth_errorcode.h"

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
    bool InitBluetoothHostProxy(void);

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

    sptr<IBluetoothHost> proxy_ = nullptr;

    bool isHostProxyInit = false;

    bool InitBluetoothHostObserver(void);

    std::mutex proxyMutex_;
private:
    class BluetoothHostDeathRecipient;
    sptr<BluetoothHostDeathRecipient> deathRecipient_ = nullptr;
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
        HILOGD("bluetooth state, transport: %{public}s, status: %{public}s",
            GetBtTransportName(transport).c_str(), GetBtStateName(status).c_str());
        host_.observers_.ForEach([transport, status](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnStateChanged(transport, status);
        });
    }

    void OnDiscoveryStateChanged(int32_t status) override
    {
        HILOGI("enter, status: %{public}d", status);
        host_.observers_.ForEach(
            [status](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDiscoveryStateChanged(status); });
    }

    void OnDiscoveryResult(const BluetoothRawAddress &device) override
    {
        HILOGD("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.observers_.ForEach([remoteDevice](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnDiscoveryResult(remoteDevice);
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
        HILOGI("enter");
        host_.observers_.ForEach(
            [address](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDeviceAddrChanged(address); });
    }

private:
    BluetoothHost::impl &host_;
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
        HILOGI("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
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
        HILOGI("enter, device: %{public}s, deviceName: %{public}s",
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
        HILOGI("enter, device: %{public}s, cod: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), cod);
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

class BluetoothHost::impl::BluetoothHostDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BluetoothHostDeathRecipient(BluetoothHost::impl &impl) : impl_(impl)
    {};
    ~BluetoothHostDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHostDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("bluetooth_servi died and then re-registered");
        std::lock_guard<std::mutex> lock(impl_.proxyMutex_);
        impl_.proxy_ = nullptr;
        impl_.isHostProxyInit = false;

        // Notify the upper layer that bluetooth is disabled.
        if (impl_.observerImp_ != nullptr && impl_.bleObserverImp_ != nullptr) {
            HILOGI("bluetooth_servi died and send state off to app");
            impl_.observerImp_->OnStateChanged(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
            impl_.bleObserverImp_->OnStateChanged(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
        }
    }

private:
    bool ResetHostProxy()
    {
        HILOGI("proxy is null, try to get proxy.");
        return true;
    }
    BluetoothHost::impl &impl_;
};

BluetoothHost::impl::impl()
{
    BluetootLoadSystemAbility::GetInstance().RegisterNotifyMsg(PROFILE_ID_HOST);
    if (!BluetootLoadSystemAbility::GetInstance().HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance().SubScribeBluetoothSystemAbility();
        return;
    }
    InitBluetoothHostProxy();
}

BluetoothHost::impl::~impl()
{
    HILOGI("starts");
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return;
    }
    proxy_->DeregisterObserver(observerImp_);
    proxy_->DeregisterBleAdapterObserver(bleObserverImp_);
    proxy_->DeregisterRemoteDeviceObserver(remoteObserverImp_);
    proxy_->DeregisterBlePeripheralCallback(bleRemoteObserverImp_);
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

bool BluetoothHost::impl::InitBluetoothHostProxy(void)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (proxy_) {
        return true;
    }
    HILOGE("enter");
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr.");
        return false;
    }
    auto object = samgrProxy->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (object == nullptr) {
        HILOGE("object is nullptr.");
        return false;
    }
    proxy_ = iface_cast<IBluetoothHost>(object);
    if (!InitBluetoothHostObserver()) {
        return false;
    }
    return true;
}

bool BluetoothHost::impl::InitBluetoothHostObserver(void)
{
    HILOGI("enter");
    deathRecipient_ = new BluetoothHostDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        HILOGE("deathRecipient_ is null");
        return false;
    }
    observerImp_ = new (std::nothrow) BluetoothHostObserverImp(*this);
    if (observerImp_ == nullptr) {
        HILOGE("observerImp_ is null");
        return false;
    }
    remoteObserverImp_ = new (std::nothrow) BluetoothRemoteDeviceObserverImp(*this);
    if (remoteObserverImp_ == nullptr) {
        HILOGE("remoteObserverImp_ is null");
        return false;
    }
    bleRemoteObserverImp_ = new (std::nothrow) BluetoothBlePeripheralCallbackImp(*this);
    if (bleRemoteObserverImp_ == nullptr) {
        HILOGE("bleRemoteObserverImp_ is null");
        return false;
    }
    bleObserverImp_ = new (std::nothrow) BluetoothHostObserverImp(*this);
    if (bleObserverImp_ == nullptr) {
        HILOGE("bleObserverImp_ is null");
        return false;
    }
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return false;
    }
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    proxy_->RegisterObserver(observerImp_);
    proxy_->RegisterBleAdapterObserver(bleObserverImp_);
    proxy_->RegisterRemoteDeviceObserver(remoteObserverImp_);
    proxy_->RegisterBlePeripheralCallback(bleRemoteObserverImp_);
    return true;
}

bool BluetoothHost::impl::LoadBluetoothHostService()
{
    std::unique_lock<std::mutex> lock(proxyMutex_);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr.");
        return false;
    }
    auto object = samgrProxy->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (object != nullptr) {
        proxy_ = iface_cast<IBluetoothHost>(object);
        if (!InitBluetoothHostObserver()) {
            HILOGE("InitBluetoothHostObserver fail");
            return false;
        }
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
        lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS), [this]() { return proxy_ != nullptr; });
    if (!waitStatus) {
        HILOGE("load bluetooth systemAbility timeout");
        return false;
    }
    return true;
}

void BluetoothHost::impl::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    HILOGI("LoadSystemAbilitySuccess FinishStart SA");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (remoteObject != nullptr) {
        if (isHostProxyInit == false && proxy_ == nullptr) {
            proxy_ = iface_cast<IBluetoothHost>(remoteObject);
            isHostProxyInit = true;
        }
    } else {
        HILOGE("LoadSystemAbilitySuccess remoteObject is NULL.");
    }
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::LoadSystemAbilityFail()
{
    HILOGI("LoadSystemAbilityFail FinishStart SA");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    proxy_ = nullptr;
    isHostProxyInit = false;
    proxyConVar_.notify_one();
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

void BluetoothHost::RegisterObserver(BluetoothHostObserver &observer)
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }

    std::shared_ptr<BluetoothHostObserver> pointer(&observer, [](BluetoothHostObserver *) {});
    pimpl->observers_.Register(pointer);
}

void BluetoothHost::DeregisterObserver(BluetoothHostObserver &observer)
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }

    std::shared_ptr<BluetoothHostObserver> pointer(&observer, [](BluetoothHostObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

void BluetoothHost::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("samgrProxy is nullptr.");
        return;
    }
    auto object = samgrProxy->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (object == nullptr) {
        HILOGE("object is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(pimpl->proxyMutex_);
    if (pimpl->isHostProxyInit == false && pimpl->proxy_ == nullptr) {
        pimpl->proxy_ = iface_cast<IBluetoothHost>(object);
    }
    pimpl->InitBluetoothHostObserver();
}

int BluetoothHost::EnableBt()
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    return pimpl->proxy_->EnableBt();
}

int BluetoothHost::DisableBt()
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    return pimpl->proxy_->DisableBt();
}

int BluetoothHost::GetBtState() const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BTStateID::STATE_TURN_OFF;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BTStateID::STATE_TURN_OFF;
    }

    int state = BTStateID::STATE_TURN_OFF;
    pimpl->proxy_->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtState(int &state) const
{
    HILOGI("enter");
    state = BTStateID::STATE_TURN_OFF;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_NO_ERROR;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INVALID_STATE;
    }

    int ret = pimpl->proxy_->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

bool BluetoothHost::BluetoothFactoryReset()
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->BluetoothFactoryReset();
}

bool BluetoothHost::IsValidBluetoothAddr(const std::string &addr)
{
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
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
}

BluetoothRemoteDevice BluetoothHost::GetRemoteDevice(const std::string &addr, int transport) const
{
    BluetoothRemoteDevice remoteDevice(addr, transport);
    return remoteDevice;
}

int BluetoothHost::EnableBle()
{
    HILOGI("enter");
    if (!pimpl || !pimpl->LoadBluetoothHostService()) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->proxy_->EnableBle();
}

int BluetoothHost::DisableBle()
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->proxy_->DisableBle();
}

bool BluetoothHost::IsBrEnabled() const
{
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->IsBrEnabled();
}

bool BluetoothHost::IsBleEnabled() const
{
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->IsBleEnabled();
}

std::string BluetoothHost::GetLocalAddress() const
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return std::string();
    }

    return pimpl->proxy_->GetLocalAddress();
}

std::vector<uint32_t> BluetoothHost::GetProfileList() const
{
    HILOGI("enter");
    std::vector<uint32_t> profileList;
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return profileList;
    }

    profileList = pimpl->proxy_->GetProfileList();
    return profileList;
}

int BluetoothHost::GetMaxNumConnectedAudioDevices() const
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return INVALID_VALUE;
    }

    return pimpl->proxy_->GetMaxNumConnectedAudioDevices();
}

int BluetoothHost::GetBtConnectionState() const
{
    HILOGI("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return state;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return state;
    }

    pimpl->proxy_->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtConnectionState(int &state) const
{
    HILOGI("enter");
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    int ret = pimpl->proxy_->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

int BluetoothHost::GetBtProfileConnState(uint32_t profileId, int &state) const
{
    HILOGI("enter, profileId: %{public}d", profileId);
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->GetBtProfileConnState(profileId, state);
}

void BluetoothHost::GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids)
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return;
    }

    std::vector<std::string> stringUuids;
    pimpl->proxy_->GetLocalSupportedUuids(stringUuids);
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
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BluetoothDeviceClass(0);
    }

    int LocalDeviceClass = pimpl->proxy_->GetLocalDeviceClass();
    return BluetoothDeviceClass(LocalDeviceClass);
}

bool BluetoothHost::SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass)
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    int cod = deviceClass.GetClassOfDevice();
    return pimpl->proxy_->SetLocalDeviceClass(cod);
}

std::string BluetoothHost::GetLocalName() const
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return std::string();
    }

    std::string name = INVALID_NAME;
    pimpl->proxy_->GetLocalName(name);
    return name;
}

int BluetoothHost::GetLocalName(std::string &name) const
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->GetLocalName(name);
}

int BluetoothHost::SetLocalName(const std::string &name)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->SetLocalName(name);
}

int BluetoothHost::GetBtScanMode(int32_t &scanMode) const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->GetBtScanMode(scanMode);
}

int BluetoothHost::SetBtScanMode(int mode, int duration)
{
    HILOGI("enter, mode: %{public}d, duration: %{public}d", mode, duration);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->SetBtScanMode(mode, duration);
}

int BluetoothHost::GetBondableMode(int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return INVALID_VALUE;
    }

    return pimpl->proxy_->GetBondableMode(transport);
}

bool BluetoothHost::SetBondableMode(int transport, int mode)
{
    HILOGI("enter, transport: %{public}d, mode: %{public}d", transport, mode);
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->SetBondableMode(transport, mode);
}

int BluetoothHost::StartBtDiscovery()
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->StartBtDiscovery();
}

int BluetoothHost::CancelBtDiscovery()
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->proxy_->CancelBtDiscovery();
}

bool BluetoothHost::IsBtDiscovering(int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->IsBtDiscovering(transport);
}

long BluetoothHost::GetBtDiscoveryEndMillis() const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return 0;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return 0;
    }

    return pimpl->proxy_->GetBtDiscoveryEndMillis();
}

int32_t BluetoothHost::GetPairedDevices(int transport, std::vector<BluetoothRemoteDevice> &pairedDevices) const
{
    HILOGI("enter, transport: %{public}d", transport);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    std::vector<BluetoothRawAddress> pairedAddr;
    int32_t ret = pimpl->proxy_->GetPairedDevices(transport, pairedAddr);

    for (auto it = pairedAddr.begin(); it != pairedAddr.end(); it++) {
        BluetoothRemoteDevice device((*it).GetAddress(), transport);
        pairedDevices.emplace_back(device);
    }
    return ret;
}

int32_t BluetoothHost::RemovePair(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("Invalid remote device.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(device.GetDeviceAddr());
    return pimpl->proxy_->RemovePair(device.GetTransportType(), rawAddrSptr);
}

bool BluetoothHost::RemoveAllPairs()
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return false;
    }

    return pimpl->proxy_->RemoveAllPairs();
}

void BluetoothHost::RegisterRemoteDeviceObserver(std::shared_ptr<BluetoothRemoteDeviceObserver> observer)
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    pimpl->remoteObservers_.Register(observer);
}

void BluetoothHost::DeregisterRemoteDeviceObserver(BluetoothRemoteDeviceObserver &observer)
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    std::shared_ptr<BluetoothRemoteDeviceObserver> pointer(&observer, [](BluetoothRemoteDeviceObserver *) {});
    pimpl->remoteObservers_.Deregister(pointer);
}

int BluetoothHost::GetBleMaxAdvertisingDataLength() const
{
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return INVALID_VALUE;
    }

    return pimpl->proxy_->GetBleMaxAdvertisingDataLength();
}

void BluetoothHost::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    pimpl->LoadSystemAbilitySuccess(remoteObject);
}

void BluetoothHost::LoadSystemAbilityFail()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    pimpl->LoadSystemAbilityFail();
}

int32_t BluetoothHost::GetLocalProfileUuids(std::vector<std::string> &uuids)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    return pimpl->proxy_->GetLocalProfileUuids(uuids);
}

int BluetoothHost::SetFastScan(bool isEnable)
{
    HILOGI("enter, isEnable: %{public}d", isEnable);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    return pimpl->proxy_->SetFastScan(isEnable);
}

int BluetoothHost::GetRandomAddress(const std::string &realAddr, std::string &randomAddr) const
{
    HILOGI("enter.");
    randomAddr = "";
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl || !pimpl->proxy_) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    if (!IsValidBluetoothAddr(realAddr)) {
        HILOGE("realAddr is invalid: %{public}s", realAddr.c_str());
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->proxy_->GetRandomAddress(realAddr, randomAddr);
}
} // namespace Bluetooth
} // namespace OHOS
