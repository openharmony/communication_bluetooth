/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AVRCP_TG_BIP_OBEX_SERVER_H
#define AVRCP_TG_BIP_OBEX_SERVER_H

#include <memory>
#include <string>
#include <vector>

#include "../obex/obex_def.h"
#include "../obex/obex_request_handler.h"

namespace OHOS {
namespace bluetooth {
class BipService;
class AvrcpBipObexServer : public ObexRequestHandler {
public:
    explicit AvrcpBipObexServer(std::string &device, const std::weak_ptr<BipService> &bipService);
    virtual ~AvrcpBipObexServer(){};

    int OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader) override;
    int OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished) override;
    int OnConnect(ObexHeader &request, ObexHeader &reply) override;
    int OnDisconnect(ObexHeader &request, ObexHeader &reply) override;
    int OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create) override;
    int OnAbort(ObexHeader &request, ObexHeader &reply) override;
    void OnClose(bool isThreadStart) override;

    int OnDelete(ObexHeader &request, ObexHeader &reply) override;
    void SetConnectionId(int connectionId) override;
    int GetConnectionId() override;

private:
    int32_t connectionId_ = -1;
    std::string deviceAddr_;
    std::weak_ptr<BipService> bipService_ = {};
    int HandleGetImageProperties(ObexHeader &request, ObexHeader &reply,
        std::vector<uint8_t> &output, bool &sendBodyHeader);
    int HandleGetImage(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_TG_BIP_OBEX_SERVER_H