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
#define LOG_TAG "bt_service_hid_host"
#endif

#include "hid_host_service.h"

#include "adapter_manager.h"
#include "bluetooth_errorcode.h"
#include "connect_strategy_manager.h"
#include "common_util.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_dft_exception.h"
#include "permission_manager.h"
#include "thread_util.h"
#include "hitrace_meter.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"
#include "btif_common.h"  // do_in_jni_thread
#include "bluetooth_common_event_helper.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
HidHostService::HidHostService() : utility::Context(PROFILE_NAME_HID_HOST, "1.1.1")
{}

HidHostService::~HidHostService()
{}

utility::Context *HidHostService::GetContext()
{
    return this;
}

HidHostService *HidHostService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<HidHostService *>(servManager->GetProfileService(PROFILE_NAME_HID_HOST));
}

void HidHostService::RegisterObserver(IHidHostObserver &hidHostObserver)
{
    hidHostObservers_.Register(hidHostObserver);
}

void HidHostService::DeregisterObserver(IHidHostObserver &hidHostObserver)
{
    hidHostObservers_.Deregister(hidHostObserver);
}

void HidHostService::NotifyStateChanged(const RawAddress &device, int state)
{
    HITRACE_METER(BT_TRACE_TAG);
    // Reference "HID_HOST_STATE_CONNECTED"
    HILOGI("[HID_SERVICE]ConnectState:%{public}d", state);
    int newState = stateMap_.at(state);
    if (newState == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_HID_HOST, false);
    }
    int cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device,
        PROFILE_ID_HID_HOST, newState);
    NotifyConnStateChangedInner(device, newState, cause);
}

void HidHostService::Enable(void)
{
    HidHostMessage event(HID_HOST_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void HidHostService::Disable(void)
{
    HidHostMessage event(HID_HOST_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

void HidHostService::ConnectionStateCallbackInner(RawAddress rawAddr, int state)
{
    if (!IsAcceptConnection(rawAddr, state)) {
        if (bluetoothHidInterface != nullptr) {
            STACK::RawAddress device = ServiceUtil::AddrToStack(rawAddr);
            bluetoothHidInterface->disconnect(&device, BLE_ADDR_PUBLIC, BT_TRANSPORT_BR_EDR, false);
        }
        return;
    }
    HidHostMessage event(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.sendData_.param = state;
    PostEvent(event);
}

uint8_t HidHostService::CovertConnectStateFromStack(BthhConnectionState state)
{
    if (state == BTHH_CONN_STATE_CONNECTING) {
        return HID_HOST_STATE_CONNECTING;
    }
    if (state == BTHH_CONN_STATE_DISCONNECTING) {
        return HID_HOST_STATE_DISCONNECTING;
    }
    if (state == BTHH_CONN_STATE_CONNECTED) {
        return HID_HOST_STATE_CONNECTED;
    }
    return HID_HOST_STATE_DISCONNECTED;
}

void HidHostService::HidProcessBtChrEvent(const std::string& addr, int toState)
{
    BtChrEventWriteInt(CHR_USER_DISCONNECT, addr, "HIDTYPE", HIDHOSTTYPE);
    if (toState == HID_HOST_STATE_CONNECTING) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDCONNECTIONTIME");
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "HIDSTATUS", HID_HOST_STATE_CONNECTING);
    } else if (toState == HID_HOST_STATE_CONNECTED) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDCONNECTEDTIME");
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "HIDSTATUS", HID_HOST_STATE_CONNECTED);
    } else if (toState == HID_HOST_STATE_DISCONNECTED) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDDISCONNECTEDTIME");
    } else {
    }
}

void HidHostService::ConnectionStateCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhConnectionState state)
{
    HILOGI("[HID_SERVICE]HidConnectState = %{public}d", state);
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    int connectState = HidHostService::CovertConnectStateFromStack(state);
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    hidHostService->HidProcessBtChrEvent(rawAddr.GetAddress(), connectState);
    hidHostService->ConnectionStateCallbackInner(rawAddr, connectState);
}

bool HidHostService::IsAcceptConnection(RawAddress &rawAddr, int state)
{
    int connectStrategy = GetConnectStrategy(rawAddr);
    if (state == HID_HOST_STATE_CONNECTED
        && connectStrategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGW("rejected incoming HID connection");
        return false;
    }
    return true;
}

