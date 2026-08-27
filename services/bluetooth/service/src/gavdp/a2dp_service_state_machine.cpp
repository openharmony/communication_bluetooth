/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_a2dp"
#endif

#include "a2dp_service_state_machine.h"
#include "a2dp_service.h"
#include "adapter_manager.h"
#include "log.h"
#include "classic_adapter.h"
#include "common_util.h"
#include "thread_util.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "permission_manager.h"
#include "remote_device_properties.h"
#include "common/bluetooth_hw_interface.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
void A2dpDisconnected::Entry()
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress device = stateManager_.GetDevice();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null");
    std::shared_ptr<A2dpDeviceInfo> a2dpDevice = service->GetDeviceFromList(device);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(a2dpDevice != nullptr && bluetoothDevice != nullptr, "a2dpDevice or device is null");
    bool isNeedRetryConnect = a2dpDevice->GetPreConnectState() == static_cast<int>(BTConnectState::CONNECTING) &&
        a2dpDevice->GetTryReconnectA2dp() &&
        bluetoothDevice->IsAclConnected() && bluetoothDevice->IsPaired() && !bluetoothDevice->isBandMode();
    if (isNeedRetryConnect) {
        CHECK_AND_RETURN_LOG(AdapterManager::GetInstance() != nullptr &&
            AdapterManager::GetInstance()->GetClassicAdapter() != nullptr, "service is null");
        bool checkRetryConnect = !AdapterManager::GetInstance()->GetClassicAdapter()->IsIncomingConnection(device) &&
            bluetoothDevice->GetAutoConnSwitch() != 0;
        if (checkRetryConnect) {
            HILOGI("DISCONNECTED reconnect ignore, do not allow");
            return;
        }
        bluetoothDevice->DialogControlAddConnectingProfileCount();
        uint delayReconnectTime = 50;
        auto reconnectFunc = [service, device]() {
            std::shared_ptr<BluetoothDevice> bluetoothDevice =
                RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
            if (bluetoothDevice != nullptr && bluetoothDevice->IsAclConnected()) {
                HILOGI("reconnect a2dp, device is %{public}s", GET_ENCRYPT_ADDR(device));
                service->ConnectManager().A2dpConnect(device);
            }
        };
        ThreadUtil::GetInstance().PostTask(THREAD_ID_A2DP, reconnectFunc, delayReconnectTime, "A2dpReconnectFunc");
        a2dpDevice->SetTryReconnectA2dp(false);
    }
}

bool A2dpDisconnected::Dispatch(const utility::Message &msg)
{
    HILOGI("[A2dpDisconnected] role[%{public}u]", msg.arg1_);

    if (msg.arg2_ == nullptr) {
        HILOGE("[A2dpDisconnected] msg.arg2_ is null");
        return false;
    }

    int ret = BT_STATUS_SUCCESS;
    RawAddress device = *(static_cast<RawAddress *>(msg.arg2_));
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    A2dpService *service = GetServiceInstance(msg.arg1_);
    if (service == nullptr) {
        HILOGE("[A2dpDisconnected] service is null");
        return false;
    }

    btav_source_interface_t* bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothA2dpSrcInterface, false, "bluetoothA2dpSrcInterface is nullptr");

    switch (msg.what_) {
        case A2DP_MSG_CONNECT:
            Transition(A2DP_STATE_CONNECTING);
            ret = bluetoothA2dpSrcInterface->connect(rawAddr);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::CONNECTING), device);
            break;
        case A2DP_MSG_PROFILE_CONNECTED:
            Transition(A2DP_STATE_CONNECTED);
            break;
        case A2DP_MSG_PROFILE_CONNECTING:
            Transition(A2DP_STATE_CONNECTING);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::CONNECTING), device);
            break;
        case A2DP_MSG_CONNECT_FORBIDDEN:
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            break;
        default:
            break;
    }

    return (ret == BT_STATUS_SUCCESS) ? true : false;
}

void A2dpDisconnecting::Entry()
{
    stateManager_.StartConnectionTimer();
}

