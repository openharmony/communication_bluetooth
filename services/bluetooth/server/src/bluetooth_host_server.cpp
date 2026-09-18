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
#define LOG_TAG "bt_server_host"
#endif

#include "bluetooth_host_server.h"

#include "file_ex.h"
#include "hisysevent.h"
#include "system_ability_definition.h"

#include <set>

#include "adapter_device_config.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "bluetooth_a2dp_sink_server.h"
#include "bluetooth_a2dp_source_server.h"
#ifdef BLUETOOTH_AVRCP_CT_FEATURE_ENABLE
#include "bluetooth_avrcp_ct_server.h"
#endif
#include "bluetooth_avrcp_tg_server.h"
#include "bluetooth_ble_advertiser_server.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_state_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_gatt_server_server.h"
#include "bluetooth_hfp_ag_server.h"
#ifdef BLUETOOTH_HFP_HF_ENABLE
#include "bluetooth_hfp_hf_server.h"
#endif
#include "bluetooth_hid_host_server.h"
#include "bluetooth_host_dumper.h"
#include "bluetooth_audio_manager_server.h"
#include "bluetooth_map_mse_server.h"
#include "bluetooth_observer_application_container.h"
#ifdef BLUETOOTH_PAN_FEATURE_ENABLE
#include "bluetooth_pan_server.h"
#endif
#include "bluetooth_socket_server.h"
#ifdef BLUETOOTH_PBAP_PSE_FEATURE_ENABLE
#include "bluetooth_pbap_pse_server.h"
#endif
#ifdef BLUETOOTH_OPP_FEATURE_ENABLE
#include "bluetooth_opp_server.h"
#endif
#ifdef BLUETOOTH_HEARINGAID_FEATURE_ENABLE
#include "bluetooth_hearingaid_server.h"
#endif
#ifdef BLUETOOTH_HIDDEVICE_FEATURE_ENABLE
#include "bluetooth_hid_device_server.h"
#endif
#include "bluetooth_utils_server.h"
#include "classic_config.h"
#include "common_util.h"
#include "interface_adapter_manager.h"
#include "interface_profile_resource_manager.h"

#include "interface_adapter_ble.h"
#include "interface_adapter_classic.h"
#include "interface_profile_manager.h"
#include "ipc_skeleton.h"
#include "../../common/log.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "remote_observer_list.h"
#include "preferences_manager.h"
#include "safe_map.h"
#include "string_ex.h"
#include "hitrace_meter.h"
#include "log_utils.h"
#include "xcollie/watchdog.h"
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
#include "bluetooth_bas_server.h"
#endif

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

static constexpr int32_t CONNECT_ALL_TIMEOUT = 30000; // 30s
static constexpr int32_t AUDIO_SERVER_UID = 1041;
const std::string CALLING_NAME_RESOURCE_SCHEDULE = "resource_schedule_service";
static int64_t GetNowTimestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return timestamp;
}

struct BluetoothHostServer::impl {
    impl();
    ~impl();
    void Init();
    void Clear();
    void SetDeviceConnectable(const bluetooth::BTStateID state);
    void LoadAppTypeCache();
    std::string GetAppTypeFromCache(const std::string &key, const std::string &defaultValue);

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    /// adapter state observer
    class AdapterStateObserver;
    std::unique_ptr<AdapterStateObserver> observerImp_ = nullptr;

    /// classic observer
    class AdapterClassicObserver;
    std::unique_ptr<AdapterClassicObserver> classicObserverImp_ = nullptr;

    /// classic remote device observer
    class ClassicRemoteDeviceObserver;
    std::unique_ptr<ClassicRemoteDeviceObserver> remoteObserverImp_ = nullptr;

    /// ble observer
    class AdapterBleObserver;
    std::unique_ptr<AdapterBleObserver> bleObserverImp_ = nullptr;

    /// ble remote device observer
    class BlePeripheralCallback;
    std::unique_ptr<BlePeripheralCallback> bleRemoteObserverImp_ = nullptr;

    /// resource manager observer
    class BluetoothResourceMgrObserver;
    std::shared_ptr<IResourceManagerObserver> resourceMgrObserverImp_ = nullptr;

    /// user regist observers
    RemoteObserverList<IBluetoothHostObserver> observers_;
    RemoteObserverList<IBluetoothHostObserver> bleObservers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    std::shared_ptr<BluetoothObserverApplicationContainer> bleAppContainer_ = nullptr;

    /// user regist remote observers
    RemoteObserverList<IBluetoothRemoteDeviceObserver> remoteObservers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> remoteAppContainer_ = nullptr;

    /// user regist remote observers
    RemoteObserverList<IBluetoothBlePeripheralObserver> bleRemoteObservers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> bleRemoteAppContainer_ = nullptr;

    /// user regist remote observers
    RemoteObserverList<IBluetoothResourceManagerObserver> resourceMgrObservers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> resourceMgrAppContainer_ = nullptr;

    std::map<std::string, sptr<IRemoteObject>> servers_;
    std::map<std::string, sptr<IRemoteObject>> bleServers_;

    sptr<BluetoothBleCentralManagerServer> bleCentralManger_;

    /// user regist oob observers
    std::mutex oobObserverMutex_;
    sptr<IBluetoothOobObserver> oobObserver_ = nullptr;
    std::mutex pairRecordMutex_;
    PairRecord pairRecord_;
    SafeMap<std::string, PairRecord> pairRecords_;
    SafeMap<std::string, std::string> appTypeCache_;

#ifdef FUSION_CONNECTIVITY_SUPPORTED
    class FusionConnectivityObserverDeathRecipient;
    std::mutex fusionConnectivityObserverMutex_ {};
    sptr<FusionConnectivityObserverDeathRecipient> fusionConnectivityObserverDeathRecipient_ = nullptr;
    sptr<IBluetoothHostObserver> fusionConnectivityObserver_ = nullptr;
#endif  // FUSION_CONNECTIVITY_SUPPORTED

    /// BLE-owner-only mode: server-side cache of the owner pid set,
    /// kept in sync with BluetoothSwitchStateMachine (state machine wins)
    std::mutex ownerPidCacheMutex_ {};
    std::set<int32_t> ownerPidCache_ {};

private:
    void createServers();
};

class BluetoothHostServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const BTSystemState state) override
    {
        if (!impl_) {
            HILOGI("failed: impl_ is null");
            return;
        }
        auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
        auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
        IBluetoothResourceManager *resourceMgrService = nullptr;
#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
        resourceMgrService = IBluetoothResourceManager::GetInstance();
#endif
        switch (state) {
            case BTSystemState::ON:
                if (classicService) {
                    classicService->RegisterClassicAdapterObserver(
                        *reinterpret_cast<IAdapterClassicObserver *>(impl_->classicObserverImp_.get()));
                    classicService->RegisterRemoteDeviceObserver(
                        *reinterpret_cast<IClassicRemoteDeviceObserver *>(impl_->remoteObserverImp_.get()));
                }
                if (bleService) {
                    bleService->RegisterBleAdapterObserver(
                        *reinterpret_cast<IAdapterBleObserver *>(impl_->bleObserverImp_.get()));
                    bleService->RegisterBlePeripheralCallback(
                        *reinterpret_cast<IBlePeripheralCallback *>(impl_->bleRemoteObserverImp_.get()));
                }
                if (resourceMgrService) {
                    resourceMgrService->RegisterObserver(impl_->resourceMgrObserverImp_);
                }
                break;

            case BTSystemState::OFF:
                if (classicService) {
                    classicService->DeregisterClassicAdapterObserver(
                        *reinterpret_cast<IAdapterClassicObserver *>(impl_->classicObserverImp_.get()));
                    classicService->DeregisterRemoteDeviceObserver(
                        *reinterpret_cast<IClassicRemoteDeviceObserver *>(impl_->remoteObserverImp_.get()));
                }
                if (bleService) {
                    bleService->DeregisterBleAdapterObserver(
                        *reinterpret_cast<IAdapterBleObserver *>(impl_->bleObserverImp_.get()));
                    bleService->DeregisterBlePeripheralCallback(
                        *reinterpret_cast<IBlePeripheralCallback *>(impl_->bleRemoteObserverImp_.get()));
                }
                if (resourceMgrService) {
                    resourceMgrService->DeregisterObserver(impl_->resourceMgrObserverImp_);
                }
                break;
            default:
                break;
        }
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
};

class BluetoothHostServer::impl::AdapterStateObserver : public IAdapterStateObserver {
public:
    explicit AdapterStateObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterStateObserver() override = default;

    void OnStateChange(const bluetooth::BTTransport transport, const BTStateID state) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        CHECK_AND_RETURN_LOG(impl_, "impl_ is nullptr");
        if (transport == BTTransport::ADAPTER_BREDR) {
            impl_->SetDeviceConnectable(state);
            impl_->observers_.ForEach([this, transport, state](sptr<IBluetoothHostObserver> observer) {
                int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
                uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
                if (IAdapterManager::GetInstance()->IsBluetoothRestricted() &&
                    (!PermissionManager::IsSystemHap(tokenId)) && (!PermissionManager::IsNativeCaller(tokenId))) {
                    if (state == BTStateID::STATE_TURN_OFF) {
                        HILOGD("During Bluetooth Bluetooth restricted state, notify third-party app BLE TURN OFF");
                        observer->OnStateChanged(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
                    }
                } else {
                    observer->OnStateChanged(transport, state);
                }
            });
            if (state == BTStateID::STATE_TURN_ON) {
                impl_->LoadAppTypeCache();
            }
            if (state == BTStateID::STATE_TURN_ON || state == BTStateID::STATE_TURN_OFF) {
                int32_t pid = IPCSkeleton::GetCallingPid();
                int32_t uid = IPCSkeleton::GetCallingUid();
                HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "BR_SWITCH_STATE",
                    HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid, "STATE", state);
            }
        } else if (transport == BTTransport::ADAPTER_BLE) {
            impl_->bleObservers_.ForEach([this, transport, state](sptr<IBluetoothHostObserver> observer) {
                int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
                uint64_t tokenId = this->impl_->bleAppContainer_->GetTokenId(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                    return;
                }
                observer->OnStateChanged(transport, state);
            });
            if (state == BTStateID::STATE_TURN_ON || state == BTStateID::STATE_TURN_OFF) {
                int32_t pid = IPCSkeleton::GetCallingPid();
                int32_t uid = IPCSkeleton::GetCallingUid();
                HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "BLE_SWITCH_STATE",
                    HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid, "STATE", state);
            }
        }
    }

    void OnBluetoothStateChanged(int state) override
    {
        if (!impl_) {
            return;
        }
        impl_->observers_.ForEach([this, state](sptr<IBluetoothHostObserver> observer) {
            int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            observer->OnBluetoothStateChanged(state);
        });
    }

    void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) override
    {
        if (!impl_) {
            return;
        }
        impl_->observers_.ForEach([this, pid, prohibitedSecondsTime](sptr<IBluetoothHostObserver> observer) {
            int32_t remotePid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
            if (remotePid == pid) {
                observer->OnRefusePolicyChanged(pid, prohibitedSecondsTime);
            }
        });
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterStateObserver);
};

