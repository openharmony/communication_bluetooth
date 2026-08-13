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

#ifndef OBEX_REQUEST_HANDLER_H
#define OBEX_REQUEST_HANDLER_H

#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obex request handler
 *
 */
class ObexRequestHandler {
public:
    virtual int OnConnect(ObexHeader &request, ObexHeader &reply) = 0;
    virtual int OnDisconnect(ObexHeader &request, ObexHeader &reply) = 0;
    virtual int OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create) = 0;
    virtual int OnDelete(ObexHeader &request, ObexHeader &reply) = 0;
    virtual int OnAbort(ObexHeader &request, ObexHeader &reply) = 0;
    virtual int OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished) = 0;
    virtual int OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader) = 0;
    virtual void OnClose(bool isThreadStart) = 0;
    virtual void SetConnectionId(int connectionId) = 0;
    virtual int GetConnectionId() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_REQUEST_HANDLER_H