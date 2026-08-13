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

#ifndef OBEX_SERVER_ACTION_INTERFACE_H
#define OBEX_SERVER_ACTION_INTERFACE_H

#include <memory>
#include "obex_server_transport.h"
#include "obex_header.h"
#include "obex_request_handler.h"
#include "obex_def.h"

namespace OHOS {
namespace bluetooth {
class ObexServerActionInterface {
public:
    explicit ObexServerActionInterface(
        std::shared_ptr<ObexServerTransport> transport, std::shared_ptr<ObexRequestHandler> requestHandler)
        : transport_(transport), requestHandler_(requestHandler)
    {}
    virtual ~ObexServerActionInterface() { transport_ = nullptr; };
    virtual void SendReply(uint8_t responseCode) = 0;
    virtual bool HandleRequest(uint8_t requestType) = 0;
    uint32_t GetMaxPacketLength() { return maxPacketLength_; }
    void SetMaxPacketLength(uint32_t maxPacketLength) { maxPacketLength_ = maxPacketLength; }

protected:
    void SendResponseData(uint8_t code, const std::vector<uint8_t> &data, uint32_t headLength);
    void ParseReqeustPackageData(uint32_t readLength, uint8_t requestType);
    void SetReplyHeaderConnectionId();
    uint8_t ValidateResponseCode(uint8_t code);

protected:
    bool srmEnabled_ = false;
    bool srmActive_ = false;
    bool hasSrmResponseSent_ = false;
    ObexHeader request_;
    ObexHeader reply_;
    std::vector<uint8_t> privateOutput_{};
    std::vector<uint8_t> privateInput_{};
    bool sendBodyHeader_ = true;
    uint32_t maxPacketLength_ = 256;
    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    std::shared_ptr<ObexRequestHandler> requestHandler_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif