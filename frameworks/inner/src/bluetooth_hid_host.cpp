/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_his_host"
#endif

#include "bluetooth_hid_host.h"
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_hid_host_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_hid_host.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_hidProxyMutex;
class HidHostInnerObserver : public BluetoothHidHostObserverStub {
public:
    explicit HidHostInnerObserver(BluetoothObserverList<HidHostObserver> &observers) : observers_(observers)
    {
        HILOGD("Enter!");
    }
    ~HidHostInnerObserver() override
    {
        HILOGD("Enter!");
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGD("hid conn state, device: %{public}s, state: %{public}s, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), GetProfileConnStateName(state).c_str(), cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state, cause](std::shared_ptr<HidHostObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state, cause);
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<HidHostObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidHostInnerObserver);
};

struct HidHost::impl {
    impl();
    ~impl();

    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("Enter!");
        std::vector<BluetoothRawAddress> rawDevices;
        std::vector<int32_t> tmpStates;
        for (int32_t state : states) {
            tmpStates.push_back((int32_t)state);
        }
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "failed: no proxy");
        int32_t ret = proxy->GetDevicesByStates(tmpStates, rawDevices);
        if (ret != BT_NO_ERROR) {
            HILOGE("inner error.");
            return ret;
        }

        for (BluetoothRawAddress rawDevice : rawDevices) {
            BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 1);
            result.push_back(remoteDevice);
        }

        return BT_NO_ERROR;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        HILOGI("hid connect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        return proxy->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hid disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    void RegisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        HILOGD("Enter!");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        HILOGI("Enter!");
        observers_.Deregister(observer);
    }

    void HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy != nullptr) {
            proxy->HidHostVCUnplug(device, id, size, type, result);
        }
    }

    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy != nullptr) {
            proxy->HidHostSendData(device, id, size, type, result);
        }
    }

    void HidHostSetReport(std::string device, uint8_t type, std::string &report)
    {
        HILOGI("Enter!");
        int result;
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy != nullptr) {
            proxy->HidHostSetReport(device, type, report, result);
        }
    }

    void HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            proxy->HidHostGetReport(device, id, size, type, result);
        }
    }

    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        HILOGI("enter");
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");
        return proxy->SetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
    {
        HILOGI("enter");
        sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");
        return proxy->GetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int32_t profileRegisterId = 0;
private:
    BluetoothObserverList<HidHostObserver> observers_;
    sptr<HidHostInnerObserver> innerObserver_;
};

HidHost::impl::impl()
{
    innerObserver_ = new HidHostInnerObserver(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_HID_HOST_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHidHost> proxy = iface_cast<IBluetoothHidHost>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(innerObserver_);
    });
}

HidHost::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(innerObserver_);
}

HidHost::HidHost()
{
    pimpl = std::make_unique<impl>();
}

HidHost::~HidHost() {}

HidHost *HidHost::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<HidHost> instance;
    return instance.get();
#else
    static HidHost instance;
    return &instance;
#endif
}

int32_t HidHost::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetDevicesByStates(states, result);
}

int32_t HidHost::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetDeviceState(device, state);
}

int32_t HidHost::Connect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->Connect(device);
}

int32_t HidHost::Disconnect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->Disconnect(device);
}

int HidHost::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->SetConnectStrategy(device, strategy);
}

int HidHost::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->GetConnectStrategy(device, strategy);
}

void HidHost::RegisterObserver(std::shared_ptr<HidHostObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->RegisterObserver(observer);
}

void HidHost::DeregisterObserver(std::shared_ptr<HidHostObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->DeregisterObserver(observer);
}

void HidHost::HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    return pimpl->HidHostVCUnplug(device, id, size, type);
}

void HidHost::HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    return pimpl->HidHostSendData(device, id, size, type);
}

void HidHost::HidHostSetReport(std::string device, uint8_t type, std::string &report)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    return pimpl->HidHostSetReport(device, type, report);
}

void HidHost::HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothHidHost> proxy = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    return pimpl->HidHostGetReport(device, id, size, type);
}
} // namespace Bluetooth
} // namespace OHOS