/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_pbap_service_impl"
#endif

#include "pbap_pce_service_impl.h"
#include "datetime_ex.h"
#include "adapter_config.h"
#include "class_creator.h"
#include "interface_adapter.h"
#include "log.h"
#include "pbap_pce_base_state.h"
#include "pbap_pce_state_machine.h"
#include "profile_config.h"
#include "pbap_pce_header_msg.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
extern "C" PbapPceServiceImplInterface *CreatePbapPceServiceImplInterface(void)
{
    HILOGI("enter");
    return new PbapPceServiceImpl();
}

extern "C" void DestroyPbapPceServiceImplInterface(PbapPceServiceImplInterface *interface)
{
    HILOGI("enter");
    if (interface == nullptr) {
        HILOGE("pbap pce manager Interface is nullptr");
        return;
    }
    delete interface;
}

PbapPceServiceImpl::PbapPceServiceImpl()
{
    HILOGI("PbapPceServiceImpl Create");
    pbapPceSdp_ = std::make_unique<PbapPceSdp>(
        [this](const std::string &addr, const utility::Message &msg) {
            OnSdpResponse(addr, msg);
        });
}

PbapPceServiceImpl::~PbapPceServiceImpl()
{
    HILOGI("PbapPceServiceImpl Release");
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    machineMap_.clear();
}

void PbapPceServiceImpl::SetTryShutDownCallback(const std::function<void(bool)> &callback)
{
    tryShutDownCallback_ = callback;
}

void PbapPceServiceImpl::RegisterObserver(std::shared_ptr<IPbapPceObserver> &observer)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    pceConnectionObserver_ = observer;
}

void PbapPceServiceImpl::DeregisterObserver(std::shared_ptr<IPbapPceObserver> &observer)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    pceConnectionObserver_ = nullptr;
}

void PbapPceServiceImpl::PostConnectEvent(const PbapPceMessage &event)
{
    DoInPbapThread(std::bind(&PbapPceServiceImpl::ProcessConnectEvent, this, event));
}

void PbapPceServiceImpl::ProcessConnectEvent(const PbapPceMessage &event)
{
    switch (event.what_) {
        case PBAP_PCE_CONNECT_EVT:
            ConnectInternal(event.dev_);
            break;
        case PBAP_PCE_DISCONNECT_EVT:
            DisconnectInternal(event.dev_);
            break;
        default:
            break;
    }
}

int32_t PbapPceServiceImpl::Connect(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    RawAddress rDevice(device.GetAddress());
    BTStrategyType strate = BTStrategyType(GetConnectionStrategy(rDevice));
    // if (strate == BTStrategyType::CONNECTION_FORBIDDEN) {
    //     PBAP_PCE_LOG_ERROR("%{public}s end, connect forbidden.", __PRETTY_FUNCTION__);
    //     return RET_BAD_STATUS;
    // }

    int state = GetDeviceState(device);
    PBAP_PCE_LOG_INFO("%{public}s GetDeviceState=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__, state,
        GetEncryptAddr(device.GetAddress()).c_str());
    if ((state == static_cast<int>(BTConnectState::CONNECTING)) ||
        (state == static_cast<int>(BTConnectState::CONNECTED)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTING))) {
        PBAP_PCE_LOG_ERROR("%{public}s end, bad status. state=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__, state,
            GetEncryptAddr(device.GetAddress()).c_str());
        return RET_BAD_STATUS;
    }

    if (GetConnectedDevices().size() == (std::size_t)GetMaxConnectNum()) {
        PBAP_PCE_LOG_ERROR("%{public}s end, maximum devices is %{public}d.", __PRETTY_FUNCTION__, GetMaxConnectNum());
        return RET_NO_SUPPORT;
    }

    if (IsBusy(device)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, Is busy! addr=%{public}s", __PRETTY_FUNCTION__,
            GetEncryptAddr(device.GetAddress()).c_str());
        return RET_BAD_STATUS;
    }

    PbapPceMessage event(PBAP_PCE_CONNECT_EVT);
    event.dev_ = device;
    PostConnectEvent(event);
    PBAP_PCE_LOG_INFO("%{public}s end, posted connect event, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    return BT_NO_ERROR;
}

