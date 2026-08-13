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

#ifndef OBEX_CLIENT_SESSION_H
#define OBEX_CLIENT_SESSION_H

#include <atomic>
#include <thread>
#include <future>
#include "obex_body.h"
#include "obex_server_transport.h"
#include "obex_request_handler.h"
#include "obex_response_handler.h"
#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
class ObexClientSendObject {
public:
    ObexClientSendObject(std::shared_ptr<ObexHeader> bodyHeader,
        std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen);
    virtual ~ObexClientSendObject() = default;
    std::vector<uint8_t> GetSendFileBodyData();
    std::shared_ptr<ObexHeader> GetBodyHeaderReq();
    bool ConvertSendFileToVector(const uint16_t &headerLength, std::vector<uint8_t> &outvector);
    bool IsDone() const;
    void SetBusy(bool isBusy);
    bool IsBusy() const;
    uint64_t fileLen_ = 0;

private:
    bool isDone_ = false;
    bool isBusy_ = false;
    uint16_t mtu_ = 0;
    uint64_t sendIndex_ = 0;
    uint64_t sendPacketLen_ = 0;
    std::shared_ptr<ObexHeader> bodyHeaderReq_ = nullptr;
    std::shared_ptr<ObexBodyObject> bodyReader_ = nullptr;
};
class ObexClientSession {
public:
    ObexClientSession(std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexResponseHandler> handler);
    ~ObexClientSession();

    void Connect(ObexHeader &header, ObexHeader &outHeader);
    void DisConnect(ObexHeader &header, ObexHeader &outHeader);
    void Put(ObexHeader &header, ObexHeader &outHeader, bool isFinal, std::vector<uint8_t> bodyData);
    void Abort(ObexHeader &header, ObexHeader &outHeader);
    void SendRequest(uint8_t code, const std::vector<uint8_t> &package, ObexHeader &outHeader);
    int ReadResponse(uint8_t code, uint8_t &responseCode, uint32_t &length, ObexHeader &outHeader);
    int HandleResponse(uint8_t code, uint8_t &responseCode, ObexHeader &outHeader);
    void SendEvent(ObexHeader &request, const std::string &eventData);
    void SetBusy(bool isBusy);
    bool IsBusy();
    void CloseTransport();
    std::shared_ptr<ObexClientSendObject> CreateSendObject(std::shared_ptr<ObexHeader> bodyHeader,
        std::shared_ptr<ObexBodyObject> bodyReader, uint32_t mtu, uint64_t fileLen);
    std::shared_ptr<ObexClientSendObject> GetSendObject();
    std::string GetRemoteAddress();
    uint32_t GetMaxTxPackageSize();
    uint32_t GetTransportMaxTxPackageSize();
    void FreeSendObject();
    uint8_t GetLastOpeId();
    void SetLastOpeId(uint8_t lastOpeId);
    uint8_t GetLastRespCd();
    void SetLastRespCd(uint8_t lastRespCd);
    bool isSupportSrm_ = false;
private:
    void HandleConnectResult(uint32_t length, ObexHeader &outHeader);
    uint8_t SendEventRequest(uint8_t code, const std::vector<uint8_t> &eventData);
private:
    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    bool isBusy_ = false;
    uint32_t maxTxPackageSize_ = 0;
    uint8_t lastOpeId_ = 0;
    uint8_t lastRespCd_ = 0;
    std::shared_ptr<ObexClientSendObject> sendObject_ = nullptr;
    std::weak_ptr<ObexResponseHandler> responseHandler_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_SERVER_SESSION_H
