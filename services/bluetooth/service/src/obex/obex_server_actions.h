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

#ifndef OBEX_SERVER_ACTIONS_H
#define OBEX_SERVER_ACTIONS_H

#include "obex_server_action_interface.h"

namespace OHOS {
namespace bluetooth {
// for connection action
class ObexServerConnectAction : public ObexServerActionInterface {
public:
    ObexServerConnectAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerConnectAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    uint32_t ReadRequestPackageLength();
};

// for disconnection action
class ObexServerDisconnectAction : public ObexServerActionInterface {
public:
    ObexServerDisconnectAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerDisconnectAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    uint32_t ReadRequestPackageLength();
};

// for setpath action
class ObexServerSetPathAction : public ObexServerActionInterface {
public:
    ObexServerSetPathAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerSetPathAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    uint32_t ReadRequestPackageLength();

private:
    bool backup_ = false;
    bool create_ = false;
};

// for abort action
class ObexServerAbortAction : public ObexServerActionInterface {
public:
    ObexServerAbortAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerAbortAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    uint32_t ReadRequestPackageLength();
};

// for default action
class ObexServerDefaultAction : public ObexServerActionInterface {
public:
    ObexServerDefaultAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerDefaultAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    uint32_t ReadRequestPackageLength();
};

// for get or get final action
class ObexServerGetAction : public ObexServerActionInterface {
public:
    ObexServerGetAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerGetAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    bool HandleClientGetContinue();
    bool HandleNextGetOp();
    bool NeedSubcontractPackage();
    void SendSubcontractPackage();
    bool SendBigHeader(const std::vector<uint8_t> &headerData);
    bool SendPackage(uint8_t code, uint32_t sendLength);
    void FillSendData(uint8_t code, uint32_t sendLength, uint32_t headSize, std::vector<uint8_t> &sendData);
    uint32_t ReadRequestPackageLength();
    bool HandleSendReplyContinue(bool ignoreReceive);

private:
    std::atomic<bool> isAbort_ = false;
    bool requestFinished_ = false;
    uint32_t hasSentLength_ = 0;
};

// for put or put final action
class ObexServerPutAction : public ObexServerActionInterface {
public:
    ObexServerPutAction(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : ObexServerActionInterface(transport, requestHandler)
    {}
    ~ObexServerPutAction() override {}

    void SendReply(uint8_t responseCode) override;
    bool HandleRequest(uint8_t requestType) override;

private:
    bool HandleClientPutContinue();
    bool HandleNextPutOp();
    void SendContinueReply();
    uint32_t ReadRequestPackageLength();
    bool SendPackage(uint8_t code);
    bool HandleSendReplyContinue(bool hasNextResponse);
    bool ParseReplyContinueRequestType(uint8_t &requestType, uint32_t &requestPacketLength);

private:
    bool requestFinished_ = false;
    std::atomic_bool hasOkResponse_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif