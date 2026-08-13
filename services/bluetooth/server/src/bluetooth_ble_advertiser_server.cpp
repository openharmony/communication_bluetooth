/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_ble_advertiser"
#endif

#include "bluetooth_ble_advertiser_server.h"

#include <sstream>

#include "bluetooth_ble_advertiser_application_container.h"
#include "log_utils.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "ble_defs.h"
#include "bt_chr_dft_statictics.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "hitrace_meter.h"
#include "parameters.h"
#include "parameter.h"
#include "bluetooth_utils_server.h"

#define ADVERTISE_FAILED_TOO_MANY_ADVERTISERS 0x02

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
class BleAdvertiserCallback : public IBleAdvertiserCallback {
public:
    explicit BleAdvertiserCallback(const std::weak_ptr<BluetoothBleAdvertiserApplicationContainer> &applications)
        : applications_(applications) {}
    ~BleAdvertiserCallback() override = default;

    void OnStartResultEvent(int result, uint8_t advHandle, int opcode) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("result:%{public}d, advHandle:%{public}d, opcode:%{public}d", result, advHandle, opcode);
        auto sptr = applications_.lock();
        observers_->ForEach([this, result, advHandle, opcode, sptr](IBluetoothBleAdvertiseCallback *observer) {
            if (sptr) {
                int32_t pid = sptr->GetRemotePid(observer->AsObject());
                int32_t uid = sptr->GetRemoteUid(observer->AsObject());
                if (!sptr->IsApplicationAdv(pid, uid, advHandle)) {
                    return;
                }
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
            }
            observer->OnStartResultEvent(result, advHandle, opcode);
        });
        if (sptr && result != ADVERTISE_SUCCESS && opcode == BLE_ADV_START_FAILED_OP_CODE) {
            sptr->RemoveAdvHandle(advHandle);
        }
    }

    void OnEnableResultEvent(int result, uint8_t advHandle) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
        observers_->ForEach([this, result, advHandle](IBluetoothBleAdvertiseCallback *observer) {
            auto sptr = applications_.lock();
            if (sptr) {
                int32_t pid = sptr->GetRemotePid(observer->AsObject());
                int32_t uid = sptr->GetRemoteUid(observer->AsObject());
                if (!sptr->IsApplicationAdv(pid, uid, advHandle)) {
                    return;
                }
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
            }
            observer->OnEnableResultEvent(result, advHandle);
        });
    }

    void OnDisableResultEvent(int result, uint8_t advHandle) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
        observers_->ForEach([this, result, advHandle](IBluetoothBleAdvertiseCallback *observer) {
            auto sptr = applications_.lock();
            if (sptr) {
                int32_t pid = sptr->GetRemotePid(observer->AsObject());
                int32_t uid = sptr->GetRemoteUid(observer->AsObject());
                if (!sptr->IsApplicationAdv(pid, uid, advHandle)) {
                    return;
                }
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
            }
            observer->OnDisableResultEvent(result, advHandle);
        });
    }

    void OnStopResultEvent(int result, uint8_t advHandle) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
        auto sptr = applications_.lock();
        observers_->ForEach([result, advHandle, sptr](IBluetoothBleAdvertiseCallback *observer) {
            if (sptr) {
                int32_t pid = sptr->GetRemotePid(observer->AsObject());
                int32_t uid = sptr->GetRemoteUid(observer->AsObject());
                if (!sptr->IsApplicationAdv(pid, uid, advHandle)) {
                    return;
                }
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
            }
            observer->OnStopResultEvent(result, advHandle);
        });
        if (sptr) {
            sptr->RemoveAdvHandle(advHandle);
            if (sptr->preemptingAdvHandle == advHandle) {
                std::unique_lock<std::mutex> lock(sptr->advPreemptionLock);
                HILOGI("advHandle: %{public}d has been preempted successfully.", advHandle);
                sptr->preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
                sptr->advPreemptionCV.notify_all();
            }
        }
    }

    void OnAutoStopAdvEvent(uint8_t advHandle) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("advHandle: %{public}d", advHandle);
        auto sptr = applications_.lock();
        if (sptr) {
            sptr->RemoveAdvHandle(advHandle);
        }

        observers_->ForEach(
            [advHandle](IBluetoothBleAdvertiseCallback *observer) { observer->OnAutoStopAdvEvent(advHandle); });
    }

    void OnSetAdvDataEvent(int result, uint8_t advHandle,
        bluetooth::SetAdvDataType type = bluetooth::SET_ADV_DATA_BOTH) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("result: %{public}d, advHandle: %{public}d, type: %{public}d", result, advHandle, type);
        observers_->ForEach(
            [this, result, advHandle, type](IBluetoothBleAdvertiseCallback *observer) {
                auto sptr = applications_.lock();
                if (sptr) {
                    int32_t pid = sptr->GetRemotePid(observer->AsObject());
                    if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                        return;
                    }
                }
                observer->OnSetAdvDataEvent(result, advHandle, type);
            });
    }

    void OnChangeAdvResultEvent(int result, uint8_t advHandle) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);

        observers_->ForEach([this, result, advHandle](IBluetoothBleAdvertiseCallback *observer) {
            auto sptr = applications_.lock();
            if (sptr != nullptr) {
                int32_t pid = sptr->GetRemotePid(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    return;
                }
            }
            observer->OnChangeAdvResultEvent(result, advHandle);
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothBleAdvertiseCallback> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothBleAdvertiseCallback> *observers_;
    std::weak_ptr<BluetoothBleAdvertiserApplicationContainer> applications_;
};

struct BluetoothBleAdvertiserServer::impl {
    impl();
    ~impl();

    BleAdvertiserDataImpl ConvertAdvertisingData(const BluetoothBleAdvertiserData &data) const;
    void SetDeviceName(const BluetoothBleAdvertiserData &data, BleAdvertiserDataImpl &outData) const;
    BleAdvertiserSettingsImpl CovertImplSettingDataInner(const std::string callingName,
        const BluetoothBleAdvertiserSettings &settings) const;

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    RemoteObserverList<IBluetoothBleAdvertiseCallback> observers_;
    std::unique_ptr<BleAdvertiserCallback> observerImp_ = nullptr;
    std::shared_ptr<BluetoothBleAdvertiserApplicationContainer> applications = nullptr;
};

class BluetoothBleAdvertiserServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothBleAdvertiserServer::impl *pimpl) : pimpl_(pimpl) {};
    void OnSystemStateChange(const BTSystemState state) override
    {
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        switch (state) {
            case BTSystemState::ON:
                if (bleService != nullptr) {
                    bleService->RegisterBleAdvertiserCallback(*pimpl_->observerImp_.get());
                }
                break;
            default:
                break;
        }
    };