void HidHostService::GetProtocolModeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhStatus hhStatus, BthhProtocolMode mode)
{
}

void HidHostService::GetIdleTimeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhStatus hhStatus, int idleTime)
{
}

void HidHostService::GetReportCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhStatus hhStatus, uint8_t* rptData, int rptSize)
{
}

void HidHostService::VirtualUnplugCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhStatus hhStatus)
{
}

void HidHostService::HandshakeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
    BtTransport transport, BthhStatus hhStatus)
{
}

BthhCallbacks sBluetoothHidCallbacks = {
    sizeof(sBluetoothHidCallbacks),
    HidHostService::ConnectionStateCallback,
    nullptr,
    HidHostService::GetProtocolModeCallback,
    HidHostService::GetIdleTimeCallback,
    HidHostService::GetReportCallback,
    HidHostService::VirtualUnplugCallback,
    HidHostService::HandshakeCallback
};

void HidHostService::PostEvent(const HidHostMessage &event)
{
    DoInHidHostThread([this, event] {this->ProcessEvent(event);});
}

void HidHostService::ProcessEvent(const HidHostMessage &event)
{
    std::string address = event.dev_;
    HILOGI("[HID_HOST_SERVICE]Address=[%{public}s], eventName=[%{public}s]", GetEncryptAddr(address).c_str(),
        GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case HID_HOST_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case HID_HOST_CONNECT_EVT:
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectEvent(event);
            break;
        case HID_HOST_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event.dev_);
            break;
        default:
            ProcessDefaultEvent(event);
            break;
    }
}

int HidHostService::GetConnectedDeviceNum()
{
    int size = 0;
    stateMachines_.Iterate([&size](const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        if (stateMachine != nullptr) {
            auto connectionState = stateMachine->GetDeviceStateInt();
            if ((connectionState == HID_HOST_STATE_CONNECTING) || (connectionState >= HID_HOST_STATE_CONNECTED)) {
                size++;
            }
        }
    });
    return size;
}

void HidHostService::ProcessConnectEvent(const HidHostMessage &event)
{
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr && stateMachine->IsRemoving()) {
        // peer device may send connect request before we remove statemachine for last connection.
        // so post this connect request, process it after we remove statemachine completely.
        PostEvent(event);
    } else if (!ret || stateMachine == nullptr) {
        stateMachines_.EnsureInsert(event.dev_, std::make_shared<HidHostStateMachine>(event.dev_));
        stateMachines_.ReadVal(event.dev_)->Init();
        stateMachines_.ReadVal(event.dev_)->ProcessMessage(event);
    } else {
        stateMachine->ProcessMessage(event);
    }
}

void HidHostService::ProcessDefaultEvent(const HidHostMessage &event)
{
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr) {
        stateMachine->ProcessMessage(event);
    } else {
        HILOGE("[HID_SERVICE]Invalid address[%{public}s]", GetEncryptAddr(event.dev_).c_str());
    }
}

int HidHostService::GetMaxConnectedDeviceNum() const
{
    int number = HID_HOST_MAX_DEFAULT_CONNECTIONS_NUM;
    return number;
}

void HidHostService::StartUp()
{
    if (isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_HID_HOST, true);
        HILOGW("[HID_SERVICE]HidHostService has already been started before.");
        return;
    }
    BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("[HID_SERVICE]bluetoothInterface is null");
        return;
    }
    bluetoothHidInterface = reinterpret_cast<BthhInterface*>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_PROFILE_HIDHOST_ID)));
    if (bluetoothHidInterface == nullptr) {
        HILOGE("[HID_SERVICE]bluetoothHidInterface is null");
        return;
    }
    if (!AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        BtStackStatus status = bluetoothHidInterface->init(&sBluetoothHidCallbacks);
        if (status != BT_STATUS_SUCCESS) {
            HILOGI("[HID_SERVICE]Failed to initialize Bluetooth HID, status: %{public}d", status);
            return;
        }
    }
    maxConnectionsNum_ = GetMaxConnectedDeviceNum();
    GetContext()->OnEnable(PROFILE_NAME_HID_HOST, true);
    isStarted_ = true;
}

void HidHostService::ShutDown()
{
    if (!isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_HID_HOST, true);
        HILOGW("[HID_SERVICE]HidHostService has already been shutdown before.");
        return;
    }
    isShuttingDown_ = true;
    bool isDisconnected = false;
    utility::SafeVector<std::string> deviceToDisconnect {};
    stateMachines_.Iterate([&isDisconnected, &deviceToDisconnect](
        const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        if ((stateMachine != nullptr) && (stateMachine->GetDeviceStateInt() > HID_HOST_STATE_DISCONNECTED)) {
            deviceToDisconnect.PushNoDuplicate(device);
            isDisconnected = true;
        }
    });
    for (std::string address : deviceToDisconnect.GetVector()) {
        Disconnect(RawAddress(address));
    }
    if (!isDisconnected) {
        ShutDownDone(true);
    }
}

void HidHostService::ShutDownDone(bool isAllDisconnected)
{
    if (!isAllDisconnected) {
        stateMachines_.Iterate([&isAllDisconnected](
            const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
            if ((stateMachine != nullptr) && (stateMachine->GetDeviceStateInt() > HID_HOST_STATE_DISCONNECTED)) {
                isAllDisconnected = true;
            }
        });
        CHECK_AND_RETURN_LOG(!isAllDisconnected, "not all device disconnected.");
    }
    stateMachines_.Clear();

    GetContext()->OnDisable(PROFILE_NAME_HID_HOST, true);
    isShuttingDown_ = false;
    isStarted_ = false;
    if (bluetoothHidInterface != nullptr) {
        bluetoothHidInterface->cleanup();
        bluetoothHidInterface = nullptr;
    }
    HILOGI("[HID_SERVICE]HidHostService shutdown");
}

void HidHostService::CleanUpAllStateMachine()
{
    stateMachines_.Iterate([this](const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        if (stateMachine != nullptr && stateMachine->GetDeviceStateInt() != HID_HOST_STATE_DISCONNECTED) {
            HidHostMessage event(HID_HOST_CONNECTION_STATE_CHANGED_EVT);
            event.dev_ = device;
            event.sendData_.param = HID_HOST_STATE_DISCONNECTED;
            PostEvent(event);
        }
    });
}

void HidHostService::ClearUpStackHidProfile()
{
    HILOGI("ClearUpStackHidProfile");
    if (bluetoothHidInterface != nullptr) {
        bluetoothHidInterface->cleanup();
    }
}

void HidHostService::ReStartStackHidProfile()
{
    if (bluetoothHidInterface == nullptr) {
        HILOGI("bluetoothHidInterface is null");
        return;
    }
    BtStackStatus status = bluetoothHidInterface->init(&sBluetoothHidCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGI("[HID_SERVICE]Failed to ReStart Stack Bluetooth HID, status: %{public}d", status);
    }
}

