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
#define LOG_TAG "bt_fwk_ble_central_manager"
#endif

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_ble_central_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
struct BleCentralManager::impl {
    impl();
    ~impl();

    void ConvertBleScanSetting(const BleScanSettings &inSettings, BluetoothBleScanSettings &outSetting);
    void ConvertBleScanFilter(const std::vector<BleScanFilter> &filters,
        std::vector<BluetoothBleScanFilter> &bluetoothBleScanFilters);
    void ConvertAdvertiserSetting(const BleAdvertiserSettings &inSettings, BluetoothBleAdvertiserSettings &outSettings);
    void ConvertActiveDeviceInfo(const std::vector<BleActiveDeviceInfo> &inDeviceInfos,
        std::vector<BluetoothActiveDeviceInfo> &outDeviceInfos);
    bool InitScannerId(void);
    int32_t CheckScanParams(const BleScanSettings &settings, const std::vector<BleScanFilter> &filters,
        bool checkFilter);

    class BluetoothBleCentralManagerCallbackImp : public BluetoothBleCentralManagerCallBackStub {
    public:
        explicit BluetoothBleCentralManagerCallbackImp() {};
        ~BluetoothBleCentralManagerCallbackImp() override = default;
        __attribute__((no_sanitize("cfi")))
        void OnScanCallback(const BluetoothBleScanResult &result, uint8_t callbackType) override
        {
            callbacks_.ForEach(
                [callbackType, &result](std::shared_ptr<BleCentralManagerCallback> observer) {
                BluetoothBleScanResult tempResult(result);
                BleScanResult scanResult;
                for (auto &manufacturerData : tempResult.GetManufacturerData()) {
                    scanResult.AddManufacturerData(manufacturerData.first, manufacturerData.second);
                }

                for (auto &serviceUuidData : tempResult.GetServiceUuids()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                    scanResult.AddServiceUuid(uuid);
                }

                for (auto &serviceData : tempResult.GetServiceData()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                    scanResult.AddServiceData(uuid, serviceData.second);
                }

                scanResult.SetAdvertiseFlag(tempResult.GetAdvertiseFlag());
                scanResult.SetRssi(tempResult.GetRssi());
                scanResult.SetConnectable(tempResult.IsConnectable());
                BluetoothRemoteDevice device(tempResult.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                scanResult.SetPeripheralDevice(device);
                scanResult.SetPayload(tempResult.GetPayload());
                scanResult.SetName(tempResult.GetName());
                scanResult.SetEventType(tempResult.GetEventType());
                std::string address = result.GetPeripheralDevice().GetAddress();
                HILOGI("device: %{public}s, len: %{public}d",
                    GetEncryptAddr(address).c_str(), static_cast<int>(scanResult.GetPayload().size()));
                if (callbackType == BLE_SCAN_CALLBACK_TYPE_ALL_MATCH) {
                    observer->OnScanCallback(scanResult);
                } else {
                    observer->OnFoundOrLostCallback(scanResult, callbackType);
                }
            });
        }

        void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) override
        {
            HILOGI("enter");
            callbacks_.ForEach([&results](std::shared_ptr<BleCentralManagerCallback> observer) {
                std::vector<BleScanResult> scanResults;
                for (auto &result : results) {
                    BleScanResult scanResult;
                    for (auto &manufacturerData : result.GetManufacturerData()) {
                        scanResult.AddManufacturerData(manufacturerData.first, manufacturerData.second);
                    }

                    for (auto &serviceData : result.GetServiceData()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                        scanResult.AddServiceData(uuid, serviceData.second);
                    }

                    for (auto &serviceUuidData : result.GetServiceUuids()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                        scanResult.AddServiceUuid(uuid);
                    }

                    scanResult.SetAdvertiseFlag(result.GetAdvertiseFlag());
                    scanResult.SetConnectable(result.IsConnectable());
                    scanResult.SetRssi(result.GetRssi());
                    BluetoothRemoteDevice device(result.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                    scanResult.SetPeripheralDevice(device);
                    scanResult.SetPayload(result.GetPayload());

                    scanResults.push_back(scanResult);
                }

                observer->OnBleBatchScanResultsEvent(scanResults);
            });
        }

        __attribute__((no_sanitize("cfi")))
        void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override
        {
            HILOGD("resultCode: %{public}d, isStartScan: %{public}d", resultCode, isStartScan);
            callbacks_.ForEach(
                [resultCode, isStartScan](std::shared_ptr<BleCentralManagerCallback> observer) {
                    observer->OnStartOrStopScanEvent(resultCode, isStartScan);
            });
        }

        void OnNotifyMsgReportFromLpDevice(const bluetooth::Uuid &uuid, int msgType,
            const std::vector<uint8_t> &value) override
        {
            callbacks_.ForEach(
                [uuid, msgType, value](std::shared_ptr<BleCentralManagerCallback> observer) {
                    UUID btUuid = UUID::ConvertFrom128Bits(uuid.ConvertTo128Bits());
                    observer->OnNotifyMsgReportFromLpDevice(btUuid, msgType, value);
            });
        }
    public:
        BluetoothObserverList<BleCentralManagerCallback> callbacks_;
    private:
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleCentralManagerCallbackImp);
    };
    sptr<BluetoothBleCentralManagerCallbackImp> callbackImp_ = nullptr;

