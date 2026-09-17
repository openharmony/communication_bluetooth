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
#define LOG_TAG "bt_server_ble_central_manager"
#endif

#include "bluetooth_ble_central_manager_server.h"

#include "bt_chr_transaction_manager.h"
#include "ble_defs.h"
#include "ble_scanner_state_machine.h"
#include "ble_service_data.h"
#include "bluetooth_ble_scanner_application_container.h"
#include "bluetooth_ble_filter_matcher.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_hci_process_data.h"
#include "bt_chr_dft_exception.h"
#include "hisysevent.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_manager.h"
#include "interface_profile_resource_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "hitrace_meter.h"
#include "log_utils.h"
#include "parameters.h"
#include "wifi_device.h"
#include "thread_util.h"
#ifdef CONTEXTHUB_BLE_V3
#include "bluetooth_sensorhub_collaboration.h"
#endif

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
constexpr size_t BLE_CENTRAL_MANAGER_SCAN_FILTER_MAX_SIZE = 0x100;
constexpr uint32_t BLE_SCAN_UPDATE_FILTER_ADD = 1;
constexpr uint32_t BLE_SCAN_UPDATE_FILTER_DELETE = 2;
constexpr uint32_t BLE_SCAN_UPDATE_FILTER_MODIFY = 3;
constexpr uint32_t BATCH_SCAN_FAST_REPORT_TIME = 1000; // 毫秒

struct BluetoothBleCentralManagerServer::impl {
    impl();
    ~impl();
    BleScanSettingsImpl ConvertToBleScanSettingsImpl(const BluetoothBleScanSettings &settings);
    std::vector<BleScanFilterImpl> ConvertToBleScanFilterImpl(
        const std::vector<BluetoothBleScanFilter> &filters, bool useRealAddr);
    BleAdvertiserSettingsImpl ConvertToBleAdvertiserSettingsImpl(const BluetoothBleAdvertiserSettings &settings);
    BleAdvertiserDataImpl ConvertToBleAdvertiserDataImpl(const BluetoothBleAdvertiserData &data);
    std::vector<BleActiveDeviceInfoImpl> ConvertToBleActiveDeviceInfoImpl(
        const std::vector<BluetoothActiveDeviceInfo> &infos);
    void ConvertFilterDeviceId(const BluetoothBleScanFilter &filter, BleScanFilterImpl &filterImpl,
        bool isUseRealAddrFlag);
    void GetRandomAddr(const BlePeripheralDevice &device, RawAddress &randomAddr);
    void GetBatchScanRandAddr(std::vector<BluetoothBleScanResult> &results, std::vector<RawAddress> &randomAddrs);
    void GetOptimalParam(uint16_t &optimalScanInterval, uint16_t &optimalScanWindow,
        std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue);
    void EnableBatchScanNative(int scannerId, std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue);
    void EnableBatchScanTimer(std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue);
    bool ResetBatchScan(int32_t pid, int32_t uid, int32_t scannerId);

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    /// adapter state observer
    class AdapterStateObserver;
    std::unique_ptr<AdapterStateObserver> adapterStateObserver = nullptr;

#ifdef CONTEXTHUB_BLE_V3
    std::unique_ptr<SensorhubCollaboration> shCollaboration_ = nullptr;
#endif

    RemoteObserverList<IBluetoothBleCentralManagerCallback> observers_;
    class BleCentralManagerCallback;
    std::unique_ptr<BleCentralManagerCallback> observerImp_ = std::make_unique<BleCentralManagerCallback>(this);

    std::shared_ptr<BleScannerStateMachine> scannerSM_ = nullptr;
    std::shared_ptr<BluetoothBleScannerApplicationContainer> applications = nullptr;
    std::unique_ptr<utility::Timer> batchScanTimer_ = nullptr;
    // 开启batchscan后的第一秒向芯片读取一次扫描结果上报给应用，以满足部分应用的即时性诉求
    std::unique_ptr<utility::Timer> batchScanTimerFastReport_ = nullptr;
};

namespace {
int ConvertToScannerStateMachineEvent(int resultCode, bool isStartScanEvent)
{
    int event = BleScannerStateMachine::INVALID_EVENT;
    bool isSuccess = (resultCode == 0);
    if (isStartScanEvent) {
        event = isSuccess ? BleScannerStateMachine::START_SUCCESS_EVENT : BleScannerStateMachine::START_FAILED_EVENT;
    } else {
        event = isSuccess ? BleScannerStateMachine::STOP_SUCCESS_EVENT : BleScannerStateMachine::STOP_FAILED_EVENT;
    }
    return event;
}
}  // namespace {}

static void VirtualizeScanResultAddr(BluetoothBleScanResult &bleScanResult, int32_t addrTypeInMatchedFilter,
    bool isUseRealAddrFlag, RawAddress randomAddr)
{
    bool isVirtualAddrType = (addrTypeInMatchedFilter == AddressType::VIRTUAL_ADDRESS);
    bool isUnsetAddressType = (addrTypeInMatchedFilter == AddressType::UNSET_ADDRESS);
    if (isVirtualAddrType || (isUnsetAddressType && !isUseRealAddrFlag)) {
        bleScanResult.SetPeripheralDevice(randomAddr);
        bleScanResult.SetAddressType(AddressType::VIRTUAL_ADDRESS);
        return;
    }
    bleScanResult.SetAddressType(AddressType::REAL_ADDRESS);
    return;
}

class BluetoothBleCentralManagerServer::impl::BleCentralManagerCallback : public IBleCentralManagerCallback {
public:
    explicit BleCentralManagerCallback(BluetoothBleCentralManagerServer::impl *pimpl) : pimpl_(pimpl) {};
    ~BleCentralManagerCallback() override = default;

    void OnScanCallback(const BleScanResultImpl &result) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        RawAddress advAddr = result.GetPeripheralDevice().GetRawAddress();
        HILOGD("OnScanCallback: %{public}s", GetEncryptAddr(advAddr.GetAddress()).c_str());
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(advAddr, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(advAddr);
        }
        observers_->ForEach([this, &result, &advAddr, &randomAddr](IBluetoothBleCentralManagerCallback *observer) {
            if (!pimpl_->applications) {
                HILOGE("OnScanCallback: applications is null");
                return;
            }
            uint64_t tokenId = pimpl_->applications->GetTokenId(observer->AsObject());
            int sdkVersion = pimpl_->applications->GetSdkVersion(observer->AsObject());
            int pid = pimpl_->applications->GetRemotePid(observer->AsObject());
            int32_t uid = pimpl_->applications->GetRemoteUid(observer->AsObject());
            bool isUseRealAddrFlag = pimpl_->applications->GetUseRealAddrFlag(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid, WakeType::BLE_SCAN_WAKE_TYPE)) {
                HILOGD("OnScanCallback: pid:%{public}d uid:%{public}d is proxy uid, not callback.", pid, uid);
                return;
            }
            if (PermissionManager::IsNativeCaller(tokenId) || sdkVersion >= API_VERSION_10) {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            } else {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(USE_BLUETOOTH, tokenId),
                    "[PERMISSION] check USE_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            }

            BluetoothBleScanResult bleScanResult(result);
            bluetooth::BleScanFilterImpl matchedFilter;
            if (!isMatchFilters(observer, bleScanResult, matchedFilter)) {
                return;
            }
            VirtualizeScanResultAddr(bleScanResult, matchedFilter.GetAddressType(), isUseRealAddrFlag, randomAddr);
            HILOGI_TIME_LIMIT(advAddr.GetAddress() + std::to_string(pid),
                "OnScanCallback: pid:%{public}d uid:%{public}d address:%{public}s passed filter.",
                pid, uid, GetEncryptAddr(advAddr.GetAddress()).c_str());
            observer->OnScanCallback(bleScanResult);
        });
    }

    bool isMatchFilters(IBluetoothBleCentralManagerCallback *observer,
         const BluetoothBleScanResult &bleScanResult, bluetooth::BleScanFilterImpl &matchedFilter)
    {
        int32_t scannerId = pimpl_->applications->GetScannerId(observer->AsObject());
        HILOGD("OnScanCallback start get bleScanFilters scanId:%{public}d", scannerId);
        return pimpl_->applications->IsBleScanFilterMatched(scannerId, bleScanResult, matchedFilter);
    }

    void OnFoundOrLostCallback(uint8_t scannerId, uint8_t advertiserState, const BleScanResultImpl &result) override
    {
        if (!pimpl_->applications) {
            HILOGE("applications is null");
            return;
        }
        sptr<IRemoteObject> remote = pimpl_->applications->GetRemote(scannerId);
        if (remote == nullptr) {
            HILOGE("remote observer is null");
            return;
        }
        sptr<IBluetoothBleCentralManagerCallback> observer = iface_cast<IBluetoothBleCentralManagerCallback>(remote);
        if (!observer) {
            HILOGE("proxy remote observer is null");
            return;
        }

        BleScanSettingsImpl settings = pimpl_->applications->GetScanSetting(scannerId);
        uint8_t callbackType = settings.GetCallbackType();
        bool isUseRealAddrFlag = pimpl_->applications->GetUseRealAddrFlag(remote);
        HILOGI("Address: %{public}s, scanId: %{public}d, advState: %{public}d, cbType: %{public}d, flag: %{public}d",
            GetEncryptAddr(result.GetPeripheralDevice().GetRawAddress().GetAddress()).c_str(),
            scannerId, advertiserState, callbackType, isUseRealAddrFlag);
        BluetoothBleScanResult bleScanResult(result);
        bluetooth::BleScanFilterImpl matchedFilter;
        if (!isMatchFilters(observer, bleScanResult, matchedFilter)) {
            return;
        }
        RawAddress randomAddr;
        pimpl_->GetRandomAddr(result.GetPeripheralDevice(), randomAddr);
        VirtualizeScanResultAddr(bleScanResult, matchedFilter.GetAddressType(), isUseRealAddrFlag, randomAddr);
        if (advertiserState == ADV_STATE_ON_FOUND && (callbackType & BLE_SCAN_CALLBACK_TYPE_FIRST_MATCH) != 0) {
            observer->OnScanCallback(bleScanResult, BLE_SCAN_CALLBACK_TYPE_FIRST_MATCH);
        } else if (advertiserState == ADV_STATE_ON_LOST && (callbackType & BLE_SCAN_CALLBACK_TYPE_LOST_MATCH) != 0) {
            observer->OnScanCallback(bleScanResult, BLE_SCAN_CALLBACK_TYPE_LOST_MATCH);
        } else {
            HILOGW("No report onFound/onLost scanId: %{public}d, advState: %{public}d, callbackType: %{public}d",
                scannerId, advertiserState, callbackType);
        }
    }

    void OnBleBatchScanResultsEvent(int clientIf, std::vector<BluetoothBleScanResult> &results) override
    {
        if (!pimpl_->applications) {
            HILOGE("OnBleBatchScanResultsEvent: applications is null");
            return;
        }
        std::vector<RawAddress> randomAddrs;
        pimpl_->GetBatchScanRandAddr(results, randomAddrs);

        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, &results, &randomAddrs](IBluetoothBleCentralManagerCallback *observer) {
            int pid = pimpl_->applications->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }

            bool isUseRealAddrFlag = pimpl_->applications->GetUseRealAddrFlag(observer->AsObject());
            std::vector<BluetoothBleScanResult> bleScanResults = {};
            for (size_t i = 0; i < results.size(); ++i) {
                // the size of results and randomAddrs was ensured to be equal when GetRandomAddrVector
                BluetoothBleScanResult bleScanResult(results[i]);
                bluetooth::BleScanFilterImpl matchedFilter;
                if (!isMatchFilters(observer, bleScanResult, matchedFilter)) {
                    continue;
                }
                VirtualizeScanResultAddr(bleScanResult, matchedFilter.GetAddressType(), isUseRealAddrFlag,
                    randomAddrs[i]);
                bleScanResults.push_back(bleScanResult);
            }
            if (!bleScanResults.empty()) {
                // only report when there are some results
                observer->OnBleBatchScanResultsEvent(bleScanResults);
            }
        });
        // make sure all data is read out
        if (!results.empty()) {
            int scanMode = BATCH_SCAN_FORMAT_FULL; // full mode
            auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
            if (!bleService) {
                HILOGE("bleService is nullptr.");
                return;
            }
            bleService->BatchscanReadReports(clientIf, scanMode);
        }
    }

    void OnStartOrStopScanEvent(int resultCode, bool isStartScanEvt) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        if (pimpl_ == nullptr) {
            HILOGE("pimpl_ is nullptr");
            return;
        }
        HILOGI_TIME_LIMIT(__func__, "code:%{public}d, isStartScanEvt:%{public}d", resultCode, isStartScanEvt);
        int event = ConvertToScannerStateMachineEvent(resultCode, isStartScanEvt);
        BleScannerMessage msg (event);
        pimpl_->scannerSM_->SendMessage(msg);

