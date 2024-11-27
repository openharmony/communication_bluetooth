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
#define LOG_TAG "bt_fwk_gatt_server"
#endif

#include <condition_variable>
#include <memory>
#include <set>
#include "bluetooth_gatt_server.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_gatt_server_proxy.h"
#include "bluetooth_gatt_server_callback_stub.h"
#include "gatt_data.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
const int EIGHT_BITS = 8;
constexpr uint8_t REQUEST_TYPE_CHARACTERISTICS_READ = 0x00;
constexpr uint8_t REQUEST_TYPE_CHARACTERISTICS_WRITE = 0x01;
constexpr uint8_t REQUEST_TYPE_DESCRIPTOR_READ = 0x02;
constexpr uint8_t REQUEST_TYPE_DESCRIPTOR_WRITE = 0x03;

struct RequestInformation {
    uint8_t type_;
    bluetooth::GattDevice device_;
    union {
        GattCharacteristic *characteristic_;
        GattDescriptor *descriptor_;
    } context_;

    RequestInformation(uint8_t type, const bluetooth::GattDevice &device, GattCharacteristic *characteristic)
        : type_(type), device_(device), context_ {
            .characteristic_ = characteristic
        }
    {}

    RequestInformation(uint8_t type, const bluetooth::GattDevice &device, GattDescriptor *decriptor)
        : type_(type), device_(device), context_ {
            .descriptor_ = decriptor
        }
    {}

    RequestInformation(uint8_t type, const bluetooth::GattDevice &device) : type_(type), device_(device)
    {}

    bool operator==(const RequestInformation &rhs) const
    {
        return (device_ == rhs.device_ && type_ == rhs.type_);
    };

    bool operator<(const RequestInformation &rhs) const
    {
        return (device_ < rhs.device_ && type_ == rhs.type_);
    };
};

struct GattServer::impl {
    class BluetoothGattServerCallbackStubImpl;
    bool isRegisterSucceeded_ = false;
    std::mutex serviceListMutex_;
    std::mutex requestListMutex_;
    std::list<GattService> gattServices_;
    sptr<BluetoothGattServerCallbackStubImpl> serviceCallback_;
    std::optional<std::reference_wrapper<GattCharacteristic>> FindCharacteristic(uint16_t handle);
    std::optional<std::reference_wrapper<GattDescriptor>> FindDescriptor(uint16_t handle);
    std::set<RequestInformation> requests_;
    std::list<bluetooth::GattDevice> devices_;
    std::shared_ptr<GattServerCallback> callback_;
    int BuildRequestId(uint8_t type, uint8_t transport);
    int RespondCharacteristicRead(
        const bluetooth::GattDevice &device, uint16_t handle, const uint8_t *value, size_t length, int ret);
    int RespondCharacteristicWrite(const bluetooth::GattDevice &device, uint16_t handle, int ret);
    int RespondDescriptorRead(
        const bluetooth::GattDevice &device, uint16_t handle, const uint8_t *value, size_t length, int ret);
    int RespondDescriptorWrite(const bluetooth::GattDevice &device, uint16_t handle, int ret);
    int applicationId_ = 0;
    std::mutex deviceListMutex_;
    GattService *GetIncludeService(uint16_t handle);
    bluetooth::GattDevice *FindConnectedDevice(const BluetoothRemoteDevice &device);
    GattService BuildService(const BluetoothGattService &service);
    void BuildIncludeService(GattService &svc, const std::vector<bluetooth::Service> &iSvcs);
    impl(std::shared_ptr<GattServerCallback> callback);
    bool Init(std::weak_ptr<GattServer>);
    int32_t profileRegisterId = 0;
};

