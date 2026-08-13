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
#define LOG_TAG "bt_service_pbap_service"
#endif
#include "pbap_pce_service.h"
#include "class_creator.h"
#include "log.h"
#include "remote_device_properties.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "bt_def.h"
#include "pbap_pce_def.h"

namespace OHOS {
namespace bluetooth {

PbapPceService::PbapPceService() : utility::Context(PROFILE_NAME_PBAP_PCE, "1.2.3"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{
    HILOGI("PbapPceService Create");
    PBAP_PCE_LOG_INFO("ProfileService:%{public}s Create", Name().c_str());
    connectionObserver_ = std::make_shared<PbapPceObserver>();
}

PbapPceService::~PbapPceService()
{}

utility::Context *PbapPceService::GetContext()
{
    return this;
}

void PbapPceService::RegisterObserver(IPbapPceObserver &observer)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    pbapPceObservers_.Register(observer);
}

void PbapPceService::DeregisterObserver(IPbapPceObserver &observer)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    pbapPceObservers_.Deregister(observer);
}

void PbapPceObserver::OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause)
{
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_PBAP_PCE, false);
    }
    cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device, PROFILE_ID_PBAP_PCE, state);
    PbapPceService *pbapPceService = PbapPceService::GetService();
    if (pbapPceService == nullptr) {
        HILOGE("pbapPceService is null");
        return;
    }
    PbapPcePhoneBookStateType pbState = PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED;
    switch (state) {
        case static_cast<int>(BTConnectState::DISCONNECTED):
            pbState = PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED;
            break;
        case static_cast<int>(BTConnectState::CONNECTING):
            pbState = PBAP_PCE_PHONEBOOK_STATE_CONNECTING;
            break;
        case static_cast<int>(BTConnectState::CONNECTED):
            pbState = PBAP_PCE_PHONEBOOK_STATE_CONNECTED;
            break;
        case static_cast<int>(BTConnectState::DISCONNECTING):
            pbState = PBAP_PCE_PHONEBOOK_STATE_DISCONNECTING;
            break;
        default:
            break;
    }
    pbapPceService->SetPhoneBookSyncState(device.GetAddress(), pbState);
    pbapPceService->NotifyStateChanged(device, state, cause);
}

void PbapPceObserver::OnActionCompleted(const RawAddress &device, int respCode, int actionType)
{
    PbapPceService *pbapPceService = PbapPceService::GetService();
    if (pbapPceService == nullptr) {
        HILOGE("pbapPceService is null");
        return;
    }
    if (actionType == PBAP_ACTION_PULLPHONEBOOK) {
        pbapPceService->SetPhoneBookSyncState(device.GetAddress(), PBAP_PCE_PHONEBOOK_STATE_DOWNLOADED);
    } else if (actionType == PBAP_ACTION_PULLPHONEBOOKSIZE) {
        pbapPceService->SetPhoneBookSyncState(device.GetAddress(), PBAP_PCE_PHONEBOOK_STATE_DOWNLOADING);
    }
    pbapPceService->NotifyActionCompleted(device, respCode, actionType);
}

PbapPceService *PbapPceService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<PbapPceService *>(servManager->GetProfileService(PROFILE_NAME_PBAP_PCE));
}

void PbapPceService::NotifyStateChanged(const RawAddress &device, int state, int cause)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("NotifyStateChanged ConnectState:%{public}d, cause:%{public}d", state, cause);
    pbapPceObservers_.ForEach([device, state, cause](IPbapPceObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
}

void PbapPceService::NotifyActionCompleted(const RawAddress &device, int respCode, int actionType)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("NotifyActionCompleted respCode:%{public}d, actionType:%{public}d", respCode, actionType);
    pbapPceObservers_.ForEach([device, respCode, actionType](IPbapPceObserver &observer) {
        observer.OnActionCompleted(device, respCode, actionType);
    });
}

void PbapPceService::SetPhoneBookSyncState(const std::string &addr, PbapPcePhoneBookStateType state)
{
    phoneBookStateMap_[addr] = state;
    if (state == PbapPcePhoneBookStateType::PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR
        || state == PbapPcePhoneBookStateType::PBAP_PCE_PHONEBOOK_STATE_DOWNLOADING
        || state == PbapPcePhoneBookStateType::PBAP_PCE_PHONEBOOK_STATE_DOWNLOADED) {
        pbapPceObservers_.ForEach([addr, state = static_cast<int>(state)](IPbapPceObserver &observer) {
            observer.OnActionCompleted(RawAddress(addr), 0, state);
        });
    }
}

int32_t PbapPceService::GetPhoneBookSyncState(const RawAddress &device)
{
    auto it = phoneBookStateMap_.find(device.GetAddress());
    if (it != phoneBookStateMap_.end()) {
        return static_cast<int32_t>(it->second);
    }
    return static_cast<int32_t>(PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED);
}

