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
#define LOG_TAG "bt_server_gatt_server"
#endif

#include "bluetooth_gatt_server_server.h"

#include "hisysevent.h"
#include "ipc_skeleton.h"

#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "gatt_data.h"
#include "i_bluetooth_gatt_server.h"
#include "interface_adapter_manager.h"
#include "interface_profile_gatt_server.h"
#include "interface_profile_manager.h"
#include "permission_manager.h"
#include "hitrace_meter.h"
#include "bluetooth_state_manager.h"
#ifdef RES_SCHED_SUPPORT
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
struct BluetoothGattServerServer::impl {
    class GattServerCallbackImpl;
    class SystemStateObserver;

    bluetooth::IProfileGattServer *serverService_;
    std::unique_ptr<SystemStateObserver> systemStateObserver_;
    std::list<std::shared_ptr<GattServerCallbackImpl>> callbacks_;
    std::mutex registerMutex_;

    impl();
    ~impl();

    bool ConvertBluetoothGattDevice(const BluetoothGattDevice &bluetoothDevice, bluetooth::GattDevice &device)
    {
        device = (bluetooth::GattDevice)bluetoothDevice;
        if (PermissionManager::IsUseRealAddr()) {
            return true;
        }
        RawAddress realAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(bluetoothDevice.addr_, realAddr);
        if (ret == RET_NO_EXIST) {
            HILOGE("addr not exist.");
            return false;
        }
        device.addr_ = realAddr;
        return true;
    }

    bluetooth::IProfileGattServer *GetServicePtr()
    {
        if (bluetooth::IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<bluetooth::IProfileGattServer *>(
            bluetooth::IProfileManager::GetInstance()->GetProfileService(bluetooth::PROFILE_NAME_GATT_SERVER));
    }
};
class BluetoothGattServerServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothGattServerServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        std::lock_guard<std::mutex> lck(impl_->registerMutex_);
        switch (state) {
            case bluetooth::BTSystemState::ON:
                impl_->serverService_ = impl_->GetServicePtr();
                break;
            case bluetooth::BTSystemState::OFF:
                impl_->serverService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothGattServerServer::impl *impl_;
};

namespace {

bool CheckGattServerPermission(uint64_t tokenId, int sdkVersion)
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

class BluetoothGattServerServer::impl::GattServerCallbackImpl : public bluetooth::IGattServerCallback {
public:
    bool ConvertGattDevice(const bluetooth::GattDevice &device, bool addFlag, BluetoothGattDevice &bluetoothDevice)
    {
        bluetoothDevice = (BluetoothGattDevice)device;
        if (isUseRealAddrFlag) {
            return true;
        }
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device.addr_, randomAddr);
        if (ret == RET_NO_EXIST) {
            if (addFlag) {
                randomAddr = BluetoothDeviceManager::GetInstance()->AddDeviceInfo(device.addr_);
                BluetoothDeviceManager::GetInstance()->SetDeviceRetentionFlag(device.addr_, true);
            } else {
                HILOGE("addr not exist.");
                return false;
            }
        }
        bluetoothDevice.addr_ = randomAddr;
        return true;
    }
    void OnCharacteristicReadRequest(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic) override
    {
        HILOGI("addr: %{public}s", GET_ENCRYPT_GATT_ADDR(device));
        if (!CheckGattServerPermission(tokenId_, sdkVersion_)) {
            return;
        }
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnCharacteristicReadRequest(
            bluetoothDevice, (BluetoothGattCharacteristic)characteristic);
    }
    void OnCharacteristicReadByUuidRequest(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic) override
    {
        HILOGI("addr: %{public}s", GET_ENCRYPT_GATT_ADDR(device));
    }
    void OnCharacteristicWriteRequest(const bluetooth::GattDevice &device,
        const bluetooth::Characteristic &characteristic, bool needRespones) override
    {
        HILOGI("addr: %{public}s, needRespones: %{public}d", GET_ENCRYPT_GATT_ADDR(device), needRespones);
        if (!CheckGattServerPermission(tokenId_, sdkVersion_)) {
            return;
        }
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnCharacteristicWriteRequest(
            bluetoothDevice, (BluetoothGattCharacteristic)characteristic, needRespones);
    }
    void OnDescriptorReadRequest(const bluetooth::GattDevice &device, const bluetooth::Descriptor &descriptor) override
    {
        HILOGI("addr: %{public}s", GET_ENCRYPT_GATT_ADDR(device));
        if (!CheckGattServerPermission(tokenId_, sdkVersion_)) {
            return;
        }
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnDescriptorReadRequest(bluetoothDevice, (BluetoothGattDescriptor)descriptor);
    }
    void OnDescriptorWriteRequest(const bluetooth::GattDevice &device, const bluetooth::Descriptor &descriptor) override
    {
        HILOGI("addr: %{public}s", GET_ENCRYPT_GATT_ADDR(device));
        if (!CheckGattServerPermission(tokenId_, sdkVersion_)) {
            return;
        }
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnDescriptorWriteRequest(bluetoothDevice, (BluetoothGattDescriptor)descriptor);
    }
    void OnNotifyConfirm(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic, int result) override
    {
        HILOGI("addr: %{public}s, result: %{public}d", GET_ENCRYPT_GATT_ADDR(device), result);
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnNotifyConfirm(bluetoothDevice, (BluetoothGattCharacteristic)characteristic, result);
    }
    void OnConnectionStateChanged(const bluetooth::GattDevice &device, int ret, int state, int disconnectReason,
        const std::string &reasonMessage) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGD("addr: %{public}s, ret: %{public}d, state: %{public}d", GET_ENCRYPT_GATT_ADDR(device), ret, state);
        if (!CheckGattServerPermission(tokenId_, sdkVersion_)) {
            return;
        }
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        if (state == static_cast<int>(BTConnectState::CONNECTED) ||
            state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_SERVER_CONN_STATE",
                OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pid, "UID", uid, "STATE", state);
        }
        if (callback_ == nullptr) {
            HILOGE("callback is nullptr.");
            return;
        }
        BluetoothGattDevice bluetoothDevice;
        bool addFlag = false;
        if (state == static_cast<int>(BTConnectState::CONNECTED)) {
            addFlag = true;
        }
        if (!ConvertGattDevice(device, addFlag, bluetoothDevice)) {
            return;
        }
        callback_->OnConnectionStateChanged(bluetoothDevice, ret, state, disconnectReason, reasonMessage);
        if (state == static_cast<int>(BTConnectState::CONNECTED)) {
            BluetoothStateManager::GetInstance()->AddClientDeviceList(device.addr_.GetAddress());
        }
        if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            BluetoothStateManager::GetInstance()->RemoveClientDeviceList(device.addr_.GetAddress());
        }
    }
    void OnMtuChanged(const bluetooth::GattDevice &device, int mtu) override
    {
        HILOGI("addr: %{public}s, mtu: %{public}d", GET_ENCRYPT_GATT_ADDR(device), mtu);
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnMtuChanged(bluetoothDevice, mtu);
    }
    void OnAddService(int ret, const bluetooth::Service &services) override
    {
        HILOGI("ret: %{public}d", ret);
        callback_->OnAddService(ret, (BluetoothGattService)services);
    }
    void OnServiceChanged(const bluetooth::Service &services) override
    {
    }
    void OnConnectionParameterChanged(
        const bluetooth::GattDevice &device, int interval, int latency, int timeout, int status) override
    {
        HILOGI("addr: %{public}s, interval: %{public}d, latency: %{public}d, timeout: %{public}d, status: %{public}d",
            GET_ENCRYPT_GATT_ADDR(device), interval, latency, timeout, status);
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnConnectionParameterChanged(bluetoothDevice, interval, latency, timeout, status);
    }
    void OnBlePhyUpdate(const bluetooth::GattDevice &device, int32_t txPhy, int32_t rxPhy, int32_t status) override
    {
        HILOGI("addr: %{public}s, txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d",
            GET_ENCRYPT_GATT_ADDR(device), txPhy, rxPhy, status);
        BluetoothGattDevice bluetoothDevice;
        if (!ConvertGattDevice(device, false, bluetoothDevice)) {
            return;
        }
        callback_->OnBlePhyUpdate(bluetoothDevice, txPhy, rxPhy, status);
    }
 
    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override
    {
        HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
        callback_->OnBlePhyRead(txPhy, rxPhy, status);
    }

    sptr<IBluetoothGattServerCallback> GetCallback()
    {
        return callback_;
    }

    void SetAppId(int32_t appId)
    {
        HILOGI("SetAppId = %{public}d", appId);
        appId_ = appId;
    }

    int32_t GetAppId()
    {
        return appId_;
    }

    void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    int32_t GetPid()
    {
        return pid_;
    }

    void SetUid(int32_t uid)
    {
        uid_ = uid;
    }

    int32_t GetUid()
    {
        return uid_;
    }

    GattServerCallbackImpl(const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner);
    ~GattServerCallbackImpl() override
    {
        if (!callback_->AsObject()->RemoveDeathRecipient(deathRecipient_)) {
            HILOGE("Failed to unlink death recipient to callback");
        }
        callback_ = nullptr;
        deathRecipient_ = nullptr;
    };

private:
    class GattServerCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        GattServerCallbackDeathRecipient(
            const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner);

        sptr<IBluetoothGattServerCallback> GetCallback() const
        {
            return callback_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<IBluetoothGattServerCallback> callback_;
        BluetoothGattServerServer &owner_;
    };

    sptr<IBluetoothGattServerCallback> callback_;
    sptr<GattServerCallbackDeathRecipient> deathRecipient_;
    uint64_t tokenId_;
    int32_t appId_;
    int pid_ = 0;
    int uid_ = 0;
    int sdkVersion_;  // JS application api sdk version
    bool isUseRealAddrFlag;
};

BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackImpl(
    const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner)
    : callback_(callback), deathRecipient_(new GattServerCallbackDeathRecipient(callback, owner))
{
    if (!callback_->AsObject()->AddDeathRecipient(deathRecipient_)) {
        HILOGE("Failed to link death recipient to callback");
    }
    tokenId_ = IPCSkeleton::GetCallingFullTokenID();
    sdkVersion_ = PermissionManager::GetApiVersion();
    appId_ = -1;
    isUseRealAddrFlag = PermissionManager::IsUseRealAddr();
}

BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackDeathRecipient
    ::GattServerCallbackDeathRecipient(const sptr<IBluetoothGattServerCallback> &callback,
    BluetoothGattServerServer &owner): callback_(callback), owner_(owner) {}

void BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    HILOGI("GattServerCallbackDeathRecipient OnRemoteDied start, list size = %{public}lu",
        static_cast<unsigned long>(owner_.pimpl->callbacks_.size()));
    std::lock_guard<std::mutex> lck(owner_.pimpl->registerMutex_);
    for (auto it = owner_.pimpl->callbacks_.begin(); it != owner_.pimpl->callbacks_.end(); ++it) {
        if ((*it) != nullptr && (*it)->GetCallback() != nullptr && (*it)->GetCallback()->AsObject() == remote) {
            int appId = (*it)->GetAppId();
            HILOGI("callback is erased from callbacks, appId: %{public}d", appId);
            sptr<GattServerCallbackDeathRecipient> dr = (*it)->deathRecipient_;
            if (!dr->GetCallback()->AsObject()->RemoveDeathRecipient(dr)) {
                HILOGE("Failed to unlink death recipient from callback");
            }
#ifdef RES_SCHED_SUPPORT
            std::unordered_map<std::string, std::string> payload = {
                {"ACTION", "deregister"},
                {"SIDE", "server"},
                {"ADDRESS", "empty"},
                {"PID", std::to_string((*it)->GetPid())},
                {"UID", std::to_string((*it)->GetUid())},
                {"APPID", std::to_string(appId)},
            };
            ResourceSchedule::ResSchedClient::GetInstance().ReportData(
                OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
                OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
                payload);
#endif
            if (owner_.pimpl->serverService_ != nullptr) {
                int ret = owner_.pimpl->serverService_->ClearServices(appId);
                ret = owner_.pimpl->serverService_->DeregisterApplication(appId);
                HILOGI("DeregisterApplication result:%{public}d, appId:%{public}d", ret, appId);
            }
            owner_.pimpl->callbacks_.erase(it);
            return;
        }
    }
}
BluetoothGattServerServer::impl::impl() : serverService_(nullptr), systemStateObserver_(new SystemStateObserver(this))
{
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattServerServer::impl::~impl()
{
    bluetooth::IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);
}

int BluetoothGattServerServer::AddService(int32_t appId, BluetoothGattService *services)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::Service svc = (bluetooth::Service)*services;

    int ret = pimpl->serverService_->AddService(appId, svc);
    return (ret == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::ClearServices(int appId)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = pimpl->serverService_->ClearServices(appId);
    return (ret == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::Connect(int appId, const BluetoothGattDevice &device, bool isDirect)
{
    HILOGI("appId: %{public}d, addr: %{public}s, isDirect: %{public}d", appId, GET_ENCRYPT_GATT_ADDR(device), isDirect);

    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->serverService_, BT_ERR_INTERNAL_ERROR, "serverService_ is null");

    bluetooth::GattDevice gattDevice;
    CHECK_AND_RETURN_LOG_RET(pimpl->ConvertBluetoothGattDevice(device, gattDevice), BT_ERR_INTERNAL_ERROR,
        "ConvertBluetoothGattDevice failed");

    return pimpl->serverService_->Connect(appId, gattDevice, isDirect);
}

int BluetoothGattServerServer::CancelConnection(int appId, const BluetoothGattDevice &device)
{
    HILOGI("appId: %{public}d, addr: %{public}s", appId, GET_ENCRYPT_GATT_ADDR(device));

    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->serverService_, BT_ERR_INTERNAL_ERROR, "serverService_ is null");

    bluetooth::GattDevice gattDevice;
    CHECK_AND_RETURN_LOG_RET(pimpl->ConvertBluetoothGattDevice(device, gattDevice), BT_ERR_INTERNAL_ERROR,
        "ConvertBluetoothGattDevice failed");

    return pimpl->serverService_->CancelConnection(appId, gattDevice);
}