#ifdef CONTEXTHUB_BLE_V3
        // query sensorhub capability at first scan started event,
        // because the sensorhub callback of BleCentralManagerImpl is registered in member function StartScan,
        // otherwise the result will not be uploaded in OnNotifyCollaborationFromLpDevice
        if (event == BleScannerStateMachine::START_SUCCESS_EVENT) {
            pimpl_->shCollaboration_->QueryCapability();
        }
#endif
    }

    void OnNotifyMsgReportFromLpDevice(
        const FilterIdxInfo &info, uint8_t msgType, const std::vector<uint8_t> &notifyValue) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        observers_->ForEach([this, info, msgType, notifyValue](IBluetoothBleCentralManagerCallback *observer) {
            if (!pimpl_->applications) {
                HILOGE("applications is null");
                return;
            }

            int32_t pid = pimpl_->applications->GetRemotePid(observer->AsObject());
            int32_t uid = pimpl_->applications->GetRemoteUid(observer->AsObject());
            if (pid != info.pid || uid != info.uid) {
                return;
            }

            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d uid:%{public}d is proxy uid, not callback.",
                    pid, uid);
                return;
            }

            observer->OnNotifyMsgReportFromLpDevice(info.uuid, msgType, notifyValue);
        });
        HILOGI("pid: %{public}d, uid: %{public}d, msgType: %{public}d, dataLen: %{public}lu", info.pid, info.uid,
            msgType, notifyValue.size());
    }

    void SetObserver(RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers)
    {
        observers_ = observers;
    }

    RemoteObserverList<IBluetoothBleCentralManagerCallback>* GetObserver()
    {
        return observers_;
    }

private:
    RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers_ = nullptr;
    BluetoothBleCentralManagerServer::impl *pimpl_ = nullptr;
};

class BluetoothBleCentralManagerServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothBleCentralManagerServer::impl *pimpl) : pimpl_(pimpl){};
    void OnSystemStateChange(const BTSystemState state) override
    {
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        switch (state) {
            case BTSystemState::ON:
                if (bleService != nullptr) {
                    bleService->RegisterBleCentralManagerCallback(*pimpl_->observerImp_.get());
                }
                break;
            default:
                break;
        }
    };

private:
    BluetoothBleCentralManagerServer::impl *pimpl_ = nullptr;
};

class BluetoothBleCentralManagerServer::impl::AdapterStateObserver : public IAdapterStateObserver {
public:
    explicit AdapterStateObserver(std::weak_ptr<BluetoothBleScannerApplicationContainer> applications,
        std::weak_ptr<BleScannerStateMachine> sm, RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers)
        : IAdapterStateObserver(PRIORITY_HIGH), applications_(applications), sm_(sm), observers_(observers) {};
    ~AdapterStateObserver() override = default;

    // The OnStateChange is called high priority than BluetoothHostServer' OnStateChange().
    void OnStateChange(const BTTransport transport, const BTStateID state) override
    {
        auto appSptr = applications_.lock();
        auto scannerSmSptr = sm_.lock();
        if (!appSptr || !scannerSmSptr) {
            HILOGE("BluetoothBleScannerApplicationContainer or BleScannerStateMachine is nullptr");
            return;
        }
        auto clearResources = [scannerSmSptr, appSptr, observers = observers_]() {
            DoInBleThread([scannerSmSptr]() {
                scannerSmSptr->Reset();
            });
            appSptr->Clear();
            if (observers) {
                observers->Clear();
            }
        };
        // Clear resources when ble turn on or off for restoring to the initial state
        if (transport == BTTransport::ADAPTER_BLE && (state == STATE_TURN_OFF || state == STATE_TURNING_ON)) {
            HILOGI("Clear scanner resources when ble state(%{public}d) (1:on, 0:off)", (state == STATE_TURNING_ON));
            clearResources();
        }
    }

    void OnBluetoothStateChanged(int state) override {}

    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override {}

private:
    std::weak_ptr<BluetoothBleScannerApplicationContainer> applications_;
    std::weak_ptr<BleScannerStateMachine> sm_;
    RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers_;
};

BluetoothBleCentralManagerServer::impl::impl()
{
    auto batchscanTimerCb = []() {
        int scanMode = BATCH_SCAN_FORMAT_FULL; // only full mode supported currently
        int fakeScannerId = 0; // ScannerId doesn't matter when reading reports
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        if (!bleService) {
            HILOGE("bleService is nullptr.");
            return;
        }
        bleService->BatchscanReadReports(fakeScannerId, scanMode);
    };

    auto resetBatchScanCb = [this](int32_t pid, int32_t uid, int32_t scannerId) -> bool {
        return ResetBatchScan(pid, uid, scannerId);
    };

    batchScanTimer_ = std::make_unique<utility::Timer>(batchscanTimerCb);
    batchScanTimerFastReport_ = std::make_unique<utility::Timer>(batchscanTimerCb);
    scannerSM_ = std::make_shared<BleScannerStateMachine>();
    applications = std::make_shared<BluetoothBleScannerApplicationContainer>(scannerSM_, resetBatchScanCb);
    applications->Init();
    adapterStateObserver = std::make_unique<AdapterStateObserver>(applications, scannerSM_, &observers_);
    IAdapterManager::GetInstance()->RegisterStateObserver(*adapterStateObserver);

#ifdef CONTEXTHUB_BLE_V3
    shCollaboration_ = std::make_unique<SensorhubCollaboration>([this](const BleScannerMessage &msg) {
        scannerSM_->SendMessage(msg);
    });
#endif
}

BluetoothBleCentralManagerServer::impl::~impl()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->DeregisterBleCentralManagerCallback();
    }
    IAdapterManager::GetInstance()->DeregisterStateObserver(*adapterStateObserver);
}

BleScanSettingsImpl BluetoothBleCentralManagerServer::impl::ConvertToBleScanSettingsImpl(
    const BluetoothBleScanSettings &settings)
{
    BleScanSettingsImpl settingImpl;
    settingImpl.SetScanMode(settings.GetScanMode());
    settingImpl.SetLegacy(settings.GetLegacy());
    settingImpl.SetPhy(settings.GetPhy());
    settingImpl.SetReportDelay(settings.GetReportDelayMillisValue());
    return settingImpl;
}

