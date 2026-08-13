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
#define LOG_TAG "bt_service_opp_service_impl"
#endif

#include "opp_file_utils.h"
#include "opp_service_impl.h"
#include "datetime_ex.h"

namespace OHOS {
namespace bluetooth {
//using namespace OHOS::Bluetooth;
extern "C" OppServiceImplInterface *CreateOppServiceImplInterface(void)
{
    return new OppServiceImpl();
}

extern "C" void DestroyOppServiceImplInterface(OppServiceImplInterface *interface)
{
    if (interface == nullptr) {
        HILOGE("opp manager Interface is nullptr");
        return;
    }
    delete interface;
}

OppServiceImpl::~OppServiceImpl()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    stateMachines_.clear();
    DeregisterObserver();
}

void OppServiceImpl::RegisterObserver(std::shared_ptr<IOppObserver> &observer)
{
    oppConnectionObserver_ = observer;
}

void OppServiceImpl::DeregisterObserver()
{
    oppConnectionObserver_ = nullptr;
}

IOppTransferInformation OppServiceImpl::GetCurrentTransferInformation()
{
    IOppTransferInformation ret;
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        return ret;
    }
    return oppTransfer->GetCurrentTransferInformation();
}

void OppServiceImpl::SdpFoundCallback(const OppSdpFoundRecord &record)
{
    if (record.status != 0) {
        HILOGE("remote device %{public}s not support opp status %{public}d",
            GET_ENCRYPT_STR_ADDR(record.deviceAddr), record.status);
        std::shared_ptr<OppTransfer> iter = nullptr;
        std::string deviceAddr = record.deviceAddr;
        int index = TRANSFER_ID_MAX + 1;
        oppTransferList_.ForEach([deviceAddr, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
            if (oppTransfer->GetDeviceAddress() == deviceAddr && index > oppTransfer->transferId_) {
                HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
                iter = oppTransfer;
                index = oppTransfer->transferId_;
            }
        });
        if (iter != nullptr) {
            std::shared_ptr<IOppTransferInformation> file = nullptr;
            if (iter->fileList_.Front(file) && oppConnectionObserver_ != nullptr) {
                file->SetStatus(OPP_TRANSFER_STATUS_FINISH);
                file->SetResult(OPP_TRANSFER_RESULT_ERROR_UNSUPPORTED_TYPE);
                oppConnectionObserver_->OnTransferStateChange(*file);
            }
            oppTransferList_.Erase(iter);
            OppMessage event(OPP_DISCONNECTED_EVT);
            event.dev_ = iter->GetDeviceAddress();
            PostEvent(event);
            StartNextTransfer();
        }
        return;
    }
    OppMessage event(OPP_SDP_CMPL_EVT);
    event.dev_ = record.deviceAddr;
    event.oppRecord_ = record;
    PostEvent(event);
}

int OppServiceImpl::SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders)
{
    HILOGI("device is %{public}s file size is %{public}lu", GET_ENCRYPT_ADDR(device), fileHolders.size());
    std::string address = device.GetAddress();

    std::shared_ptr<OppTransfer> oppTransfer = std::make_shared<OppTransfer>(
        address, fileHolders, OPP_TRANSFER_DIRECTION_OUTBOUND, nullptr, this);
    if (oppTransfer->GetFileNumber() <= 0) {
        HILOGE("send file size is 0");
        return RET_BAD_STATUS;
    }
    OppAbilityManagerUtils::SubscribTransactionChrEvent();
    oppTransferList_.Push(oppTransfer);
    std::shared_ptr<OppTransfer> doingTransfer = nullptr;
    if (oppTransferList_.Front(doingTransfer) && doingTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGI("receive task is doing. sendFile task line up");
    } else {
        std::string fileName = GetFirstTransferFileName(address);
        std::thread([fileName] {OppAbilityManagerUtils::WakeUpOppSendUIAbility(fileName);}).detach();
    }
    HILOGI("oppTransferList_ size is %{public}d", oppTransferList_.Size());
    if (oppTransferList_.Size() == 1) {
        return RET_NO_SUPPORT;
    }
    return RET_NO_ERROR;
}

int OppServiceImpl::CancelTransfer()
{
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        return RET_BAD_STATUS;
    }
    int result = oppTransfer->CancelTransfer();
    oppTransferList_.Erase(oppTransfer);
    HILOGI("cancelTransfer result is %{public}d", result);
    return result;
}

int OppServiceImpl::SetIncomingFileConfirmation(const bool accept, int fd)
{
    std::shared_ptr<OppTransfer> inboundTransfer = nullptr;
    if (oppTransferList_.Size() <= 0) {
        HILOGI("opp transfer is nullptr");
        return RET_BAD_STATUS;
    }
    oppTransferList_.ForEach([&inboundTransfer](std::shared_ptr<OppTransfer> &oppTransfer) {
        HILOGI("Address=[%{public}s], direction=[%{public}d]",
            GET_ENCRYPT_STR_ADDR(oppTransfer->GetDeviceAddress()), oppTransfer->GetDirection());
        if (oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND) {
            inboundTransfer = oppTransfer;
        }
    });
    if (!inboundTransfer) {
        HILOGE("no inbound Transfer.");
        return RET_BAD_STATUS;
    }
    HILOGI("SetIncomingFileConfirmation result is %{public}d", accept);
    return inboundTransfer->SetIncomingFileConfirmation(accept, fd);
}

int OppServiceImpl::SetLastReceivedFileUri(const std::string &uri)
{
    if (uri.empty()) {
        HILOGE("uri is empty");
        return RET_BAD_STATUS;
    }

    std::map<std::string, std::string> params;
    if (uri.compare(0, MEDIA_PREFIX.size(), MEDIA_PREFIX) == 0) {
        params.emplace("fileUriType", "media");
        params.emplace("bundleName", PHOTO_BUNDLENAME);
        params.emplace("abilityName", PHOTO_ABILITYNAME);
    } else if (uri.compare(0, DOCS_PREFIX.size(), DOCS_PREFIX) == 0) {
        params.emplace("fileUriType", "files");
        params.emplace("bundleName", FILES_BUNDLENAME);
        params.emplace("abilityName", FILES_ABILITYNAME);
    }  else if (uri.compare(0, REMOVEFILE_PREFIX.size(), REMOVEFILE_PREFIX) == 0) {
        params.emplace("removeFileUri", uri.substr(REMOVEFILE_PREFIX.size()));
    } else {
        HILOGE("uri is invalid");
        return RET_BAD_STATUS;
    }
    params.emplace("fileUri", uri);
    OppAbilityManagerUtils::StartUIExtentionAbility(params);
    return RET_NO_ERROR;
}

void OppServiceImpl::ConnectObex(std::shared_ptr<ObexClientConfig> obexConfig)
{
    if (obexConfig == nullptr) {
        HILOGE("not found obexConfig");
        return;
    }
    HILOGI("obexConfig->addr_ is %{public}s", GET_ENCRYPT_STR_ADDR(obexConfig->addr));

    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        HILOGE("not find transfer");
        return;
    }
    obexConfig->fileCount = oppTransfer->GetFileNumber();
    HILOGI("obexConfig->addr_ is %{public}s fileNum is %{public}d",
        GET_ENCRYPT_STR_ADDR(obexConfig->addr), obexConfig->fileCount);
    if (oppTransfer->GetDeviceAddress() == obexConfig->addr) {
        if (oppTransfer->ConnectObex(obexConfig) != RET_NO_ERROR) {
            HILOGE("ConnectObex error");
            oppTransferList_.Erase(oppTransfer);
            DoInOppThread([this] {this->StartNextTransfer();});
        }
    } else {
        HILOGE("oppTransferList_.front() %{public}s not current device",
            GET_ENCRYPT_STR_ADDR(obexConfig->addr));
    }
}

void OppServiceImpl::OnObexDisconnected(const std::string &device)
{
    std::shared_ptr<OppTransfer> iter = nullptr;
    int index = TRANSFER_ID_MAX + 1;
        oppTransferList_.ForEach([device, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
            if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_) {
                HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
                iter = oppTransfer;
                index = oppTransfer->transferId_;
            }
        });
    if (iter == nullptr) {
        HILOGE("address %{public}s not find transfer", GET_ENCRYPT_STR_ADDR(device));
        return;
    }
    DoInOppThread([this] {this->StartNextTransfer();});

    iter->OnObexDisconnected();
    oppTransferList_.Erase(iter);
}

void OppServiceImpl::StartNextTransfer()
{
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        HILOGI("no more file to transfer");
        return;
    }
    if (oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_OUTBOUND) {
        createOppSdpFunc_(oppTransfer->GetDeviceAddress());
    } else {
        oppTransfer->AcceptConnect();
    }
}

void OppServiceImpl::OnObexConnected(const std::string &device)
{
    HILOGI("obexConnected device %{public}s", GET_ENCRYPT_STR_ADDR(device));
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        HILOGE("current device don't have transfer task.");
        return;
    }
    int ret = RET_NO_ERROR;
    if (oppTransfer->GetDeviceAddress() == device) {
        ret = oppTransfer->StartTransfer();
        if (ret != RET_NO_ERROR) {
            oppTransfer->OnTransferStateChange(OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        }
    }
}

void OppServiceImpl::DisconnectObex(const std::string &device)
{
    HILOGE("disconnectObex device %{public}s", GET_ENCRYPT_STR_ADDR(device));
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        HILOGE("disconnect obex not find transfer");
        return;
    }
    if (oppTransfer->GetDeviceAddress() == device) {
        oppTransfer->DisconnectObex();
    } else {
        HILOGE("not current device");
    }
}

void OppServiceImpl::RemoveTransfer(const std::string &device)
{
    HILOGE("RemoveTransfer device %{public}s", GET_ENCRYPT_STR_ADDR(device));
    if (oppTransferList_.Empty()) {
        HILOGE("RemoveTransfer not find transfer");
        return;
    }
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    oppTransferList_.Front(oppTransfer);
    if (oppTransfer->GetDeviceAddress() == device) {
        HILOGI("erase transfer");
        oppTransferList_.Erase(oppTransfer);
    } else {
        HILOGE("not current device");
    }
}

void OppServiceImpl::RemoveAllDeviceTransfer(const std::string &device)
{
    HILOGE("RemoveAllDeviceTransfer device %{public}s", GET_ENCRYPT_STR_ADDR(device));
    if (oppTransferList_.Empty()) {
        HILOGE("RemoveAllDeviceTransfer not find transfer");
        return;
    }
    std::shared_ptr<OppTransfer> currentOppTransfer = nullptr;
    oppTransferList_.Front(currentOppTransfer);
    bool isCurrentTransfer = (currentOppTransfer->GetDeviceAddress() == device);
    if (isCurrentTransfer) {
        currentOppTransfer->SetConfirm(OPP_TRANSFER_CONFIRM_REJECT);
        OppMessage event(OPP_DISCONNECTED_EVT);
        event.dev_ = device;
        PostEvent(event);

        std::shared_ptr<IOppTransferInformation> file = std::make_shared<IOppTransferInformation>();
        file->SetStatus(OPP_TRANSFER_STATUS_FINISH);
        file->SetResult(OPP_TRANSFER_RESULT_ERROR_CANCELED);
        file->SetDirection(OPP_TRANSFER_DIRECTION_INBOUND);
        file->SetTotalCount(TRANSFER_DEFAULT_COUNT);
        file->SetDeviceAddress(device);
        NotifyTransferStateChanged(*file);
    } else {
        HILOGI("RemoveAllDeviceTransfer not current transfer");
    }
    oppTransferList_.EraseIf([device](std::shared_ptr<OppTransfer> &oppTransfer) {
        return oppTransfer->GetDeviceAddress() == device;
    });
}

void OppServiceImpl::RemoveStateMachine(const std::string &device)
{
    OppMessage event(OPP_REMOVE_STATE_MACHINE_EVT);
    event.dev_ = device;
    PostEvent(event);
}

void OppServiceImpl::NotifyTransferStateChanged(IOppTransferInformation info)
{
    HILOGI("GetCurrentBytes %{public}d, GetTotalBytes %{public}d", info.GetCurrentBytes(), info.GetTotalBytes());
    if (info.GetStatus() == OPP_TRANSFER_STATUS_RUNNING && info.GetCurrentBytes() != info.GetTotalBytes()) {
        int64_t now = GetSecondsSince1970ToNow();
        if (now - lastNotifyRunningStateTime_ < NOTICE_INTERVAL_TIME) {
            return;
        }
        lastNotifyRunningStateTime_ = now;
    }
    if (info.GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND &&
        info.GetStatus() == OPP_TRANSFER_STATUS_FINISH &&
        info.GetResult() == OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED) {
        std::shared_ptr<OppTransfer> iter;
        std::string device = info.GetDeviceAddress();
        int index = TRANSFER_ID_MAX + 1;
        oppTransferList_.ForEach([device, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
            if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_ &&
                oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND) {
                HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
                iter = oppTransfer;
                index = oppTransfer->transferId_;
            }
        });
        if (iter != nullptr) {
            oppTransferList_.Erase(iter);
            OppMessage event(OPP_DISCONNECTED_EVT);
            event.dev_ = iter->GetDeviceAddress();
            PostEvent(event);
            DoInOppThread([this] {this->StartNextTransfer();});
        }
    }
    if (oppConnectionObserver_ != nullptr) {
        oppConnectionObserver_->OnTransferStateChange(info);
    }
}