private:
    BluetoothBleAdvertiserServer::impl *pimpl_ = nullptr;
};

BluetoothBleAdvertiserServer::impl::impl()
{
    applications = std::make_shared<BluetoothBleAdvertiserApplicationContainer>();
    applications->Init();
    observerImp_ = std::make_unique<BleAdvertiserCallback>(applications);
}

BluetoothBleAdvertiserServer::impl::~impl()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->DeregisterBleAdvertiserCallback();
    }
}

__attribute__((no_sanitize("cfi")))
BluetoothBleAdvertiserServer::BluetoothBleAdvertiserServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->RegisterBleAdvertiserCallback(*pimpl->observerImp_.get());
    }
}

BluetoothBleAdvertiserServer::~BluetoothBleAdvertiserServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
}

void BluetoothBleAdvertiserServer::impl::SetDeviceName(const BluetoothBleAdvertiserData &data,
    BleAdvertiserDataImpl &outData) const
{
    if (!data.GetIncludeDeviceName() && !data.HasAdvertiseName()) {
        return;
    }

    if (data.HasAdvertiseName()) { // prefer to use advertiseName if given
        outData.SetDeviceName(data.GetAdvertiseName());
        return;
    }
    // includeDeviceName is true
    std::string advName = "";
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (PermissionManager::IsInAdvWhiteList(tokenId)) { // temp solution
        advName = IAdapterManager::GetInstance()->GetTmpAdvName(tokenId);
    } else {
        auto adapterService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
        if (adapterService) {
            advName = adapterService->GetLocalName();
        }
    }
    outData.SetDeviceName(advName);
    return;
}

