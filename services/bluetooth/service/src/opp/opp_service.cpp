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
#define LOG_TAG "bt_service_opp_service"
#endif

#include "opp_service.h"
#include "adapter_config.h"
#include "adapter_manager.h"
#include "base_observer_list.h"
#include "class_creator.h"
#include "connect_strategy_manager.h"
#include "dialog_pair.h"
#include "dialog_switch.h"
#include "log.h"
#include "common_util.h"
#include "opp_def.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "thread_util.h"
#include "system_ability_manager_utils.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const std::string OBEX_OBJECT_PUSH_UUID = "00001105-0000-1000-8000-00805f9b34fb";
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
struct OppService::impl {};

OppService::OppService() : utility::Context(PROFILE_NAME_OPP, "1.2.3"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME),
    pimpl(std::make_unique<impl>())
{
    HILOGI("OppService Create");
}

OppService::~OppService()
{
    isWakeUpApplication_ = false;
    UnloadOppServiceManagerInterfaceLib();
    HILOGI("OppService Release");
}

utility::Context *OppService::GetContext()
{
    return this;
}

OppService *OppService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<OppService *>(servManager->GetProfileService(PROFILE_NAME_OPP));
}


int32_t OppService::Connect(const RawAddress &device)
{
    return BT_NO_ERROR;
}

int32_t OppService::Disconnect(const RawAddress &device)
{
    return BT_NO_ERROR;
}

std::list<RawAddress> OppService::GetConnectDevices()
{
    std::list<RawAddress> devList;
    return devList;
}

int32_t OppService::GetConnectState()
{
    return BT_NO_ERROR;
}

int32_t OppService::GetMaxConnectNum()
{
    return RET_NO_SUPPORT;
}

void OppService::RegisterObserver(IOppObserver &observer)
{
    HILOGI("RegisterObserver");
    oppObservers_.Register(observer);
}

void OppService::DeregisterObserver(IOppObserver &observer)
{
    HILOGI("DeregisterObserver");
    oppObservers_.Deregister(observer);
}

void OppService::Enable()
{
    DoInOppThread([this] {this->EnableService(); });
}

void OppService::EnableService()
{
    StartSocketListener();
    GetContext()->OnEnable(PROFILE_NAME_OPP, true);
}

void OppService::StartSocketListener()
{
    if (obexServerSocket_ != nullptr) {
        HILOGI("obexServerSocket_ is alread create.");
        return;
    }
    socketObserver_ = std::make_shared<OppSocketObserver>();
    obexServerSocket_ = std::make_shared<ObexServerSocket>(OPP_SERVICE_NAME,
        OPP_RFCOMM_CHANNEL, OPP_L2CAP_PSM, EMPTY_UUID, socketObserver_);
    int securityFlags = 0; // SOCK_FLAG_AUTHENTICATION | SOCK_FLAG_ENCRYPTION;
    obexServerSocket_->Startup(securityFlags);
    CreateSdpRecord();
}

void OppService::CreateSdpRecord()
{
    if (oppSdpHandle > -1) {
        HILOGI("SDP record already created, oppSdpHandle = %{public}d", oppSdpHandle);
    }
    OppSdpCreateParam param;
    param.name = const_cast<char *>(OPP_SERVICE_NAME.data());
    param.channel = OPP_RFCOMM_CHANNEL;
    param.l2capPsm = OPP_L2CAP_PSM;
    param.version = SDP_OPP_SERVER_VERSION;
    oppSdpHandle = SdpAdapter::GetInstance().CreateOppSdpRecord(param);
    HILOGI("oppSdpHandle = %{public}d", oppSdpHandle);
}

int OppService::SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders)
{
    std::vector<std::string> filePaths;
    for (const auto &file : fileHolders) {
        filePaths.push_back(file.GetFilePath());
    }
    ControlInterceptMessage msg {
        .addr = device.GetAddress(),
        .filePaths = filePaths,
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedOppWrite(msg)) {
        return RET_NO_SUPPORT;
    }
    int ret = RET_NO_ERROR;
    LoadOppServiceManagerInterfaceLib();
    HILOGI("device is %{public}s", GET_ENCRYPT_ADDR(device));
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, RET_BAD_STATUS, "opp service manager impl not load");
    ret = serviceManagerIntfImpl->SendFile(device, fileHolders);
    if (ret == RET_NO_SUPPORT) {
        SdpAdapter::GetInstance().SetOppSdpFoundCallback(
            [this] (const OppSdpFoundRecord &record) {this->SdpFoundCallback(record);});
        SdpAdapter::GetInstance().StartRemoteSdpSearch(device.GetAddress(), OBEX_OBJECT_PUSH_UUID);
        ret = RET_NO_ERROR;
    }
    return ret;
}

