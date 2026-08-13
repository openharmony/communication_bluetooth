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
#define LOG_TAG "bt_server_bas"
#endif

#include "bluetooth_bas_server.h"

#include "interface_adapter_manager.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_observer_application_container.h"
#include "interface_profile_bas.h"
#include "interface_profile_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "../../common/log.h"
#include "common_util.h"
#include "hitrace_meter.h"
#include "safe_map.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

constexpr int32_t MAX_BATTERY_REQUEST_MAP_SIZE = 50;
constexpr int32_t MAX_OBSERVER_LIST_SIZE = 20;

static bool CheckIfCanCallback(
    const std::shared_ptr<BluetoothObserverApplicationContainer> &container,
    const sptr<IRemoteObject> &remoteObject)
{
    if (container == nullptr || remoteObject == nullptr) {
        HILOGE("container or observer is nullptr");
        return false;
    }
    int32_t pid = container->GetRemotePid(remoteObject);
    if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
        HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
        return false;
    }
    uint64_t tokenId = container->GetTokenId(remoteObject);
    if (!PermissionManager::IsSystemHap(tokenId) ||
        !PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId)) {
        HILOGE("[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
            PermissionManager::GetCallingName(tokenId).c_str());
        return false;
    }
    return true;
}

static IProfileBas* GetBasService()
{
    auto profileManager = IProfileManager::GetInstance();
    if (profileManager == nullptr) {
        HILOGE("profileManager is nullptr");
        return nullptr;
    }
    return static_cast<IProfileBas *>(profileManager->GetProfileService(PROFILE_NAME_BAS));
}

struct BluetoothBasServer::impl {
    impl();
    ~impl();

    class BasObserver;
    std::shared_ptr<IBasObserver> basObserverImp_ = nullptr;
    RemoteObserverList<IBluetoothDeviceBatteryObserver> batteryObservers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> batteryAppContainer_ = nullptr;
    SafeMap<RawAddress, std::vector<sptr<IBluetoothDeviceBatteryObserver>>> batteryReqInfoMap_;

    BluetoothBasServer* server_ = nullptr;
};

class BluetoothBasServer::impl::BasObserver : public IBasObserver {
public:
    explicit BasObserver(BluetoothBasServer::impl *impl) : impl_(impl){};
    ~BasObserver() override = default;

    void OnGetBatteryLevelEvent(const RawAddress &device, int32_t batteryLevel) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
        std::vector<sptr<IBluetoothDeviceBatteryObserver>> tmpList;
        if (!impl_->batteryReqInfoMap_.Find(device, tmpList)) {
            HILOGE("no observer");
            return;
        }
        impl_->batteryReqInfoMap_.ChangeValueByLambda<>(device,
            [this, &device, &batteryLevel](std::vector<sptr<IBluetoothDeviceBatteryObserver>> &observerList) {
                for (auto &observer : observerList) {
                    if (observer == nullptr ||
                        !CheckIfCanCallback(this->impl_->batteryAppContainer_, observer->AsObject())) {
                        continue;
                    }
                    uint64_t tokenId = impl_->batteryAppContainer_->GetTokenId(observer->AsObject());
                    RawAddress randomAddr;
                    if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                        tokenId, device, randomAddr)) {
                        observer->OnGetBatteryLevelEvent(randomAddr, batteryLevel);
                    }
                }
            });
        impl_->batteryReqInfoMap_.Erase(device);
    }

    void OnBatteryLevelChanged(const RawAddress &device, int32_t batteryLevel) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
        impl_->batteryObservers_.ForEach([this, device, batteryLevel](IBluetoothDeviceBatteryObserver *observer) {
            if (observer == nullptr || !CheckIfCanCallback(this->impl_->batteryAppContainer_, observer->AsObject())) {
                return;
            }
            uint64_t tokenId = this->impl_->batteryAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnBatteryLevelChanged(randomAddr, batteryLevel);
            }
        });
    }

private:
    BluetoothBasServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BasObserver);
};

BluetoothBasServer::impl::impl()
{
    basObserverImp_ = std::make_unique<BasObserver>(this);
    batteryAppContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    batteryAppContainer_->Init();
}

BluetoothBasServer::impl::~impl() {}

BluetoothBasServer::BluetoothBasServer()
{
    pimpl = std::make_unique<impl>();
    IProfileBas* basService = GetBasService();
    if (basService != nullptr) {
        basService->RegisterObserver(pimpl->basObserverImp_);
    }
}

BluetoothBasServer::~BluetoothBasServer()
{
    IProfileBas* basService = GetBasService();
    if (basService != nullptr) {
        basService->DeregisterObserver(pimpl->basObserverImp_);
    }
}