BleAdvertiserDataImpl BluetoothBleAdvertiserServer::impl::ConvertAdvertisingData(
    const BluetoothBleAdvertiserData &data) const
{
    BleAdvertiserDataImpl outData;

    std::map<uint16_t, std::string> manufacturerData = data.GetManufacturerData();
    for (auto iter = manufacturerData.begin(); iter != manufacturerData.end(); iter++) {
        outData.AddManufacturerData(iter->first, iter->second);
    }
    std::map<Uuid, std::string> serviceData = data.GetServiceData();
    for (auto it = serviceData.begin(); it != serviceData.end(); it++) {
        outData.AddServiceData(it->first, it->second);
    }
    std::vector<Uuid> serviceUuids = data.GetServiceUuids();
    outData.AddServiceUuids(serviceUuids);
    outData.AddData(data.GetPayload());
    SetDeviceName(data, outData);
    if (data.GetIncludeTxPower()) {
        outData.SetTxPowerLevel(0); // hard-coded. The data will be overwritten in the Bluetooth protocol stack.
    }

    return outData;
}

bool IsAtomicPerformanceTestDebug()
{
    if (system::GetIntParameter("ro.bluetooth.advertiser.interval.debug.on", 0) != 0) {
        HILOGI("atomic performanceTest debug mode");
        return true;
    }
    return false;
}

BleAdvertiserSettingsImpl BluetoothBleAdvertiserServer::impl::CovertImplSettingDataInner(const std::string callingName,
    const BluetoothBleAdvertiserSettings &settings) const
{
    constexpr int FAST_ADV_SIZE = 3;
    std::array<std::string, FAST_ADV_SIZE> fastAdvArray = { "softbus_server" };
    BleAdvertiserSettingsImpl outImplSetting{};
    outImplSetting.SetConnectable(settings.IsConnectable());
    int interval = settings.GetInterval();
    // 1. 160 slots means 100ms, and only fastAdvArray is allowed to use high-speed advertiser.
    //    If other hap's adv interval < 160 slots, then Change to 160 slots.
    // 2. if open debug, atomic performance test hap can use high-speed advertiser.
    if (interval < DEFAULT_MIN_ADV_INTERVAL &&
        std::find(fastAdvArray.begin(), fastAdvArray.end(), callingName) == fastAdvArray.end() &&
        !IsAtomicPerformanceTestDebug()) {
        interval = DEFAULT_MIN_ADV_INTERVAL;
    }
    outImplSetting.SetInterval(interval);
    outImplSetting.SetLegacyMode(settings.IsLegacyMode());
    outImplSetting.SetTxPower(settings.GetTxPower());
    outImplSetting.SetOwnAddr(settings.GetOwnAddr());
    outImplSetting.SetOwnAddrType(settings.GetOwnAddrType());
    return outImplSetting;
}

void BluetoothBleAdvertiserServer::AdjustTxPowerForWalletService(const std::string& callingName,
    const uint32_t& tokenId, BleAdvertiserSettingsImpl &settings)
{
    if (!IsWalletService(callingName, tokenId)) {
        return;
    }
    char btSubChipType[propertyValueMax] = {0};
    if (GetParameter(connectivityBtChipType, "", btSubChipType, (propertyValueMax - 1)) == 0) { // 1代表防止读越界
        HILOGE("GetParameter faile");
        return;
    }
    std::string chipType(btSubChipType);
    if (chipType07c != chipType && chipType1112 != chipType) {
        return;
    }
    const int8_t currentTxPower = settings.GetTxPower();
    if (currentTxPower < minTxPower || currentTxPower > maxTxPower) {
        HILOGE("Invalid current power %{public}d", currentTxPower);
        return;
    }
    int8_t newTxPower = settings.GetTxPower() + advTxPowerAdjust;
    settings.SetTxPower(newTxPower);
}

int BluetoothBleAdvertiserServer::StartAdvertising(const BluetoothBleAdvertiserSettings &settings,
    const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse, int32_t advHandle,
    uint16_t duration, bool isRawData)
{
    if (advData.HasAdvertiseName() || scanResponse.HasAdvertiseName()) {
        bool hasAdvNamePerm = PermissionManager::VerifyPermission(MANAGE_BLUETOOTH_ADVERTISER_NAME);
        if (!hasAdvNamePerm) {
            HILOGE("require MANAGE_BLUETOOTH_ADVERTISER_NAME permission to use advertiseName");
            return BT_ERR_PERMISSION_FAILED;
        }
    }

    std::string callingName = PermissionManager::GetCallingName();
    std::string logKey = "(StartAdvertising)callingName:" + callingName;
    HILOGI_ACCUMULATE(logKey, "interval:%d, advHandle:%u", settings.GetInterval(), advHandle);
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        BleAdvertiserSettingsImpl implSettings = pimpl->CovertImplSettingDataInner(callingName, settings);
        BleAdvertiserDataImpl bleAdvertiserData = pimpl->ConvertAdvertisingData(advData);
        BleAdvertiserDataImpl bleScanResponse = pimpl->ConvertAdvertisingData(scanResponse);

        uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
        AdjustTxPowerForWalletService(callingName, tokenId, implSettings);
        bleService->StartAdvertising(implSettings, bleAdvertiserData, bleScanResponse, advHandle, duration);
    }
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserServer::EnableAdvertising(uint8_t advHandle, uint16_t duration)
{
    HILOGI("advHandle: %{public}d, duration: %{public}d", advHandle, duration);
    int pid = IPCSkeleton::GetCallingPid();
    int uid = IPCSkeleton::GetCallingUid();
    if (!pimpl->applications->IsApplicationAdv(pid, uid, advHandle)) {
        return BT_ERR_INVALID_PARAM;
    }
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->EnableAdvertising(advHandle, duration);
    }

    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserServer::DisableAdvertising(uint8_t advHandle)
{
    HILOGI("advHandle: %{public}d", advHandle);
    int pid = IPCSkeleton::GetCallingPid();
    int uid = IPCSkeleton::GetCallingUid();
    if (!pimpl->applications->IsApplicationAdv(pid, uid, advHandle)) {
        return BT_ERR_INVALID_PARAM;
    }
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->DisableAdvertising(advHandle);
    }

    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserServer::StopAdvertising(int32_t advHandle)
{
    int pid = IPCSkeleton::GetCallingPid();
    int uid = IPCSkeleton::GetCallingUid();
    if (!pimpl->applications->IsApplicationAdv(pid, uid, advHandle)) {
        return BT_ERR_INVALID_PARAM;
    }
    std::string logKey = "(StopAdvertising)callingName:" + PermissionManager::GetCallingName();
    HILOGI_ACCUMULATE(logKey, "advHandle:%d", advHandle);
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (PermissionManager::IsInAdvWhiteList(tokenId)) {
        IAdapterManager::GetInstance()->EraseTmpAdvName(tokenId);
    }
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->StopAdvertising(advHandle);
    }
    return BT_NO_ERROR;
}

void BluetoothBleAdvertiserServer::Close(int32_t advHandle)
{
    HILOGI("advHandle: %{public}d", advHandle);

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->Close(advHandle);
    }
    pimpl->applications->RemoveAdvHandle(advHandle);
}

void BluetoothBleAdvertiserServer::RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    int pid = IPCSkeleton::GetCallingPid();
    int uid = IPCSkeleton::GetCallingUid();
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("pid: %{public}d, uid: %{public}d, callingName: %{public}s", pid, uid, callingName.c_str());

    if (callback == nullptr) {
        HILOGE("callback is null");
        return;
    }
    if (pimpl != nullptr) {
        if (pimpl->observers_.Size() > MAX_MAP_SIZE) {
            HILOGE("observers_ too much");
            return;
        }
        pimpl->observers_.Register(callback);
        pimpl->applications->AddRemoteObject(pid, uid, callback->AsObject());
        pimpl->applications->AddTokenId(callback->AsObject(), IPCSkeleton::GetCallingFullTokenID());
        pimpl->applications->SetCallingName(callback->AsObject(), callingName);
    }
}

void BluetoothBleAdvertiserServer::DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    if (callback == nullptr || pimpl == nullptr) {
        HILOGE("callback is null, or pimpl is null");
        return;
    }

    if (pimpl->applications->Contain(callback->AsObject())) {
        HILOGI("Deregister observer");
        pimpl->observers_.Deregister(callback);
        pimpl->applications->RemoveRemoteObject(callback->AsObject());
    }
}

