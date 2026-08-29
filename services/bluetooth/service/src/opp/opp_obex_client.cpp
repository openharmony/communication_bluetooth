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
#define LOG_TAG "bt_service_opp_obex_client"
#endif

#include "opp_obex_client.h"
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include "bt_sock.h"
#include "service_util.h"
#include "interface_profile_socket.h"
#include "interface_profile_manager.h"
#include "opp_service_impl.h"
#include "opp_def.h"
#include "opp_service.h"
#include "../obex/obex_server_transport.h"
#include "../obex/obex_header.h"
#include "../obex/obex_body.h"
#include "log.h"
#include "common_util.h"
#include "thread_util.h"
#include <iostream>
#include <fstream>
#include <cstdio> // 包含 fdopen 的头文件

namespace OHOS {
namespace bluetooth {

const uint8_t MAC_ADDR_LEN = 6;

OppSendFileBodyObject::OppSendFileBodyObject(const int fd, const std::string &fileName)
{
    OpenFile(fd);
}

OppSendFileBodyObject::~OppSendFileBodyObject()
{
    Close();
}

void OppSendFileBodyObject::OpenFile(const int fd)
{
    fdFile_ = fdopen(fd, "r");
    if (fdFile_ == nullptr) {
        HILOGE("fd %{public}d fail open file", fd);
        return;
    }
    fd_ = fd;
    int fseekResult = fseek(fdFile_, 0L, SEEK_END);
    if (fseekResult < 0) {
        HILOGE("fseek file error");
        fclose(fdFile_);
        fdFile_ = nullptr;
        return;
    }
    fileSize_ = ftell(fdFile_);
    rewind(fdFile_);
    if (fileSize_ <= 0) {
        HILOGE("fd %{public}d open file file, read size is %{public}d", fd, fileSize_);
        fclose(fdFile_);
        fdFile_ = nullptr;
        return;
    }
    HILOGI("open file fd %{public}d success, file size is %{public}d", fd, fileSize_);
}

size_t OppSendFileBodyObject::Read(uint8_t *buf, size_t bufLen)
{
    size_t readSize = bufLen;
    size_t fileSize = static_cast<size_t>(fileSize_);
    if (fileSize < fileSendSize_) {
        HILOGE("something error, fileSize less than fileSendSize");
        return 0;
    }
    size_t remainSize = fileSize - fileSendSize_;
    if (remainSize < readSize) {
        readSize = remainSize;
    }
    if (fileSize == 0) {
        HILOGI("read file empty");
        return 0;
    }
    ssize_t bytesRead = read(fd_, reinterpret_cast<char*>(buf), readSize);
    HILOGI("read file bytes is %{public}zd", bytesRead);
    if (bytesRead == -1) {
        HILOGE("read file error fileSendSize_ is %{public}d", fileSendSize_);
        return 0;
    }
    fileSendSize_ += readSize;
    return readSize;
}

bool OppSendFileBodyObject::IsReadFinished()
{
    if (fileSize_ < 0) {
        HILOGE("something error, fileSize less than 0");
        return false;
    }
    size_t fileSize = static_cast<size_t>(fileSize_);
    if (fileSize < fileSendSize_) {
        HILOGE("something error, fileSize less than fileSendSize_");
        return false;
    }
    size_t remainSize = fileSize - fileSendSize_;
    if (remainSize == 0) {
        return true;
    }
    return false;
}

size_t OppSendFileBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    return bufLen;
}

size_t OppSendFileBodyObject::GetFileSize() const
{
    return fileSize_;
}

size_t OppSendFileBodyObject::GetFileSendSize() const
{
    return fileSendSize_;
}

int OppSendFileBodyObject::Close()
{
    if (fdFile_ != nullptr) {
        HILOGI("close fdFile");
        fclose(fdFile_);
        fdFile_ = nullptr;
    }
    return RET_NO_ERROR;
}

OppObexClient::OppObexClient(std::shared_ptr<ObexClientConfig> obexConfig, OppServiceImpl *oppServiceImpl)
{
    obexConfig_ = obexConfig;
    address_ = obexConfig->addr;
    isObexConnected_ = false;
    oppServiceImpl_ = oppServiceImpl;
}

void OppObexClient::Connect(std::shared_ptr<OppObexClient> obexClient)
{
    std::unique_lock<std::mutex> lock(socketFdLock_);
    if (socketFd_ != -1) {
        HILOGI("socketFd_ is %{public}d connected, so return", socketFd_);
        return;
    }
    int securityFlags = 0;
    CHECK_AND_RETURN_LOG(obexConfig_, "obexConfig_ is null");
    HILOGI("deviceAddr %{public}s, l2capPsm %{public}d, channel %{public}d.", GET_ENCRYPT_STR_ADDR(obexConfig_->addr),
        obexConfig_->l2capPsm, obexConfig_->rfCommChannel);
    CHECK_AND_RETURN_LOG(oppServiceImpl_, "oppServiceImpl_ is null");
    std::string fileName = oppServiceImpl_->GetFirstTransferFileName(obexConfig_->addr);
    std::thread([fileName] {OppAbilityManagerUtils::WakeUpOppSendUIAbility(fileName);}).detach();
    CHECK_AND_RETURN_LOG(oppServiceImpl_->connectSocketFunc_, "connect socket is null");
    SocketType type;
    auto uuid = Uuid::ConvertFromString(obexConfig_->serviceUUID);
    if (obexConfig_->l2capPsm > 0 && obexConfig_->l2capPsm < OPP_MAX_PSM) {
        HILOGI("l2cap socket is create");
        type = SocketType::TYPE_L2CAP;
        socketFd_ = oppServiceImpl_->connectSocketFunc_(obexConfig_->addr,
            uuid, securityFlags, 1, obexConfig_->l2capPsm);
    } else if (obexConfig_->rfCommChannel > 0 && obexConfig_->rfCommChannel < OPP_MAX_PSM) {
        HILOGI("rfComm socket is create");
        type = SocketType::TYPE_RFCOMM;
        socketFd_ = oppServiceImpl_->connectSocketFunc_(obexConfig_->addr,
            uuid, securityFlags, 0, obexConfig_->rfCommChannel);
    } else {
        HILOGE("l2capPsm and rfCommChannel not found.");
        return;
    }

    if (socketFd_ <= 0) {
        HILOGE("connect error");
        return;
    }
    HILOGI("socketFd_ is %{public}d", socketFd_);
    auto device = RecvSocketDevice(type);
    if (!device) {
        lock.unlock();
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
            TRANSACTION_RESULT_FAIL, obexConfig_->fileCount, TRANSACTION_SCENECODE_9, obexConfig_->fileCount);
        Disconnect(false);
        return;
    }
    connected_ = true;
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(device);
    clientSession_ = std::make_shared<ObexClientSession>(transport, obexClient);

    ObexHeader header;
    header.SetCount(obexConfig_->fileCount);
    clientSession_->Connect(header, connectHeader_);
}

static RawAddress ConvertRfcommAddr(OHOS::bluetooth::RawAddress &addr)
{
    uint8_t addrBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    addr.ConvertToUint8(addrBytes);
    uint8_t reversedBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    for (int i = 0; i < MAC_ADDR_LEN; i++) {
        reversedBytes[i] = addrBytes[MAC_ADDR_LEN - 1 - i];
    }
    OHOS::bluetooth::RawAddress newAddr =
        OHOS::bluetooth::RawAddress::ConvertToString(reversedBytes);
    return newAddr;
}

std::shared_ptr<ObexSocketDevice> OppObexClient::RecvSocketDevice(SocketType socketType)
{
    HILOGI("socketType is %{public}d", socketType);
    int32_t channel = 0;
    auto ret = recv(socketFd_, &channel, sizeof(channel), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[sock] recv error");
        return nullptr;
    }
    SockConnectSignal cs;
    ret = recv(socketFd_, &cs, sizeof(SockConnectSignal), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[sock] recv error");
        return nullptr;
    }
    HILOGI("cs.maxTxPacketSize is %{public}d cs.maxRxPacketSize is %{public}d",
        cs.maxTxPacketSize, cs.maxRxPacketSize);
    RawAddress newAddr = ConvertRfcommAddr(cs.bdAddr);

    std::shared_ptr<ObexSocketDevice> socketDevice = nullptr;
    if (socketType == SocketType::TYPE_L2CAP) {
        socketDevice = std::make_shared<ObexSocketDevice>(
            socketFd_, cs.maxTxPacketSize, cs.maxRxPacketSize, socketType, newAddr.GetAddress());
    } else if (socketType == SocketType::TYPE_RFCOMM) {
        socketDevice = std::make_shared<ObexSocketDevice>(
            socketFd_, MAX_PACKET_SIZE, MAX_PACKET_SIZE, socketType, newAddr.GetAddress());
    } else {
        HILOGE("[sock] socketType error");
    }

    return socketDevice;
}

