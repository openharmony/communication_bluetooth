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
#define LOG_TAG "bt_service_obex_server_session"
#endif

#include "common_util.h"
#include "log.h"
#include "obex_def.h"
#include "obex_server_actions.h"
#include "obex_server_action_interface.h"
#include "obex_server_session.h"

namespace OHOS {
namespace bluetooth {
ObexServerSession::ObexServerSession(
    std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> handler)
    : transport_(transport), requestHandler_(handler)
{
    HILOGI("ObexServerSession Create");
}

ObexServerSession::~ObexServerSession()
{
    HILOGI("ObexServerSession destroy");
}

void ObexServerSession::UpdateHandler(std::shared_ptr<ObexRequestHandler> handler)
{
    if (handler == nullptr) {
        HILOGI("handler is nullptr.");
        return;
    }
    requestHandler_ = handler;
}

void ObexServerSession::Start()
{
    HILOGI("ObexServerSession Start");
    if (!start_) {
        start_ = true;
        std::promise<void> startPromise;
        std::future<void> startFuture = startPromise.get_future();
        thread_ = std::make_unique<std::thread>([this,
            promise = std::move(startPromise)]() mutable {this->Run(std::move(promise));});
        startFuture.wait();
    }
}

void ObexServerSession::Stop()
{
    HILOGI("ObexServerSession Stop");
    if (start_) {
        start_ = false;
        transport_->Close();
        if (thread_ && thread_->joinable()) {
            thread_->join();
            transport_ = nullptr;
            requestHandler_ = nullptr;
            thread_ = nullptr;
        }
    }
}

__attribute__((no_sanitize("cfi")))
void ObexServerSession::Close()
{
    HILOGI("ObexServerSession Close");
    if (requestHandler_ != nullptr) {
        requestHandler_->OnClose(start_);
    }
}

void ObexServerSession::Run(std::promise<void> promise)
{
    promise.set_value();
    bool isOk = true;
    while (isOk && start_) {
        uint8_t requestType;
        int ret = transport_->Read(&requestType, 1);
        if (ret <= 0) {
            HILOGE("transport Read failed, ret = %{public}d", ret);
            break;
        }
        HILOGI("transport requestType = %{public}x", requestType);
        switch (requestType) {
            case OBEX_OPCODE_CONNECT:
                isOk = HandleConnectRequest();
                break;
            case OBEX_OPCODE_DISCONNECT:
                isOk = HandleDisconnectRequest();
                break;
            case OBEX_OPCODE_GET:
            case OBEX_OPCODE_GET_FINAL:
                isOk = HandleGetRequest(requestType);
                break;
            case OBEX_OPCODE_PUT:
            case OBEX_OPCODE_PUT_FINAL:
                isOk = HandlePutRequest(requestType);
                break;
            case OBEX_OPCODE_SETPATH:
                isOk = HandleSetPathRequest();
                break;
            case OBEX_OPCODE_ABORT:
                isOk = HandleAbortRequest();
                break;
            default:
                isOk = HandleDefaultRequest();
                break;
        }
    }
    Close();
}

bool ObexServerSession::HandleConnectRequest()
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerConnectAction>(transport_, requestHandler_);
    bool ret = action->HandleRequest(OBEX_OPCODE_CONNECT);
    maxPacketLength_ = action->GetMaxPacketLength();
    return ret;
}

bool ObexServerSession::HandleDisconnectRequest()
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerDisconnectAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(OBEX_OPCODE_DISCONNECT);
}

bool ObexServerSession::HandleSetPathRequest()
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerSetPathAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(OBEX_OPCODE_SETPATH);
}

bool ObexServerSession::HandleAbortRequest()
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerAbortAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(OBEX_OPCODE_ABORT);
}

bool ObexServerSession::HandleDefaultRequest()
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerDefaultAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(OBEX_OPCODE_UNIMPLEMENTED);
}

bool ObexServerSession::HandleGetRequest(uint8_t requestType)
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerGetAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(requestType);
}

bool ObexServerSession::HandlePutRequest(uint8_t requestType)
{
    std::unique_ptr<ObexServerActionInterface> action =
        std::make_unique<ObexServerPutAction>(transport_, requestHandler_);
    action->SetMaxPacketLength(maxPacketLength_);
    return action->HandleRequest(requestType);
}

void ObexServerSession::FreeReceivedObject()
{
    receivedObject_ = nullptr;
}

std::shared_ptr<ObexServerReceivedObject> ObexServerSession::CreateReceivedObject(
    ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> &writer)
{
    receivedObject_ = std::make_shared<ObexServerReceivedObject>(firstReq, writer);
    return receivedObject_;
}

void ObexServerSession::OpenFile(const int fileFd)
{
    HILOGI("openFile fd is %{public}d", fileFd);
    if (fileFd == -1) {
        HILOGE("fd is invaild");
        return;
    }
    if (receivedObject_ == nullptr) {
        HILOGE("receivedObject_ is nullptr");
        return;
    }
    if (receivedObject_->GetWriter() == nullptr) {
        HILOGE("writer is nullptr");
        return;
    }
    receivedObject_->GetWriter()->OpenFile(fileFd);
}

ObexServerReceivedObject::ObexServerReceivedObject(ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> writer)
    : writer_(writer)
{
}

void ObexServerReceivedObject::AppendBody(const uint8_t *body, const uint16_t bodyLen)
{
    writer_->Write(body, bodyLen);
    receivedSize_ += bodyLen;
}

std::shared_ptr<ObexBodyObject> &ObexServerReceivedObject::GetWriter()
{
    return writer_;
}

std::unique_ptr<ObexHeader> ObexServerReceivedObject::CreateReceivedHeader()
{
    return nullptr;
}


std::string ObexServerSession::GetRemoteAddress()
{
    return transport_->GetRemoteAddress();
}

std::shared_ptr<ObexServerReceivedObject> ObexServerSession::GetReceivedObject()
{
    return receivedObject_;
}
}  // namespace bluetooth
}  // namespace OHOS