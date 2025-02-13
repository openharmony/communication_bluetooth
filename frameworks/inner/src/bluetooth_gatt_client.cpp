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
#define LOG_TAG "bt_fwk_gatt_client"
#endif

#include <condition_variable>
#include <memory>
#include <set>
#include <thread>
#include "bluetooth_def.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_client_proxy.h"
#include "bluetooth_gatt_client_callback_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "gatt_data.h"
#include "i_bluetooth_gatt_client.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
#define WPTR_GATT_CBACK(cbWptr, func, ...)      \
do {                                            \
    auto cbSptr = (cbWptr).lock();               \
    if (cbSptr) {                                \
        cbSptr->func(__VA_ARGS__);               \
    } else {                                     \
        HILOGE(#cbWptr ": callback is nullptr"); \
    }                                            \
} while (0)

constexpr uint8_t REQUEST_TYPE_CHARACTERISTICS_READ = 0x00;
constexpr uint8_t REQUEST_TYPE_CHARACTERISTICS_WRITE = 0x01;
constexpr uint8_t REQUEST_TYPE_DESCRIPTOR_READ = 0x02;
constexpr uint8_t REQUEST_TYPE_DESCRIPTOR_WRITE = 0x03;
constexpr uint8_t REQUEST_TYPE_SET_NOTIFY_CHARACTERISTICS = 0x04;
constexpr uint8_t REQUEST_TYPE_READ_REMOTE_RSSI_VALUE = 0x05;

constexpr const int WAIT_TIMEOUT = 10; // 10s
std::mutex g_gattClientProxyMutex;
struct DiscoverInfomation {
    struct Characteristics {
        bool isDiscoverDescCompleted_;
        Characteristics() : isDiscoverDescCompleted_(false)
        {}
    };

    struct Service {
        bool isDiscoverCompleted_;
        bool isDiscoverCharacteristicCompleted_;
        bool isDiscoverIncludeSvcCompleted_;
        uint16_t endHandle_;
        std::map<uint16_t, Characteristics> characteristics_;
        Service(uint16_t endHandle)
            : isDiscoverCompleted_(false),
              isDiscoverCharacteristicCompleted_(false),
              isDiscoverIncludeSvcCompleted_(false),
              endHandle_(endHandle)
        {}
    };
    bool isDiscovering_;
    bool needNotify_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::map<uint16_t, Service> service_;
    DiscoverInfomation() : isDiscovering_(false), needNotify_(false)
    {}
};

struct RequestInformation {
    bool doing_;
    uint8_t type_ = 0;
    std::mutex mutex_;
    RequestInformation() : doing_(false)
    {}
};

struct GattClient::impl {
    class BluetoothGattClientCallbackStubImpl;

    bool isGetServiceYet_;
    bool isRegisterSucceeded_;
    std::weak_ptr<GattClientCallback> callback_;
    int applicationId_;
    int connectionState_;
    BluetoothRemoteDevice device_;
    sptr<IBluetoothGattClient> proxy;
    sptr<BluetoothGattClientCallbackStubImpl> clientCallback_;
    std::vector<GattService> gattServices_;
    std::mutex gattServicesMutex_;
    std::mutex connStateMutex_;
    RequestInformation requestInformation_;
    DiscoverInfomation discoverInformation_;
    int32_t profileRegisterId = 0;

    explicit impl(const BluetoothRemoteDevice &device);
    ~impl();

    bool Init(std::weak_ptr<GattClient> client);

    int DiscoverStart();
    void DiscoverComplete(int state);
    void BuildServiceList(const std::vector<BluetoothGattService> &src);
    GattService *FindService(uint16_t handle);
    void GetServices();
    void CleanConnectionInfo();
    bool GetCharacteristicByHandle(uint16_t handle, GattCharacteristic &outCharac);
    bool GetDescriptorByHandle(uint16_t handle, GattDescriptor &outDesc);
};

class GattClient::impl::BluetoothGattClientCallbackStubImpl : public BluetoothGattClientCallbackStub {
public:
    void OnServicesChanged(std::vector<BluetoothGattService> &service) override
    {
        HILOGI("enter");
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        clientSptr->pimpl->DiscoverStart();
    }

    void OnConnectionStateChanged(int32_t state, int32_t newState) override
    {
        HILOGD("gattClient conn state, status: %{public}d, newState: %{public}s",
            state, GetProfileConnStateName(newState).c_str());
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        if (newState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            clientSptr->pimpl->CleanConnectionInfo();
        }

        {
            std::lock_guard<std::mutex> lck(clientSptr->pimpl->connStateMutex_);
            clientSptr->pimpl->connectionState_ = newState;
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnConnectionStateChanged, newState, state);
    }

    void OnCharacteristicChanged(const BluetoothGattCharacteristic &characteristic) override
    {
        HILOGD("recv notification, length:%{public}zu", characteristic.length_);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        std::lock_guard<std::mutex> lock(clientSptr->pimpl->gattServicesMutex_);
        for (auto &svc : clientSptr->pimpl->gattServices_) {
            for (auto &character : svc.GetCharacteristics()) {
                if (character.GetHandle() == characteristic.handle_) {
                    character.SetValue(characteristic.value_.get(), characteristic.length_);
                    WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnCharacteristicChanged, character);
                    return;
                }
            }
        }
        HILOGE("recv notification failed, characteristic is not exist.");
    }

    void OnCharacteristicRead(int32_t ret, const BluetoothGattCharacteristic &characteristic) override
    {
        HILOGI("ret:%{public}d, length:%{public}zu", ret, characteristic.length_);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        uint8_t requestType;
        {
            std::lock_guard<std::mutex> lock(clientSptr->pimpl->requestInformation_.mutex_);
            clientSptr->pimpl->requestInformation_.doing_ = false;
            requestType = clientSptr->pimpl->requestInformation_.type_;
        }
        GattCharacteristic charac(UUID(), 0, 0);
        bool isExist = clientSptr->pimpl->GetCharacteristicByHandle(characteristic.handle_, charac);
        if (!isExist) {
            HILOGE("no expected characteristic handle:%{public}d type:%{public}d",
                characteristic.handle_, requestType);
            ret = BT_ERR_INTERNAL_ERROR;
        }
        if (requestType != REQUEST_TYPE_CHARACTERISTICS_READ) {
            HILOGE("Unexpected call!");
            return;
        }
        if (ret == GattStatus::GATT_SUCCESS) {
            charac.SetValue(characteristic.value_.get(), characteristic.length_);
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnCharacteristicReadResult, charac, ret);
    }

    void OnCharacteristicWrite(int32_t ret, const BluetoothGattCharacteristic &characteristic) override
    {
        HILOGI("ret:%{public}d, length:%{public}zu", ret, characteristic.length_);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        uint8_t requestType;
        {
            std::lock_guard<std::mutex> lock(clientSptr->pimpl->requestInformation_.mutex_);
            clientSptr->pimpl->requestInformation_.doing_ = false;
            requestType = clientSptr->pimpl->requestInformation_.type_;
        }
        GattCharacteristic charac(UUID(), 0, 0);
        bool isExist = clientSptr->pimpl->GetCharacteristicByHandle(characteristic.handle_, charac);
        if (!isExist) {
            HILOGE("no expected characteristic handle:%{public}d type:%{public}d",
                characteristic.handle_, requestType);
            ret = BT_ERR_INTERNAL_ERROR;
        }
        if (requestType != REQUEST_TYPE_CHARACTERISTICS_WRITE) {
            HILOGE("Unexpected call!");
            return;
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnCharacteristicWriteResult, charac, ret);
    }

    void OnDescriptorRead(int32_t ret, const BluetoothGattDescriptor &descriptor) override
    {
        HILOGI("ret:%{public}d, length:%{public}zu", ret, descriptor.length_);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        uint8_t requestType;
        {
            std::lock_guard<std::mutex> lock(clientSptr->pimpl->requestInformation_.mutex_);
            clientSptr->pimpl->requestInformation_.doing_ = false;
            requestType = clientSptr->pimpl->requestInformation_.type_;
        }
        GattDescriptor desc(UUID(), 0);
        bool isExist = clientSptr->pimpl->GetDescriptorByHandle(descriptor.handle_, desc);
        if (!isExist) {
            HILOGE("no expected descriptor handle:%{public}d type:%{public}d",
                descriptor.handle_, requestType);
            ret = BT_ERR_INTERNAL_ERROR;
        }
        if (requestType != REQUEST_TYPE_DESCRIPTOR_READ) {
            HILOGE("Unexpected call!");
            return;
        }
        if (ret == GattStatus::GATT_SUCCESS) {
            desc.SetValue(descriptor.value_.get(), descriptor.length_);
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnDescriptorReadResult, desc, ret);
    }

    void OnDescriptorWrite(int32_t ret, const BluetoothGattDescriptor &descriptor) override
    {
        HILOGD("ret:%{public}d, length:%{public}zu", ret, descriptor.length_);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        uint8_t requestType;
        {
            std::lock_guard<std::mutex> lock(clientSptr->pimpl->requestInformation_.mutex_);
            clientSptr->pimpl->requestInformation_.doing_ = false;
            requestType = clientSptr->pimpl->requestInformation_.type_;
        }
        GattDescriptor desc(UUID(), 0);
        bool isExist = clientSptr->pimpl->GetDescriptorByHandle(descriptor.handle_, desc);
        if (!isExist) {
            HILOGE("no expected descriptor handle:%{public}d type:%{public}d",
                descriptor.handle_, requestType);
            ret = BT_ERR_INTERNAL_ERROR;
        }
        if (requestType == REQUEST_TYPE_DESCRIPTOR_WRITE) {
            WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnDescriptorWriteResult, desc, ret);
        } else if (requestType == REQUEST_TYPE_SET_NOTIFY_CHARACTERISTICS) {
            GattCharacteristic charac(UUID(), 0, 0);
            if (isExist && desc.GetCharacteristic() != nullptr) {
                charac = *desc.GetCharacteristic();
            }
            WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnSetNotifyCharacteristic, charac, ret);
        } else {
            HILOGE("Unexpected call!");
            return;
        }
    }

    void OnMtuChanged(int32_t state, int32_t mtu) override
    {
        HILOGI("state: %{public}d, mtu: %{public}d", state, mtu);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnMtuUpdate, mtu, state);
    }

    void OnServicesDiscovered(int32_t status) override
    {
        HILOGI("status: %{public}d", status);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        clientSptr->pimpl->DiscoverComplete(status);
    }

    void OnConnectionParameterChanged(int32_t interval, int32_t latency, int32_t timeout, int32_t status) override
    {
        HILOGD("interval: %{public}d, latency: %{public}d, timeout: %{public}d, status: %{public}d",
            interval, latency, timeout, status);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }

        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnConnectionParameterChanged, interval, latency, timeout, status);
    }

    void OnReadRemoteRssiValue(const bluetooth::RawAddress &addr, int32_t rssi, int32_t status) override
    {
        HILOGI("rssi: %{public}d, status: %{public}d", rssi, status);
        std::shared_ptr<GattClient> clientSptr = (client_).lock();
        if (!clientSptr) {
            HILOGE("callback client is nullptr");
            return;
        }
        uint8_t requestType;
        {
            std::lock_guard<std::mutex> lock(clientSptr->pimpl->requestInformation_.mutex_);
            clientSptr->pimpl->requestInformation_.doing_ = false;
            requestType = clientSptr->pimpl->requestInformation_.type_;
        }
        if (requestType != REQUEST_TYPE_READ_REMOTE_RSSI_VALUE) {
            HILOGE("Unexpected call!");
        }
        WPTR_GATT_CBACK(clientSptr->pimpl->callback_, OnReadRemoteRssiValueResult, rssi, status);
    }

    explicit BluetoothGattClientCallbackStubImpl(std::weak_ptr<GattClient> client) : client_(client)
    {}
    ~BluetoothGattClientCallbackStubImpl() override
    {}

