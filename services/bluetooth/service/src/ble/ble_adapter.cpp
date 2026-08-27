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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_ble_adapter"
#endif

#include "ble_adapter.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <atomic>

#include "adapter_manager.h"
#include "base_observer_list.h"
#include "ble_advertiser_impl.h"
#include "ble_central_manager_impl.h"
#include "ble_defs.h"
#include "bt_types.h"
#include "btif_config.h"
#include "btif_storage.h"
#include "class_creator.h"
#include "bluetooth_connection_manager.h"
#include "compat.h"
#include "gatt/bluetooth_gatt_interface.h"
#include "common/bluetooth_hw_interface.h"
#include "interface_profile_gatt_client.h"
#include "interface_profile_manager.h"
#include "log.h"
#include "common_util.h"
#include "service_util.h"
#include "securec.h"
#include "bt_chr_dft_exception.h"
#include "thread_util.h"
#include "ble_range_impl.h"
#ifdef FEATURE_MOUSE_FAST_CONNECTION
#include "pc_power_manager.h"
#endif

namespace OHOS {
namespace bluetooth {
struct BleAdapter::impl {
    explicit impl(BleAdapter &bleAdapter);
    impl(const impl &);
    impl &operator=(const impl &);
    ~impl();

    BtRecursiveMutex syncMutex_ {};
    std::unique_ptr<BaseObserverList<IAdapterBleObserver>> observer_ =
        std::make_unique<BaseObserverList<IAdapterBleObserver>>();
    std::unique_ptr<BaseObserverList<IBlePeripheralCallback>> blePeripheralCallback_ =
        std::make_unique<BaseObserverList<IBlePeripheralCallback>>();

    std::atomic_bool btmEnableFlag_ = false;

    std::unique_ptr<BleAdvertiserImpl> bleAdvertiser_ = nullptr;
    std::unique_ptr<BleCentralManagerImpl> bleCentralManager_ = nullptr;
    std::shared_ptr<BleRangeImpl> bleRangeImpl_ = nullptr;
};

BleAdapter::impl::impl(BleAdapter &bleAdapter)
    : bleRangeImpl_(std::make_shared<BleRangeImpl>())
{
    bleRangeImpl_->Init();
}

BleAdapter::impl::~impl()
{}

BleAdapter::BleAdapter() : utility::Context(ADAPTER_NAME_BLE, "5.0"),
    adapterProperties_(AdapterProperties::GetInstance()),
    remoteDeviceProperties_(RemoteDeviceProperties::GetInstance()),
    pimpl(std::make_unique<BleAdapter::impl>(*this))
{
    HILOGI("[BleAdapter] %{public}s:Create", Name().c_str());
}

BleAdapter::~BleAdapter()
{
    HILOGI("[BleAdapter] %{public}s:Destroy", Name().c_str());

    pimpl->bleAdvertiser_ = nullptr;
    pimpl->bleCentralManager_ = nullptr;
    pimpl->bleRangeImpl_ = nullptr;
}

void BleAdapter::Enable()
{
    DoInBleThread([this]() { this->EnableTask(); });
}

namespace {
bool InitStackGattProfile(void)
{
    // Init stack gatt profile.
    const bt_interface_t *btInterface = nullptr;
    int ret = hal_util_load_bt_library(&btInterface);
    if (ret != 0 || btInterface == nullptr) {
        HILOGE("Load bluetooth library failed");
        return false;
    }

    const btgatt_interface_t* gattInterface =
        reinterpret_cast<const btgatt_interface_t*>(btInterface->get_profile_interface(BT_PROFILE_GATT_ID));
    if (gattInterface == nullptr) {
        HILOGE("Failed to get gatt interface handle");
        return false;
    }
    return BluetoothGattInterface::GetInstance()->Initialize(gattInterface);
}
}  // namespace

bool BleAdapter::EnableTask()
{
    bool ret = InitStackGattProfile();
    if (!ret) {
        HILOGI("failed");
        pimpl->btmEnableFlag_ = false;
    } else {
        HILOGI("success");
        pimpl->btmEnableFlag_ = true;
    }

    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface()) {
        BluetoothHwInterface::GetInstance()->GetBtHwInterface()->resetSensorhub(true);
    }

    if (pimpl->bleCentralManager_) {
        pimpl->bleCentralManager_->Reset();
    }

    GetContext()->OnEnable(ADAPTER_NAME_BLE, ret);
    return ret;
}

void BleAdapter::Disable()
{
    HILOGI("[BleAdapter] %{public}s", Name().c_str());

    DoInBleThread([this]() { this->DisableTask(); });
}

void BleAdapter::PostEnable()
{
    if (!pimpl->btmEnableFlag_.load()) {
        return;
    }
}

bool BleAdapter::DisableTask()
{
    if (!pimpl->btmEnableFlag_.load()) {
        GetContext()->OnDisable(ADAPTER_NAME_BLE, pimpl->btmEnableFlag_.load());
        return false;
    }

    if (pimpl->bleAdvertiser_) {
        pimpl->bleAdvertiser_->StopAdvertisingAll();
    }

    if (pimpl->bleCentralManager_) {
        pimpl->bleCentralManager_->StopScanWithDisabling();
    }
    remoteDeviceProperties_->FreeMemory();

    BluetoothConnectionManager::GetInstance()->ClearBleConnect();
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface()) {
        BluetoothHwInterface::GetInstance()->GetBtHwInterface()->resetSensorhub(false);
    }

    GetContext()->OnDisable(ADAPTER_NAME_BLE, true);
    return true;
}

std::vector<RawAddress> BleAdapter::GetConnectedDevices() const
{
    std::vector<RawAddress> pairedList;
    return pairedList;
}

utility::Context *BleAdapter::GetContext()
{
    return this;
}

int BleAdapter::GetBleMaxAdvertisingDataLength() const
{
    return AdapterProperties::GetInstance()->GetBleMaxAdvertisingDataLength();
}

int BleAdapter::GetIoCapability() const
{
    return adapterProperties_->GetIoCapability();
}

bool BleAdapter::SetIoCapability(int ioCapability) const
{
    return adapterProperties_->SetIoCapability(ioCapability);
}

bool BleAdapter::IsBleEnabled() const
{
    int status = AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE);
    return (status == BTStateID::STATE_TURN_ON);
}

void BleAdapter::RegisterBleAdvertiserCallback(IBleAdvertiserCallback &callback)
{
    if (pimpl->bleAdvertiser_ == nullptr) {
        pimpl->bleAdvertiser_ = std::make_unique<BleAdvertiserImpl>(callback, *this);
    }
}

void BleAdapter::DeregisterBleAdvertiserCallback() const
{
}

void BleAdapter::RegisterBleCentralManagerCallback(IBleCentralManagerCallback &callback)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        pimpl->bleCentralManager_ = std::make_unique<BleCentralManagerImpl>(callback);
    }
}

void BleAdapter::DeregisterBleCentralManagerCallback() const
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->DeregisterCallbackToGap();
    }
}

void BleAdapter::RegisterBlePeripheralCallback(IBlePeripheralCallback &callback) const
{
    if (pimpl->blePeripheralCallback_ != nullptr) {
        pimpl->blePeripheralCallback_->Register(callback);
    }
}

void BleAdapter::DeregisterBlePeripheralCallback(IBlePeripheralCallback &callback) const
{
    if (pimpl != nullptr && pimpl->blePeripheralCallback_ != nullptr) {
        pimpl->blePeripheralCallback_->Deregister(callback);
    }
}

bool BleAdapter::RegisterBleAdapterObserver(IAdapterBleObserver &observer) const
{
    if (pimpl->observer_ != nullptr) {
        pimpl->observer_->Register(observer);
    }
    return true;
}

bool BleAdapter::DeregisterBleAdapterObserver(IAdapterBleObserver &observer) const
{
    if (pimpl != nullptr && pimpl->observer_ != nullptr) {
        pimpl->observer_->Deregister(observer);
    }
    return true;
}