class BluetoothHostServer::impl::AdapterClassicObserver : public IAdapterClassicObserver {
public:
    explicit AdapterClassicObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterClassicObserver() override = default;

    bool CheckDiscoverOrAccessPermission(uint64_t tokenId)
    {
        if (PermissionManager::IsNativeCaller(tokenId) ||
            PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
            if (!PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId)) {
                HILOGI_TIME_LIMIT(__func__,
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
                return false;
            }
        } else {
            CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH, tokenId), false,
                "[PERMISSION] check DISCOVER_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
        }
        return true;
    }

    void OnDiscoveryStateChanged(const int32_t status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("status: %{public}d", status);
        impl_->observers_.ForEach([this, status](sptr<IBluetoothHostObserver> observer) {
            int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            observer->OnDiscoveryStateChanged(static_cast<int32_t>(status));
        });
        if (status == DISCOVERY_STOPED) {
            int32_t pid = IPCSkeleton::GetCallingPid();
            int32_t uid = IPCSkeleton::GetCallingUid();
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "DISCOVERY_STATE",
                HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid, "STATE", DISCOVERY_STOPED);
        }
    }

    void OnDiscoveryResult(
        const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("device: %{public}s, rssi: %{public}d", GET_ENCRYPT_ADDR(device), rssi);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        std::ostringstream oss;
        impl_->observers_.ForEach([this, device, randomAddr, rssi, deviceName, deviceClass, &oss](
            IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (!CheckDiscoverOrAccessPermission(tokenId)) {
                return;
            }
            HILOGI_TIME_LIMIT(device.GetAddress(),
                "BR OnDiscoveryResult: device: %{public}s, rssi: %{public}d", GET_ENCRYPT_ADDR(device), rssi);
            oss << " " << pid;
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnDiscoveryResult(reportAddr, rssi, deviceName, deviceClass);
        });
        std::string outVal = oss.str();
        HILOGI_TIME_LIMIT(outVal, "BR OnDiscoveryResult appPid:%{public}s", outVal.c_str());
    }

    void OnPairRequested(const BTTransport transport, const RawAddress &device) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
        impl_->observers_.ForEach([this, transport, device](IBluetoothHostObserver *observer) {
            uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnPairRequested(transport, randomAddr);
            }
        });
    }

    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, int32_t reqType, int32_t number) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, reqType: %{public}d, number: %{public}s",
            GET_ENCRYPT_ADDR(device), reqType, GetEncryptPinCode(number).c_str());
        impl_->observers_.ForEach([this, transport, device, reqType, number](IBluetoothHostObserver *observer) {
            uint64_t tokenId = this->impl_->appContainer_->GetTokenId(observer->AsObject());
            if (!CheckDiscoverOrAccessPermission(tokenId)) {
                return;
            }

            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnPairConfirmed(transport, randomAddr, reqType, number);
            }
        });
    }

    void OnScanModeChanged(int32_t mode) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("mode: %{public}d", mode);
        impl_->observers_.ForEach([this, mode](IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            observer->OnScanModeChanged(mode);
        });
    }

    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("deviceName: %{public}s", GET_ENCRYPT_DEVICE_NAME(deviceName));
        impl_->observers_.ForEach(
            [this, deviceName](IBluetoothHostObserver *observer) {
                int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    return;
                }
                observer->OnDeviceNameChanged(deviceName);
            });
    }

    void OnDeviceAddrChanged(const std::string &address) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
        impl_->observers_.ForEach([this, address](IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            observer->OnDeviceAddrChanged(address);
        });
    }

    void OnGenerateLocalOobData(int32_t ret, const Bluetooth::BluetoothOobData &oobData) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("OnGenerateLocalOobData ret: %{public}d", ret);
        {
            std::lock_guard<std::mutex> lock(impl_->oobObserverMutex_);
            if (impl_->oobObserver_ == nullptr) {
                HILOGE("oobObserver_ is null");
                return;
            }
            impl_->oobObserver_->OnGenerateLocalOobData(ret, oobData);
            impl_->oobObserver_ = nullptr;
        }
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterClassicObserver);
};

class BluetoothHostServer::impl::ClassicRemoteDeviceObserver : public IClassicRemoteDeviceObserver {
public:
    explicit ClassicRemoteDeviceObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~ClassicRemoteDeviceObserver() override = default;

    bool CheckDiscoverOrAccessPermission(uint64_t tokenId)
    {
        if (PermissionManager::IsNativeCaller(tokenId) ||
            PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
            CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId), false,
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
        } else {
            CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH, tokenId), false,
                "[PERMISSION] check DISCOVER_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
        }
        return true;
    }

    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("device: %{public}s, state: %{public}d, reason: %{public}u", GET_ENCRYPT_ADDR(device), state, reason);
        RawAddress randomAddr;
        if (BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr) == RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        impl_->remoteObservers_.ForEach([this, device, randomAddr, state, reason
            ](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(
                pid, BluetoothBleCentralManagerServer::WakeType::ACL_STATE_WAKE_TYPE)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnAclStateChanged(reportAddr, state, reason);
        });
    }

    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOG_COMM_INFO("OnPairStatusChanged: %{public}s, status: %{public}d, cause: %{public}d",
            GET_ENCRYPT_ADDR(device), status, cause);
        RawAddress randomAddr;
        if (BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr) == RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        impl_->remoteObservers_.ForEach([this,
            transport, device, randomAddr, status, cause, causeMessage](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            CHECK_AND_RETURN_LOG(CheckDiscoverOrAccessPermission(tokenId), "Permission failed");
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnPairStatusChanged(transport, reportAddr, status, cause, causeMessage);
        });
        if (status == PAIR_PAIRED) {
            impl_->pairRecords_.EnsureInsert(device.GetAddress(), impl_->pairRecord_);
            std::lock_guard<std::mutex> lock(impl_->pairRecordMutex_);
            impl_->pairRecord_ = PairRecord{};
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device, true);
            OHOS::bluetooth::ClassicConfig::GetInstance().SetRemoteRandomAddr(device.GetAddress(),
                randomAddr.GetAddress());
        }
        if (status == PAIR_NONE) {
            impl_->pairRecords_.Erase(device.GetAddress());
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device, false);
            OHOS::bluetooth::ClassicConfig::GetInstance().RemovePairedDevice(device.GetAddress());
            HILOGI("start to remove virtual device");
            auto a2dpRemote = BluetoothHostServer::GetInstance()->GetProfile(PROFILE_A2DP_SRC);
            if (a2dpRemote != nullptr) {
                sptr<IBluetoothA2dpSrc> a2dpSource = iface_cast<IBluetoothA2dpSrc>(a2dpRemote);
                if (a2dpSource != nullptr) {
                    a2dpSource->UpdateVirtualDevice(1, device.GetAddress());  // 1表示上线
                }
            }
            auto hfpRemote = BluetoothHostServer::GetInstance()->GetProfile(PROFILE_HFP_AG);
            if (hfpRemote != nullptr) {
                sptr<IBluetoothHfpAg> hfpAg = iface_cast<IBluetoothHfpAg>(hfpRemote);
                if (hfpAg != nullptr) {
                    hfpAg->UpdateVirtualDevice(1, device.GetAddress());  // 1表示上线
                }
            }
        }
    }

    void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
        std::vector<bluetooth::Uuid> btUuids;
        for (const auto &val : uuids) {
            btUuids.push_back(val);
        }
        impl_->remoteObservers_.ForEach([this, device, btUuids](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteUuidChanged(randomAddr, btUuids);
            }
        });
    }

    void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("device: %{public}s", GET_ENCRYPT_ADDR(device));
        impl_->remoteObservers_.ForEach([this, device, deviceName](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteNameChanged(randomAddr, deviceName);
            }
        });
    }

    void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, alias: %{public}s", GET_ENCRYPT_ADDR(device), GET_ENCRYPT_DEVICE_NAME(alias));
        impl_->remoteObservers_.ForEach([this, device, alias](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteAliasChanged(randomAddr, alias);
            }
        });
    }

    void OnRemoteCodChanged(const RawAddress &device, int cod) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, cod: %{public}d", GET_ENCRYPT_ADDR(device), cod);
        impl_->remoteObservers_.ForEach([this, device, cod](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteCodChanged(randomAddr, cod);
            }
        });
    }

    void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
        BluetoothBatteryInfo info(batteryInfo);
        impl_->remoteObservers_.ForEach([this, device, info](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteBatteryChanged(randomAddr, info);
            }
        });
    }

    void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, dataLen: %{public}lu", GET_ENCRYPT_ADDR(device), value.size());
        impl_->remoteObservers_.ForEach([this, device, value](IBluetoothRemoteDeviceObserver *observer) {
            int32_t pid = this->impl_->remoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->remoteAppContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnRemoteDeviceCommonInfoReport(randomAddr, value);
            }
        });
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(ClassicRemoteDeviceObserver);
};

class BluetoothHostServer::impl::AdapterBleObserver : public IAdapterBleObserver {
public:
    explicit AdapterBleObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterBleObserver() override = default;

    bool CheckDiscoverOrAccessPermission(uint64_t tokenId)
    {
        if (PermissionManager::IsNativeCaller(tokenId) ||
            PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
            if (!PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId)) {
                HILOGI_TIME_LIMIT(__func__,
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
                return false;
            }
        } else {
            CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH, tokenId), false,
                "[PERMISSION] check DISCOVER_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
        }
        return true;
    }

    void OnDiscoveryStateChanged(const int32_t status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("status: %{public}d", status);
        impl_->bleObservers_.ForEach([this, status](sptr<IBluetoothHostObserver> observer) {
            int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            observer->OnDiscoveryStateChanged(static_cast<int32_t>(status));
        });
    }

    void OnDiscoveryResult(
        const RawAddress &device,  int rssi, const std::string deviceName, int deviceClass) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("device: %{public}s, rssi: %{public}d", GET_ENCRYPT_ADDR(device), rssi);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        std::ostringstream oss;
        impl_->bleObservers_.ForEach([this, device, randomAddr, rssi, deviceName, deviceClass, &oss](
            IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->bleAppContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (!CheckDiscoverOrAccessPermission(tokenId)) {
                return;
            }
            HILOGI_TIME_LIMIT(device.GetAddress(),
                "BLE OnDiscoveryResult: device: %{public}s, rssi: %{public}d, deviceName: %{public}s",
                GET_ENCRYPT_ADDR(device), rssi, GET_ENCRYPT_DEVICE_NAME(deviceName));
            oss << " " << pid;
#ifdef MMI_NOT_DISCOVER_BLE
            if (PermissionManager::IsInMmiTmpWhiteList(tokenId)) {
                HILOGW("not send ble discovery result to MMI");
                return;
            }
#endif
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnDiscoveryResult(reportAddr, rssi, deviceName, deviceClass);
        });
        std::string outVal = oss.str();
        HILOGI_TIME_LIMIT(outVal, "BLE OnDiscoveryResult appPid:%{public}s", outVal.c_str());
    }

    void OnPairRequested(const BTTransport transport, const RawAddress &device) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
        impl_->bleObservers_.ForEach(
            [this, transport, device](IBluetoothHostObserver *observer) {
                uint64_t tokenId = this->impl_->bleAppContainer_->GetTokenId(observer->AsObject());
                RawAddress randomAddr;
                if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                    observer->OnPairRequested(transport, randomAddr);
                }
        });
    }

    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, const int32_t reqType, const int32_t number) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, reqType: %{public}d, number: %{public}s",
            GET_ENCRYPT_ADDR(device), reqType, GetEncryptPinCode(number).c_str());
        impl_->bleObservers_.ForEach([this, transport, device, reqType, number](IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            uint64_t tokenId = this->impl_->bleAppContainer_->GetTokenId(observer->AsObject());
            if (!CheckDiscoverOrAccessPermission(tokenId)) {
                return;
            }
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnPairConfirmed(transport, randomAddr, reqType, number);
            }
        });
    }

    void OnScanModeChanged(const int32_t mode) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("mode: %{public}d", mode);
        impl_->bleObservers_.ForEach([this, mode](IBluetoothHostObserver *observer) {
            int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            observer->OnScanModeChanged(mode);
        });
    }

    void OnDeviceNameChanged(const std::string deviceName) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        impl_->bleObservers_.ForEach(
            [this, deviceName](IBluetoothHostObserver *observer) {
                int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    return;
                }
                observer->OnDeviceNameChanged(deviceName);
            });
    }

    void OnDeviceAddrChanged(const std::string address) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
        impl_->bleObservers_.ForEach([this, address](IBluetoothHostObserver *observer) {
                int32_t pid = this->impl_->bleAppContainer_->GetRemotePid(observer->AsObject());
                if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                    return;
                }
                observer->OnDeviceAddrChanged(address);
            });
    }

    void OnAdvertisingStateChanged(const int32_t state) override
    {}

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterBleObserver);
};

