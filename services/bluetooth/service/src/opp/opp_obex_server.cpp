/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_opp_obex_server"
#endif

#include "opp_obex_server.h"
#include "opp_file_utils.h"

#include <dirent.h>
#include "opp_service_impl.h"
#include "log.h"
#include "string_ex.h"
#include "classic_config.h"

namespace OHOS {
namespace bluetooth {

OppReceiveFileBodyObject::OppReceiveFileBodyObject(const std::string &address,
    OppServiceImpl *oppServiceImpl, int32_t totalSize)
{
    address_ = address;
    oppServiceImpl_ = oppServiceImpl;
    totalSize_ = static_cast<size_t>(totalSize);
}

OppReceiveFileBodyObject::~OppReceiveFileBodyObject()
{
    Close();
}

void OppReceiveFileBodyObject::OpenFile(const int fd)
{
    if (fd == -1) {
        HILOGE("fd %{public}d is invaild", fd);
        return;
    }
    fdFile_ = fdopen(fd, "w");
    fd_ = fd;
    if (fdFile_ == nullptr) {
        HILOGE("fd %{public}d fail open file failed", fd);
    } else {
        HILOGI("fd %{public}d opened.", fd);
    }
    return;
}

size_t OppReceiveFileBodyObject::Read(uint8_t *buf, size_t bufLen)
{
    return bufLen;
}

bool OppReceiveFileBodyObject::IsReadFinished()
{
    return false;
}

size_t OppReceiveFileBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    size_t writeSize = 0;
    if (buf == nullptr || bufLen == 0) {
        return writeSize;
    }

    if (oppServiceImpl_ == nullptr) {
        return writeSize;
    }
    if (fileReceiveSize_ > totalSize_) {
        HILOGE("fileReceiveSize_ more than totalSize_ => CancelTransfer");
        oppServiceImpl_->CancelTransfer(address_);
        return writeSize;
    }

    if (fdFile_ != nullptr && fd_ != -1) {
        auto buffer = reinterpret_cast<const char*>(buf);
        write(fd_, buffer, bufLen);
        ofs_.write(buffer, bufLen);
        fileReceiveSize_ += bufLen;
        writeSize = bufLen;
        oppServiceImpl_->OnTransferPositionChange(address_, fileReceiveSize_);
        HILOGI("write file bufLen=%{public}zu", bufLen);
    } else {
        oppServiceImpl_->CancelTransfer(address_);
        HILOGE("file open failed fd_ is %{public}d", fd_);
    }
    return writeSize;
}

int OppReceiveFileBodyObject::Close()
{
    if (fd_ != -1) {
        HILOGI("close fd %{public}d", fd_);
        fclose(fdFile_);
        fdFile_ = nullptr;
    }
    return RET_NO_ERROR;
}

OppObexServer::OppObexServer(std::string &device, OppServiceImpl *oppServiceImpl) :  oppServiceImpl_(oppServiceImpl),
    deviceAddr_(device)
{}

void OppObexServer::SetObexServerSession(std::shared_ptr<ObexServerSession> serverSession)
{
    serverSession_ = serverSession;
}

int OppObexServer::OnConnect(ObexHeader &request, ObexHeader &reply)
{
    OppAbilityManagerUtils::SubscribTransactionChrEvent();
    int32_t connectId = request.GetConnectionId();
    int32_t totalFileCount = request.GetCount();
    int reponseCode = oppServiceImpl_->OnReceiveIncomingConnect(connectId, totalFileCount, deviceAddr_, serverSession_);
    return reponseCode;
}

void OppObexServer::OnClose(bool isThreadStart)
{
    HILOGI("isThreadStart is %{public}d", isThreadStart);
    if (!isThreadStart) {
        return;
    }
    oppServiceImpl_->OnTransferStateChange(
        serverSession_->GetRemoteAddress(), OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED);
}

int OppObexServer::OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

int OppObexServer::OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished)
{
    int ret = ResponseCodes::OBEX_HTTP_OK;
    int32_t connectId = request.GetConnectionId();
    reply.SetConnectionId(connectId);
    if (serverSession_ == nullptr) {
        HILOGE("serverSession_ is nullptr");
        return ret;
    }
    if (serverSession_->GetReceivedObject() == nullptr) {
        ret = ReceiveFileHeader(request, reply);
    }
    //user accept
    if (ret == ResponseCodes::OBEX_HTTP_OK) {
        ret = ReceiveFileBody(request, reply, requestFinished);
    }
    HILOGI("OnPut ret is %{public}x", ret);
    return ret;
}