private:
    std::weak_ptr<GattClient> client_;
};

bool GattClient::impl::Init(std::weak_ptr<GattClient> client)
{
    if (clientCallback_ != nullptr) {
        return true;
    }
    clientCallback_ = new BluetoothGattClientCallbackStubImpl(client);
    return true;
}

GattClient::impl::impl(const BluetoothRemoteDevice &device)
    : isGetServiceYet_(false),
      isRegisterSucceeded_(false),
      applicationId_(0),
      connectionState_(static_cast<int>(BTConnectState::DISCONNECTED)),
      device_(device)
{
    auto bluetoothTurnOffFunc = [this]() {
        applicationId_ = 0;
        isRegisterSucceeded_ = false;
    };
    ProfileFunctions profileFunctions = {
        .bluetoothLoadedfunc = nullptr,
        .bleTurnOnFunc = nullptr,
        .bluetoothTurnOffFunc = bluetoothTurnOffFunc,
    };
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(
        PROFILE_GATT_CLIENT, profileFunctions);
}

GattClient::impl::~impl()
{
    HILOGI("GattClient ~impl");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
}

int GattClient::impl::DiscoverStart()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    {
        std::unique_lock<std::mutex> lock(discoverInformation_.mutex_);
        while (discoverInformation_.isDiscovering_) {
            auto ret = discoverInformation_.condition_.wait_for(lock, std::chrono::seconds(WAIT_TIMEOUT));
            if (ret == std::cv_status::timeout) {
                HILOGE("timeout");
                return BT_ERR_INTERNAL_ERROR;
            }
        }
        discoverInformation_.isDiscovering_ = true;
    }

    if (!isRegisterSucceeded_) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = BT_ERR_INTERNAL_ERROR;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    if (!proxy) {
        HILOGE("proxy is null");
    } else {
        result = proxy->DiscoveryServices(applicationId_);
        if (result != BT_NO_ERROR) {
            DiscoverComplete(BT_ERR_INTERNAL_ERROR);
        }
    }
    return result;
}

void GattClient::impl::DiscoverComplete(int state)
{
    bool ret = false;
    {
        std::unique_lock<std::mutex> lock(discoverInformation_.mutex_);
        if (discoverInformation_.isDiscovering_) {
            discoverInformation_.isDiscovering_ = false;
            isGetServiceYet_ = false;
            discoverInformation_.condition_.notify_all();
            ret = true;
        }
    }
    if (ret) {
        std::shared_ptr<GattClientCallback> clientSptr = (callback_).lock();
        clientSptr->OnServicesDiscovered(state);
    }
}

void GattClient::impl::BuildServiceList(const std::vector<BluetoothGattService> &src)
{
    HILOGI("enter");
    for (auto &svc : src) {
        GattService svcTmp(UUID::ConvertFrom128Bits(svc.uuid_.ConvertTo128Bits()),
            svc.handle_,
            svc.endHandle_,
            svc.isPrimary_ ? GattServiceType::PRIMARY : GattServiceType::SECONDARY);
        for (auto &character : svc.characteristics_) {
            GattCharacteristic characterTmp(UUID::ConvertFrom128Bits(character.uuid_.ConvertTo128Bits()),
                character.handle_,
                character.permissions_,
                character.properties_);
            for (auto &desc : character.descriptors_) {
                characterTmp.AddDescriptor(GattDescriptor(
                    UUID::ConvertFrom128Bits(desc.uuid_.ConvertTo128Bits()), desc.handle_, desc.permissions_));
            }
            svcTmp.AddCharacteristic(std::move(characterTmp));
        }
        std::lock_guard<std::mutex> lock(gattServicesMutex_);
        gattServices_.emplace_back(std::move(svcTmp));
    }
    for (auto &svc : src) {
        GattService *ptr = FindService(svc.handle_);
        if (ptr == NULL) {
            return;
        }
        for (auto &isvc : svc.includeServices_) {
            GattService *iptr = FindService(isvc.startHandle_);
            if (iptr == nullptr) {
                return;
            }
            ptr->AddService(*iptr);
        }
    }
}