class GattServer::impl::BluetoothGattServerCallbackStubImpl : public BluetoothGattServerCallbackStub {
public:
    inline std::shared_ptr<GattServer> GetServerSptr(void)
    {
        auto serverSptr = server_.lock();
        if (!serverSptr) {
            HILOGE("server_ is nullptr");
            return nullptr;
        }
        if (!serverSptr->pimpl) {
            HILOGE("impl is nullptr");
            return nullptr;
        }
        return serverSptr;
    }
    void OnCharacteristicReadRequest(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic) override
    {
        HILOGI("remote device: %{public}s, handle: 0x%{public}04X",
            GET_ENCRYPT_GATT_ADDR(device), characteristic.handle_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(serverSptr->pimpl->serviceListMutex_);
        auto gattcharacter = serverSptr->pimpl->FindCharacteristic(characteristic.handle_);
        if (gattcharacter.has_value()) {
            {
                std::lock_guard<std::mutex> lck(serverSptr->pimpl->requestListMutex_);
                auto ret = serverSptr->pimpl->requests_.emplace(
                    RequestInformation(REQUEST_TYPE_CHARACTERISTICS_READ, device, &gattcharacter.value().get()));
                if (!ret.second) {
                    HILOGE("insert request failed, type: %{public}u, addr: %{public}s",
                        REQUEST_TYPE_CHARACTERISTICS_READ, GET_ENCRYPT_GATT_ADDR(device));
                }
            }

            if (serverSptr->pimpl->callback_) {
                serverSptr->pimpl->callback_->OnCharacteristicReadRequest(
                    BluetoothRemoteDevice(device.addr_.GetAddress(),
                        (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                    gattcharacter.value().get(),
                    serverSptr->pimpl->BuildRequestId(REQUEST_TYPE_CHARACTERISTICS_READ, device.transport_));
            }
            
            return;
        } else {
            HILOGE("Can not Find Characteristic!");
        }
        return;
    }

    void OnCharacteristicWriteRequest(const BluetoothGattDevice &device,
        const BluetoothGattCharacteristic &characteristic, bool needRespones) override
    {
        HILOGI("remote device: %{public}s, handle: 0x%{public}04X, length: %{public}zu",
            GET_ENCRYPT_GATT_ADDR(device), characteristic.handle_, characteristic.length_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(serverSptr->pimpl->serviceListMutex_);
        auto gattcharacter = serverSptr->pimpl->FindCharacteristic(characteristic.handle_);
        if (gattcharacter.has_value()) {
            gattcharacter.value().get().SetValue(characteristic.value_.get(), characteristic.length_);
            gattcharacter.value().get().SetWriteType(
                needRespones ? GattCharacteristic::WriteType::DEFAULT : GattCharacteristic::WriteType::NO_RESPONSE);

            if (needRespones) {
                std::lock_guard<std::mutex> lck(serverSptr->pimpl->requestListMutex_);
                auto ret = serverSptr->pimpl->requests_.emplace(
                    RequestInformation(REQUEST_TYPE_CHARACTERISTICS_WRITE, device, &gattcharacter.value().get()));
                if (!ret.second) {
                    HILOGE("insert request failed, type: %{public}u, addr: %{public}s",
                        REQUEST_TYPE_CHARACTERISTICS_WRITE, GET_ENCRYPT_GATT_ADDR(device));
                }
            }

            if (serverSptr->pimpl->callback_) {
                serverSptr->pimpl->callback_->OnCharacteristicWriteRequest(
                    BluetoothRemoteDevice(device.addr_.GetAddress(),
                        (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                    gattcharacter.value().get(),
                    serverSptr->pimpl->BuildRequestId(REQUEST_TYPE_CHARACTERISTICS_WRITE, device.transport_));
            }
            
            return;
        } else {
            HILOGE("Can not Find Characteristic!");
        }

        return;
    }

    void OnDescriptorReadRequest(const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) override
    {
        HILOGI("remote device: %{public}s, handle: 0x%{public}04X",
            GET_ENCRYPT_GATT_ADDR(device), descriptor.handle_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(serverSptr->pimpl->serviceListMutex_);
        auto gattdesc = serverSptr->pimpl->FindDescriptor(descriptor.handle_);
        if (gattdesc.has_value()) {
            {
                std::lock_guard<std::mutex> lck(serverSptr->pimpl->requestListMutex_);
                auto ret = serverSptr->pimpl->requests_.emplace(
                    RequestInformation(REQUEST_TYPE_DESCRIPTOR_READ, device, &gattdesc.value().get()));
                if (!ret.second) {
                    HILOGE("insert request failed, type: %{public}u, addr: %{public}s",
                        REQUEST_TYPE_DESCRIPTOR_READ, GET_ENCRYPT_GATT_ADDR(device));
                }
            }

            if (serverSptr->pimpl->callback_) {
                serverSptr->pimpl->callback_->OnDescriptorReadRequest(
                    BluetoothRemoteDevice(device.addr_.GetAddress(),
                        (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                    gattdesc.value().get(),
                    serverSptr->pimpl->BuildRequestId(REQUEST_TYPE_DESCRIPTOR_READ, device.transport_));
            }
        
            return;
        } else {
            HILOGE("Can not Find Descriptor!");
        }

        return;
    }

    void OnDescriptorWriteRequest(const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) override
    {
        HILOGI("remote device: %{public}s, handle: 0x%{public}04X",
            GET_ENCRYPT_GATT_ADDR(device), descriptor.handle_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(serverSptr->pimpl->serviceListMutex_);
        auto gattdesc = serverSptr->pimpl->FindDescriptor(descriptor.handle_);
        if (gattdesc.has_value()) {
            gattdesc.value().get().SetValue(descriptor.value_.get(), descriptor.length_);

            {
                std::lock_guard<std::mutex> lck(serverSptr->pimpl->requestListMutex_);
                auto ret = serverSptr->pimpl->requests_.emplace(
                    RequestInformation(REQUEST_TYPE_DESCRIPTOR_WRITE, device, &gattdesc.value().get()));
                if (!ret.second) {
                    HILOGE("insert request failed, type: %{public}u, addr: %{public}s",
                        REQUEST_TYPE_DESCRIPTOR_WRITE, GET_ENCRYPT_GATT_ADDR(device));
                }
            }

            if (serverSptr->pimpl->callback_) {
                serverSptr->pimpl->callback_->OnDescriptorWriteRequest(
                    BluetoothRemoteDevice(device.addr_.GetAddress(),
                        (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                    gattdesc.value().get(),
                    serverSptr->pimpl->BuildRequestId(REQUEST_TYPE_DESCRIPTOR_WRITE, device.transport_));
            }
            
            return;
        } else {
            HILOGE("Can not Find Descriptor!");
        }
        return;
    }

    void OnNotifyConfirm(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int result) override
    {
        HILOGI("remote device: %{public}s, handle: 0x%{public}04X",
            GET_ENCRYPT_GATT_ADDR(device), characteristic.handle_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        if (serverSptr->pimpl->callback_) {
            serverSptr->pimpl->callback_->OnNotificationCharacteristicChanged(
                BluetoothRemoteDevice(device.addr_.GetAddress(),
                    (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                result);
        }
        return;
    }

    void OnConnectionStateChanged(const BluetoothGattDevice &device, int32_t ret, int32_t state) override
    {
        HILOGD("gattServer conn state, remote device: %{public}s, ret: %{public}d, state: %{public}s",
            GET_ENCRYPT_GATT_ADDR(device), ret, GetProfileConnStateName(state).c_str());
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        if (state == static_cast<int>(BTConnectState::CONNECTED)) {
            std::lock_guard<std::mutex> lck(serverSptr->pimpl->deviceListMutex_);
            serverSptr->pimpl->devices_.push_back((bluetooth::GattDevice)device);
        } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            std::lock_guard<std::mutex> lck(serverSptr->pimpl->deviceListMutex_);
            for (auto it = serverSptr->pimpl->devices_.begin(); it != serverSptr->pimpl->devices_.end(); it++) {
                if (*it == (bluetooth::GattDevice)device) {
                    serverSptr->pimpl->devices_.erase(it);
                    break;
                }
            }
        }

        if (serverSptr->pimpl->callback_) {
            serverSptr->pimpl->callback_->OnConnectionStateUpdate(
                BluetoothRemoteDevice(device.addr_.GetAddress(),
                    (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                state);
        }
        
        return;
    }

    void OnMtuChanged(const BluetoothGattDevice &device, int32_t mtu) override
    {
        HILOGI("remote device: %{public}s, mtu: %{public}d", GET_ENCRYPT_GATT_ADDR(device), mtu);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        if (serverSptr->pimpl->callback_) {
            serverSptr->pimpl->callback_->OnMtuUpdate(
                BluetoothRemoteDevice(device.addr_.GetAddress(),
                    (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                mtu);
        }
        
        return;
    }

    void OnAddService(int32_t ret, const BluetoothGattService &service) override
    {
        HILOGI("enter, ret: %{public}d, handle: %{public}d", ret, service.handle_);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        GattService gattService(UUID(), GattServiceType::PRIMARY);
        if (ret == GattStatus::GATT_SUCCESS) {
            gattService = serverSptr->pimpl->BuildService(service);
            serverSptr->pimpl->BuildIncludeService(gattService, service.includeServices_);
            {
                std::lock_guard<std::mutex> lck(serverSptr->pimpl->serviceListMutex_);
                serverSptr->pimpl->gattServices_.emplace(serverSptr->pimpl->gattServices_.end(), gattService);
            }
        }
        if (serverSptr->pimpl && serverSptr->pimpl->callback_) {
            serverSptr->pimpl->callback_->OnServiceAdded(gattService, ret);
        }
        return;
    }

    void OnConnectionParameterChanged(
        const BluetoothGattDevice &device, int32_t interval, int32_t latency, int32_t timeout, int32_t status) override
    {
        HILOGD("remote device: %{public}s, interval: %{public}d, "
            "latency: %{public}d, timeout: %{public}d, status: %{public}d",
            GET_ENCRYPT_GATT_ADDR(device), interval, latency, timeout, status);
        auto serverSptr = GetServerSptr();
        if (!serverSptr) {
            return;
        }

        if (serverSptr->pimpl->callback_) {
            serverSptr->pimpl->callback_->OnConnectionParameterChanged(
                BluetoothRemoteDevice(device.addr_.GetAddress(),
                    (device.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR),
                interval,
                latency,
                timeout,
                status);
        }

        return;
    }

    explicit BluetoothGattServerCallbackStubImpl(std::weak_ptr<GattServer> server) : server_(server)
    {
        HILOGD("enter");
    }
    ~BluetoothGattServerCallbackStubImpl() override
    {
        HILOGD("enter");
    }

private:
    std::weak_ptr<GattServer> server_;
};

GattService GattServer::impl::BuildService(const BluetoothGattService &service)
{
    GattService gattService(UUID::ConvertFrom128Bits(service.uuid_.ConvertTo128Bits()),
        service.handle_,
        service.endHandle_,
        service.isPrimary_ ? GattServiceType::PRIMARY : GattServiceType::SECONDARY);

    for (auto &character : service.characteristics_) {
        GattCharacteristic gattcharacter(UUID::ConvertFrom128Bits(character.uuid_.ConvertTo128Bits()),
            character.handle_,
            character.permissions_,
            character.properties_);

        gattcharacter.SetValue(character.value_.get(), character.length_);

        for (auto &desc : character.descriptors_) {
            GattDescriptor gattDesc(
                UUID::ConvertFrom128Bits(desc.uuid_.ConvertTo128Bits()), desc.handle_, desc.permissions_);

            gattDesc.SetValue(desc.value_.get(), desc.length_);
            gattcharacter.AddDescriptor(std::move(gattDesc));
        }

        gattService.AddCharacteristic(std::move(gattcharacter));
    }

    return gattService;
}

void GattServer::impl::BuildIncludeService(GattService &svc, const std::vector<bluetooth::Service> &iSvcs)
{
    for (auto &iSvc : iSvcs) {
        GattService *pSvc = GetIncludeService(iSvc.startHandle_);
        if (!pSvc) {
            HILOGE("Can not find include service entity in service ");
            continue;
        }
        svc.AddService(std::ref(*pSvc));
    }
}

GattService *GattServer::impl::GetIncludeService(uint16_t handle)
{
    for (auto &svc : gattServices_) {
        if (svc.GetHandle() == handle) {
            return &svc;
        }
    }

    return nullptr;
}

GattServer::impl::impl(std::shared_ptr<GattServerCallback> callback)
    : isRegisterSucceeded_(false), callback_(callback), applicationId_(0)
{
    HILOGD("enter");
};

GattServer::GattServer(std::shared_ptr<GattServerCallback> callback)
{
    HILOGI("create GattServer start.");
    pimpl = std::make_unique<GattServer::impl>(callback);
    if (!pimpl) {
        HILOGE("create GattServer failed.");
    }
}

bool GattServer::impl::Init(std::weak_ptr<GattServer> server)
{
    if (profileRegisterId != 0) { //ProxyManager has register callback
        return true;
    }
    serviceCallback_ = new BluetoothGattServerCallbackStubImpl(server);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_GATT_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothGattServer> proxy = iface_cast<IBluetoothGattServer>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        int result = proxy->RegisterApplication(serviceCallback_);
        if (result > 0) {
            applicationId_ = result;
            isRegisterSucceeded_ = true;
        } else {
            HILOGE("Can not Register to gatt server service! result = %{public}d", result);
        }
    });
    return true;
}

std::shared_ptr<GattServer> GattServer::CreateInstance(std::shared_ptr<GattServerCallback> callback)
{
    // The passkey pattern used here.
    auto instance = std::make_shared<GattServer>(PassKey(), callback);
    if (!instance->pimpl) {
        HILOGE("failed: no impl");
        return nullptr;
    }

    instance->pimpl->Init(instance);
    return instance;
}

std::optional<std::reference_wrapper<GattCharacteristic>> GattServer::impl::FindCharacteristic(uint16_t handle)
{
    for (auto &svc : gattServices_) {
        for (auto &character : svc.GetCharacteristics()) {
            if (character.GetHandle() == handle) {
                return character;
            }
        }
    }
    return std::nullopt;
}
bluetooth::GattDevice *GattServer::impl::FindConnectedDevice(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> lock(deviceListMutex_);
    for (auto &gattDevice : devices_) {
        if (device.GetDeviceAddr().compare(gattDevice.addr_.GetAddress()) == 0 &&
            (device.GetTransportType() ==
            (gattDevice.transport_ == GATT_TRANSPORT_TYPE_LE) ? BT_TRANSPORT_BLE : BT_TRANSPORT_BREDR)) {
            return &gattDevice;
        }
    }
    return nullptr;
}
std::optional<std::reference_wrapper<GattDescriptor>> GattServer::impl::FindDescriptor(uint16_t handle)
{
    for (auto &svc : gattServices_) {
        for (auto &character : svc.GetCharacteristics()) {
            for (auto &desc : character.GetDescriptors()) {
                if (desc.GetHandle() == handle) {
                    return desc;
                }
            }
        }
    }
    return std::nullopt;
}

int GattServer::impl::BuildRequestId(uint8_t type, uint8_t transport)
{
    int requestId = 0;

    requestId = type << EIGHT_BITS;
    requestId |= transport;

    return requestId;
}

int GattServer::impl::RespondCharacteristicRead(
    const bluetooth::GattDevice &device, uint16_t handle, const uint8_t *value, size_t length, int ret)
{
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, GattStatus::REQUEST_NOT_SUPPORT, "failed: no proxy");

    if (ret == GattStatus::GATT_SUCCESS) {
        BluetoothGattCharacteristic character(bluetooth::Characteristic(handle, value, length));

        return proxy->RespondCharacteristicRead(device, &character, ret);
    }
    BluetoothGattCharacteristic character;
    character.handle_ = handle;
    return proxy->RespondCharacteristicRead(device, &character, ret);
}

int GattServer::impl::RespondCharacteristicWrite(const bluetooth::GattDevice &device, uint16_t handle, int ret)
{
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, GattStatus::REQUEST_NOT_SUPPORT, "failed: no proxy");
    return proxy->RespondCharacteristicWrite(
        device, (BluetoothGattCharacteristic)bluetooth::Characteristic(handle), ret);
}

int GattServer::impl::RespondDescriptorRead(
    const bluetooth::GattDevice &device, uint16_t handle, const uint8_t *value, size_t length, int ret)
{
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, GattStatus::REQUEST_NOT_SUPPORT, "failed: no proxy");
    if (ret == GattStatus::GATT_SUCCESS) {
        BluetoothGattDescriptor desc(bluetooth::Descriptor(handle, value, length));
        return proxy->RespondDescriptorRead(device, &desc, ret);
    }
    BluetoothGattDescriptor desc;
    desc.handle_ = handle;
    return proxy->RespondDescriptorRead(device, &desc, ret);
}

int GattServer::impl::RespondDescriptorWrite(const bluetooth::GattDevice &device, uint16_t handle, int ret)
{
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, GattStatus::REQUEST_NOT_SUPPORT, "failed: no proxy");
    return proxy->RespondDescriptorWrite(device, (BluetoothGattDescriptor)bluetooth::Descriptor(handle), ret);
}

int GattServer::AddService(GattService &service)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    BluetoothGattService svc;
    svc.isPrimary_ = service.IsPrimary();
    svc.uuid_ = bluetooth::Uuid::ConvertFrom128Bits(service.GetUuid().ConvertTo128Bits());

    for (auto &isvc : service.GetIncludedServices()) {
        svc.includeServices_.push_back(bluetooth::Service(isvc.get().GetHandle()));
    }

    for (auto &character : service.GetCharacteristics()) {
        size_t length = 0;
        uint8_t *value = character.GetValue(&length).get();
        bluetooth::Characteristic c(bluetooth::Uuid::ConvertFrom128Bits(character.GetUuid().ConvertTo128Bits()),
            character.GetHandle(),
            character.GetProperties(),
            character.GetPermissions(),
            value,
            length);

        for (auto &desc : character.GetDescriptors()) {
            value = desc.GetValue(&length).get();
            bluetooth::Descriptor d(bluetooth::Uuid::ConvertFrom128Bits(desc.GetUuid().ConvertTo128Bits()),
                desc.GetHandle(),
                desc.GetPermissions(),
                value,
                length);

            c.descriptors_.push_back(std::move(d));
        }

        svc.characteristics_.push_back(std::move(c));
    }
    int appId = pimpl->applicationId_;
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    int ret = proxy->AddService(appId, &svc);
    HILOGI("appId = %{public}d, ret = %{public}d.", appId, ret);
    return ret;
}

void GattServer::ClearServices()
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    int appId = pimpl->applicationId_;
    proxy->ClearServices(int(appId));
    pimpl->gattServices_.clear();
}

int GattServer::Close()
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    int ret = proxy->DeregisterApplication(pimpl->applicationId_);
    HILOGI("ret: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        pimpl->isRegisterSucceeded_ = false;
    }
    return ret;
}

int GattServer::Connect(const BluetoothRemoteDevice &device, bool isDirect)
{
    CHECK_AND_RETURN_LOG_RET(IS_BLE_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");

    int appId = pimpl->applicationId_;
    HILOGI("appId: %{public}d, device: %{public}s, isDirect: %{public}d", appId, GET_ENCRYPT_ADDR(device), isDirect);
    
    bluetooth::GattDevice gattDevice(bluetooth::RawAddress(device.GetDeviceAddr()), GATT_TRANSPORT_TYPE_LE);
    return proxy->Connect(appId, gattDevice, isDirect);
}

int GattServer::CancelConnection(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_LOG_RET(IS_BLE_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INTERNAL_ERROR, "Invalid remote device");

    auto gattDevice = pimpl->FindConnectedDevice(device);
    if (gattDevice == nullptr) {
        HILOGE("gattDevice is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    int appId = pimpl->applicationId_;
    HILOGI("appId: %{public}d, device: %{public}s", appId, GET_ENCRYPT_ADDR(device));
    return proxy->CancelConnection(appId, *gattDevice);
}
std::optional<std::reference_wrapper<GattService>> GattServer::GetService(const UUID &uuid, bool isPrimary)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::nullopt;
    }
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::nullopt, "failed: no proxy");

    std::unique_lock<std::mutex> lock(pimpl->serviceListMutex_);

    for (auto &svc : pimpl->gattServices_) {
        if (svc.GetUuid().Equals(uuid) && svc.IsPrimary() == isPrimary) {
            HILOGI("Find service, handle: 0x%{public}04X", svc.GetHandle());
            return svc;
        }
    }

    return std::nullopt;
}

std::list<GattService> &GattServer::GetServices()
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return pimpl->gattServices_;
    }
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, pimpl->gattServices_, "failed: no proxy");

    std::unique_lock<std::mutex> lock(pimpl->serviceListMutex_);
    return pimpl->gattServices_;
}
int GattServer::NotifyCharacteristicChanged(
    const BluetoothRemoteDevice &device, const GattCharacteristic &characteristic, bool confirm)
{
    HILOGI("remote device: %{public}s, handle: 0x%{public}04X, confirm: %{public}d",
        GET_ENCRYPT_ADDR(device), characteristic.GetHandle(), confirm);
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("Invalid remote device");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->FindConnectedDevice(device) == nullptr) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }

    size_t length = 0;
    auto &characterValue = characteristic.GetValue(&length);

    BluetoothGattCharacteristic character(
        bluetooth::Characteristic(characteristic.GetHandle(), characterValue.get(), length));
    std::string address = device.GetDeviceAddr();
    int ret = proxy->NotifyClient(
        bluetooth::GattDevice(bluetooth::RawAddress(address), 0), &character, confirm);
    HILOGI("ret = %{public}d.", ret);
    return ret;
}

int GattServer::RemoveGattService(const GattService &service)
{
    HILOGD("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGD("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    int ret = BT_ERR_INVALID_PARAM;
    for (auto sIt = pimpl->gattServices_.begin(); sIt != pimpl->gattServices_.end(); sIt++) {
        if (sIt->GetUuid().Equals(service.GetUuid())) {
            ret = proxy->RemoveService(
                pimpl->applicationId_, (BluetoothGattService)bluetooth::Service(sIt->GetHandle()));
            pimpl->gattServices_.erase(sIt);
            break;
        }
    }
    HILOGI("result = %{public}d.", ret);
    return ret;
}
int GattServer::SendResponse(
    const BluetoothRemoteDevice &device, int requestId, int status, int offset, const uint8_t *value, int length)
{
    HILOGI("remote device: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(device), status);
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("pimpl or gatt server proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl->FindConnectedDevice(device) == nullptr) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(device));
        return BT_ERR_INTERNAL_ERROR;
    }

    int result = BT_ERR_INTERNAL_ERROR;
    uint8_t requestType = requestId >> EIGHT_BITS;
    uint8_t transport = requestId & 0xFF;
    if (transport != GATT_TRANSPORT_TYPE_CLASSIC && transport != GATT_TRANSPORT_TYPE_LE) {
        return result;
    }
    bluetooth::GattDevice gattdevice(bluetooth::RawAddress(device.GetDeviceAddr()), transport);

    std::lock_guard<std::mutex> lock(pimpl->requestListMutex_);
    auto request = pimpl->requests_.find(RequestInformation(requestType, gattdevice));
    if (request != pimpl->requests_.end()) {
        switch (requestType) {
            case REQUEST_TYPE_CHARACTERISTICS_READ:
                result = pimpl->RespondCharacteristicRead(
                    gattdevice, request->context_.characteristic_->GetHandle(), value, length, status);
                break;
            case REQUEST_TYPE_CHARACTERISTICS_WRITE:
                result = pimpl->RespondCharacteristicWrite(
                    gattdevice, request->context_.characteristic_->GetHandle(), status);
                break;
            case REQUEST_TYPE_DESCRIPTOR_READ:
                result = pimpl->RespondDescriptorRead(
                    gattdevice, request->context_.descriptor_->GetHandle(), value, length, status);
                break;
            case REQUEST_TYPE_DESCRIPTOR_WRITE:
                result = pimpl->RespondDescriptorWrite(gattdevice, request->context_.descriptor_->GetHandle(), status);
                break;
            default:
                HILOGE("Error request Id!");
                break;
        }
        if (result == BT_NO_ERROR) {
            pimpl->requests_.erase(request);
        }
    }
    HILOGD("result = %{public}d.", result);
    return result;
}

GattServer::~GattServer()
{
    HILOGD("enter");
    BluetoothProfileManager::GetInstance().DeregisterFunc(pimpl->profileRegisterId);
    sptr<IBluetoothGattServer> proxy = GetRemoteProxy<IBluetoothGattServer>(PROFILE_GATT_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    if (pimpl->isRegisterSucceeded_) {
        proxy->DeregisterApplication(pimpl->applicationId_);
    }
    HILOGI("end");
}
}  // namespace Bluetooth
}  // namespace OHOS
