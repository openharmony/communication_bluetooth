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
#define LOG_TAG "bt_service_opp_transfer"
#endif

#include "opp_transfer.h"

#include <fstream>
#include "common_util.h"
#include "opp_service_impl.h"
#include "log.h"
#include "opp_message.h"
#include "bt_chr_transaction_manager.h"

namespace OHOS {
namespace bluetooth {
int OppTransfer::currentTransferId_ = 0;
int g_operatorNum = 0;

OppTransfer::OppTransfer(const std::string &address, const std::vector<IOppTransferFileHolder> fileHolders,
    int direction, std::shared_ptr<ObexServerSession> serverSession, OppServiceImpl *oppServiceImpl)
{
    HILOGI("Address=[%{public}s], direction=[%{public}d], fileSize is [%{public}lu]",
        GET_ENCRYPT_STR_ADDR(address), direction, fileHolders.size());
    incomingConnectTimer_ = std::make_unique<utility::Timer>([this] {this->IncomingFileTimeout();});
    time(&timeStamp_);
    currentTransferId_++;
    if (currentTransferId_ >= TRANSFER_ID_MAX) {
        currentTransferId_ = 0;
    }
    transferId_ = currentTransferId_;
    int fileCount = 1;
    for (auto &fileHolder : fileHolders) {
        HILOGI("transferId_ is [%{public}d] fileSize is [%{public}d]", transferId_, fileHolder.GetFileSize());
        std::shared_ptr<IOppTransferInformation> transferInfo = std::make_shared<IOppTransferInformation>();
        transferInfo->SetFilePath(fileHolder.GetFilePath());
        transferInfo->SetFileName(GetFileNameFromFilePath(fileHolder.GetFilePath(), false));
        transferInfo->SetFileType(GetMimeTypeFromFilePath(fileHolder.GetFilePath()));
        transferInfo->SetFileFd(fileHolder.GetFileFd());
        transferInfo->SetDeviceAddress(address);
        transferInfo->SetDirection(direction);
        transferInfo->SetStatus(OPP_TRANSFER_STATUS_PENDING);
        transferInfo->SetTimeStamp(static_cast<uint64_t>(timeStamp_));
        transferInfo->SetTotalBytes(static_cast<uint64_t>(fileHolder.GetFileSize()));
        transferInfo->SetIsAccept(OPP_TRANSFER_OPERATION_UNKNOWN);
        transferInfo->SetTotalCount(fileHolders.size());
        uint64_t fileSize = transferInfo->GetTotalBytes();
        if (fileSize > UINT32_MAX) {
            HILOGE("fileSize is too large, fileSize is %{public}llu", fileSize);
            BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
                TRANSACTION_RESULT_FAIL, fileCount, TRANSACTION_SCENECODE_11, fileCount);
            fileCount++;
            continue;
        }
        transferInfo->SetCurrentCount(fileCount++);
        HILOGI("fileType is %{public}s fileFd is %{public}d fileSize is %{public}llu",
            transferInfo->GetFileType().c_str(), transferInfo->GetFileFd(), fileSize);
        fileList_.Push(transferInfo);
    }
    direction_ = direction;
    address_ = address;
    serverSession_ = serverSession;
    oppServiceImpl_ = oppServiceImpl;
}

OppTransfer::~OppTransfer()
{
    isConnected_ = false;
}

int OppTransfer::GetFileNumber()
{
    return fileList_.Size();
}

std::string OppTransfer::GetDeviceAddress()
{
    return address_;
}

int OppTransfer::ConnectObex(std::shared_ptr<ObexClientConfig> obexConfig)
{
    if (direction_ == OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGE("inbond, no need send connect request");
        return RET_BAD_STATUS;
    }
    if (fileList_.Empty()) {
        HILOGE("send file list is empty");
        return RET_BAD_STATUS;
    }

    obexClient_ = std::make_shared<OppObexClient>(obexConfig, oppServiceImpl_);
    std::thread([client = obexClient_] {
        client->Connect(client);
    }).detach();
    return RET_NO_ERROR;
}

void OppTransfer::OnReceiveIncomingConnect(uint32_t connectId, int32_t totalFileCount)
{
    HILOGI("[OppTransfer]totalFileCount is %{public}d", totalFileCount);
    if (direction_ != OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGE("[OppTransfer] is outbond");
        return;
    }
    connectId_ = connectId;
    totalReceiveFileCount_ = totalFileCount;
    incomingConnectTimer_->Start(INCOMING_CONNECT_TIMEOUT_MS);
}

bool OppTransfer::AcceptConnect()
{
    HILOGI("AcceptConnect");
    if (direction_ != OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGE("[OppTransfer] is outbond");
        return false;
    }
    incomingConnectTimer_->Stop();
    OppMessage event(OPP_CONNECTED_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
    return true;
}

int OppTransfer::StartTransfer()
{
    if (direction_ == OPP_TRANSFER_DIRECTION_OUTBOUND) {
        std::shared_ptr<IOppTransferInformation> file = nullptr;
        if (!fileList_.Front(file)) {
            HILOGE("file list is null");
            return RET_BAD_STATUS;
        }
        isConnected_ = true;
        if (obexClient_ == nullptr) {
            return RET_BAD_STATUS;
        }
        curretTransferInfo_ = file;
        curretTransferInfo_->SetStatus(OPP_TRANSFER_STATUS_RUNNING);
        return obexClient_->SendFile(file);
    } else if (direction_ == OPP_TRANSFER_DIRECTION_INBOUND) {
        isConnected_ = true;
        return RET_NO_ERROR;
    } else {
        HILOGE("[OPP TRANSFER] unknow direction");
    }

    return RET_BAD_STATUS;
}

void OppTransfer::OnTransferStateChangeRunning()
{
    if (!isConnected_) {
        HILOGE("obex is not connected");
        return;
    }
    std::shared_ptr<IOppTransferInformation> file = nullptr;
    if (!fileList_.Front(file) || file == nullptr) {
        HILOGE("file list is empty");
        return;
    }
    curretTransferInfo_ = file;
    curretTransferInfo_->SetStatus(OPP_TRANSFER_STATUS_RUNNING);
    oppServiceImpl_->NotifyTransferStateChanged(*curretTransferInfo_);
}

std::shared_ptr<IOppTransferInformation> OppTransfer::GetOppTransferInformation(IOppTransferInformation info) {
    std::shared_ptr<IOppTransferInformation> infoPtr = std::make_shared<IOppTransferInformation>(info);
    infoPtr->SetTimeStamp(static_cast<uint64_t>(timeStamp_));
    infoPtr->SetDeviceName(deviceName_);
    infoPtr->SetDeviceAddress(address_);
    infoPtr->SetDirection(direction_);
    infoPtr->SetStatus(OPP_TRANSFER_STATUS_PENDING);
    infoPtr->SetCurrentCount(++curReceiveFileCount_);
    infoPtr->SetTotalCount(totalReceiveFileCount_);
    return infoPtr;
}

int OppTransfer::OnReceiveIncomingFile(IOppTransferInformation info)
{
    int ret = ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    if (direction_ != OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGE("error is outbond");
        return ret;
    }
    if (!fileList_.Empty()) {
        HILOGE("file list not empty");
        return SetIncomingFileConfirmation(false, -1);
    }
    currentTransferId_++;
    if (currentTransferId_ >= TRANSFER_ID_MAX) {
        currentTransferId_ = 0;
    }
    std::shared_ptr infoPtr = GetOppTransferInformation(info);
    fileList_.Push(infoPtr);
    HILOGI("curFileCount is %{public}d totalFileCount is %{public}d", curReceiveFileCount_, totalReceiveFileCount_);
    if (curReceiveFileCount_ > TRANSFER_FILE_COUNT_MAX) {
        return ResponseCodes::OBEX_HTTP_FORBIDDEN;
    }
    if (curReceiveFileCount_ > 1 && confirm_.load() == OPP_TRANSFER_CONFIRM_ACCEPT) {
        HILOGI("mutifile file receive notify %{public}d", curReceiveFileCount_);
        oppServiceImpl_->NotifyReceiveIncomingNoConfirmFile(*infoPtr);
        std::unique_lock<std::mutex> lock(receiveFileReadySyncMutex_);
        if (!receiveFileReadycv_.wait_for(lock, std::chrono::seconds(INCOMING_CONNECT_TIMEOUT_MS),
            [this]() { return receiveFileReady_.load(); })) {
            HILOGE("receive incomingFile timeout");
            receiveFileReady_ = false;
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        }
        HILOGI("wait here receiveFileReady_ %{public}d", receiveFileReady_.load());
        receiveFileReady_ = false;
    }

    if (confirm_.load() == OPP_TRANSFER_CONFIRM_PENDING) {
        std::unique_lock<std::mutex> lock(syncMutex);
        oppServiceImpl_->NotifyReceiveIncomingFile(*infoPtr);
        if (!cv.wait_for(lock, std::chrono::seconds(INCOMING_FILE_TIMEOUT_MS),
            [this]() { return confirm_.load() != OPP_TRANSFER_CONFIRM_PENDING; })) {
            HILOGE("receive incomingFile timeout");
            confirm_ = OPP_TRANSFER_CONFIRM_REJECT;
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_NOT_ACCEPTABLE);
        }
    }
    if (confirm_.load() == OPP_TRANSFER_CONFIRM_ACCEPT) {
        ret = ResponseCodes::OBEX_HTTP_OK;
    } else {
        ret = ResponseCodes::OBEX_HTTP_FORBIDDEN;
    }
    return ret;
}

int OppTransfer::SetIncomingFileConfirmation(bool accept, int fd)
{
    HILOGI("fd is %{public}d accept is %{public}d", fd, accept);
    if (direction_ != OPP_TRANSFER_DIRECTION_INBOUND) {
        return RET_BAD_STATUS;
    }
    if (accept && fd == -1) {
        HILOGI("wakeup begin GET_URI");
        std::shared_ptr<IOppTransferInformation> file = nullptr;
        if (!fileList_.Front(file) || file == nullptr) {
            HILOGE("file list is empty");
            return RET_BAD_STATUS;
        }
        std::map<std::string, std::string> params;
        params.emplace("fileName", file->GetFileName());
        OppAbilityManagerUtils::StartUIExtentionAbility(params);
        return RET_NO_ERROR;
    }
    if (accept && fd != -1) {
        confirm_ = OPP_TRANSFER_CONFIRM_ACCEPT;
        serverSession_->OpenFile(fd);
        HILOGI("accept fd is %{public}d", fd);
    } else if (!accept) {
        confirm_ = OPP_TRANSFER_CONFIRM_REJECT;
    }
    if (accept) {
        OnTransferStateChangeRunning();
    } else {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_BAD_REQUEST);
    }
    if (curretTransferInfo_ == nullptr || curretTransferInfo_->GetCurrentCount() == 1) {
        cv.notify_all();
    } else {
        receiveFileReady_ = true;
        receiveFileReadycv_.notify_all();
    }
    return RET_NO_ERROR;
}