std::string OppServiceImpl::GetFirstTransferFileName(const std::string &device) 
{
    std::string fileName = "";
    if (oppTransferList_.Empty()) {
        HILOGE("[OPP Service]:Transfer list_ is empty");
        return fileName;
    }
    std::shared_ptr<OppTransfer> iter = nullptr;
    int index = TRANSFER_ID_MAX + 1;
    oppTransferList_.ForEach([device, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
        if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_) {
            HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
            iter = oppTransfer;
            index = oppTransfer->transferId_;
        }
    });
    if (iter == nullptr) {
        HILOGE("not find transfer");
        return fileName;
    }

    fileName = iter->GetFirstTransferFileName();
    return fileName;
}


void OppServiceImpl::NotifyReceiveIncomingNoConfirmFile(IOppTransferInformation info)
{
    AAFwk::Want abilityWant;
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothReceiveUIAbility";
    abilityWant.SetElementName(bundleName, abilityName);
 
    AAFwk::Want eventWant;
    std::string eventAction = "usual.event.bluetooth.OPP.TAP.ACCEPT";
    eventWant.SetAction(eventAction);
    eventWant.SetParam("acceptType", OPP_TRANSFER_AUTO_ACCEPT);
    eventWant.SetParam("fileName", info.GetFileName());
    eventWant.SetParam("currentCount", info.GetCurrentCount());
    eventWant.SetParam("totalCount", info.GetTotalCount());

    OppAbilityManagerUtils::StartUIAbilityByCall(abilityWant, eventWant);
    if (oppConnectionObserver_ != nullptr) {
        oppConnectionObserver_->OnReceiveIncomingFile(info);
    }
}

void OppServiceImpl::NotifyReceiveIncomingFile(IOppTransferInformation info)
{
    HILOGI("totalCount %{public}d", info.GetTotalCount());
    AAFwk::Want abilityWant;
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothReceiveUIAbility";
    abilityWant.SetElementName(bundleName, abilityName);
 
    AAFwk::Want eventWant;
    std::string eventAction = "usual.event.bluetooth.OPP.RECEIVE";
    eventWant.SetAction(eventAction);
    eventWant.SetParam("fileName", info.GetFileName());
    eventWant.SetParam("totalCount", info.GetTotalCount());

    OppAbilityManagerUtils::StartUIAbilityByCall(abilityWant, eventWant);
    if (oppConnectionObserver_ != nullptr) {
        oppConnectionObserver_->OnReceiveIncomingFile(info);
    }
}

void OppServiceImpl::NotifyStateChanged(const RawAddress &device, int state)
{
    HILOGI("newState=%{public}d", stateMap_.at(state));
}

int OppServiceImpl::OnReceiveIncomingConnect(uint32_t connectId, int32_t totalFileCount,
    std::string remoteAddr, std::shared_ptr<ObexServerSession> serverSession)
{
    HILOGI("onConnect, creat a transfer");
    // when receiving a file, reject all income transfer request.
    bool isAllow = true;
    oppTransferList_.ForEach([&isAllow, remoteAddr](std::shared_ptr<OppTransfer> &oppTransfer) {
        HILOGI("Address=[%{public}s], direction=[%{public}d]",
            GET_ENCRYPT_STR_ADDR(oppTransfer->GetDeviceAddress()), oppTransfer->GetDirection());
        if (oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND ||
            oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_OUTBOUND) {
            isAllow = false;
        }
    });
    if (!isAllow) {
        HILOGE("[OppObexServer]:Had a transfering connect, reject incoming connect");
        return ResponseCodes::OBEX_HTTP_UNAVAILABLE;
    }
    std::vector<IOppTransferFileHolder> fileHolders;
    std::shared_ptr<OppTransfer> oppTransfer = std::make_shared<OppTransfer>(
        remoteAddr, fileHolders, OPP_TRANSFER_DIRECTION_INBOUND, serverSession, this);
    oppTransfer->OnReceiveIncomingConnect(connectId, totalFileCount);
    oppTransferList_.Push(oppTransfer);

    if (oppTransferList_.Size() == 1) {
        std::shared_ptr<OppTransfer> transfer = nullptr;
        if (!oppTransferList_.Front(transfer)) {
            HILOGI("not find transfer");
            return ResponseCodes::OBEX_HTTP_OK;
        }
        bool accept = transfer->AcceptConnect();
        if (!accept) {
            return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
        }
        return ResponseCodes::OBEX_HTTP_OK;
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

void OppServiceImpl::OnTransferPositionChange(const std::string &device, size_t position)
{
    if (oppTransferList_.Empty()) {
        HILOGE("[OPP Service]:Transfer list_ is empty");
        return;
    }
    std::shared_ptr<OppTransfer> iter = nullptr;
    int index = TRANSFER_ID_MAX + 1;
    oppTransferList_.ForEach([device, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
        if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_) {
            HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
            iter = oppTransfer;
            index = oppTransfer->transferId_;
        }
    });
    if (iter == nullptr) {
        HILOGE("not find transfer");
        return;
    }
    iter->OnTransferPositionChange(position);
}

void OppServiceImpl::OnTransferStateChange(const std::string &device, int state, int reason)
{
    HILOGI("OnTransferStateChange %{public}s .", GET_ENCRYPT_STR_ADDR(device));
    if (oppTransferList_.Empty()) {
        HILOGE("transfer list is empty");
        return;
    }
    std::shared_ptr<OppTransfer> iter = nullptr;
    int index = TRANSFER_ID_MAX + 1;
    oppTransferList_.ForEach([device, &iter, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
        if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_) {
            HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
            iter = oppTransfer;
            index = oppTransfer->transferId_;
        }
    });
    if (iter == nullptr) {
        HILOGE("not find transfer");
        return;
    }
    iter->OnTransferStateChange(state, reason);
}

void OppServiceImpl::OnOpreationStateChange(const std::string &device, int state)
{
    HILOGI("OnOpreationStateChange %{public}s .", GET_ENCRYPT_STR_ADDR(device));
    if (oppTransferList_.Empty()) {
        HILOGE("transfer list is empty");
        return;
    }
    int index = TRANSFER_ID_MAX + 1;
    oppTransferList_.ForEach([device, state, &index](std::shared_ptr<OppTransfer> &oppTransfer) {
        if (oppTransfer->GetDeviceAddress() == device && index > oppTransfer->transferId_) {
            HILOGI("index is %{public}d, transferId is %{public}d", index, oppTransfer->transferId_);
            oppTransfer->GetCurrentTransferInformation().SetIsAccept(state);
        }
    });
}

int OppServiceImpl::OnReceiveIncomingFile(IOppTransferInformation info)
{
    if (oppTransferList_.Size() <= 0) {
        HILOGE("transfer list is empty");
        return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }
    std::shared_ptr<OppTransfer> inboundTransfer = nullptr;
    oppTransferList_.ForEach([&inboundTransfer](std::shared_ptr<OppTransfer> &oppTransfer) {
        HILOGI("Address=[%{public}s], direction=[%{public}d]",
            GET_ENCRYPT_STR_ADDR(oppTransfer->GetDeviceAddress()), oppTransfer->GetDirection());
        if (oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND) {
            inboundTransfer = oppTransfer;
        }
    });
    if (!inboundTransfer) {
        HILOGE("no inbound Transfer.");
        return ResponseCodes::OBEX_HTTP_UNAVAILABLE;
    }

    if (inboundTransfer->GetDeviceAddress() != info.GetDeviceAddress()) {
        HILOGE("not current device");
        return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }
    return inboundTransfer->OnReceiveIncomingFile(info);
}

void OppServiceImpl::HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    bool isAllow = true;
    oppTransferList_.ForEach([&isAllow](std::shared_ptr<OppTransfer> &oppTransfer) {
        HILOGI("Address=[%{public}s], direction=[%{public}d]",
            GET_ENCRYPT_STR_ADDR(oppTransfer->GetDeviceAddress()), oppTransfer->GetDirection());
        if (oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_INBOUND ||
            oppTransfer->GetDirection() == OPP_TRANSFER_DIRECTION_OUTBOUND) {
            isAllow = false;
        }
    });
    if (!isAllow) {
        HILOGE("transfer exist.");
        return;
    }
    OppMessage event(OPP_CONNECT_REQ_EVT);
    event.dev_ = socketDevice->mDeviceAddress;
    event.socketDevice_ = socketDevice;
    HILOGI("device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    PostEvent(event);
}

