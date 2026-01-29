/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_hid_device"
#endif

#include "bluetooth_hid_device.h"
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_hid_device_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_hid_device.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
class HidDeviceInnerObserver : public BluetoothHidDeviceObserverStub {
public:
    explicit HidDeviceInnerObserver(BluetoothObserverList<HidDeviceObserver> &observers) : observers_(observers)
    {
        HILOGD("enter!");
    }
    ~HidDeviceInnerObserver() override
    {
        HILOGD("enter!");
    }

    ErrCode OnAppStatusChanged(int state) override
    {
        HILOGI("hid device app status changed, state: %{public}d", state);
        observers_.ForEach([state](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnAppStatusChanged(state);
        });
        return NO_ERROR;
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int state) override
    {
        HILOGD("hid device conn state, device: %{public}s, state: %{public}s",
            GET_ENCRYPT_RAW_ADDR(device), GetProfileConnStateName(state).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BT_TRANSPORT_BREDR);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
        return NO_ERROR;
    }

    ErrCode OnGetReport(int type, int id, uint16_t bufferSize) override
    {
        HILOGD("enter");
        observers_.ForEach([type, id, bufferSize](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnGetReport(type, id, bufferSize);
        });
        return NO_ERROR;
    }

    ErrCode OnInterruptDataReceived(int id, std::vector<uint8_t> data) override
    {
        HILOGD("enter");
        observers_.ForEach([id, data](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnInterruptDataReceived(id, data);
        });
        return NO_ERROR;
    }

    ErrCode OnSetProtocol(int protocol) override
    {
        HILOGD("enter");
        observers_.ForEach([protocol](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnSetProtocol(protocol);
        });
        return NO_ERROR;
    }

    ErrCode OnSetReport(int type, int id, std::vector<uint8_t> data) override
    {
        HILOGD("enter");
        observers_.ForEach([type, id, data](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnSetReport(type, id, data);
        });
        return NO_ERROR;
    }

    ErrCode OnVirtualCableUnplug() override
    {
        HILOGD("enter");
        observers_.ForEach([](std::shared_ptr<HidDeviceObserver> observer) {
            observer->OnVirtualCableUnplug();
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<HidDeviceObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidDeviceInnerObserver);
};

struct HidDevice::impl {
    impl();
    ~impl();

    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("enter!");
        std::vector<BluetoothRawAddress> rawDevices;
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        int32_t ret = proxy->GetConnectedDevices(rawDevices);
        if (ret != BT_NO_ERROR) {
            HILOGE("inner error.");
            return ret;
        }

        for (BluetoothRawAddress &rawDevice : rawDevices) {
            BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), BT_TRANSPORT_BREDR);
            result.push_back(remoteDevice);
        }
        return BT_NO_ERROR;
    }

    int32_t GetConnectionState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        return proxy->GetConnectionState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    void RegisterObserver(std::shared_ptr<HidDeviceObserver> observer)
    {
        HILOGD("enter!");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HidDeviceObserver> observer)
    {
        HILOGD("enter!");
        observers_.Deregister(observer);
    }

    int RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
        const BluetoothHidDeviceQos &inQos,
        const BluetoothHidDeviceQos &outQos)
    {
        HILOGI("enter!");
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->RegisterHidDevice(sdpSetting, inQos, outQos);
    }

    int UnregisterHidDevice()
    {
        HILOGI("enter!");
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->UnregisterHidDevice();
    }

    int32_t Connect(const AddressInfo &device)
    {
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        HILOGI("hid connect remote device: %{public}s", GET_ENCRYPT_RAW_ADDR(device));
        return proxy->Connect(BluetoothRawAddress(device.GetAddressType(), device.GetAddress()));
    }

    int32_t Disconnect()
    {
        HILOGI("hid disconnect remote device");
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->Disconnect();
    }

    int32_t SendReport(int id, const std::vector<uint8_t> &data)
    {
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->SendReport(id, data);
    }

    int32_t ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data)
    {
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->ReplyReport(type, id, data);
    }

    int32_t ReportError(ErrorReason type)
    {
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->ReportError(type);
    }

    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        HILOGI("enter");
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->SetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
    {
        HILOGI("enter");
        sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
        return proxy->GetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int32_t profileRegisterId = 0;
private:
    BluetoothObserverList<HidDeviceObserver> observers_;
    sptr<HidDeviceInnerObserver> innerObserver_;
};

HidDevice::impl::impl()
{
    innerObserver_ = new HidDeviceInnerObserver(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_HID_DEVICE_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHidDevice> proxy = iface_cast<IBluetoothHidDevice>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(innerObserver_);
    });
}

HidDevice::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHidDevice> proxy = GetRemoteProxy<IBluetoothHidDevice>(PROFILE_HID_DEVICE_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(innerObserver_);
}

HidDevice::HidDevice()
{
    pimpl = std::make_unique<impl>();
}

HidDevice::~HidDevice() {}

HidDevice *HidDevice::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<HidDevice> instance;
    return instance.get();
#else
    static HidDevice instance;
    return &instance;
#endif
}

void HidDevice::RegisterObserver(std::shared_ptr<HidDeviceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(observer != nullptr, "observer is null.");
    pimpl->RegisterObserver(observer);
}

void HidDevice::DeregisterObserver(std::shared_ptr<HidDeviceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(observer != nullptr, "observer is null.");
    pimpl->DeregisterObserver(observer);
}

int32_t HidDevice::GetConnectedDevices(std::vector<BluetoothRemoteDevice> &result)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->GetConnectedDevices(result);
}

int32_t HidDevice::GetConnectionState(const BluetoothRemoteDevice &device, int32_t &state)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->GetConnectionState(device, state);
}


int HidDevice::RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
    const BluetoothHidDeviceQos &inQos,
    const BluetoothHidDeviceQos &outQos)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->RegisterHidDevice(sdpSetting, inQos, outQos);
}

int HidDevice::UnregisterHidDevice()
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->UnregisterHidDevice();
}

int32_t HidDevice::Connect(const AddressInfo &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->Connect(device);
}

int32_t HidDevice::Disconnect()
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->Disconnect();
}

int32_t HidDevice::SendReport(int id, const std::vector<uint8_t> &data)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->SendReport(id, data);
}

int32_t HidDevice::ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->ReplyReport(type, id, data);
}

int32_t HidDevice::ReportError(ErrorReason type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }
    return pimpl->ReportError(type);
}

int HidDevice::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->SetConnectStrategy(device, strategy);
}

int HidDevice::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    int error = CheckProfileState();
    if (error != BT_NO_ERROR) {
        return error;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->GetConnectStrategy(device, strategy);
}

int32_t HidDevice::CheckProfileState() const
{
    sptr<IBluetoothHost> proxy = GetRemoteProxy<IBluetoothHost>(BLUETOOTH_HOST);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    bool isProfileExist = false;
    int32_t ret = proxy->IsProfileExist(PROFILE_HID_DEVICE_SERVER, isProfileExist);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "IPC fail");
    CHECK_AND_RETURN_LOG_RET(isProfileExist, BT_ERR_API_NOT_SUPPORT, "failed: api not support");
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS