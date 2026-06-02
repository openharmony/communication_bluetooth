/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_bas_host"
#endif

#include "bluetooth_bas_host.h"
#include "bluetooth_host.h"
#include <memory>
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_device_battery_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_bas.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
namespace {
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
const std::string ACCESS_BLUETOOTH = "ohos.permission.ACCESS_BLUETOOTH";
#endif
}

struct BluetoothBasHost::impl {
    impl();
    ~impl();

    int32_t profileRegisterId = 0;
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    // remote device observer
    class BluetoothDeviceBatteryObserverImp;
    sptr<BluetoothDeviceBatteryObserverImp> deviceBatteryObserverImp_ = nullptr;

    // user regist remote battery observers
    BluetoothObserverList<BluetoothRemoteDeviceBatteryObserver> remoteBatteryObservers_;
#endif
};

#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
class BluetoothBasHost::impl::BluetoothDeviceBatteryObserverImp : public BluetoothDeviceBatteryObserverStub {
public:
    explicit BluetoothDeviceBatteryObserverImp(BluetoothBasHost::impl &host) : host_(host) {}
    ~BluetoothDeviceBatteryObserverImp() override = default;

    void OnGetBatteryLevelEvent(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        HILOGD("enter, device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_RAW_ADDR(device), batteryLevel);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BLE);
        host_.remoteBatteryObservers_.ForEach(
            [remoteDevice, batteryLevel](std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer) {
                if (observer != nullptr) {
                    observer->OnGetBatteryLevelEvent(remoteDevice, batteryLevel);
                }
            });
    }

    void OnBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        HILOGD("enter, device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_RAW_ADDR(device), batteryLevel);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BLE);
        host_.remoteBatteryObservers_.ForEach(
            [remoteDevice, batteryLevel](std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer) {
                if (observer != nullptr) {
                    observer->OnBatteryLevelChanged(remoteDevice, batteryLevel);
                }
            });
    }

private:
    BluetoothBasHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothDeviceBatteryObserverImp);
};
#endif

BluetoothBasHost::impl::impl()
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    deviceBatteryObserverImp_ = new BluetoothDeviceBatteryObserverImp(*this);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_BAS_SERVER,
        [this](sptr<IRemoteObject> remote) {
            sptr<IBluetoothBas> proxy = iface_cast<IBluetoothBas>(remote);
            CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
            proxy->RegisterDeviceBatteryObserver(deviceBatteryObserverImp_);
        });
#endif
}

BluetoothBasHost::impl::~impl()
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothBas> proxy = GetRemoteProxy<IBluetoothBas>(PROFILE_BAS_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    proxy->DeregisterDeviceBatteryObserver(deviceBatteryObserverImp_);
#endif
}

BluetoothBasHost::BluetoothBasHost()
{
    pimpl = std::make_unique<impl>();
}

BluetoothBasHost::~BluetoothBasHost() {}

BluetoothBasHost *BluetoothBasHost::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<BluetoothBasHost> instance;
    return instance.get();
#else
    static BluetoothBasHost instance;
    return &instance;
#endif
}

int32_t BluetoothBasHost::IsBasSupported(bool &isSupported)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    isSupported = true;
#else
    isSupported = false;
#endif
    return BT_NO_ERROR;
}

int32_t BluetoothBasHost::RegisterBatteryObserver(std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, BT_ERR_INVALID_PARAM, "observer is null.");

    std::map<std::string, int32_t> batteryInfos;
    int32_t ret = GetConnectedDeviceBatteryInfos(batteryInfos);
    if (ret != BT_NO_ERROR) {
        HILOGE("GetConnectedDeviceBatteryInfos failed, ret: %{public}d", ret);
    }
    const int32_t maxBatteryLevel = 100;
    for (const auto &[deviceAddr, batteryLevel] : batteryInfos) {
        if (batteryLevel < 0 || batteryLevel > maxBatteryLevel ||
            !BluetoothHost::IsValidBluetoothAddr(deviceAddr)) {
            continue;
        }
        BluetoothRemoteDevice remoteDevice(deviceAddr, BTTransport::ADAPTER_BLE);
        observer->OnBatteryLevelChanged(remoteDevice, batteryLevel);
    }
    pimpl->remoteBatteryObservers_.Register(observer);
    return BT_NO_ERROR;
#else
    return BT_ERR_API_NOT_SUPPORT;
#endif
}

int32_t BluetoothBasHost::DeregisterBatteryObserver(std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, BT_ERR_INVALID_PARAM, "observer is null.");
    pimpl->remoteBatteryObservers_.Deregister(observer);
    return BT_NO_ERROR;
#else
    return BT_ERR_API_NOT_SUPPORT;
#endif
}

int32_t BluetoothBasHost::GetBatteryLevel(const std::string &address)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothBas> proxy = GetRemoteProxy<IBluetoothBas>(PROFILE_BAS_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    return proxy->GetBatteryLevel(address);
#else
    return BT_ERR_API_NOT_SUPPORT;
#endif
}

int32_t BluetoothBasHost::GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothBas> proxy = GetRemoteProxy<IBluetoothBas>(PROFILE_BAS_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    return proxy->GetConnectedDeviceBatteryInfos(batteryInfos);
#else
    return BT_ERR_API_NOT_SUPPORT;
#endif
}
} // namespace Bluetooth
} // namespace OHOS