void BleAdapter::OnStartAdvertisingEvt() const
{
    if (pimpl->observer_ != nullptr) {
        int status = BLE_ADV_STATE_ADVERTISING;
        pimpl->observer_->ForEach(
            [status](IAdapterBleObserver &observer) { observer.OnAdvertisingStateChanged(status); });
    }
}

void BleAdapter::OnStopAdvertisingEvt() const
{
    if (pimpl->observer_ != nullptr) {
        int status = BLE_ADV_STATE_IDLE;
        pimpl->observer_->ForEach(  // 每停一个 handle 广播都会上报该事件，底层有多个 handle，此处回调是什么逻辑？
            [status](IAdapterBleObserver &observer) { observer.OnAdvertisingStateChanged(status); });
    }
}

int BleAdapter::GetPeerDeviceAddrType(const RawAddress &device) const
{
    int ret = BLE_ADDR_TYPE_UNKNOWN;

    auto remoteAddr = ServiceUtil::AddrToStack(device);
    uint8_t remoteAddrType;
    if (btif_storage_get_remote_addr_type(&remoteAddr, &remoteAddrType) == BT_STATUS_SUCCESS) {
        ret = ServiceUtil::AddrTypeFromStack(remoteAddrType);
    }

    return ret;
}

bool BleAdapter::ReadRemoteRssiValue(const RawAddress &device) const
{
    return true;
}

uint8_t BleAdapter::GetAdvertiserHandle(int &status) const
{
    if (pimpl->bleAdvertiser_ != nullptr) {
        return pimpl->bleAdvertiser_->CreateAdvertiserSetHandle(status);
    }
    return BLE_INVALID_ADVERTISING_HANDLE;
}

void BleAdapter::UnregisterAdvertisingHandle(int advHandle) const
{
    if (pimpl->bleAdvertiser_ == nullptr) {
        HILOGE("bleAdvertiser_ is nullptr");
        return;
    }
    pimpl->bleAdvertiser_->UnregisterAdvertisingHandle(advHandle);
}

void BleAdapter::StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
    const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle, uint16_t duration) const
{
    if (pimpl->bleAdvertiser_ == nullptr) {
        HILOGE("bleAdvertiser_ is nullptr");
        return;
    }
    DoInBleThread([this, settings, advData, scanResponse, advHandle, duration]() {
            pimpl->bleAdvertiser_->StartAdvertising(settings, advData, scanResponse, advHandle, duration);
        });
}

void BleAdapter::SetAdvertisingData(const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanResponse,
    uint8_t advHandle) const
{
    if (pimpl->bleAdvertiser_) {
        pimpl->bleAdvertiser_->SetAdvertisingData(advData, scanResponse, advHandle);
    }
}

void BleAdapter::SetAdvOrRspData(const BleAdvertiserDataImpl &data,
    bool isAdv, uint8_t advHandle) const
{
    if (pimpl->bleAdvertiser_) {
        pimpl->bleAdvertiser_->SetAdvOrRspData(data, isAdv, advHandle);
    }
}

void BleAdapter::EnableAdvertising(uint8_t advHandle, uint16_t duration) const
{
    if (pimpl->bleAdvertiser_ != nullptr) {
        pimpl->bleAdvertiser_->EnableAdvertising(advHandle, duration);
    }
}

void BleAdapter::DisableAdvertising(uint8_t advHandle) const
{
    if (pimpl->bleAdvertiser_ != nullptr) {
        pimpl->bleAdvertiser_->DisableAdvertising(advHandle);
    }
}

void BleAdapter::StopAdvertising(uint8_t advHandle) const
{
    if (pimpl->bleAdvertiser_) {
        if (pimpl->bleAdvertiser_->CheckBurstAdvertise(advHandle)) {
            //all burst advertise param set default value
            pimpl->bleCentralManager_->SetLpDeviceAdvParam(0, 0, 0, 0, advHandle);
        }
    }
    if (pimpl->bleAdvertiser_ != nullptr) {
        DoInBleThread(std::bind(
            [this](uint8_t advHandle) {
                pimpl->bleAdvertiser_->StopAdvertising(advHandle);
            },
            advHandle));
    }
}

void BleAdapter::Close(uint8_t advHandle) const
{
    if (pimpl->bleAdvertiser_ != nullptr) {
        pimpl->bleAdvertiser_->Close(advHandle);
    }
}

void BleAdapter::StartScan(const BleScanSettingsImpl &setting) const
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->StartScan(setting);
#ifdef FEATURE_MOUSE_FAST_CONNECTION
        PowerMgrHelper::GetInstance()->SetBleSettings(setting);
#endif
    }
}

void BleAdapter::BatchscanConfigStorage(int clientIf, int batchScanFullMax, int batchScanTruncMax,
    int batchScanNotifyThreshold)
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->BatchscanConfigStorage(clientIf, batchScanFullMax, batchScanTruncMax,
            batchScanNotifyThreshold);
    }
}

void BleAdapter::BatchscanEnable(int scanMode, int scanInterval, int scanWindow, int addrType, int discardRule)
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->BatchscanEnable(scanMode, scanInterval, scanWindow, addrType, discardRule);
    }
}

void BleAdapter::BatchscanDisable()
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->BatchscanDisable();
    }
}

void BleAdapter::BatchscanReadReports(int clientIf, int scanMode)
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->BatchscanReadReports(clientIf, scanMode);
    }
}

void BleAdapter::StopScan() const
{
    if (pimpl->bleCentralManager_ != nullptr) {
        pimpl->bleCentralManager_->StopScan();
    }
}

int BleAdapter::ConfigScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters)
{
    int ret = 0;
    std::lock_guard<BtRecursiveMutex> lk(pimpl->syncMutex_);
    if (pimpl->bleCentralManager_) {
        ret = pimpl->bleCentralManager_->ConfigScanFilter(scannerId, settings, filters);
    }
    return ret;
}

void BleAdapter::RemoveScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings, uint16_t filtersSize)
{
    HILOGD("scannerId: %{public}d", scannerId);
    std::lock_guard<BtRecursiveMutex> lk(pimpl->syncMutex_);
    if (pimpl->bleCentralManager_) {
        pimpl->bleCentralManager_->RemoveScanFilter(scannerId, settings, filtersSize);
    }
}

int32_t BleAdapter::AllocScannerId()
{
    if (IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE) != BTStateID::STATE_TURN_ON) {
        HILOGE("ble state is not turn on, can't not alloc scannerId");
        return BLE_SCAN_INVALID_ID;
    }
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return BLE_SCAN_INVALID_ID;
    }
    std::lock_guard<BtRecursiveMutex> lk(pimpl->syncMutex_);
    return pimpl->bleCentralManager_->AllocScannerId();
}

void BleAdapter::RemoveScannerId(int32_t scannerId)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    std::lock_guard<BtRecursiveMutex> lk(pimpl->syncMutex_);
    return pimpl->bleCentralManager_->RemoveScannerId(scannerId);
}

int BleAdapter::GetAdvertisingStatus() const
{
    if (pimpl->bleAdvertiser_ != nullptr) {
        if (pimpl->bleAdvertiser_->GetAdvertisingStatus() == ADVERTISE_FAILED_ALREADY_STARTED) {
            return BLE_ADV_STATE_ADVERTISING;
        } else {
            return BLE_ADV_STATE_IDLE;
        }
    }
    return BLE_ADV_STATE_IDLE;
}

bool BleAdapter::IsLlPrivacySupported() const
{
    return false;
}


void BleAdapter::AddCharacteristicValue(uint8_t adtype, const std::string &data) const
{
}

void BleAdapter::SendPairStatusChanged(const BTTransport transport, const RawAddress &device, int status) const
{
    HILOGI("transport: %{public}d, address: %{public}s, status: %{public}d",
        transport, GetEncryptAddr(device.GetAddress()).c_str(), status);
    pimpl->blePeripheralCallback_->ForEach([transport, device, status](IBlePeripheralCallback &observer) {
        observer.OnPairStatusChanged(transport, device, status, PAIR_COMMON_BOND_CAUSE, BOND_MSG_NO_ERROR);
    });
}

