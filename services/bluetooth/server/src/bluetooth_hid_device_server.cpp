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
#define LOG_TAG "bt_server_hid_device"
#endif

#include "ipc_skeleton.h"
#include "bluetooth_def.h"
#include "common_util.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_hid_device_server.h"
#include "bluetooth_utils_server.h"
#include "hilog/log.h"
#include "interface_profile.h"
#include "interface_profile_hid_device.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "i_bluetooth_host_observer.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

const int32_t HID_DEVICE_OBSERVER_SIZE = 1000;
const int32_t HID_DEVICE_APPCONTAINER_SIZE = 1000;
const uint64_t INVALID_TOKEN = 0;

struct BluetoothHidDeviceServer::impl {
    impl();
    ~impl();

    uint64_t currentHapTokenId_{};
    std::mutex tokenIdMutex_;
    class BluetoothHidDeviceCallback;
    std::unique_ptr<BluetoothHidDeviceCallback> observerImp_ = nullptr;

    class BluetoothHidDeviceApplicationContainer;
    RemoteObserverList<IBluetoothHidDeviceObserver> observers_;
    std::shared_ptr<BluetoothHidDeviceApplicationContainer> appContainer_ = nullptr;
    IProfileHidDevice *hidDeviceService_ = nullptr;

    void SetHapTokenId(uint64_t tokenId)
    {
        std::lock_guard<std::mutex> lock(tokenIdMutex_);
        currentHapTokenId_ = tokenId;
    }

    uint64_t GetHapTokenId()
    {
        std::lock_guard<std::mutex> lock(tokenIdMutex_);
        return currentHapTokenId_;
    }

    IProfileHidDevice *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfileHidDevice *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_DEVICE));
    }
    class BluetoothHidDeviceApplicationContainer : public BluetoothObserverApplicationContainer {
    public:
        BluetoothHidDeviceApplicationContainer(BluetoothHidDeviceServer::impl &owner) : owner_(owner){};

        void AddObject(const sptr<IRemoteObject> &remote)
        {
            AddRemoteObject(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), remote);
            std::lock_guard<std::mutex> lock(containerMutex_);
            auto it = GetApplicationIter(remote);
            if (it != container_.end() && it->tokenId == 0) {
                it->tokenId = IPCSkeleton::GetCallingFullTokenID();
            }
        }
        uint64_t GetTokenId(const wptr<IRemoteObject> &remote)
        {
            std::lock_guard<std::mutex> lock(containerMutex_);
            auto it = GetApplicationIter(remote);
            if (it == container_.end()) {
                HILOGE("Unknown remote");
                return 0;
            }
            return it->tokenId;
        }
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            if (!Contain(remote)) {
                HILOGE("Invalid remote object");
                return;
            }
            HILOGI("hap close");
            owner_.SetHapTokenId(INVALID_TOKEN);
            auto hidDeviceService = static_cast<IProfileHidDevice *>(
                IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_DEVICE));
            if (hidDeviceService) {
                hidDeviceService->AutoUnregisterHidDevice();
            }

            RemoveRemoteObject(remote);
        }

    private:
        BluetoothHidDeviceServer::impl &owner_;
    };
};

class BluetoothHidDeviceServer::impl::BluetoothHidDeviceCallback : public IHidDeviceObserver {
public:
    explicit BluetoothHidDeviceCallback(BluetoothHidDeviceServer::impl *pimpl) :
        observers_(nullptr), pimpl_(pimpl) {};
    ~BluetoothHidDeviceCallback() override = default;

    void OnAppStatusChanged(int state) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, state](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnAppStatusChanged(state);
                if (state == HID_DEVICE_UNREGISTERED) {
                    pimpl_->SetHapTokenId(INVALID_TOKEN);
                }
            }
        });
    }

    void OnConnectionStateChanged(RawAddress device, int state) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, device, state](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            RawAddress randomAddr;
            if (tokenId == pimpl_->GetHapTokenId() &&
                BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state);
            }
        });
    }

    void OnGetReport(int type, int id, uint16_t bufferSize) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, type, id, bufferSize](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnGetReport(type, id, bufferSize);
            }
        });
    }

    void OnInterruptDataReceived(int reportId, std::vector<uint8_t> data) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, reportId, data](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnInterruptDataReceived(reportId, data);
            }
        });
    }

    void OnSetProtocol(int protocol) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, protocol](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnSetProtocol(protocol);
            }
        });
    }

    void OnSetReport(int type, int id, std::vector<uint8_t> data) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, type, id, data](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnSetReport(type, id, data);
            }

        });
    }

    void OnVirtualCableUnplug() override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this](sptr<IBluetoothHidDeviceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            if (tokenId == pimpl_->GetHapTokenId()) {
                observer->OnVirtualCableUnplug();
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothHidDeviceObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHidDeviceObserver> *observers_;
    BluetoothHidDeviceServer::impl *pimpl_ = nullptr;
};

BluetoothHidDeviceServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothHidDeviceApplicationContainer>(*this);
    appContainer_->Init();
}