void OppObexClient::Disconnect(bool withObexReq)
{
    HILOGI("withObexReq is %{public}d", withObexReq);
    if (withObexReq && clientSession_ != nullptr) {
        ObexHeader header;
        header.SetConnectionId(connectHeader_.GetConnectionId());
        clientSession_->DisConnect(header, connectHeader_);
        clientSession_->FreeSendObject();
        clientSession_->CloseTransport();
    } else {
        //socketFd close manager at ObexSocketDevice(socketFd be copied to there to use)
        std::unique_lock<std::mutex> lock(socketFdLock_);
        HILOGI("OPP_DISCONNECTED_EVT publish");
        OppMessage event(OPP_DISCONNECTED_EVT);
        event.dev_ = address_;
        oppServiceImpl_->PostEvent(event);
        oppServiceImpl_->RemoveTransfer(address_);
        clientSession_ = nullptr;
        socketFd_ = -1;
    }
    fileObject_ = nullptr;
}

void OppObexClient::DisconnectLocal(bool withObexReq)
{
    HILOGI("withObexReq is %{public}d", withObexReq);
    if (withObexReq && clientSession_ != nullptr) {
        sendAbort_ = true;
        clientSession_->FreeSendObject();
        clientSession_->CloseTransport();
    } else {
        //socketFd close manager at ObexSocketDevice(socketFd be copied to there to use)
        std::unique_lock<std::mutex> lock(socketFdLock_);
        clientSession_ = nullptr;
        socketFd_ = -1;
    }
}

int OppObexClient::SendFile(std::shared_ptr<IOppTransferInformation> fileInfo)
{
    int ret = RET_BAD_STATUS;
    if (fileInfo == nullptr) {
        HILOGE("SendFile file is nullptr");
        return RET_BAD_STATUS;
    }
    fileObject_ = std::make_shared<OppSendFileBodyObject>(fileInfo->GetFileFd(), fileInfo->GetFileName());

    std::shared_ptr<ObexHeader> bodyHeader = std::make_shared<ObexHeader>();

    if (clientSession_ == nullptr) {
        return RET_BAD_STATUS;
    }
    uint32_t mtu = clientSession_->GetTransportMaxTxPackageSize();
    bodyHeader->SetConnectionId(connectHeader_.GetConnectionId());
    std::shared_ptr<ObexClientSendObject> sendObject = clientSession_->CreateSendObject(bodyHeader,
        fileObject_, mtu, fileInfo->GetTotalBytes());
    status_ = OPP_OBEX_STATUS_WAITING_CONTINUE;
    clientSession_->isSupportSrm_ = false;
    oppServiceImpl_->OnTransferPositionChange(address_, 0);
    std::thread([sendObject, fileInfo, clientSession = clientSession_] {
        OppObexClient::SendFileHeader(sendObject, fileInfo, clientSession);
    }).detach();
    return RET_NO_ERROR;
}

void OppObexClient::SendFileHeader(std::shared_ptr<ObexClientSendObject> sendObject,
    std::shared_ptr<IOppTransferInformation> fileInfo, std::shared_ptr<ObexClientSession> clientSession)
{
    if (fileInfo == nullptr) {
        HILOGE("SendFile file is nullptr");
        return;
    }
    ObexHeader request;
    uint16_t headerSize = HEADER_PREFIX_LENGTH * 2;
    std::string fileType = fileInfo->GetFileType();
    std::string fileName = fileInfo->GetFileName();
    request.SetHeader(TYPE, std::vector<uint8_t>(fileType.begin(), fileType.end()));
    headerSize += (HEADER_PREFIX_LENGTH + fileType.size());
    request.SetName(fileName);
    headerSize += (HEADER_PREFIX_LENGTH + 1 + (fileName.size() + 1) * PER_WORD_LEN);
    uint64_t length = fileInfo->GetTotalBytes();
    HILOGI("begin is %{public}d, send file length %{public}llu", fileInfo->GetIsAccept(), length);
    request.SetLength(length);
    request.SetSrm(true);
    headerSize += (WORD_HEADER_LEN * PER_WORD_LEN + BYTE_HEADER_LEN);

    std::vector<uint8_t> bodyData {};
    bool convertResult = true;
    if (sendObject != nullptr) {
        headerSize += HEADER_PREFIX_LENGTH;
        convertResult = sendObject->ConvertSendFileToVector(headerSize, bodyData);
    }
    if (!convertResult) {
        HILOGE("read file error");
        return;
    }

    if (clientSession == nullptr) {
        HILOGI("clientSession_ is nullptr");
        return;
    }
    ObexHeader replyHeader;
    replyHeader.SetRecvTimeoutMs(INCOMING_FILE_TIMEOUT_MS);
    clientSession->Put(request, replyHeader, false, bodyData);
}

void OppObexClient::OnConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    if (clientSession_ == nullptr) {
        return;
    }
    std::string address = clientSession_->GetRemoteAddress();
    if (address_ != address) {
        HILOGE("not current device");
        return;
    }
    if (responseCode != OBEX_HTTP_OK) {
        HILOGE("connect response is %{public}0x", responseCode);
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_SEND,
            TRANSACTION_RESULT_FAIL, obexConfig_->fileCount, TRANSACTION_SCENECODE_10, obexConfig_->fileCount);
        oppServiceImpl_->RemoveTransfer(address_);
        return;
    }
    int32_t connectId = reply.GetConnectionId();
    isObexConnected_ = true;
    connectionId_ = connectId;
    OppMessage event(OPP_CONNECTED_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
    return;
}

void OppObexClient::OnDisConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    if (clientSession_ == nullptr) {
        return;
    }
    std::string address = clientSession_->GetRemoteAddress();
    if (address_ != address) {
        HILOGE("end, not current device");
        return;
    }
    connectionId_ = 0;
    isObexConnected_ = false;
    HILOGI("OPP_DISCONNECTED_EVT publish");
    OppMessage event(OPP_DISCONNECTED_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
    return;
}

void OppObexClient::OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("responseCode: %{public}0x", responseCode);
    if (isWaitingSendAbort_) {
        isProcessing_ = false;
        SendAbortRequest();
        return;
    }
    SetBusy(false);
    OnActionCompleted(reply, responseCode);
}

void OppObexClient::OnAbortResp()
{
    HILOGI("OnAbortResp");
    sendAbort_ = true;
    oppServiceImpl_->OnTransferStateChange(
        address_, OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_CANCELED);
}

void OppObexClient::OnActionCompleted(ObexHeader &reply, uint8_t responseCode)
{
    if (clientSession_ == nullptr || fileObject_ == nullptr) {
        HILOGE("clientSession_ or fileObject_ is nullptr");
        return;
    }
    HILOGI("responseCode is %{public}0x status_ is %{public}d isSupportSrm_  is %{public}d",
        responseCode, status_, clientSession_->isSupportSrm_);
    std::string address = clientSession_->GetRemoteAddress();
    if (address_ != address) {
        HILOGE("end, not current device");
        return;
    }
    if ((responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE)) ||
        (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_OK))) {
        if (status_ == OPP_OBEX_STATUS_WAITING_END) {
            status_ = OPP_OBEX_STATUS_IDLE;
            HILOGI("transfer success");
            oppServiceImpl_->OnTransferPositionChange(address_,
                static_cast<OppSendFileBodyObject *>(fileObject_.get())->GetFileSendSize());
            oppServiceImpl_->OnTransferStateChange(address_, OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_SUCCESS);
        } else if (status_ == OPP_OBEX_STATUS_WAITING_CONTINUE && clientSession_->isSupportSrm_ == true) {
            this->SendFileBody();
        } else if (status_ == OPP_OBEX_STATUS_WAITING_CONTINUE && clientSession_->isSupportSrm_ == false &&
            isFirstFileBody_ == true) {
            this->SendFileOneBody();
        }
        return;
    } else if (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_LENGTH_REQUIRED)) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_LENGTH);
    } else if (status_ == OPP_OBEX_STATUS_WAITING_CONTINUE) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_CONNECTION_FAILED);
    } else if (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_FORBIDDEN)) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_BAD_REQUEST);
    } else if (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE)) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_NOT_ACCEPTABLE);
    } else if (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_UNSUPPORTED_TYPE)) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNSUPPORTED_TYPE);
    } else if (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_UNAVAILABLE)) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_CONNECTION_FAILED);
    } else {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
    }
    Disconnect(true);
}

