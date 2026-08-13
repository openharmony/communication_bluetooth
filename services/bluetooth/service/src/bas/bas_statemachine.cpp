/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_bas"
#endif

#include "bas_statemachine.h"

#include "common_util.h"
#include "hitrace_meter.h"
#include "profile_service_manager.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {

namespace {
const std::string GATT_BATTERY_SERVICE_UUID = "0000180F-0000-1000-8000-00805F9B34FB";
const std::string GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID = "00002A19-0000-1000-8000-00805F9B34FB";
const std::string CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805F9B34FB";
}

class BasStateMachine::GattClientCallbackImpl : public IGattClientCallback {
public:
    explicit GattClientCallbackImpl(BasStateMachine &owner) : owner_(owner) {}
    ~GattClientCallbackImpl() override = default;

    void OnConnectionStateChanged(int state, int newState, const RawAddress &addr, int disconnectReason,
        const std::string &reasonMessage = "") override
    {
        HILOGI("[BAS_MACHINE] state: %{public}d, newState: %{public}d, addr: %{public}s, reason: %{public}d",
            state, newState, GetEncryptAddr(addr.GetAddress()).c_str(), disconnectReason);
        BasMessage event(BAS_CONNECTION_STATE_CHANGED_EVT);
        uint8_t basState = BAS_STATE_DISCONNECTED;
        if (newState == static_cast<int>(BTConnectState::CONNECTED)) {
            basState = BAS_STATE_CONNECTED;
        } else if (newState == static_cast<int>(BTConnectState::CONNECTING)) {
            basState = BAS_STATE_CONNECTING;
        } else if (newState == static_cast<int>(BTConnectState::DISCONNECTING)) {
            basState = BAS_STATE_DISCONNECTING;
        }
        event.arg1_ = basState;
        event.dev_ = addr.GetAddress();
        BasServiceImplInterface* service = owner_.GetBasService();
        if (service != nullptr) {
            service->PostEvent(event);
        }
    }

    void OnCharacteristicChanged(const Characteristic &characteristic) override
    {
        HILOGI("[BAS_MACHINE] uuid: %{public}s, handle: %{public}d",
            characteristic.uuid_.ToString().c_str(), characteristic.handle_);
        if (characteristic.handle_ == owner_.basCharacHandle_.load() &&
            characteristic.value_ != nullptr && characteristic.length_ > 0) {
            int batteryLevel = static_cast<int>(characteristic.value_[0]);
            owner_.UpdateBatteryLevel(batteryLevel);
        }
    }

    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {
        HILOGI("[BAS_MACHINE] uuid: %{public}s, handle: %{public}d",
            characteristic.uuid_.ToString().c_str(), characteristic.handle_);
        if (ret != GATT_SUCCESS && characteristic.handle_ == owner_.basCharacHandle_.load()) {
            HILOGE("[BAS_MACHINE], read battery failed, ret: %{public}d", ret);
            owner_.OnBatteryLevelRead(BAS_INVALID_BATTERY_LEVEL);  // 读取电量失败，上报-1
            return;
        }
        if (characteristic.handle_ == owner_.basCharacHandle_.load() &&
            characteristic.value_ != nullptr && characteristic.length_ > 0) {
            int batteryLevel = static_cast<int>(characteristic.value_[0]);
            owner_.UpdateBatteryLevel(batteryLevel);
            owner_.OnBatteryLevelRead(batteryLevel);
        }
    }

    void OnServicesDiscovered(int status) override
    {
        HILOGI("[BAS_MACHINE] OnServicesDiscovered status: %{public}d", status);
        owner_.OnServicesDiscovered();
    }

    void OnServicesChanged() override {}
    void OnCharacteristicWrite(int ret, const Characteristic &characteristic,
        const Bluetooth::BluetoothGattRspContext &rspContext) override {}
    void OnDescriptorRead(int ret, const Descriptor &descriptor) override {}
    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override {}
    void OnMtuChanged(int state, int mtu) override {}
    void OnReadRemoteRssiValue(const RawAddress &addr, int rssi, int status) override {}
    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override {}
    void OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status) override {}
    void OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status) override {}

private:
    BasStateMachine &owner_;
};


BasStateMachine::BasStateMachine(const std::string &address, BasServiceImplInterface *basServiceImpl)
    : address_(address), basServiceImpl_(basServiceImpl)
{
}

BasStateMachine::~BasStateMachine()
{
    CloseGatt();
}

void BasStateMachine::Init()
{
    InitGatt();
    connTimer_ = std::make_unique<utility::Timer>(std::bind([this] {this->ConnectionTimeout();}));
    disconnTimer_ = std::make_unique<utility::Timer>(std::bind([this] {this->DisconnectionTimeout();}));

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<BasDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<BasConnectingState>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<BasDisconnectingState>(DISCONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<BasConnectedState>(CONNECTED, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(connectedState);

    InitState(DISCONNECTED);
}

bool BasStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void BasStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

std::string BasStateMachine::GetDeviceAddress()
{
    return address_;
}

IProfileGattClient *BasStateMachine::GetGattClientService()
{
    if (IProfileManager::GetInstance() == nullptr) {
        return nullptr;
    }
    return static_cast<IProfileGattClient *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));
}

void BasStateMachine::InitGatt()
{
    if (appId_.load() != -1) {
        HILOGI("already registered appId: %{public}d", appId_.load());
        return;
    }
    callback_ = std::make_shared<GattClientCallbackImpl>(*this);
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService != nullptr) {
        RawAddress device(address_);
        appId_.store(gattClientService->RegisterApplication(callback_, device, TRANSPORT_LE));
    } else {
        HILOGE("gattClientService is nullptr");
    }
    HILOGI("appId: %{public}d", appId_.load());
}

bool BasStateMachine::ConnectGatt()
{
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return false;
    }
    int ret = gattClientService->Connect(appId_, false);
    if (ret != GATT_SUCCESS) {
        HILOGE("Connect appId %{public}d failed, ret: %{public}d", appId_.load(), ret);
        return false;
    }
    return true;
}

bool BasStateMachine::DisconnectGatt()
{
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return false;
    }
    int ret = gattClientService->Disconnect(appId_);
    if (ret != GATT_SUCCESS) {
        HILOGE("Disconnect appId %{public}d failed, ret: %{public}d", appId_.load(), ret);
        return false;
    }
    return true;
}

void BasStateMachine::CloseGatt()
{
    if (appId_.load() == -1) {
        HILOGI("already deregistered");
        return;
    }
    HILOGI("appId: %{public}d", appId_.load());

    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return;
    }
    int ret = gattClientService->DeregisterApplication(appId_.load());
    if (ret != GATT_SUCCESS) {
        HILOGE("Disconnect appId %{public}d failed, ret: %{public}d", appId_.load(), ret);
    }
    appId_.store(-1);
}

bool BasStateMachine::DiscoverServicesGatt()
{
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return false;
    }
    int ret = gattClientService->DiscoveryServices(appId_.load());
    if (ret != GATT_SUCCESS) {
        HILOGE("DiscoveryServices appId %{public}d failed, ret: %{public}d", appId_.load(), ret);
        return false;
    }
    return true;
}

bool BasStateMachine::GetBatteryLevel()
{
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return false;
    }

    std::vector<Service> services = gattClientService->GetServices(appId_.load());
    const Characteristic* batteryCharac = FindBatteryCharacteristic(services);
    if (batteryCharac == nullptr) {
        HILOGW("battery characteristic not found");
        return false;
    }
    basCharacHandle_.store(batteryCharac->handle_);
    int ret = gattClientService->ReadCharacteristic(appId_.load(), *batteryCharac);
    if (ret != GATT_SUCCESS) {
        HILOGE("ReadCharacteristic failed, ret: %{public}d", ret);
        return false;
    }
    HILOGW("Battery service or characteristic not found");
    return false;
}

