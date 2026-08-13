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
#define LOG_TAG "bt_service_obex_server_action"
#endif

#include "obex_server_action_interface.h"
#include "log.h"
#include "common_util.h"
#include <set>

namespace OHOS {
namespace bluetooth {
void ObexServerActionInterface::SendResponseData(uint8_t code, const std::vector<uint8_t> &data, uint32_t headLength)
{
    uint32_t totalLength = headLength;
    std::vector<uint8_t> sendData{};
    totalLength += data.size();
    sendData.push_back(code);
    sendData.push_back((totalLength >> BIT_LEN_8) & 0xFF);
    sendData.push_back(totalLength & 0xFF);

    if (!data.empty()) {
        sendData.insert(sendData.end(), data.begin(), data.end());
    }
    transport_->Write(sendData.data(), sendData.size());
}

__attribute__((no_sanitize("cfi")))
void ObexServerActionInterface::ParseReqeustPackageData(uint32_t readLength, uint8_t requestType)
{
    std::vector<uint8_t> readData(readLength, 0);
    int32_t recvLen = 0;
    HILOGI("recvLen = %{public}d. readLength = %{public}d.", recvLen, readLength);
    while (recvLen < static_cast<int32_t>(readLength)) {
        int32_t readLen = transport_->Read(readData.data() + recvLen, readLength - recvLen);
        if (readLen <= 0) {
            HILOGE("transport Read failed, readLen = %{public}d", readLen);
            return;
        }
        recvLen += readLen;
    }
    std::vector<uint8_t> newBody {};
    request_.SetHeader(BODY, newBody);
    request_.SetHeader(END_OF_BODY, newBody);
    request_.ParseHeaderValue(readData);

    if (requestHandler_) {
        int connectionId = 1;  // Default value
        if (requestHandler_->GetConnectionId() != -1) {
            connectionId = request_.GetConnectionId();
        }
        requestHandler_->SetConnectionId(connectionId);
    }

    auto body = request_.GetHeader(BODY);
    if (body.empty()) {
        body = request_.GetHeader(END_OF_BODY);
    }
    privateInput_.insert(privateInput_.end(), body.begin(), body.end());

    auto srmRaw = request_.GetHeader(SINGLE_RESPONSE_MODE);
    if (srmRaw.empty() || (srmRaw[0] == 0) || !transport_->IsSrmSupported()) {
        HILOGI("srm not enable");
        return;
    }

    srmEnabled_ = true;

    if (requestType == OBEX_OPCODE_GET || requestType == OBEX_OPCODE_GET_FINAL || requestType == OBEX_OPCODE_PUT) {
        auto srmParamRaw = request_.GetHeader(SINGLE_RESPONSE_MODE_PARAMETER);
        if (srmParamRaw.empty() || srmParamRaw[0] != 1) {  // no srm wait
            srmActive_ = true;
        } else {
            request_.SetHeader(SINGLE_RESPONSE_MODE_PARAMETER, std::move(std::vector<uint8_t>{}));
        }
    }
}

__attribute__((no_sanitize("cfi")))
void ObexServerActionInterface::SetReplyHeaderConnectionId()
{
    if (requestHandler_) {
        int32_t id = requestHandler_->GetConnectionId();
        if (id != -1) {
            reply_.SetConnectionId(id);
        }
    }
}

uint8_t ObexServerActionInterface::ValidateResponseCode(uint8_t code)
{
    if ((code >= ResponseCodes::OBEX_HTTP_CONTINUE) && (code <= ResponseCodes::OBEX_HTTP_PARTIAL)) {
        return code;
    }
    if ((code >= ResponseCodes::OBEX_HTTP_MULT_CHOICE) && (code <= ResponseCodes::OBEX_HTTP_USE_PROXY)) {
        return code;
    }
    if ((code >= ResponseCodes::OBEX_HTTP_BAD_REQUEST) && (code <= ResponseCodes::OBEX_HTTP_UNSUPPORTED_TYPE)) {
        return code;
    }
    if ((code >= ResponseCodes::OBEX_HTTP_INTERNAL_ERROR) && (code <= ResponseCodes::OBEX_HTTP_VERSION)) {
        return code;
    }
    if ((code >= ResponseCodes::OBEX_DATABASE_FULL) && (code <= ResponseCodes::OBEX_DATABASE_LOCKED)) {
        return code;
    }
    return ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
}
}  // namespace bluetooth
}  // namespace OHOS