int32_t BluetoothBasServer::RegisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return BT_ERR_INVALID_PARAM;
    }
    if (pimpl->batteryAppContainer_->Size() > MAX_MAP_SIZE || pimpl->batteryObservers_.Size() > MAX_MAP_SIZE) {
        HILOGE("batteryAppContainer_ or batteryObservers_ too much");
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->batteryAppContainer_->AddObject(observer->AsObject());
    pimpl->batteryObservers_.Register(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothBasServer::DeregisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return BT_ERR_INVALID_PARAM;
    }
    pimpl->batteryObservers_.Deregister(observer);
    pimpl->batteryAppContainer_->RemoveRemoteObject(observer->AsObject());
    return BT_NO_ERROR;
}

sptr<IBluetoothDeviceBatteryObserver> BluetoothBasServer::FindCachedObserver(uint64_t fullTokenId,
    int32_t callingUid)
{
    sptr<IBluetoothDeviceBatteryObserver> cachedObserver = nullptr;
    pimpl->batteryObservers_.ForEach([this, &fullTokenId, &callingUid, &cachedObserver](
        const sptr<IBluetoothDeviceBatteryObserver>& observer) {
        if (observer != nullptr) {
            int32_t observerUid = pimpl->batteryAppContainer_->GetRemoteUid(observer->AsObject());
            uint64_t observerTokenId = pimpl->batteryAppContainer_->GetTokenId(observer->AsObject());
            if (observerTokenId == fullTokenId && observerUid == callingUid) {
                cachedObserver = observer;
            }
        }
    });
    return cachedObserver;
}

bool BluetoothBasServer::CheckIfBasRequestExist(
    const std::vector<sptr<IBluetoothDeviceBatteryObserver>> &observerList,
    uint64_t fullTokenId, int32_t callingUid)
{
    for (auto& observer : observerList) {
        if (observer == nullptr) {
            continue;
        }
        int32_t observerUid = pimpl->batteryAppContainer_->GetRemoteUid(observer->AsObject());
        uint64_t observerTokenId = pimpl->batteryAppContainer_->GetTokenId(observer->AsObject());
        if (observerTokenId == fullTokenId && observerUid == callingUid) {
            HILOGE("duplicate battery request for same device");
            return true;
        }
    }
    return false;
}

int32_t BluetoothBasServer::GetBatteryLevel(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGI("real address not exist");
        realAddr = addr;
    }

    IProfileBas* basService = GetBasService();
    CHECK_AND_RETURN_LOG_RET(basService != nullptr, BT_ERR_INTERNAL_ERROR, "basService is nullptr!");

    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    sptr<IBluetoothDeviceBatteryObserver> cachedObserver = FindCachedObserver(fullTokenId, callingUid);
    CHECK_AND_RETURN_LOG_RET(cachedObserver != nullptr,
        BT_ERR_INTERNAL_ERROR, "no matching observer found for this caller");

    std::vector<sptr<IBluetoothDeviceBatteryObserver>> tmpList;
    int mapSize = pimpl->batteryReqInfoMap_.Size();
    if (mapSize >= MAX_BATTERY_REQUEST_MAP_SIZE && !pimpl->batteryReqInfoMap_.Find(realAddr, tmpList)) {
        HILOGE("batteryReqInfoMap_ oversize");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = BT_NO_ERROR;
    pimpl->batteryReqInfoMap_.ChangeValueByLambda<>(realAddr,
        [&, this](std::vector<sptr<IBluetoothDeviceBatteryObserver>> &observerList) {
            if (observerList.size() > 0 && observerList.size() <= MAX_OBSERVER_LIST_SIZE) {
                if (CheckIfBasRequestExist(observerList, fullTokenId, callingUid)) {
                    HILOGE("duplicate battery request for same device");
                    return;
                }
                observerList.push_back(cachedObserver);
                return;
            }
            if (observerList.size() >= MAX_OBSERVER_LIST_SIZE) {
                HILOGE("observerList oversize");
                ret = BT_ERR_INTERNAL_ERROR;
                return;
            }
            ret = basService->GetBatteryLevel(realAddr);
            if (ret == BT_NO_ERROR) {
                observerList.push_back(cachedObserver);
            }
        });
    return ret;
}

int32_t BluetoothBasServer::GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos)
{
    IProfileBas* basService = GetBasService();
    CHECK_AND_RETURN_LOG_RET(basService != nullptr, BT_ERR_INTERNAL_ERROR, "basService is nullptr!");

    std::map<RawAddress, int32_t> batteryCache = basService->GetConnectedDeviceBatteryInfos();
    const int32_t maxBatteryLevel = 100;
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    for (const auto& [device, batteryLevel] : batteryCache) {
        RawAddress randomAddr;
        if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(fullTokenId, device, randomAddr)) {
            batteryInfos[randomAddr.GetAddress()] = batteryLevel;
        }
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS