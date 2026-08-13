/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_pan_service"
#endif

#include "pan_service.h"

#include "adapter_manager.h"
#include "bluetooth_state_manager.h"
#include "connect_strategy_manager.h"
#include "log.h"
#include "common_util.h"
#include "pan_defines.h"
#include "thread_util.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
PanService::PanService() : utility::Context(PROFILE_NAME_PAN, "1.0"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS,
            DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{}

PanService::~PanService() {}

void PanService::LoadPanServiceManagerInterfaceLib(void)
{
    if (isLoaded_) {
        return;
    }
    loader_.OpenLib();
    isLoaded_ = true;
    serviceImpl_ = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
    serviceImpl_->DllIsTetheringOnFunc(
        [this]() {
            return IsTetheringOn();
        }
    );
    serviceImpl_->DllNotifyStateChangedFunc(
        [this](const bluetooth::RawAddress &device, int state, int role) {
            NotifyStateChanged(device, state, role);
        }
    );
    serviceImpl_->DllPostEventFunc(
        [this](const PanMessage &event) {
            PostEvent(event);
        }
    );

    serviceImpl_->DllGetDeviceStateFunc(
        [this](const bluetooth::RawAddress &device) {
            return GetDeviceState(device);
        }
    );

    serviceImpl_->DllSetDevProactiveDisconnectFlagFunc(
        [](const std::string &addr, uint32_t profileId, bool isProactive) {
            RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(addr, profileId, isProactive);
        }
    );
}

void PanService::UnloadPanServiceManagerInterfaceLib(void)
{
    serviceImpl_.reset();
    loader_.CloseLib();
    isLoaded_ = false;
}

utility::Context *PanService::GetContext()
{
    return this;
}

PanService *PanService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    if (servManager == nullptr) {
        HILOGI("servManager is nullptr.");
        return nullptr;
    }
    return static_cast<PanService *>(servManager->GetProfileService(PROFILE_NAME_PAN));
}

void PanService::RegisterObserver(IPanObserver &panObserver)
{
    panObservers_.Register(panObserver);
}

void PanService::DeregisterObserver(IPanObserver &panObserver)
{
    panObservers_.Deregister(panObserver);
}

