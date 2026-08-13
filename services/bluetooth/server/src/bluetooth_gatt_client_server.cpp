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
#define LOG_TAG "bt_server_gatt_client"
#endif

#include "bluetooth_gatt_client_server.h"
#include "freeze_utils.h"
#include "bt_chr_app_behavior_reporter.h"

#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hitrace.h"
#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "interface_profile_gatt_client.h"
#include "interface_profile_manager.h"
#include "permission_manager.h"
#include "hitrace_meter.h"
#include "bluetooth_state_manager.h"
#ifdef RES_SCHED_SUPPORT
#include "res_type.h"
#include "res_sched_client.h"
#endif

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
struct BluetoothGattClientServer::impl {
    class GattClientCallbackImpl;
    class SystemStateObserver;

    IProfileGattClient *clientService_;
    std::unique_ptr<SystemStateObserver> systemStateObserver_;
    std::list<std::shared_ptr<GattClientCallbackImpl>> callbacks_;
    std::mutex registerMutex_;

    impl();
    ~impl();

    IProfileGattClient *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfileGattClient *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));
    }
};

class BluetoothGattClientServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothGattClientServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const BTSystemState state) override
    {
        std::lock_guard<std::mutex> lck(impl_->registerMutex_);
        switch (state) {
            case BTSystemState::ON:
                impl_->clientService_ = impl_->GetServicePtr();
                break;
            case BTSystemState::OFF:
                impl_->clientService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothGattClientServer::impl *impl_;
};

namespace {

bool CheckGattClientPermission(uint64_t tokenId, int sdkVersion)
{
    if (PermissionManager::IsNativeCaller(tokenId) || sdkVersion >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
            false, "[PERMISSION] Check ACCESS_BLUETOOTH permission failed, tokenId:%{public}lu", tokenId);
    } else {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::VerifyPermission(USE_BLUETOOTH, tokenId),
            false, "[PERMISSION] Check USE_BLUETOOTH permission failed, tokenId:%{public}lu", tokenId);
    }
    return true;
}
}  // namespace {}

class BluetoothGattClientServer::impl::GattClientCallbackImpl : public IGattClientCallback {
public:
    __attribute__((no_sanitize("cfi")))
    void OnConnectionStateChanged(int state, int newState, const RawAddress &addr, int disconnectReason,
        const std::string &reasonMessage) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("curState(%{public}d) -> newState(%{public}d)", state, newState);
        if (!CheckGattClientPermission(tokenId_, sdkVersion_)) {
            return;
        }
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        if (state == static_cast<int>(BTConnectState::CONNECTED) ||
            state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_CLIENT_CONN_STATE",
                OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid, "STATE", state);
        }
        if (callback_ == nullptr) {
            HILOGE("callback is nullptr.");
            return;
        }
        callback_->OnConnectionStateChanged(state, newState, disconnectReason, reasonMessage);
        if (newState == static_cast<int>(BTConnectState::CONNECTED)) {
            BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(addr, true);
            BluetoothStateManager::GetInstance()->AddServerDeviceList(addr.GetAddress());
        }
        if (newState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            BluetoothStateManager::GetInstance()->RemoveServerDeviceList(addr.GetAddress());
        }
    }

    void OnCharacteristicChanged(const Characteristic &characteristic) override
    {
        if (!CheckGattClientPermission(tokenId_, sdkVersion_)) {
            return;
        }
#ifdef RES_SCHED_SUPPORT
        OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(GetPid(), GetUid(), "OnCharacteristicChanged");
#endif
        bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
            { .appName = GetAppName(), .appUid = GetUid(),
            .appPid = GetPid(), .businessType = bluetooth::GATT_TRANSFER });
        callback_->OnCharacteristicChanged((BluetoothGattCharacteristic)characteristic);
    }

    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {
        HILOGI("ret: %{public}d", ret);
        callback_->OnCharacteristicRead(ret, (BluetoothGattCharacteristic)characteristic);
    }

    void OnCharacteristicWrite(int ret, const Characteristic &characteristic,
        const BluetoothGattRspContext &rspContext) override
    {
        HILOGD("ret: %{public}d", ret);
        callback_->OnCharacteristicWrite(ret, (BluetoothGattCharacteristic)characteristic, rspContext);
    }

    void OnDescriptorRead(int ret, const Descriptor &descriptor) override
    {
        HILOGI("ret: %{public}d", ret);
        callback_->OnDescriptorRead(ret, (BluetoothGattDescriptor)descriptor);
    }

    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override
    {
        HILOGI("ret: %{public}d", ret);
        callback_->OnDescriptorWrite(ret, (BluetoothGattDescriptor)descriptor);
    }

    void OnMtuChanged(int state, int mtu) override
    {
        HILOGI("state: %{public}d, mtu: %{public}d", state, mtu);
        if (PermissionManager::IsNativeCaller(tokenId_) || sdkVersion_ >= API_VERSION_10) {
            CHECK_AND_RETURN_LOG(
                PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId_),
                "[PERMISSION] Check ACCESS_BLUETOOTH permission failed, tokenId:0x%{public}lx", tokenId_);
        }
        callback_->OnMtuChanged(state, mtu);
    }

    void OnReadRemoteRssiValue(const RawAddress &addr, int rssi, int status) override
    {
        HILOGI("address: %{public}s, rssi: %{public}d, status: %{public}d", GetEncryptAddr(addr.GetAddress()).c_str(),
            rssi, status);
        RawAddress randomAddr = addr;
        if (!isUseRealAddrFlag_) {
            BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
        }
        callback_->OnReadRemoteRssiValue(randomAddr, rssi, status);
    }

    void OnServicesDiscovered(int status) override
    {
        HILOGI("status: %{public}d", status);
        callback_->OnServicesDiscovered(status);
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {
        HILOGI("interval: %{public}d, latency: %{public}d, timeout: %{public}d, status: %{public}d",
            interval, latency, timeout, status);
        callback_->OnConnectionParameterChanged(interval, latency, timeout, status);
    }

    void OnServicesChanged() override
    {
        callback_->OnServicesChanged();
    }

    void OnBlePhyUpdate(int txPhy, int rxPhy, int status) override
    {
        HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
        callback_->OnBlePhyUpdate(txPhy, rxPhy, status);
    }
 
    void OnBlePhyRead(int txPhy, int rxPhy, int status) override
    {
        HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
        callback_->OnBlePhyRead(txPhy, rxPhy, status);
    }

    sptr<IBluetoothGattClientCallback> GetCallback()
    {
        return callback_;
    }

    void SetAppId(int appId)
    {
        applicationId_ = appId;
    }

    int GetAppId()
    {
        return applicationId_;
    }

    void SetUid(int32_t uid)
    {
        uid_ = uid;
    }

    int GetUid()
    {
        return uid_;
    }

    void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    int GetPid()
    {
        return pid_;
    }

    void SetAppName(std::string name)
    {
        appName_ = name;
    }

    std::string GetAppName()
    {
        return appName_;
    }

    void SetIsUseRealAddrFlag(bool flag)
    {
        isUseRealAddrFlag_ = flag;
    }

    GattClientCallbackImpl(const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner);
    ~GattClientCallbackImpl() override
    {
        if (!callback_->AsObject()->RemoveDeathRecipient(deathRecipient_)) {
            HILOGE("Failed to unlink death recipient to callback");
        }
        callback_ = nullptr;
        deathRecipient_ = nullptr;
    };

private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallbackDeathRecipient(const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner);

        sptr<IBluetoothGattClientCallback> GetCallback() const
        {
            return callback_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<IBluetoothGattClientCallback> callback_;
        BluetoothGattClientServer &owner_;
    };

    sptr<IBluetoothGattClientCallback> callback_;
    sptr<CallbackDeathRecipient> deathRecipient_;
    int applicationId_;
    uint64_t tokenId_;
    int sdkVersion_;  // JS application api sdk version
    bool isUseRealAddrFlag_;
    int32_t uid_ = -1;
    int32_t pid_ = -1;
    std::string appName_ = "";
};

BluetoothGattClientServer::impl::GattClientCallbackImpl::GattClientCallbackImpl(
    const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner)
    : callback_(callback), deathRecipient_(new CallbackDeathRecipient(callback, owner))
{
    if (!callback_->AsObject()->AddDeathRecipient(deathRecipient_)) {
        HILOGE("Failed to link death recipient to callback");
    }
    tokenId_ = IPCSkeleton::GetCallingFullTokenID();
    sdkVersion_ = PermissionManager::GetApiVersion();
}