int32_t BluetoothBleAdvertiserServer::GetAdvertiserHandle(int32_t &advHandle,
    const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int status = -1;
    advHandle = bleService->GetAdvertiserHandle(status);
    // When advHandles reaching max, trigger preemption
    if (advHandle == BLE_INVALID_ADVERTISING_HANDLE && status == ADVERTISE_FAILED_TOO_MANY_ADVERTISERS) {
        if (!PermissionManager::IsNativeCaller()) {
            HILOGE("non-native caller advHandle reaching max, callingName: %{public}s",
                PermissionManager::GetCallingName().c_str());
            return BT_ERR_MAX_RESOURCES;
        }
        HILOGW("advHandle reaching max, preemption triggered");
        int preemptibleAdvHandle = pimpl->applications->PreemptAdvHandle();
        if (preemptibleAdvHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            HILOGE("failed to preempt advHandle");
            return BT_ERR_MAX_RESOURCES;
        }
        advHandle = bleService->GetAdvertiserHandle(status);
    }
    if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        if (status == ADVERTISE_FAILED_TOO_MANY_ADVERTISERS) {
            HILOGE("advHandle reaching max after preemption");
            return BT_ERR_MAX_RESOURCES;
        }
        HILOGE("advHandle invalid.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // Get advertiser handle successed, add it to application
    int ret = pimpl->applications->AddAdvHandle(callback->AsObject(), advHandle);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail to add advHandle: %{public}d, unregister advInst", advHandle);
        bleService->UnregisterAdvertisingHandle(advHandle);
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

void BluetoothBleAdvertiserServer::SetAdvertisingData(const BluetoothBleAdvertiserData &advData,
    const BluetoothBleAdvertiserData &scanResponse, int32_t advHandle, bluetooth::SetAdvDataType type)
{
    HILOGI("advHandle: %{public}d, type: %{public}d", advHandle, type);
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService) {
        BleAdvertiserDataImpl bleAdvertiserData = pimpl->ConvertAdvertisingData(advData);
        BleAdvertiserDataImpl bleScanResponse = pimpl->ConvertAdvertisingData(scanResponse);
        switch (type) {
            case bluetooth::SetAdvDataType::SET_ADV_DATA_BOTH:
                bleService->SetAdvertisingData(bleAdvertiserData, bleScanResponse, advHandle);
                break;
            case bluetooth::SetAdvDataType::SET_ADV_DATA_ONLY_ADV:
                bleService->SetAdvOrRspData(bleAdvertiserData, true, advHandle);
                break;
            case bluetooth::SetAdvDataType::SET_ADV_DATA_ONLY_RSP:
                bleService->SetAdvOrRspData(bleScanResponse, false, advHandle);
                break;
            default:
                HILOGW("wrong type");
        }
    }
}

int BluetoothBleAdvertiserServer::ChangeAdvertisingParams(uint8_t advHandle,
    const BluetoothBleAdvertiserSettings &settings)
{
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("callingName(%{public}s), advHandle:%{public}u,minterval=%{public}d", callingName.c_str(), advHandle,
        settings.GetInterval());
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    BleAdvertiserSettingsImpl implSettings = pimpl->CovertImplSettingDataInner(callingName, settings);
    return bleService->ChangeAdvertisingParams(advHandle, implSettings);
}

int BluetoothBleAdvertiserServer::GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType, int8_t &advpower)
{
    HILOGI("GetAdvPowerForRangingBusiness appType: %{public}d", appType);
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("request not support.");
        advpower = INNER_BLE_RANGING_INVALID_ADVPOWER;
        return BT_ERR_INTERNAL_ERROR;
    }
    advpower = bleService->GetAdvPowerForRangingBusiness(appType);
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserServer::BleRestoreRangingAntSwitch(bluetooth::BleAppType appType)
{
    HILOGI("BleRestoreRangingAntSwitch appType: %{public}d", appType);
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return bleService->BleRestoreRangingAntSwitch(appType);
}

namespace {
std::string SetToString(const std::set<AdvHandleRecord> &v)
{
    std::stringstream ss;
    ss << "[";
    for (AdvHandleRecord n : v) {
        ss << n.advHandle << " ";
    }
    ss << "]";
    return ss.str();
}
}  // namespace {}

void BluetoothBleAdvertiserApplicationContainer::AddTokenId(const wptr<IRemoteObject> &remote, uint64_t tokenId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return;
    }
    HILOGD("pid: %{public}d, uid: %{public}d, tokenId: %{public}lu", it->pid, it->uid, tokenId);
    it->tokenId = tokenId;
}

void BluetoothBleAdvertiserApplicationContainer::SetCallingName(const wptr<IRemoteObject> &remote,
    const std::string &name)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return;
    }
    HILOGD("pid: %{public}d, uid: %{public}d, name: %{public}s", it->pid, it->uid, name.c_str());
    it->callingName = name;
}