    int32_t scannerId_ = BLE_SCAN_INVALID_ID;  // lock by scannerIdMutex_
    std::mutex scannerIdMutex_ {};
    bool enableRandomAddrMode_ = true;
    int32_t profileRegisterId = 0;
};

BleCentralManager::impl::impl()
{
    callbackImp_ = new BluetoothBleCentralManagerCallbackImp();
    auto bleTurnOnFunc = [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothBleCentralManager> proxy = iface_cast<IBluetoothBleCentralManager>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        std::lock_guard<std::mutex> lock(scannerIdMutex_);
        if (scannerId_ == BLE_SCAN_INVALID_ID) {
            proxy->RegisterBleCentralManagerCallback(scannerId_, enableRandomAddrMode_, callbackImp_);
        }
    };
    auto bluetoothTurnOffFunc = [this]() {
        scannerId_ = BLE_SCAN_INVALID_ID;
    };
    ProfileFunctions profileFunctions = {
        .bluetoothLoadedfunc = nullptr,
        .bleTurnOnFunc = bleTurnOnFunc,
        .bluetoothTurnOffFunc = bluetoothTurnOffFunc,
    };
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(
        BLE_CENTRAL_MANAGER_SERVER, profileFunctions);
}

bool BleCentralManager::impl::InitScannerId(void)
{
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    std::lock_guard<std::mutex> lock(scannerIdMutex_);
    if (scannerId_ != BLE_SCAN_INVALID_ID) {
        HILOGI("scannerId(%{public}d) is already registered", scannerId_);
        return true;
    }

    proxy->RegisterBleCentralManagerCallback(scannerId_, enableRandomAddrMode_, callbackImp_);
    return scannerId_ != BLE_SCAN_INVALID_ID;
}

void BleCentralManager::impl::ConvertBleScanSetting(const BleScanSettings &inSettings,
    BluetoothBleScanSettings &outSetting)
{
    outSetting.SetReportDelay(0);
    outSetting.SetScanMode(inSettings.GetScanMode());
    outSetting.SetLegacy(inSettings.GetLegacy());
    outSetting.SetPhy(inSettings.GetPhy());
    outSetting.SetCallbackType(inSettings.GetCallbackType());
    outSetting.SetSensitivityMode(inSettings.GetSensitivityMode());
    outSetting.SetMatchTrackAdvType(inSettings.GetMatchTrackAdvType());
}

void BleCentralManager::impl::ConvertBleScanFilter(const std::vector<BleScanFilter> &filters,
    std::vector<BluetoothBleScanFilter> &bluetoothBleScanFilters)
{
    for (auto filter : filters) {
        BluetoothBleScanFilter scanFilter;
        scanFilter.SetDeviceId(filter.GetDeviceId());
        scanFilter.SetName(filter.GetName());
        if (filter.HasServiceUuid()) {
            scanFilter.SetServiceUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuid().ToString()));
        }
        if (filter.HasServiceUuidMask()) {
            scanFilter.SetServiceUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuidMask().ToString()));
        }
        if (filter.HasSolicitationUuid()) {
            scanFilter.SetServiceSolicitationUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuid().ToString()));
        }
        if (filter.HasSolicitationUuidMask()) {
            scanFilter.SetServiceSolicitationUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuidMask().ToString()));
        }
        scanFilter.SetServiceData(filter.GetServiceData());
        scanFilter.SetServiceDataMask(filter.GetServiceDataMask());
        scanFilter.SetManufacturerId(filter.GetManufacturerId());
        scanFilter.SetManufactureData(filter.GetManufactureData());
        scanFilter.SetManufactureDataMask(filter.GetManufactureDataMask());
        scanFilter.SetAdvIndReportFlag(filter.GetAdvIndReportFlag());
        scanFilter.SetFilterIndex(filter.GetFilterIndex());
        bluetoothBleScanFilters.push_back(scanFilter);
    }
}

void BleCentralManager::impl::ConvertAdvertiserSetting(const BleAdvertiserSettings &inSettings,
    BluetoothBleAdvertiserSettings &outSettings)
{
    outSettings.SetConnectable(inSettings.IsConnectable());
    outSettings.SetInterval(inSettings.GetInterval());
    outSettings.SetLegacyMode(inSettings.IsLegacyMode());
    outSettings.SetTxPower(inSettings.GetTxPower());
}

void BleCentralManager::impl::ConvertActiveDeviceInfo(const std::vector<BleActiveDeviceInfo> &inDeviceInfos,
    std::vector<BluetoothActiveDeviceInfo> &outDeviceInfos)
{
    for (auto info : inDeviceInfos) {
        BluetoothActiveDeviceInfo deviceInfo;
        deviceInfo.deviceId = info.deviceId;
        deviceInfo.status = info.status;
        deviceInfo.timeOut = info.timeOut;
        outDeviceInfos.push_back(deviceInfo);
    }
}

int32_t BleCentralManager::impl::CheckScanParams(const BleScanSettings &settings,
    const std::vector<BleScanFilter> &filters, bool checkFilter)
{
    uint8_t callbackType = settings.GetCallbackType();
    if (callbackType != BLE_SCAN_CALLBACK_TYPE_ALL_MATCH &&
        callbackType != BLE_SCAN_CALLBACK_TYPE_FIRST_MATCH &&
        callbackType != BLE_SCAN_CALLBACK_TYPE_LOST_MATCH &&
        callbackType != BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH) {
        HILOGE("Illegal callbackType argument %{public}d", callbackType);
        return BT_ERR_INVALID_PARAM;
    }

    if ((callbackType & BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH) != 0 && checkFilter) {
        if (filters.size() == 0) {
            HILOGE("onFound/onLost need non-empty filters callbackType: %{public}d", callbackType);
            return BT_ERR_INVALID_PARAM;
        }
        for (auto filter : filters) {
            BleScanFilter emptyFilter;
            if (filter == emptyFilter) {
                HILOGE("onFound/onLost need non-empty filter callbackType: %{public}d", callbackType);
                return BT_ERR_INVALID_PARAM;
            }
        }
    }

    uint8_t matchTrackAdvType = settings.GetMatchTrackAdvType();
    if (matchTrackAdvType < ONE_MATCH_TRACK_ADV || matchTrackAdvType > MAX_MATCH_TRACK_ADV) {
        HILOGE("Illegal matchTrackAdvType argument %{public}d", matchTrackAdvType);
        return BT_ERR_INVALID_PARAM;
    }

    return BT_NO_ERROR;
}

BleCentralManager::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterBleCentralManagerCallback(scannerId_, callbackImp_);
}

BleCentralManager::BleCentralManager(BleCentralManagerCallback &callback) : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (pimpl == nullptr) {
            HILOGE("failed, no pimpl");
            return;
        }
    }

    HILOGI("successful");
    std::shared_ptr<BleCentralManagerCallback> pointer(&callback, [](BleCentralManagerCallback *) {});
    bool ret = pimpl->callbackImp_->callbacks_.Register(pointer);
    if (ret)
        return;
}

BleCentralManager::BleCentralManager(std::shared_ptr<BleCentralManagerCallback> callback, bool enableRandomAddrMode)
    : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (pimpl == nullptr) {
            HILOGE("failed, no pimpl");
            return;
        }
    }
    HILOGI("successful");
    pimpl->enableRandomAddrMode_ = enableRandomAddrMode;
    pimpl->callbackImp_->callbacks_.Register(callback);
}

BleCentralManager::~BleCentralManager()
{
    HILOGD("~BleCentralManager");
}

int BleCentralManager::StartScan(const BleScanSettings &settings, const std::vector<BleScanFilter> &filters)
{
    if (!IS_BLE_ENABLED()) {
        HILOGD("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    int ret = pimpl->CheckScanParams(settings, filters, true);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID && !pimpl->InitScannerId()) {
        HILOGE("init scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    HILOGD("StartScan with params, scannerId: %{public}d, callbackType: %{public}d, sensitivityMode: %{public}d",
        pimpl->scannerId_, settings.GetCallbackType(), settings.GetSensitivityMode());
    BluetoothBleScanSettings parcelSettings;
    pimpl->ConvertBleScanSetting(settings, parcelSettings);

    std::vector<BluetoothBleScanFilter> parcelFilters;
    pimpl->ConvertBleScanFilter(filters, parcelFilters);

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->StartScan(pimpl->scannerId_, parcelSettings, parcelFilters);
}

int BleCentralManager::StopScan()
{
    if (!IS_BLE_ENABLED()) {
        HILOGD("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID && !pimpl->InitScannerId()) {
        HILOGE("init scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    HILOGD("scannerId_: %{public}d", pimpl->scannerId_);

    int ret = proxy->StopScan(pimpl->scannerId_);
    proxy->RemoveScanFilter(pimpl->scannerId_);
    return ret;
}

int BleCentralManager::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
}

int BleCentralManager::SetScanReportChannelToLpDevice(bool enable)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    // need start scan first
    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID && !pimpl->InitScannerId()) {
        HILOGE("init scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SetScanReportChannelToLpDevice(pimpl->scannerId_, enable);
}

int BleCentralManager::EnableSyncDataToLpDevice()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->EnableSyncDataToLpDevice();
}

int BleCentralManager::DisableSyncDataToLpDevice()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->DisableSyncDataToLpDevice();
}

int BleCentralManager::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SendParamsToLpDevice(dataValue, type);
}

bool BleCentralManager::IsLpDeviceAvailable()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");
    return proxy->IsLpDeviceAvailable();
}

int BleCentralManager::SetLpDeviceParam(const BleLpDeviceParamSet &lpDeviceParamSet)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothLpDeviceParamSet paramSet;
    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) != 0) {
        pimpl->ConvertBleScanSetting(lpDeviceParamSet.scanSettings, paramSet.btScanSettings);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) != 0) {
        pimpl->ConvertBleScanFilter(lpDeviceParamSet.scanFilters, paramSet.btScanFilters);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        pimpl->ConvertAdvertiserSetting(lpDeviceParamSet.advSettings, paramSet.btAdvSettings);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        paramSet.btAdvData.SetPayload(std::string(lpDeviceParamSet.advData.begin(),
            lpDeviceParamSet.advData.end()));
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        paramSet.btRespData.SetPayload(std::string(lpDeviceParamSet.respData.begin(),
            lpDeviceParamSet.respData.end()));
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
        pimpl->ConvertActiveDeviceInfo(lpDeviceParamSet.activeDeviceInfos, paramSet.activeDeviceInfos);
    }
    paramSet.fieldValidFlagBit = lpDeviceParamSet.fieldValidFlagBit;
    paramSet.uuid = bluetooth::Uuid::ConvertFromString(lpDeviceParamSet.uuid.ToString());
    paramSet.advHandle = lpDeviceParamSet.advHandle;
    paramSet.deliveryMode = lpDeviceParamSet.deliveryMode;
    paramSet.duration = lpDeviceParamSet.duration;
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SetLpDeviceParam(paramSet);
}

int BleCentralManager::RemoveLpDeviceParam(const UUID &uuid)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    bluetooth::Uuid btUuid = bluetooth::Uuid::ConvertFromString(uuid.ToString());
    return proxy->RemoveLpDeviceParam(btUuid);
}

int BleCentralManager::ChangeScanParams(const BleScanSettings &settings, const std::vector<BleScanFilter> &filters,
    uint32_t filterAction)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl->scannerId_ != BLE_SCAN_INVALID_ID), BT_ERR_INVALID_PARAM, "scannerId invalid");
    int ret = pimpl->CheckScanParams(settings, filters, filters.size() != 0);
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "param invalid");

    HILOGI("scannerId:%{public}d, callbackType:%{public}d", pimpl->scannerId_, settings.GetCallbackType());
    BluetoothBleScanSettings parcelSetting;
    pimpl->ConvertBleScanSetting(settings, parcelSetting);

    std::vector<BluetoothBleScanFilter> parcelFilters;
    if (filters.size() != 0) {
        pimpl->ConvertBleScanFilter(filters, parcelFilters);
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->ChangeScanParams(pimpl->scannerId_, parcelSetting, parcelFilters, filterAction);
}

BleScanResult::BleScanResult()
{}

BleScanResult::~BleScanResult()
{}

std::vector<UUID> BleScanResult::GetServiceUuids() const
{
    return serviceUuids_;
}

std::map<uint16_t, std::string> BleScanResult::GetManufacturerData() const
{
    return manufacturerSpecificData_;
}

std::map<UUID, std::string> BleScanResult::GetServiceData() const
{
    return serviceData_;
}

BluetoothRemoteDevice BleScanResult::GetPeripheralDevice() const
{
    return peripheralDevice_;
}

int8_t BleScanResult::GetRssi() const
{
    return rssi_;
}

bool BleScanResult::IsConnectable() const
{
    return connectable_;
}

uint8_t BleScanResult::GetAdvertiseFlag() const
{
    return advertiseFlag_;
}

std::vector<uint8_t> BleScanResult::GetPayload() const
{
    return payload_;
}

void BleScanResult::AddManufacturerData(uint16_t manufacturerId, const std::string &data)
{
    manufacturerSpecificData_.insert(std::make_pair(manufacturerId, data));
}

void BleScanResult::AddServiceData(const UUID &uuid, const std::string &data)
{
    serviceData_.insert(std::make_pair(uuid, data));
}

void BleScanResult::AddServiceUuid(const UUID &serviceUuid)
{
    serviceUuids_.push_back(serviceUuid);
}

void BleScanResult::SetPayload(std::string payload)
{
    payload_.assign(payload.begin(), payload.end());
}

void BleScanResult::SetPeripheralDevice(const BluetoothRemoteDevice &device)
{
    peripheralDevice_ = device;
}

void BleScanResult::SetRssi(int8_t rssi)
{
    rssi_ = rssi;
}

void BleScanResult::SetConnectable(bool connectable)
{
    connectable_ = connectable;
}

void BleScanResult::SetAdvertiseFlag(uint8_t flag)
{
    advertiseFlag_ = flag;
}

void BleScanResult::SetName(const std::string &name)
{
    name_ = name;
}
std::string BleScanResult::GetName(void)
{
    return name_;
}

void BleScanResult::SetEventType(uint16_t eventType)
{
    eventType_ = eventType;
}

uint16_t BleScanResult::GetEventType(void) const
{
    return eventType_;
}

BleScanSettings::BleScanSettings()
{}

BleScanSettings::~BleScanSettings()
{}

void BleScanSettings::SetReportDelay(long reportDelayMillis)
{
    reportDelayMillis_ = reportDelayMillis;
}

long BleScanSettings::GetReportDelayMillisValue() const
{
    return reportDelayMillis_;
}

int BleScanSettings::SetScanMode(int scanMode)
{
    if (scanMode < SCAN_MODE_LOW_POWER || scanMode >= SCAN_MODE_INVALID) {
        return RET_BAD_PARAM;
    }

    scanMode_ = scanMode;
    return RET_NO_ERROR;
}

int BleScanSettings::GetScanMode() const
{
    return scanMode_;
}

void BleScanSettings::SetLegacy(bool legacy)
{
    legacy_ = legacy;
}

bool BleScanSettings::GetLegacy() const
{
    return legacy_;
}

void BleScanSettings::SetPhy(int phy)
{
    phy_ = phy;
}

int BleScanSettings::GetPhy() const
{
    return phy_;
}

void BleScanSettings::SetCallbackType(uint8_t callbackType)
{
    callbackType_ = callbackType;
}

uint8_t BleScanSettings::GetCallbackType() const
{
    return callbackType_;
}

void BleScanSettings::SetSensitivityMode(uint8_t sensitivityMode)
{
    sensitivityMode_ = sensitivityMode;
}

uint8_t BleScanSettings::GetSensitivityMode() const
{
    return sensitivityMode_;
}

void BleScanSettings::SetMatchTrackAdvType(uint8_t matchTrackAdvType)
{
    matchTrackAdvType_ = matchTrackAdvType;
}

uint8_t BleScanSettings::GetMatchTrackAdvType() const
{
    return matchTrackAdvType_;
}

BleScanFilter::BleScanFilter()
{}

BleScanFilter::~BleScanFilter()
{}

void BleScanFilter::SetDeviceId(std::string deviceId)
{
    deviceId_ = deviceId;
}

std::string BleScanFilter::GetDeviceId() const
{
    return deviceId_;
}

void BleScanFilter::SetName(std::string name)
{
    name_ = name;
}

std::string BleScanFilter::GetName() const
{
    return name_;
}

void BleScanFilter::SetServiceUuid(const UUID &uuid)
{
    serviceUuid_ = uuid;
    hasServiceUuid_ = true;
}

bool BleScanFilter::HasServiceUuid()
{
    return hasServiceUuid_;
}

UUID BleScanFilter::GetServiceUuid() const
{
    return serviceUuid_;
}

void BleScanFilter::SetServiceUuidMask(const UUID &serviceUuidMask)
{
    serviceUuidMask_ = serviceUuidMask;
    hasServiceUuidMask_ = true;
}

bool BleScanFilter::HasServiceUuidMask()
{
    return hasServiceUuidMask_;
}

UUID BleScanFilter::GetServiceUuidMask() const
{
    return serviceUuidMask_;
}

void BleScanFilter::SetServiceSolicitationUuid(const UUID &serviceSolicitationUuid)
{
    serviceSolicitationUuid_ = serviceSolicitationUuid;
    hasSolicitationUuid_ = true;
}

bool BleScanFilter::HasSolicitationUuid()
{
    return hasSolicitationUuid_;
}

UUID BleScanFilter::GetServiceSolicitationUuid() const
{
    return serviceSolicitationUuid_;
}

void BleScanFilter::SetServiceSolicitationUuidMask(const UUID &serviceSolicitationUuidMask)
{
    serviceSolicitationUuidMask_ = serviceSolicitationUuidMask;
    hasSolicitationUuidMask_ = true;
}

bool BleScanFilter::HasSolicitationUuidMask()
{
    return hasSolicitationUuidMask_;
}

UUID BleScanFilter::GetServiceSolicitationUuidMask() const
{
    return serviceSolicitationUuidMask_;
}

void BleScanFilter::SetServiceData(std::vector<uint8_t> serviceData)

{
    serviceData_ = serviceData;
}

std::vector<uint8_t> BleScanFilter::GetServiceData() const
{
    return serviceData_;
}

void BleScanFilter::SetServiceDataMask(std::vector<uint8_t> serviceDataMask)
{
    serviceDataMask_ = serviceDataMask;
}

std::vector<uint8_t> BleScanFilter::GetServiceDataMask() const
{
    return serviceDataMask_;
}

void BleScanFilter::SetManufacturerId(uint16_t manufacturerId)
{
    manufacturerId_ = manufacturerId;
}

uint16_t BleScanFilter::GetManufacturerId() const
{
    return manufacturerId_;
}

void BleScanFilter::SetManufactureData(std::vector<uint8_t> manufactureData)
{
    manufactureData_ = manufactureData;
}

std::vector<uint8_t> BleScanFilter::GetManufactureData() const
{
    return manufactureData_;
}

void BleScanFilter::SetManufactureDataMask(std::vector<uint8_t> manufactureDataMask)
{
    manufactureDataMask_ = manufactureDataMask;
}

std::vector<uint8_t> BleScanFilter::GetManufactureDataMask() const
{
    return manufactureDataMask_;
}

void BleScanFilter::SetAdvIndReportFlag(bool advIndReprot)
{
    advIndReprot_ = advIndReprot;
}

bool BleScanFilter::GetAdvIndReportFlag() const
{
    return advIndReprot_;
}

void BleScanFilter::SetFilterIndex(uint8_t index)
{
    filterIndex_ = index;
}

uint8_t BleScanFilter::GetFilterIndex() const
{
    return filterIndex_;
}
}  // namespace Bluetooth
}  // namespace OHOS