BluetoothHidDeviceServer::impl::~impl()
{
}

BluetoothHidDeviceServer::BluetoothHidDeviceServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::BluetoothHidDeviceCallback>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));

    pimpl->hidDeviceService_ = pimpl->GetServicePtr();
    if (pimpl->hidDeviceService_ != nullptr) {
        pimpl->hidDeviceService_->RegisterObserver(*pimpl->observerImp_.get());
    }
}

BluetoothHidDeviceServer::~BluetoothHidDeviceServer()
{
    if (pimpl->hidDeviceService_ != nullptr) {
        pimpl->hidDeviceService_->DeregisterObserver(*pimpl->observerImp_.get());
    }
}

ErrCode BluetoothHidDeviceServer::RegisterObserver(const sptr<IBluetoothHidDeviceObserver> observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->observers_.Size() > HID_DEVICE_OBSERVER_SIZE ||
        pimpl->appContainer_->Size() > HID_DEVICE_APPCONTAINER_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return BT_ERR_INTERNAL_ERROR;
    }

    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
    return BT_NO_ERROR;
}



ErrCode BluetoothHidDeviceServer::DeregisterObserver(const sptr<IBluetoothHidDeviceObserver> observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceServer::Connect(const BluetoothRawAddress &device)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    int32_t addressType = device.GetAddressType();
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddrByAddrType(RawAddress(device), addressType, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->Connect(realAddr);
}

int32_t BluetoothHidDeviceServer::Disconnect()
{
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->DisconnectHidDevice();
}

int32_t BluetoothHidDeviceServer::GetConnectedDevices(std::vector<BluetoothRawAddress>& result)
{
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::list<bluetooth::RawAddress> serviceDeviceList = pimpl->hidDeviceService_->GetConnectDevices();
    auto resultVec = std::vector<bluetooth::RawAddress> (serviceDeviceList.begin(), serviceDeviceList.end());
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(resultVec, result);
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceServer::GetConnectionState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    state = pimpl->hidDeviceService_->GetConnectionState(realAddr);
    HILOGI("result:%{public}d", state);
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceServer::RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
    const BluetoothHidDeviceQos &inQos, const BluetoothHidDeviceQos &outQos)
{
    if (pimpl->hidDeviceService_ == nullptr) {
         HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->GetHapTokenId() != INVALID_TOKEN) {
        HILOGE("dismatched tokenID");
        return BT_ERR_HID_APP_HAS_BEEN_REGISTERED;
    }
    pimpl->SetHapTokenId(IPCSkeleton::GetCallingFullTokenID());
    return pimpl->hidDeviceService_->RegisterHidDevice(sdpSetting, inQos, outQos);
}

int32_t BluetoothHidDeviceServer::UnregisterHidDevice() {
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->UnregisterHidDevice();
}

int32_t BluetoothHidDeviceServer::SendReport(int id, const std::vector<uint8_t> &data)
{
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->SendReport(id, data);
}

int32_t BluetoothHidDeviceServer::ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data)
{
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->ReplyReport(static_cast<uint8_t>(type), id, data);
}

int32_t BluetoothHidDeviceServer::ReportError(ErrorReason type)
{
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidDeviceService_->ReportError(static_cast<uint8_t>(type));
}

int32_t BluetoothHidDeviceServer::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");

    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    return pimpl->hidDeviceService_->SetConnectStrategy(realAddr, strategy);
}

int32_t BluetoothHidDeviceServer::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hidDeviceService_ == nullptr) {
        HILOGE("hidDeviceService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    if (pimpl->hidDeviceService_) {
        strategy = pimpl->hidDeviceService_->GetConnectStrategy(realAddr);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
