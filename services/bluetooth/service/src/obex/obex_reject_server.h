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

#ifndef OBEX_REJECT_SERVER_H
#define OBEX_REJECT_SERVER_H

#include "obex_request_handler.h"

namespace OHOS {
namespace bluetooth {
constexpr int DISCONNECT_DELAY_TIME_MS = 1000; // 1s
class ObexRejectServer : public ObexRequestHandler {
public:
    ObexRejectServer() = default;
    virtual ~ObexRejectServer() {}
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
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_REJECT_SERVER_H