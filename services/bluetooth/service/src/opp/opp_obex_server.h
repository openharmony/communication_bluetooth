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

#ifndef OPP_OBEX_SERVER_H
#define OPP_OBEX_SERVER_H

#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>

#include "../obex/obex_def.h"
#include "../obex/obex_request_handler.h"
#include "obex_server_session.h"

namespace OHOS {
namespace bluetooth {
class OppServiceImpl;
class OppReceiveFileBodyObject : public ObexBodyObject {
public:
    explicit OppReceiveFileBodyObject(const std::string &address,
        OppServiceImpl *oppServiceImpl, int32_t totalSize);
    OppReceiveFileBodyObject() = default;
    ~OppReceiveFileBodyObject() override;
    size_t Read(uint8_t *buf, size_t bufLen) override;
    bool IsReadFinished() override;
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    int Close() override;
    void OpenFile(const int fd) override;

private:
    std::ofstream ofs_ {};
    FILE* fdFile_ = nullptr;
    int fd_ = -1;
    size_t fileReceiveSize_ = 0;
    size_t totalSize_ = 0;
    std::string address_;
    OppServiceImpl *oppServiceImpl_ = nullptr;
};


class OppObexServer : public ObexRequestHandler {
public:
    explicit OppObexServer(std::string &device, OppServiceImpl *oppServiceImpl);
    virtual ~OppObexServer() {}

    int OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader) override;
    int OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished) override;
    int OnConnect(ObexHeader &request, ObexHeader &reply) override;
    int OnDisconnect(ObexHeader &request, ObexHeader &reply) override;
    int OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create) override;
    int OnDelete(ObexHeader &request, ObexHeader &reply) override;
    int OnAbort(ObexHeader &request, ObexHeader &reply) override;
    void OnClose(bool isThreadStart) override;
    void SetConnectionId(int connectionId) override;
    int GetConnectionId() override;
    void SetObexServerSession(std::shared_ptr<ObexServerSession> serverSession);

private:
    int ReceiveFileHeader(ObexHeader &request, ObexHeader &reply);
    int ReceiveFileBody(ObexHeader &request, ObexHeader &reply, bool requestFinished);
    OppServiceImpl *oppServiceImpl_ = nullptr;
    std::shared_ptr<ObexServerSession> serverSession_ = nullptr;
    std::string deviceAddr_;
    std::string currentPath_ = "";
};
} // namespace bluetooth
} // namespace OHOS
#endif // OPP_OBEX_SERVER_H