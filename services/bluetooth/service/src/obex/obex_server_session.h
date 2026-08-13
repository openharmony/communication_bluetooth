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

#ifndef OBEX_SERVER_SESSION_H
#define OBEX_SERVER_SESSION_H

#include <atomic>
#include <thread>
#include <future>
#include "obex_body.h"
#include "obex_server_transport.h"
#include "obex_request_handler.h"
#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
class ObexServerReceivedObject {
public:
    ObexServerReceivedObject(ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> writer);
    virtual ~ObexServerReceivedObject() = default;
    void AppendBody(const uint8_t *body, const uint16_t bodyLen);
    std::shared_ptr<ObexBodyObject> &GetWriter();
    std::unique_ptr<ObexHeader> CreateReceivedHeader();

private:
    std::shared_ptr<ObexBodyObject> writer_ = nullptr;
    size_t receivedSize_ = 0;
};
class ObexServerSession {
public:
    ObexServerSession(std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> handler);
    ~ObexServerSession();
    void Start();
    void Stop();
    void Run(std::promise<void> promise);
    std::shared_ptr<ObexServerReceivedObject> GetReceivedObject();
    std::string GetRemoteAddress();
    void FreeReceivedObject();
    std::shared_ptr<ObexServerReceivedObject> CreateReceivedObject(ObexHeader &firstReq,
        std::shared_ptr<ObexBodyObject> &writer);
    void UpdateHandler(std::shared_ptr<ObexRequestHandler> handler);
    void OpenFile(const int fileFd);
    void Close();

private:
    bool HandleConnectRequest();
    bool HandleDefaultRequest();
    bool HandleAbortRequest();
    bool HandlePutRequest(uint8_t requestType);
    bool HandleGetRequest(uint8_t requestType);
    bool HandleSetPathRequest();
    bool HandleDisconnectRequest();

private:
    std::unique_ptr<std::thread> thread_ {nullptr};
    std::atomic_bool start_ = ATOMIC_FLAG_INIT;
    std::shared_ptr<ObexServerReceivedObject> receivedObject_ = nullptr;
    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    std::shared_ptr<ObexRequestHandler> requestHandler_ = nullptr;

    uint32_t maxPacketLength_ = 256;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_SERVER_SESSION_H
