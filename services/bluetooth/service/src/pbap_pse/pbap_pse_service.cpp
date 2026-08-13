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
#define LOG_TAG "bt_service_pbap_pse_service"
#endif

#include "adapter_config.h"
#include "adapter_manager.h"
#include "base_observer_list.h"
#include "class_creator.h"
#include "connect_strategy_manager.h"
#include "log.h"
#include "common_util.h"
#include "parameter.h"
#include "profile_config.h"
#include "pbap_pse_message.h"
#include "pbap_pse_service.h"
#include "profile_service_manager.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "../dialog/bluetooth_dialog.h"
#include "thread_util.h"
#include "datetime_ex.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
constexpr int REQUEST_PERMISSION_TIMEOUT_MS = 30000;  // 30s
constexpr const char* PERSIST_SMS_CALLLOG_ENABLE = "persist.bluetooth.sync_sms_calllog_enable";
struct PbapPseService::impl {};

PbapPseService::PbapPseService() : utility::Context(PROFILE_NAME_PBAP_PSE, "1.2.3"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME),
    pimpl(std::make_unique<impl>())
{
    HILOGI("PbapPseService Create");
}

PbapPseService::~PbapPseService()
{
    HILOGI("PbapPseService Release");
}

utility::Context *PbapPseService::GetContext()
{
    return this;
}

PbapPseService *PbapPseService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<PbapPseService *>(servManager->GetProfileService(PROFILE_NAME_PBAP_PSE));
}

void PbapPseService::EnableService()
{
    StartSocketListener();
    GetContext()->OnEnable(PROFILE_NAME_PBAP_PSE, true);
    permissionRequestTimeMap_ = std::make_shared<SafeMap<const std::string, int64_t>>();
    const char* isHasPermission = HasPbapOrMapPermission() ? "1" : "0";
    SetParameter(PERSIST_SMS_CALLLOG_ENABLE, isHasPermission);
}

void PbapPseService::Enable()
{
    DoInPbapThread([this]() { this->EnableService(); });
}

void PbapPseService::DisableService()
{
    StopSocketListener();
    permissionRequestTimeMap_ = nullptr;
    GetContext()->OnDisable(PROFILE_NAME_PBAP_PSE, true);
}

void PbapPseService::LoadPbapPseServiceManagerInterfaceLib(void)
{
    if (!isLoaded_) {
        loader_.OpenLib();
    }

    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "Get pbap service manager interface failed");

    isLoaded_ = true;
    pbapPseConnectionObserver_ = std::make_shared<PbapPseConnectionObserver>();
    serviceManagerIntfImpl->RegisterObserver(pbapPseConnectionObserver_);
    // if isShow is true, show pbap dialog, otherwise dismiss pbap dialog
    auto dllRegisterFunc = [](const std::string &address, std::shared_ptr<utility::Timer> timer, bool isShow) -> bool {
        if (isShow) {
            DialogInfo dialog{address, PBAP_AUTH_DIALOG, REQUEST_PERMISSION_TIMEOUT_MS, timer};
            return BluetoothDialog::RequestAuthDialog(dialog);
        } else {
            return BluetoothDialog::DismissCurAndShowNext();
        }
    };
    serviceManagerIntfImpl->DllRegisterFunc(dllRegisterFunc, permissionRequestTimeMap_);
}

void PbapPseService::UnloadPbapPseServiceManagerInterfaceLib(void)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "Get pbap service manager interface failed");
    serviceManagerIntfImpl->DeregisterObserver(pbapPseConnectionObserver_);
}

void PbapPseService::Disable()
{
    DoInPbapThread([this]() { this->DisableService(); });
}

void PbapPseService::StartSocketListener()
{
    if (obexServerSocket_ != nullptr) {
        HILOGI("obexServerSocket_ is alread create.");
        return;
    }
    socketObserver_ = std::make_shared<PbapPseSocketObserver>();
    obexServerSocket_ = std::make_shared<ObexServerSocket>(PBAP_PSE_SERVICE_NAME, PBAP_RFCOMM_CHANNEL,
        -1, EMPTY_UUID, socketObserver_);
    int securityFlags = 3; // SOCK_FLAG_AUTHENTICATION | SOCK_FLAG_ENCRYPTION;
    obexServerSocket_->Startup(securityFlags);
    CreateSdpRecord();
}

void PbapPseService::StopSocketListener()
{
    // 蓝牙关闭后，关闭socket
    HILOGI("StopSocketListener");
    if (obexServerSocket_ != nullptr) {
        obexServerSocket_->Shutdown();
        obexServerSocket_ = nullptr;
    }
    
    CleanSdpRecord();
}

void PbapPseService::ReStartWhenSocketServerException()
{
    DoInPbapThread([this]() { this->StopSocketListener(); });
    DoInPbapThread([this]() { this->StartSocketListener(); });
}

void PbapPseService::CreateSdpRecord()
{
    if (mSdpHandle > -1) {
        HILOGI("CreateSdpRecord, SDP record already created, mSdpHandle = %{public}d", mSdpHandle);
    }
    PbapPseSdpCreateParam param;
    param.name = const_cast<char *>(PBAP_PSE_SERVICE_NAME.data());
    param.channel = PBAP_RFCOMM_CHANNEL;
    param.l2capPsm = SDP_PBAP_OMIT_L2CAP;
    param.version = SDP_PBAP_SERVER_VERSION;
    param.supportedRepositories = SDP_PBAP_SUPPORTED_REPOSITORIES;
    param.features = SDP_PBAP_SUPPORTED_FEATURES;
    mSdpHandle = SdpAdapter::GetInstance().CreatePbapPseSdpRecord(param);
    HILOGI("CreateSdpRecord, mSdpHandle = %{public}d", mSdpHandle);
}

void PbapPseService::CleanSdpRecord()
{
    HILOGI("mSdpHandle = %{public}d", mSdpHandle);
    if (mSdpHandle < 0) {
        HILOGI("cleanUpSdpRecord, SDP record never created");
        return;
    }
    SdpAdapter::GetInstance().RemoveSdpRecord(mSdpHandle);
}

int32_t PbapPseService::Connect(const RawAddress &device)
{
    return BT_NO_ERROR;
}

int32_t PbapPseService::Disconnect(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "pbap pse service manager impl not load");
    int32_t ret = serviceManagerIntfImpl->Disconnect(device);
    if (ret == BT_NO_ERROR) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_PBAP_PSE, true);
    }
    return ret;
}

std::list<RawAddress> PbapPseService::GetConnectDevices()
{
    std::list<RawAddress> devList;
    return devList;
}

int32_t PbapPseService::GetConnectState()
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, PROFILE_STATE_DISCONNECTED,
        "pbap pse service manager impl not load");
    return serviceManagerIntfImpl->GetConnectState();
}

int32_t PbapPseService::GetMaxConnectNum()
{
    return RET_NO_SUPPORT;
}

int32_t PbapPseService::GetDeviceState(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, static_cast<int>(BTConnectState::DISCONNECTED),
        "pbap pse service manager impl not load");
    return serviceManagerIntfImpl->GetDeviceState(device);
}

std::vector<RawAddress> PbapPseService::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<RawAddress> devices;
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, devices, "pbap pse service manager impl not load");
    return serviceManagerIntfImpl->GetDevicesByStates(states);
}

int32_t PbapPseService::SetConnectionStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_PBAP_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED)) {
            Disconnect(device);
            HILOGI("disconnect pbap pse.");
        }
    }
    return BT_NO_ERROR;
}

int32_t PbapPseService::GetConnectionStrategy(const RawAddress &device) const
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_PBAP_CONNECTION_POLICY);
}

