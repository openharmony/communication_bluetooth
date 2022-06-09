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

#include "bluetooth_ble_central_manager_server.h"
#include "ble_service_data.h"
#include "bluetooth_log.h"
#include "event_handler.h"
#include "event_runner.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "remote_observer_list.h"
#include "permission_utils.h"
#include <string>

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
struct BluetoothBleCentralManagerServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    RemoteObserverList<IBluetoothBleCentralManagerCallback> observers_;
    std::map<sptr<IRemoteObject>, uint32_t>     observersToken_;
    class BleCentralManagerCallback;
    std::unique_ptr<BleCentralManagerCallback> observerImp_ = std::make_unique<BleCentralManagerCallback>(this);
    IAdapterBle *bleService_ = nullptr;

    struct ScanCallbackInfo;
    std::vector<ScanCallbackInfo> scanCallbackInfo_;

    BleScanSettingsImpl scanSettingImpl_;
    bool isScanning;

    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
};

struct BluetoothBleCentralManagerServer::impl::ScanCallbackInfo {
    int pid_;
    int uid_;
    bool isStart_;
    sptr<IBluetoothBleCentralManagerCallback> callback_;
};

class BluetoothBleCentralManagerServer::impl::BleCentralManagerCallback : public IBleCentralManagerCallback {
public:
    explicit BleCentralManagerCallback(BluetoothBleCentralManagerServer::impl *pimpl) : pimpl_(pimpl) {};
    ~BleCentralManagerCallback() override = default;

    void OnScanCallback(const BleScanResultImpl &result) override
    {
        HILOGI("BleCentralManageCallback::OnScanCallback start.");
        observers_->ForEach([this, result](IBluetoothBleCentralManagerCallback *observer) {
            HILOGI("BleCentralManageCallback::OnScanCallback:Address= %{public}s",
                result.GetPeripheralDevice().GetRawAddress().GetAddress().c_str());
            uint32_t  tokenId = this->pimpl_->observersToken_[observer->AsObject()];
            if (PermissionUtils::VerifyUseBluetoothPermission(tokenId) == PERMISSION_DENIED) {
                HILOGE("OnScanCallback() false, check permission failed");
            } else {
                BluetoothBleScanResult bleScanResult;
                if (result.GetPeripheralDevice().IsRSSI()) {
                    bleScanResult.SetRssi(result.GetPeripheralDevice().GetRSSI());
                }

                bleScanResult.SetAdvertiseFlag(result.GetPeripheralDevice().GetAdFlag());

                if (result.GetPeripheralDevice().IsManufacturerData()) {
                    std::map<uint16_t, std::string> manuData = result.GetPeripheralDevice().GetManufacturerData();
                    for (auto it = manuData.begin(); it != manuData.end(); it++) {
                        bleScanResult.AddManufacturerData(it->first, it->second);
                    }
                }

                bleScanResult.SetConnectable(result.GetPeripheralDevice().IsConnectable());

                if (result.GetPeripheralDevice().IsServiceUUID()) {
                    std::vector<Uuid> uuids = result.GetPeripheralDevice().GetServiceUUID();
                    for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
                        bleScanResult.AddServiceUuid(*iter);
                    }
                }

                if (result.GetPeripheralDevice().IsServiceData()) {
                    std::vector<Uuid> uuids = result.GetPeripheralDevice().GetServiceDataUUID();
                    int index = 0;
                    for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
                        bleScanResult.AddServiceData(*iter, result.GetPeripheralDevice().GetServiceData(index));
                        ++index;
                    }
                }

                bleScanResult.SetPeripheralDevice(result.GetPeripheralDevice().GetRawAddress());

                bleScanResult.SetPayload(std::string(result.GetPeripheralDevice().GetPayload(),
                result.GetPeripheralDevice().GetPayload() + result.GetPeripheralDevice().GetPayloadLen()));

                observer->OnScanCallback(bleScanResult);
            }
        });
    }

    void OnBleBatchScanResultsEvent(std::vector<BleScanResultImpl> &results) override
    {
        HILOGI("BleCentralManageCallback::OnBleBatchScanResultsEvent start.");

        observers_->ForEach([results](IBluetoothBleCentralManagerCallback *observer) {
            std::vector<BluetoothBleScanResult> bleScanResults;

            for (auto iter = results.begin(); iter != results.end(); iter++) {
                HILOGI("BleCentralManageCallback::OnScanCallback:Address= %{public}s",
                    iter->GetPeripheralDevice().GetRawAddress().GetAddress().c_str());

                BluetoothBleScanResult bleScanResult;

                if (iter->GetPeripheralDevice().IsRSSI()) {
                    bleScanResult.SetRssi(iter->GetPeripheralDevice().GetRSSI());
                }

                bleScanResult.SetAdvertiseFlag(iter->GetPeripheralDevice().GetAdFlag());

                if (iter->GetPeripheralDevice().IsManufacturerData()) {
                    std::map<uint16_t, std::string> manuData = iter->GetPeripheralDevice().GetManufacturerData();
                    for (auto manuDataIter = manuData.begin(); manuDataIter != manuData.end(); manuDataIter++) {
                        bleScanResult.AddManufacturerData(manuDataIter->first, manuDataIter->second);
                    }
                }

                bleScanResult.SetConnectable(iter->GetPeripheralDevice().IsConnectable());

                if (iter->GetPeripheralDevice().IsServiceUUID()) {
                    std::vector<Uuid> uuids = iter->GetPeripheralDevice().GetServiceUUID();
                    for (auto serviceUuidIter = uuids.begin(); serviceUuidIter != uuids.end(); serviceUuidIter++) {
                        bleScanResult.AddServiceUuid(*serviceUuidIter);
                    }
                }

                if (iter->GetPeripheralDevice().IsServiceData()) {
                    std::vector<Uuid> uuids = iter->GetPeripheralDevice().GetServiceDataUUID();
                    int index = 0;
                    for (auto serviceDataIter = uuids.begin(); serviceDataIter != uuids.end(); serviceDataIter++) {
                        bleScanResult.AddServiceData(
                            *serviceDataIter, iter->GetPeripheralDevice().GetServiceData(index));
                        ++index;
                    }
                }

                bleScanResult.SetPeripheralDevice(iter->GetPeripheralDevice().GetRawAddress());

                bleScanResult.SetPayload(std::string(iter->GetPeripheralDevice().GetPayload(),
                    iter->GetPeripheralDevice().GetPayload() + iter->GetPeripheralDevice().GetPayloadLen()));

                bleScanResults.push_back(bleScanResult);
            }
            observer->OnBleBatchScanResultsEvent(bleScanResults);
        });
    }

    void OnStartScanFailed(int resultCode) override
    {
        HILOGI("BleCentralManageCallback::OnStartScanFailed start.");

        pimpl_->eventHandler_->PostTask([=]() {
            pimpl_->bleService_ =
                static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
            if (pimpl_->bleService_ != nullptr && !pimpl_->bleService_->IsBtDiscovering()) {
                for (auto iter = pimpl_->scanCallbackInfo_.begin(); iter != pimpl_->scanCallbackInfo_.end(); ++iter) {
                    if (iter->isStart_) {
                        pimpl_->bleService_->StartScan(pimpl_->scanSettingImpl_);
                        return;
                    }
                }
            }
            observers_->ForEach([resultCode](IBluetoothBleCentralManagerCallback *observer) {
                observer->OnStartScanFailed(resultCode);
            });
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers)
    {
        observers_ = observers;
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
        pimpl_->bleService_ =
            static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
        switch (state) {
            case BTSystemState::ON:
                if (pimpl_->bleService_ != nullptr) {
                    pimpl_->bleService_->RegisterBleCentralManagerCallback(*pimpl_->observerImp_.get());
                }
                break;
            case BTSystemState::OFF:
                pimpl_->bleService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothBleCentralManagerServer::impl *pimpl_ = nullptr;
};

BluetoothBleCentralManagerServer::impl::impl()
{
    eventRunner_ = AppExecFwk::EventRunner::Create("bt central manager server");
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner_);
    isScanning = false;
}

BluetoothBleCentralManagerServer::impl::~impl()
{
    bleService_ = static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
    if (bleService_ != nullptr) {
        bleService_->DeregisterBleCentralManagerCallback();
    }
}

BluetoothBleCentralManagerServer::BluetoothBleCentralManagerServer()
{
    pimpl = std::make_unique<impl>();

    pimpl->eventHandler_->PostSyncTask(
        [&]() {
            pimpl->observerImp_->SetObserver(&(pimpl->observers_));
            pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
            IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

            pimpl->bleService_ =
                static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
            if (pimpl->bleService_ != nullptr) {
                pimpl->bleService_->RegisterBleCentralManagerCallback(*pimpl->observerImp_.get());
            }
        },
        AppExecFwk::EventQueue::Priority::HIGH);
}

BluetoothBleCentralManagerServer::~BluetoothBleCentralManagerServer()
{
    pimpl->eventHandler_->PostSyncTask(
        [&]() { IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_)); },
        AppExecFwk::EventQueue::Priority::HIGH);
}