void BleAdapter::SendAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) const
{
    HILOG_COMM_INFO("SendAclStateChanged: %{public}s, state: %{public}d, reason: %{public}u",
        GetEncryptAddr(device.GetAddress()).c_str(), state, reason);
    pimpl->blePeripheralCallback_->ForEach([device, state, reason, pairStatus](IBlePeripheralCallback &observer) {
        observer.OnAclStateChanged(device, state, reason, pairStatus);
    });
}

void BleAdapter::SendDiscoveryResult(
    const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) const
{
    HILOGD("address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    pimpl->observer_->ForEach([device, rssi, deviceName, deviceClass](IAdapterBleObserver &observer) {
        observer.OnDiscoveryResult(device, rssi, deviceName, deviceClass);
    });
}

void BleAdapter::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    if (pimpl->bleAdvertiser_) {
        pimpl->bleAdvertiser_->SaveBurstAdvertisingParam(window, interval, advHandle);
    }
    pimpl->bleCentralManager_->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
}

void BleAdapter::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    std::lock_guard<BtRecursiveMutex> lk(pimpl->syncMutex_);
    pimpl->bleCentralManager_->SetScanReportChannelToLpDevice(scannerId, enable);
}

void BleAdapter::EnableSyncDataToLpDevice()
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    pimpl->bleCentralManager_->EnableSyncDataToLpDevice();
}

void BleAdapter::DisableSyncDataToLpDevice()
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    pimpl->bleCentralManager_->DisableSyncDataToLpDevice();
}

void BleAdapter::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    pimpl->bleCentralManager_->SendParamsToLpDevice(dataValue, type);
}

bool BleAdapter::IsLpDeviceAvailable()
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return false;
    }
    return pimpl->bleCentralManager_->IsLpDeviceAvailable();
}

void BleAdapter::SetLpDeviceParam(const FilterIdxInfo &filterIdx, const BleLpDeviceParamSetImpl &paramImpl)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    pimpl->bleCentralManager_->SetLpDeviceParam(filterIdx, paramImpl);
}

void BleAdapter::RemoveLpDeviceParam(const FilterIdxInfo &filterIdx)
{
    if (pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return;
    }
    pimpl->bleCentralManager_->RemoveLpDeviceParam(filterIdx);
}

int BleAdapter::ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettingsImpl &settings)
{
    if (pimpl == nullptr || pimpl->bleAdvertiser_ == nullptr) {
        HILOGE("bleAdvertiser_ is nullptr");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    DoInBleThread(
        [this, advHandle, settings]() { pimpl->bleAdvertiser_->ChangeAdvertisingParams(advHandle, settings); });
    return Bluetooth::BT_NO_ERROR;
}

int BleAdapter::UpdateScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters, uint32_t filterAction)
{
    if (pimpl == nullptr || pimpl->bleCentralManager_ == nullptr) {
        HILOGE("bleCentralManager is null");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->bleCentralManager_->UpdateScanFilter(scannerId, settings, filters, filterAction);
}

int BleAdapter::GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType)
{
    if (pimpl == nullptr || pimpl->bleRangeImpl_ == nullptr) {
        HILOGE("bleRangeImpl_ is nullptr");
        return INNER_BLE_RANGING_INVALID_ADVPOWER;
    }
    return pimpl->bleRangeImpl_->GetAdvPowerForRangingBusiness(appType);
}

int BleAdapter::BleRestoreRangingAntSwitch(bluetooth::BleAppType appType)
{
    if (pimpl == nullptr || pimpl->bleRangeImpl_ == nullptr) {
        HILOGE("bleRangeImpl_ is nullptr");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->bleRangeImpl_->BleRestoreRangingAntSwitch(appType);
}

void BleAdapter::SendBleScanMsg(bool isStarted)
{
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("Failed to open the Bluetooth module");
        return;
    }
    btInterface->send_ble_scan_msg(isStarted);
}

REGISTER_CLASS_CREATOR(BleAdapter);
}  // namespace bluetooth
}  // namespace OHOS
