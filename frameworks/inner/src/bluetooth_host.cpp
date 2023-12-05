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
    int32_t profileRegisterId;

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
    observerImp_ = new BluetoothHostObserverImp(*this);
    remoteObserverImp_ = new BluetoothRemoteDeviceObserverImp(*this);
    bleRemoteObserverImp_ = new BluetoothBlePeripheralCallbackImp(*this);
    bleObserverImp_ = new BluetoothHostObserverImp(*this);

    profileRegisterId = Singleton<BluetoothProfileManager>::GetInstance().RegisterFunc(BLUETOOTH_HOST,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHost> proxy = iface_cast<IBluetoothHost>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(observerImp_);
        proxy->RegisterBleAdapterObserver(bleObserverImp_);
        proxy->RegisterRemoteDeviceObserver(remoteObserverImp_);
        proxy->RegisterBlePeripheralCallback(bleRemoteObserverImp_);
    });
}

BluetoothHost::impl::~impl()
{
    HILOGI("starts");
    Singleton<BluetoothProfileManager>::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
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
   sptr<IRemoteObject> hostRemote = Singleton<BluetoothProfileManager>::GetInstance().GetProfileRemote(BLUETOOTH_HOST);
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
    HILOGD("SyncRandomAddrToService.");
    if (!IsValidBluetoothAddr(stagingRealAddr_)) {
        HILOGE("stagingRealAddr_ is invalid.");
        return;
    }
    if (!IsValidBluetoothAddr(stagingRandomAddr_)) {
        HILOGE("stagingRandomAddr_ is invalid.");
        return;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
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
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    return proxy->CountEnableTimes(enable);
}
int BluetoothHost::EnableBt()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    return proxy->EnableBt();
}

int BluetoothHost::DisableBt()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CountEnableTimes(false);
    return proxy->DisableBt();
}

int BluetoothHost::GetBtState() const
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BTStateID::STATE_TURN_OFF;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BTStateID::STATE_TURN_OFF, "failed: no proxy");

    int state = BTStateID::STATE_TURN_OFF;
    proxy->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtState(int &state) const
{
    HILOGD("enter");
    state = BTStateID::STATE_TURN_OFF;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_NO_ERROR;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");

    int ret = proxy->GetBtState(state);
    HILOGI("state: %{public}d", state);
    return ret;
}

bool BluetoothHost::BluetoothFactoryReset()
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return proxy->BluetoothFactoryReset();
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
    HILOGD("enter");
    if (!pimpl->LoadBluetoothHostService()) {
        HILOGE("pimpl or bluetooth host is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CountEnableTimes(true);
    return proxy->EnableBle();
}

int BluetoothHost::DisableBle()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->DisableBle();
}

bool BluetoothHost::IsBrEnabled() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return proxy->IsBrEnabled();
}

bool BluetoothHost::IsBleEnabled() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return proxy->IsBleEnabled();
}

std::string BluetoothHost::GetLocalAddress() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::string(), "failed: no proxy");

    return proxy->GetLocalAddress();
}

std::vector<uint32_t> BluetoothHost::GetProfileList() const
{
    HILOGD("enter");
    std::vector<uint32_t> profileList;
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, profileList, "failed: no proxy");

    profileList = proxy->GetProfileList();
    return profileList;
}

int BluetoothHost::GetMaxNumConnectedAudioDevices() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "failed: no proxy");

    return proxy->GetMaxNumConnectedAudioDevices();
}

int BluetoothHost::GetBtConnectionState() const
{
    HILOGD("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return state;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, state, "failed: no proxy");

    proxy->GetBtConnectionState(state);
    HILOGI("state: %{public}d", state);
    return state;
}

int BluetoothHost::GetBtConnectionState(int &state) const
{
    HILOGD("enter");
    state = static_cast<int>(BTConnectState::DISCONNECTED);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    int ret = proxy->GetBtConnectionState(state);
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

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->GetBtProfileConnState(profileId, state);
}

void BluetoothHost::GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids)
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

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
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BluetoothDeviceClass(0), "failed: no proxy");

    int LocalDeviceClass = proxy->GetLocalDeviceClass();
    return BluetoothDeviceClass(LocalDeviceClass);
}

bool BluetoothHost::SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass)
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    int cod = deviceClass.GetClassOfDevice();
    return proxy->SetLocalDeviceClass(cod);
}

std::string BluetoothHost::GetLocalName() const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::string(), "failed: no proxy");

    std::string name = INVALID_NAME;
    proxy->GetLocalName(name);
    return name;
}

int BluetoothHost::GetLocalName(std::string &name) const
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->GetLocalName(name);
}

int BluetoothHost::SetLocalName(const std::string &name)
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->SetLocalName(name);
}

int BluetoothHost::GetBtScanMode(int32_t &scanMode) const
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->GetBtScanMode(scanMode);
}

int BluetoothHost::SetBtScanMode(int mode, int duration)
{
    HILOGI("enter, mode: %{public}d, duration: %{public}d", mode, duration);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->SetBtScanMode(mode, duration);
}

int BluetoothHost::GetBondableMode(int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "failed: no proxy");

    return proxy->GetBondableMode(transport);
}

bool BluetoothHost::SetBondableMode(int transport, int mode)
{
    HILOGI("enter, transport: %{public}d, mode: %{public}d", transport, mode);
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return proxy->SetBondableMode(transport, mode);
}

int BluetoothHost::StartBtDiscovery()
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->StartBtDiscovery();
}

int BluetoothHost::CancelBtDiscovery()
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return proxy->CancelBtDiscovery();
}

int32_t BluetoothHost::IsBtDiscovering(bool &isDiscovering, int transport) const
{
    HILOGI("enter, transport: %{public}d", transport);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");

    return proxy->IsBtDiscovering(isDiscovering, transport);
}

long BluetoothHost::GetBtDiscoveryEndMillis() const
{
    HILOGD("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return 0;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, 0, "failed: no proxy");

    return proxy->GetBtDiscoveryEndMillis();
}

int32_t BluetoothHost::GetPairedDevices(int transport, std::vector<BluetoothRemoteDevice> &pairedDevices) const
{
    HILOGI("enter, transport: %{public}d", transport);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

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
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("Invalid remote device.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(device.GetDeviceAddr());
    return proxy->RemovePair(device.GetTransportType(), rawAddrSptr);
}

bool BluetoothHost::RemoveAllPairs()
{
    HILOGD("enter");
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

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
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, INVALID_VALUE, "failed: no proxy");

    return proxy->GetBleMaxAdvertisingDataLength();
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
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");
    return proxy->GetLocalProfileUuids(uuids);
}

int BluetoothHost::SetFastScan(bool isEnable)
{
    HILOGI("enter, isEnable: %{public}d", isEnable);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");
    return proxy->SetFastScan(isEnable);
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
} // namespace Bluetooth
} // namespace OHOS
