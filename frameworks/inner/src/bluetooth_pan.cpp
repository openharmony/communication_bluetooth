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
#define LOG_TAG "bt_fwk_pan"
#endif

#include "bluetooth_pan.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_pan_observer_stub.h"
#include "i_bluetooth_pan.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_proxyMutex;
class PanInnerObserver : public BluetoothPanObserverStub {
public:
    explicit PanInnerObserver(BluetoothObserverList<PanObserver> &observers) : observers_(observers)
    {
        HILOGD("enter");
    }
    ~PanInnerObserver() override
    {
        HILOGD("enter");
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}d, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), state, cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 1);
        observers_.ForEach([remoteDevice, state, cause](std::shared_ptr<PanObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state, cause);
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<PanObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PanInnerObserver);
};

struct Pan::impl {
    impl();
    ~impl();

    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("enter");
        sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "failed: no proxy");

        std::vector<BluetoothRawAddress> rawDevices;
        std::vector<int32_t> tmpStates;
        for (int32_t state : states) {
            tmpStates.push_back((int32_t)state);
        }

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
        sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    void RegisterObserver(std::shared_ptr<PanObserver> observer)
    {
        HILOGD("enter");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<PanObserver> observer)
    {
        HILOGI("enter");
        observers_.Deregister(observer);
    }

    int32_t SetTethering(bool value)
    {
        HILOGI("enter");
        sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "failed: no proxy");

        int32_t ret = proxy->SetTethering(value);
        HILOGI("fwk ret:%{public}d", ret);
        return ret;
    }

    int32_t IsTetheringOn(bool &value)
    {
        HILOGI("enter");
        sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "failed: no proxy");

        return proxy->IsTetheringOn(value);
    }
    int32_t profileRegisterId = 0;
private:

    BluetoothObserverList<PanObserver> observers_;
    sptr<PanInnerObserver> innerObserver_;
};

Pan::impl::impl()
{
    innerObserver_ = new PanInnerObserver(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_PAN_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothPan> proxy = iface_cast<IBluetoothPan>(remote);
        if (proxy == nullptr) {
            HILOGD("failed: no proxy");
            return;
        }
        proxy->RegisterObserver(innerObserver_);
    });
}

Pan::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_PAN_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(innerObserver_);
}

Pan::Pan()
{
    pimpl = std::make_unique<impl>();
}

Pan::~Pan()
{}

Pan *Pan::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<Pan> instance;
    return instance.get();
#else
    static Pan instance;
    return &instance;
#endif
}

int32_t Pan::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetDevicesByStates(states, result);
}

int32_t Pan::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetDeviceState(device, state);
}

int32_t Pan::Disconnect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->Disconnect(device);
}

void Pan::RegisterObserver(std::shared_ptr<PanObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->RegisterObserver(observer);
}

void Pan::DeregisterObserver(std::shared_ptr<PanObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->DeregisterObserver(observer);
}

int32_t Pan::SetTethering(bool value)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->SetTethering(value);
}

int32_t Pan::IsTetheringOn(bool &value)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothPan> proxy = GetRemoteProxy<IBluetoothPan>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->IsTetheringOn(value);
}
}  // namespace Bluetooth
}  // namespace OHOS