BluetoothGattClientServer::impl::GattClientCallbackImpl::CallbackDeathRecipient::CallbackDeathRecipient(
    const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner)
    : callback_(callback), owner_(owner)
{}

void BluetoothGattClientServer::impl::GattClientCallbackImpl::CallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lck(owner_.pimpl->registerMutex_);
    if (owner_.pimpl == nullptr || owner_.pimpl->clientService_ == nullptr) {
        HILOGE("gattClientServerImpl clientService_ is not support.");
        return;
    }
    for (auto it = owner_.pimpl->callbacks_.begin(); it != owner_.pimpl->callbacks_.end(); ++it) {
        if ((*it)->GetCallback()->AsObject() == remote) {
            HILOGI("callback is found from callbacks");
            sptr<CallbackDeathRecipient> dr = (*it)->deathRecipient_;
            if (!dr->GetCallback()->AsObject()->RemoveDeathRecipient(dr)) {
                HILOGE("Failed to unlink death recipient from callback");
            }
            HILOGI("App id is %{public}d", (*it)->GetAppId());
            owner_.pimpl->clientService_->Disconnect((*it)->GetAppId());
            owner_.pimpl->clientService_->DeregisterApplication((*it)->GetAppId());
#ifdef RES_SCHED_SUPPORT
            std::unordered_map<std::string, std::string> payload = {
                {"ACTION", "deregister"},
                {"SIDE", "client"},
                {"ADDRESS", "empty"},
                {"PID", std::to_string((*it)->GetPid())},
                {"UID", std::to_string((*it)->GetUid())},
                {"APPID", std::to_string((*it)->GetAppId())},
            };
            ResourceSchedule::ResSchedClient::GetInstance().ReportData(
                OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
                OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
                payload);
#endif
            owner_.pimpl->callbacks_.erase(it);
            return;
        }
    }
    HILOGE("No callback erased from callbacks");
}

BluetoothGattClientServer::impl::impl() : clientService_(nullptr), systemStateObserver_(new SystemStateObserver(this))
{
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattClientServer::impl::~impl()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattClientServer::BluetoothGattClientServer() : pimpl(new impl())
{
}

BluetoothGattClientServer::~BluetoothGattClientServer()
{}

int BluetoothGattClientServer::RegisterApplication(
    const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport)
{
    int appId = 0;
    int ret = RegisterApplication(callback, addr, transport, appId);
    return (ret == BT_NO_ERROR) ? appId : ret;
}

int BluetoothGattClientServer::RegisterApplication(
    const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport, int &appId)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("address: %{public}s, transport: %{public}d", GetEncryptAddr(addr.GetAddress()).c_str(), transport);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->clientService_ = pimpl->GetServicePtr();
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto it = pimpl->callbacks_.emplace(
        pimpl->callbacks_.begin(), std::make_shared<impl::GattClientCallbackImpl>(callback, *this));

    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr);
    (*it)->SetIsUseRealAddrFlag(realAddr == addr);
    appId = pimpl->clientService_->RegisterSharedApplication(*it, realAddr, transport);
    if (appId >= 0) {
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        std::string name = PermissionManager::GetCallingName();
        (*it)->SetAppId(appId);
        (*it)->SetUid(uid);
        (*it)->SetPid(pid);
        (*it)->SetAppName(name);
        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_APP_REGISTER",
            OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ACTION", "register",
            "SIDE", "client", "ADDRESS", GetEncryptAddr(realAddr.GetAddress()), "PID",
            pid, "UID", uid, "APPID", appId);
#ifdef RES_SCHED_SUPPORT
        std::unordered_map<std::string, std::string> payload;
        payload["ACTION"] = "register";
        payload["SIDE"] = "client";
        payload["ADDRESS"] = GetEncryptAddr(realAddr.GetAddress());
        payload["PID"] = std::to_string(pid);
        payload["UID"] = std::to_string(uid);
        payload["APPID"] = std::to_string(appId);
        ResourceSchedule::ResSchedClient::GetInstance().ReportData(
            OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
            OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
            payload);
