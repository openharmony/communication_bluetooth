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
#define LOG_TAG "bt_service_pan_service_impl"
#endif

#include "pan_service_impl.h"
#include "bluetooth_errorcode.h"
#include "common_util.h"
#include "bt_def.h"
#include "hal_util.h"
#include "pan_network_manager.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>


namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

extern "C" PanServiceImplInterface *CreatePanServiceImplInterface(void)
{
    return new PanServiceImpl();
}

extern "C" void DestroyPanServiceImplInterface(PanServiceImpl *interface)
{
    if (interface == nullptr) {
        HILOGE("pan service interface is nullptr");
        return;
    }
    delete interface;
    interface = nullptr;
}

PanServiceImpl::PanServiceImpl()
{
    bt_interface_t* bluetoothInterface = nullptr;
    int status = hal_util_load_bt_library((const bt_interface_t**)&(bluetoothInterface));
    if (status) {
        HILOGE("Failed to open the Bluetooth stack");
        return;
    }
    panInterface_ = reinterpret_cast<btpan_interface_t*>(
        const_cast<void *>(bluetoothInterface->get_profile_interface(BT_PROFILE_PAN_ID)));
}

PanServiceImpl::~PanServiceImpl()
{
}

std::vector<bluetooth::RawAddress> PanServiceImpl::GetDevicesByStates(std::vector<int> states)
{
    std::vector<RawAddress> devices;
    CHECK_AND_RETURN_LOG_RET(GetDeviceStateFunc_, devices, "GetDeviceStateFunc_ is nullptr");
    stateMachines_.Iterate([&](const std::string& key, const std::shared_ptr<PanStateMachine>& stateMachine) {
        RawAddress device(key);
        for (size_t i = 0; i < states.size(); i++) {
            if (GetDeviceStateFunc_(device) == states[i]) {
                devices.push_back(device);
                break;
            }
        }
    });
    return devices;
}

int PanServiceImpl::GetConnectState()
{
    uint32_t result = 0;
    stateMachines_.Iterate([&](const std::string& key, const std::shared_ptr<PanStateMachine>& stateMachine) {
        if (stateMachine == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (stateMachine->GetDeviceStateInt() >= PAN_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (stateMachine->GetDeviceStateInt() == PAN_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == PAN_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (stateMachine->GetDeviceStateInt() == PAN_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    });
    return static_cast<int>(result);
}

void PanServiceImpl::ProcessConnect(const PanMessage &msg)
{
    std::shared_ptr<PanStateMachine> stateMachine;
    if (!stateMachines_.Find(msg.dev_, stateMachine)) {
        stateMachine = std::make_shared<PanStateMachine>(msg.dev_);
        stateMachine->Init(this);
        stateMachines_.EnsureInsert(msg.dev_, stateMachine);
        stateMachine->SetPanRole(msg);
        stateMachine->ProcessMessage(msg);
    } else {
        stateMachine->SetPanRole(msg);
        stateMachine->ProcessMessage(msg);
    }
}

void PanServiceImpl::ProcessDisconnect(const PanMessage &msg)
{
    std::shared_ptr<PanStateMachine> stateMachine;
    if (stateMachines_.Find(msg.dev_, stateMachine) && stateMachine != nullptr) {
        if (SetDevProactiveDisconnectFlagFunc_ != nullptr) {
            SetDevProactiveDisconnectFlagFunc_(msg.dev_, PROFILE_ID_PAN, true);
        }
        stateMachine->ProcessMessage(msg);
    } else {
        HILOGE("[PAN Service]Can't find this address[%{public}s]", GetEncryptAddr(msg.dev_).c_str());
    }
}

void PanServiceImpl::ProcessConnectStateChange(const PanMessage &msg)
{
    std::shared_ptr<PanStateMachine> stateMachine;
    if (!stateMachines_.Find(msg.dev_, stateMachine) || stateMachine == nullptr) {
        stateMachine = std::make_shared<PanStateMachine>(msg.dev_);
        stateMachine->Init(this);
        stateMachines_.EnsureInsert(msg.dev_, stateMachine);
        stateMachine->SetPanRole(msg);
        stateMachine->ProcessMessage(msg);
    } else {
        stateMachine->SetPanRole(msg);
        stateMachine->ProcessMessage(msg);
    }
}

void PanServiceImpl::ProcessConnectionTimeout(const PanMessage &msg)
{
    std::shared_ptr<PanStateMachine> stateMachine;
    if (stateMachines_.Find(msg.dev_, stateMachine) && stateMachine != nullptr) {
        stateMachine->ProcessMessage(msg);
    } else {
        HILOGE("Can't find this address[%{public}s]", GetEncryptAddr(msg.dev_).c_str());
    }
}

void PanServiceImpl::RemoveStateMachine(const std::string &device)
{
    stateMachines_.Erase(device);
}

bool PanServiceImpl::IsStateMachineEmpty()
{
    return stateMachines_.IsEmpty();
}

void PanServiceImpl::ClearStatemachine()
{
    stateMachines_.Clear();
}

btpan_interface_t* PanServiceImpl::GetPanInterface()
{
    return panInterface_;
}

std::string PanServiceImpl::GetIfname()
{
    return ifname_;
}

void PanServiceImpl::DllIsTetheringOnFunc(const std::function<bool()> IsTetheringOnFunc)
{
    IsTetheringOnFunc_ = IsTetheringOnFunc;
}

void PanServiceImpl::DllNotifyStateChangedFunc(
    const std::function<void(const RawAddress &device, int state, int role)> NotifyStateChangedFunc)
{
    NotifyStateChangedFunc_ = NotifyStateChangedFunc;
}

void PanServiceImpl::DllPostEventFunc(const std::function<void(const PanMessage &event)> PostEventFunc)
{
    PostEventFunc_ = PostEventFunc;
}

void PanServiceImpl::DllGetDeviceStateFunc(const std::function<int(const RawAddress &device)> GetDeviceStateFunc)
{
    GetDeviceStateFunc_ = GetDeviceStateFunc;
}

void PanServiceImpl::DllSetDevProactiveDisconnectFlagFunc(
    const std::function<void(const std::string &addr, uint32_t profileId, bool isProactive)>
    SetDevProactiveDisconnectFlagFunc)
{
    SetDevProactiveDisconnectFlagFunc_ = SetDevProactiveDisconnectFlagFunc;
}

int PanServiceImpl::InitIfreq(struct ifreq &ifr)
{
    if (memset_s(&ifr, sizeof(ifr), 0, sizeof(ifr)) != EOK) {
        HILOGE("memset_s error.");
        return PAN_FAILURE;
    }
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ifname_.c_str(), strlen(ifname_.c_str())) != EOK) {
        HILOGE("strncpy_s error");
        return PAN_FAILURE;
    }
    return PAN_SUCCESS;
}

int PanServiceImpl::SetNetworkAddress(int inetSocket)
{
    struct ifreq ifr;
    if (InitIfreq(ifr) != PAN_SUCCESS) {
        return PAN_FAILURE;
    }
    struct in_addr ipv4Addr = {INADDR_ANY};
    if (inet_aton(PAN_NETWORK_IPV4_ADDRESS, &ipv4Addr) == 0) {
        HILOGE("inet_aton error.");
        return PAN_FAILURE;
    }
    struct in_addr mask4Addr = {INADDR_ANY};
    size_t mask = 0xffffffff;
    mask = mask << (MAX_MASK_LENGTH - PAN_NETWORK_IPV4_PREFIX_LENGTH);
    mask4Addr.s_addr = htonl(mask);

    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr);
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr = ipv4Addr;
    if (ioctl(inetSocket, SIOCSIFADDR, &ifr) < 0) {
        HILOGE("ioctl set ip address failed");
        return PAN_FAILURE;
    }
    sin->sin_addr = mask4Addr;
    if (ioctl(inetSocket, SIOCSIFNETMASK, &ifr) < 0) {
        HILOGE("ioctl set mask address failed");
        return PAN_FAILURE;
    }
    return PAN_SUCCESS;
}

int PanServiceImpl::SetInterfaceUp(int inetSocket)
{
    struct ifreq ifr;
    if (InitIfreq(ifr) != PAN_SUCCESS) {
        return PAN_FAILURE;
    }
    ifr.ifr_flags |= static_cast<short>(IFF_UP);
    ifr.ifr_flags |= static_cast<short>(IFF_MULTICAST);
    if (ioctl(inetSocket, SIOCSIFFLAGS, &ifr) < 0) {
        HILOGE("ioctl set iface up failed");
        return PAN_FAILURE;
    }
    if (ioctl(inetSocket, SIOCGIFADDR, &ifr) < 0) {
        HILOGE("ioctl get ip address failed");
        return PAN_FAILURE;
    }
    return PAN_SUCCESS;
}

int PanServiceImpl::BringUpNetwork()
{
    int inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
    CHECK_AND_RETURN_LOG_RET(inetSocket >= 0, PAN_FAILURE, "socket failed");
    if (SetNetworkAddress(inetSocket) != PAN_SUCCESS) {
        close(inetSocket);
        return PAN_FAILURE;
    }
    if (SetInterfaceUp(inetSocket) != PAN_SUCCESS) {
        close(inetSocket);
        return PAN_FAILURE;
    }
    close(inetSocket);
    return PAN_SUCCESS;
}
}  // namespace bluetooth
}  // namespace OHOS