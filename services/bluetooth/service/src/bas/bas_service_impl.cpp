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
#define LOG_TAG "bt_service_bas_impl"
#endif

#include "bas_service_impl.h"

#include "bluetooth_errorcode.h"
#include "common_util.h"
#include "context.h"
#include "hitrace_meter.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {

extern "C" BasServiceImplInterface *CreateBasServiceImplInterface(void)
{
    return new (std::nothrow) BasServiceImpl();
}

extern "C" void DestroyBasServiceImplInterface(BasServiceImplInterface *interface)
{
    if (interface == nullptr) {
        return;
    }
    delete interface;
    interface = nullptr;
}

BasServiceImpl::BasServiceImpl()
{}

BasServiceImpl::~BasServiceImpl() = default;

void BasServiceImpl::Init()
{
    BasMessage event(BAS_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void BasServiceImpl::DeInit()
{
    BasMessage event(BAS_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

void BasServiceImpl::RegisterObserver(std::shared_ptr<IBasObserver> observer)
{
    basObservers_.Register(observer);
}

void BasServiceImpl::DeregisterObserver(std::shared_ptr<IBasObserver> observer)
{
    basObservers_.Deregister(observer);
}

int BasServiceImpl::Connect(const RawAddress &device)
{
    int ret = IsLocalDeviceConnectAllowed(device);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGE("LocalDevice Connect not Allowed");
        return ret;
    }
    std::string address = device.GetAddress();
    BasMessage event(BAS_CONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    return BAS_SUCCESS;
}

int BasServiceImpl::Disconnect(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (!ret || stateMachine == nullptr) {
        HILOGW("[BAS_SERVICE_IMPL]The state machine is not available!");
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }

    int slcState = stateMachine->GetDeviceStateInt();
    if ((slcState != BAS_STATE_CONNECTING) && (slcState < BAS_STATE_CONNECTED)) {
        HILOGW("[BAS_SERVICE_IMPL]This device not connected, slcState:%{public}d", slcState);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }

    BasMessage event(BAS_DISCONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    return BAS_SUCCESS;
}

int BasServiceImpl::GetBatteryLevel(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (!ret || stateMachine == nullptr) {
        HILOGE("[BAS_SERVICE_IMPL]Invalid address[%{public}s]", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
    }

    int state = stateMachine->GetDeviceStateInt();
    if (state < BAS_STATE_CONNECTED) {
        HILOGE("[BAS_SERVICE_IMPL]Device not connected, state: %{public}d", state);
        return Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
    }

    BasMessage event(BAS_READ_BATTERY_EVT);
    event.dev_ = address;
    PostEvent(event);
    return BAS_SUCCESS;
}

int BasServiceImpl::GetDeviceState(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (!ret || stateMachine == nullptr) {
        HILOGD("[BAS_SERVICE_IMPL]GetDeviceState:The state machine is not available!");
        return stateMap_.at(BAS_STATE_DISCONNECTED);
    }

    int state = stateMachine->GetDeviceStateInt();
    if (state >= BAS_STATE_CONNECTED) {
        return stateMap_.at(BAS_STATE_CONNECTED);
    } else {
        return stateMap_.at(state);
    }
}

std::list<RawAddress> BasServiceImpl::GetConnectDevices(void)
{
    std::list<RawAddress> devList;
    stateMachines_.Iterate([&devList](const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        if (stateMachine != nullptr && (stateMachine->GetDeviceStateInt() >= BAS_STATE_CONNECTED)) {
            devList.push_back(RawAddress(device));
        }
    });
    return devList;
}

int BasServiceImpl::GetConnectState(void)
{
    uint8_t result = 0;
    stateMachines_.Iterate([&result](const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        if (stateMachine == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (stateMachine->GetDeviceStateInt() >= BAS_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (stateMachine->GetDeviceStateInt() == BAS_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == BAS_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == BAS_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    });
    return static_cast<int>(result);
}

std::vector<RawAddress> BasServiceImpl::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<RawAddress> devices;
    stateMachines_.Iterate(
        [this, &devices, &states](const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        RawAddress addr(device);
        int deviceState = stateMap_.at(BAS_STATE_DISCONNECTED);
        if (stateMachine != nullptr) {
            int state = stateMachine->GetDeviceStateInt();
            if (state >= BAS_STATE_CONNECTED) {
                deviceState = stateMap_.at(BAS_STATE_CONNECTED);
            } else {
                deviceState = stateMap_.at(state);
            }
        }
        for (size_t i = 0; i < states.size(); i++) {
            if (deviceState == states[i]) {
                devices.push_back(addr);
                break;
            }
        }
    });
    return devices;
}

std::map<RawAddress, int32_t> BasServiceImpl::GetConnectedDeviceBatteryInfos()
{
    std::map<RawAddress, int32_t> batteryCache;
    stateMachines_.Iterate([&batteryCache](const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        if (stateMachine != nullptr && stateMachine->GetDeviceStateInt() == BAS_STATE_CONNECTED) {
            int batteryLevel = stateMachine->GetCachedBatteryLevel();
            batteryCache[RawAddress(device)] = batteryLevel;
        }
    });
    return batteryCache;
}

void BasServiceImpl::StartUp(void)
{
    isShuttingDown_ = false;
    stateMachines_.Clear();
}

void BasServiceImpl::ShutDown(void)
{
    if (isShuttingDown_) {
        HILOGW("[BAS_SERVICE_IMPL]BasServiceImpl is already shutting down.");
        return;
    }
    isShuttingDown_ = true;
    bool hasConnectedDevices = false;
    std::vector<std::string> devicesToDisconnect {};
    stateMachines_.Iterate([&hasConnectedDevices, &devicesToDisconnect](
        const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        if ((stateMachine != nullptr) && (stateMachine->GetDeviceStateInt() > BAS_STATE_DISCONNECTED)) {
            devicesToDisconnect.push_back(device);
            hasConnectedDevices = true;
        }
    });
    for (const std::string &address : devicesToDisconnect) {
        Disconnect(RawAddress(address));
    }
    if (!hasConnectedDevices) {
        ShutDownDone(true);
    }
}

void BasServiceImpl::ShutDownDone(bool isAllDisconnected)
{
    if (!isAllDisconnected) {
        isAllDisconnected = true;
        stateMachines_.Iterate([&isAllDisconnected](
            const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
            if ((stateMachine != nullptr) && (stateMachine->GetDeviceStateInt() > BAS_STATE_DISCONNECTED)) {
                isAllDisconnected = false;
            }
        });
    }
    if (isAllDisconnected) {
        stateMachines_.Clear();
        isShuttingDown_ = false;
        HILOGI("[BAS_SERVICE_IMPL] BasServiceImpl shutdown complete.");
    } else {
        HILOGI("[BAS_SERVICE_IMPL] not all device disconnected.");
    }
}

void BasServiceImpl::PostEvent(const BasMessage &event)
{
    DoInBleThread([this, event] {this->ProcessEvent(event);});
}

void BasServiceImpl::ProcessEvent(const BasMessage &event)
{
    std::string address = event.dev_;
    HILOGI("[BAS_SERVICE_IMPL] Address=[%{public}s], eventName=[%{public}s]",
        GetEncryptAddr(address).c_str(), GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BAS_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case BAS_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case BAS_CONNECT_EVT:
        case BAS_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectEvent(event);
            break;
        case BAS_BATTERY_LEVEL_READ_EVT:
            ProcessBatteryLevelRead(event);
            break;
        case BAS_BATTERY_LEVEL_CHANGED_EVT:
            ProcessBatteryLevelChanged(event);
            break;
        case BAS_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event.dev_);
            break;
        case BAS_READ_BATTERY_EVT:
            ProcessDefaultEvent(event);
            break;
        default:
            ProcessDefaultEvent(event);
            break;
    }
}

void BasServiceImpl::ProcessConnectEvent(const BasMessage &event)
{
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr && stateMachine->IsRemoving()) {
        PostEvent(event);
    } else if (!ret || stateMachine == nullptr) {
        auto newStateMachine = std::make_shared<BasStateMachine>(event.dev_, this);
        stateMachines_.EnsureInsert(event.dev_, newStateMachine);
        auto value = stateMachines_.ReadVal(event.dev_);
        if (value != nullptr) {
            value->Init();
            value->ProcessMessage(event);
        }
    } else {
        stateMachine->ProcessMessage(event);
    }
}

void BasServiceImpl::ProcessDefaultEvent(const BasMessage &event)
{
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr) {
        stateMachine->ProcessMessage(event);
    } else {
        HILOGE("[BAS_SERVICE_IMPL]Invalid address[%{public}s]", GetEncryptAddr(event.dev_).c_str());
    }
}

void BasServiceImpl::ProcessRemoveStateMachine(const std::string &address)
{
    stateMachines_.Erase(address);
    if (isShuttingDown_) {
        ShutDownDone(false);
    }
}

void BasServiceImpl::ProcessBatteryLevelChanged(const BasMessage &event)
{
    RawAddress device(event.dev_);
    NotifyBatteryLevelChanged(device, event.batteryData_.batteryLevel);
}

void BasServiceImpl::ProcessBatteryLevelRead(const BasMessage &event)
{
    RawAddress device(event.dev_);
    NotifyReadBatteryLevelEvent(device, event.batteryData_.batteryLevel);
}

void BasServiceImpl::NotifyReadBatteryLevelEvent(const RawAddress &device, int batteryLevel)
{
    basObservers_.ForEach([device, batteryLevel](std::shared_ptr<IBasObserver> observer) {
        if (observer != nullptr) {
            observer->OnGetBatteryLevelEvent(device, batteryLevel);
        }
    });
}

void BasServiceImpl::NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel)
{
    basObservers_.ForEach([device, batteryLevel](std::shared_ptr<IBasObserver> observer) {
        if (observer != nullptr) {
            observer->OnBatteryLevelChanged(device, batteryLevel);
        }
    });
}

void BasServiceImpl::RemoveStateMachine(const std::string &device)
{
    BasMessage event(BAS_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = device;
    PostEvent(event);
}

int BasServiceImpl::IsLocalDeviceConnectAllowed(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::shared_ptr<BasStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (ret && stateMachine != nullptr) {
        int slcState = stateMachine->GetDeviceStateInt();
        if ((slcState >= BAS_STATE_CONNECTED) || (slcState == BAS_STATE_CONNECTING)) {
            HILOGE("[BAS_SERVICE_IMPL]This device has connected!");
            return Bluetooth::BT_ERR_INTERNAL_ERROR;
        }
    }

    if (GetConnectedDeviceNum() >= maxConnectionsNum_) {
        HILOGE("[BAS_SERVICE_IMPL]Max connection number has reached!");
        return Bluetooth::BT_ERR_MAX_CONNECTION;
    }
    return Bluetooth::BT_NO_ERROR;
}

std::string BasServiceImpl::GetEventName(int what)
{
    switch (what) {
        case BAS_SERVICE_STARTUP_EVT:
            return "BAS_SERVICE_STARTUP_EVT";
        case BAS_SERVICE_SHUTDOWN_EVT:
            return "BAS_SERVICE_SHUTDOWN_EVT";
        case BAS_CONNECT_EVT:
            return "BAS_CONNECT_EVT";
        case BAS_DISCONNECT_EVT:
            return "BAS_DISCONNECT_EVT";
        case BAS_CONNECTION_STATE_CHANGED_EVT:
            return "BAS_CONNECTION_STATE_CHANGED_EVT";
        case BAS_REMOVE_STATE_MACHINE_EVT:
            return "BAS_REMOVE_STATE_MACHINE_EVT";
        case BAS_CONNECTION_TIMEOUT_EVT:
            return "BAS_CONNECTION_TIMEOUT_EVT";
        case BAS_DISCONNECTION_TIMEOUT_EVT:
            return "BAS_DISCONNECTION_TIMEOUT_EVT";
        case BAS_READ_BATTERY_EVT:
            return "BAS_READ_BATTERY_EVT";
        case BAS_SERVICE_DISCOVERED_EVT:
            return "BAS_SERVICE_DISCOVERED_EVT";
        case BAS_BATTERY_LEVEL_READ_EVT:
            return "BAS_BATTERY_LEVEL_READ_EVT";
        case BAS_BATTERY_LEVEL_CHANGED_EVT:
            return "BAS_BATTERY_LEVEL_CHANGED_EVT";
        default:
            return "Unknown";
    }
}

int BasServiceImpl::GetConnectedDeviceNum()
{
    int size = 0;
    stateMachines_.Iterate([&size](const std::string device, std::shared_ptr<BasStateMachine> &stateMachine) {
        if (stateMachine != nullptr) {
            auto connectionState = stateMachine->GetDeviceStateInt();
            if ((connectionState == BAS_STATE_CONNECTING) || (connectionState >= BAS_STATE_CONNECTED)) {
                size++;
            }
        }
    });
    return size;
}

}  // namespace bluetooth
}  // namespace OHOS