#endif
    } else {
        HILOGE("RegisterSharedApplication failed, appId: %{public}d", appId);
        pimpl->callbacks_.erase(it);
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientServer::DeregisterApplication(int32_t appId)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_APP_REGISTER",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ACTION", "deregister",
        "SIDE", "client", "ADDRESS", "empty", "PID", OHOS::IPCSkeleton::GetCallingPid(),
        "UID", OHOS::IPCSkeleton::GetCallingUid(), "APPID", appId);
#ifdef RES_SCHED_SUPPORT
    std::unordered_map<std::string, std::string> payload;
    payload["ACTION"] = "deregister";
    payload["SIDE"] = "client";
    payload["ADDRESS"] = "empty";
    payload["PID"] = std::to_string(OHOS::IPCSkeleton::GetCallingPid());
    payload["UID"] = std::to_string(OHOS::IPCSkeleton::GetCallingUid());
    payload["APPID"] = std::to_string(appId);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
        OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
        payload);
#endif
    auto it = std::find_if(pimpl->callbacks_.begin(), pimpl->callbacks_.end(),
        [appId](const auto &p) { return p->GetAppId() == appId; });
    if (it == pimpl->callbacks_.end()) {
        HILOGE("Unknown appId: %{public}d", appId);
        return INVALID_PARAMETER;
    }

    int ret = pimpl->clientService_->DeregisterApplication(appId);
    pimpl->callbacks_.erase(it);
    return ret;
}

int BluetoothGattClientServer::Connect(int32_t appId, bool autoConnect)
{
    HILOGI("appId: %{public}d, autoConnect: %{public}d", appId, autoConnect);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    OHOS::Bluetooth::BluetoothHiTrace::BluetoothStartAsyncTrace("GATT_CLIENT_CONNECT", 1);
    int result = pimpl->clientService_->Connect(appId, autoConnect);
    OHOS::Bluetooth::BluetoothHiTrace::BluetoothFinishAsyncTrace("GATT_CLIENT_CONNECT", 1);
    return result;
}

int BluetoothGattClientServer::Disconnect(int32_t appId)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->Disconnect(appId);
}

int BluetoothGattClientServer::DiscoveryServices(int32_t appId)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->DiscoveryServices(appId);
}

int BluetoothGattClientServer::ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic &characteristic)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
#ifdef RES_SCHED_SUPPORT
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "ReadCharacteristic");
#endif
    std::string appName = PermissionManager::GetCallingName();
    bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
        { .appName = appName, .appUid = uid, .appPid = pid, .businessType = bluetooth::GATT_TRANSFER });
    return pimpl->clientService_->ReadCharacteristic(appId, (Characteristic)characteristic);
}

int BluetoothGattClientServer::ReadCharacteristicByUuid(int32_t appId, const std::string &uuid,
    int32_t startHandle, int32_t endHandle)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
#ifdef RES_SCHED_SUPPORT
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "ReadUsingCharacteristicUuid");
#endif
    Uuid inUuid = Uuid::ConvertFromString(uuid);
    return pimpl->clientService_->ReadCharacteristicByUuid(appId, inUuid, startHandle, endHandle);
}

int BluetoothGattClientServer::WriteCharacteristic(int32_t appId, BluetoothGattCharacteristic *characteristic,
    bool withoutRespond, bool isWithContext)
{
    if (isWithContext && !PermissionManager::IsSystemHap()) {
        // WriteCharacteristicWithContext is a private solution for smartLock product and system HAPs.
        HILOGE("Non-system applications are not allowed to use system APIs.");
        return BT_ERR_SYSTEM_PERMISSION_FAILED;
    }
    Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_GATT_CLIENT_ERROR;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
#ifdef RES_SCHED_SUPPORT
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "WriteCharacteristic");
#endif
    std::string appName = PermissionManager::GetCallingName();
    bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
        { .appName = appName, .appUid = uid, .appPid = pid, .businessType = bluetooth::GATT_TRANSFER });
    return pimpl->clientService_->WriteCharacteristic(appId, character, withoutRespond);
}
int BluetoothGattClientServer::SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic *characteristic)
{
    HILOGI("appId: %{public}d", appId);
    Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->SignedWriteCharacteristic(appId, character);
}