const Characteristic* BasStateMachine::FindBatteryCharacteristic(const std::vector<Service>& services)
{
    Uuid batteryServiceUuid = Uuid::ConvertFromString(GATT_BATTERY_SERVICE_UUID);
    Uuid batteryLevelCharUuid = Uuid::ConvertFromString(GATT_BATTERY_LEVEL_CHARACTERISTIC_UUID);

    for (const auto& service : services) {
        if (service.uuid_ != batteryServiceUuid) {
            continue;
        }
        for (const auto& charac : service.characteristics_) {
            if (charac.uuid_ == batteryLevelCharUuid) {
                return &charac;
            }
        }
    }
    return nullptr;
}

const Descriptor* BasStateMachine::FindCccdDescriptor(const Characteristic& charac)
{
    Uuid cccdUuid = Uuid::ConvertFromString(CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID);
    for (const auto& descriptor : charac.descriptors_) {
        if (descriptor.uuid_ == cccdUuid) {
            return &descriptor;
        }
    }
    return nullptr;
}

bool BasStateMachine::EnableBasNotification()
{
    IProfileGattClient* gattClientService = GetGattClientService();
    if (gattClientService == nullptr) {
        HILOGE("gattClientService is nullptr");
        return false;
    }

    std::vector<Service> services = gattClientService->GetServices(appId_.load());
    const Characteristic* batteryCharac = FindBatteryCharacteristic(services);
    if (batteryCharac == nullptr) {
        HILOGW("battery characteristic not found");
        return false;
    }
    basCharacHandle_.store(batteryCharac->handle_);

    const Descriptor* descriptor = FindCccdDescriptor(*batteryCharac);
    if (descriptor == nullptr) {
        HILOGW("CCCD descriptor not found");
        return false;
    }
    const size_t descriptorValueSize = 2; // 2 bytes
    uint8_t value[descriptorValueSize] = {0x01, 0x00}; // enable notify data
    Descriptor cccdDescriptor(descriptor->handle_, value, descriptorValueSize);
    int ret = gattClientService->WriteDescriptor(appId_.load(), cccdDescriptor);
    if (ret != GATT_SUCCESS) {
        HILOGE("WriteDescriptor to enable notification failed, ret: %{public}d", ret);
        return false;
    }
    gattClientService->RequestNotification(appId_.load(), batteryCharac->handle_, true);
    HILOGI("EnableBasNotification success, handle: %{public}#x", cccdDescriptor.handle_);
    return true;
}

void BasStateMachine::OnServicesDiscovered()
{
    BasMessage event(BAS_SERVICE_DISCOVERED_EVT);
    event.dev_ = address_;
    BasServiceImplInterface* service = basServiceImpl_;
    if (service != nullptr) {
        service->PostEvent(event);
    }
}

void BasStateMachine::UpdateBatteryLevel(int batteryLevel)
{
    if (batteryLevel < 0 || batteryLevel > BAS_MAX_BATTERY_LEVEL) {
        HILOGE("Address=[%{public}s] invalid batteryLevel: %{public}d",
            GetEncryptAddr(address_).c_str(), batteryLevel);
        return;
    }
    HILOGI("Address=[%{public}s] UpdateBatteryLevel: old=%{public}d, new=%{public}d",
        GetEncryptAddr(address_).c_str(), batteryLevel_, batteryLevel);
    if (batteryLevel_ != batteryLevel) {
        batteryLevel_ = batteryLevel;
        BasMessage event(BAS_BATTERY_LEVEL_CHANGED_EVT);
        event.dev_ = address_;
        event.batteryData_.batteryLevel = batteryLevel;
        BasServiceImplInterface* service = basServiceImpl_;
        if (service != nullptr) {
            service->PostEvent(event);
        }
    }
}

