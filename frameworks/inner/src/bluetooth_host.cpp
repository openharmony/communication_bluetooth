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

    bool InitBluetoothHostObserver(void);

    void SyncRandomAddrToService(void);

    std::mutex proxyMutex_;
    std::string stagingRealAddr_;
    std::string stagingRandomAddr_;
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
        HILOGD("bluetooth state, transport: %{public}s, status: %{public}s",
            GetBtTransportName(transport).c_str(), GetBtStateName(status).c_str());
        if (status == BTStateID::STATE_TURN_ON) {
            host_.SyncRandomAddrToService();
        }
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
        HILOGD("enter");
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
    BluetootLoadSystemAbility::GetInstance()->RegisterNotifyMsg(PROFILE_ID_HOST);
    if (!BluetootLoadSystemAbility::GetInstance()->HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance()->SubScribeBluetoothSystemAbility();
    }

    observerImp_ = new BluetoothHostObserverImp(*this);
    remoteObserverImp_ = new BluetoothRemoteDeviceObserverImp(*this);
    bleRemoteObserverImp_ = new BluetoothBlePeripheralCallbackImp(*this);
    bleObserverImp_ = new BluetoothHostObserverImp(*this);

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
    HILOGD("enter");

    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return false;
    }
    // The bluetooth service will be de-weight, repeated register without side effects.
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
        if (proxy_ == nullptr) {
            proxy_ = iface_cast<IBluetoothHost>(object);
        }
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
        if (proxy_ == nullptr) {
            proxy_ = iface_cast<IBluetoothHost>(remoteObject);
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
    proxyConVar_.notify_one();
}

void BluetoothHost::impl::SyncRandomAddrToService(void)
{
    HILOGD("SyncRandomAddrToService.");
    if (!IsValidBluetoothAddr(stagingRealAddr_)) {
        HILOGE("stagingRealAddr_ is invalid.");
        return;
    }
    if (!IsValidBluetoothAddr(stagingRandomAddr_)) {
        HILOGE("stagingRandomAddr_ is invalid.");
        return;
    }
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is nullptr.");
        return;
    }
    proxy_->SyncRandomAddress(stagingRealAddr_, stagingRandomAddr_);
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
    if (pimpl->proxy_ == nullptr) {
        pimpl->proxy_ = iface_cast<IBluetoothHost>(object);
    }
    pimpl->InitBluetoothHostObserver();
}

void BluetoothHost::Uinit()
{
    HILOGI("enter BluetoothHost Uinit");
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    std::lock_guard<std::mutex> lock(pimpl->proxyMutex_);
    pimpl->proxy_ = nullptr;
    // Notify the upper layer that bluetooth is disabled.
    if (pimpl->observerImp_ != nullptr && pimpl->bleObserverImp_ != nullptr) {
        HILOGI("bluetooth_servi died and send state off to app");
        pimpl->observerImp_->OnStateChanged(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
        pimpl->bleObserverImp_->OnStateChanged(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    }
}

int BluetoothHost::CountEnableTimes(bool enable)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INTERNAL_ERROR, "pimpl or proxy_ is nullptr");
    return pimpl->proxy_->CountEnableTimes(enable);
}

int BluetoothHost::EnableBt()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(!IsBtProhibitedByEdm(), BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INTERNAL_ERROR, "pimpl or proxy is null");
    return pimpl->proxy_->EnableBt();
}

int BluetoothHost::DisableBt()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INTERNAL_ERROR, "pimpl or proxy_ is nullptr");
    CountEnableTimes(false);
    return pimpl->proxy_->DisableBt();
}

int BluetoothHost::GetBtState() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BTStateID::STATE_TURN_OFF, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INTERNAL_ERROR, "pimpl or proxy_ is nullptr");

    int state = BTStateID::STATE_TURN_OFF;
    pimpl->proxy_->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtState(int &state) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_NO_ERROR, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INVALID_STATE, "pimpl or proxy_ is nullptr");

    state = BTStateID::STATE_TURN_OFF;
    int ret = pimpl->proxy_->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

bool BluetoothHost::BluetoothFactoryReset()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), false, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->BluetoothFactoryReset();
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
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(!IsBtProhibitedByEdm(), BT_ERR_PROHIBITED_BY_EDM, "bluetooth is prohibited");

    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->LoadBluetoothHostService(), BT_ERR_INTERNAL_ERROR,
        "pimpl or proxy is null");

    CountEnableTimes(true);
    return pimpl->proxy_->EnableBle();
}

int BluetoothHost::DisableBle()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_INTERNAL_ERROR, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->DisableBle();
}

bool BluetoothHost::IsBrEnabled() const
{
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->IsBrEnabled();
}

bool BluetoothHost::IsBleEnabled() const
{
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->IsBleEnabled();
}

std::string BluetoothHost::GetLocalAddress() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, std::string(), "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetLocalAddress();
}

std::vector<uint32_t> BluetoothHost::GetProfileList() const
{
    HILOGD("enter");
    std::vector<uint32_t> profileList;
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, profileList, "pimpl or proxy_ is nullptr");

    profileList = pimpl->proxy_->GetProfileList();
    return profileList;
}

int BluetoothHost::GetMaxNumConnectedAudioDevices() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, INVALID_VALUE, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetMaxNumConnectedAudioDevices();
}

int BluetoothHost::GetBtConnectionState() const
{
    HILOGD("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), state, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, state, "pimpl or proxy_ is nullptr");

    pimpl->proxy_->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtConnectionState(int &state) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    state = static_cast<int>(BTConnectState::DISCONNECTED);
    int ret = pimpl->proxy_->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

int BluetoothHost::GetBtProfileConnState(uint32_t profileId, int &state) const
{
    HILOGI("enter, profileId: %{public}d", profileId);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    state = static_cast<int>(BTConnectState::DISCONNECTED);
    return pimpl->proxy_->GetBtProfileConnState(profileId, state);
}

void BluetoothHost::GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl && pimpl->proxy_, "pimpl or proxy_ is nullptr");

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
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BluetoothDeviceClass(0), "pimpl or proxy_ is nullptr");

    int LocalDeviceClass = pimpl->proxy_->GetLocalDeviceClass();
    return BluetoothDeviceClass(LocalDeviceClass);
}

bool BluetoothHost::SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    int cod = deviceClass.GetClassOfDevice();
    return pimpl->proxy_->SetLocalDeviceClass(cod);
}

std::string BluetoothHost::GetLocalName() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, std::string(), "pimpl or proxy_ is nullptr");

    std::string name = INVALID_NAME;
    pimpl->proxy_->GetLocalName(name);
    return name;
}

int BluetoothHost::GetLocalName(std::string &name) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetLocalName(name);
}

int BluetoothHost::SetLocalName(const std::string &name)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->SetLocalName(name);
}

int BluetoothHost::GetBtScanMode(int32_t &scanMode) const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetBtScanMode(scanMode);
}

int BluetoothHost::SetBtScanMode(int mode, int duration)
{
    HILOGI("enter, mode: %{public}d, duration: %{public}d", mode, duration);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->SetBtScanMode(mode, duration);
}

int BluetoothHost::GetBondableMode(int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, INVALID_VALUE, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetBondableMode(transport);
}

bool BluetoothHost::SetBondableMode(int transport, int mode)
{
    HILOGI("enter, transport: %{public}d, mode: %{public}d", transport, mode);
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->SetBondableMode(transport, mode);
}

int BluetoothHost::StartBtDiscovery()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->StartBtDiscovery();
}

int BluetoothHost::CancelBtDiscovery()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->CancelBtDiscovery();
}

int32_t BluetoothHost::IsBtDiscovering(bool &isDiscovering, int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->IsBtDiscovering(isDiscovering, transport);
}

long BluetoothHost::GetBtDiscoveryEndMillis() const
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), 0, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, 0, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetBtDiscoveryEndMillis();
}

int32_t BluetoothHost::GetPairedDevices(int transport, std::vector<BluetoothRemoteDevice> &pairedDevices) const
{
    HILOGI("enter, transport: %{public}d", transport);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    std::vector<BluetoothRawAddress> pairedAddr;
    int32_t ret = pimpl->proxy_->GetPairedDevices(pairedAddr);

    for (auto it = pairedAddr.begin(); it != pairedAddr.end(); it++) {
        BluetoothRemoteDevice device((*it).GetAddress(), transport);
        pairedDevices.emplace_back(device);
    }
    return ret;
}

int32_t BluetoothHost::RemovePair(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(device.GetDeviceAddr());
    return pimpl->proxy_->RemovePair(device.GetTransportType(), rawAddrSptr);
}

bool BluetoothHost::RemoveAllPairs()
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, false, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->RemoveAllPairs();
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
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, INVALID_VALUE, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetBleMaxAdvertisingDataLength();
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
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->GetLocalProfileUuids(uuids);
}

int BluetoothHost::SetFastScan(bool isEnable)
{
    HILOGI("enter, isEnable: %{public}d", isEnable);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->proxy_, BT_ERR_UNAVAILABLE_PROXY, "pimpl or proxy_ is nullptr");

    return pimpl->proxy_->SetFastScan(isEnable);
}

int BluetoothHost::GetRandomAddress(const std::string &realAddr, std::string &randomAddr) const
{
    HILOGI("enter.");
    randomAddr = realAddr;
    return BT_NO_ERROR;
}

int BluetoothHost::ConnectAllowedProfiles(const std::string &remoteAddr) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET((pimpl && pimpl->proxy_), BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");
    return pimpl->proxy_->ConnectAllowedProfiles(remoteAddr);
}

int BluetoothHost::DisconnectAllowedProfiles(const std::string &remoteAddr) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    CHECK_AND_RETURN_LOG_RET((pimpl && pimpl->proxy_), BT_ERR_UNAVAILABLE_PROXY, "pimpl or bluetooth host is nullptr");
    return pimpl->proxy_->DisconnectAllowedProfiles(remoteAddr);
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
    HILOGI("prohibit_bluetooth = %{public}s", result);
    if (strncmp(result, PARAM_TRUE, PARAM_TRUE_LEN) == 0) {
        HILOGW("Bluetooth is prohibited by EDM. You won't be able to turn on bluetooth !");
        return true;
    }
    return false;
}
} // namespace Bluetooth
} // namespace OHOS