void OppService::SdpFoundCallback(const OppSdpFoundRecord &record)
{
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceImplIntf, "Get opp service impl interface failed");
    serviceImplIntf->SdpFoundCallback(record);
}

void OppService::Disable()
{
    DoInOppThread([this] {this->DisableService();});
}

void OppService::DisableService()
{
    StopSocketListener();
    GetContext()->OnDisable(PROFILE_NAME_OPP, true);
}

int OppService::ConnectSocket(const std::string &addr, const Uuid &uuid, int securityFlag, int type, int psm)
{
    HILOGI("securityFlag is %{public}d type is %{public}d psm is %{public}d", securityFlag, type, psm);
    IProfileSocket *socketService =
        (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
    CHECK_AND_RETURN_LOG_RET(socketService, -1, "socketService is null");
    return socketService->Connect(addr, uuid, securityFlag, type, psm);
}

void OppService::LoadOppServiceManagerInterfaceLib(void)
{
    if (!isLoaded_) {
        loader_.OpenLib();
    }
 
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "Get opp service manager interface failed");
    isLoaded_ = true;

    oppConnectionObserver_ = std::make_shared<OppConnectionObserver>();
    serviceManagerIntfImpl->RegisterObserver(oppConnectionObserver_);
    auto connectSocketFunc = [this](const std::string &addr, const Uuid &uuid, int securityFlag,
        int type, int psm) -> int {
        return this->ConnectSocket(addr, uuid, securityFlag, type, psm);
    };
    auto unloadOppFunc = [this]() -> bool {
        UnloadOppServiceManagerInterfaceLib();
        return true;
    };
    auto createOppSdpFunc = [this](const std::string &addr) -> void {
        SdpAdapter::GetInstance().SetOppSdpFoundCallback(
            [this] (const OppSdpFoundRecord &record) {this->SdpFoundCallback(record);});
        SdpAdapter::GetInstance().StartRemoteSdpSearch(addr, OBEX_OBJECT_PUSH_UUID);
        return;
    };
    auto isAllowedOppReadFunc = [](const std::string &addr) -> bool {
        ControlInterceptMessage msg {
            .addr = addr,
            .pid = IPCSkeleton::GetCallingPid(),
            .uid = IPCSkeleton::GetCallingUid(),
        };
        return ControlInterceptIsAllowedOppRead(msg);
    };
    auto oppContinuousTaskFunc = [this](const std::string &action) -> void {
        HILOGI("ContinuousTaskEvent receive event.");
        return this->OppContinuousTask(action);
    };
    auto aclDisConnEventFunc = [this](const std::string &addr) -> void {
        return this->AclDisConnEvent(addr);
    };
    serviceManagerIntfImpl->DllRegisterConnectSocketFunc(connectSocketFunc);
    serviceManagerIntfImpl->DllRegisterUnloadOppFunc(unloadOppFunc);
    serviceManagerIntfImpl->DllRegisterCreateOppSdpFunc(createOppSdpFunc);
    serviceManagerIntfImpl->DllRegisterIsAllowedOppReadFunc(isAllowedOppReadFunc);
    serviceManagerIntfImpl->DllRegisterOppContinuousTaskFunc(oppContinuousTaskFunc);
    serviceManagerIntfImpl->DllRegisterAclDisConnEventFunc(aclDisConnEventFunc);
}

void OppService::AclDisConnEvent(const std::string &addr)
{
    HILOGI("AclDisConnEvent %{public}s", GetEncryptAddr(addr).c_str());
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "Get opp service manager interface failed");
    serviceManagerIntfImpl->RemoveAllDeviceTransfer(addr);
}

void OppService::OppContinuousTask(const std::string &action)
{
    HILOGI("OppContinuousTask action: %{public}s", action.c_str());
    oppObservers_.ForEach([action](IOppObserver &observer) {
        observer.OppContinuousTask(action);
    });
}

void OppService::UnloadOppServiceManagerInterfaceLib(void)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "Get opp service manager interface failed");
    serviceManagerIntfImpl->DeregisterObserver();
}


void OppService::StopSocketListener()
{
    // 蓝牙关闭后，关闭socket
    HILOGI("StopSocketListener");
    if (obexServerSocket_ != nullptr) {
        obexServerSocket_->Shutdown();
        obexServerSocket_ = nullptr;
    }
    CleanSdpRecord();
}

void OppService::CleanSdpRecord()
{
    HILOGI("oppSdpHandle = %{public}d", oppSdpHandle);
    if (oppSdpHandle < 0) {
        HILOGI("cleanUpSdpRecord, SDP record never created");
        return;
    }
    SdpAdapter::GetInstance().RemoveSdpRecord(oppSdpHandle);
}

void OppService::LoadLibAndConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    LoadOppServiceManagerInterfaceLib();
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceManagerIntfImpl, "opp service manager impl not load");
    HILOGI("device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    serviceManagerIntfImpl->HandlerConnectEvt(socketDevice);
}

void OppSocketObserver::OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    if (socketDevice == nullptr) {
        HILOGE("socketDevice is null");
        return;
    }
    HILOGI("device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    if (OppService::GetService() == nullptr) {
        HILOGI("OppService is nullptr");
        return;
    }
    OppService::GetService()->LoadLibAndConnect(socketDevice);
}

void OppService::ReStartWhenSocketServerException()
{
    DoInOppThread([this]() { this->StopSocketListener(); });
    DoInOppThread([this]() { this->StartSocketListener(); });
}

void OppService::NotifyTransferStateChanged(IOppTransferInformation info)
{
    HILOGI("status %{public}d reason %{public}d", info.GetStatus(), info.GetResult());
    oppObservers_.ForEach([info](IOppObserver &observer) {
        observer.OnTransferStateChange(info);
    });
    if (info.GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND &&
        info.GetStatus() == OPP_TRANSFER_STATUS_FINISH &&
        info.GetResult() == OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED) {
        HILOGI("ReStartWhenSocketServerException");
        ReStartWhenSocketServerException();
    }
}

void OppService::NotifyReceiveIncomingFile(IOppTransferInformation info)
{
    HILOGI("status: %{public}d", info.GetStatus());
    oppObservers_.ForEach([info](IOppObserver &observer) {
        observer.OnReceiveIncomingFile(info);
    });
}

void OppConnectionObserver::OnReceiveIncomingFile(const IOppTransferInformation &transferInformation)
{
    if (OppService::GetService() == nullptr) {
        HILOGI("OppService is nullptr");
        return;
    }
    OppService::GetService()->NotifyReceiveIncomingFile(transferInformation);
}

void OppConnectionObserver::OnTransferStateChange(const IOppTransferInformation &transferInformation)
{
    if (OppService::GetService() == nullptr) {
        HILOGI("OppService is nullptr");
        return;
    }
    OppService::GetService()->NotifyTransferStateChanged(transferInformation);
}

std::vector<RawAddress> OppService::GetDevicesByStates(std::vector<int> states)
{
    std::vector<RawAddress> devices;
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, devices, "opp service manager impl not load");
    return serviceManagerIntfImpl->GetDevicesByStates(states);
}

int32_t OppService::GetDeviceState(const RawAddress &device)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, static_cast<int>(BTConnectState::DISCONNECTED),
        "opp service manager impl not load");
    return serviceManagerIntfImpl->GetDeviceState(device);
}

int OppService::CancelTransfer()
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, RET_BAD_STATUS, "opp service manager impl not load");
    return serviceManagerIntfImpl->CancelTransfer();
}

int OppService::SetIncomingFileConfirmation(const bool accept, int fd)
{
    HILOGI("accept is %{public}d fd is %{public}d", accept, fd);
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, RET_BAD_STATUS, "opp service manager impl not load");
    return serviceManagerIntfImpl->SetIncomingFileConfirmation(accept, fd);
}

int OppService::SetLastReceivedFileUri(const std::string &uri)
{
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, RET_BAD_STATUS, "opp service manager impl not load");
    return serviceManagerIntfImpl->SetLastReceivedFileUri(uri);
}

IOppTransferInformation OppService::GetCurrentTransferInformation()
{
    IOppTransferInformation ret;
    auto serviceManagerIntfImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl, ret, "opp service manager impl not load");
    return serviceManagerIntfImpl->GetCurrentTransferInformation();
}
REGISTER_CLASS_CREATOR(OppService);
}  // namespace bluetooth
}  // namespace OHOS