void BluetoothBleCentralManagerServer::StartScan()
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGI("BluetoothBleCentralManagerServer::StartScan start. pid:%{public}d uid:%{public}d", pid, uid);
    if (PermissionUtils::VerifyDiscoverBluetoothPermission() == PERMISSION_DENIED ||
        PermissionUtils::VerifyManageBluetoothPermission() == PERMISSION_DENIED ||
        PermissionUtils::VerifyLocationPermission() == PERMISSION_DENIED) {
        HILOGE("StartScan error, check permission failed");
        return;
    }

    pimpl->eventHandler_->PostSyncTask([&]() {
        pimpl->bleService_ =
            static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

        for (auto iter = pimpl->scanCallbackInfo_.begin(); iter != pimpl->scanCallbackInfo_.end(); ++iter) {
            HILOGI("BluetoothBleCentralManagerServer::StartScan baomin. iter->pid_:%{public}d iter->uid_:%{public}d", iter->pid_, iter->uid_);
            if (iter->pid_ == pid && iter->uid_ == uid) {
                iter->isStart_ = true;
            }
        }

        if (pimpl->bleService_ != nullptr) {
            HILOGI("BluetoothBleCentralManagerServer::StartScan baomin. pimpl->isScanning:%{public}d", pimpl->isScanning);
            if (!pimpl->isScanning) {
                pimpl->bleService_->StartScan();
            }
        } else {
            HILOGI("BluetoothBleCentralManagerServer::StartScan baomin. pimpl->bleService_ is nullptr");
        }
        pimpl->isScanning = true;
    });
}

void BluetoothBleCentralManagerServer::StartScan(const BluetoothBleScanSettings &settings)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGI("BluetoothBleCentralManagerServer::StartScan with settings start. pid:%{public}d uid:%{public}d", pid, uid);
    if (PermissionUtils::VerifyDiscoverBluetoothPermission() == PERMISSION_DENIED ||
        PermissionUtils::VerifyManageBluetoothPermission() == PERMISSION_DENIED ||
        PermissionUtils::VerifyLocationPermission() == PERMISSION_DENIED) {
        HILOGE("StartScan error, check permission failed");
        return;
    }

    pimpl->eventHandler_->PostSyncTask([&]() {
        pimpl->bleService_ =
            static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

        for (auto iter = pimpl->scanCallbackInfo_.begin(); iter != pimpl->scanCallbackInfo_.end(); ++iter) {
            if (iter->pid_ == pid && iter->uid_ == uid) {
                iter->isStart_ = true;
            }
        }

        if (pimpl->bleService_ != nullptr) {
            if (!pimpl->isScanning) {
                pimpl->scanSettingImpl_.SetReportDelay(settings.GetReportDelayMillisValue());
                pimpl->scanSettingImpl_.SetScanMode(settings.GetScanMode());
                pimpl->scanSettingImpl_.SetLegacy(settings.GetLegacy());
                pimpl->scanSettingImpl_.SetPhy(settings.GetPhy());
                pimpl->bleService_->StartScan(pimpl->scanSettingImpl_);
                pimpl->isScanning = true;
            } else if (pimpl->scanSettingImpl_.GetReportDelayMillisValue() != settings.GetReportDelayMillisValue() ||
                       pimpl->scanSettingImpl_.GetScanMode() != settings.GetScanMode() ||
                       pimpl->scanSettingImpl_.GetLegacy() != settings.GetLegacy() ||
                       pimpl->scanSettingImpl_.GetPhy() != settings.GetPhy()) {
                pimpl->scanSettingImpl_.SetReportDelay(settings.GetReportDelayMillisValue());
                pimpl->scanSettingImpl_.SetScanMode(settings.GetScanMode());
                pimpl->scanSettingImpl_.SetLegacy(settings.GetLegacy());
                pimpl->scanSettingImpl_.SetPhy(settings.GetPhy());
                pimpl->bleService_->StopScan();
                pimpl->isScanning = false;
            }
        }
    });
}