int PbapPceServiceImpl::ConnectInternal(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    if (serviceState_ != PBAP_PCE_STATE_STARTUP) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pbap pce service isn't started.", __PRETTY_FUNCTION__);
        return RET_NO_SUPPORT;
    }

    if (GetConnectedDevices().size() == (std::size_t)GetMaxConnectNum()) {
        PBAP_PCE_LOG_ERROR("%{public}s end, maximum devices is %{public}d.",  __PRETTY_FUNCTION__, GetMaxConnectNum());
        return RET_NO_SUPPORT;
    }

    std::string addr = device.GetAddress();
    HILOGI("Connect to rawaddr: %{public}s", GetEncryptAddr(addr).c_str());
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    if (machineMap_.find(addr) == machineMap_.end()) {
        PBAP_PCE_LOG_INFO("%{public}s creating NEW state machine, addr=%{public}s, mapSize=%{public}zu",
            __PRETTY_FUNCTION__, GetEncryptAddr(addr).c_str(), machineMap_.size());
        machineMap_[addr] = std::make_unique<PbapPceStateMachine>(device, *this, *pbapPceSdp_, pceConnectionObserver_);
        PBAP_PCE_LOG_INFO("%{public}s NEW state machine created, addr=%{public}s", __PRETTY_FUNCTION__,
            GetEncryptAddr(addr).c_str());
    } else {
        int state = GetDeviceState(device);
        PBAP_PCE_LOG_INFO("%{public}s existing sm found, state=%{public}d, addr=%{public}s", __PRETTY_FUNCTION__,
            state, GetEncryptAddr(addr).c_str());
        if ((state == static_cast<int>(BTConnectState::DISCONNECTING)) ||
            (state == static_cast<int>(BTConnectState::DISCONNECTED))) {
            PBAP_PCE_LOG_INFO("start connect for disconnected device.");
            machineMap_[addr]->InitState(PCE_CONNECTING_STATE);
        } else {
            PBAP_PCE_LOG_ERROR("%{public}s end, already connecting or for connected device.", __PRETTY_FUNCTION__);
            return RET_BAD_STATUS;
        }
    }
    PBAP_PCE_LOG_INFO("%{public}s end, addr=%{public}s", __PRETTY_FUNCTION__, GetEncryptAddr(addr).c_str());
    return BT_NO_ERROR;
}

bool PbapPceServiceImpl::IsBusy(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);

    bool ret = false;
    std::string strAddr = device.GetAddress();
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        ret = sm->IsBusy();
    }

    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return ret;
}

int32_t PbapPceServiceImpl::GetConnectionStrategy(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    int strategy = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    if (!LoadConnectPolicy(device.GetAddress(), strategy)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, load strategy value failed.", __PRETTY_FUNCTION__);
        return strategy;
    }
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED) ||
        strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
        return strategy;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return strategy;
}

bool PbapPceServiceImpl::LoadConnectPolicy(const std::string &addr, int &strategy)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (!config->GetValue(addr, SECTION_CONNECTION_POLICIES, PROPERTY_PBAP_CLIENT_CONNECTION_POLICY, strategy)) {
        HILOGE("addr: %{public}s %{public}s load fail.",
            GetEncryptAddr(addr).c_str(), PROPERTY_PBAP_CLIENT_CONNECTION_POLICY);
        return false;
    }
    return true;
}

int32_t PbapPceServiceImpl::GetDeviceState(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(device.GetAddress()).c_str());
    BTConnectState result = BTConnectState::DISCONNECTED;
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    auto target = machineMap_.find(device.GetAddress());
    if (target != machineMap_.end()) {
        std::string name = target->second->GetState()->Name();
        PBAP_PCE_LOG_INFO("%{public}s found sm, stateName=%{public}s, addr=%{public}s", __PRETTY_FUNCTION__,
            name.c_str(), GetEncryptAddr(device.GetAddress()).c_str());
        if (name == PCE_DISCONNECTED_STATE) {
            result = BTConnectState::DISCONNECTED;
        } else if (name == PCE_CONNECTING_STATE) {
            result = BTConnectState::CONNECTING;
        } else if (name == PCE_CONNECTED_STATE) {
            result = BTConnectState::CONNECTED;
        } else if (name == PCE_DISCONNECTING_STATE) {
            result = BTConnectState::DISCONNECTING;
        }
    } else {
        PBAP_PCE_LOG_INFO("%{public}s device NOT in machineMap_, returning DISCONNECTED, addr=%{public}s",
            __PRETTY_FUNCTION__, GetEncryptAddr(device.GetAddress()).c_str());
    }
    PBAP_PCE_LOG_INFO("%{public}s end, result=%{public}d", __PRETTY_FUNCTION__, static_cast<int>(result));
    return static_cast<int>(result);
}

std::vector<RawAddress> PbapPceServiceImpl::GetConnectedDevices()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    std::vector<RawAddress> devices;
    std::vector<int> states {static_cast<int>(BTConnectState::CONNECTED)};
    auto allDevices = GetDevicesByStates(states);
    for (auto &dev : allDevices) {
        devices.push_back(dev);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return devices;
}

std::vector<RawAddress> PbapPceServiceImpl::GetDevicesByStates(const std::vector<int32_t> &states)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    std::vector<RawAddress> devices;
    for (auto &state : states) {
        for (auto &it : machineMap_) {
            if (static_cast<int>(GetDeviceState(RawAddress(it.first))) == state) {
                auto sm = static_cast<PbapPceStateMachine *>(it.second.get());
                devices.push_back(sm->GetDevice());
            }
        }
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return devices;
}

int32_t PbapPceServiceImpl::GetConnectState()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    uint8_t result = 0;
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    for (auto &sm : machineMap_) {
        std::string name = sm.second->GetState()->Name();
        if (name == PCE_DISCONNECTED_STATE) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (name == PCE_CONNECTING_STATE) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (name == PCE_CONNECTED_STATE) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (name == PCE_DISCONNECTING_STATE) {
            result |= PROFILE_STATE_DISCONNECTING;
        }
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return result;
}

bool PbapPceServiceImpl::IsDownloading(const RawAddress &device)
{
    return IsBusy(device);
}

int32_t PbapPceServiceImpl::GetPhoneBookSyncState(const RawAddress &device)
{
    return 0;
}

int PbapPceServiceImpl::GetMaxConnectNum()
{
    return pbapPceConfig_.pceMaxDevices_;
}

const PbapPceConfig &PbapPceServiceImpl::GetPceConfig() const
{
    return pbapPceConfig_;
}

int32_t PbapPceServiceImpl::Disconnect(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    int state = GetDeviceState(device);
    if ((state == static_cast<int>(BTConnectState::CONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTING)) ||
        (state == static_cast<int>(BTConnectState::DISCONNECTED))) {
        PBAP_PCE_LOG_ERROR("%{public}s end, bad status.", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }

    PbapPceMessage event(PBAP_PCE_DISCONNECT_EVT);
    event.dev_ = device;
    PostConnectEvent(event);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

int PbapPceServiceImpl::DisconnectInternal(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    utility::Message msg(PCE_REQ_DISCONNECTED);
    ForwardMsgTosm(device, msg);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

void PbapPceServiceImpl::ForwardMsgTosm(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d, addr=%{public}s, arg2=%{public}p",
        __PRETTY_FUNCTION__, msg.what_, GetEncryptAddr(device.GetAddress()).c_str(), msg.arg2_);
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    auto it = machineMap_.find(device.GetAddress());
    if (it == machineMap_.end()) {
        PBAP_PCE_LOG_ERROR("%{public}s device NOT in machineMap_, msg.what=%{public}d, addr=%{public}s, "
            "mapSize=%{public}zu", __PRETTY_FUNCTION__, msg.what_,
            GetEncryptAddr(device.GetAddress()).c_str(), machineMap_.size());
        for (auto &entry : machineMap_) {
            PBAP_PCE_LOG_ERROR("%{public}s machineMap_ key=%{public}s", __PRETTY_FUNCTION__,
                GetEncryptAddr(entry.first).c_str());
        }
        PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d (not found)", __PRETTY_FUNCTION__, msg.what_);
        return;
    }
    PBAP_PCE_LOG_INFO("%{public}s found sm, calling ProcessMessage, msg.what=%{public}d", __PRETTY_FUNCTION__,
        msg.what_);
    bool ret = it->second->ProcessMessage(msg);
    PBAP_PCE_LOG_INFO("%{public}s ProcessMessage ret=%{public}d, msg.what=%{public}d", __PRETTY_FUNCTION__,
        ret, msg.what_);
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

int32_t PbapPceServiceImpl::SetConnectionStrategy(const RawAddress &device, int strategy)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    switch (strategy) {
        case static_cast<int>(BTStrategyType::CONNECTION_ALLOWED):
        case static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN):
            if (!SaveConnectPolicy(device.GetAddress(), strategy)) {
                PBAP_PCE_LOG_ERROR("%{public}s end, save strategy value failed.", __PRETTY_FUNCTION__);
                return RET_NO_SUPPORT;
            }
            break;
        default:
            PBAP_PCE_LOG_ERROR("%{public}s end, save strategy invalid value.", __PRETTY_FUNCTION__);
            return RET_BAD_PARAM;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (IsConnected(device)) {
            Disconnect(device);
        } else if (GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTING)) {
            utility::Message msg(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_DISCONNECTED);
            ForwardMsgTosm(device, msg);
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if (GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
        } else if (GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTING)) {
            utility::Message msg(PCE_REQ_SET_TARGET_STATE, PCE_TARGET_STATE_CONNECTED);
            ForwardMsgTosm(device, msg);
        }
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

bool PbapPceServiceImpl::SaveConnectPolicy(const std::string &addr, int strategy)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (!config->SetValue(addr, SECTION_CONNECTION_POLICIES, PROPERTY_PBAP_CLIENT_CONNECTION_POLICY, strategy)) {
        HILOGE("addr: %{public}s %{public}s save fail.",
            GetEncryptAddr(addr).c_str(), PROPERTY_PBAP_CLIENT_CONNECTION_POLICY);
        return false;
    }
    return true;
}

bool PbapPceServiceImpl::IsConnected(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    if (GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTED)) {
        return true;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return false;
}

bool PbapPceServiceImpl::SetBusy(const RawAddress &device, bool busy)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    bool ret = false;
    std::string strAddr = device.GetAddress();
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        ret = sm->SetBusy(busy);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return ret;
}

void PbapPceServiceImpl::OnObexResponse(const RawAddress &device, const utility::Message &msg)
{
    DoInPbapThread([this, device, msg]() {
        ProcessObexRespMessage(device, msg);
    });
}

void PbapPceServiceImpl::ProcessObexRespMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    SetPowerStatusBusy(device, false);
    switch (msg.what_) {
        case PCE_OBEX_CONNECTED:
            ProcessObexConnected(device, msg);
            break;
        case PCE_OBEX_DISCONNECTED:
        case PCE_OBEX_CONNECT_FAILED:
        case PCE_TRANSPORT_FAILED:
            ProcessObexDisconnected(device, msg);
            break;
        case PCE_DOWNLOAD_COMPLETE:
            PBAP_PCE_LOG_INFO("%{public}s PCE_DOWNLOAD_COMPLETE, forwarding to sm", __PRETTY_FUNCTION__);
            ForwardMsgTosm(device, msg);
            break;
        default:
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::ProcessObexConnected(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    ForwardMsgTosm(device, msg);
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::ProcessObexDisconnected(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    ForwardMsgTosm(device, msg);
    if (tryShutDownCallback_) {
        tryShutDownCallback_(true);
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::ProcessPhonebookMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    SetPowerStatusBusy(device, false);
    switch (msg.what_) {
        case PCE_PULLPHONEBOOKSIZE_COMPLETED:
        case PCE_PULLPHONEBOOK_COMPLETED:
        case PCE_ABORTDOWNLOADING_COMPLETED:
            ProcessPhoneBookActionCompleted(device, msg);
            break;
        default:
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::ProcessPhoneBookActionCompleted(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
    ForwardMsgTosm(device, msg);
    utility::Message msg2(PCE_REQ_TRANSIT_TARGET_STATE);
    ForwardMsgTosm(device, msg2);
    if (tryShutDownCallback_) {
        tryShutDownCallback_(true);
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::SetPowerStatusBusy(const RawAddress &device, bool busy)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    std::string strAddr = device.GetAddress();
    std::lock_guard<std::recursive_mutex> lock(machineMapMutex_);
    if (machineMap_.find(strAddr) != machineMap_.end()) {
        auto sm = static_cast<PbapPceStateMachine *>(machineMap_[strAddr].get());
        sm->SetPowerStatusBusy(busy);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceServiceImpl::OnSdpResponse(const std::string &addr, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d, addr=%{public}s, arg2=%{public}p",
        __PRETTY_FUNCTION__, msg.what_, GetEncryptAddr(addr).c_str(), msg.arg2_);
    RawAddress device(addr);
    DoInPbapThread([this, device, msg]() {
        ProcessSdpMessage(device, msg);
    });
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::ProcessSdpMessage(const RawAddress &device, const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what=%{public}d, addr=%{public}s, arg2=%{public}p",
        __PRETTY_FUNCTION__, msg.what_, GetEncryptAddr(device.GetAddress()).c_str(), msg.arg2_);
    switch (msg.what_) {
        case PCE_SDP_FAILED:
            PBAP_PCE_LOG_INFO("%{public}s PCE_SDP_FAILED, forwarding", __PRETTY_FUNCTION__);
            ForwardMsgTosm(device, msg);
            break;
        case PCE_SDP_FINISH:
            PBAP_PCE_LOG_INFO("%{public}s PCE_SDP_FINISH, forwarding, arg2=%{public}p",
                __PRETTY_FUNCTION__, msg.arg2_);
            ForwardMsgTosm(device, msg);
            break;
        default:
            PBAP_PCE_LOG_ERROR("%{public}s unknown msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what=%{public}d", __PRETTY_FUNCTION__, msg.what_);
}

void PbapPceServiceImpl::LoadPceConfig()
{
    IAdapterConfig *adpterConfig = AdapterConfig::GetInstance();
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_SRM_ENABLE, pbapPceConfig_.srmEnable_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_SRM_ENABLE.c_str());
    }
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_RFCOMM_MTU, pbapPceConfig_.rfcommMtu_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_RFCOMM_MTU.c_str());
    }
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_L2CAP_MTU, pbapPceConfig_.l2capMtu_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_L2CAP_MTU.c_str());
    }
    if (!adpterConfig->GetValue(SECTION_PBAP_PCE_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, pbapPceConfig_.pceMaxDevices_)) {
        PBAP_PCE_LOG_ERROR("Load config %{public}s failure", PROPERTY_MAX_CONNECTED_DEVICES.c_str());
    }
    PBAP_PCE_LOG_INFO("srmEnable = %{public}d, rfcommMtu=0x%X, l2capMtu_=0x%X, maxDevices=%{public}d",
        pbapPceConfig_.srmEnable_,
        pbapPceConfig_.rfcommMtu_,
        pbapPceConfig_.l2capMtu_,
        pbapPceConfig_.pceMaxDevices_);
}

void PbapPceServiceImpl::Enable()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    serviceState_ = PBAP_PCE_STATE_STARTUPING;
    LoadPceConfig();
    int retVal = pbapPceSdp_->Register();
    if (retVal != Bluetooth::BT_NO_ERROR) {
        PBAP_PCE_LOG_ERROR("end, Call pbapPceSdp_->Register() Error");
        serviceState_ = PBAP_PCE_STATE_STARTUP;
        return;
    }
    serviceState_ = PBAP_PCE_STATE_STARTUP;
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceServiceImpl::Disable()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    serviceState_ = PBAP_PCE_STATE_SHUTDOWNING;

    int retVal = 0;
    if (pbapPceSdp_) {
        retVal = pbapPceSdp_->Deregister();
        if (retVal != Bluetooth::BT_NO_ERROR) {
            PBAP_PCE_LOG_ERROR("pbapPceSdp_->Deregister() error");
        }
    }

    std::vector<int32_t> states {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED),
        static_cast<int32_t>(BTConnectState::DISCONNECTING)
    };
    auto devices = GetDevicesByStates(states);
    for (auto &device : devices) {
        Disconnect(device);
    }

    serviceState_ = PBAP_PCE_STATE_SHUTDOWN;
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

}  // namespace bluetooth
}  // namespace OHOS