void BasStateMachine::OnBatteryLevelRead(int batteryLevel)
{
    HILOGI("OnBatteryLevelRead: old=%{public}d, new=%{public}d", batteryLevel_, batteryLevel);
    if (batteryLevel >= 0 && batteryLevel <= BAS_MAX_BATTERY_LEVEL) {
        batteryLevel_ = batteryLevel;
    }
    BasMessage event(BAS_BATTERY_LEVEL_READ_EVT);
    event.dev_ = address_;
    event.batteryData_.batteryLevel = batteryLevel;
    BasServiceImplInterface* service = basServiceImpl_;
    if (service != nullptr) {
        service->PostEvent(event);
    }
}

void BasDisconnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        stateMachine_.NotifyStateTransitions();
        BasServiceImplInterface* service = stateMachine_.GetBasService();
        if (service != nullptr) {
            service->RemoveStateMachine(stateMachine_.GetDeviceAddress());
        }
    }
}

void BasDisconnectedState::Exit()
{
    isReentry_ = true;
}

bool BasDisconnectedState::Dispatch(const utility::Message &msg)
{
    BasMessage &event = static_cast<BasMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[BAS_MACHINE][Disconnected]EventName=%{public}s", BasStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BAS_CONNECT_EVT: {
            if (!stateMachine_.ConnectGatt()) {
                break;
            }
            Transition(BasStateMachine::CONNECTING);
            break;
        }
        case BAS_CONNECTION_STATE_CHANGED_EVT: {
            int state = event.arg1_;
            if (state == BAS_STATE_CONNECTING) {
                Transition(BasStateMachine::CONNECTING);
            }
            if (state == BAS_STATE_CONNECTED) {
                Transition(BasStateMachine::CONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void BasConnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void BasConnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool BasConnectingState::Dispatch(const utility::Message &msg)
{
    BasMessage &event = static_cast<BasMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[BAS_MACHINE][Connecting]EventName=%{public}s", BasStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BAS_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case BAS_DISCONNECT_EVT:
            stateMachine_.DisconnectGatt();
            Transition(BasStateMachine::DISCONNECTED);
            break;
        case BAS_CONNECTION_STATE_CHANGED_EVT: {
            int state = event.arg1_;
            if (state == BAS_STATE_CONNECTED) {
                Transition(BasStateMachine::CONNECTED);
            }
            if (state == BAS_STATE_DISCONNECTED) {
                Transition(BasStateMachine::DISCONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void BasDisconnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartDisconnectionTimer();
}

void BasDisconnectingState::Exit()
{
    stateMachine_.StopDisconnectionTimer();
}

bool BasDisconnectingState::Dispatch(const utility::Message &msg)
{
    BasMessage &event = static_cast<BasMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[BAS_MACHINE][Disconnecting]EventName=%{public}s", BasStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BAS_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case BAS_DISCONNECT_EVT:
            Transition(BasStateMachine::DISCONNECTED);
            break;
        case BAS_CONNECTION_STATE_CHANGED_EVT: {
            int state = event.arg1_;
            if (state == BAS_STATE_DISCONNECTED) {
                Transition(BasStateMachine::DISCONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void BasConnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    stateMachine_.DiscoverServicesGatt();
}

void BasConnectedState::Exit()
{
}

bool BasConnectedState::Dispatch(const utility::Message &msg)
{
    BasMessage &event = static_cast<BasMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[BAS_MACHINE][Connected]EventName=%{public}s", BasStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BAS_CONNECT_EVT:
            HILOGE("[BAS_MACHINE][Connected] device has been connected!");
            break;
        case BAS_DISCONNECT_EVT: {
            if (!stateMachine_.DisconnectGatt()) {
                Transition(BasStateMachine::DISCONNECTED);
                break;
            }
            Transition(BasStateMachine::DISCONNECTING);
            break;
        }
        case BAS_CONNECTION_STATE_CHANGED_EVT: {
            int state = event.arg1_;
            if (state == BAS_STATE_CONNECTED) {
                Transition(BasStateMachine::CONNECTED);
                break;
            }
            if (state == BAS_STATE_DISCONNECTED) {
                Transition(BasStateMachine::DISCONNECTED);
            }
            break;
        }
        case BAS_READ_BATTERY_EVT:
            stateMachine_.GetBatteryLevel();
            break;
        case BAS_SERVICE_DISCOVERED_EVT:
            stateMachine_.EnableBasNotification();
            break;
        default:
            break;
    }
    return true;
}

int BasStateMachine::GetDeviceStateInt() const
{
    const BasState* state = static_cast<const BasState*>(GetState());
    if (state == nullptr) {
        return BAS_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

BasServiceImplInterface* BasStateMachine::GetBasService(void)
{
    return basServiceImpl_;
}

void BasStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("[BAS_MACHINE]Start connection timer!");
}

void BasStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("[BAS_MACHINE]Stop connection timer!");
}

void BasStateMachine::ConnectionTimeout() const
{
    HITRACE_METER(BT_TRACE_TAG);
    BasMessage event(BAS_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    if (basServiceImpl_ != nullptr) {
        basServiceImpl_->PostEvent(event);
    }
}

void BasStateMachine::StartDisconnectionTimer() const
{
    disconnTimer_->Start(DISCONNECTION_TIMEOUT_MS);
    HILOGI("[BAS_MACHINE]Start disconnection timer!");
}

void BasStateMachine::StopDisconnectionTimer() const
{
    disconnTimer_->Stop();
    HILOGI("[BAS_MACHINE]Stop disconnection timer!");
}

void BasStateMachine::DisconnectionTimeout() const
{
    HITRACE_METER(BT_TRACE_TAG);
    BasMessage event(BAS_DISCONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    if (basServiceImpl_ != nullptr) {
        basServiceImpl_->PostEvent(event);
    }
}

std::string BasStateMachine::GetEventName(int what)
{
    switch (what) {
        case BAS_CONNECTION_TIMEOUT_EVT:
            return "BAS_CONNECTION_TIMEOUT_EVT";
        case BAS_DISCONNECTION_TIMEOUT_EVT:
            return "BAS_DISCONNECTION_TIMEOUT_EVT";
        case BAS_CONNECT_EVT:
            return "BAS_CONNECT_EVT";
        case BAS_DISCONNECT_EVT:
            return "BAS_DISCONNECT_EVT";
        case BAS_CONNECTION_STATE_CHANGED_EVT:
            return "BAS_CONNECTION_STATE_CHANGED_EVT";
        case BAS_READ_BATTERY_EVT:
            return "BAS_READ_BATTERY_EVT";
        case BAS_SERVICE_DISCOVERED_EVT:
            return "BAS_SERVICE_DISCOVERED_EVT";
        case BAS_BATTERY_LEVEL_READ_EVT:
            return "BAS_BATTERY_LEVEL_READ_EVT";
        case BAS_BATTERY_LEVEL_CHANGED_EVT:
            return "BAS_BATTERY_LEVEL_CHANGED_EVT";
        case BAS_REMOVE_STATE_MACHINE_EVT:
            return "BAS_REMOVE_STATE_MACHINE_EVT";
        default:
            return "Unknown";
    }
}

void BasStateMachine::NotifyStateTransitions()
{
    preState_ = GetDeviceStateInt();
}

void BasStateMachine::AddDeferredMessage(const BasMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void BasStateMachine::ProcessDeferredMessage()
{
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        BasMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        if (basServiceImpl_ != nullptr) {
            basServiceImpl_->PostEvent(event);
        }
        size--;
    }
}

int BasStateMachine::GetCachedBatteryLevel() const
{
    return batteryLevel_;
}

}  // namespace bluetooth
}  // namespace OHOS