class BluetoothHostServer::impl::BlePeripheralCallback : public IBlePeripheralCallback {
public:
    explicit BlePeripheralCallback(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~BlePeripheralCallback() override = default;

    void OnReadRemoteRssiEvent(const RawAddress &device, int rssi, int status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, rssi: %{public}d, status: %{public}d", GET_ENCRYPT_ADDR(device), rssi, status);
        impl_->bleRemoteObservers_.ForEach([this, device, rssi, status](IBluetoothBlePeripheralObserver *observer) {
            int32_t pid = this->impl_->bleRemoteAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            uint64_t tokenId = this->impl_->bleRemoteAppContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnReadRemoteRssiEvent(randomAddr, rssi, status);
            }
        });
    }

    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device: %{public}s, status: %{public}d, cause: %{public}d",
            GET_ENCRYPT_ADDR(device), status, cause);
        RawAddress randomAddr;
        if (BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr) == RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        impl_->bleRemoteObservers_.ForEach([this, transport, device, randomAddr, status, cause, causeMessage](
            IBluetoothBlePeripheralObserver *observer) {
            uint64_t tokenId = this->impl_->bleRemoteAppContainer_->GetTokenId(observer->AsObject());
            if (PermissionManager::IsNativeCaller(tokenId) ||
                PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            } else {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(USE_BLUETOOTH, tokenId),
                    "[PERMISSION] check USE_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            }
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnPairStatusChanged(transport, reportAddr, status, cause, causeMessage);
        });

        if (status == PAIR_PAIRED) {
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device, true);
            OHOS::bluetooth::ClassicConfig::GetInstance().SetRemoteRandomAddr(device.GetAddress(),
                randomAddr.GetAddress());
        }
        if (status == PAIR_NONE) {
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device, false);
            OHOS::bluetooth::ClassicConfig::GetInstance().RemovePairedDevice(device.GetAddress());
        }
    }

    void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("device: %{public}s, state: %{public}d, reason: %{public}u", GET_ENCRYPT_ADDR(device), state, reason);
        RawAddress randomAddr;
        if (BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr) == RET_NO_EXIST) {
            randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device);
        }
        if (state == BLE_CONNECTION_STATE_DISCONNECTED) {
            BluetoothStateManager::GetInstance()->RemoveClientDeviceList(device.GetAddress());
            BluetoothStateManager::GetInstance()->RemoveServerDeviceList(device.GetAddress());
        }
        impl_->bleRemoteObservers_.ForEach([this, device, randomAddr, state, reason
            ](IBluetoothBlePeripheralObserver *observer) {
            int32_t pid = this->impl_->bleRemoteAppContainer_->GetRemotePid(observer->AsObject());
            uint64_t tokenId = this->impl_->bleRemoteAppContainer_->GetTokenId(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(
                pid, BluetoothBleCentralManagerServer::WakeType::ACL_STATE_WAKE_TYPE)) {
                HILOGI("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            auto reportAddr = PermissionManager::IsUseRealAddr(tokenId) ? device : randomAddr;
            observer->OnAclStateChanged(reportAddr, state, reason);
        });
        if (state == BLE_CONNECTION_STATE_DISCONNECTED && pairStatus == PAIR_NONE) {
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device, false);
        }
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BlePeripheralCallback);
};

class BluetoothHostServer::impl::BluetoothResourceMgrObserver : public IResourceManagerObserver {
public:
    explicit BluetoothResourceMgrObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~BluetoothResourceMgrObserver() override = default;

    bool CheckResourceManagerPermission(uint64_t tokenId)
    {
        std::string callingName = PermissionManager::GetCallingName(tokenId);
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId), false,
            "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)", callingName.c_str());
        if (PermissionManager::GetCallingName(tokenId) != COMM_PROTOCOL_RESOURCE_MANAGER) {
            HILOGD("[PERMISSION] check callingName failed, callingName(%{public}s)", callingName.c_str());
            return false;
        }
        return true;
    }

    void OnSensingStateChanged(uint8_t eventId, const SensingInfo &info) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("eventId: %{public}d", eventId);
        impl_->resourceMgrObservers_.ForEach([this, eventId, info](IBluetoothResourceManagerObserver *observer) {
            int32_t pid = this->impl_->resourceMgrAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            uint64_t tokenId = this->impl_->resourceMgrAppContainer_->GetTokenId(observer->AsObject());
            if (!CheckResourceManagerPermission(tokenId)) {
                return;
            }
            observer->OnSensingStateChanged(eventId, BluetoothSensingInfo(info));
        });
    }

    void OnBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("eventId: %{public}d, result: %{public}d", eventId, result);
        impl_->resourceMgrObservers_.ForEach([this,
            eventId, info, &result](IBluetoothResourceManagerObserver *observer) {
            int32_t pid = this->impl_->resourceMgrAppContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                return;
            }
            uint64_t tokenId = this->impl_->resourceMgrAppContainer_->GetTokenId(observer->AsObject());
            if (!CheckResourceManagerPermission(tokenId)) {
                return;
            }
            observer->OnBluetoothResourceDecision(eventId, BluetoothSensingInfo(info), result);
        });
    }

private:
    const std::string COMM_PROTOCOL_RESOURCE_MANAGER = "comm_protocol_resource_manager";
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothResourceMgrObserver);
};

#ifdef FUSION_CONNECTIVITY_SUPPORTED
class BluetoothHostServer::impl::FusionConnectivityObserverDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    FusionConnectivityObserverDeathRecipient(BluetoothHostServer::impl *owner) : owner_(owner) {}
 
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        HILOGI("FusionConnectivity remoteDied");
        if (owner_) {
            std::lock_guard<std::mutex> lock(owner_->fusionConnectivityObserverMutex_);
            owner_->fusionConnectivityObserver_ = nullptr;
        }
    }
 
private:
    BluetoothHostServer::impl *owner_ {};
};
#endif

std::mutex BluetoothHostServer::instanceLock;
sptr<BluetoothHostServer> BluetoothHostServer::instance;

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(BluetoothHostServer::GetInstance().GetRefPtr());

BluetoothHostServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    observerImp_ = std::make_unique<AdapterStateObserver>(this);
    classicObserverImp_ = std::make_unique<AdapterClassicObserver>(this);
    remoteObserverImp_ = std::make_unique<ClassicRemoteDeviceObserver>(this);
    bleObserverImp_ = std::make_unique<AdapterBleObserver>(this);
    bleRemoteObserverImp_ = std::make_unique<BlePeripheralCallback>(this);
    resourceMgrObserverImp_ = std::make_shared<BluetoothResourceMgrObserver>(this);
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
    bleAppContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    bleAppContainer_->Init();
    remoteAppContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    remoteAppContainer_->Init();
    bleRemoteAppContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    bleRemoteAppContainer_->Init();
    resourceMgrAppContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    resourceMgrAppContainer_->Init();
}

BluetoothHostServer::impl::~impl()
{
}

void BluetoothHostServer::impl::Init()
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();

    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);

    IAdapterManager::GetInstance()->Start();
    IAdapterManager::GetInstance()->RegisterStateObserver(*observerImp_);
    auto virtualDeviceIdManageFunc = [](const RawAddress &realAddr, RawAddress &randomAddr) -> int {
        return BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr);
    };
    IAdapterManager::GetInstance()->RegisterVirtualDeviceIdManagerFunc(virtualDeviceIdManageFunc);
    if (classicService) {
        classicService->RegisterClassicAdapterObserver(*classicObserverImp_.get());
        classicService->RegisterRemoteDeviceObserver(*remoteObserverImp_.get());
    }

    if (bleService) {
        bleService->RegisterBleAdapterObserver(*bleObserverImp_.get());
        bleService->RegisterBlePeripheralCallback(*bleRemoteObserverImp_.get());
    }
#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    auto resourceMgrService = IBluetoothResourceManager::GetInstance();
    if (resourceMgrService) {
        resourceMgrService->RegisterObserver(resourceMgrObserverImp_);
    }
#endif
    BluetoothDeviceManager::GetInstance()->AddPairedDevInfo();
    createServers();
}

void BluetoothHostServer::impl::Clear()
{
    /// systerm state observer
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);

    /// adapter state observer
    IAdapterManager::GetInstance()->Stop();
    IAdapterManager::GetInstance()->DeregisterStateObserver(*observerImp_);

    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();

    if (classicService) {
        /// classic observer
        classicService->DeregisterClassicAdapterObserver(*classicObserverImp_.get());
        /// classic remote observer
        classicService->DeregisterRemoteDeviceObserver(*remoteObserverImp_.get());
    }

    if (bleService) {
        /// ble observer
        bleService->DeregisterBleAdapterObserver(*bleObserverImp_.get());
        /// ble remote observer
        bleService->DeregisterBlePeripheralCallback(*bleRemoteObserverImp_.get());
    }
#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    auto resourceMgrService = IBluetoothResourceManager::GetInstance();
    if (resourceMgrService) {
        /// bluetooth resource manager observer
        resourceMgrService->DeregisterObserver(resourceMgrObserverImp_);
    }
#endif
}