int32_t PbapPseService::SetShareType(const RawAddress& device, int32_t shareType)
{
    HILOGI("device: %{public}s, shareType: %{public}d", GET_ENCRYPT_ADDR(device), shareType);
    bool result = SetPbapShareType(device.GetAddress(), shareType);
    if (!result) {
        HILOGE("SetShareType fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t PbapPseService::GetShareType(const RawAddress& device, int32_t &shareType)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    bool result = GetPbapShareType(device.GetAddress(), shareType);
    if (!result) {
        HILOGE("GetShareType fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t PbapPseService::SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization)
{
    HILOG_COMM_INFO("SetPhoneBookAccessAuthorization: %{public}s, accessAuthorization: %{public}d",
        GET_ENCRYPT_ADDR(device), accessAuthorization);
    BluetoothDialog::DismissCurAndShowNext();
    bool result = SetPbapPermission(device.GetAddress(), accessAuthorization);
    if (!result) {
        HILOGE("SetPbapPermission fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    const char* isHasPermission = HasPbapOrMapPermission() ? "1" : "0";
    SetParameter(PERSIST_SMS_CALLLOG_ENABLE, isHasPermission);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    int32_t pbapLoadFlag = serviceManagerIntfImpl != nullptr ? 1 : 0;
    NotifyPhonebookAuthResultToHfp(device, accessAuthorization, pbapLoadFlag);
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, BT_NO_ERROR, "pbap pse service manager impl not load");
    serviceManagerIntfImpl->SetPhoneBookAccessAuthorization(device, accessAuthorization);
    return BT_NO_ERROR;
}

void PbapPseService::NotifyPhonebookAuthResultToHfp(const RawAddress& device, int32_t accessAuthorization,
    int32_t pbapLoadFlag)
{
    IProfileHfpAg *hfpAgService_ =
        static_cast<IProfileHfpAg *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG(hfpAgService_ != nullptr, "hfpAgService_ is null");
    hfpAgService_->NotifyPhonebookAuthResult(device, accessAuthorization, pbapLoadFlag);
}

int32_t PbapPseService::GetPhoneBookAccessAuthorization(const RawAddress& device, int32_t &accessAuthorization)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    bool result = GetPbapPermission(device.GetAddress(), accessAuthorization);
    if (!result) {
        HILOGE("GetPhoneBookAccessAuthorization fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

void PbapPseService::RegisterObserver(IPbapPseObserver &observer)
{
    HILOGI("RegisterObserver");
    pbapPseObservers_.Register(observer);
}

void PbapPseService::DeregisterObserver(IPbapPseObserver &observer)
{
    HILOGI("DeregisterObserver");
    pbapPseObservers_.Deregister(observer);
}

void PbapPseService::LoadLibAndConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    LoadPbapPseServiceManagerInterfaceLib();
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "pbap pse service manager impl not load");
    serviceManagerIntfImpl->HandlerConnectEvt(socketDevice);
}

void PbapPseService::NotifyStateChanged(const RawAddress &device, int state, int cause)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("NotifyStateChanged ConnectState:%{public}d, cause:%{public}d", state, cause);
    pbapPseObservers_.ForEach([device, state, cause](IPbapPseObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
}

void PbapPseSocketObserver::OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    PbapPseService *pbapPseService = PbapPseService::GetService();
    if (pbapPseService == nullptr) {
        HILOGE("pbapPseService is null");
        return;
    }
    if (socketDevice == nullptr) {
        HILOGE("socket server error, restart socket listener");
        pbapPseService->ReStartWhenSocketServerException();
        return;
    }
    HILOGI("PbapPseSocketObserver OnConnect, device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    RawAddress device(socketDevice->mDeviceAddress);
    int state = pbapPseService->GetDeviceState(device);
    if (state != static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGW("device is not disconnected, ignore");
        return;
    }

    pbapPseService->LoadLibAndConnect(socketDevice);
}

void PbapPseConnectionObserver::OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause)
{
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_PBAP_PSE, false);
    }
    cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device, PROFILE_ID_PBAP_PSE, state);
    PbapPseService *pbapPseService = PbapPseService::GetService();
    if (pbapPseService == nullptr) {
        HILOGE("pbapPseService is null");
        return;
    }
    pbapPseService->NotifyStateChanged(device, state, cause);
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        DoInPbapThread([service = PbapPseService::GetService()]() {
            service->UnloadPbapPseServiceManagerInterfaceLib();
        });
    }
}

REGISTER_CLASS_CREATOR(PbapPseService);
}  // namespace bluetooth
}  // namespace OHOS