void OppObexClient::OnTransferStateChangeFaild(int reason)
{
    if (reason == OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED) {
        HILOGI("receive local cancel disconnectLocal");
        DisconnectLocal(true);
    }
    oppServiceImpl_->OnTransferStateChange(address_, OPP_TRANSFER_STATUS_FINISH, reason);
}

void OppObexClient::OnOperationActionSync(uint8_t responseCode)
{
    if ((responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE)) ||
        (responseCode == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_OK))) {
        oppServiceImpl_->OnOpreationStateChange(address_, OPP_TRANSFER_OPERATION_ACCEPT);
    } else {
        oppServiceImpl_->OnOpreationStateChange(address_, OPP_TRANSFER_OPERATION_FORBIDDEN);
    }
}

void OppObexClient::SetBusy(bool isBusy)
{
    if (clientSession_ == nullptr) {
        return;
    }
    if (clientSession_->IsBusy() != isBusy) {
        clientSession_->SetBusy(isBusy);
        HILOGI("ObexBusy=%{public}d", isBusy);
    }
}

int OppObexClient::CancelSendFile(bool isImmediately)
{
    HILOGI("isImmediately is %{public}d", isImmediately);
    sendAbort_ = true;
    if (isImmediately) {
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED);
    }
    return RET_NO_ERROR;
}

void OppObexClient::OnBusy(bool isBusy)
{
    if (clientSession_ == nullptr) {
        return;
    }
    auto device = clientSession_->GetRemoteAddress();
    HILOGI("device=%{public}s", GET_ENCRYPT_STR_ADDR(device));
    isBusy_ = isBusy;
    if (!isBusy_) {
        std::lock_guard<std::mutex> lock(mutexBusyChanged_);
        cvWaitBusyChanged_.notify_all();
    }
    HILOGI("end, isBusy=%{public}d", isBusy ? 1 : 0);
}

void OppObexClient::SendFileBody()
{
    if (clientSession_ == nullptr || fileObject_ == nullptr) {
        HILOGE("end, clientSession_ or fileObject_ is null");
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        return;
    }
    std::shared_ptr<ObexClientSendObject> sendObject = clientSession_->GetSendObject();
    if (sendObject == nullptr) {
        HILOGE("end, sendObject is null");
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        return;
    }
    HILOGE("sendObject->IsDone() is %{public}d", sendObject->IsDone());
    while (!sendObject->IsDone()) {
        if (isBusy_) {
            std::unique_lock<std::mutex> lock(mutexBusyChanged_);
            cvWaitBusyChanged_.wait(lock, [this] { return !isBusy_; });
        }
        if (sendAbort_) {
            Abort();
            clientSession_->FreeSendObject();
            sendAbort_ = false;
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_CANCELED);
            return;
        }
        std::vector<uint8_t> bodyData = sendObject->GetSendFileBodyData();
        if (!sendAbort_ && sendObject->IsDone()) {
            status_ = OPP_OBEX_STATUS_WAITING_END;
        }
        if (!sendAbort_ && sendObject->GetBodyHeaderReq() == nullptr) {
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
            return;
        }
        if (!sendAbort_) {
            oppServiceImpl_->OnTransferPositionChange(
                address_, static_cast<OppSendFileBodyObject *>(fileObject_.get())->GetFileSendSize());
        }
        clientSession_->Put(*(sendObject->GetBodyHeaderReq()), replyHeader_, false, bodyData);
    }
    status_ = OPP_OBEX_STATUS_WAITING_END;
    clientSession_->Put(*(sendObject->GetBodyHeaderReq()), replyHeader_, sendObject->IsDone(), {});
}