void BluetoothHostServer::impl::createServers()
{
    sptr<BluetoothSocketServer> socket = new BluetoothSocketServer();
    servers_[PROFILE_SOCKET] = socket->AsObject();

    sptr<BluetoothGattServerServer> gattserver = new BluetoothGattServerServer();
    servers_[PROFILE_GATT_SERVER] = gattserver->AsObject();

    sptr<BluetoothGattClientServer> gattclient = new BluetoothGattClientServer();
    servers_[PROFILE_GATT_CLIENT] = gattclient->AsObject();

    sptr<BluetoothHfpAgServer> hfpAg = new BluetoothHfpAgServer();
    servers_[PROFILE_HFP_AG] = hfpAg->AsObject();

#ifdef BLUETOOTH_HFP_HF_ENABLE
    sptr<BluetoothHfpHfServer> hfpHf = new BluetoothHfpHfServer();
    servers_[PROFILE_HFP_HF] = hfpHf->AsObject();
#endif

#ifdef BLUETOOTH_AVRCP_CT_FEATURE_ENABLE
    sptr<BluetoothAvrcpCtServer> avrcpCtServer = new BluetoothAvrcpCtServer();
    servers_[PROFILE_AVRCP_CT] = avrcpCtServer->AsObject();
#endif

    sptr<BluetoothAvrcpTgServer> avrcpTgServer = new BluetoothAvrcpTgServer();
    servers_[PROFILE_AVRCP_TG] = avrcpTgServer->AsObject();

    sptr<BluetoothBleAdvertiserServer> bleAdvertiser = new BluetoothBleAdvertiserServer();
    bleServers_[BLE_ADVERTISER_SERVER] = bleAdvertiser->AsObject();

    bleCentralManger_ = new BluetoothBleCentralManagerServer();
    bleCentralManger_->Init(bleCentralManger_);
    bleServers_[BLE_CENTRAL_MANAGER_SERVER] = bleCentralManger_->AsObject();

    sptr<BluetoothA2dpSourceServer> a2dpSource = new BluetoothA2dpSourceServer();
    servers_[PROFILE_A2DP_SRC] = a2dpSource->AsObject();

#ifdef BLUETOOTH_A2DP_SINK_ENABLE
    sptr<BluetoothA2dpSinkServer> a2dpSink = new BluetoothA2dpSinkServer();
    servers_[PROFILE_A2DP_SINK] = a2dpSink->AsObject();
#endif
    sptr<BluetoothHidHostServer> hidHostServer = new BluetoothHidHostServer();
    servers_[PROFILE_HID_HOST_SERVER] = hidHostServer->AsObject();
#ifdef BLUETOOTH_HIDDEVICE_FEATURE_ENABLE
    sptr<BluetoothHidDeviceServer> hidDeviceServer = new BluetoothHidDeviceServer();
    servers_[PROFILE_HID_DEVICE_SERVER] = hidDeviceServer->AsObject();
#endif
#ifdef BLUETOOTH_PAN_FEATURE_ENABLE
    sptr<BluetoothPanServer> panServer = new BluetoothPanServer();
    servers_[PROFILE_PAN_SERVER] = panServer->AsObject();
#endif
    sptr<BluetoothAudioManagerServer> audioManagerServer = new BluetoothAudioManagerServer();
    servers_[PROFILE_AUDIO_MANAGER] = audioManagerServer->AsObject();

#ifdef BLUETOOTH_PBAP_PSE_FEATURE_ENABLE
    sptr<BluetoothPbapPseServer> pbapPseServer = new BluetoothPbapPseServer();
    servers_[PROFILE_PBAP_PSE] = pbapPseServer->AsObject();
#endif

#ifdef BLUETOOTH_MAP_MSE_FEATURE_ENABLE
    sptr<BluetoothMapMseServer> mapMseServer = new BluetoothMapMseServer();
    servers_[PROFILE_MAP_MSE] = mapMseServer->AsObject();
#endif

#ifdef BLUETOOTH_OPP_FEATURE_ENABLE
    sptr<BluetoothOppServer> oppServer = new BluetoothOppServer();
    servers_[PROFILE_OPP_SERVER] = oppServer->AsObject();
#endif

#ifdef BLUETOOTH_HEARINGAID_FEATURE_ENABLE
    sptr<BluetoothHearingAidServer> hearingAidServer = new BluetoothHearingAidServer();
    servers_[PROFILE_HEARINGAID_SERVER] = hearingAidServer->AsObject();
#endif
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    sptr<BluetoothBasServer> basServer = new BluetoothBasServer();
    servers_[PROFILE_BAS_SERVER] = basServer->AsObject();
#endif
    HILOGI("servers_ constructed, size is %{public}zu", servers_.size());
}

void BluetoothHostServer::impl::SetDeviceConnectable(const bluetooth::BTStateID state)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicService && state == BTStateID::STATE_TURN_ON) {
        classicService->SetBtScanMode(SCAN_MODE_CONNECTABLE, 0);
    }
}

void BluetoothHostServer::impl::LoadAppTypeCache()
{
    std::map<std::string, PreferencesValue> allData =
        PreferencesManager::GetAll(PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
    for (const auto &entry : allData) {
        std::string key = entry.first;
        std::string value = entry.second;
        appTypeCache_.EnsureInsert(key, value);
    }
}

std::string BluetoothHostServer::impl::GetAppTypeFromCache(const std::string &key, const std::string &defaultValue)
{
    std::string value;
    if (appTypeCache_.Find(key, value)) {
        return value;
    }
    return PreferencesManager::GetString(key, defaultValue, PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
}

BluetoothHostServer::BluetoothHostServer() : SystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, true)
{
    pimpl = std::make_unique<impl>();
}

BluetoothHostServer::~BluetoothHostServer()
{
    HILOGI("~BluetoothHostServer called.");
}

sptr<BluetoothHostServer> BluetoothHostServer::GetInstance()
{
    std::lock_guard<std::mutex> autoLock(instanceLock);
    if (instance == nullptr) {
        sptr<BluetoothHostServer> temp = new BluetoothHostServer();
        instance = temp;
    }
    return instance;
}

void BluetoothHostServer::OnStart()
{
    HILOGI("starting service.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOGI("service is already started.");
        return;
    }

    if (!Init()) {
        HILOGE("initiation failed");
        OnStop();
        return;
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    IPCSkeleton::SetMaxWorkThreadNum(32); //  default ipc thread num is 16, increase it to 32
    HILOGI("service has been started successfully");
    return;
}

bool BluetoothHostServer::Init()
{
    // For ffrt watchdog available in foundation
    HiviewDFX::Watchdog::GetInstance().InitFfrtWatchdog();
    pimpl->Init();
    if (!registeredToService_) {
        bool ret = Publish(BluetoothHostServer::GetInstance());
        if (!ret) {
            HILOGE("init publish failed!");
            return false;
        }
        registeredToService_ = true;
    }
    HILOGI("init success");
    return true;
}

void BluetoothHostServer::OnStop()
{
    HILOGI("stopping service.");
    pimpl->Clear();
    state_ = ServiceRunningState::STATE_IDLE;
    registeredToService_ = false;
    return;
}

#ifdef FUSION_CONNECTIVITY_SUPPORTED
static bool IsFusionConnectivityApp()
{
    int32_t fusionConnUid = 7035;
    return  (PermissionManager::IsSystemHap() && IPCSkeleton::GetCallingUid() == fusionConnUid);
}

static bool IsRssApp()
{
    int32_t rssUid = 1096;
    return  (PermissionManager::IsSystemHap() && IPCSkeleton::GetCallingUid() == rssUid);
}

static bool IsBluetoothSwitchAllowed()
{
    // 1) 三方应用开关蓝牙，需要直接弹框
    // 2）系统应用开关蓝牙，且是融合短距模块
    return (!PermissionManager::IsSystemHap()) || IsFusionConnectivityApp() || IsRssApp();
}
#endif  // FUSION_CONNECTIVITY_SUPPORTED

void BluetoothHostServer::RegisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("RegisterObserver observer is null");
        return;
    }
    if (pimpl->appContainer_->Size() > MAX_MAP_SIZE || pimpl->observers_.Size() > MAX_MAP_SIZE) {
        HILOGE("appContainer_ or observers_ too much");
        return;
    }
    pimpl->appContainer_->AddObject(observer->AsObject());
    pimpl->observers_.Register(observer);

#ifdef FUSION_CONNECTIVITY_SUPPORTED
    {
        std::lock_guard<std::mutex> lock(pimpl->fusionConnectivityObserverMutex_);
        if (IsFusionConnectivityApp() && pimpl->fusionConnectivityObserver_ == nullptr) {
            pimpl->fusionConnectivityObserverDeathRecipient_ =
                sptr<impl::FusionConnectivityObserverDeathRecipient>::MakeSptr(pimpl.get());
            pimpl->fusionConnectivityObserver_ = observer;
            if (!pimpl->fusionConnectivityObserver_->AsObject()->
                AddDeathRecipient(pimpl->fusionConnectivityObserverDeathRecipient_)) {
                HILOGE("Failed to link fusionConnectivityObserverDeathRecipient_ to callback");
            }
        }
    }
#endif  // FUSION_CONNECTIVITY_SUPPORTED
}

void BluetoothHostServer::DeregisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr || pimpl == nullptr) {
        HILOGE("DeregisterObserver observer is null");
        return;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

int32_t BluetoothHostServer::EnableBt()
{
    HILOGE("This interface is not supported currently.");
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::DisableBt(bool isAsync, const std::string &callingName)
{
    // bluetooth switch action may has been transferred to fusion connectivity
    std::string realCallingName = (callingName == "" ? PermissionManager::GetCallingName() : callingName);
#ifdef FUSION_CONNECTIVITY_SUPPORTED
    do {
        if (!IsBluetoothSwitchAllowed()) {
            std::lock_guard<std::mutex> lock(pimpl->fusionConnectivityObserverMutex_);
            if (!pimpl->fusionConnectivityObserver_) {
                HILOGW("fusionConnectivityObserver_ is nullptr, Attempt enable bluetooth directly");
                break;
            }
            pimpl->fusionConnectivityObserver_->OnBluetoothSwitchAction(
                TRANS_ACTION_DISABLE_BLUETOOTH, realCallingName);
            HILOGI("%{public}s disable bluetooth to restrict mode is transferred", realCallingName.c_str());
            return BT_ERR_SWITCH_OP_TRANSFERRED;
        }
    } while (0);
#endif

    HILOGI("disable bluetooth, calling by (%{public}s)", realCallingName.c_str());
    IAdapterManager::GetInstance()->SetApplicationDisableBluetoothFlag(true);
    return IAdapterManager::GetInstance()->Disable(bluetooth::BTTransport::ADAPTER_BREDR, isAsync,
        realCallingName, true);
}

static int32_t ConvertBTStateIDToBluetoothState(int32_t brState, int32_t bleState)
{
    BluetoothState state = BluetoothState::STATE_OFF;
    switch (brState) {
        case BTStateID::STATE_TURN_ON: state = BluetoothState::STATE_ON; break;
        case BTStateID::STATE_TURNING_ON: state = BluetoothState::STATE_TURNING_ON; break;
        case BTStateID::STATE_TURNING_OFF: state = BluetoothState::STATE_TURNING_OFF; break;
        case BTStateID::STATE_TURN_OFF: {
            switch (bleState) {
                case BTStateID::STATE_TURN_ON: state = BluetoothState::STATE_BLE_ON; break;
                case BTStateID::STATE_TURNING_ON: state = BluetoothState::STATE_BLE_TURNING_ON; break;
                case BTStateID::STATE_TURNING_OFF: state = BluetoothState::STATE_BLE_TURNING_OFF; break;
                default: break;
            }
            break;
        }
        default: HILOGE("Invalid bt state"); break;
    }
    return static_cast<int32_t>(state);
}

int32_t BluetoothHostServer::GetBtState(int32_t &state)
{
    int32_t brState = BTStateID::STATE_TURN_OFF;
    int32_t bleState = BTStateID::STATE_TURN_OFF;
    if (IAdapterManager::GetInstance()->IsBluetoothRestricted()) {
        brState = IAdapterManager::GetInstance()->GetRestrictedState(bluetooth::BTTransport::ADAPTER_BREDR);
        bleState = IAdapterManager::GetInstance()->GetRestrictedState(bluetooth::BTTransport::ADAPTER_BLE);
    } else if (IAdapterManager::GetInstance()->IsBleOwnerOnlyMode()) {
        // strict owner-only visibility: BR is never usable; BLE is visible
        // only to owner pids (system apps are NOT exempted). Reconcile the
        // server-side cache with the state machine on this path.
        RefreshOwnerPidCache();
        brState = BTStateID::STATE_TURN_OFF;
        bleState = IAdapterManager::GetInstance()->GetState(bluetooth::BTTransport::ADAPTER_BLE);
        if (bleState == BTStateID::STATE_TURN_ON && !IsOwnerPidCached(IPCSkeleton::GetCallingPid())) {
            bleState = BTStateID::STATE_TURN_OFF;
        }
    } else {
        brState = IAdapterManager::GetInstance()->GetState(bluetooth::BTTransport::ADAPTER_BREDR);
        bleState = IAdapterManager::GetInstance()->GetState(bluetooth::BTTransport::ADAPTER_BLE);
    }
    state = ConvertBTStateIDToBluetoothState(brState, bleState);
    return BT_NO_ERROR;
}

sptr<IRemoteObject> BluetoothHostServer::GetProfile(const std::string &name)
{
    HILOGD("seraching %{public}s ", name.c_str());
    CHECK_AND_RETURN_LOG_RET(pimpl != nullptr, nullptr, "pimpl is null");
    auto it = pimpl->servers_.find(name);
    if (it != pimpl->servers_.end()) {
        return pimpl->servers_[name];
    } else {
        return nullptr;
    }
}

sptr<IRemoteObject> BluetoothHostServer::GetBleRemote(const std::string &name)
{
    HILOGI("GetBleRemote %{public}s ", name.c_str());
    auto iter = pimpl->bleServers_.find(name);
    if (iter != pimpl->bleServers_.end()) {
        return pimpl->bleServers_[name];
    } else {
        return nullptr;
    }
}

// Fac_Res_CODE
int32_t BluetoothHostServer::BluetoothFactoryReset()
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    CHECK_AND_RETURN_LOG_RET(classicService->FactoryReset() == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR,
        "factoryreset failed!");
    int32_t ret = DisableBt();
    if (ret == BT_ERR_SWITCH_OP_TRANSFERRED) {
        ret = BT_NO_ERROR;
    }
    return ret;
}

int32_t BluetoothHostServer::GetDeviceType(int32_t transport, const std::string &address)
{
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, INVALID_VALUE,
        "transport invalid or BT current state is not enabled!");
    transport = classicService->GetDeviceType(realAddr);
    HILOGI("address: %{public}s, transport: %{public}d", GET_ENCRYPT_STR_ADDR(address), transport);
    return transport;
}

int32_t BluetoothHostServer::GetLocalAddress(std::string &addr)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "GetLocalAddress failed");
    addr = classicService->GetLocalAddress();
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GenerateLocalOobData(int32_t transport, const sptr<IBluetoothOobObserver> &observer)
{
    HILOGI("callingName(%{public}s, transport(%{public}d)", PermissionManager::GetCallingName().c_str(), transport);
    if (!(transport == BT_TRANSPORT_BREDR || transport == BT_TRANSPORT_BLE)) {
        HILOGE("invalid transport");
        return BT_ERR_INVALID_PARAM;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is invalid!");
    {
        std::lock_guard<std::mutex> lock(pimpl->oobObserverMutex_);
        if (pimpl->oobObserver_ != nullptr) {
            HILOGE("GenerateLocalOobData busy");
            return BT_ERR_INTERNAL_ERROR;
        }
        pimpl->oobObserver_ = observer;
    }
    int32_t ret = classicService->GenerateLocalOobData(transport);
    if (ret != BT_NO_ERROR) {
        HILOGE("[host_server] GenerateLocalOobData failed, reset oobObserver.");
        std::lock_guard<std::mutex> lock(pimpl->oobObserverMutex_);
        pimpl->oobObserver_ = nullptr;
    }
    return ret;
}

int32_t BluetoothHostServer::EnableBle(bool noAutoConnect, bool isAsync, const std::string &callingName)
{
    if (noAutoConnect) {
        IAdapterManager::GetInstance()->SetQuietMode(true);
    }

    // bluetooth switch action may has been transferred to fusion connectivity
    std::string realCallingName = (callingName == "" ? PermissionManager::GetCallingName() : callingName);
#ifdef FEATURE_MOUSE_FAST_CONNECTION
    // if RSS called EnableBluetoothNoAutoConnect, set needReconnectActiveDevice_ true
    if (noAutoConnect && PermissionManager::IsSystemHap()
        && realCallingName.compare(CALLING_NAME_RESOURCE_SCHEDULE) == 0) {
        IAdapterManager::GetInstance()->SetNeedReconnectActiveDevice();
        HILOGI("RSS EnableBluetoothNoAutoConnect, set NeedReconnectActiveDevice");
    }
#endif
#ifdef FUSION_CONNECTIVITY_SUPPORTED
    do {
        if (!IsBluetoothSwitchAllowed()) {
            std::lock_guard<std::mutex> lock(pimpl->fusionConnectivityObserverMutex_);
            if (!pimpl->fusionConnectivityObserver_) {
                HILOGW("fusionConnectivityObserver_ is nullptr, Attempt enable bluetooth directly");
                break;
            }
            pimpl->fusionConnectivityObserver_->OnBluetoothSwitchAction(
                TRANS_ACTION_ENABLE_BLUETOOTH, realCallingName);
            HILOGI("%{public}s enable bluetooth is transferred", realCallingName.c_str());
            return BT_ERR_SWITCH_OP_TRANSFERRED;
        }
    } while (0);
#endif

    HILOGI("enable bluetooth, calling by (%{public}s)", realCallingName.c_str());
    // If is in restricted mode, just change to br on.
    if (IAdapterManager::GetInstance()->IsBluetoothRestricted()) {
        return IAdapterManager::GetInstance()->EnablebluetoothFromRestricted(realCallingName, isAsync, true);
    }
    // If is in BLE-owner-only mode, upgrade to full-on; both stacks are already
    // up and switch interfaces are not owner-restricted.
    if (IAdapterManager::GetInstance()->IsBleOwnerOnlyMode()) {
        int32_t ret = IAdapterManager::GetInstance()->EnableBluetoothFromBleOwnerOnlyMode(realCallingName);
        if (ret == BT_NO_ERROR) {
            RefreshOwnerPidCache();
        }
        return ret;
    }

    return IAdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE, isAsync, realCallingName, true);
}

int32_t BluetoothHostServer::DisableBle()
{
    HILOGE("This interface is not supported currently.");
    return BT_ERR_INTERNAL_ERROR;
}

std::vector<uint32_t> BluetoothHostServer::GetProfileList()
{
    return IProfileManager::GetInstance()->GetProfileServicesList();
}

int32_t BluetoothHostServer::GetMaxNumConnectedAudioDevices()
{
    return IAdapterManager::GetInstance()->GetMaxNumConnectedAudioDevices();
}

int32_t BluetoothHostServer::GetBtConnectionState(int32_t &state)
{
    if (IsBtEnabled()) {
        state = (int32_t)IAdapterManager::GetInstance()->GetAdapterConnectState();
        HILOGI("state: %{public}d", state);
    } else {
        HILOGW("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetBtProfileConnState(uint32_t profileId, int &state)
{
    if (IsBtEnabled()) {
        state = (int32_t)IProfileManager::GetInstance()->GetProfileServiceConnectState(profileId);
    } else {
        HILOGW("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    return BT_NO_ERROR;
}

void BluetoothHostServer::GetLocalSupportedUuids(std::vector<std::string> &uuids)
{
    IProfileManager::GetInstance()->GetProfileServicesSupportedUuids(uuids);
}

int32_t BluetoothHostServer::GetLocalDeviceClass()
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, 0, "BT current state is not enabled!");
    return classicService->GetLocalDeviceClass();
}

bool BluetoothHostServer::SetLocalDeviceClass(const int32_t &deviceClass)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "BT current state is not enabled!");
    return classicService->SetLocalDeviceClass(deviceClass);
}

int32_t BluetoothHostServer::GetLocalName(std::string &name)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    name = classicService->GetLocalName();
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetLocalName(const std::string &name)
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (PermissionManager::IsInAdvWhiteList(tokenId)) { // temp solution
        IAdapterManager::GetInstance()->SetTmpAdvName(tokenId, name);
        return BT_NO_ERROR;
    }
    if (PermissionManager::IsNeededDiscarded(ONLY_THIRDLY_HAP, API_VERSION_12)) {
        return BT_ERR_API_NOT_SUPPORT;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    if (classicService->SetLocalName(name)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::GetBtScanMode(int32_t &scanMode)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    scanMode = classicService->GetBtScanMode();
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetBtScanMode(int32_t mode, int32_t duration)
{
    HILOGI("mode: %{public}d, duration: %{public}d", mode, duration);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    if (classicService->SetBtScanMode(mode, duration)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::GetBondableMode(int32_t transport)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BONDABLE_MODE_OFF,
        "Parameter::transport invalid or BT current state is not enabled!");
    return classicService->GetBondableMode();
}

bool BluetoothHostServer::SetBondableMode(int32_t transport, int32_t mode)
{
    HILOGI("mode: %{public}d", mode);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false,
        "Parameter::transport invalid or BT current state is not enabled!");
    return classicService->SetBondableMode(mode);
}

int32_t BluetoothHostServer::StartBtDiscovery()
{
    int version = PermissionManager::GetApiVersion();
    std::string callingName = PermissionManager::GetCallingName();
    HILOGI("version(%{public}d), callingName(%{public}s)", version, callingName.c_str());
    if (PermissionManager::IsNativeCaller() || version >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH),
            BT_ERR_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    } else {
        if (!PermissionManager::VerifyPermission(DISCOVER_BLUETOOTH)) {
            HILOGE("[PERMISSION] check DISCOVER_BLUETOOTH permission failed, callingName(%{public}s)",
                callingName.c_str());
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE5, callingName);
            return BT_ERR_PERMISSION_FAILED;
        }
        if (!PermissionManager::VerifyPermission(APPROXIMATELY_LOCATION) &&
            !PermissionManager::VerifyPermission(LOCATION)) {
            HILOGE("[PERMISSION] check LOCATION permission failed, callingName(%{public}s)",
                callingName.c_str());
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE6, callingName);
            return BT_ERR_PERMISSION_FAILED;
        }
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->StartBtDiscovery();
}