int BluetoothGattServerServer::NotifyClient(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, bool needConfirm)
{
    HILOGI("addr: %{public}s, needConfirm: %{public}d", GET_ENCRYPT_GATT_ADDR(device), needConfirm);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;

    bluetooth::GattDevice gattDevice;
    if (!pimpl->ConvertBluetoothGattDevice(device, gattDevice)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = pimpl->serverService_->NotifyClient(gattDevice, character, needConfirm);
    return (ret == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RemoveService(int32_t appId, const BluetoothGattService &services)
{
    HILOGI("appId: %{public}d", appId);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = pimpl->serverService_->RemoveService(appId, (bluetooth::Service)services);
    return (ret == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RespondCharacteristicRead(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, int32_t ret)
{
    HILOGI("addr: %{public}s, ret: %{public}d", GET_ENCRYPT_GATT_ADDR(device), ret);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;

    bluetooth::GattDevice gattDevice;
    if (!pimpl->ConvertBluetoothGattDevice(device, gattDevice)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = pimpl->serverService_->RespondCharacteristicRead(gattDevice, character, ret);
    return (result == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RespondCharacteristicWrite(
    const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int32_t ret)
{
    HILOGI("addr: %{public}s, ret: %{public}d", GET_ENCRYPT_GATT_ADDR(device), ret);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    bluetooth::GattDevice gattDevice;
    if (!pimpl->ConvertBluetoothGattDevice(device, gattDevice)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = pimpl->serverService_->RespondCharacteristicWrite(
        gattDevice, (bluetooth::Characteristic)characteristic, ret);
    return (result == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RespondDescriptorRead(
    const BluetoothGattDevice &device, BluetoothGattDescriptor *descriptor, int32_t ret)
{
    HILOGI("addr: %{public}s, ret: %{public}d", GET_ENCRYPT_GATT_ADDR(device), ret);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::Descriptor desc(descriptor->handle_);
    desc.length_ = descriptor->length_;
    desc.value_ = std::move(descriptor->value_);
    descriptor->length_ = 0;

    bluetooth::GattDevice gattDevice;
    if (!pimpl->ConvertBluetoothGattDevice(device, gattDevice)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = pimpl->serverService_->RespondDescriptorRead(gattDevice, desc, ret);
    return (result == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RespondDescriptorWrite(
    const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor, int32_t ret)
{
    HILOGI("addr: %{public}s, ret: %{public}d", GET_ENCRYPT_GATT_ADDR(device), ret);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    bluetooth::GattDevice gattDevice;
    if (!pimpl->ConvertBluetoothGattDevice(device, gattDevice)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = pimpl->serverService_->RespondDescriptorWrite(gattDevice, (bluetooth::Descriptor)descriptor, ret);
    return (result == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}

int BluetoothGattServerServer::RegisterApplication(const sptr<IBluetoothGattServerCallback> &callback)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->serverService_ = pimpl->GetServicePtr();
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto it = pimpl->callbacks_.emplace(
        pimpl->callbacks_.begin(), std::make_shared<impl::GattServerCallbackImpl>(callback, *this));

    int ret = pimpl->serverService_->RegisterApplication(*it);
    if (ret >= 0) {
        HILOGI("appId, %{public}d", ret);
        int pid = OHOS::IPCSkeleton::GetCallingPid();
        int uid = OHOS::IPCSkeleton::GetCallingUid();
        (*it)->SetAppId(ret);
        (*it)->SetUid(uid);
        (*it)->SetPid(pid);
        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_APP_REGISTER",
            OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ACTION", "register",
            "SIDE", "server", "ADDRESS", "empty", "PID", pid, "UID", uid, "APPID", ret);
#ifdef RES_SCHED_SUPPORT
        std::unordered_map<std::string, std::string> payload;
        payload["ACTION"] = "register";
        payload["SIDE"] = "server";
        payload["ADDRESS"] = "empty";
        payload["PID"] = std::to_string(pid);
        payload["UID"] = std::to_string(uid);
        payload["APPID"] = std::to_string(ret);
        ResourceSchedule::ResSchedClient::GetInstance().ReportData(
            OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
            OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
            payload);
#endif
    } else {
        HILOGE("RegisterApplication failed, ret: %{public}d", ret);
        pimpl->callbacks_.erase(it);
    }
    return ret;
}

int BluetoothGattServerServer::DeregisterApplication(int32_t appId)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGE("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = pimpl->serverService_->DeregisterApplication(appId);
    HILOGI("appId: %{public}d, list size: %{public}lu", appId, (unsigned long)pimpl->callbacks_.size());
    for (auto it = pimpl->callbacks_.begin(); it != pimpl->callbacks_.end(); ++it) {
        if ((*it) != nullptr && (*it)->GetAppId() == appId) {
            HILOGI("erase appId: %{public}d", appId);
            pimpl->callbacks_.erase(it);
            break;
        }
    }
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_APP_REGISTER",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ACTION", "deregister",
        "SIDE", "server", "ADDRESS", "empty", "PID", OHOS::IPCSkeleton::GetCallingPid(),
        "UID", OHOS::IPCSkeleton::GetCallingUid(), "APPID", appId);
#ifdef RES_SCHED_SUPPORT
        std::unordered_map<std::string, std::string> payload;
        payload["ACTION"] = "deregister";
        payload["SIDE"] = "server";
        payload["ADDRESS"] = "empty";
        payload["PID"] = std::to_string(OHOS::IPCSkeleton::GetCallingPid());
        payload["UID"] = std::to_string(OHOS::IPCSkeleton::GetCallingUid());
        payload["APPID"] = std::to_string(appId);
        ResourceSchedule::ResSchedClient::GetInstance().ReportData(
            OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
            OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER,
            payload);
#endif
    return (ret == GattStatus::GATT_SUCCESS ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR);
}
BluetoothGattServerServer::BluetoothGattServerServer() : pimpl(new impl())
{
    HILOGI("Bluetooth Gatt Server Server Created!");
}

BluetoothGattServerServer::~BluetoothGattServerServer()
{}

int BluetoothGattServerServer::GetConnectedState(const std::string &deviceId, int &state)
{
    RawAddress address(deviceId);
    RawAddress realUsedAddress = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(address);
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        HILOGI("serverService_ is null");
        state = static_cast<int>(BTConnectState::DISCONNECTED);
        return BT_NO_ERROR;
    }
    state = pimpl->serverService_->GetConnectedState(realUsedAddress);
    return BT_NO_ERROR;
}

int BluetoothGattServerServer::SetPhy(int32_t appId,
    const std::string &deviceId, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    RawAddress address(deviceId);
    RawAddress realUsedAddress = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(address);
    if (!pimpl->serverService_) {
        HILOGI("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->serverService_->SetPhy(appId, realUsedAddress, txPhy, rxPhy, phyOptions);
}
 
int BluetoothGattServerServer::ReadPhy(int32_t appId, const std::string &deviceId)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    RawAddress address(deviceId);
    RawAddress realUsedAddress = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(address);
    if (!pimpl->serverService_) {
        HILOGI("serverService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->serverService_->ReadPhy(appId, realUsedAddress);
}
}  // namespace Bluetooth
}  // namespace OHOS