std::vector<BleScanFilterImpl> BluetoothBleCentralManagerServer::impl::ConvertToBleScanFilterImpl(
    const std::vector<BluetoothBleScanFilter> &filters, bool useRealAddr)
{
    std::vector<BleScanFilterImpl> filterImpls {};
    for (auto filter : filters) {
        BleScanFilterImpl filterImpl;
        ConvertFilterDeviceId(filter, filterImpl, useRealAddr);
        filterImpl.SetAddressType(filter.GetAddressType());
        filterImpl.SetRawAddressType(filter.GetRawAddressType());
        filterImpl.SetIrk(filter.GetIrk());
        filterImpl.SetName(filter.GetName());
        if (filter.HasServiceUuid()) {
            filterImpl.SetServiceUuid(filter.GetServiceUuid());
        }
        if (filter.HasServiceUuidMask()) {
            filterImpl.SetServiceUuidMask(filter.GetServiceUuidMask());
        }
        if (filter.HasSolicitationUuid()) {
            filterImpl.SetServiceSolicitationUuid(filter.GetServiceSolicitationUuid());
        }
        if (filter.HasSolicitationUuidMask()) {
            filterImpl.SetServiceSolicitationUuidMask(filter.GetServiceSolicitationUuidMask());
        }
        if (filter.HasRssiThreshold()) {
            filterImpl.SetRssiThreshold(filter.GetRssiThreshold());
        }
        filterImpl.SetServiceData(filter.GetServiceData());
        filterImpl.SetServiceDataMask(filter.GetServiceDataMask());
        filterImpl.SetManufacturerId(filter.GetManufacturerId());
        filterImpl.SetManufactureData(filter.GetManufactureData());
        filterImpl.SetManufactureDataMask(filter.GetManufactureDataMask());
        filterImpl.SetAdvIndReportFlag(filter.GetAdvIndReportFlag());
        filterImpl.SetFiltIndex(filter.GetFilterIndex());
        filterImpls.push_back(filterImpl);
    }
    HILOGD("filtersize: %{public}lu", filterImpls.size());
    return filterImpls;
}

BleAdvertiserSettingsImpl BluetoothBleCentralManagerServer::impl::ConvertToBleAdvertiserSettingsImpl(
    const BluetoothBleAdvertiserSettings &settings)
{
    BleAdvertiserSettingsImpl settingsImpl;
    settingsImpl.SetConnectable(settings.IsConnectable());
    settingsImpl.SetInterval(settings.GetInterval());
    settingsImpl.SetLegacyMode(settings.IsLegacyMode());
    settingsImpl.SetTxPower(settings.GetTxPower());
    return settingsImpl;
}

BleAdvertiserDataImpl BluetoothBleCentralManagerServer::impl::ConvertToBleAdvertiserDataImpl(
    const BluetoothBleAdvertiserData &data)
{
    BleAdvertiserDataImpl outData;
    if (data.GetIncludeDeviceName()) {
        auto adapterService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
        if (adapterService) {
            outData.SetDeviceName(adapterService->GetLocalName());
        }
    }
    std::map<uint16_t, std::string> manufacturerData = data.GetManufacturerData();
    for (auto iter = manufacturerData.begin(); iter != manufacturerData.end(); iter++) {
        outData.AddManufacturerData(iter->first, iter->second);
    }
    std::vector<Uuid> serviceUuids = data.GetServiceUuids();
    for (auto it = serviceUuids.begin(); it != serviceUuids.end(); it++) {
        outData.AddServiceUuid(*it);
    }
    std::map<Uuid, std::string> serviceData = data.GetServiceData();
    for (auto it = serviceData.begin(); it != serviceData.end(); it++) {
        outData.AddServiceData(it->first, it->second);
    }
    outData.AddData(data.GetPayload());
    return outData;
}

std::vector<BleActiveDeviceInfoImpl> BluetoothBleCentralManagerServer::impl::ConvertToBleActiveDeviceInfoImpl(
    const std::vector<BluetoothActiveDeviceInfo> &infos)
{
    std::vector<BleActiveDeviceInfoImpl> deviceInfoImpls {};
    for (auto info : infos) {
        BleActiveDeviceInfoImpl infoImpl;
        if (info.deviceId.size() != BLE_ADV_DEVICE_ID_LEN) {
            HILOGE("deviceId Len: %{public}lu", info.deviceId.size());
            continue;
        }
        infoImpl.deviceId = info.deviceId;
        infoImpl.status = info.status;
        infoImpl.timeOut = info.timeOut;
        deviceInfoImpls.push_back(infoImpl);
    }
    return deviceInfoImpls;
}

void BluetoothBleCentralManagerServer::impl::ConvertFilterDeviceId(const BluetoothBleScanFilter &filter,
    BleScanFilterImpl &filterImpl, bool isUseRealAddrFlag)
{
    bool realAddressType = filter.GetAddressType() == AddressType::REAL_ADDRESS;
    bool unsetAddressType = filter.GetAddressType() == AddressType::UNSET_ADDRESS;
    if (realAddressType || (unsetAddressType && isUseRealAddrFlag) || filter.GetDeviceId().empty()) {
        filterImpl.SetDeviceId(filter.GetDeviceId());
        return;
    }
    // 应用设置了virtual类型，或者没设类型且没有使用真实地址的权限，则认为使用的是virtual类型
    // 如果过滤器中地址对应的随机地址存在，则获取对应的真实地址设置下去, 否则直接使用配置的地址设下去
    // 实际不支持使用虚拟地址扫描，napi层已做虚拟地址类型拦截；
    // 因为此处不方便返回错误码给StartScan()，所以还是发下去了，下发该地址之后BTC也扫不到目标；如果设空地址下去反而能扫到设备
    RawAddress realAddr;
    int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(RawAddress(filter.GetDeviceId()), realAddr);
    if (ret == OHOS::bluetooth::RET_SUCCESS) {
        HILOGI("convert rawAddress.");
        filterImpl.SetDeviceId(realAddr.GetAddress());
        return;
    }
    HILOGE("Virtual address not exists, address: %{public}s, use as real address.",
        GetEncryptAddr(filter.GetDeviceId()).c_str());
    filterImpl.SetDeviceId(filter.GetDeviceId());
    return;
}

void BluetoothBleCentralManagerServer::impl::GetRandomAddr(const BlePeripheralDevice &device, RawAddress &randomAddr)
{
    int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device.GetRawAddress(), randomAddr);
    if (ret == OHOS::bluetooth::RET_NO_EXIST) {
        randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device.GetRawAddress());
    }
}

void BluetoothBleCentralManagerServer::impl::GetBatchScanRandAddr(std::vector<BluetoothBleScanResult> &results,
    std::vector<RawAddress> &randomAddrs)
{
    for (auto &result : results) {
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(result.addr_, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(result.addr_);
        }
        randomAddrs.push_back(randomAddr);
    }
}

void BluetoothBleCentralManagerServer::impl::GetOptimalParam(uint16_t &optimalScanInterval, uint16_t &optimalScanWindow,
    std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue)
{
    BleScannerStateMachine::ScannerParameters optimalParam;
    BleScannerStateMachine::GetOptimalParam(batchscanAppQueue, optimalParam); // max DutyCycle or smaller ScanInterval
    optimalScanInterval = optimalParam.scanInterval;
    optimalScanWindow = optimalParam.scanWindow;
}

void BluetoothBleCentralManagerServer::impl::EnableBatchScanNative(int scannerId,
    std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue)
{
    uint16_t optimalScanInterval = 0;
    uint16_t optimalScanWindow = 0;
    GetOptimalParam(optimalScanInterval, optimalScanWindow, batchscanAppQueue); // support multi batchscan Apps
    int batchScanFullMax = 100; // Max storage space (in %) allocated to full style [Range: 0-100]
    int batchScanTruncMax = 0; // Max storage space (in %) allocated to truncated style [Range: 0-100]
    int batchScanNotifyThreshold = 95; // Notification level (in %) for individual storage pool [Range: 0-100]
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (!bleService) {
        HILOGE("bleService is nullptr.");
        return;
    }
    bleService->BatchscanConfigStorage(scannerId, batchScanFullMax, batchScanTruncMax, batchScanNotifyThreshold);
    int addrType = BLE_ADDR_TYPE_RANDOM;
    int discardRule = DISCARD_OLDEST;
    int scanMode = BATCH_SCAN_FORMAT_FULL; // only full mode supported currently
    bleService->BatchscanEnable(scanMode, optimalScanInterval, optimalScanWindow, addrType, discardRule);
}

void BluetoothBleCentralManagerServer::impl::EnableBatchScanTimer(
    std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue)
{
    batchScanTimerFastReport_->Start(BATCH_SCAN_FAST_REPORT_TIME, false); // 非周期一次性定时器
    HILOGI("BatchScan fast report timer started, period is 1000ms.");
    // use the min reportDelay to satisfy all batchscan Apps
    long minReportDelayMillisValue = applications->GetMinReportDelayMillisValue(batchscanAppQueue);
    batchScanTimer_->Start(minReportDelayMillisValue, true);  // 周期性定时器
    HILOGI("BatchScan timer started, period is %{public}ldms.", minReportDelayMillisValue);
}

bool BluetoothBleCentralManagerServer::impl::ResetBatchScan(int32_t pid, int32_t uid, int32_t scannerId)
{
    if (applications == nullptr) {
        HILOGE("applications is null");
        return false;
    }
    batchScanTimerFastReport_->Stop();
    batchScanTimer_->Stop();
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (!bleService) {
        HILOGE("bleService is nullptr.");
        return false;
    }
    // BT chip doesn't support changing batchscan settings when it's running, so stop first
    bleService->BatchscanDisable();

    std::list<BleScannerStateMachine::ScannerAppInfo> batchscanAppQueue = {};
    applications->GetBatchScanApps(batchscanAppQueue);
    if (!batchscanAppQueue.empty()) {
        EnableBatchScanNative(scannerId, batchscanAppQueue);
        EnableBatchScanTimer(batchscanAppQueue);
    }

    return true;
}

