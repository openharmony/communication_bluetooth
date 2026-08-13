/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "pbap_pse_service_impl.h"
#include "datetime_ex.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
extern "C" PbapPseServiceImplInterface *CreatePbapServiceImplInterface(void)
{
    return new PbapPseServiceImpl();
}

extern "C" void DestroyPbapServiceImplInterface(PbapPseServiceImplInterface *interface)
{
    if (interface == nullptr) {
        HILOGE("pbap pse manager Interface is nullptr");
        return;
    }
    delete interface;
}

PbapPseServiceImpl::~PbapPseServiceImpl()
{
    HILOGI("PbapPseServiceImpl Release");
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    stateMachines_.clear();
}

void PbapPseServiceImpl::RegisterObserver(std::shared_ptr<IPbapPseObserver> &observer)
{
    HILOGI("RegisterObserver");
    pbapPseConnectionObserver_ = observer;
}

void PbapPseServiceImpl::DeregisterObserver(std::shared_ptr<IPbapPseObserver> &observer)
{
    HILOGI("DeregisterObserver");
    pbapPseConnectionObserver_ = nullptr;
}

int32_t PbapPseServiceImpl::Disconnect(const RawAddress &device)
{
    std::string address = device.GetAddress();
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGW("[PBAP_SERVICE]The state machine is not available!");
            return BT_ERR_INTERNAL_ERROR;
        }

        int32_t state = it->second->GetDeviceStateInt();
        if (state == PBAP_PSE_STATE_DISCONNECTED) {
            HILOGW("device not connected, state:%{public}d", state);
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    PbapPseMessage event(PBAP_PSE_DISCONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    return BT_NO_ERROR;
}

std::vector<RawAddress> PbapPseServiceImpl::GetDevicesByStates(const std::vector<int32_t> &states)
{
    std::vector<RawAddress> devices;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second == nullptr) {
            continue;
        }
        RawAddress device(it->first);
        int tmpState = it->second->GetDeviceStateInt();
        for (size_t i = 0; i < states.size(); i++) {
            if (tmpState == states[i]) {
                devices.push_back(device);
                break;
            }
        }
    }
    return devices;
}

int32_t PbapPseServiceImpl::GetDeviceState(const RawAddress &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    std::string address = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGE("GetDeviceState:The state machine is not available!");
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    int state = it->second->GetDeviceStateInt();
    if (state > PBAP_PSE_STATE_CONNECTED) {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    return stateMap_.at(state);
}

int32_t PbapPseServiceImpl::GetConnectState(void)
{
    int32_t result = 0;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
            continue;
        }
        int32_t state = it->second->GetDeviceStateInt();
        if (state == PBAP_PSE_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else { // 除了连接状态，其他状态都归为断连
            result |= PROFILE_STATE_DISCONNECTED;
        }
    }
    HILOGI("Result = %{public}d", result);
    return result;
}

void PbapPseServiceImpl::PostEvent(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    DoInPbapThread(std::bind(&PbapPseServiceImpl::ProcessEvent, this, event));
}

void PbapPseServiceImpl::ProcessEvent(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    switch (event.what_) {
        case PBAP_PSE_CONNECT_EVT:
            ProcessConnectEvent(event);
            break;
        case PBAP_PSE_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event);
            break;
        case PBAP_PSE_REQUEST_PERMISSION_EVT:
            CheckOrGetPermission(event.dev_, event.timer_);
            break;
        case PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT:
            ProcessReqPermissionTimeOut(event);
            break;
        case PBAP_PSE_VERIFY_RESULT_EVT:
            ProcessPermissionResultEvent(event);
            break;
        case PBAP_PSE_DISCONNECT_EVT:
            ProcessCommmonEvent(event);
            break;
        case PBAP_PSE_STATE_CHANGED:
            NotifyStateChanged(RawAddress(event.dev_), event.state_);
            break;
        default:
            break;
    }
}

void PbapPseServiceImpl::NotifyStateChanged(const RawAddress &device, int state)
{
    HILOGI("NotifyStateChanged ConnectState:%{public}d", state);
    int convertState;
    if (state > PBAP_PSE_STATE_CONNECTED) {
        convertState = static_cast<int>(BTConnectState::DISCONNECTED);
    } else {
        convertState = stateMap_.at(state);
    }
    if (pbapPseConnectionObserver_ != nullptr) {
        pbapPseConnectionObserver_->OnConnectionStateChanged(device, convertState,
            static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
    }
}

void PbapPseServiceImpl::SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization)
{
    if (accessAuthorization == static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)) {
        int32_t rejectCount = 0;
        bool getResult = GetPbapRejectCount(device.GetAddress(), rejectCount);
        if (getResult) {
            rejectCount++;
        } else {
            rejectCount = 1;
        }
        HILOGI("device: %{public}s, rejectCount: %{public}d", GET_ENCRYPT_ADDR(device), rejectCount);
        bool setResult = SetPbapRejectCount(device.GetAddress(), rejectCount);
        if (!setResult) {
            HILOGE("SetPbapRejectCount fail");
        }

        if (permissionRequestTimeMap_ != nullptr) {
            permissionRequestTimeMap_->EnsureInsert(device.GetAddress(), GetSecondsSince1970ToNow());
        }
    } else if (accessAuthorization == static_cast<int>(BTPermissionType::ACCESS_ALLOWED)) {
        int32_t rejectCount = 0;
        bool setResult = SetPbapRejectCount(device.GetAddress(), rejectCount);
        HILOGI("device: %{public}s allowed, rejectCount reset result %{public}d", GET_ENCRYPT_ADDR(device), setResult);
    }
    bool isAccess = (accessAuthorization == static_cast<int>(BTPermissionType::ACCESS_ALLOWED));
    VerifyPermissionResult(device.GetAddress(), isAccess);
}

void PbapPseServiceImpl::VerifyPermissionResult(const std::string &address, bool isAccess)
{
    HILOGI("VerifyPermissionResult");
    PbapPseMessage event(PBAP_PSE_VERIFY_RESULT_EVT);
    event.dev_ = address;
    event.isAccess_ = isAccess;
    PostEvent(event);
    if (isAccess && permissionRequestTimeMap_ != nullptr) {
        permissionRequestTimeMap_->Erase(address);
    }
}

void PbapPseServiceImpl::HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    PbapPseMessage event(PBAP_PSE_CONNECT_EVT);
    event.dev_ = socketDevice->mDeviceAddress;
    event.socketDevice_ = socketDevice;
    HILOGI("device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    PostEvent(event);
}

void PbapPseServiceImpl::ProcessReqPermissionTimeOut(const PbapPseMessage &event)
{
    PbapPseMessage newEvent(PBAP_PSE_DISCONNECT_EVT);
    newEvent.dev_ = event.dev_;
    PostEvent(newEvent);

    // dismiss pbap dialog
    if (dialogFunc_ != nullptr) {
        dialogFunc_("", nullptr, false);
    }
}

void PbapPseServiceImpl::DllRegisterFunc(const std::function<bool(const std::string&, std::shared_ptr<utility::Timer>,
    bool)> &dialogFunc, std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap)
{
    dialogFunc_ = dialogFunc;
    permissionRequestTimeMap_ = permissionRequestTimeMap;
}


void PbapPseServiceImpl::RemoveStateMachine(const std::string &device)
{
    PbapPseMessage event(PBAP_PSE_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = device;
    PostEvent(event);
}

void PbapPseServiceImpl::CheckOrGetPermission(const std::string &address, std::shared_ptr<utility::Timer> timer)
{
    int dataValue = 0;
    bool isAccess = false;
    GetPbapPermission(address, dataValue);
    HILOGI("address: %{public}s, permission: %{public}d", GET_ENCRYPT_STR_ADDR(address), dataValue);
    if (IsNeedSetPermissionToUnknown(address, dataValue)) {
        dataValue = static_cast<int>(BTPermissionType::ACCESS_UNKNOWN);
        HILOGI("permission fix to %{public}d", dataValue);
    }

    if (dataValue == static_cast<int>(BTPermissionType::ACCESS_ALLOWED)) {
        isAccess = true;
    } else if (dataValue == static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)) {
        isAccess = false;
    } else if (dataValue == static_cast<int>(BTPermissionType::ACCESS_UNKNOWN)) {
        int64_t currentTime = GetSecondsSince1970ToNow();
        int64_t lastTime = 0;
        if (permissionRequestTimeMap_ != nullptr && permissionRequestTimeMap_->Find(address, lastTime)) {
            int64_t timeDiff = currentTime - lastTime;
            if (timeDiff <= PBAP_PERMISSION_REQUEST_MIN_TIME_DIFF) {
                HILOGI("request interval time is too short, address: %{public}s, timeDiff: %{public}d",
                    GET_ENCRYPT_STR_ADDR(address), timeDiff);
                VerifyPermissionResult(address, false);
                permissionRequestTimeMap_->EnsureInsert(address, currentTime);
                return;
            }
        }

        if (dialogFunc_ != nullptr && dialogFunc_(address, timer, true) && permissionRequestTimeMap_ != nullptr) {
            permissionRequestTimeMap_->EnsureInsert(address, currentTime);
            return;
        }
        isAccess = false;
    } else {
        HILOGE("Get Permission is error, device: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    }

    VerifyPermissionResult(address, isAccess);
}

void PbapPseServiceImpl::ProcessConnectEvent(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if (it != stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
        // peer device may send connect request before we remove statemachine for last connection.
        // so post this connect request, process it after we remove statemachine completely.
        PostEvent(event);
    } else if (it == stateMachines_.end() || it->second == nullptr) {
        stateMachines_[event.dev_] = std::make_unique<PbapPseStateMachine>(event.socketDevice_, this);
        stateMachines_[event.dev_]->Init();
        stateMachines_[event.dev_]->ProcessMessage(event);
    } else {
        it->second->ProcessMessage(event);
    }
}

void PbapPseServiceImpl::ProcessRemoveStateMachine(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    stateMachines_.erase(event.dev_);
}

void PbapPseServiceImpl::ProcessCommmonEvent(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if ((it != stateMachines_.end()) && (it->second != nullptr)) {
        it->second->ProcessMessage(event);
    } else {
        HILOGE("Invalid address[%{public}s]", GET_ENCRYPT_STR_ADDR(event.dev_));
    }
}

void PbapPseServiceImpl::ProcessPermissionResultEvent(const PbapPseMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if ((it != stateMachines_.end()) && (it->second != nullptr)) {
        if (!it->second->GetRequestPermissionFlag()) {
            HILOGI("PermissionFlag error, Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
            return;
        }
        it->second->SetRequestPermissionFlag(false);
        it->second->ProcessMessage(event);
    } else {
        HILOGE("Invalid address[%{public}s]", GET_ENCRYPT_STR_ADDR(event.dev_));
    }
}

bool PbapPseServiceImpl::IsNeedSetPermissionToUnknown(const std::string &address, int permission)
{
    if (permission != static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)) {
        return false;
    }
    int32_t rejectCount = 0;
    bool getResult = GetPbapRejectCount(address, rejectCount);
    HILOGI("rejectCount %{public}d  getResult %{public}d", rejectCount, getResult);
    if (!getResult) {
        return true;
    }
    return rejectCount < PBAP_REJECT_MAX_TIMES;
}
}  // namespace bluetooth
}  // namespace OHOS