int32_t BluetoothHostServer::CancelBtDiscovery()
{
    HILOG_COMM_INFO("CancelBtDiscovery: callingName(%{public}s)", PermissionManager::GetCallingName().c_str());
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    if (classicService->CancelBtDiscovery()) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::IsBtDiscovering(bool &isDiscovering, int32_t transport)
{
    HILOGI("transport: %{public}d", transport);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "Parameter::transport invalid or BT current state is not enabled!");
    isDiscovering = classicService->IsBtDiscovering();
    return BT_NO_ERROR;
}

long BluetoothHostServer::GetBtDiscoveryEndMillis()
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, INVALID_VALUE, "BT current state is not enabled!");
    return classicService->GetBtDiscoveryEndMillis();
}

int32_t BluetoothHostServer::GetPairedDevices(std::vector<BluetoothRawAddress> &pairedAddr)
{
    std::vector<RawAddress> rawAddrVec;
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    rawAddrVec = classicService->GetPairedDevices();
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawAddrVec, pairedAddr);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::RemovePair(int32_t transport, const sptr<BluetoothRawAddress> &device)
{
    if (device == nullptr) {
        HILOGE("device is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    HILOGI("addr:%{public}s, transport:%{public}d", GET_ENCRYPT_ADDR(*device), transport);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(*device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT/BLE current state is not enabled!");
    if (classicService->RemovePair(transport, realAddr)) {
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, realAddr.GetAddress(),
            "REMOVEPAIRRSCENE", CHR_WATCH_REMOVE_PAIR);
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, realAddr.GetAddress(),
            "REMOVEPAIRINITIATOR", CHR_INITIATOR_LOCAL);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

bool BluetoothHostServer::RemoveAllPairs()
{
    if (BTStateID::STATE_TURN_ON != IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR) &&
        BTStateID::STATE_TURN_ON != IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE)) {
        HILOGW("BT current state is not enabled!");
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false,
        "transport invalid or BT/BLE current state is not enabled!");
    if (!classicService->RemoveAllPairs()) {
        HILOGE("BREDR RemoveAllPairs failed");
        return false;
    }
    return true;
}

int32_t BluetoothHostServer::GetBleMaxAdvertisingDataLength()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(bleService != nullptr, INVALID_VALUE, "BT current state is not enabled!");
    return bleService->GetBleMaxAdvertisingDataLength();
}

int32_t BluetoothHostServer::GetPhonebookPermission(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    return (int32_t)IAdapterManager::GetInstance()->GetPhonebookPermission(address);
}

bool BluetoothHostServer::SetPhonebookPermission(const std::string &address, int32_t permission)
{
    HILOGI("address: %{public}s, permission: %{public}d", GET_ENCRYPT_STR_ADDR(address), permission);
    return IAdapterManager::GetInstance()->SetPhonebookPermission(address, (BTPermissionType)permission);
}

int32_t BluetoothHostServer::GetMessagePermission(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    return (int32_t)IAdapterManager::GetInstance()->GetMessagePermission(address);
}

bool BluetoothHostServer::SetMessagePermission(const std::string &address, int32_t permission)
{
    HILOGI("address: %{public}s, permission: %{public}d", GET_ENCRYPT_STR_ADDR(address), permission);
    return IAdapterManager::GetInstance()->SetMessagePermission(address, (BTPermissionType)permission);
}

int32_t BluetoothHostServer::GetPowerMode(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    return IAdapterManager::GetInstance()->GetPowerMode(address);
}

int32_t BluetoothHostServer::GetDeviceName(int32_t transport, const std::string &address, std::string &name, bool alias)
{
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    name = classicService->GetDeviceName(realAddr, alias);
    return BT_NO_ERROR;
}

std::string BluetoothHostServer::GetDeviceAlias(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, INVALID_NAME, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr);
    return classicService->GetAliasName(realAddr);
}

int32_t BluetoothHostServer::SetDeviceAlias(const std::string &address, const std::string &aliasName)
{
    HILOGI("addr: %{public}s, alias: %{public}s", GET_ENCRYPT_STR_ADDR(address), GET_ENCRYPT_DEVICE_NAME(aliasName));
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INVALID_PARAM;
    }
    return classicService->SetAliasName(realAddr, aliasName) ? BT_NO_ERROR: BT_ERR_INVALID_PARAM;
}

int32_t BluetoothHostServer::GetRemoteDeviceBatteryInfo(const std::string &address, BluetoothBatteryInfo &batteryInfo)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "BT current state is not enabled!");
    BatteryInfo info;
    int32_t ret = classicService->GetRemoteDeviceBatteryInfo(realAddr, info);
    batteryInfo = BluetoothBatteryInfo(info);
    return ret;
}

int32_t BluetoothHostServer::SetRemoteDeviceBatteryInfo(const std::string &address,
    const BluetoothBatteryInfo &batteryInfo)
{
    HILOGI("callingName(%{public}s), address(%{public}s)", PermissionManager::GetCallingName().c_str(),
        GET_ENCRYPT_STR_ADDR(address));

    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != AUDIO_SERVER_UID) {
        HILOGE("This API is only for audio_server.");
        return BT_ERR_PERMISSION_FAILED;
    }

    BatteryInfo info;
    info.batteryLevel_ = batteryInfo.batteryLevel_;
    info.leftEarBatteryLevel_ = batteryInfo.leftEarBatteryLevel_;
    info.leftEarChargeState_ = batteryInfo.leftEarChargeState_;
    info.rightEarBatteryLevel_ = batteryInfo.rightEarBatteryLevel_;
    info.rightEarChargeState_ = batteryInfo.rightEarChargeState_;
    info.boxBatteryLevel_ = batteryInfo.boxBatteryLevel_;
    info.boxChargeState_ = batteryInfo.boxChargeState_;

    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "BT current state is not enabled!");
    int32_t ret = classicService->SetRemoteDeviceBatteryInfo(RawAddress(address), info);
    return ret;
}

int32_t BluetoothHostServer::GetPairState(int32_t transport, const std::string &address, int32_t &pairState)
{
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "BT current state is not enabled!");
    pairState = classicService->GetPairState(realAddr);
    std::string logKey = std::string("(GetPairState)") + GET_ENCRYPT_STR_ADDR(address);
    HILOGI_ACCUMULATE(logKey, "pairState: %d", pairState);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::StartPair(int32_t transport, const BluetoothRawAddress &bluetoothRawAddress,
    const BluetoothOobData &oobData)
{
    if (oobData.HasOobData() && !PermissionManager::IsSystemHap()) {
        // PairDeviceOutOfBand is a systemAPI.
        HILOGE("Non-system applications are not allowed to use system APIs.");
        return BT_ERR_SYSTEM_PERMISSION_FAILED;
    }
    std::string address = bluetoothRawAddress.GetAddress();
    int32_t addressType = bluetoothRawAddress.GetAddressType();
    HILOGI("callingName(%{public}s, transport(%{public}d), address(%{public}s)), addressType(%{public}d),"
        "isPairDeviceOob(%{public}d)", PermissionManager::GetCallingName().c_str(), transport,
        GET_ENCRYPT_STR_ADDR(address), addressType, oobData.HasOobData());
    RawAddress addr(address);
    RawAddress realAddr;
    BtChrAddConnSceneInfo(address, PAIR_TYPE_USER_PAIR, PermissionManager::GetCallingName(), 0);
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddrByAddrType(addr, addressType, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    if (classicService->StartPair(transport, realAddr, oobData)) {
        uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
        std::string pkgName = PermissionManager::GetCallingName();
        std::lock_guard<std::mutex> lock(pimpl->pairRecordMutex_);
        pimpl->pairRecord_ = {GetNowTimestamp(), GetPairedAppIdentifier(tokenId), pkgName, true};
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::StartCrediblePair(int32_t transport, const std::string &address)
{
    HILOGI("callingName(%{public}s, transport(%{public}d), address(%{public}s))",
        PermissionManager::GetCallingName().c_str(), transport, GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    BtChrAddConnSceneInfo(address, PAIR_TYPE_USER_PAIR, PermissionManager::GetCallingName(), 0);
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "transport invalid!");
    if (classicService->StartCrediblePair(transport, realAddr)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

bool BluetoothHostServer::CancelPairing(int32_t transport, const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->CancelPairing(realAddr);
}

bool BluetoothHostServer::IsBondedFromLocal(int32_t transport, const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->IsBondedFromLocal(realAddr);
}

int32_t BluetoothHostServer::StartRemoteSdpSearch(const std::string &address, const std::string &uuid)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    if (classicService->StartRemoteSdpSearch(address, uuid)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::GetRemoteServices(const std::string &address)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    if (classicService->GetRemoteServices(address)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

bool BluetoothHostServer::IsAclConnected(int32_t transport, const std::string &address)
{
    HILOGD("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        HILOGE("Covert to real address fail");
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->IsAclConnected(realAddr);
}

bool BluetoothHostServer::IsAclEncrypted(int32_t transport, const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->IsAclEncrypted(realAddr);
}

int32_t BluetoothHostServer::GetDeviceClass(const std::string &address, int32_t &cod)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    cod = classicService->GetDeviceClass(realAddr);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetDeviceUuids(const std::string &address, std::vector<std::string> &uuids)
{
    if (!IsBtEnabled()) {
        HILOGE("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    HILOGD("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicService) {
        uuids = classicService->GetDeviceUuids(realAddr);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetConnectedBLEDevices(int32_t bleProfile, std::vector<std::string> &connectedDevices)
{
    if (!IsBtEnabled()) {
        HILOGE("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    std::set<std::string> resConnectedDevices {};
    std::vector<std::string> clientDevices = BluetoothStateManager::GetInstance()->GetClientDeviceList();
    std::vector<std::string> serverDevices = BluetoothStateManager::GetInstance()->GetServerDeviceList();
    switch (bleProfile) {
        case static_cast<int32_t>(BleProfile::GATT):
            resConnectedDevices.insert(clientDevices.begin(), clientDevices.end());
            resConnectedDevices.insert(serverDevices.begin(), serverDevices.end());
            break;
        case static_cast<int32_t>(BleProfile::GATT_CLIENT):
            resConnectedDevices.insert(serverDevices.begin(), serverDevices.end());
            break;
        case static_cast<int32_t>(BleProfile::GATT_SERVER):
            resConnectedDevices.insert(clientDevices.begin(), clientDevices.end());
            break;
        default:
            break;
    }
    connectedDevices.clear();
    if (PermissionManager::IsUseRealAddr()) {
        connectedDevices.assign(resConnectedDevices.begin(), resConnectedDevices.end());
    } else {
        for (auto &realAddr : resConnectedDevices) {
            RawAddress realAddrDevice(realAddr);
            RawAddress randomAddrDevice;
            int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(realAddrDevice, randomAddrDevice);
            if (ret == OHOS::bluetooth::RET_NO_EXIST) {
                randomAddrDevice = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(realAddrDevice);
            }
            connectedDevices.emplace_back(randomAddrDevice.GetAddress());
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetLocalProfileUuids(std::vector<std::string> &uuids)
{
    if (!IsBtEnabled()) {
        HILOGE("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicService) {
        uuids = classicService->GetLocalProfileUuids();
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetDevicePin(const std::string &address, const std::string &pin)
{
    HILOGI("address: %{public}s, pin: %{public}s", GET_ENCRYPT_STR_ADDR(address), GetEncryptPinCode(pin).c_str());
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (classicService->SetDevicePin(realAddr, pin)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept)
{
    HILOGI("address: %{public}s, accept: %{public}d", GET_ENCRYPT_STR_ADDR(address), accept);
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    if (classicService->SetDevicePairingConfirmation(realAddr, accept)) {
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

bool BluetoothHostServer::SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept)
{
    HILOGI("address: %{public}s, passkey: %{public}s, accept: %{public}d",
        GET_ENCRYPT_STR_ADDR(address), GetEncryptPinCode(passkey).c_str(), accept);

    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->SetDevicePasskey(realAddr, passkey, accept);
}

bool BluetoothHostServer::PairRequestReply(int32_t transport, const std::string &address, bool accept)
{
    HILOGI("address: %{public}s, accept: %{public}d", GET_ENCRYPT_STR_ADDR(address), accept);
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "transport invalid or BT current state is not enabled!");
    return classicService->PairRequestReply(realAddr, accept);
}

bool BluetoothHostServer::ReadRemoteRssiValue(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(bleService != nullptr, false, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    return bleService->ReadRemoteRssiValue(realAddr);
}

void BluetoothHostServer::RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    if (pimpl->remoteAppContainer_->Size() > MAX_MAP_SIZE || pimpl->remoteObservers_.Size() > MAX_MAP_SIZE) {
        HILOGE("remoteAppContainer_ or remoteObservers_ too much");
        return;
    }
    pimpl->remoteAppContainer_->AddObject(observer->AsObject());
    pimpl->remoteObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    if (observer == nullptr || pimpl == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    pimpl->remoteObservers_.Deregister(observer);
    pimpl->remoteAppContainer_->RemoveRemoteObject(observer->AsObject());
}

bool BluetoothHostServer::IsBtEnabled()
{
    int state = static_cast<int>(BluetoothState::STATE_OFF);
    GetBtState(state);
    bool isEnabled = (state == static_cast<int32_t>(BluetoothState::STATE_ON)) ? true : false;
    HILOGD("%{public}s", isEnabled ? "true" : "false");
    return isEnabled;
}

void BluetoothHostServer::RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    if (pimpl->bleAppContainer_->Size() > MAX_MAP_SIZE || pimpl->bleObservers_.Size() > MAX_MAP_SIZE) {
        HILOGE("bleAppContainer_ or bleObservers_ too much");
        return;
    }
    pimpl->bleAppContainer_->AddObject(observer->AsObject());
    pimpl->bleObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr || pimpl == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    pimpl->bleObservers_.Deregister(observer);
    pimpl->bleAppContainer_->RemoveRemoteObject(observer->AsObject());
}

void BluetoothHostServer::RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    if (pimpl->bleRemoteAppContainer_->Size() > MAX_MAP_SIZE ||
        pimpl->bleRemoteObservers_.Size() > MAX_MAP_SIZE) {
        HILOGE("bleRemoteAppContainer_ or bleRemoteObservers_ too much");
        return;
    }
    pimpl->bleRemoteAppContainer_->AddObject(observer->AsObject());
    pimpl->bleRemoteObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    pimpl->bleRemoteObservers_.Deregister(observer);
    pimpl->bleRemoteAppContainer_->RemoveRemoteObject(observer->AsObject());
}

int32_t BluetoothHostServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    HILOGI("fd: %{public}d", fd);
    std::vector<std::string> argsInStr8;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr8), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    std::string result;
    BluetoothHostDumper::BluetoothDump(argsInStr8, result);

    if (!SaveStringToFd(fd, result)) {
        HILOGE("bluetooth dump save string to fd failed!");
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t BluetoothHostServer::SetFastScan(bool isEnable)
{
    HILOGI("isEnable: %{public}d", isEnable);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (IsBtEnabled() && classicService) {
        if (classicService->SetFastScan(isEnable)) {
            return BT_NO_ERROR;
        }
    } else {
        HILOGW("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::GetRandomAddress(const std::string &realAddr, std::string &randomAddr, uint64_t tokenId)
{
    HILOGI("GetRandomAddress, realAddr: %{public}s", GetEncryptAddr(realAddr).c_str());
    uint64_t fullTokenId;
    // If the external application does not transfer tokenId, obtain the tokenId of the invoker.
    if (tokenId == 0) {
        fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    } else {
        fullTokenId = tokenId;
    }
    RawAddress deviceRandom;
    RawAddress deviceReal(realAddr);
    if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(fullTokenId, deviceReal, deviceRandom)) {
        randomAddr = deviceRandom.GetAddress();
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::GetRealAddress(const std::string &randomAddr, std::string &realAddr)
{
    RawAddress realDevice;
    int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(RawAddress(randomAddr), realDevice);
    if (ret != BT_NO_ERROR) {
        HILOGE("virtual addr: %{public}s not found real addr.", GetEncryptAddr(randomAddr).c_str());
        return BT_ERR_INTERNAL_ERROR;
    }
    realAddr = realDevice.GetAddress();
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SyncRandomAddress(const std::string &realAddr, const std::string &randomAddr)
{
    return BT_NO_ERROR;
}

std::string BluetoothHostServer::GetPairedAppIdentifier(uint32_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return "";
    }
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (accessRet != 0) {
        // not hap application
        return "";
    }
    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    std::string appId = GetHapAppIdentifier(userId, bundleName);
    if (appId.empty()) {
        HILOGE("can not get appIdentifier.");
        return "";
    }
    return appId;
}

bool BluetoothHostServer::IsAllowedConnectProfile(const std::string &address)
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    std::string appIdentifier = GetPairedAppIdentifier(tokenId);
    std::string pkgName = PermissionManager::GetCallingName();
    bool isSatisfyTimeAndAppLimit = true;
    PairRecord record;
    pimpl->pairRecords_.Find(address, record);
    if (record.appIdentifier != appIdentifier || record.pkgName != pkgName) {
        isSatisfyTimeAndAppLimit = false;
    }
    int64_t nowTime = GetNowTimestamp();
    if (nowTime - record.timestamp > CONNECT_ALL_TIMEOUT) {
        HILOGE("time out, duration of connectAll is: %{public}ld", nowTime - record.timestamp);
        isSatisfyTimeAndAppLimit = false;
    }
    pimpl->pairRecords_.ChangeValueByLambda(address, [isSatisfyTimeAndAppLimit](PairRecord& record) {
        record.isSatisfyTimeAndAppLimit = isSatisfyTimeAndAppLimit;
    });
    return true;
}

bool BluetoothHostServer::IsAllowedDisconnectProfile(bool isSystemHap, const std::string &address)
{
    if (!isSystemHap &&
        pimpl->GetAppTypeFromCache(address, "false") == "true") {
        return false;
    }
    return true;
}

int32_t BluetoothHostServer::ConnectAllowedProfiles(const std::string &address)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    bool isSystemHap = PermissionManager::IsSystemHap();
    if (!isSystemHap && !IsAllowedConnectProfile(realAddr.GetAddress())) {
        HILOGE("not satisfy invoking condition.");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    uint32_t deviceType = static_cast<uint32_t>(classicService->GetDeviceType(realAddr));
    if (classicService->IsNeedDelayConnect(realAddr) &&
        (deviceType == DEVICE_TYPE_BREDR || deviceType == DEVICE_TYPE_DUAL_MONO)) {
        classicService->PassivePairToDelayConnect(realAddr);
        HILOGI("paired from remote, delay 3s to connect profiles");
        return BT_NO_ERROR;
    }
    pimpl->appTypeCache_.EnsureInsert(realAddr.GetAddress(), isSystemHap ? "true" : "false");
    PreferencesManager::SaveString(realAddr.GetAddress(), isSystemHap ? "true" : "false",
        PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
    if (isSystemHap) {
        classicService->ConnectAllowedProfiles(realAddr);
    } else {
        PairRecord record;
        bool found = pimpl->pairRecords_.Find(address, record);
        classicService->ConnectAllowedProfiles(realAddr, found ? record.isSatisfyTimeAndAppLimit : true);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::DisconnectAllowedProfiles(const std::string &address)
{
    HILOGI(" address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    bool isSystemHap = PermissionManager::IsSystemHap();
    if (!IsAllowedDisconnectProfile(isSystemHap, realAddr.GetAddress())) {
        HILOGE("not satisfy invoking condition.");
        return BT_ERR_INTERNAL_ERROR;
    }

    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE,
        "transport invalid or BT current state is not enabled!");
    classicService->DisconnectAllowedProfiles(realAddr, isSystemHap);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetDeviceCustomType(const std::string &address, int32_t deviceCustomType)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INVALID_PARAM;
    }
    return classicService->SetDeviceCustomType(realAddr, deviceCustomType);
}

int32_t BluetoothHostServer::GetRemoteDeviceInfo(const std::string &address,
    std::shared_ptr<BluetoothRemoteDeviceInfo> &deviceInfo, int type)
{
    CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
        BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    CHECK_AND_RETURN_LOG_RET(type != bluetooth::DeviceInfoType::DEVICE_INFO_UNKNOWN,
        BT_ERR_INVALID_PARAM, "Unknown info type");
    RawAddress addr(address);
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr);
    CHECK_AND_RETURN_LOG_RET(IsBtEnabled(), BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    if (deviceInfo == nullptr) {
        deviceInfo = std::make_shared<BluetoothRemoteDeviceInfo>();
    }
    return classicService->GetRemoteDeviceInfo(realAddr, *deviceInfo);
}

int32_t BluetoothHostServer::SatelliteControl(int type, int state)
{
    return IAdapterManager::GetInstance()->SatelliteControl(type, state);
}

int32_t BluetoothHostServer::IsSupportVirtualAutoConnect(const std::string &address, bool &support)
{
    support = IAdapterManager::GetInstance()->IsSupportVirtualAutoConnect(address);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetVirtualAutoConnectType(const std::string &address, int connType, int businessType)
{
    return IAdapterManager::GetInstance()->SetVirtualAutoConnectType(address, connType, businessType);
}

int32_t BluetoothHostServer::SetFastScanLevel(int32_t level)
{
    HILOGI("level: %{public}d", level);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (IsBtEnabled() && classicService) {
        if (classicService->SetFastScanLevel(level)) {
            return BT_NO_ERROR;
        }
    } else {
        HILOGW("BT current state is not enabled!");
        return BT_ERR_INVALID_STATE;
    }
    return BT_ERR_INTERNAL_ERROR;
}

std::string BluetoothHostServer::DumpRssFreezeInfo()
{
    std::string ret = "";
    if (pimpl->bleCentralManger_) {
        ret = pimpl->bleCentralManger_->DumpRssFreezeInfo();
    }
    return ret;
}

void BluetoothHostServer::RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    if (pimpl->resourceMgrAppContainer_->Size() > MAX_MAP_SIZE ||
        pimpl->resourceMgrObservers_.Size() > MAX_MAP_SIZE) {
        HILOGE("bleRemoteObserversToken_ or bleRemoteObservers_ too much");
        return;
    }
    pimpl->resourceMgrAppContainer_->AddObject(observer->AsObject());
    pimpl->resourceMgrObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer)
{
    if (observer == nullptr || pimpl == nullptr) {
        HILOGE("observer is nullptr!");
        return;
    }
    pimpl->resourceMgrObservers_.Deregister(observer);
    pimpl->resourceMgrAppContainer_->RemoveRemoteObject(observer->AsObject());
}

void BluetoothHostServer::RefreshOwnerPidCache()
{
    auto ownerPids = IAdapterManager::GetInstance()->GetOwnerPids();
    std::lock_guard<std::mutex> lock(pimpl->ownerPidCacheMutex_);
    pimpl->ownerPidCache_ = std::move(ownerPids);
}

bool BluetoothHostServer::IsOwnerPidCached(int32_t pid)
{
    std::lock_guard<std::mutex> lock(pimpl->ownerPidCacheMutex_);
    return pimpl->ownerPidCache_.find(pid) != pimpl->ownerPidCache_.end();
}

int32_t BluetoothHostServer::EnableBluetoothToBleOwnerOnlyMode(const std::string &callingName)
{
    // bluetooth switch action may has been transferred to fusion connectivity
    std::string realCallingName = (callingName == "" ? PermissionManager::GetCallingName() : callingName);
    int32_t callingPid = IPCSkeleton::GetCallingPid();
#ifdef FUSION_CONNECTIVITY_SUPPORTED
    do {
        if (!IsBluetoothSwitchAllowed()) {
            std::lock_guard<std::mutex> lock(pimpl->fusionConnectivityObserverMutex_);
            if (!pimpl->fusionConnectivityObserver_) {
                HILOGW("fusionConnectivityObserver_ is nullptr, Attempt enable bluetooth directly");
                break;
            }
            pimpl->fusionConnectivityObserver_->OnBluetoothSwitchAction(
                TRANS_ACTION_ENABLE_BLUETOOTH_TO_BLE_OWNER_ONLY, realCallingName);
            HILOGI("%{public}s enable bluetooth to ble owner only mode is transferred", realCallingName.c_str());
            return BT_ERR_SWITCH_OP_TRANSFERRED;
        }
    } while (0);
#endif

    HILOGI("enable bluetooth to ble owner only mode, calling by (%{public}s), pid(%{public}d)",
        realCallingName.c_str(), callingPid);
    auto adapterManager = IAdapterManager::GetInstance();
    if (adapterManager == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t ret = adapterManager->EnableBluetoothToBleOwnerOnlyMode(callingPid, realCallingName, true);
    if (ret == BT_NO_ERROR) {
        RefreshOwnerPidCache();
    }
    return ret;
}

int32_t BluetoothHostServer::EnableBluetoothToRestrictMode(const std::string &callingName)
{
    // bluetooth switch action may has been transferred to fusion connectivity
    std::string realCallingName = (callingName == "" ? PermissionManager::GetCallingName() : callingName);
#ifdef FUSION_CONNECTIVITY_SUPPORTED
    do {
        if (!IsBluetoothSwitchAllowed()) {
            std::lock_guard<std::mutex> lock(pimpl->fusionConnectivityObserverMutex_);
            if (!pimpl->fusionConnectivityObserver_) {
                HILOGW("fusionConnectivityObserver_ is nullptr, Attempt enable bluetooth directly");
                break;
            }
            pimpl->fusionConnectivityObserver_->OnBluetoothSwitchAction(
                TRANS_ACTION_ENABLE_BLUETOOTH_TO_RESTRICT_MODE, realCallingName);
            HILOGI("%{public}s enable bluetooth to restrict mode is transferred", realCallingName.c_str());
            return BT_ERR_SWITCH_OP_TRANSFERRED;
        }
    } while (0);
#endif

    HILOGI("enable bluetooth to restirct mode, calling by (%{public}s)", realCallingName.c_str());
    auto adapterManager = IAdapterManager::GetInstance();
    if (adapterManager) {
        return adapterManager->EnableBluetoothToRestrictMode(realCallingName, true);
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostServer::ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
    uint32_t controlTypeVal, uint32_t controlObject)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "ControlDeviceAction failed.");
    return classicService->ControlDeviceAction(deviceId, controlType, controlTypeVal, controlObject);
}

int32_t BluetoothHostServer::GetLastConnectionTime(const std::string &address, int64_t& connectionTime)
{
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(IsBtEnabled(), BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->GetLastConnectionTime(realAddr, connectionTime);
}

int32_t BluetoothHostServer::UpdateCloudBluetoothDevice(std::vector<BluetoothTrustPairDevice> &cloudDevices)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->UpdateCloudBluetoothDevice(cloudDevices);
}
int32_t BluetoothHostServer::GetCloudBondState(const std::string &address, int32_t& cloudBondState)
{
    RawAddress addr(address);
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr);
    CHECK_AND_RETURN_LOG_RET(IsBtEnabled(), BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->GetCloudBondState(realAddr, cloudBondState);
}

int32_t BluetoothHostServer::GetDeviceTransport(const std::string &address, int32_t &transport)
{
    RawAddress addr(address);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exist");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->GetDeviceTransport(realAddr, transport);
}

int32_t BluetoothHostServer::UpdateRefusePolicy(const int32_t protocolType,
    const int32_t pid, const int64_t prohibitedSecondsTime)
{
#ifdef BLUETOOTH_KIA_ENABLE
    HILOG_COMM_INFO("UpdateRefusePolicy: callingName(%{public}s)", PermissionManager::GetCallingName().c_str());
    return IAdapterManager::GetInstance()->UpdateRefusePolicy(protocolType, pid, prohibitedSecondsTime);
#else
    return BT_ERR_API_NOT_SUPPORT;
#endif
}

int32_t BluetoothHostServer::ProcessRandomDeviceIdCommand(
    int32_t command, std::vector<std::string> &deviceIdVec, bool &isValid)
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (command == static_cast<int>(RandomDeviceIdCommand::GET)) {
        return BluetoothDeviceManager::GetInstance()->GetPersistDeviceIds(fullTokenId, deviceIdVec);
    }
    CHECK_AND_RETURN_LOG_RET(deviceIdVec.size() > 0, BT_ERR_INVALID_PARAM, "invalid param size");

    int32_t ret = BT_ERR_INTERNAL_ERROR;
    RawAddress addr(deviceIdVec[0]);
    if (command == static_cast<int>(RandomDeviceIdCommand::ADD)) {
        ret = BluetoothDeviceManager::GetInstance()->AddPersistRandomDeviceId(addr, fullTokenId);
    }
    if (command == static_cast<int>(RandomDeviceIdCommand::DELETE)) {
        ret = BluetoothDeviceManager::GetInstance()->DeletePersistRandomDeviceId(addr, fullTokenId);
    }
    if (command == static_cast<int>(RandomDeviceIdCommand::IS_VALID)) {
        isValid = BluetoothDeviceManager::GetInstance()->IsValidRandomDeviceId(addr);
        ret = BT_NO_ERROR;
    }
    return ret;
}

int32_t BluetoothHostServer::IsProfileExist(const std::string &profileName, bool &isProfileExist)
{
    sptr<IRemoteObject> profilePtr = GetProfile(profileName);
    if (profilePtr == nullptr) {
        isProfileExist = false;
    } else {
        isProfileExist = true;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetCarKeyDfxData(std::string &dfxData)
{
    CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
        BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    dfxData = classicService->GetCarKeyDfxData();
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::SetCarKeyCardData(const std::string &address, int32_t action)
{
    CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
        BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    classicService->SetCarKeyCardData(address, action);
    return BT_NO_ERROR;
}

int32_t BluetoothHostServer::GetVirtualAddressByHash(int hashAlgorithmType,
    const std::string &hashValue, std::string &virtualAddress)
{
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    std::vector<bluetooth::RawAddress> pairedAddrs =  classicService->GetPairedDevices();
    for (const auto &rawAddress : pairedAddrs) {
        std::string hashRes =
            BluetoothDeviceManager::GetInstance()->GetHashValue(hashAlgorithmType, rawAddress.GetAddress());
        if (hashAlgorithmType == SHA256_TYPE &&
            hashRes.size() == HASH256_SIZE && hashRes.substr(hashRes.size() - SUBSTRING_SIZE) == hashValue) {
            RawAddress randomAddr;
            int result = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(rawAddress, randomAddr);
            if (result == bluetooth::RET_NO_EXIST) {
                HILOGE("convert to random addr failed");
                return BT_ERR_INTERNAL_ERROR;
            }
            virtualAddress = randomAddr.GetAddress();
            return BT_NO_ERROR;
        }
    }
    return BT_ERR_UNPAIRED_DEVICE;
}

int32_t BluetoothHostServer::NotifyDialogResult(uint32_t dialogType, bool dialogResult)
{
    return IAdapterManager::GetInstance()->NotifyDialogResult(dialogType, dialogResult);
}

void BluetoothHostServer::SetCallingPackageName(const std::string &address, const std::string &packageName)
{
    CHECK_AND_RETURN_LOG(PermissionManager::IsSystemHap(), "[PERMISSION] system hap needed.");
    BtChrAddCallingPackageName(address, packageName);
}

int32_t BluetoothHostServer::SetConnectionPriority(const std::string &address, int32_t priority)
{
    HILOGI("Set connection priority, addr: %{public}s, priority: %{public}d", GET_ENCRYPT_STR_ADDR(address), priority);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INVALID_STATE, "BT current state is not enabled!");
    return classicService->SetConnectionPriority(address, priority);
}

int32_t BluetoothHostServer::VerifyMultiPermissions(bool systemHapNeeded,
    const std::set<std::string> &permissions)
{
    std::shared_ptr<PermissionItem> item =
        PermissionManager::CreateItem(systemHapNeeded, {}, permissions);
    return PermissionManager::VerifyMultiPermissions(item);
}

int32_t BluetoothHostServer::UpdateSecondaryPhonePairMode(int32_t mode)
{
    return BT_ERR_API_NOT_SUPPORT;
}

int32_t BluetoothHostServer::SetBtChannelScan(bool isEnable, uint32_t interval)
{
    return BT_ERR_API_NOT_SUPPORT;
}
}  // namespace Bluetooth
}  // namespace OHOS