BluetoothBleCentralManagerServer::BluetoothBleCentralManagerServer()
{
    pimpl = std::make_unique<impl>();

    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService != nullptr) {
        bleService->RegisterBleCentralManagerCallback(*pimpl->observerImp_.get());
    }
}

BluetoothBleCentralManagerServer::~BluetoothBleCentralManagerServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
}

void BluetoothBleCentralManagerServer::Init(const wptr<BluetoothBleCentralManagerServer> &wptr)
{
    auto postTaskFunc = [](std::function<void(void)> func) {
        DoInBleThread(func);
    };
    auto startScanFunc = [](const bluetooth::BleScanSettingsImpl &setting) {
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        if (!bleService) {
            HILOGE("bleService is nullptr.");
            return;
        }
        bleService->StartScan(setting);
    };
    auto stopScanFunc = []() {
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        if (!bleService) {
            HILOGE("bleService is nullptr.");
            return;
        }
        bleService->StopScan();
    };

    auto reportStartedEventFunc = [this](int pid, int uid, int scannerId, int resultCode) {
        this->ReportStartedOrStoppedEvent(pid, uid, scannerId, resultCode, true);
    };

    auto reportStoppedEventFunc = [this](int pid, int uid, int scannerId, int resultCode) {
        this->ReportStartedOrStoppedEvent(pid, uid, scannerId, resultCode, false);
    };
#ifdef CONTEXTHUB_BLE_V3
    pimpl->scannerSM_->Init(postTaskFunc, startScanFunc, stopScanFunc, reportStartedEventFunc, reportStoppedEventFunc,
        BleScannerStateMachine::CollabrationFunc(SendScanCollaborationMsg, IsScanCollborationEnabled));
#else
    pimpl->scannerSM_->Init(postTaskFunc, startScanFunc, stopScanFunc, reportStartedEventFunc, reportStoppedEventFunc);
#endif
}

void BluetoothBleCentralManagerServer::ReportStartedOrStoppedEvent(int pid, int uid, int scannerId, int resultCode,
    bool isStartScan)
{
    HILOGD("isStartScanEvent = %{public}d", isStartScan);
    pimpl->observerImp_->GetObserver()->ForEach(
        [this, pid, uid, scannerId, resultCode, isStartScan](IBluetoothBleCentralManagerCallback *observer) {
            if (!pimpl->applications) {
                HILOGE("applications is null");
                return;
            }
            if (pimpl->applications->GetRemotePid(observer->AsObject()) != pid &&
                pimpl->applications->GetRemoteUid(observer->AsObject()) != uid) {
                return;
            }
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGW("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (pimpl->applications->GetScannerId(observer->AsObject()) != scannerId) {
                return;
            }

            HILOGD("OnStartOrStopScanEvent: pid:%{public}d uid:%{public}d scannerId:%{public}d", pid, uid, scannerId);
            observer->OnStartOrStopScanEvent(resultCode, isStartScan);
        });
}

int BluetoothBleCentralManagerServer::UpdateScanFilterInner(int32_t scannerId,
    const bluetooth::BleScanSettingsImpl &settings, const std::vector<BluetoothBleScanFilter> &filters,
    uint32_t filterAction)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr || pimpl->applications == nullptr) {
        HILOGE("bleService or applications is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bool isUseRealAddrFlag = pimpl->applications->GetUseRealAddrFlag(scannerId);
    auto filterImpls = pimpl->ConvertToBleScanFilterImpl(filters, isUseRealAddrFlag);

    int ret = bleService->UpdateScanFilter(scannerId, settings, filterImpls, filterAction);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    switch (filterAction) {
        case BLE_SCAN_UPDATE_FILTER_ADD:
            pimpl->applications->AppendBleScanFilter(scannerId, filterImpls);
            break;
        case BLE_SCAN_UPDATE_FILTER_DELETE:
            pimpl->applications->RemoveBleScanFilter(scannerId, filterImpls);
            break;
        case BLE_SCAN_UPDATE_FILTER_MODIFY:
            pimpl->applications->RemoveBleScanFilter(scannerId, filterImpls);
            pimpl->applications->AppendBleScanFilter(scannerId, filterImpls);
            break;
        default:
            break;
    }
    return ret;
}

int BluetoothBleCentralManagerServer::UpdateScanSettingsInner(int32_t pid, int32_t uid, int32_t scannerId,
    const bluetooth::BleScanSettingsImpl &settings)
{
    if ((pimpl == nullptr) || (pimpl->scannerSM_ == nullptr) || (pimpl->applications == nullptr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    BleScannerMessage stopMsg {
        BleScannerStateMachine::STOP_EVENT,
        BleScannerStateMachine::ScannerAppInfo(pid, uid, scannerId),
    };
    if (!pimpl->applications->GetScanIdState(pid, uid, scannerId)) {
        HILOGE("pid:%{public}d uid:%{public}d scannerId:%{public}d is not scaning", pid, uid, scannerId);
        return BT_ERR_BLE_CHANGE_SCAN_WRONG_STATE;
    }
    pimpl->scannerSM_->SendMessage(stopMsg);

    auto params = BleScannerStateMachine::ScannerParameters(settings);
    BleScannerMessage startMsg {
        BleScannerStateMachine::START_EVENT,
        BleScannerStateMachine::ScannerAppInfo(pid, uid, scannerId, params),
    };
    pimpl->scannerSM_->SendMessage(startMsg);
    pimpl->applications->SetScanIdSetting(pid, uid, scannerId, settings);
    return BT_NO_ERROR;
}

std::mutex BluetoothBleCentralManagerServer::proxyMutex_;
std::map<int32_t, uint8_t> BluetoothBleCentralManagerServer::proxyPids_;

bool BluetoothBleCentralManagerServer::FreezeByRss(std::set<int> pidSet, bool isFreeze, uint8_t freezeType)
{
    HILOGI("bluetooth freeze, pid[%{public}s] isFreeze: %{public}d freezeType : %{public}d",
        ToLogString(pidSet).c_str(), isFreeze, freezeType);
    CHECK_AND_RETURN_LOG_RET(freezeType <= FreezeType::LOW_POWER_TYPE, false, "Invalid freezeType");
    bool ret = true;
    for (int pid : pidSet) {
        std::lock_guard<std::mutex> lock(proxyMutex_);
        bool tmpRet = true;
        if (isFreeze) {
            if (freezeType != FreezeType::LOW_POWER_TYPE) {
                tmpRet = StartFreezeByRss(pid);
            } else if(proxyPids_.find(pid) != proxyPids_.end()) {
                tmpRet = StopFreezeByRss(pid);
            }
            proxyPids_[pid] = freezeType;
        } else {
            proxyPids_.erase(pid);
            tmpRet = StopFreezeByRss(pid);
        }
        ret = tmpRet ? ret : false;
    }
    return ret;
}

bool BluetoothBleCentralManagerServer::ResetAllProxy()
{
    HILOGI("Start bluetooth ResetAllProxy");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    proxyPids_.clear();
    return true;
}

bool BluetoothBleCentralManagerServer::IsResourceScheduleApp(int32_t pid, uint8_t wakeType)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (proxyPids_.find(pid) != proxyPids_.end()) {
        if (proxyPids_[pid] == FreezeType::LOW_POWER_TYPE && (wakeType == WakeType::BLE_SCAN_WAKE_TYPE ||
            wakeType == WakeType::ACL_STATE_WAKE_TYPE)) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

std::string BluetoothBleCentralManagerServer::DumpRssFreezeInfo()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    std::string ret = "";
    ret.append("Rss proxy pid list: \n");
    for (auto& iter: proxyPids_) {
        ret.append(std::to_string(iter.first)).append("  ");
    }

    if (pimpl == nullptr || pimpl->applications == nullptr) {
        HILOGE("pimpl is null.");
        return "\nInfo ERROR\n";
    }

    ret.append("\nScanFreezeAppInfo listInfo(pid uid scannerId): \n");
    if (pimpl->applications != nullptr) {
        ret.append(pimpl->applications->DumpScanFreezeAppInfo());
    } else {
        HILOGE("pimpl->applications is null.");
    }

    return ret;
}

bool CheckBleScanPermission(void)
{
    if (PermissionManager::IsNativeCaller() || PermissionManager::GetApiVersion() >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH), false,
            "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
            PermissionManager::GetCallingName().c_str());
    } else {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH), false,
            "[PERMISSION] check DISCOVER_BLUETOOTH permission failed, callingName(%{public}s)",
            PermissionManager::GetCallingName().c_str());

        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(MANAGE_BLUETOOTH), false,
            "[PERMISSION] check MANAGE_BLUETOOTH permission failed, callingName(%{public}s)",
            PermissionManager::GetCallingName().c_str());

        CHECK_AND_RETURN_LOG_RET((PermissionManager::VerifyPermission(APPROXIMATELY_LOCATION) ||
            PermissionManager::VerifyPermission(LOCATION)), false,
            "[PERMISSION] check LOCATION permission failed, callingName(%{public}s)",
            PermissionManager::GetCallingName().c_str());
    }
    return true;
}

void BluetoothBleCentralManagerServer::SetScanSettingsInfo(BleScanSettingsImpl &settingsImpl,
    const BluetoothBleScanSettings &settings)
{
    settingsImpl.SetReportDelay(settings.GetReportDelayMillisValue());
    settingsImpl.SetScanMode(settings.GetScanMode());
    settingsImpl.SetReportMode(settings.GetReportMode());
    settingsImpl.SetLegacy(settings.GetLegacy());
    settingsImpl.SetPhy(settings.GetPhy());
    settingsImpl.SetCallbackType(settings.GetCallbackType());
    settingsImpl.SetSensitivityMode(settings.GetSensitivityMode());
    settingsImpl.SetMatchTrackAdvType(settings.GetMatchTrackAdvType());
    bool isBleScanEnhanceModeEnable = (settings.GetEnhanceMode().mode != BLE_SCAN_ENHANCE_MODE_INVALID);
    settingsImpl.SetEnhanceMode(BleScanEnhanceModeImpl(isBleScanEnhanceModeEnable, settings.GetEnhanceMode().mode,
        settings.GetEnhanceMode().timeout));
}