int BluetoothBleAdvertiserApplicationContainer::AddAdvHandle(const wptr<IRemoteObject> &remote, int handle)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByHandle(handle);
    if (it != container_.end()) {
        HILOGE("advHandle(%{public}d) exist in pid(%{public}d) uid(%{public}d)", handle, it->pid, it->uid);
        return BT_ERR_INTERNAL_ERROR;
    }

    it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Not found remote");
        return BT_ERR_INTERNAL_ERROR;
    }

    it->advHandles.insert(AdvHandleRecord(handle));
    HILOGI("pid: %{public}d, uid: %{public}d, handles: %{public}s",
        it->pid, it->uid, SetToString(it->advHandles).c_str());
    return BT_NO_ERROR;
}

int BluetoothBleAdvertiserApplicationContainer::AddAdvHandle(
    const wptr<IRemoteObject> &remote, const AdvHandleRecord &handleRecord)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByHandle(handleRecord.advHandle);
    if (it != container_.end()) {
        HILOGE("advHandle(%{public}d) exist in pid(%{public}d) uid(%{public}d)",
            handleRecord.advHandle, it->pid, it->uid);
        return BT_ERR_INTERNAL_ERROR;
    }

    it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Not found remote");
        return BT_ERR_INTERNAL_ERROR;
    }

    it->advHandles.insert(handleRecord);
    HILOGI("pid: %{public}d, uid: %{public}d, handles: %{public}s",
        it->pid, it->uid, SetToString(it->advHandles).c_str());
    return BT_NO_ERROR;
}

void BluetoothBleAdvertiserApplicationContainer::RemoveAdvHandle(int handle)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByHandle(handle);
    if (it == container_.end()) {
        HILOGE("Invalid advHandle: %{public}d", handle);
        return;
    }

    it->advHandles.erase(AdvHandleRecord(handle));
    HILOGI("pid: %{public}d, uid: %{public}d, handles: %{public}s",
        it->pid, it->uid, SetToString(it->advHandles).c_str());
}

int BluetoothBleAdvertiserApplicationContainer::PreemptAdvHandle()
{
    // AdvHandles not involved in auditing should be preemted first, after that advHandles involved can be preempted
    std::set<AdvHandleRecord> preemptibleAdvAuditingList = {};
    std::set<AdvHandleRecord> preemptibleAdvList = {};
    {
        std::lock_guard<std::mutex> lock(containerMutex_);
        for (auto it = container_.begin(); it != container_.end(); it++) {
            std::pair<std::string, int> appInfo = std::make_pair(it->callingName, it->uid);
            if (std::find(g_advAuditingWhiteList.begin(), g_advAuditingWhiteList.end(), appInfo) !=
                g_advAuditingWhiteList.end()) {
                preemptibleAdvAuditingList.merge(GetPreemptibleAdvHandles(it->remote));
                continue;
            }
            preemptibleAdvList.merge(GetPreemptibleAdvHandles(it->remote));
        }
    }

    if (preemptibleAdvAuditingList.empty() && preemptibleAdvList.empty()) {
        HILOGW("no advHandle is ready for preemption.");
        return BLE_INVALID_ADVERTISING_HANDLE;
    }
    BtChrDftStatictics::GetInstance()->BleAdvReachingMaxStatisticsEvent();

    std::vector<AdvHandleRecord> sortedPreemptibleAuditingList(
        preemptibleAdvAuditingList.begin(), preemptibleAdvAuditingList.end());
    std::sort(sortedPreemptibleAuditingList.begin(), sortedPreemptibleAuditingList.end(), AdvHandleComparator());
    std::vector<AdvHandleRecord> sortedPreemptibleList(preemptibleAdvList.begin(), preemptibleAdvList.end());
    std::sort(sortedPreemptibleList.begin(), sortedPreemptibleList.end(), AdvHandleComparator());

    auto preemptibleAdvHandle = (sortedPreemptibleList.empty() && !sortedPreemptibleAuditingList.empty()) ?
        sortedPreemptibleAuditingList.front() : sortedPreemptibleList.front();
    HILOGI("preemting advHandle: %{public}d", preemptibleAdvHandle.advHandle);
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("request not support.");
        return BLE_INVALID_ADVERTISING_HANDLE;
    }
    bleService->StopAdvertising(preemptibleAdvHandle.advHandle);

    std::unique_lock<std::mutex> lock(advPreemptionLock);
    this->preemptingAdvHandle = preemptibleAdvHandle.advHandle;
    if (!advPreemptionCV.wait_for(lock, std::chrono::milliseconds(ADV_PREEMPTION_TIMEOUT),
        [this]() { return this->preemptingAdvHandle == BLE_INVALID_ADVERTISING_HANDLE; })) {
        HILOGE("adv preemption timeout.");
        preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
        return BLE_INVALID_ADVERTISING_HANDLE;
    }
    return preemptibleAdvHandle.advHandle;
}

