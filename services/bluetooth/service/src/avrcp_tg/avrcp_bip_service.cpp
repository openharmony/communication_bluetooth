/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "avrcp_bip_service"
#endif

#include "common_util.h"
#include "hal_util.h"
#include "log.h"
#include "service_util.h"
#include "thread_util.h"
#include "avrcp_bip_service.h"

namespace OHOS {
namespace bluetooth {
std::shared_ptr<BipService> BipService::GetInstance()
{
    static std::shared_ptr<BipService> instance = std::make_shared<BipService>();
    return instance;
}

BipService::BipService()
{
    HILOGI("BipService Create");
}

BipService::~BipService()
{
    HILOGI("BipService Release");
    stateMachines_.Clear();
}
void BipService::SetBipClientStatus(const RawAddress &rawAddr, bool connected)
{
    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0 || btInterface == nullptr) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d.", status);
        return;
    }
    avrcpServiceInterface_ = btInterface->getAvrcpService();
    if (avrcpServiceInterface_ != nullptr) {
        avrcpServiceInterface_->SetBipClientStatus(rawAddr, connected);
    }
}

void BipService::RemoveStateMachine(const std::string &address)
{
    BipMessage event(BIP_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = address;
    PostEvent(event);
}

void BipService::HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    BipMessage event(BIP_CONNECT_EVT);
    event.dev_ = socketDevice->mDeviceAddress;
    event.socketDevice_ = socketDevice;
    HILOGI("event connect-device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    PostEvent(event);
}

void BipService::PostEvent(const BipMessage &event)
{
    HILOGI("Post event Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    DoInBipThread([this, event] {this->ProcessEvent(event);});
}

void BipService::ProcessEvent(const BipMessage &event)
{
    HILOGI("Process Event Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    switch (event.what_) {
        case BIP_CONNECT_EVT:
            ProcessConnectEvent(event);
            break;
        case BIP_DISCONNECT_EVT:
            ProcessCommonEvent(event);
            break;
        case BIP_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event);
            break;
        default:
            break;
    }
}

void BipService::ProcessConnectEvent(const BipMessage &event)
{
    HILOGI("Process Connect Event Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::shared_ptr<BipStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr && stateMachine->IsRemoving()) {
        // peer device may send connect request before we remove statemachine for last connection.
        // so post this connect request, process it after we remove statemachine completely.
        PostEvent(event);
    } else if (!ret || stateMachine == nullptr) {
        std::weak_ptr<BipService> weakBipService = weak_from_this();
        stateMachines_.EnsureInsert(event.dev_,
            std::make_shared<BipStateMachine>(event.socketDevice_, weakBipService));
        stateMachines_.ReadVal(event.dev_)->Init();
        stateMachines_.ReadVal(event.dev_)->ProcessMessage(event);
    } else {
        stateMachine->ProcessMessage(event);
    }
}

void BipService::ProcessCommonEvent(const BipMessage &event)
{
    HILOGI("Process Common Event Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::shared_ptr<BipStateMachine> stateMachine = nullptr;
    bool ret = stateMachines_.Find(event.dev_, stateMachine);
    if (ret && stateMachine != nullptr) {
        stateMachine->ProcessMessage(event);
    } else {
        HILOGE("Invalid address[%{public}s]", GET_ENCRYPT_STR_ADDR(event.dev_));
    }
}
void BipService::ProcessRemoveStateMachine(const BipMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    stateMachines_.Erase(event.dev_);
}

}  // namespace bluetooth
}  // namespace OHOS