GattService *GattClient::impl::FindService(uint16_t handle)
{
    std::lock_guard<std::mutex> lock(gattServicesMutex_);
    for (auto &item : gattServices_) {
        if (item.GetHandle() == handle) {
            return &item;
        }
    }
    return nullptr;
}

void GattClient::impl::GetServices()
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    std::unique_lock<std::mutex> lock(discoverInformation_.mutex_);
    while (discoverInformation_.isDiscovering_) {
        auto ret = discoverInformation_.condition_.wait_for(lock, std::chrono::seconds(WAIT_TIMEOUT));
        if (ret == std::cv_status::timeout) {
            HILOGE("timeout");
            return;
        }
    }
    if (isGetServiceYet_) {
        HILOGD("isGetServiceYet_ is true");
        return;
    }
    if (!isRegisterSucceeded_) {
        HILOGE("isRegisterSucceeded_ is false");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(gattServicesMutex_);
        gattServices_.clear();
    }
    std::vector<BluetoothGattService> result;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    if (!proxy) {
        HILOGE("proxy is null");
    } else {
        proxy->GetServices(applicationId_, result);
        BuildServiceList(result);
        isGetServiceYet_ = true;
    }
}

void GattClient::impl::CleanConnectionInfo()
{
    DiscoverComplete(GattStatus::GATT_FAILURE);
    std::lock_guard<std::mutex> lock(requestInformation_.mutex_);
    requestInformation_.doing_ = false;
}

bool GattClient::impl::GetCharacteristicByHandle(uint16_t handle, GattCharacteristic &outCharac)
{
    std::lock_guard<std::mutex> lock(gattServicesMutex_);
    for (auto &svc : gattServices_) {
        std::vector<GattCharacteristic> &characs = svc.GetCharacteristics();
        for (auto &charac : characs) {
            if (handle == charac.GetHandle()) {
                outCharac = charac;
                return true;
            }
        }
    }
    return false;
}

bool GattClient::impl::GetDescriptorByHandle(uint16_t handle, GattDescriptor &outDesc)
{
    std::lock_guard<std::mutex> lock(gattServicesMutex_);
    auto getDescriptorFunc = [handle](std::vector<GattCharacteristic> &characs) -> GattDescriptor* {
        for (auto &charac : characs) {
            std::vector<GattDescriptor> &descs = charac.GetDescriptors();
            for (auto &desc : descs) {
                if (handle == desc.GetHandle()) {
                    return &desc;
                }
            }
        }
        return nullptr;
    };

    for (auto &svc : gattServices_) {
        GattDescriptor *descPtr = getDescriptorFunc(svc.GetCharacteristics());
        if (descPtr != nullptr) {
            outDesc = *descPtr;
            return true;
        }
    }
    return false;
}

GattClient::GattClient(const BluetoothRemoteDevice &device) : pimpl(new GattClient::impl(device))
{
    HILOGI("enter");
}

bool GattClient::Init()
{
    HILOGI("GattClient Init");
    return pimpl->Init(weak_from_this());
}

GattClient::~GattClient()
{
    HILOGI("~GattClient");
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    if (pimpl->isRegisterSucceeded_) {
        proxy->DeregisterApplication(pimpl->applicationId_);
    }
}

int GattClient::Connect(std::weak_ptr<GattClientCallback> callback, bool isAutoConnect, int transport)
{
    HILOGI("enter, isAutoConnect: %{public}d, transport: %{public}d", isAutoConnect, transport);
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lock(pimpl->connStateMutex_);
    if (pimpl->connectionState_ == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Already connected");
        return BT_ERR_INTERNAL_ERROR;
    }
    HILOGI("isRegisterSucceeded: %{public}d", pimpl->isRegisterSucceeded_);
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    if (pimpl->isRegisterSucceeded_) {
        return proxy->Connect(pimpl->applicationId_, isAutoConnect);
    }
    pimpl->callback_ = callback;
    if ((transport == GATT_TRANSPORT_TYPE_LE && !IS_BLE_ENABLED()) ||
        (transport == GATT_TRANSPORT_TYPE_CLASSIC && !IS_BT_ENABLED())) {
        HILOGE("Unsupported mode");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (transport == GATT_TRANSPORT_TYPE_CLASSIC && isAutoConnect) {
        HILOGE("Unsupported mode");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!pimpl->device_.IsValidBluetoothRemoteDevice()) {
        HILOGE("Invalid remote device");
        return BT_ERR_INTERNAL_ERROR;
    }

    int appId = 0;
    int32_t result = proxy->RegisterApplication(
        pimpl->clientCallback_, bluetooth::RawAddress(pimpl->device_.GetDeviceAddr()), transport, appId);
    HILOGI("Proxy register application : %{public}d", appId);
    if (result != BT_NO_ERROR) {
        HILOGE("register application fail");
        return result;
    }
    if (appId > 0) {
        pimpl->applicationId_ = appId;
        pimpl->isRegisterSucceeded_ = true;
        result = proxy->Connect(pimpl->applicationId_, isAutoConnect);
    }
    return result;
}

int GattClient::Disconnect()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lock(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = BT_ERR_INTERNAL_ERROR;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->Disconnect(pimpl->applicationId_);
    return result;
}

int GattClient::Close()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    if (pimpl->isRegisterSucceeded_) {
        int32_t result = proxy->DeregisterApplication(pimpl->applicationId_);
        HILOGI("result: %{public}d", result);
        if (result == BT_NO_ERROR) {
            pimpl->isRegisterSucceeded_ = false;
        }
        return result;
    }
    HILOGI("isRegisterSucceeded_ is false");
    return BT_NO_ERROR;
}

int GattClient::DiscoverServices()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lck(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->DiscoverStart();
}

std::optional<std::reference_wrapper<GattService>> GattClient::GetService(const UUID &uuid)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::nullopt;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return std::nullopt;
    }

    pimpl->GetServices();
    std::lock_guard<std::mutex> lock(pimpl->gattServicesMutex_);
    for (auto &svc : pimpl->gattServices_) {
        if (svc.GetUuid().Equals(uuid)) {
            HILOGD("successful");
            return svc;
        }
    }
    HILOGE("failed");
    return std::nullopt;
}

std::vector<GattService> &GattClient::GetService()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return pimpl->gattServices_;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return pimpl->gattServices_;
    }

    pimpl->GetServices();
    return pimpl->gattServices_;
}