std::set<AdvHandleRecord> BluetoothBleAdvertiserApplicationContainer::GetAdvHandles(
    const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.cend()) {
        HILOGE("Not found remote");
        return {};
    }
    return it->advHandles;
}

std::set<AdvHandleRecord> BluetoothBleAdvertiserApplicationContainer::GetAdvHandles(int pid, int uid)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(pid, uid);
    if (it == container_.cend()) {
        HILOGE("Not found pid uid");
        return {};
    }
    return it->advHandles;
}

std::set<AdvHandleRecord> BluetoothBleAdvertiserApplicationContainer::GetPreemptibleAdvHandles(
    const wptr<IRemoteObject> &remote)
{
    auto application = GetApplicationIter(remote);
    if (application == container_.cend()) {
        HILOGE("Not found remote");
        return {};
    }

    std::set<AdvHandleRecord> preemptibleList = {};
    std::pair<std::string, int> appInfo;
    appInfo.first = application->callingName;
    appInfo.second = application->uid;
    auto it = g_advNumWhiteList.find(appInfo);
    size_t advNumLimit = (it != g_advNumWhiteList.end()) ? it->second : ADV_NUM_LIMIT;

    // Get advHandles exceeded max adv num limit
    if (application->advHandles.size() > advNumLimit) {
        std::vector<AdvHandleRecord> advHandleSorted(application->advHandles.begin(), application->advHandles.end());
        std::sort(advHandleSorted.begin(), advHandleSorted.end(), AdvHandleComparator());
        for (auto it = advHandleSorted.begin();
            it != advHandleSorted.begin() + advHandleSorted.size() - advNumLimit; it++) {
            preemptibleList.insert(*it);
        }
    }

    // Get advHandles exceeded max adv time limit
    TimePoint currentTime = std::chrono::system_clock::now();
    for (auto advHandle : application->advHandles) {
        int64_t holdingTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - advHandle.timestamp).count();
        if (holdingTime > ADV_HOLDING_TIME_LIMIT) {
            preemptibleList.insert(advHandle);
        }
    }

    if (!preemptibleList.empty()) {
        HILOGI("advertsing application: %{public}s, preemptibleList: %{public}s", application->callingName.c_str(),
            SetToString(preemptibleList).c_str());
        int64_t maxHoldingTime = 0;
        int64_t holdingTime;
        for (auto &it : preemptibleList) {
            holdingTime =
                std::chrono::duration_cast<std::chrono::seconds>(currentTime - it.timestamp).count();
            maxHoldingTime = holdingTime > maxHoldingTime ? holdingTime : maxHoldingTime;
        }
        BtChrDftStatictics::GetInstance()->SetBleAdvReachingMaxRecord(application->callingName,
            static_cast<int32_t>(application->advHandles.size()), maxHoldingTime);
    }
    return preemptibleList;
}

bool BluetoothBleAdvertiserApplicationContainer::IsApplicationAdv(int pid, int uid, int handle)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByHandle(handle);
    if (it == container_.end()) {
        HILOGE("can't find advHandle: %{public}d", handle);
        return false;
    }
    bool ret = (it->pid == pid && it->uid == uid);
    if (ret) {
        HILOGI("handle: %{public}d, belong to pid: %{public}d, uid: %{public}d", handle, pid, uid);
    }
    return ret;
}

void BluetoothBleAdvertiserApplicationContainer::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (!Contain(remote)) {
        HILOGE("Invalid remote object");
        return;
    }

    auto handles = GetAdvHandles(remote);
    for (AdvHandleRecord handle : handles) {
        HILOGI("StopAdvertising handle: %{public}d", handle.advHandle);
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        if (bleService) {
            bleService->StopAdvertising(handle.advHandle);
        }
        RemoveAdvHandle(handle.advHandle);
    }
    RemoveRemoteObject(remote);
}
}  // namespace Bluetooth
}  // namespace OHOS