void PanService::Enable(void)
{
    PanMessage event(PAN_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void PanService::Disable(void)
{
    PanMessage event(PAN_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

int PanService::Connect(const bluetooth::RawAddress &device)
{
    if (GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGE("Connection Strategy is FORBIDDEN!");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (!IsRemotePanSupported(device)) {
        HILOGE("Cannot connect %{public}s, no pan UUID ", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    LoadPanServiceManagerInterfaceLib();
    PanMessage event(PAN_CONNECT_EVT);
    event.dev_ = device.GetAddress();
    event.remoteRole = REMOTE_NAP_ROLE;
    PostEvent(event);
    return PAN_SUCCESS;
}

int PanService::Disconnect(const bluetooth::RawAddress &device)
{
    if (!IsRemotePanSupported(device)) {
        HILOGE("Cannot disconnect %{public}s, no pan UUID ", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    PanMessage event(PAN_DISCONNECT_EVT);
    event.dev_ = device.GetAddress();
    PostEvent(event);
    return PAN_SUCCESS;
}

int PanService::SetTethering(bool enable)
{
    HILOGI("enable:%{public}d, isTetheringOn_:%{public}d", enable, isTetheringOn_);
    if (!enable && isTetheringOn_ != enable) {
        PanMessage event(PAN_DISCONNECT_ALL_EVT);
        PostEvent(event);
    }
    isTetheringOn_ = enable;
    return PAN_SUCCESS;
}

bool PanService::IsTetheringOn()
{
    return isTetheringOn_;
}

int PanService::SetConnectStrategy(const bluetooth::RawAddress &device, int strategy)
{
    HILOGI("SetConnectStrategy device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_PAN_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t connectionState = GetDeviceState(device);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if (connectionState == static_cast<int>(bluetooth::BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect pan.");
        }
    } else if (strategy == static_cast<int>(bluetooth::BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (connectionState != static_cast<int>(bluetooth::BTConnectState::DISCONNECTED)) {
            Disconnect(device);
            HILOGI("disconnect pan.");
        }
    } else {
        HILOGE("Strategy set failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int PanService::GetConnectStrategy(const bluetooth::RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_PAN_CONNECTION_POLICY);
}

std::vector<bluetooth::RawAddress> PanService::GetDevicesByStates(std::vector<int> states)
{
    CHECK_AND_RETURN_LOG_RET(serviceImpl_ != nullptr, {}, "serviceImpl_ is nullptr");
    return serviceImpl_->GetDevicesByStates(states);
}

int PanService::GetDeviceState(const bluetooth::RawAddress &device)
{
    std::string address = device.GetAddress();
    int state = PAN_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDevicePanStateByAddr(address, state)) {
        HILOGI("The state machine is not available!");
        return stateMap_.at(PAN_STATE_DISCONNECTED);
    }
    if (state >= PAN_STATE_CONNECTED) {
        return stateMap_.at(PAN_STATE_CONNECTED);
    } else {
        return stateMap_.at(state);
    }
}

std::list<bluetooth::RawAddress> PanService::GetConnectDevices()
{
    std::list<std::string> addressList;
    addressList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByPanConnState<std::string,
        int>([](const std::string &address, int &state) -> bool {
            return state >= PAN_STATE_CONNECTED;
    });
    std::list<RawAddress> devList;
    for (std::string &address : addressList) {
        devList.push_back(RawAddress(address));
    }
    return devList;
}

int PanService::GetConnectState(void)
{
    CHECK_AND_RETURN_LOG_RET(serviceImpl_ != nullptr, INVALID_CONNECT_STATE, "serviceImpl_ is nullptr");
    return serviceImpl_->GetConnectState();
}

int PanService::GetMaxConnectNum(void)
{
    return maxConnectionsNum_;
}

btpan_interface_t* PanService::getBluetoothPanInterface() const
{
    return bluetoothPanInterface_;
}

void PanService::NotifyStateChanged(const bluetooth::RawAddress &device, int state, int role)
{
    HITRACE_METER(BT_TRACE_TAG);
    int newState = stateMap_.at(state);
    HILOGI("state = %{public}d, addr = %{public}s", state, GetEncryptAddr(device.GetAddress()).c_str());
    if (state == PAN_STATE_CONNECTED) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_PAN, false);
    }
    if (state == PAN_STATE_NETWORK_ESTABLISHED) {
        return;
    }
    int cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device, PROFILE_ID_PAN, newState);
    panObservers_.ForEach([device, newState, cause, role](IPanObserver &observer) {
        observer.OnConnectionStateChanged(device, newState, cause, role);
    });
}

void PanService::ConnectionStateCallback(btpan_connection_state_t state,
    bt_status_t error, const BLUEDROID::RawAddress* addr, int localRole, int remoteRole)
{
    HILOGI("error=%{public}d, state=%{public}d, addr = %{public}s",
        error, state, GetEncryptAddr(addr->ToString()).c_str());
    if (error != BT_STATUS_SUCCESS) {
        HILOGE("connect error(%{public}d), addr = %{public}s",
            error, GetEncryptAddr(addr->ToString()).c_str());
        return;
    }
    bluetooth::RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*addr);
    auto service = PanService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is nullptr");
    service->LoadPanServiceManagerInterfaceLib();
    int eventId = PAN_DISCONNECTED_IND_EVT;
    int panState = PAN_STATE_DISCONNECTED;
    switch (state) {
        case BTPAN_STATE_CONNECTED:
            eventId = PAN_CONNECTED_IND_EVT;
            panState = PAN_STATE_CONNECTED;
            break;
        case BTPAN_STATE_CONNECTING:
            eventId = PAN_CONNECTING_IND_EVT;
            panState = PAN_STATE_CONNECTING;
            break;
        case BTPAN_STATE_DISCONNECTED:
            eventId = PAN_DISCONNECTED_IND_EVT;
            panState = PAN_STATE_DISCONNECTED;
            break;
        case BTPAN_STATE_DISCONNECTING:
            eventId = PAN_DISCONNECTING_IND_EVT;
            panState = PAN_STATE_DISCONNECTING;
            break;
        default:
            break;
    }
    bluetooth::BluetoothStateManager::GetInstance()->SetRemoteDevicePanState(rawAddr.GetAddress(), panState);
    PanMessage event(eventId);
    event.dev_ = rawAddr.GetAddress();
    event.localRole = localRole;
    event.remoteRole = remoteRole;
    service->PostEvent(event);
}

void PanService::ControlStateCallback(btpan_control_state_t state, int localRole,
    bt_status_t error, const char* ifname)
{
    HILOGI("ifname=%{public}s, error=%{public}d", ifname, error);
    if (error != BT_STATUS_SUCCESS) {
        HILOGE("error= %{public}d", error);
        return;
    }
    PanMessage event(PAN_CONTROL_STATE_CHANGE_EVT);
    event.ifname_ = ifname;
    auto service = PanService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is nullptr");
    service->PostEvent(event);
}

void PanService::RemoveStateMachine(const std::string &device)
{
    PanMessage event(PAN_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = device;
    PostEvent(event);
}

void PanService::PostEvent(const PanMessage &event)
{
    DoInPanThread([this, event] {this->ProcessEvent(event);});
}

void PanService::ProcessEvent(const PanMessage &event)
{
    HILOGD("address[%{public}s] event_no[%{public}d]", 
        GetEncryptAddr(event.dev_).c_str(), event.what_);
    switch (event.what_) {
        case PAN_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case PAN_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case PAN_CONNECT_EVT:
            CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
            serviceImpl_->ProcessConnect(event);
            break;
        case PAN_DISCONNECT_EVT:
            CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
            serviceImpl_->ProcessDisconnect(event);
            break;
        case PAN_DISCONNECTED_IND_EVT:
        case PAN_CONNECTING_IND_EVT:
        case PAN_CONNECTED_IND_EVT:
        case PAN_DISCONNECTING_IND_EVT:
            CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
            serviceImpl_->ProcessConnectStateChange(event);
            break;
        case PAN_CONTROL_STATE_CHANGE_EVT:
            ProcessControlStateChange(event);
            break;
        case PAN_DISCONNECT_ALL_EVT:
            ProcessDisconnectAll();
            break;
        case PAN_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event);
            break;
        case PAN_CONNECT_TIMEOUT_EVT:
            ProcessConnectionTimeout(event);
            break;
        default:
            HILOGW("invalid event: %{public}d", event.what_);
            break;
    }
}

btpan_callbacks_t sBluetoothPanCallbacks = {
    sizeof(sBluetoothPanCallbacks),
    PanService::ControlStateCallback,
    PanService::ConnectionStateCallback,
};


void PanService::StartUp()
{
    HILOGD("[PAN Service]==========<start>==========");
    if (isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_PAN, true);
        HILOGW("PanService has already been started before.");
        return;
    }

    GetContext()->OnEnable(PROFILE_NAME_PAN, true);
    bt_interface_t* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        return;
    }
    bluetoothPanInterface_ = reinterpret_cast<btpan_interface_t*>(
        const_cast<void *>(bluetoothInterface->get_profile_interface(BT_PROFILE_PAN_ID)));
    if (bluetoothPanInterface_ == nullptr) {
        return;
    }
    bt_status_t status = bluetoothPanInterface_->init(&sBluetoothPanCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed to initialize Bluetooth PAN, status: %{public}d", status);
        return;
    }
    HILOGI("[PAN Service]==========<start success>==========");
    isStarted_ = true;
}

void PanService::ShutDown()
{
    if (!isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_PAN, true);
        HILOGW("PanService has already been shutdown before.");
        return;
    }

    isShuttingDown_ = true;
    bool isDisconnected = false;
    std::list<std::string> disconnectAddress;
    disconnectAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByPanConnState<std::string,
        int>([](const std::string &address, int &state) -> bool {
            return state > PAN_STATE_DISCONNECTED;
    });
    for (std::string &address : disconnectAddress) {
        Disconnect(RawAddress(address));
        isDisconnected = true;
    }
    if (!isDisconnected) {
        ShutDownDone();
    }
}

void PanService::ShutDownDone()
{
    GetContext()->OnDisable(PROFILE_NAME_PAN, true);
    isShuttingDown_ = false;
    isStarted_ = false;
    if (bluetoothPanInterface_ != nullptr) {
        bluetoothPanInterface_->cleanup();
        bluetoothPanInterface_ = nullptr;
    }
    if (serviceImpl_ != nullptr) {
        serviceImpl_->ClearStatemachine();
    }
    HILOGI("PanService shutdown");
}

void PanService::ProcessControlStateChange(const PanMessage &msg)
{
    ifname_ = msg.ifname_;
}

void PanService::ProcessDisconnectAll()
{
    std::list<std::string> disconnectAddress;
    disconnectAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByPanConnState<std::string,
        int>([](const std::string &address, int &state) -> bool {
            return state > PAN_STATE_DISCONNECTED;
    });
    for (std::string &address : disconnectAddress) {
        PanMessage event(PAN_DISCONNECT_EVT);
        event.dev_ = address;
        PostEvent(event);
    }
}

void PanService::ProcessRemoveStateMachine(const PanMessage &msg)
{
    CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
    serviceImpl_->RemoveStateMachine(msg.dev_);

    if (isShuttingDown_ && serviceImpl_->IsStateMachineEmpty()) {
        ShutDownDone();
    }
}

void PanService::ProcessConnectionTimeout(const PanMessage &msg)
{
    CHECK_AND_RETURN_LOG(serviceImpl_ != nullptr, "serviceImpl_ is nullptr");
    serviceImpl_->ProcessConnectionTimeout(msg);
}

int PanService::BringUpNetwork()
{
    CHECK_AND_RETURN_LOG_RET(serviceImpl_ != nullptr, PAN_FAILURE, "serviceImpl_ is nullptr");
    return serviceImpl_->BringUpNetwork();
}

bool PanService::IsRemotePanSupported(const bluetooth::RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_PAN)) != uuids.end();
}

REGISTER_CLASS_CREATOR(PanService);
}  // namespace bluetooth
}  // namespace OHOS