void A2dpDisconnecting::Exit()
{
    stateManager_.StopConnectionTimer();
}

bool A2dpDisconnecting::Dispatch(const utility::Message &msg)
{
    HILOGI("[A2dpDisconnecting] role[%{public}u]", msg.arg1_);
    if (msg.arg2_ == nullptr) {
        HILOGE("[A2dpDisconnecting] msg.arg2_ is null");
        return false;
    }

    int ret = BT_STATUS_SUCCESS;
    RawAddress device = *(static_cast<RawAddress *>(msg.arg2_));
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    A2dpService *service = GetServiceInstance(msg.arg1_);
    if (service == nullptr) {
        HILOGE("[A2dpDisconnecting] service is null");
        return false;
    }

    btav_source_interface_t* bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothA2dpSrcInterface, false, "bluetoothA2dpSrcInterface is nullptr");

    switch (msg.what_) {
        case A2DP_MSG_CONNECT_TIMEOUT: {
            Transition(A2DP_STATE_DISCONNECTED);
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTED), device);
            BtChrBtExcpEvent(rawAddr.ToString(), BTOPT_A2DP, CHR_SUB_ERRCODE_CASE1);
            std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(device);
            CHECK_AND_RETURN_LOG_RET(deviceInfo != nullptr, false, "not find device");
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            break;
        }
        case A2DP_MSG_PROFILE_DISCONNECTED:
            Transition(A2DP_STATE_DISCONNECTED);
            break;
        default:
            break;
    }
    return (ret == BT_STATUS_SUCCESS) ? true : false;
}

bool A2dpConnected::Dispatch(const utility::Message &msg)
{
    CHECK_AND_RETURN_LOG_RET(msg.arg2_, false, "msg.arg2_ is null");
    int ret = BT_STATUS_SUCCESS;
    uint8_t role = msg.arg1_;
    RawAddress device = *(static_cast<RawAddress *>(msg.arg2_));
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);

    A2dpService *service = GetServiceInstance(role);
    if (service == nullptr) {
        HILOGE("[A2dpConnected] service is null");
        return false;
    }
    btav_source_interface_t* bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothA2dpSrcInterface, false, "bluetoothA2dpSrcInterface is nullptr");
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHwSrcInterface, false, "bluetoothHwSrcInterface interface nullptr");

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(device);
    CHECK_AND_RETURN_LOG_RET(deviceInfo, false, "[A2dpConnected] Can't get the device by address");
    switch (msg.what_) {
        case A2DP_MSG_DISCONNECT:
            Transition(A2DP_STATE_DISCONNECTING);
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTING), device);
            break;
        case A2DP_MSG_PROFILE_DISCONNECTED:
            Transition(A2DP_STATE_DISCONNECTED);
            break;
        case A2DP_MSG_PROFILE_DISCONNECTING:
            Transition(A2DP_STATE_DISCONNECTING);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTING), device);
            break;
        case A2DP_MSG_PROFILE_AUDIO_PLAY_START:
            UpdateDeviceInformation(device, true, role);
            bluetoothHwSrcInterface->a2dpOffloadSetSinkStreamStart(rawAddr);
            break;
        case A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND:
        case A2DP_MSG_PROFILE_AUDIO_PLAY_STOP:
            UpdateDeviceInformation(device, false, role);
            bluetoothHwSrcInterface->a2dpOffloadSetSinkStreamStop(rawAddr);
            break;
        case A2DP_MSG_PROFILE_CONNECTING:
            Transition(A2DP_STATE_CONNECTING);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::CONNECTING), device);
            UpdateDeviceInformation(device, false, role);
            break;
        default:
            break;
    }
    return (ret == BT_STATUS_SUCCESS) ? true : false;
}

void A2dpConnected::UpdateDeviceInformation(RawAddress rawAddr, bool value, uint8_t role)
{
    HILOGI("[A2dpConnected] PlayingStatus[%{public}d]", value);
    A2dpService *service = GetServiceInstance(role);
    if (service == nullptr) {
        HILOGE("[A2dpConnected] Can't get the service of a2dp");
        return;
    }

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(rawAddr);
    if (deviceInfo == nullptr) {
        HILOGE("[A2dpConnected] Can't get the device by address");
        return;
    }

    if (deviceInfo->GetPlayingState()) {
        deviceInfo->SetPlayingState(value);
        std::string callingName = PermissionManager::GetCallingName();
        BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(rawAddr, value,
            deviceInfo->GetCodecStatus().codecInfo.codecType, callingName);
    }
}

void A2dpConnecting::Entry()
{
    stateManager_.StartConnectionTimer();
}

void A2dpConnecting::Exit()
{
    stateManager_.StopConnectionTimer();
}

bool A2dpConnecting::Dispatch(const utility::Message &msg)
{
    HILOGI("[A2dpConnecting] role[%{public}u]", msg.arg1_);
    if (msg.arg2_ == nullptr) {
        HILOGE("[A2dpConnecting] msg.arg2_ is null");
        return false;
    }

    int ret = BT_STATUS_SUCCESS;
    uint8_t role = msg.arg1_;
    RawAddress device = *(static_cast<RawAddress *>(msg.arg2_));
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    A2dpService *service = GetServiceInstance(role);
    if (service == nullptr) {
        HILOGE("[A2dpConnected] Can't get the service of a2dp");
        return BT_STATUS_FAIL;
    }
    btav_source_interface_t* bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothA2dpSrcInterface, false, "bluetoothA2dpSrcInterface is nullptr");

    switch (msg.what_) {
        case A2DP_MSG_CONNECT_TIMEOUT: {
            Transition(A2DP_STATE_DISCONNECTED);
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTED), device);
            BtChrBtExcpEvent(rawAddr.ToString(), BTOPT_A2DP, CHR_SUB_ERRCODE_CASE1);
            std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(device);
            CHECK_AND_RETURN_LOG_RET(deviceInfo != nullptr, false, "not find device");
            deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
            break;
        }
        case A2DP_MSG_PROFILE_CONNECTED:
            Transition(A2DP_STATE_CONNECTED);
            break;
        case A2DP_MSG_PROFILE_DISCONNECTED:
            Transition(A2DP_STATE_DISCONNECTED);
            break;
        case A2DP_MSG_PROFILE_DISCONNECTING:
            Transition(A2DP_STATE_DISCONNECTING);
            service->ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTING), device);
            break;
        case A2DP_MSG_DISCONNECT:
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            break;
        case A2DP_MSG_CONNECT_FORBIDDEN:
            ret = bluetoothA2dpSrcInterface->disconnect(rawAddr);
            break;
        default:
            break;
    }
    return (ret == BT_STATUS_SUCCESS) ? true : false;
}

void A2dpStateManager::SetRole(uint8_t role)
{
    role_ = role;
}

void A2dpStateManager::SetDevice(const RawAddress &device)
{
    device_ = device;
}

RawAddress A2dpStateManager::GetDevice() const
{
    return device_;
}

void A2dpStateManager::ConnectionTimeout() const
{
    auto func = [this]() {
        RawAddress device = device_;
        utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, role_, &device);
        HILOGE("A2dpConnectionTimeout: %{public}s", GET_ENCRYPT_ADDR(device));
        ProcessMessage(msg);
    };

    DoInA2dpThread(func);
}

void A2dpStateManager::StartConnectionTimer() const
{
    if (connTimer_ == nullptr) {
        HILOGE("connTimer_ null");
        return;
    }

    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("[A2DP_STATE_MACHINE]Start connection timer!");
}

void A2dpStateManager::StopConnectionTimer() const
{
    if (connTimer_ == nullptr) {
        HILOGE("connTimer_ null");
        return;
    }
    
    connTimer_->Stop();
    HILOGI("[A2DP_STATE_MACHINE]Stop connection timer!");
}

}  // namespace bluetooth
}  // namespace OHOS