void BluetoothBleCentralManagerServer::StartNormalScan(int32_t pid, int32_t uid, int32_t scannerId,
    BleScanSettingsImpl &settingsImpl)
{
    auto params = BleScannerStateMachine::ScannerParameters(settingsImpl);
    BleScannerMessage msg {
        BleScannerStateMachine::START_EVENT,
        BleScannerStateMachine::ScannerAppInfo(pid, uid, scannerId, params),
    };
    pimpl->scannerSM_->SendMessage(msg);
}

void BluetoothBleCentralManagerServer::ReportStartScanHiSysEvent(int32_t pid, int32_t uid, std::string callingName,
    const BluetoothBleScanSettings &settings)
{
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "BLE_SCAN_START",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid,
        "TYPE", (settings.GetReportDelayMillisValue() > 0) ? 1 : 0);
    int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE1);
    if ((settings.GetCallbackType() & BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH) != 0) {
        sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE2);
    }
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_START_SCAN, sceneCode, callingName);
}

bool BluetoothBleCentralManagerServer::IsValidBleScanEnhanceMode(bool isEnable,
    bluetooth::BleScanEnhanceMode &enhanceMode)
{
    if (enhanceMode.timeout < 0) {
        HILOGE("Invalid enhanceMode timeout: %{public}ld", enhanceMode.timeout);
        return false;
    }
    if (enhanceMode.mode < 0 || enhanceMode.mode > BLE_SCAN_ENHANCE_MODE_INVALID) {
        HILOGE("Invalid mode: %{public}d", enhanceMode.mode);
        return false;
    }
    if (isEnable && (enhanceMode.timeout == 0 || enhanceMode.mode == BLE_SCAN_ENHANCE_MODE_INVALID)) {
        HILOGE("Invalid enhanceMode timeout: %{public}ld, mode: %{public}d", enhanceMode.timeout, enhanceMode.mode);
        return false;
    }
    if (enhanceMode.timeout > BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS) {
        enhanceMode.timeout = BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS;
    }
    return true;
}

int BluetoothBleCentralManagerServer::SetBleScanEnhanceMode(bool isEnable,
    bluetooth::BleScanEnhanceMode enhanceMode)
{
#ifndef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    HILOGI("not support ble scan enhance mode");
    return BT_NO_ERROR;
#else
    if (enhanceMode.mode != BLE_SCAN_ENHANCE_MODE_INVALID && !PermissionManager::IsSystemHap()) {
        HILOGE("Non-system applications are not allowed to use system APIs.");
        return BT_ERR_PERMISSION_FAILED;
    }
    if (enhanceMode.mode != BLE_SCAN_ENHANCE_MODE_INVALID && !IsValidBleScanEnhanceMode(isEnable, enhanceMode)) {
        return BT_ERR_INVALID_PARAM;
    }
    IBluetoothResourceManager *resourceMgrService = IBluetoothResourceManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(resourceMgrService != nullptr, BT_ERR_INVALID_PARAM,
        "error, fail to get resourceMgrService");
    BleScanEnhanceModeImpl enhanceModeImpl = BleScanEnhanceModeImpl(isEnable, enhanceMode.mode, enhanceMode.timeout);
    HILOGI("enter, isEnable: %{public}d, mode: %{public}d, timeout: %{public}ld", isEnable, enhanceMode.mode,
        enhanceMode.timeout);
    resourceMgrService->SetBleScanEnhanceMode(enhanceModeImpl);
    return BT_NO_ERROR;
#endif
}

void SendBleScanMsg(std::string callingName, bool isStarted)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService) {
        uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
        if (IsWalletService(callingName, tokenId)) {
            bleService->SendBleScanMsg(isStarted);
        }
    }
}

bool BluetoothBleCentralManagerServer::IsWifiConnected() {
    std::shared_ptr<Wifi::WifiDevice> wifiDevice = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
    if (wifiDevice == nullptr) {
        HILOGE("Can not get Wifi::WifiDevice.");
        return false;
    }

    bool isWifiConnected = false;
    auto err = wifiDevice->IsConnected(isWifiConnected);
    if (err != 0) {
        HILOGE("fail to check wifi connect status: %{public}d", err);
        return false;
    }
    return isWifiConnected;
}

void BluetoothBleCentralManagerServer::AdjustScanModeForHi1112Chip(BleScanSettingsImpl &settingsImpl) {
    if (PermissionManager::IsSystemHap() &&
        (OHOS::system::GetParameter("ohos.boot.odm.conn.schiptype", "unknown") == "mp12") &&
        IsWifiConnected()) {
        HILOGI("system hap start ble low latency scan");
        settingsImpl.SetScanMode(SCAN_MODE_OP_P75_30_40);
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_BLE_LOW_LATENCY_SCAN,
            TRANSACTION_RESULT_SUCCESS);
    }
}

int BluetoothBleCentralManagerServer::StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings,
    const std::vector<BluetoothBleScanFilter> &filters, bool isNewApi)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("callingName(%{public}s), pid(%{public}d), uid(%{public}d), scannerId(%{public}d), reportMode(%{public}d)",
        callingName.c_str(), pid, uid, scannerId, settings.GetReportMode());

    CHECK_AND_RETURN_LOG_RET(CheckBleScanPermission(), BT_ERR_PERMISSION_FAILED, "CheckBleScanPermission failed");
    CHECK_AND_RETURN_LOG_RET(scannerId != BLE_SCAN_INVALID_ID, BT_ERR_INTERNAL_ERROR, "Invalid scannerId");
    if (IAdapterManager::GetInstance()->IsBleOnlyMode() &&
        !IAdapterManager::GetInstance()->IsBleAccessible(callingName)) {
        HILOGW("BLE_ONLY mode, refuse ble scan from non-owner app(%{public}s)", callingName.c_str());
        return BT_ERR_INVALID_STATE;
    }
    BleScanSettingsImpl settingsImpl;
    SetScanSettingsInfo(settingsImpl, settings);
    if (settings.GetScanMode() == SCAN_MODE_LOW_LATENCY) {
        AdjustScanModeForHi1112Chip(settingsImpl);
    }
    if (settings.GetEnhanceMode().mode != BLE_SCAN_ENHANCE_MODE_INVALID) {
        CHECK_AND_RETURN_LOG_RET(SetBleScanEnhanceMode(true, settings.GetEnhanceMode()) == BT_NO_ERROR,
            BT_ERR_INTERNAL_ERROR, "fail to start scan due to invalid enhance mode");
    }

    int ret = ConfigScanFilterInner(scannerId, settingsImpl, filters);
    if (ret != BT_NO_ERROR) {
        // Do not report general errorCode to CHR no longer.
        if (pimpl->applications && ret != GATT_FAILURE) {
            pimpl->applications->ChrReportScanFilter(ret);
        }
        if (ret == BT_ERR_BLE_SCAN_ALREADY_STARTED || ret == BT_ERR_BLE_SCAN_NO_RESOURCE ||
            ret == BT_ERR_SYSTEM_PERMISSION_FAILED) {
            return ret;
        }
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->applications) {
        pimpl->applications->SetScanIdSetting(pid, uid, scannerId, settingsImpl);
        pimpl->applications->SetScanIdState(pid, uid, scannerId, true);
        pimpl->applications->SetCallingName(scannerId, callingName);
    }

    if (settingsImpl.GetReportMode() == REPORT_MODE_BATCH) {
        // batchscan
        pimpl->applications->SetBatchScanFlag(pid, uid, scannerId, true); // to update batchscan app queue
        if (!pimpl->ResetBatchScan(pid, uid, scannerId)) {
            return BT_ERR_INTERNAL_ERROR;
        }
        ReportStartedOrStoppedEvent(pid, uid, scannerId, BT_NO_ERROR, true);
    } else {
        // normal scan
        StartNormalScan(pid, uid, scannerId, settingsImpl);
    }
    SendBleScanMsg(callingName, true);
    ReportStartScanHiSysEvent(pid, uid, callingName, settings);
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::StopScan(int32_t scannerId)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("callingName(%{public}s), pid(%{public}d), uid(%{public}d), scannerId(%{public}d)",
        callingName.c_str(), pid, uid, scannerId);

    if (pimpl->applications->IsBatchScanApp(pid, uid, scannerId)) {
        // batch scan
        pimpl->applications->SetBatchScanFlag(pid, uid, scannerId, false);
        bool ret = pimpl->ResetBatchScan(pid, uid, scannerId);
        if (!ret) {
            return BT_ERR_INTERNAL_ERROR;
        }
        ReportStartedOrStoppedEvent(pid, uid, scannerId, BT_NO_ERROR, false);
    } else {
        // normal scan
        BleScannerMessage msg {
            BleScannerStateMachine::STOP_EVENT,
            BleScannerStateMachine::ScannerAppInfo(pid, uid, scannerId),
        };
        pimpl->scannerSM_->SendMessage(msg);
    }

    if (pimpl->applications) {
        pimpl->applications->SetScanIdState(pid, uid, scannerId, false);
        BleScanEnhanceModeImpl enhanceMode = pimpl->applications->GetScanSetting(scannerId).GetEnhanceMode();
        int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (enhanceMode.isEnable && (currentTime - enhanceMode.timestamp) < enhanceMode.timeout) {
            SetBleScanEnhanceMode(false, bluetooth::BleScanEnhanceMode());
        }
    }
    SendBleScanMsg(callingName, false);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "BLE_SCAN_STOP",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_STOP_SCAN, UE_COMMON_SCENE_CASE1, callingName);
    return BT_NO_ERROR;
}

bool BluetoothBleCentralManagerServer::StartFreezeByRss(int32_t pid)
{
    if (pimpl->applications == nullptr) {
        HILOGE("applications is null");
        return false;
    }

    auto appsInfo = pimpl->applications->FindFreezeInfoByPid(pid);
    if (appsInfo.size() == 0) {
        return false;
    }

    for (auto appinfo : appsInfo) {
        if (appinfo.isAppActiveScan == false || appinfo.isRssFreeze == true) {
            HILOGW("freeze app has not been scanned: %{public}d or has been freezed: %{public}d",
                appinfo.isAppActiveScan, appinfo.isRssFreeze);
            continue;
        }

        uint32_t tokenId = static_cast<uint32_t>(appinfo.tokenId);
        std::string callingName = PermissionManager::GetCallingName(tokenId);

        pimpl->applications->SetFreezeState(appinfo.pid, appinfo.uid, appinfo.scannerId, true);
        RemoveScanFilterByRss(appinfo.scannerId);

        if (appinfo.isBatchScan) {
            bool ret = pimpl->ResetBatchScan(appinfo.pid, appinfo.uid, appinfo.scannerId);
            if (!ret) {
                return false;
            }
            HILOGI("Freeze batchscan app pid(%{public}d), uid(%{public}d), scannerId(%{public}d),"
                "callingName(%{public}s)", appinfo.pid, appinfo.uid, appinfo.scannerId, callingName.c_str());
        } else {
            BleScannerMessage msg{
                BleScannerStateMachine::STOP_EVENT,
                BleScannerStateMachine::ScannerAppInfo(appinfo.pid, appinfo.uid, appinfo.scannerId),
            };
            pimpl->scannerSM_->SendMessage(msg);
        }
    }
    return true;
}

bool BluetoothBleCentralManagerServer::StopFreezeByRss(int32_t pid)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr || pimpl->applications == nullptr) {
        HILOGE("bleService or applications is null");
        return false;
    }

    auto appsInfo = pimpl->applications->FindFreezeInfoByPid(pid);
    if (appsInfo.size() == 0) {
        return false;
    }

    for (auto appinfo : appsInfo) {
        if (appinfo.isAppActiveScan == false || appinfo.isRssFreeze == false) {
            HILOGW("freeze app has not been scanned: %{public}d or has been freezed: %{public}d",
                appinfo.isAppActiveScan, appinfo.isRssFreeze);
            continue;
        }

        uint32_t tokenId = static_cast<uint32_t>(appinfo.tokenId);
        std::string callingName = PermissionManager::GetCallingName(tokenId);
        int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE3);

        pimpl->applications->SetFreezeState(appinfo.pid, appinfo.uid, appinfo.scannerId, false);
        bleService->ConfigScanFilter(appinfo.scannerId, appinfo.settings,
            appinfo.bleScanFilterContainer.bleScanFilters);

        if (appinfo.isBatchScan) {
            bool ret = pimpl->ResetBatchScan(appinfo.pid, appinfo.uid, appinfo.scannerId);
            if (!ret) {
                return false;
            }
            HILOGI("Unfreeze batchscan app pid(%{public}d), uid(%{public}d), scannerId(%{public}d),"
                "callingName(%{public}s)", appinfo.pid, appinfo.uid, appinfo.scannerId, callingName.c_str());
        } else {
            auto params = BleScannerStateMachine::ScannerParameters(appinfo.settings);
            BleScannerMessage msg{
                BleScannerStateMachine::START_EVENT,
                BleScannerStateMachine::ScannerAppInfo(appinfo.pid, appinfo.uid, appinfo.scannerId, params),
            };
            pimpl->scannerSM_->SendMessage(msg);
        }
    }
    return true;
}

static std::vector<BluetoothBleScanFilter> RemoveDuplicateFilters(const std::vector<BluetoothBleScanFilter> &filters)
{
    // remove duplicate filters
    std::vector<BluetoothBleScanFilter> uniqueFilters;
    for (const auto& filter : filters) {
        if (std::find(uniqueFilters.begin(), uniqueFilters.end(), filter) == uniqueFilters.end()) {
            uniqueFilters.push_back(filter);
        }
    }
    if (filters.size() - uniqueFilters.size() > 0) {
        HILOGI("input:%{public}lu, unique:%{public}lu, removed:%{public}lu",
            filters.size(), uniqueFilters.size(), (filters.size() - uniqueFilters.size()));
    }
    return uniqueFilters;
}

int BluetoothBleCentralManagerServer::ConfigScanFilterInner(
    int32_t scannerId, const BleScanSettingsImpl &settings, const std::vector<BluetoothBleScanFilter> &filters)
{
    for (const auto& filter : filters) {
        if (!filter.GetIrk().empty() && !PermissionManager::IsSystemHap()) {
            // irk is a systemAPI.
            HILOGE("Non-system applications are not allowed to use system APIs.");
            return BT_ERR_SYSTEM_PERMISSION_FAILED;
        }
    }
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr || pimpl->applications == nullptr) {
        HILOGE("bleService or applications is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<BluetoothBleScanFilter> uniqueFilters = RemoveDuplicateFilters(filters);
    bool isUseRealAddrFlag = pimpl->applications->GetUseRealAddrFlag(scannerId);
    auto filterImpls = pimpl->ConvertToBleScanFilterImpl(uniqueFilters, isUseRealAddrFlag);
    int ret = bleService->ConfigScanFilter(scannerId, settings, filterImpls);
    if (ret != GATT_SUCCESS) {
        return ret;
    }
    pimpl->applications->SetBleScanFilter(scannerId, filterImpls);
    return BT_NO_ERROR;
}

void BluetoothBleCentralManagerServer::RemoveScanFilter(int32_t scannerId)
{
    HILOGD("scannerId: %{public}d", scannerId);

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService && pimpl->applications) {
        uint16_t filtersSize = pimpl->applications->GetBleScanFilterSize(scannerId);
        pimpl->applications->ClearBleScanFilter(scannerId);
        bleService->RemoveScanFilter(scannerId, pimpl->applications->GetScanSetting(scannerId), filtersSize);
    }
}

void BluetoothBleCentralManagerServer::RemoveScanFilterByRss(int32_t scannerId)
{
    HILOGW("scannerId: %{public}d", scannerId);

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService) {
        uint16_t filtersSize = pimpl->applications->GetBleScanFilterSize(scannerId);
        bleService->RemoveScanFilter(scannerId, pimpl->applications->GetScanSetting(scannerId), filtersSize);
    }
}

void BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback(int32_t &scannerId, bool enableRandomAddrMode,
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGD("pid: %{public}d, uid: %{public}d", pid, uid);

    if (callback == nullptr) {
        HILOGE("callback is null");
        return;
    }
    if (pimpl->applications->Contain(callback->AsObject())) {
        scannerId = pimpl->applications->GetScannerId(callback->AsObject());
        HILOGI("scanner callback is already registered, use stored scanner id: %{public}d", scannerId);
        return;
    }
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return;
    }
    scannerId = bleService->AllocScannerId();
    if (scannerId == BLE_SCAN_INVALID_ID) {
        HILOGE("Alloc ScannerId fail.");
        return;
    }
    if (pimpl->observers_.Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ too much");
        return;
    }
    pimpl->observers_.Register(callback);
    if (pimpl->applications) {
        pimpl->applications->AddRemoteObject(pid, uid, callback->AsObject());
        pimpl->applications->AddTokenId(callback->AsObject(), IPCSkeleton::GetCallingFullTokenID());
        pimpl->applications->AddSdkVersion(callback->AsObject(), PermissionManager::GetApiVersion());
        pimpl->applications->SetScannerId(callback->AsObject(), pid, uid, scannerId);
        bool isUseRealAddrFlag = PermissionManager::IsUseRealAddr();
        pimpl->applications->SetUseRealAddrFlag(callback->AsObject(), isUseRealAddrFlag);
    }
}

void BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback(int32_t scannerId,
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    HILOGI("scannerId: %{public}d", scannerId);
    if (callback == nullptr) {
        HILOGE("callback is null");
        return;
    }
    pimpl->observers_.Deregister(callback);

    // To avoid the scanner is not turn off, when BluetoothBleCentralManager is deleted with scan is not stopped.
    StopScan(scannerId);
    RemoveScanFilter(scannerId);

    if (pimpl->applications) {
        pimpl->applications->RemoveRemoteObject(callback->AsObject());
    }

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return;
    }
    bleService->RemoveScannerId(scannerId);
}

int BluetoothBleCentralManagerServer::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval,
    int advHandle)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->SetScanReportChannelToLpDevice(scannerId, enable);

#ifdef CONTEXTHUB_BLE_V3
    pimpl->shCollaboration_->SetScanReportChannelToLpDevice(scannerId, enable);
#endif

    pimpl->applications->SetLpDeviceFlag(scannerId, enable);
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::EnableSyncDataToLpDevice()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->EnableSyncDataToLpDevice();
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::DisableSyncDataToLpDevice()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->DisableSyncDataToLpDevice();
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->SendParamsToLpDevice(dataValue, type);
    return BT_NO_ERROR;
}

bool BluetoothBleCentralManagerServer::IsLpDeviceAvailable()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return false;
    }
    return bleService->IsLpDeviceAvailable();
}