int GattClient::ReadCharacteristic(GattCharacteristic &characteristic)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lock(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lck(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGE("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = GattStatus::GATT_FAILURE;
    HILOGI("applicationId: %{public}d, handle: 0x%{public}04X", pimpl->applicationId_, characteristic.GetHandle());
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->ReadCharacteristic(
        pimpl->applicationId_, (BluetoothGattCharacteristic)bluetooth::Characteristic(characteristic.GetHandle()));
    HILOGI("result: %{public}d", result);
    if (result == BT_NO_ERROR) {
        pimpl->requestInformation_.doing_ = true;
        pimpl->requestInformation_.type_ = REQUEST_TYPE_CHARACTERISTICS_READ;
    }
    return result;
}

int GattClient::ReadDescriptor(GattDescriptor &descriptor)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lck(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lock(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGE("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = BT_ERR_INTERNAL_ERROR;
    HILOGI("applicationId: %{public}d, handle: 0x%{public}04X", pimpl->applicationId_, descriptor.GetHandle());
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->ReadDescriptor(
        pimpl->applicationId_, (BluetoothGattDescriptor)bluetooth::Descriptor(descriptor.GetHandle()));
    HILOGI("result: %{public}d", result);
    if (result == BT_NO_ERROR) {
        pimpl->requestInformation_.doing_ = true;
        pimpl->requestInformation_.type_ = REQUEST_TYPE_DESCRIPTOR_READ;
    }
    return result;
}

int GattClient::RequestBleMtuSize(int mtu)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lck(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = BT_ERR_INTERNAL_ERROR;
    HILOGI("applicationId: %{public}d, mtu: %{public}d", pimpl->applicationId_, mtu);
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->RequestExchangeMtu(pimpl->applicationId_, mtu);
    HILOGI("result: %{public}d", result);
    return result;
}

int GattClient::SetNotifyCharacteristicInner(GattCharacteristic &characteristic, bool enable,
    const std::vector<uint8_t> &descriptorValue)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    int ret = proxy->RequestNotification(pimpl->applicationId_, characteristic.GetHandle(), enable);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    std::lock_guard<std::mutex> lockConn(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lock(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGI("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto descriptor = characteristic.GetDescriptor(UUID::FromString("00002902-0000-1000-8000-00805F9B34FB"));
    if (descriptor == nullptr) {
        HILOGE("descriptor not exist.");
        // some devices don't have this descriptor, call back to application
        std::thread([this, characteristic]() {
            std::lock_guard<std::mutex> lock(pimpl->requestInformation_.mutex_);
            WPTR_GATT_CBACK(pimpl->callback_, OnSetNotifyCharacteristic, characteristic, 0);
            }).detach();
        return BT_NO_ERROR;
    }
    BluetoothGattDescriptor desc(bluetooth::Descriptor(
        descriptor->GetHandle(), descriptorValue.data(), descriptorValue.size()));
    int result = GattStatus::GATT_FAILURE;
    HILOGD("applicationId: %{public}d", pimpl->applicationId_);
    result = proxy->WriteDescriptor(pimpl->applicationId_, &desc);
    HILOGD("result: %{public}d", result);
    if (result == BT_NO_ERROR) {
        pimpl->requestInformation_.type_ = REQUEST_TYPE_SET_NOTIFY_CHARACTERISTICS;
        pimpl->requestInformation_.doing_ = true;
    }
    return result;
}

int GattClient::SetNotifyCharacteristic(GattCharacteristic &characteristic, bool enable)
{
    HILOGI("handle: 0x%{public}04X, enable: %{public}d", characteristic.GetHandle(), enable);
    std::vector<uint8_t> enableNotifyValue = {1, 0};
    std::vector<uint8_t> disableValue = {0, 0};
    return SetNotifyCharacteristicInner(characteristic, enable, (enable ? enableNotifyValue : disableValue));
}

int GattClient::SetNotifyCharacteristicV2(GattCharacteristic &characteristic, bool enable)
{
    HILOGI("handle: 0x%{public}04X, enable: %{public}d", characteristic.GetHandle(), enable);
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lockConn(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->RequestNotification(pimpl->applicationId_, characteristic.GetHandle(), enable);
}

int GattClient::SetIndicateCharacteristic(GattCharacteristic &characteristic, bool enable)
{
    HILOGI("handle: 0x%{public}04X, enable: %{public}d", characteristic.GetHandle(), enable);
    std::vector<uint8_t> enableIndicateValue = {2, 0};
    std::vector<uint8_t> disableValue = {0, 0};
    return SetNotifyCharacteristicInner(characteristic, enable, (enable ? enableIndicateValue : disableValue));
}

int GattClient::WriteCharacteristic(GattCharacteristic &characteristic)
{
    size_t length = 0;
    const uint8_t *pData = characteristic.GetValue(&length).get();
    std::vector<uint8_t> value(pData, pData + length);
    return WriteCharacteristic(characteristic, std::move(value));
}

int GattClient::WriteCharacteristic(GattCharacteristic &characteristic, std::vector<uint8_t> value)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lockConn(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    size_t length = value.size();
    HILOGD("length:%{public}zu", length);
    if (length == 0) {
        HILOGE("Invalid parameters");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lock(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGE("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothGattCharacteristic character(
        bluetooth::Characteristic(characteristic.GetHandle(), value.data(), length));
    int result = BT_ERR_INTERNAL_ERROR;
    bool withoutRespond = true;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    if (characteristic.GetWriteType() == static_cast<int>(GattCharacteristic::WriteType::SIGNED)) {
        HILOGI("Signed write");
        result = proxy->SignedWriteCharacteristic(pimpl->applicationId_, &character);
    } else {
        withoutRespond = ((characteristic.GetWriteType() ==
            static_cast<int>(GattCharacteristic::WriteType::DEFAULT)) ? false : true);
        HILOGD("Write without response: %{public}d", withoutRespond);
        pimpl->requestInformation_.type_ = REQUEST_TYPE_CHARACTERISTICS_WRITE;
        // if withoutRespond is true, no need wait for callback
        pimpl->requestInformation_.doing_ = (!withoutRespond);
        result = proxy->WriteCharacteristic(pimpl->applicationId_, &character, withoutRespond);
    }
    if (result != GattStatus::GATT_SUCCESS) {
        HILOGE("Write failed, ret: %{public}d", result);
        pimpl->requestInformation_.doing_ = false;
    }
    return result;
}

int GattClient::WriteDescriptor(GattDescriptor &descriptor)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lck(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    size_t length = 0;
    auto &characterValue = descriptor.GetValue(&length);
    if (characterValue == nullptr || length == 0) {
        HILOGE("Invalid parameters");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lock(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGE("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = BT_ERR_INTERNAL_ERROR;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    BluetoothGattDescriptor desc(bluetooth::Descriptor(descriptor.GetHandle(), characterValue.get(), length));
    result = proxy->WriteDescriptor(pimpl->applicationId_, &desc);
    HILOGI("result: %{public}d", result);
    if (result == BT_NO_ERROR) {
        pimpl->requestInformation_.doing_ = true;
        pimpl->requestInformation_.type_ = REQUEST_TYPE_DESCRIPTOR_WRITE;
    }
    return result;
}

int GattClient::RequestConnectionPriority(int connPriority)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lockConn(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("Not connected");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    if (connPriority != static_cast<int>(GattConnectionPriority::BALANCED) &&
        connPriority != static_cast<int>(GattConnectionPriority::HIGH) &&
        connPriority != static_cast<int>(GattConnectionPriority::LOW_POWER)) {
        HILOGE("Invalid parameters");
        return GattStatus::INVALID_PARAMETER;
    }
    int result = GattStatus::GATT_FAILURE;
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->RequestConnectionPriority(pimpl->applicationId_, connPriority);
    HILOGI("result: %{public}d", result);
    return result;
}

int GattClient::RequestFastestConn()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    std::lock_guard<std::mutex> lock(pimpl->connStateMutex_);
    return proxy->RequestFastestConn(bluetooth::RawAddress(pimpl->device_.GetDeviceAddr()));
}
int GattClient::ReadRemoteRssiValue()
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->Init(weak_from_this())) {
        HILOGE("pimpl or gatt client proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::lock_guard<std::mutex> lock(pimpl->connStateMutex_);
    if (pimpl->connectionState_ != static_cast<int>(BTConnectState::CONNECTED) || !pimpl->isRegisterSucceeded_) {
        HILOGE("Request not supported");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::lock_guard<std::mutex> lck(pimpl->requestInformation_.mutex_);
    if (pimpl->requestInformation_.doing_) {
        HILOGE("Remote device busy");
        return BT_ERR_INTERNAL_ERROR;
    }
    int result = GattStatus::GATT_FAILURE;
    HILOGI("applicationId: %{public}d", pimpl->applicationId_);
    sptr<IBluetoothGattClient> proxy = GetRemoteProxy<IBluetoothGattClient>(PROFILE_GATT_CLIENT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    result = proxy->ReadRemoteRssiValue(pimpl->applicationId_);
    HILOGI("result: %{public}d", result);
    if (result == BT_NO_ERROR) {
        pimpl->requestInformation_.doing_ = true;
        pimpl->requestInformation_.type_ = REQUEST_TYPE_READ_REMOTE_RSSI_VALUE;
    }
    return result;
}

}  // namespace Bluetooth
}  // namespace OHOS