int BluetoothGattClientServer::ReadDescriptor(int32_t appId, const BluetoothGattDescriptor &descriptor)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
#ifdef RES_SCHED_SUPPORT
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "ReadDescriptor");
#endif
    std::string appName = PermissionManager::GetCallingName();
    bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
        { .appName = appName, .appUid = uid, .appPid = pid, .businessType = bluetooth::GATT_TRANSFER });
    return pimpl->clientService_->ReadDescriptor(appId, (Descriptor)descriptor);
}

int BluetoothGattClientServer::WriteDescriptor(int32_t appId, BluetoothGattDescriptor *descriptor)
{
    HILOGI("appId: %{public}d", appId);
    Descriptor desc(descriptor->handle_);
    desc.length_ = descriptor->length_;
    desc.value_ = std::move(descriptor->value_);
    descriptor->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
#ifdef RES_SCHED_SUPPORT
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "WriteDescriptor");
#endif
    std::string appName = PermissionManager::GetCallingName();
    bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
        { .appName = appName, .appUid = uid, .appPid = pid, .businessType = bluetooth::GATT_TRANSFER });
    return pimpl->clientService_->WriteDescriptor(appId, desc);
}

int BluetoothGattClientServer::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    HILOGD("appId: %{public}d, mtu: %{public}d", appId, mtu);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->RequestExchangeMtu(appId, mtu);
}

void BluetoothGattClientServer::GetAllDevice(::std::vector<BluetoothGattDevice> &device)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return;
    }
    for (auto &dev : pimpl->clientService_->GetAllDevice()) {
        device.push_back(dev);
    }
}

int BluetoothGattClientServer::RequestConnectionPriority(int32_t appId, int32_t connPriority)
{
    HILOGI("appId: %{public}d, connPriority: %{public}d", appId, connPriority);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return 0;
    }
    return pimpl->clientService_->RequestConnectionPriority(appId, connPriority);
}

int BluetoothGattClientServer::GetServices(int32_t appId, ::std::vector<BluetoothGattService> &service)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
#ifdef RES_SCHED_SUPPORT
    OHOS::bluetooth::FreezeUtil::GetInstance()->RequestActive(pid, uid, "GetServices");
#endif
    std::string appName = PermissionManager::GetCallingName();
    bluetooth::BtChrAppBehaviorReporter::GetInstance().ReportAppBehavior(
        { .appName = appName, .appUid = uid, .appPid = pid, .businessType = bluetooth::GATT_TRANSFER });
    for (auto &svc : pimpl->clientService_->GetServices(appId)) {
        service.push_back(svc);
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientServer::RequestFastestConn(const BluetoothRawAddress &addr)
{
    HILOGI("address: %{public}s", GetEncryptAddr(addr.GetAddress()).c_str());
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->clientService_ = pimpl->GetServicePtr();
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->RequestFastestConn(addr);
}

int BluetoothGattClientServer::ReadRemoteRssiValue(int32_t appId)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return 0;
    }
    return pimpl->clientService_->ReadRemoteRssiValue(appId);
}

int BluetoothGattClientServer::RequestNotification(int32_t appId, uint16_t characterhandle, bool enable)
{
    HILOGI("appId: %{public}d, characterhandle: %{public}u, enable: %{public}d", appId, characterhandle, enable);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl->clientService_->RequestNotification(appId, characterhandle, enable) != GattStatus::GATT_SUCCESS) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientServer::GetConnectedState(const std::string &deviceId, int &state)
{
    RawAddress address(deviceId);
    RawAddress realUsedAddress = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(address);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->clientService_) {
        HILOGI("clientService_ is null");
        state = static_cast<int>(BTConnectState::DISCONNECTED);
        return BT_NO_ERROR;
    }
    state = pimpl->clientService_->GetConnectedState(realUsedAddress);
    return BT_NO_ERROR;
}

int BluetoothGattClientServer::SetPhy(int32_t appId, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->SetPhy(appId, txPhy, rxPhy, phyOptions);
}
 
int BluetoothGattClientServer::ReadPhy(int32_t appId)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGE("request not support.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->clientService_->ReadPhy(appId);
}
}  // namespace Bluetooth
}  // namespace OHOS