void OppTransfer::OnTransferPositionChange(size_t position)
{
    HILOGI("posion is %{public}d", position);

    if (curretTransferInfo_ == nullptr) {
        HILOGE("curretTransferInfo_ is null");
        return;
    }
    if (fileList_.Empty()) {
        HILOGE("file list is empty");
        return;
    }
    curretTransferInfo_->SetCurrentBytes(static_cast<uint64_t>(position));
    oppServiceImpl_->NotifyTransferStateChanged(*curretTransferInfo_);
}

IOppTransferInformation OppTransfer::GetCurrentTransferInformation()
{
    if (curretTransferInfo_ == nullptr) {
        IOppTransferInformation ret;
        return ret;
    }
    return *curretTransferInfo_;
}

int OppTransfer::CancelTransfer()
{
    HILOGI("direction_ is %{public}d", direction_);
    if (direction_ == OPP_TRANSFER_DIRECTION_INBOUND) {
        if (serverSession_ != nullptr) {
            confirm_ = OPP_TRANSFER_CONFIRM_REJECT;
            receiveFileReady_ = true;
            receiveFileReadycv_.notify_all();
            HILOGI("receiveFileReadycv notify_all.");
            if (curretTransferInfo_ != nullptr) {
                serverSession_->Stop();
            } else {
                serverSession_->Close();
            }
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED);
            return RET_NO_ERROR;
        } else {
            HILOGE("serverSession_ is null");
            return RET_BAD_STATUS;
        }
    } else if (direction_ == OPP_TRANSFER_DIRECTION_OUTBOUND) {
        if (obexClient_ == nullptr) {
            return RET_BAD_STATUS;
        }
        if (confirm_.load() == OPP_TRANSFER_CONFIRM_PENDING) {
            return obexClient_->CancelSendFile(true);
        }
        return obexClient_->CancelSendFile(false);
    }
    HILOGE("[OPP TRANSFER] unknow direction");
    return RET_BAD_STATUS;
}