int BluetoothBleCentralManagerServer::SetLpDeviceParam(const BluetoothLpDeviceParamSet &paramSet)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }

    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    FilterIdxInfo filterIdx(pid, uid, paramSet.uuid);

    BleLpDeviceParamSetImpl paramImpl;
    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) != 0) {
        paramImpl.scanSettingImpl = pimpl->ConvertToBleScanSettingsImpl(paramSet.btScanSettings);
    }

    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) != 0) {
        paramImpl.scanFliterImpls = pimpl->ConvertToBleScanFilterImpl(paramSet.btScanFilters, true);
    }

    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        paramImpl.advSettingsImpl = pimpl->ConvertToBleAdvertiserSettingsImpl(paramSet.btAdvSettings);
    }

    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        paramImpl.advDataImpl = pimpl->ConvertToBleAdvertiserDataImpl(paramSet.btAdvData);
    }

    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        paramImpl.respDataImpl = pimpl->ConvertToBleAdvertiserDataImpl(paramSet.btRespData);
    }

    if ((paramSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
        paramImpl.activeDeviceInfoImpls = pimpl->ConvertToBleActiveDeviceInfoImpl(paramSet.activeDeviceInfos);
    }

    paramImpl.advHandle = paramSet.advHandle;
    paramImpl.duration = paramSet.duration;
    paramImpl.deliveryMode = paramSet.deliveryMode;
    paramImpl.fieldValidFlagBit = paramSet.fieldValidFlagBit;

    bleService->SetLpDeviceParam(filterIdx, paramImpl);
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::RemoveLpDeviceParam(const bluetooth::Uuid &uuid)
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }

    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    FilterIdxInfo filterIdx(pid, uid, uuid);

    bleService->RemoveLpDeviceParam(filterIdx);
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings &settings,
    const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGI("callingName(%{public}s), pid(%{public}d), uid(%{public}d), scannerId(%{public}d)",
        PermissionManager::GetCallingName().c_str(), pid, uid, scannerId);
    if (!CheckBleScanPermission()) {
        HILOGE("CheckBleScanPermission failed");
        return BT_ERR_PERMISSION_FAILED;
    }

    auto oldScanSetting = pimpl->applications->GetScanSetting(scannerId);
    if ((oldScanSetting.GetCallbackType() != settings.GetCallbackType() ||
        oldScanSetting.GetMatchTrackAdvType() != settings.GetMatchTrackAdvType()) &&
        pimpl->applications->GetBleScanFilterSize(scannerId) != 0) {
        HILOGW("not support update callbackType or trackAdvType");
        return BT_ERR_INVALID_PARAM;
    }
    BleScanSettingsImpl settingsImpl;
    settingsImpl.SetReportDelay(settings.GetReportDelayMillisValue());
    settingsImpl.SetScanMode(settings.GetScanMode());
    settingsImpl.SetLegacy(settings.GetLegacy());
    settingsImpl.SetPhy(settings.GetPhy());
    settingsImpl.SetCallbackType(settings.GetCallbackType());
    settingsImpl.SetMatchTrackAdvType(settings.GetMatchTrackAdvType());
    if (filterAction != 0 && filters.size() != 0) {
        int ret = UpdateScanFilterInner(scannerId, settingsImpl, filters, filterAction);
        CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_BLE_CHANGE_SCAN_FILTER_FAIL,
            "UpdateScanFilterInner err:%{public}d", ret);
    }
    return UpdateScanSettingsInner(pid, uid, scannerId, settingsImpl);
}

int BluetoothBleCentralManagerServer::FlushBatchScanResults(int32_t scannerId)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("callingName(%{public}s), pid(%{public}d), uid(%{public}d), scannerId(%{public}d)",
        callingName.c_str(), pid, uid, scannerId);
    if (!CheckBleScanPermission()) {
        HILOGE("CheckBleScanPermission failed");
        return BT_ERR_PERMISSION_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(scannerId != BLE_SCAN_INVALID_ID, BT_ERR_INTERNAL_ERROR, "Invalid scannerId");
    bool isValid = pimpl->applications->IsValidScannerId(pid, uid, scannerId);
    CHECK_AND_RETURN_LOG_RET(isValid, BT_ERR_INTERNAL_ERROR, "Unknown scannerId");

    int scanMode = BATCH_SCAN_FORMAT_FULL; // full mode
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (!bleService) {
        HILOGE("bleService is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    bleService->BatchscanReadReports(scannerId, scanMode);
    CHECK_AND_RETURN_LOG_RET(pimpl->ResetBatchScan(pid, uid, scannerId), BT_ERR_INTERNAL_ERROR, "resetbatchscan error");
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerServer::IsValidScannerId(int32_t scannerId, bool &isValid)
{
    if (!CheckBleScanPermission()) {
        HILOGE("CheckBleScanPermission failed");
        return BT_ERR_PERMISSION_FAILED;
    }
    int pid = IPCSkeleton::GetCallingPid();
    int uid = IPCSkeleton::GetCallingUid();
    HILOGD("callingName(%{public}s), pid(%{public}d), uid(%{public}d), scannerId(%{public}d)",
        PermissionManager::GetCallingName().c_str(), pid, uid, scannerId);
    isValid = pimpl->applications->IsValidScannerId(pid, uid, scannerId);
    return BT_NO_ERROR;
}

void BluetoothBleScannerApplicationContainer::AddTokenId(const wptr<IRemoteObject> &remote, uint64_t tokenId)
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

uint64_t BluetoothBleScannerApplicationContainer::GetTokenId(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return 0;
    }
    return it->tokenId;
}

std::vector<BluetoothBleScannerApplication> BluetoothBleScannerApplicationContainer::FindFreezeInfoByPid(int pid)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    std::vector<BluetoothBleScannerApplication> ret;
    for (const auto &app : container_) {
        if (app.pid == pid) {
            ret.push_back(app);
        }
    }
    return ret;
}

std::string BluetoothBleScannerApplicationContainer::DumpScanFreezeAppInfo()
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    std::string ret = "";
    for (auto& info: container_) {
        ret.append(std::to_string(info.pid)).append(" ");
        ret.append(std::to_string(info.uid)).append(" ");
        ret.append(std::to_string(info.scannerId)).append("\n");
    }
    return ret;
}

void BluetoothBleScannerApplicationContainer::AddSdkVersion(const wptr<IRemoteObject> &remote, int sdkVersion)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return;
    }
    HILOGD("pid: %{public}d, uid: %{public}d, sdkVersion: %{public}d", it->pid, it->uid, sdkVersion);
    it->sdkVersion = sdkVersion;
}

int BluetoothBleScannerApplicationContainer::GetSdkVersion(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return -1;
    }
    return it->sdkVersion;
}

void BluetoothBleScannerApplicationContainer::SetScannerId(const wptr<IRemoteObject> &remote, int pid, int uid,
    int scannerId)
{
    HILOGD("pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGI("SetScannerId appScanner not exist. pid: %{public}d, uid: %{public}d, scannerId: %{public}d",
            pid, uid, scannerId);
        return;
    }
    if (it->pid != pid || it->uid != uid) {
        HILOGI("SetScannerId pid uid wrong. pid: %{public}d, uid: %{public}d, scannerId: %{public}d",
            pid, uid, scannerId);
        return;
    }
    it->scannerId = scannerId;
}

void BluetoothBleScannerApplicationContainer::ClearScannerId(void)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    for (auto &app : container_) {
        app.scannerId = BLE_SCAN_INVALID_ID;
    }
}

int32_t BluetoothBleScannerApplicationContainer::GetScannerId(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return BLE_SCAN_INVALID_ID;
    }
    return it->scannerId;
}

void BluetoothBleScannerApplicationContainer::SetUseRealAddrFlag(const wptr<IRemoteObject> &remote, bool flag)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return;
    }
    it->isUseRealAddrFlag = flag;
}

bool BluetoothBleScannerApplicationContainer::GetUseRealAddrFlag(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return true;
    }
    return it->isUseRealAddrFlag;
}

bool BluetoothBleScannerApplicationContainer::GetUseRealAddrFlag(int32_t scannerId) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId");
        std::string callingName = PermissionManager::GetCallingName();
        int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE5);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_START_SCAN, sceneCode, callingName);
        return true;
    }
    return it->isUseRealAddrFlag;
}

void BluetoothBleScannerApplicationContainer::SetBleScanFilter(const int32_t scannerId,
    std::vector<BleScanFilterImpl> bleScanFilters)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter");
        std::string callingName = PermissionManager::GetCallingName();
        int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE5);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_START_SCAN, sceneCode, callingName);
        return;
    }
    HILOGD("bleScanFilters updated size = %{public}lu.", bleScanFilters.size());
    it->bleScanFilterContainer.isFiltersEnable = true;
    it->bleScanFilterContainer.bleScanFilters = bleScanFilters;
}

void BluetoothBleScannerApplicationContainer::ClearBleScanFilter(const int32_t scannerId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter");
        return;
    }
    it->bleScanFilterContainer.isFiltersEnable = false;
    it->bleScanFilterContainer.bleScanFilters.clear();
}

void BluetoothBleScannerApplicationContainer::RemoveBleScanFilter(const int32_t scannerId,
    std::vector<bluetooth::BleScanFilterImpl> bleScanFilters)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter");
        return;
    }

    for (auto filter : bleScanFilters) {
        uint8_t filterIndex = filter.GetFiltIndex();
        auto newEnd = std::remove_if(it->bleScanFilterContainer.bleScanFilters.begin(),
            it->bleScanFilterContainer.bleScanFilters.end(),
            [filterIndex] (const bluetooth::BleScanFilterImpl& scanFilter) {
            return scanFilter.GetFiltIndex() == filterIndex;
        });
        it->bleScanFilterContainer.bleScanFilters.erase(newEnd, it->bleScanFilterContainer.bleScanFilters.end());
    }
}

void BluetoothBleScannerApplicationContainer::AppendBleScanFilter(const int32_t scannerId,
    std::vector<bluetooth::BleScanFilterImpl> bleScanFilters)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter");
        return;
    }
    it->bleScanFilterContainer.bleScanFilters.insert(it->bleScanFilterContainer.bleScanFilters.end(),
        bleScanFilters.begin(), bleScanFilters.end());
}

uint16_t BluetoothBleScannerApplicationContainer::GetBleScanFilterSize(const int32_t scannerId) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter for getting");
        return 0;
    }
    size_t filterSize = it->bleScanFilterContainer.bleScanFilters.size();
    if (filterSize > BLE_CENTRAL_MANAGER_SCAN_FILTER_MAX_SIZE) {
        HILOGE("Unexpected filter size = %{public}lu for getting, ignored it.", filterSize);
        return 0;
    }
    return static_cast<uint16_t>(filterSize);
}

bool BluetoothBleScannerApplicationContainer::IsBleScanFilterMatched(const int32_t scannerId,
    const BluetoothBleScanResult &bleScanResult, bluetooth::BleScanFilterImpl &matchedFilter) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId And Filter for match");
        return false;
    }
    // If bleScanFilters been set empty when stopped scan, we need refuse all callback instead of empty filter pass
    if (!it->bleScanFilterContainer.isFiltersEnable) {
        return false;
    }
    if (it->bleScanFilterContainer.bleScanFilters.size() > BLE_CENTRAL_MANAGER_SCAN_FILTER_MAX_SIZE) {
        HILOGE("Unexpected filter size = %{public}lu for match, ignored it.",
            it->bleScanFilterContainer.bleScanFilters.size());
        return false;
    }
    if (BluetoothBleFilterMatcher::MatchesScanFilters(it->bleScanFilterContainer.bleScanFilters,
        bleScanResult, matchedFilter) == FilterCheckState::FILTER_CHECK_FAIL) {
        return false;
    }
    return true;
}

void BluetoothBleScannerApplicationContainer::SetLpDeviceFlag(int32_t scannerId, bool enable)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId and filter");
        return;
    }
    for (auto &filter : it->bleScanFilterContainer.bleScanFilters) {
        filter.SetLpDeviceFlag(enable);
    }
}

void BluetoothBleScannerApplicationContainer::SetScanIdSetting(const int32_t pid, const int32_t uid,
    const int32_t scannerId, BleScanSettingsImpl settings)
{
    HILOGD("pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        std::string callingName = PermissionManager::GetCallingName();
        int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE5);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_START_SCAN, sceneCode, callingName);
        return;
    }
    it->settings = settings;
}

void BluetoothBleScannerApplicationContainer::GetBatchScanApps(std::list<BleScannerStateMachine::ScannerAppInfo>
    &batchscanAppQueue)
{
    // mainly used to calculate batchscan parameters in multi-batchScanApps scenarios
    std::lock_guard<std::mutex> lock(containerMutex_);
    for (auto &app : container_) {
        if (app.isBatchScan && app.isAppActiveScan && (!app.isRssFreeze)) {
            // freezed batchscan app should not be considered
            BleScannerStateMachine::ScannerAppInfo scannerAppInfo(app.pid, app.uid, app.scannerId,
                BleScannerStateMachine::ScannerParameters(app.settings));
            batchscanAppQueue.push_back(scannerAppInfo);
        }
    }
}

bool BluetoothBleScannerApplicationContainer::IsBatchScanApp(const int32_t pid, const int32_t uid,
    const int32_t scannerId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it != container_.end() && it->isBatchScan) {
        return true;
    } else {
        return false;
    }
}

void BluetoothBleScannerApplicationContainer::SetBatchScanFlag(const int32_t pid, const int32_t uid,
    const int32_t scannerId, bool isBatchScan)
{
    HILOGD("pid: %{public}d, uid: %{public}d, scannerId: %{public}d, isBatchScan: %{public}d", pid, uid, scannerId,
        isBatchScan);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        return;
    }
    it->isBatchScan = isBatchScan;
}

long BluetoothBleScannerApplicationContainer::GetMinReportDelayMillisValue(
    std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue)
{
    auto minReportDelayIterator = std::min_element(batchscanAppQueue.begin(), batchscanAppQueue.end(),
        [](const BleScannerStateMachine::ScannerAppInfo& a, const BleScannerStateMachine::ScannerAppInfo& b) {
            return a.params.reportDelayMillis < b.params.reportDelayMillis;
        });
    return minReportDelayIterator->params.reportDelayMillis;
}

void BluetoothBleScannerApplicationContainer::SetScanIdState(const int32_t pid, const int32_t uid,
    const int32_t scannerId, bool isAppActiveScan)
{
    HILOGD("pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        std::string callingName = PermissionManager::GetCallingName();
        int sceneCode = static_cast<int>(UE_COMMON_SCENE_CASE5);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_START_SCAN, sceneCode, callingName);
        return;
    }
    it->isAppActiveScan = isAppActiveScan;
}

bool BluetoothBleScannerApplicationContainer::GetScanIdState(const int32_t pid, const int32_t uid, int32_t scannerId)
{
    HILOGD("pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        return false;
    }
    return it->isAppActiveScan;
}

bool BluetoothBleScannerApplicationContainer::IsValidScannerId(const int32_t pid, const int32_t uid, int32_t scannerId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        return false;
    }
    return true;
}

void BluetoothBleScannerApplicationContainer::SetFreezeState(const int32_t pid, const int32_t uid,
    const int32_t scannerId, bool isRssFreeze)
{
    HILOGI("pid: %{public}d, uid: %{public}d, scannerId: %{public}d isRssFreeze: %{public}d",
        pid, uid, scannerId, isRssFreeze);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByPidScanId(pid, uid, scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown appinfo pid: %{public}d, uid: %{public}d, scannerId: %{public}d", pid, uid, scannerId);
        return;
    }
    it->isRssFreeze = isRssFreeze;
}

void BluetoothBleScannerApplicationContainer::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (!Contain(remote)) {
        HILOGE("Invalid remote object");
        return;
    }
    int pid = GetRemotePid(remote);
    int uid = GetRemoteUid(remote);
    uint64_t tokenId = GetTokenId(remote);
    std::string callingName = PermissionManager::GetCallingName(static_cast<uint32_t>(tokenId));
    int32_t scannerId = GetScannerId(remote);
    BleScanSettingsImpl settings = GetScanSetting(scannerId);
    uint16_t filtersSize = GetBleScanFilterSize(scannerId);
    bool isBatchScan = IsBatchScanApp(pid, uid, scannerId);
    RemoveRemoteObject(remote);

    HILOGI("Stopscan pid(%{public}d), uid(%{public}d), scannerId(%{public}d)", pid, uid, scannerId);

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService == nullptr) {
        HILOGE("bleService is null");
        return;
    }
    bleService->RemoveScanFilter(scannerId, settings, filtersSize);
    bleService->RemoveScannerId(scannerId);

    if (isBatchScan) {
        if (resetBatchScanCb_ == nullptr) {
            HILOGE("resetBatchScanCb_ is null");
            return;
        }
        bool ret = resetBatchScanCb_(pid, uid, scannerId);
        if (!ret) {
            return;
        }
        HILOGI("Stop batchscan app, callingName(%{public}s), pid(%{public}d), uid(%{public}d),"
            "scannerId(%{public}d)", callingName.c_str(), pid, uid, scannerId);
    } else {
        auto sptr = sm_.lock();
        if (!sptr) {
            HILOGE("BleScannerStateMachine is nullptr");
            return;
        }
        BleScannerMessage msg {
            BleScannerStateMachine::STOP_EVENT,
            BleScannerStateMachine::ScannerAppInfo(pid, uid, scannerId),
        };
        sptr->SendMessage(msg);
    }
}

BleScanSettingsImpl BluetoothBleScannerApplicationContainer::GetScanSetting(uint8_t scannerId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByScanId(scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        BleScanSettingsImpl tmp;
        return tmp;
    }
    return it->settings;
}

sptr<IRemoteObject> BluetoothBleScannerApplicationContainer::GetRemote(uint8_t scannerId)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIterByScanId(scannerId);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return nullptr;
    }
    return it->remote.promote();
}

void BluetoothBleScannerApplicationContainer::SetCallingName(const int32_t scannerId, std::string name)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = std::find_if(container_.begin(), container_.end(),
        [scannerId](const auto &app) { return app.scannerId == scannerId; });
    if (it == container_.end()) {
        HILOGE("Unknown scannerId SetCallingName");
        return;
    }
    it->callingName = name;
}

void BluetoothBleScannerApplicationContainer::ChrReportScanFilter(int32_t subErrCode)
{
    std::string filterInfo = "filter:";
    {
        std::lock_guard<std::mutex> lock(containerMutex_);
        for (auto& info: container_) {
            filterInfo.append(info.callingName).append(" ");
            filterInfo.append(std::to_string(info.bleScanFilterContainer.bleScanFilters.size())).append(" ");
        }
    }
    HILOGE("ChrReportScanFilter info: %{public}s", filterInfo.c_str());

    int32_t errCode = 46; // error code for set ble scan filter failed
    uint32_t maxLen = 1023; // CHR hci code max length
    uint32_t hciProcessLen = 3; // CHR hci code each string length
    std::string filterFailedFakeAddress = "11:22:**:**:**:66"; // fake adress
    uint8_t remainder = filterInfo.size() % hciProcessLen;
    if (remainder != 0) {
        uint8_t padding = hciProcessLen - remainder;
        filterInfo.append(padding, ' ');
    }
    if (filterInfo.size() > maxLen) {
        filterInfo = filterInfo.substr(filterInfo.size() - maxLen);
    }
    for (uint32_t i = 0; i < filterInfo.size(); i += hciProcessLen) {
        BtChrHciProcessData::GetInstance().SaveImportantHciProcessData(filterFailedFakeAddress,
            filterInfo.substr(i, hciProcessLen));
    }

    BtChrBtExcpEvent(filterFailedFakeAddress, errCode, subErrCode);
}
}  // namespace Bluetooth
}  // namespace OHOS