int OppObexServer::ReceiveFileHeader(ObexHeader &request, ObexHeader &reply)
{
    if (!oppServiceImpl_->isAllowedOppReadFunc_(deviceAddr_)) {
        HILOGI("Receive file not allowed");
        return ResponseCodes::OBEX_HTTP_UNSUPPORTED_TYPE;
    }
    int ret = ResponseCodes::OBEX_HTTP_OK;
    std::string type = request.GetType();
    std::string name = request.GetName();
    uint32_t length = 0;
    bool retGetLen = request.GetLength(length);
    HILOGI("type %{public}s, length %{public}u", type.c_str(), length);
    if (!retGetLen) {
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_OPP_RECEIVE,
            TRANSACTION_RESULT_FAIL, 1, TRANSACTION_SCENECODE_8, 1);
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    std::string fileName = GetFileNameFromFilePath(name, true);
    if (fileName.length() > MAX_FILE_NAME_LENGTH) {
        HILOGE("fileName length too long, fileName length is : %{public}zu", fileName.length());
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    std::shared_ptr<ObexBodyObject> writer = std::make_shared<OppReceiveFileBodyObject>(deviceAddr_,
        oppServiceImpl_, length);
    if (serverSession_ == nullptr) {
        HILOGE("serverSession_ is nullptr");
        return ret;
    }
    HILOGI("receive new Object");
    std::shared_ptr<ObexServerReceivedObject> receivedObject = serverSession_->CreateReceivedObject(request, writer);
    IOppTransferInformation info;
    info.SetDeviceAddress(serverSession_->GetRemoteAddress());
    info.SetFilePath(fileName);
    info.SetFileName(fileName);
    info.SetFileType(type);
    info.SetTotalBytes(length);
    ret = oppServiceImpl_->OnReceiveIncomingFile(info);
    HILOGI("onReceiveIncomingFile ret is %{public}x", ret);
    return ret;
}

int OppObexServer::ReceiveFileBody(ObexHeader &request, ObexHeader &reply, bool requestFinished)
{
    int ret = ResponseCodes::OBEX_HTTP_OK;
    if (serverSession_== nullptr || serverSession_->GetReceivedObject() == nullptr) {
        oppServiceImpl_->OnTransferStateChange(
            deviceAddr_, OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        return OBEX_HTTP_BAD_REQUEST;
    }
    std::shared_ptr<ObexServerReceivedObject> receivedObject = serverSession_->GetReceivedObject();
    uint16_t bodyLen = 0;
    std::vector<uint8_t> body = request.GetBody(bodyLen);
    if (bodyLen == 0 && !requestFinished) {
        HILOGI("received non end bodyLen %{public}d", bodyLen);
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    if (!body.empty()) {
        HILOGI("server Received Body From Request! bodyLen is %{public}d", bodyLen);
        receivedObject->AppendBody(&(body[0]), bodyLen);
    }
    HILOGI("received body bodyLen %{public}d", bodyLen);
    if (requestFinished) {
        std::vector<uint8_t> fbody = request.GetEndOfBody(bodyLen);
        if (!fbody.empty()) {
            HILOGI("[OPP OBEX SERVER]:Server Received End-Body From Request! bodyLen is %{public}d", bodyLen);
            receivedObject->AppendBody(&(fbody[0]), bodyLen);
        }
        reply.SetEndOfBody(fbody, 0);

        OppReceiveFileBodyObject *bodyObject = static_cast<OppReceiveFileBodyObject *>(
            receivedObject->GetWriter().get());
        if (bodyObject != nullptr) {
            serverSession_->FreeReceivedObject();
            oppServiceImpl_->OnTransferStateChange(
                serverSession_->GetRemoteAddress(),  OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_SUCCESS);
            return ret;
        }
        serverSession_->FreeReceivedObject();
        oppServiceImpl_->OnTransferStateChange(
            serverSession_->GetRemoteAddress(), OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_UNKNOWN);
        ret = ResponseCodes::OBEX_HTTP_OK;
    } else {
        reply.SetSrm(true);
        ret = ResponseCodes::OBEX_HTTP_CONTINUE;
    }
    return ret;
}

int OppObexServer::OnDisconnect(ObexHeader &request, ObexHeader &reply)
{
    OppMessage event(OPP_DISCONNECTED_EVT);
    event.dev_ = deviceAddr_;
    oppServiceImpl_->PostEvent(event);
    return ResponseCodes::OBEX_HTTP_OK;
}

int OppObexServer::OnDelete(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

int OppObexServer::OnAbort(ObexHeader &request, ObexHeader &reply)
{
    if (serverSession_ == nullptr) {
        HILOGE("serverSession_ is nullptr");
        return ResponseCodes::OBEX_HTTP_OK;
    }
    oppServiceImpl_->OnTransferStateChange(
        serverSession_->GetRemoteAddress(), OPP_TRANSFER_STATUS_FINISH, OPP_TRANSFER_RESULT_ERROR_CANCELED);
    return ResponseCodes::OBEX_HTTP_OK;
}

int OppObexServer::OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

void OppObexServer::SetConnectionId(int connectionId)
{
}

int OppObexServer::GetConnectionId()
{
    return ResponseCodes::OBEX_HTTP_OK;
}
} // namespace bluetooth
} // namespace OHOS