int PbapPceService::Connect(const RawAddress &device)
{
    HILOGI("PbapPceService::Connect start");
    // [临时调试] 方案A：移除 HFP-HF 前置依赖检查
    // 原因：ConnectClassicProfiles 并发发起 HFP-HF 和 PBAP PCE 连接，
    //       PBAP PCE 检查时 HFP-HF 尚未连接，导致返回 BT_ERR_INTERNAL_ERROR，
    //       且 HFP-HF 连接成功后无重试机制，PBAP PCE 始终无法连接。
    // 注意：此为调试用临时修改，需评估是否存在依赖 HFP-HF 链路的业务场景。
    // auto hfpHfService = IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_HF);
    // if (hfpHfService != nullptr) {
    //     auto connectedDevices = hfpHfService->GetConnectDevices();
    //     bool hfpConnected = false;
    //     for (auto &dev : connectedDevices) {
    //         if (dev.GetAddress() == device.GetAddress()) {
    //             hfpConnected = true;
    //             break;
    //         }
    //     }
    //     if (!hfpConnected) {
    //         HILOGE("PbapPceService::Connect HFP-HF not connected for device %{public}s",
    //             GetEncryptAddr(device.GetAddress()).c_str());
    //         return Bluetooth::BT_ERR_INTERNAL_ERROR;
    //     }
    // }
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pce service manager impl not load");
    if (connectionObserver_) {
        std::shared_ptr<IPbapPceObserver> obs = connectionObserver_;
        serviceManagerIntfImpl->RegisterObserver(obs);
    }
    int32_t ret = serviceManagerIntfImpl->Connect(device);
    if (ret == Bluetooth::BT_NO_ERROR) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveConnectFlag(device.GetAddress(),
            PROFILE_ID_PBAP_PCE, true);
    }
    return ret;
}

int PbapPceService::Disconnect(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pce service manager impl not load");
    int32_t ret = serviceManagerIntfImpl->Disconnect(device);
    if (ret == Bluetooth::BT_NO_ERROR) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_PBAP_PCE, true);
    }
    return ret;
}

std::vector<RawAddress> PbapPceService::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<RawAddress> devices;
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, devices, "pbap pce service manager impl not load");
    std::vector<int32_t> states32(states.begin(), states.end());
    return serviceManagerIntfImpl->GetDevicesByStates(states32);
}

int PbapPceService::GetDeviceState(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, static_cast<int>(BTConnectState::DISCONNECTED),
        "pbap pce service manager impl not load");
    return serviceManagerIntfImpl->GetDeviceState(device);
}

int PbapPceService::SetConnectionStrategy(const RawAddress &device, int strategy)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pce service manager impl not load");
    return serviceManagerIntfImpl->SetConnectionStrategy(device, strategy);
}

int PbapPceService::GetConnectionStrategy(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pce service manager impl not load");
    return serviceManagerIntfImpl->GetConnectionStrategy(device);
}

std::list<RawAddress> PbapPceService::GetConnectDevices()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    std::list<RawAddress> devList;
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, devList, "pbap pce service manager impl not load");
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    auto devices = serviceManagerIntfImpl->GetDevicesByStates(states);
    for (auto &d : devices) {
        devList.push_front(d);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return devList;
}

int PbapPceService::GetConnectState()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, 0, "pbap pce service manager impl not load");
    return serviceManagerIntfImpl->GetConnectState();
}

void PbapPceService::Enable()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    DoInPbapThread([this]() { this->EnableService(); });
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceService::EnableService()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    loader_.OpenLib();
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    if (serviceManagerIntfImpl == nullptr) {
        PBAP_PCE_LOG_ERROR("pbap pce service manager impl not load");
        GetContext()->OnEnable(PROFILE_NAME_PBAP_PCE, false);
        return;
    }
    if (connectionObserver_) {
        std::shared_ptr<IPbapPceObserver> obs = connectionObserver_;
        serviceManagerIntfImpl->RegisterObserver(obs);
    }
    serviceManagerIntfImpl->Enable();
    GetContext()->OnEnable(PROFILE_NAME_PBAP_PCE, true);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceService::Disable()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    DoInPbapThread([this]() { this->DisableService(); });
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceService::DisableService()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    if (serviceManagerIntfImpl != nullptr) {
        serviceManagerIntfImpl->Disable();
        if (connectionObserver_) {
            std::shared_ptr<IPbapPceObserver> obs = connectionObserver_;
            serviceManagerIntfImpl->DeregisterObserver(obs);
        }
    }
    GetContext()->OnDisable(PROFILE_NAME_PBAP_PCE, true);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

bool PbapPceService::TryShutDown(bool ret)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, true, "pbap pce service manager impl not load");
    std::vector<int32_t> states {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED),
        static_cast<int32_t>(BTConnectState::DISCONNECTING)
    };
    if (serviceManagerIntfImpl->GetDevicesByStates(states).size() == 0) {
        GetContext()->OnDisable(PROFILE_NAME_PBAP_PCE, ret);
        PBAP_PCE_LOG_INFO("%{public}s end, Shutdown OK", __PRETTY_FUNCTION__);
        return true;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return false;
}

int PbapPceService::PullPhoneBook(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PBAP_PCE_LOG_INFO("%{public}s end (download deferred)", __PRETTY_FUNCTION__);
    return Bluetooth::BT_NO_ERROR;
}

bool PbapPceService::IsDownloading(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, false, "pbap pce service manager impl not load");
    bool ret = serviceManagerIntfImpl->IsDownloading(device);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return ret;
}

int PbapPceService::AbortDownloading(const RawAddress &device)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pce service manager impl not load");
    int32_t state = serviceManagerIntfImpl->GetDeviceState(device);
    if (state != static_cast<int>(BTConnectState::CONNECTED)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, NOT connected!", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return Bluetooth::BT_NO_ERROR;
}

int PbapPceService::GetMaxConnectNum()
{
    return 10;
}

int PbapPceService::SetDevicePassword(const RawAddress &device, const std::string &password, const std::string &userId)
{
    return 0;
}

REGISTER_CLASS_CREATOR(PbapPceService);
}  // namespace bluetooth
}  // namespace OHOS