void OppServiceImpl::PostEvent(const OppMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    DoInOppThread([this, event] {this->ProcessEvent(event);});
}

void OppServiceImpl::ProcessEvent(const OppMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    switch (event.what_) {
        case OPP_CONNECT_REQ_EVT:
        case OPP_SDP_CMPL_EVT:
        case OPP_CONNECTED_EVT:
            ProcessConnectEvent(event);
            break;
        case OPP_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event);
            break;
        default:
            ProcessDefaultEvent(event);
            break;
    }
}

void OppServiceImpl::ProcessConnectEvent(const OppMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if (it != stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
        // peer device may send connect request before we remove statemachine for last connection.
        // so post this connect request, process it after we remove statemachine completely.
        PostEvent(event);
    } else if (it == stateMachines_.end() || it->second == nullptr) {
        stateMachines_[event.dev_] = std::make_unique<OppStateMachine>(event.dev_, event.socketDevice_, this);
        stateMachines_[event.dev_]->Init();
        stateMachines_[event.dev_]->ProcessMessage(event);
    } else {
        if (event.socketDevice_ != nullptr) {
            it->second->SetSocketService(event.socketDevice_);
        }
        it->second->ProcessMessage(event);
    }
}

void OppServiceImpl::ProcessDefaultEvent(const OppMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if ((it != stateMachines_.end()) && (it->second != nullptr)) {
        it->second->ProcessMessage(event);
    } else {
        HILOGE("invalid address[%{public}s]", GetEncryptAddr(event.dev_).c_str());
    }
}
void OppServiceImpl::ProcessRemoveStateMachine(const OppMessage &event)
{
    HILOGI("Address=[%{public}s],", GET_ENCRYPT_STR_ADDR(event.dev_));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    stateMachines_.erase(event.dev_);
    OnObexDisconnected(event.dev_);
}

std::vector<RawAddress> OppServiceImpl::GetDevicesByStates(const std::vector<int> &states)
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

int32_t OppServiceImpl::GetDeviceState(const RawAddress &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);

    std::string address = device.GetAddress();
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGI("the state machine is not available!");
        return stateMap_.at(OPP_STATE_DISCONNECTED);
    }

    if (it->second->GetDeviceStateInt() >= OPP_STATE_CONNECTED) {
        return stateMap_.at(OPP_STATE_CONNECTED);
    } else {
        return stateMap_.at(it->second->GetDeviceStateInt());
    }
}

int OppServiceImpl::CancelTransfer(const std::string &device)
{
    std::shared_ptr<OppTransfer> oppTransfer = nullptr;
    if (!oppTransferList_.Front(oppTransfer)) {
        HILOGE("transfer list is empty");
        return RET_BAD_STATUS;
    }
    if (oppTransfer->GetDeviceAddress() != device) {
        HILOGE("not current device");
        return RET_BAD_STATUS;
    }
    return oppTransfer->CancelTransfer();
}

void OppServiceImpl::DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc)
{
    connectSocketFunc_ = connectSocketFunc;
}

void OppServiceImpl::DllRegisterUnloadOppFunc(const std::function<bool()> &unloadOppFunc)
{
    unloadOppFunc_ = unloadOppFunc;
}

void OppServiceImpl::DllRegisterCreateOppSdpFunc(const std::function<void(const std::string &addr)> &createOppSdpFunc)
{
    createOppSdpFunc_ = createOppSdpFunc;
}

void OppServiceImpl::DllRegisterIsAllowedOppReadFunc(const std::function<bool(const std::string &addr)>
    &isAllowedOppReadFunc)
{
    isAllowedOppReadFunc_ = isAllowedOppReadFunc;
}

void OppServiceImpl::DllRegisterOppContinuousTaskFunc(const std::function<void(const std::string &action)>
        &oppContinuousTaskFunc)
{
    OppAbilityManagerUtils::SetOppContinuousTaskFunc(oppContinuousTaskFunc);
}

void OppServiceImpl::DllRegisterAclDisConnEventFunc(const std::function<void(const std::string &action)>
    &aclDisConnEventFunc)
{
    OppAbilityManagerUtils::SetAclDisConnEventFunc(aclDisConnEventFunc);
}
}  // namespace bluetooth
}  // namespace OHOS