void BluetoothBleCentralManagerServer::StopScan()
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGI("BluetoothBleCentralManagerServer::StopScan start. pid:%{public}d uid:%{public}d", pid, uid);
    if (PermissionUtils::VerifyDiscoverBluetoothPermission() == PERMISSION_DENIED) {
        HILOGE("StopScan error, check permission failed");
        return;
    }

    pimpl->eventHandler_->PostSyncTask([&]() {
        pimpl->bleService_ =
            static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

        bool doStop = true;
        for (auto iter = pimpl->scanCallbackInfo_.begin(); iter != pimpl->scanCallbackInfo_.end(); ++iter) {
            if (iter->pid_ == pid && iter->uid_ == uid) {
                iter->isStart_ = false;
            }
            doStop = iter->isStart_ ? false : doStop;
        }

        if (pimpl->bleService_ != nullptr) {
            if (doStop) {
                pimpl->bleService_->StopScan();
                pimpl->isScanning = false;
            }
        }
    });
}

int BluetoothBleCentralManagerServer::ConfigScanFilter(
    const int clientId, const std::vector<BluetoothBleScanFilter> &filters)
{
    HILOGI("BluetoothBleCentralManagerServer::ConfigScanFilter start.");

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    if (pimpl->bleService_ != nullptr) {
        std::vector<BleScanFilterImpl> filterImpls {};
        for (auto filter : filters) {
            BleScanFilterImpl filterImpl;
            filterImpl.SetDeviceId(filter.GetDeviceId());
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
            filterImpl.SetServiceData(filter.GetServiceData());
            filterImpl.SetServiceDataMask(filter.GetServiceDataMask());
            filterImpl.SetManufacturerId(filter.GetManufacturerId());
            filterImpl.SetManufactureData(filter.GetManufactureData());
            filterImpl.SetManufactureDataMask(filter.GetManufactureDataMask());
            filterImpls.push_back(filterImpl);
        }
        return pimpl->bleService_->ConfigScanFilter(clientId, filterImpls);
    }
    return 0;
}

void BluetoothBleCentralManagerServer::RemoveScanFilter(const int clientId)
{
    HILOGI("BluetoothBleCentralManagerServer::RemoveScanFilter start.");

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    if (pimpl->bleService_ != nullptr) {
        pimpl->bleService_->RemoveScanFilter(clientId);
    }
}

void BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    HILOGI("BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback start. pid:%{public}d uid:%{public}d",
        pid,
        uid);

    if (callback == nullptr) {
        HILOGI("BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback called with NULL binder. "
               "Ignoring.");
        return;
    }

    pimpl->eventHandler_->PostSyncTask([&]() {
        if (pimpl != nullptr) {
            pimpl->observersToken_[callback->AsObject()] = IPCSkeleton::GetCallingTokenID();
            pimpl->observers_.Register(callback);
            impl::ScanCallbackInfo info;
            info.pid_ = pid;
            info.uid_ = uid;
            info.isStart_ = false;
            info.callback_ = callback;
            pimpl->scanCallbackInfo_.push_back(info);
        }
    });
}

void BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    HILOGI("BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback start.");
    pimpl->eventHandler_->PostSyncTask([&]() {
        if (callback == nullptr || pimpl == nullptr) {
            HILOGI("BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback called with NULL binder. "
                   "Ignoring.");
            return;
        }
        for (auto iter = pimpl->scanCallbackInfo_.begin(); iter != pimpl->scanCallbackInfo_.end(); ++iter) {
            if (iter->callback_->AsObject() == callback->AsObject()) {
                pimpl->observers_.Deregister(iter->callback_);
                pimpl->scanCallbackInfo_.erase(iter);
                break;
            }
        }
        for (auto iter =  pimpl->observersToken_.begin(); iter !=  pimpl->observersToken_.end(); ++iter) {
            if (iter->first == callback->AsObject()) {
                pimpl->observersToken_.erase(iter);
                break;
            }
        }
    });
}
}  // namespace Bluetooth
}  // namespace OHOS