void OppObexClient::SendFileOneBody()
{
    HILOGI("isFirstFileBody_ %{public}d", isFirstFileBody_.load());
    if (isFirstFileBody_.load() == false) {
        return;
    }
    if (clientSession_ == nullptr || fileObject_ == nullptr) {
        HILOGE("clientSession_ or fileObject_ is null");
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        return;
    }
    std::shared_ptr<ObexClientSendObject> sendObject = clientSession_->GetSendObject();
    if (sendObject == nullptr) {
        HILOGE("sendObject is null");
        OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        return;
    }
    replyHeader_.SetResponseCode(static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE));
    while (isFirstFileBody_.load() == true || (!sendObject->IsDone() &&
        replyHeader_.GetResponseCode() == static_cast<uint8_t>(ResponseCodes::OBEX_HTTP_CONTINUE))) {
        if (isBusy_) {
            std::unique_lock<std::mutex> lock(mutexBusyChanged_);
            cvWaitBusyChanged_.wait(lock, [this] { return !isBusy_; });
        }
        if (sendAbort_) {
            Abort();
            clientSession_->FreeSendObject();
            sendAbort_ = false;
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_CANCELED);
            return;
        }
        std::vector<uint8_t> bodyData = sendObject->GetSendFileBodyData();
        if (!sendAbort_ && sendObject->IsDone()) {
            status_ = OPP_OBEX_STATUS_WAITING_END;
        }
        if (!sendAbort_ && sendObject->GetBodyHeaderReq() == nullptr) {
            OnTransferStateChangeFaild(OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
            return;
        }
        if (!sendAbort_) {
            oppServiceImpl_->OnTransferPositionChange(
                address_, static_cast<OppSendFileBodyObject *>(fileObject_.get())->GetFileSendSize());
        }
        isFirstFileBody_ = false;
        clientSession_->Put(*(sendObject->GetBodyHeaderReq()), replyHeader_, sendObject->IsDone(), bodyData);
    }
    isFirstFileBody_ = true;
}

void OppObexClient::Abort()
{
    HILOGI("Call Abort");
    if (isWaitingSendAbort_ || isAbortSended_) {
        HILOGE("Abort is processing!");
        return;
    }
    isAbortSended_ = false;
    if (isProcessing_) {
        if (clientSession_ == nullptr) {
            return;
        }
        isAbortSended_ = false;
        switch (clientSession_->GetLastOpeId()) {
            case static_cast<uint8_t>(OBEX_OPCODE_PUT):
            case static_cast<uint8_t>(OBEX_OPCODE_PUT_FINAL):
            case static_cast<uint8_t>(OBEX_OPCODE_GET):
            case static_cast<uint8_t>(OBEX_OPCODE_GET_FINAL):
                isWaitingSendAbort_ = true;
                break;
            default:
                isWaitingSendAbort_ = false;
                break;
        }
        return;
    }
    SendAbortRequest();
}

void OppObexClient::SendAbortRequest()
{
    if (!CheckBeforeRequest(OBEX_OPCODE_ABORT)) {
        return;
    }
    if (clientSession_ == nullptr) {
        return;
    }
    ObexHeader inputHeader;
    ObexHeader outputHeader;
    inputHeader.SetConnectionId(connectHeader_.GetConnectionId());
    clientSession_->Abort(inputHeader, outputHeader);
    isWaitingSendAbort_ = false;
    isAbortSended_ = true;
}

bool OppObexClient::CheckBeforeRequest(uint8_t opeId)
{
    if (isProcessing_) {
        HILOGE("Another operation is being processed, please try again later.");
        return false;
    }
    bool checkConnect = true;
    bool needConnected = true;
    switch (opeId) {
        case static_cast<uint8_t>(OBEX_OPCODE_CONNECT):
            needConnected = false;
            break;
        case static_cast<uint8_t>(OBEX_OPCODE_SESSION):
            checkConnect = false;  // Session's connect check is in CheckBeforeSession
            break;
        case static_cast<uint8_t>(OBEX_OPCODE_DISCONNECT):
            if (!isObexConnected_) {
                HILOGE("Already Disconnected from the server.");
                return false;
            }
            break;
        default:
            break;
    }
    if (checkConnect) {
        if (needConnected && !isObexConnected_) {
            HILOGE(
                "Please connect first. Before obex connected, only SESSION(Create) and CONNECT Operation can "
                "be called.");
            return false;
        }
        if (!needConnected && isObexConnected_) {
            HILOGE("Already connected to server.");
            return false;
        }
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS