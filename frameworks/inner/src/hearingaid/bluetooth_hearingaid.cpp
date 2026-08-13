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
#define LOG_TAG "bt_fwk_hearingaid"
#endif

#include "bluetooth_hearingaid.h"
#include "bluetooth_hearing_aid_observer_stub.h"
#include "bluetooth_hearing_aid_proxy.h"
#include "bluetooth_override_errorcode.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_host.h"
#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {

struct BluetoothHearingAid::impl {
    impl();
    ~impl();
    BluetoothObserverList<HearingAidObserver> observers_;
    class BluetoothHearingAidObserverImp;
    sptr<BluetoothHearingAidObserverImp> observerImp_ = nullptr;
    int32_t profileRegisterId = 0;
};

class BluetoothHearingAid::impl::BluetoothHearingAidObserverImp : public BluetoothHearingAidObserverStub {
public:
    explicit BluetoothHearingAidObserverImp(BluetoothHearingAid::impl &hearingAid) :
        hearingAid_(hearingAid) {};

    ErrCode OnConnectionStateChanged(const std::string& address, ConnectionState state, int32_t cause) override
    {
        int32_t connectionState = static_cast<int32_t>(state);
        HILOGI("connectionState: %{public}d", connectionState);
        hearingAid_.observers_.ForEach([address, connectionState, cause](std::shared_ptr<HearingAidObserver> observer) {
            observer->OnConnectionStateChanged(address, connectionState, cause);
        });
        return 0;
    }

    ErrCode OnActiveDeviceChanged(const std::string& address, const std::string& name) override
    {
        HILOGI("onActiveDeviceChanged");
        hearingAid_.observers_.ForEach([address, name](std::shared_ptr<HearingAidObserver> observer) {
            observer->OnActiveDeviceChanged(address, name);
        });
        return 0;
    }
private:
    BluetoothHearingAid::impl &hearingAid_;
};

BluetoothHearingAid::impl::impl()
{
    observerImp_ = new (std::nothrow) BluetoothHearingAidObserverImp(*this);
    CHECK_AND_RETURN_LOG(observerImp_ != nullptr, "observerImp_ is nullptr");
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_HEARINGAID_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHearingAid> proxy = iface_cast<IBluetoothHearingAid>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(observerImp_);
    });
}

BluetoothHearingAid::impl::~impl()
{
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeRegisterObserver(observerImp_);
}

BluetoothHearingAid::BluetoothHearingAid()
{
    pimpl = std::make_unique<impl>();
}

BluetoothHearingAid* BluetoothHearingAid::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<BluetoothHearingAid> instance;
    return instance.get();
#else
    static BluetoothHearingAid instance;
    return &instance;
#endif
}

void BluetoothHearingAid::RegisterObserver(std::shared_ptr<HearingAidObserver> observer)
{
    HILOGI("RegisterObserver");
    pimpl->observers_.Register(observer);
}

void BluetoothHearingAid::DeregisterObserver(std::shared_ptr<HearingAidObserver> observer)
{
    HILOGI("DeregisterObserver");
    pimpl->observers_.Deregister(observer);
}

int32_t BluetoothHearingAid::IsLocalDeviceSupportHearingAid(bool& support)
{
    HILOGI("IsLocalDeviceSupportHearingAid");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    support = false;
    return proxy->IsLocalDeviceSupportHearingAid(support);
}

int32_t BluetoothHearingAid::Connect(const std::string& address)
{
    HILOGI("Connect");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    return proxy->Connect(address);
}

int32_t BluetoothHearingAid::Disconnect(const std::string& address)
{
    HILOGI("Disconnect");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    return proxy->Disconnect(address);
}

int32_t BluetoothHearingAid::SetVolume(const std::string& address, int64_t volume)
{
    HILOGI("SetVolume volume: %{public}ld", volume);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");

    if (volume < VOLUME_MIN || volume > VOLUME_MAX) {
        HILOGE("volume: %{public}ld is invalid", volume);
        return BT_OV_ERR_INVALID_PARAM;
    }

    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    return proxy->SetVolume(address, volume);
}

int32_t BluetoothHearingAid::GetHearingAidDeviceInfo(const std::string& address,
    int32_t& side, int32_t& mode, int64_t& id)
{
    HILOGI("GetHearingAidDeviceInfo");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    DeviceSide deviceSide;
    DeviceMode deviceMode;
    auto ret = proxy->GetHearingAidDeviceInfo(address, deviceSide, deviceMode, id);
    side = static_cast<int32_t>(deviceSide);
    mode = static_cast<int32_t>(deviceMode);
    return ret;
}

int32_t BluetoothHearingAid::GetConnectedDevices(std::vector<std::string>& connectedDevices)
{
    HILOGI("GetConnectedDevices");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    return proxy->GetConnectedDevices(connectedDevices);
}

int32_t BluetoothHearingAid::GetActiveDevice(std::string& address, std::string& name)
{
    HILOGI("GetActiveDevice");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    return proxy->GetActiveDevice(address, name);
}

int32_t BluetoothHearingAid::GetConnectionState(const std::string& address, int32_t& state)
{
    HILOGD("GetConnectionState");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    ConnectionState connectionState;
    auto ret = proxy->GetConnectionState(address, connectionState);
    switch (connectionState) {
        case ConnectionState::STATE_CONNECTING:
            state = static_cast<int32_t>(ProfileConnectionState::STATE_CONNECTING);
            break;
        case ConnectionState::STATE_CONNECTED:
            state = static_cast<int32_t>(ProfileConnectionState::STATE_CONNECTED);
            break;
        case ConnectionState::STATE_DISCONNECTING:
            state = static_cast<int32_t>(ProfileConnectionState::STATE_DISCONNECTING);
            break;
        case ConnectionState::STATE_DISCONNECTED:
            state = static_cast<int32_t>(ProfileConnectionState::STATE_DISCONNECTED);
            break;
        default:
            state = static_cast<int32_t>(ProfileConnectionState::STATE_DISCONNECTED);
            break;
    }
    return ret;
}

int32_t BluetoothHearingAid::SetConnectionStrategy(const std::string& address, int32_t strategy)
{
    HILOGI("SetConnectionStrategy strategy: %{public}d", strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");

    if (strategy < STRATEGY_MIN || strategy > STRATEGY_MAX) {
        HILOGE("strategy: %{public}d is invalid", strategy);
        return BT_OV_ERR_INVALID_PARAM;
    }

    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    ConnectionStrategy connectionStrategy = static_cast<ConnectionStrategy>(strategy);
    return proxy->SetConnectionStrategy(address, connectionStrategy);
}

int32_t BluetoothHearingAid::GetConnectionStrategy(const std::string& address, int32_t& strategy)
{
    HILOGI("GetConnectionStrategy");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_OV_ERR_SERVICE_DISABLED;
    }
    CHECK_AND_RETURN_LOG_RET(BluetoothHost::IsValidBluetoothAddr(address), BT_OV_ERR_INVALID_PARAM,
        "invalid bluetooth addr");
    sptr<IBluetoothHearingAid> proxy = GetRemoteProxy<IBluetoothHearingAid>(PROFILE_HEARINGAID_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_OV_ERR_SERVICE_STOPPED, "failed: no proxy");
    ConnectionStrategy connectionStrategy;
    auto ret = proxy->GetConnectionStrategy(address, connectionStrategy);
    strategy = static_cast<int32_t>(connectionStrategy);
    return ret;
}

}  // namespace Bluetooth
}  // namespace OHOS