void OppTransfer::OnTransferStateChange(int state, int reason)
{
    HILOGI("OnTransferStateChange state is %{public}d ,reason is %{public}d", state, reason);
    if (state == OPP_TRANSFER_STATUS_RUNNING) {
        OnTransferStateChangeRunning();
    } else if (state == OPP_TRANSFER_STATUS_FINISH && reason == OPP_TRANSFER_RESULT_SUCCESS) {
        OnTransferStateChangeSuccess();
    } else if (state == OPP_TRANSFER_STATUS_FINISH) {
        OnTransferStateChangeFaild(reason);
    } else {
        HILOGE("error state=%{public}d", state);
    }
}

int OppTransfer::GetDirection() const
{
    return direction_;
}

void OppTransfer::SetConfirm(int confirm)
{
    confirm_ = confirm;
    cv.notify_all();
}

void OppTransfer::OnObexDisconnected()
{
    HILOGI("totalReceiveFileCount_ is %{public}d", totalReceiveFileCount_);
    isConnected_ = false;
    connectId_ = 0;
    totalReceiveFileCount_ = curReceiveFileCount_;
    std::shared_ptr<IOppTransferInformation> transferInfo = std::make_shared<IOppTransferInformation>();
    transferInfo->SetTotalCount(totalReceiveFileCount_);
    transferInfo->SetCurrentCount(curReceiveFileCount_);
    transferInfo->SetStatus(OPP_TRANSFER_STATUS_FINISH);
    transferInfo->SetResult(OPP_TRANSFER_RESULT_SUCCESS_DISCONNECT);
    oppServiceImpl_->NotifyTransferStateChanged(*transferInfo);
}

void OppTransfer::DisconnectObex() const
{
    g_operatorNum = 0;
    if (direction_ == OPP_TRANSFER_DIRECTION_INBOUND) {
        HILOGE("inbond,send disconnect request in OppService");
        return;
    }

    if (obexClient_ == nullptr) {
        return;
    }
    return obexClient_->Disconnect(true);
}

void OppTransfer::OnTransferStateChangeSuccess()
{
    HILOGI("transferStateChangeSuccess");
    if (!isConnected_) {
        HILOGE("obex is not connected");
        return;
    }
    if (curretTransferInfo_ == nullptr) {
        HILOGE("curretTransferInfo_ is null");
        return;
    }
    if (fileList_.Empty()) {
        HILOGE("file list is empty");
        return;
    }
    curretTransferInfo_->SetStatus(OPP_TRANSFER_STATUS_FINISH);
    curretTransferInfo_->SetResult(OPP_TRANSFER_RESULT_SUCCESS);
    oppServiceImpl_->NotifyTransferStateChanged(*curretTransferInfo_);
    fileList_.Erase(curretTransferInfo_);
    HILOGI("file list size is %{public}d", fileList_.Size());
    curretTransferInfo_ = nullptr;
    if (direction_ == OPP_TRANSFER_DIRECTION_OUTBOUND) {
        if (fileList_.Empty()) {
            OppMessage event(OPP_DISCONNECT_REQ_EVT);
            event.dev_ = address_;
            oppServiceImpl_->PostEvent(event);
        } else {
            ContinueSendFileList();
        }
    }
}

void OppTransfer::ContinueSendFileList()
{
    if (!isConnected_) {
        HILOGE("obex is not connected");
        return;
    }
    if (curretTransferInfo_ != nullptr) {
        HILOGE("curretTransferInfo_ is not null");
        return;
    }
    if (fileList_.Empty()) {
        HILOGE("file list is empty");
        return;
    }
    HILOGI("file list size is %{public}d", fileList_.Size());
    if (direction_ == OPP_TRANSFER_DIRECTION_OUTBOUND) {
        if (obexClient_ == nullptr) {
            HILOGE("obexClient_ is null");
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
            return;
        }
        OnTransferStateChangeRunning();
        std::shared_ptr<IOppTransferInformation> file = nullptr;
        if (!fileList_.Front(file)) {
            HILOGE("file list is empty");
            return;
        }
        if (obexClient_->SendFile(file) != RET_NO_ERROR) {
            HILOGE("send file error");
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
            return;
        }
    }
}

std::string OppTransfer::GetFirstTransferFileName()
{
    std::string fileName = "";
    std::shared_ptr<IOppTransferInformation> file = nullptr;
    if (!fileList_.Front(file)) {
        HILOGE("file list is empty");
        return fileName;
    }
    fileName = file->GetFileName();
    return fileName;
}


void OppTransfer::OnTransferStateChangeFaild(int reason)
{
    HILOGI("direction_ is %{public}d isConnected_ %{public}d", direction_, isConnected_);
    bool isNotify = false;
    if (curretTransferInfo_ != nullptr) {
        HILOGI("curretTransferInfo_ has value");
        curretTransferInfo_->SetStatus(OPP_TRANSFER_STATUS_FINISH);
        curretTransferInfo_->SetResult(reason);
        oppServiceImpl_->NotifyTransferStateChanged(*curretTransferInfo_);
        isNotify = true;
        if (!fileList_.Empty()) {
            fileList_.Erase(curretTransferInfo_);
        }
        curretTransferInfo_ = nullptr;
    }
    std::shared_ptr<IOppTransferInformation> file = nullptr;
    while (fileList_.Front(file)) {
        HILOGI("fileList_ front a file");
        auto transferInfo = file;
        transferInfo->SetStatus(OPP_TRANSFER_STATUS_FINISH);
        transferInfo->SetResult(reason);
        isNotify = true;
        oppServiceImpl_->NotifyTransferStateChanged(*transferInfo);
        fileList_.Erase(transferInfo);
    }
    if (!isNotify) {
        file = std::make_shared<IOppTransferInformation>();
        file->SetStatus(OPP_TRANSFER_STATUS_FINISH);
        file->SetResult(reason);
        file->SetDirection(OPP_TRANSFER_DIRECTION_INBOUND);
        file->SetDeviceAddress(GetDeviceAddress());
        oppServiceImpl_->NotifyTransferStateChanged(*file);
    }
    if (direction_ == OPP_TRANSFER_DIRECTION_OUTBOUND && isConnected_) {
        OppMessage event(OPP_DISCONNECTED_EVT);
        event.dev_ = address_;
        oppServiceImpl_->PostEvent(event);
    }
}

void OppTransfer::IncomingConnectTimeout()
{
    HILOGE("[OppTransfer] IncomingConnectTimeout OnObexDisconnected");
    OppMessage event(OPP_DISCONNECT_REQ_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
}

void OppTransfer::IncomingFileTimeout()
{
    HILOGE("incomingConnectTimeout reject it.");
    confirm_ = OPP_TRANSFER_CONFIRM_REJECT;
    OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_NOT_ACCEPTABLE);
    cv.notify_all();
}


}  // namespace bluetooth
}  // namespace OHOS