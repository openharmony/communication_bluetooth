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
#define LOG_TAG "bt_service_a2dp"
#endif

#include "a2dp_service_connection.h"

#include "a2dp_service.h"
#include "log.h"
#include "common_util.h"
#include "thread_util.h"
#include "interface_profile_manager.h"
#include "interface_profile_hfp_ag.h"
#include "adapter_manager.h"

constexpr const uint16_t DELAY_HANDLE_ABNORMAL_ERROR_MS = 5000;
constexpr const int32_t ERR_CODE_AVDT_ERR_TIMEOUT = 8; /* stack avdt err AVDT_ERR_TIMEOUT */

namespace OHOS {
namespace bluetooth {
A2dpConnectManager::A2dpConnectManager(uint8_t role)
{
    role_ = role;
}

bool A2dpConnectManager::A2dpConnect(const RawAddress &device)
{
    HILOGI("[address:%{public}s] role[%{public}u]", GET_ENCRYPT_ADDR(device), role_);

    A2dpService *service = GetServiceInstance(role_);
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;

    if (service == nullptr) {
        HILOGE("[A2dpConnectManager] Can't get the instance of service");
        return false;
    }

    for (auto bdr : service->GetDeviceList()) {
        if (bdr.first == device.GetAddress().c_str()) {
            info = bdr.second;
            break;
        }
    }

    if (info == nullptr) {
        info = AddDevice(device, static_cast<int>(BTConnectState::DISCONNECTED));
    }

    if (info == nullptr) {
        HILOGE("[A2dpConnectManager] role[%{public}u] Can't add new device", role_);
        return false;
    }
    RawAddress deviceObj = device;
    utility::Message msg(A2DP_MSG_CONNECT, role_, &deviceObj);

    info->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    info->GetStateMachine()->ProcessMessage(msg);

    return true;
}

bool A2dpConnectManager::A2dpDisconnect(const RawAddress &device)
{
    HILOGI("[A2dpConnectManager] device: %{public}s", GET_ENCRYPT_ADDR(device));

    A2dpService *service = GetServiceInstance(role_);

    if (service == nullptr) {
        HILOGE("[A2dpConnectManager]  Can't get the instance of service");
        return false;
    }

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(device);
    if (deviceInfo == nullptr) {
        HILOGE("[A2dpConnectManager] role[%{public}u] Not find the device", role_);
        return false;
    }
    // send message to statemachine
    utility::Message msg(A2DP_MSG_DISCONNECT, role_, &const_cast<RawAddress &>(device));

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
    deviceInfo->GetStateMachine()->ProcessMessage(msg);

    return true;
}

bool A2dpConnectManager::A2dpStreamControl(const RawAddress &device, int event)
{
    HILOGI("[A2dpConnectManager] device: %{public}s", GET_ENCRYPT_ADDR(device));

    if ((event != static_cast<int>(A2dpMessage::A2DP_MSG_PROFILE_AUDIO_PLAY_START)) &&
        (event != static_cast<int>(A2dpMessage::A2DP_MSG_PROFILE_AUDIO_PLAY_STOP))) {
        HILOGE("[A2dpConnectManager] error control event:%{public}d", event);
        return false;
    }

    A2dpService *service = GetServiceInstance(role_);
    if (service == nullptr) {
        HILOGE("[A2dpConnectManager]  Can't get the instance of service");
        return false;
    }

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(device);
    if (deviceInfo == nullptr) {
        HILOGE("[A2dpConnectManager] role[%{public}u] Not find the device", role_);
        return false;
    }

    utility::Message msg(event, role_, &const_cast<RawAddress &>(device));
    deviceInfo->GetStateMachine()->ProcessMessage(msg);
    return true;
}

bool A2dpConnectManager::JudgeConnectedNum() const
{
    A2dpService *service = GetServiceInstance(role_);

    if (service == nullptr) {
        HILOGE("[A2dpConnectManager] Can't get the instance of service");
        return false;
    }

    bool ret = false;
    int connectCnt = 0;

    if (service->GetDeviceList().empty()) {
        ret = true;
        return ret;
    } else {
        for (auto itr : service->GetDeviceList()) {
            HILOGI("[connectCnt] %{public}u", connectCnt);
            int connectionState = 0;
            connectionState = itr.second->GetConnectState();
            if (connectionState != static_cast<int>(BTConnectState::DISCONNECTED)) {
                connectCnt++;
            }
        }
    }

    HILOGI("[connectCnt] %{public}u", connectCnt);
    ret = ((connectCnt >= service->GetMaxConnectNum()) ? false : true);
    return ret;
}

std::shared_ptr<A2dpDeviceInfo> A2dpConnectManager::AddDevice(const RawAddress &device, int state)
{
    HILOGI("[A2dpConnectManager] device: %{public}s, role[%{public}u]",
        GET_ENCRYPT_ADDR(device), role_);
    A2dpService *service = GetServiceInstance(role_);
    if (service == nullptr) {
        HILOGI("[A2dpConnectManager] Can't get the instance of service");
        return nullptr;
    }

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = std::make_shared<A2dpDeviceInfo>(device);
    if (deviceInfo != nullptr) {
        deviceInfo->GetStateMachine()->SetRole(role_);
        deviceInfo->SetConnectState(state);
        service->AddDeviceToList(device.GetAddress().c_str(), deviceInfo);
    }

    return deviceInfo;
}

void A2dpConnectManager::DeleteDevice(const RawAddress &device)
{
    HILOGI("[A2dpConnectManager] device: %{public}s, role[%{public}u]", GET_ENCRYPT_ADDR(device), role_);
    A2dpService *service = GetServiceInstance(role_);
    if (service == nullptr) {
        HILOGI("[A2dpConnectManager] Can't get the instance of service");
        return;
    }
    service->DeleteDeviceFromList(device);
}

bool A2dpConnectManager::JudgeConnectExit(const RawAddress &device, uint8_t role)
{
    HILOGI("[A2dpConnectManager] device: %{public}s, role[%{public}d]", GET_ENCRYPT_ADDR(device), role);

    A2dpService *service = nullptr;

    if (role == A2DP_ROLE_SOURCE) {
        service = GetServiceInstance(A2DP_ROLE_SOURCE);
    } else {
        service = GetServiceInstance(A2DP_ROLE_SINK);
    }

    if (service != nullptr) {
        std::map<std::string, std::shared_ptr<A2dpDeviceInfo>> devList = service->GetDeviceList();
        auto iter = devList.find(device.GetAddress().c_str());
        if (iter == devList.end()) {
            HILOGI("[A2dpService]Can't find the statemachine");
            return false;
        }
        std::shared_ptr<A2dpDeviceInfo> info = iter->second;
        if ((info->GetConnectState() == static_cast<int>(BTConnectState::CONNECTED)) ||
            (info->GetConnectState() == static_cast<int>(BTConnectState::CONNECTING))) {
            HILOGE("[A2dpService]Device have been connected as source role");
            return true;
        }
    }
    return false;
}

void A2dpConnectManager::ReceiveA2dpAbnormalError(const RawAddress &device, int32_t errCode)
{
    if (errCode == ERR_CODE_AVDT_ERR_TIMEOUT) {
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_A2DP, [this, device, errCode]() { this->HandleA2dpAbnormalError(device, errCode); },
            DELAY_HANDLE_ABNORMAL_ERROR_MS, "HandleA2dpAbnormalError");
    }
}

void A2dpConnectManager::RemoveA2dpAbnormalVirtualDevice(const RawAddress &device)
{
    std::string address = device.GetAddress();
    bool ret = abnomalDevices_.IsExist(address);
    if (!ret) {
        return;
    }
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG((a2dpService != nullptr), "a2dpService null");
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    a2dpService->UpdateVirtualDevice(VIRTUAL_DEVICE_REMOVE, address);
    abnomalDevices_.Erase(address);
}

void A2dpConnectManager::HandleA2dpAbnormalError(const RawAddress &device, int32_t errCode)
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG((a2dpService != nullptr), "a2dpService null");

    int state = a2dpService->GetDeviceState(device);
    HILOGI("state:%{public}d, errCode:%{public}d", state, errCode);
    if ((state == static_cast<int>(BTConnectState::CONNECTED)) || (errCode != ERR_CODE_AVDT_ERR_TIMEOUT)) {
        return;
    }

    std::string address = device.GetAddress();
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceMgr != nullptr), "serviceMgr null");
    IProfileHfpAg *agService = static_cast<IProfileHfpAg *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG((agService != nullptr), "agService null");
    int hfpAgState = agService->GetDeviceState(device);
    bool virtualConnect = AdapterManager::GetInstance()->IsSupportVirtualAutoConnect(address);

    HILOGI("hfpAgState:%{public}d, virtualConnect:%{public}d", hfpAgState, virtualConnect);
    if ((hfpAgState != static_cast<int>(BTConnectState::CONNECTED))  || !virtualConnect) {
        return;
    }
    std::vector<std::string> virtualDevices;
    a2dpService->GetVirtualDeviceList(virtualDevices);
    if (std::count(virtualDevices.begin(), virtualDevices.end(), address) != 0) {
        return;
    }
    a2dpService->UpdateVirtualDevice(VIRTUAL_DEVICE_ADD, address);
    abnomalDevices_.Push(address);
}
}  // namespace bluetooth
}  // namespace OHOS