void HidHostService::NotifyConnStateChangedInner(const RawAddress &device, int state, int cause)
{
    hidHostObservers_.ForEach([device, state, cause](IHidHostObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
    BluetoothHelper::BluetoothCommonEventHelper::PublishHidConnectionStateEvent(
        device.GetAddress(), state);
}

int HidHostService::IsLocalDeviceConnectAllowed(const RawAddress &device)
{
    ControlInterceptMessage msg {
        .addr = device.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedHidConn(msg),
        BT_ERR_INTERNAL_ERROR, "Restricted by control intercept");
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (ret && stateMachine != nullptr) {
        int slcState = stateMachine->GetDeviceStateInt();
        if ((slcState >= HID_HOST_STATE_CONNECTED) || (slcState == HID_HOST_STATE_CONNECTING)) {
            HILOGE("[HID_SERVICE]This device has connected!");
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_CONN, device, UE_COMMON_SCENE_CASE2,
                callingName);
            return Bluetooth::BT_ERR_INTERNAL_ERROR;
        }
    }

    if (GetConnectedDeviceNum() >= maxConnectionsNum_) {
        HILOGE("[HID_SERVICE]Max connection number has reached!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_CONN, device, UE_COMMON_SCENE_CASE3, callingName);
        NotifyConnStateChangedInner(device, GetDeviceState(device),
            static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
        return Bluetooth::BT_ERR_MAX_CONNECTION;
    }
    if (GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGE("[HID_SERVICE]Connection Strategy is FORBIDDEN!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_CONN, device, UE_COMMON_SCENE_CASE4, callingName);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return Bluetooth::BT_NO_ERROR;
}

int HidHostService::Connect(const RawAddress &device)
{
    int ret = IsLocalDeviceConnectAllowed(device);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGE("LocalDevice Connect not Allowed");
        return ret;
    }
    if (!IsHidSupportedByRemoteDevice(device)) {
        HILOGE("hid not Supported, ble device pending!");
        int deviceType = RemoteDeviceProperties::GetInstance()->GetDeviceType(device);
        if (deviceType == DEVICE_TYPE_LE) {
            auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
            CHECK_AND_RETURN_LOG_RET(classicAdapter != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR,
                "classicAdapter is null");
            classicAdapter->AddPendingConnectDevice(device, PROFILE_ID_HID_HOST);
            // 避免ble设备首次配对场景下service没有拿到RemoteUuid导致连接被阻塞，这里主动去获取一次服务
            STACK::RawAddress bdaddr = ServiceUtil::AddrToStack(device);
            BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
            CHECK_AND_RETURN_LOG_RET(bluetoothInterface != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR,
                "bluetoothInterface is null");
            bluetoothInterface->getRemoteServices(&bdaddr);
        }
        return HID_HOST_SUCCESS;
    }
    HidHostMessage event(HID_HOST_CONNECT_EVT);
    event.dev_ = device.GetAddress();
    PostEvent(event);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_CONN, device,
        UE_COMMON_SCENE_CASE1, PermissionManager::GetCallingName());
    return HID_HOST_SUCCESS;
}

int HidHostService::Disconnect(const RawAddress &device)
{
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (!ret || stateMachine == nullptr) {
        HILOGW("[HID_SERVICE]The state machine is not available!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_DISCONN, device, UE_COMMON_SCENE_CASE2, callingName);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }

    int slcState = stateMachine->GetDeviceStateInt();
    if ((slcState != HID_HOST_STATE_CONNECTING) && (slcState < HID_HOST_STATE_CONNECTED)) {
        HILOGW("[HID_SERVICE]This device not connected, slcState:%{public}d", slcState);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_DISCONN, device, UE_COMMON_SCENE_CASE3, callingName);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }
    RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
        PROFILE_ID_HID_HOST, true);

    HidHostMessage event(HID_HOST_DISCONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HID_DISCONN, device, UE_COMMON_SCENE_CASE1, callingName);
    return HID_HOST_SUCCESS;
}

int HidHostService::HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    HidHostMessage event(HID_HOST_VC_UNPLUG_EVT);
    event.dev_ = device;
    PostEvent(event);
    return HID_HOST_SUCCESS;
}

/**
  * 这里的参数和bluedroid对应不上，bluedroid需要传入一个string字符串boolean sendData(BluetoothDevice device, String report)
  * 无法对应
  */
int HidHostService::HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    HidHostMessage event(HID_HOST_SEND_DATA_EVT);
    event.dev_ = device;
    PostEvent(event);
    return HID_HOST_SUCCESS;
}

int HidHostService::HidHostSetReport(std::string device, uint8_t type, uint16_t size, const uint8_t* report)
{
    HidHostMessage event(HID_HOST_SET_REPORT_EVT);
    event.dev_ = device;
    event.sendData_.type = type;
    if ((size > 0) && (report != nullptr)) {
        event.dataLength_ = static_cast<int>(size);
        event.data_ = std::make_unique<uint8_t[]>(size);
        if (memcpy_s(event.data_.get(), size, report, size) != EOK) {
            HILOGE("[HID_SERVICE]Memcpy fail error");
            return Bluetooth::BT_ERR_INTERNAL_ERROR;
        }
    }
    PostEvent(event);
    return HID_HOST_SUCCESS;
}

int HidHostService::HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    HidHostMessage event(HID_HOST_GET_REPORT_EVT);
    event.dev_ = device;
    event.sendData_.type = type;
    event.sendData_.dataSize = size;
    event.sendData_.reportId = id;
    PostEvent(event);
    return HID_HOST_SUCCESS;
}

int HidHostService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HID_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if ((GetDeviceState(device)) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect hid.");
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if ((GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTED) &&
            (GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
            HILOGI("disconnect hid.");
        }
    } else {
        HILOGE("Strategy set failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    return Bluetooth::BT_NO_ERROR;
}

int HidHostService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HID_CONNECTION_POLICY);
}

std::vector<RawAddress> HidHostService::GetDevicesByStates(std::vector<int> states)
{
    std::vector<RawAddress> devices;
    stateMachines_.Iterate(
        [this, &devices, &states](const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        RawAddress addr(device);
        int deviceState = stateMap_.at(HID_HOST_STATE_DISCONNECTED);
        if (stateMachine != nullptr) {
            int state = stateMachine->GetDeviceStateInt();
            if (state >= HID_HOST_STATE_CONNECTED) {
                deviceState = stateMap_.at(HID_HOST_STATE_CONNECTED);
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

int HidHostService::GetDeviceState(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::shared_ptr<HidHostStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(address, stateMachine);
    if (!ret || stateMachine == nullptr) {
        HILOGD("[HID_SERVICE]GetDeviceState:The state machine is not available!");
        return stateMap_.at(HID_HOST_STATE_DISCONNECTED);
    }

    int state = stateMachine->GetDeviceStateInt();
    if (state >= HID_HOST_STATE_CONNECTED) {
        return stateMap_.at(HID_HOST_STATE_CONNECTED);
    } else {
        return stateMap_.at(state);
    }
}

std::list<RawAddress> HidHostService::GetConnectDevices(void)
{
    std::list<RawAddress> devList;
    stateMachines_.Iterate([&devList](const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        if (stateMachine != nullptr && (stateMachine->GetDeviceStateInt() >= HID_HOST_STATE_CONNECTED)) {
            devList.push_back(RawAddress(device));
        }
    });
    return devList;
}

int HidHostService::GetConnectState(void)
{
    uint8_t result = 0;
    stateMachines_.Iterate([&result](const std::string device, std::shared_ptr<HidHostStateMachine> &stateMachine) {
        if (stateMachine == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (stateMachine->GetDeviceStateInt() >= HID_HOST_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (stateMachine->GetDeviceStateInt() == HID_HOST_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == HID_HOST_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == HID_HOST_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    });
    HILOGI("[HID_SERVICE]Result=%{public}d", result);
    return static_cast<int>(result);
}

int HidHostService::GetMaxConnectNum(void)
{
    HILOGI("[HID_SERVICE]GetMaxConnectNum:%{public}d", maxConnectionsNum_);
    return maxConnectionsNum_;
}

void HidHostService::RemoveStateMachine(const std::string &device)
{
    HidHostMessage event(HID_HOST_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = device;
    PostEvent(event);
}

void HidHostService::ProcessRemoveStateMachine(const std::string &address)
{
    stateMachines_.EnsureInsert(address, nullptr);
    if (isShuttingDown_) {
        ShutDownDone(false);
    }
}

BthhInterface* HidHostService::getBluetoothHidInterface() const
{
    return bluetoothHidInterface;
}

std::string HidHostService::GetEventName(int what)
{
    switch (what) {
        case HID_HOST_SERVICE_STARTUP_EVT:
            return "HID_HOST_SERVICE_STARTUP_EVT";
        case HID_HOST_SERVICE_SHUTDOWN_EVT:
            return "HID_HOST_SERVICE_SHUTDOWN_EVT";
        case HID_HOST_CONNECT_EVT:
            return "HID_HOST_CONNECT_EVT";
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT:
            return "HID_HOST_CONNECTION_STATE_CHANGED_EVT";
        case HID_HOST_REMOVE_STATE_MACHINE_EVT:
            return "HID_HOST_REMOVE_STATE_MACHINE_EVT";
        default:
            return "Unknown";
    }
}

bool HidHostService::IsHidSupportedByRemoteDevice(const RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST)) != uuids.end() ||
        std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HOGP)) != uuids.end();
}

REGISTER_CLASS_CREATOR(HidHostService);
}  // namespace bluetooth
}  // namespace OHOS
