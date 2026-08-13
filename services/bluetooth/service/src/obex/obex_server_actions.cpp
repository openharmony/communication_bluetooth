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
#define LOG_TAG "bt_service_obex_server_actions"
#endif

#include "obex_server_actions.h"
#include "log.h"
#include "log_utils.h"

namespace OHOS {
namespace bluetooth {
// ****************connect****************
__attribute__((no_sanitize("cfi")))
bool ObexServerConnectAction::HandleRequest(uint8_t requestType)
{
    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (maxPacketLength_ > transport_->GetMaxTxPacketSize()) {
        maxPacketLength_ = transport_->GetMaxTxPacketSize();
    }
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        response = ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE;
    } else {
        if (requestPacketLength > HEAD_CONNECT_LEN) {
            ParseReqeustPackageData(requestPacketLength - HEAD_CONNECT_LEN, requestType);
        }
        if (requestHandler_) {
            response = ValidateResponseCode(requestHandler_->OnConnect(request_, reply_));
        }
    }
    SendReply(response);
    return true;
}

void ObexServerConnectAction::SendReply(uint8_t responseCode)
{
    HILOGI("responseCode: %{public}u.", responseCode);
    std::vector<uint8_t> headerData{};
    SetReplyHeaderConnectionId();
    reply_.GenerateHeaderValue(headerData);
    if (HEAD_CONNECT_LEN + headerData.size() > maxPacketLength_) {
        headerData.clear();
        responseCode = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    }

    std::vector<uint8_t> sendData{};
    sendData.push_back(0x10);  // Version
    sendData.push_back(0x00);  // Flags
    uint32_t maxRxLength = transport_->GetMaxRxPacketSize();
    if (maxRxLength > maxPacketLength_) {
        maxRxLength = maxPacketLength_;
    }
    sendData.push_back((maxRxLength >> BIT_LEN_8) & 0xFF);
    sendData.push_back(maxRxLength & 0xFF);
    sendData.insert(sendData.end(), headerData.begin(), headerData.end());

    SendResponseData(responseCode, sendData, HEAD_DEFAULT_LEN);
}

uint32_t ObexServerConnectAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_CONNECT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_CONNECT_LEN - 1);  // code已经读过了，不包含code
    if (ret != (HEAD_CONNECT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    uint8_t version = dataBuf[pos++];
    uint8_t flags = dataBuf[pos++];
    maxPacketLength_ = dataBuf[pos++];
    maxPacketLength_ = (maxPacketLength_ << BIT_LEN_8) + dataBuf[pos++];
    HILOGI("ret =%{public}d, Length = %{public}u, maxLength_ = %{public}d, ver = %{public}u, flags = %{public}u", ret,
           packetLength, maxPacketLength_, version, flags);

    if (maxPacketLength_ > MAX_PACKET_SIZE_INT) {
        maxPacketLength_ = MAX_PACKET_SIZE_INT;
    }
    return packetLength;
}

// ****************disconnect****************
__attribute__((no_sanitize("cfi")))
bool ObexServerDisconnectAction::HandleRequest(uint8_t requestType)
{
    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        response = ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE;
    } else {
        if (requestPacketLength > HEAD_DISCONNECT_LEN) {
            ParseReqeustPackageData(requestPacketLength - HEAD_DISCONNECT_LEN, requestType);
        }
        if (requestHandler_) {
            response = ValidateResponseCode(requestHandler_->OnDisconnect(request_, reply_));
        }
    }
    SendReply(response);
    return true;
}

void ObexServerDisconnectAction::SendReply(uint8_t responseCode)
{
    std::vector<uint8_t> headerData{};
    SetReplyHeaderConnectionId();
    reply_.GenerateHeaderValue(headerData);
    if (HEAD_DISCONNECT_LEN + headerData.size() > maxPacketLength_) {
        headerData.clear();
        responseCode = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    }

    std::vector<uint8_t> sendData{};
    sendData.insert(sendData.end(), headerData.begin(), headerData.end());

    SendResponseData(responseCode, sendData, HEAD_DISCONNECT_LEN);
}

uint32_t ObexServerDisconnectAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_DEFAULT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_DEFAULT_LEN - 1);
    if (ret != (HEAD_DEFAULT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    return packetLength;
}

// ****************setpath****************
__attribute__((no_sanitize("cfi")))
bool ObexServerSetPathAction::HandleRequest(uint8_t requestType)
{
    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        response = ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE;
    } else {
        if (requestPacketLength > HEAD_SETPATH_LEN) {
            ParseReqeustPackageData(requestPacketLength - HEAD_SETPATH_LEN, requestType);
        }
        if (requestHandler_) {
            response = ValidateResponseCode(requestHandler_->OnSetPath(request_, reply_, backup_, create_));
        }
    }
    SendReply(response);
    return true;
}

void ObexServerSetPathAction::SendReply(uint8_t responseCode)
{
    std::vector<uint8_t> headerData{};
    SetReplyHeaderConnectionId();
    reply_.GenerateHeaderValue(headerData);
    if (HEAD_DEFAULT_LEN + headerData.size() > maxPacketLength_) {
        headerData.clear();
        responseCode = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    }

    std::vector<uint8_t> sendData{};
    sendData.insert(sendData.end(), headerData.begin(), headerData.end());

    SendResponseData(responseCode, sendData, HEAD_DEFAULT_LEN);
}

uint32_t ObexServerSetPathAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_SETPATH_LEN];
    int ret = transport_->Read(dataBuf, HEAD_SETPATH_LEN - 1);
    if (ret != (HEAD_SETPATH_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }

    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    uint8_t flags = dataBuf[pos++];
    uint8_t constants = dataBuf[pos++];
    HILOGI("ret =%{public}d, packetLength = %{public}u, flags = %{public}u, constants = %{public}u", ret, packetLength,
           flags, constants);
    if (flags & 0x01) {
        backup_ = true;
    }
    if (flags & 0x02) {
        create_ = false;
    }
    return packetLength;
}

// ****************abort****************
__attribute__((no_sanitize("cfi")))
bool ObexServerAbortAction::HandleRequest(uint8_t requestType)
{
    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        response = ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE;
    } else {
        uint8_t dataBuf = 0;
        for (uint32_t i = HEAD_ABORT_LEN; i < requestPacketLength; i++) {
            transport_->Read(&dataBuf, 1);
        }
        if (requestHandler_) {
            response = ValidateResponseCode(requestHandler_->OnAbort(request_, reply_));
        }
    }
    SendReply(response);
    return true;
}

void ObexServerAbortAction::SendReply(uint8_t responseCode)
{
    SendResponseData(responseCode, std::vector<uint8_t>{}, HEAD_ABORT_LEN);
}

uint32_t ObexServerAbortAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_DEFAULT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_DEFAULT_LEN - 1);
    if (ret != (HEAD_DEFAULT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    return packetLength;
}

// ****************default****************
__attribute__((no_sanitize("cfi")))
bool ObexServerDefaultAction::HandleRequest(uint8_t requestType)
{
    uint8_t response;
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        response = ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE;
    } else {
        uint8_t dataBuf = 0;
        for (uint32_t i = HEAD_DEFAULT_LEN; i < requestPacketLength; i++) {
            transport_->Read(&dataBuf, 1);
        }
    }
    SendReply(ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED);
    return true;
}

void ObexServerDefaultAction::SendReply(uint8_t responseCode)
{
    SendResponseData(responseCode, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
}

uint32_t ObexServerDefaultAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_DEFAULT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_DEFAULT_LEN - 1);
    if (ret != (HEAD_DEFAULT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    return packetLength;
}

// ****************GET****************
__attribute__((no_sanitize("cfi")))
bool ObexServerGetAction::HandleRequest(uint8_t requestType)
{
    requestFinished_ = (requestType == OBEX_OPCODE_GET_FINAL);
    uint32_t packetLength = ReadRequestPackageLength();
    if (packetLength > maxPacketLength_) {
        SendResponseData(ResponseCodes::OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (packetLength > HEAD_DEFAULT_LEN) {
        ParseReqeustPackageData(packetLength - HEAD_DEFAULT_LEN, requestType);
    }
    while (!requestFinished_) {
        if (!HandleClientGetContinue()) {
            return false;
        }
    }
    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    if (requestHandler_) {
        response = ValidateResponseCode(requestHandler_->OnGet(request_, reply_, privateOutput_, sendBodyHeader_));
    }
    SendReply(response);
    return true;
}

uint32_t ObexServerGetAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_DEFAULT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_DEFAULT_LEN - 1);
    if (ret != (HEAD_DEFAULT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    return packetLength;
}

bool ObexServerGetAction::HandleClientGetContinue()
{
    bool srmRespSendPending = false;
    bool ignoreSend = false;
    SetReplyHeaderConnectionId();
    if (srmEnabled_ && !hasSrmResponseSent_) {
        srmRespSendPending = true;
        reply_.SetHeader(SINGLE_RESPONSE_MODE, std::vector<uint8_t>{1});
    }

    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    reply_.Clear();
    if (HEAD_DEFAULT_LEN + headerData.size() > maxPacketLength_) {
        HILOGE("some error must happended, return");
        return false;
    }

    if ((srmActive_) && (hasSrmResponseSent_)) {
        ignoreSend = true;
    }
    if (srmRespSendPending) {
        hasSrmResponseSent_ = true;
    }
    if (!ignoreSend) {
        std::vector<uint8_t> sendData{};
        sendData.insert(sendData.end(), headerData.begin(), headerData.end());
        SendResponseData(OBEX_HTTP_CONTINUE, sendData, HEAD_DEFAULT_LEN);
    }

    return HandleNextGetOp();
}

bool ObexServerGetAction::HandleNextGetOp()
{
    uint8_t requestType;
    if (transport_->Read(&requestType, sizeof(uint8_t)) <= 0) {
        return false;
    }

    // handle next get action
    if (requestType == OBEX_OPCODE_ABORT) {
        HILOGE("abort recieved, return");
        SendResponseData(OBEX_HTTP_OK, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (requestType != OBEX_OPCODE_GET && requestType != OBEX_OPCODE_GET_FINAL) {
        HILOGE("invalid action");
        SendResponseData(OBEX_HTTP_BAD_REQUEST, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }

    requestFinished_ = (requestType == OBEX_OPCODE_GET_FINAL);
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        SendResponseData(OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if ((requestPacketLength > HEAD_DEFAULT_LEN) || (srmEnabled_ && requestPacketLength == HEAD_DEFAULT_LEN)) {
        ParseReqeustPackageData(requestPacketLength - HEAD_DEFAULT_LEN, requestType);
    }
    return true;
}

void ObexServerGetAction::SendReply(uint8_t responseCode)
{
    if (NeedSubcontractPackage()) {
        SendSubcontractPackage();
    }
    if (isAbort_.load() == true) {
        HILOGE("received abort");
        isAbort_ = false;
        return;
    }
    if (privateOutput_.size() >= hasSentLength_) {
        SendPackage(responseCode, privateOutput_.size() - hasSentLength_);
    } else {
        SendPackage(responseCode, 0);
    }
}

bool ObexServerGetAction::NeedSubcontractPackage()
{
    SetReplyHeaderConnectionId();
    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    HILOGD("privateOutputSize: %{public}u, headerDataSize: %{public}u", privateOutput_.size(), headerData.size());
    if (privateOutput_.size() + headerData.size() + HEAD_AND_RSPDATA_LEN > maxPacketLength_) {
        return true;
    }
    return false;
}

void ObexServerGetAction::SendSubcontractPackage()
{
    SetReplyHeaderConnectionId();
    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    uint32_t contractTrunkLength = maxPacketLength_ - headerData.size() - HEAD_AND_RSPDATA_LEN;
    while (privateOutput_.size() > hasSentLength_ + contractTrunkLength) {
        HILOGD("privateOutputSize: %{public}u, hasSentLength_: %{public}u, contractTrunkLength: %{public}u",
               privateOutput_.size(), hasSentLength_, contractTrunkLength);
        if (!SendPackage(ResponseCodes::OBEX_HTTP_CONTINUE, contractTrunkLength)) {
            return;
        }
    }
}

bool ObexServerGetAction::SendBigHeader(const std::vector<uint8_t> &headerData)
{
    uint32_t end = 0;
    uint32_t start = 0;
    while (end < headerData.size()) {
        end = ObexHeader::FindHeaderEnd(headerData, start, maxPacketLength_ - HEAD_DEFAULT_LEN);
        if (end == 0) {
            privateOutput_.clear();
            SendResponseData(ResponseCodes::OBEX_HTTP_INTERNAL_ERROR, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
            return false;
        }
        std::vector<uint8_t> sendHeader{};
        sendHeader.insert(sendHeader.end(), headerData.begin() + start, headerData.begin() + end);
        SendResponseData(ResponseCodes::OBEX_HTTP_CONTINUE, sendHeader, HEAD_DEFAULT_LEN);
        start = end;
    }
    return true;
}

bool ObexServerGetAction::SendPackage(uint8_t code, uint32_t sendLength)
{
    HILOGD("code: %{public}x, sendLength: %{public}u", code, sendLength);
    bool srmRespSendPending = false;
    SetReplyHeaderConnectionId();
    if (srmEnabled_ && !hasSrmResponseSent_) {
        HILOGI("srm enabled, set srm mode");
        srmRespSendPending = true;
        reply_.SetHeader(SINGLE_RESPONSE_MODE, std::vector<uint8_t>{1});
    }

    std::vector<uint8_t> sendData{};
    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    reply_.Clear();

    // 如果header本身就超大了，先把header发送出去
    if ((HEAD_DEFAULT_LEN + headerData.size()) > maxPacketLength_) {
        if (!SendBigHeader(headerData)) {
            HILOGE("header is too huge, error");
            return false;
        }
        return true;
    } else {
        HILOGD("insert header data ok");
        sendData.insert(sendData.end(), headerData.begin(), headerData.end());
    }

    if (srmRespSendPending) {
        hasSrmResponseSent_ = true;
    }
    FillSendData(code, sendLength, headerData.size(), sendData);
    SendResponseData(code, sendData, HEAD_DEFAULT_LEN);
    if (code == ResponseCodes::OBEX_HTTP_CONTINUE) {
        return HandleSendReplyContinue(srmActive_);
    }
    return true;
}

void ObexServerGetAction::FillSendData(uint8_t code, uint32_t sendLength, uint32_t headSize,
                                       std::vector<uint8_t> &sendData)
{
    bool finalBitSet = (code == ResponseCodes::OBEX_HTTP_OK);
    if (finalBitSet || ((headSize + MIN_BUFFER_LEN < maxPacketLength_) && sendLength > 0)) {
        uint32_t bodyLength = sendLength;
        if (bodyLength > (maxPacketLength_ - headSize - HEAD_AND_RSPDATA_LEN)) {
            bodyLength = maxPacketLength_ - headSize - HEAD_AND_RSPDATA_LEN;
        }
        auto start = privateOutput_.begin() + hasSentLength_;
        auto end = privateOutput_.begin() + hasSentLength_ + bodyLength;
        std::vector<uint8_t> bodyToSend(start, end);
        hasSentLength_ += bodyLength;
        bodyLength += HEAD_DEFAULT_LEN;
        if (sendBodyHeader_) {
            finalBitSet ? sendData.push_back(END_OF_BODY) : sendData.push_back(BODY);
            sendData.push_back(bodyLength >> BIT_LEN_8);
            sendData.push_back(bodyLength);
            sendData.insert(sendData.end(), bodyToSend.begin(), bodyToSend.end());
        }
    }
    if (finalBitSet && sendBodyHeader_ && sendLength == 0) {
        sendData.push_back(END_OF_BODY);
        sendData.push_back(HEAD_DEFAULT_LEN >> BIT_LEN_8);
        sendData.push_back(HEAD_DEFAULT_LEN);
    }
}

bool ObexServerGetAction::HandleSendReplyContinue(bool ignoreReceive)
{
    if (ignoreReceive) {
        HILOGI_TIME_LIMIT(__func__, "ignoreReceive return");
        return true;
    }
    uint8_t requestType;
    int32_t ret = transport_->Read(&requestType, sizeof(uint8_t));
    if (ret <= 0) {
        HILOGE("read return <= 0, error");
        return false;
    }

    // handle next get action
    if (requestType == OBEX_OPCODE_ABORT) {
        isAbort_ = true;
        uint32_t packetLength = ReadRequestPackageLength();
        HILOGE("abort recieved, length is %{public}d", packetLength);
        if (packetLength > HEAD_DEFAULT_LEN) {
            ParseReqeustPackageData(packetLength - HEAD_DEFAULT_LEN, requestType);
        }
        SendResponseData(OBEX_HTTP_OK, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (requestType != OBEX_OPCODE_GET && requestType != OBEX_OPCODE_GET_FINAL) {
        HILOGE("invalid action, requestType : %{public}x", requestType);
        SendResponseData(OBEX_HTTP_BAD_REQUEST, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }

    requestFinished_ = (requestType == OBEX_OPCODE_GET_FINAL);
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        SendResponseData(OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if ((requestPacketLength > HEAD_DEFAULT_LEN) || (srmEnabled_ && requestPacketLength == HEAD_DEFAULT_LEN)) {
        ParseReqeustPackageData(requestPacketLength - HEAD_DEFAULT_LEN, requestType);
    }
    return true;
}

// ****************PUT****************
__attribute__((no_sanitize("cfi")))
bool ObexServerPutAction::HandleRequest(uint8_t requestType)
{
    requestFinished_ = (requestType == OBEX_OPCODE_PUT_FINAL);
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > maxPacketLength_) {
        SendResponseData(OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (requestPacketLength > HEAD_DEFAULT_LEN) {
        ParseReqeustPackageData(requestPacketLength - HEAD_DEFAULT_LEN, requestType);
        while (privateInput_.empty() && !srmEnabled_ && !requestFinished_) {
            if (!HandleClientPutContinue()) {
                return false;
            }
        }
    }
    while (!srmEnabled_ && !requestFinished_ && privateInput_.empty()) {
        if (!HandleClientPutContinue()) {
            return false;
        }
    }

    uint8_t response = ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    if (requestHandler_) {
        response = ValidateResponseCode(requestHandler_->OnPut(request_, reply_, privateInput_, requestFinished_));
    }
    SendReply(response);
    return true;
}

void ObexServerPutAction::SendReply(uint8_t responseCode)
{
    if (responseCode != ResponseCodes::OBEX_HTTP_OK) {
        SendPackage(responseCode);
        return;
    }
    while (!requestFinished_) {
        hasOkResponse_ = true;
        if (!SendPackage(ResponseCodes::OBEX_HTTP_CONTINUE)) {
            hasOkResponse_ = false;
            return;
        }
        hasOkResponse_ = false;
    }
    SendPackage(ResponseCodes::OBEX_HTTP_OK);
}

uint32_t ObexServerPutAction::ReadRequestPackageLength()
{
    uint8_t dataBuf[HEAD_DEFAULT_LEN];
    int ret = transport_->Read(dataBuf, HEAD_DEFAULT_LEN - 1);
    if (ret != (HEAD_DEFAULT_LEN - 1)) {
        HILOGI("transport Read failed, ret = %{public}d", ret);
        return 0;
    }
    int pos = 0;
    uint32_t packetLength = dataBuf[pos++];
    packetLength = (packetLength << BIT_LEN_8) + dataBuf[pos++];
    return packetLength;
}

bool ObexServerPutAction::HandleClientPutContinue()
{
    SetReplyHeaderConnectionId();

    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    reply_.Clear();

    if (HEAD_DEFAULT_LEN + headerData.size() > maxPacketLength_) {
        HILOGE("some error must happended, return");
        return false;
    }

    std::vector<uint8_t> sendData{};
    sendData.insert(sendData.end(), headerData.begin(), headerData.end());
    SendResponseData(OBEX_HTTP_CONTINUE, sendData, HEAD_DEFAULT_LEN);

    uint8_t requestType;
    int32_t ret = transport_->Read(&requestType, sizeof(requestType));
    if (ret <= 0) {
        HILOGE("read return <= 0, error");
        return false;
    }
    // handle next put action
    if (requestType == OBEX_OPCODE_ABORT) {
        HILOGE("abort recieved, return");
        SendResponseData(OBEX_HTTP_OK, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (requestType != OBEX_OPCODE_PUT && requestType != OBEX_OPCODE_PUT_FINAL) {
        HILOGE("invalid action");
        SendResponseData(OBEX_HTTP_BAD_REQUEST, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    uint32_t requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        SendResponseData(OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }

    requestFinished_ = (requestType == OBEX_OPCODE_PUT_FINAL);
    if (requestPacketLength > HEAD_DEFAULT_LEN) {
        ParseReqeustPackageData(requestPacketLength - HEAD_DEFAULT_LEN, requestType);
    }
    return true;
}

bool ObexServerPutAction::SendPackage(uint8_t code)
{
    bool srmRespSendPending = false;
    bool ignoreSend = false;
    SetReplyHeaderConnectionId();
    if (srmEnabled_ && !hasSrmResponseSent_) {
        srmRespSendPending = true;
        reply_.SetHeader(SINGLE_RESPONSE_MODE, std::vector<uint8_t>{1});
    }
    std::vector<uint8_t> headerData{};
    reply_.GenerateHeaderValue(headerData);
    reply_.Clear();
    if (HEAD_DEFAULT_LEN + headerData.size() > maxPacketLength_) {
        HILOGE("some error must happended, return");
        return false;
    }

    std::vector<uint8_t> sendData{};
    sendData.insert(sendData.end(), headerData.begin(), headerData.end());

    if ((srmActive_) && (hasSrmResponseSent_)) {
        ignoreSend = true;
    }

    if (srmRespSendPending) {
        hasSrmResponseSent_ = true;
    }

    if (ignoreSend && !requestFinished_) {
        bool ret = HandleSendReplyContinue(true);
        if (!ret) {
            HILOGE("HandleSendReplyContinue ret is %{public}d some error must happended, return", ret);
            return false;
        }
        if (code == ResponseCodes::OBEX_HTTP_CONTINUE) {
            return ret;
        }
    }

    if (requestFinished_ || code == OBEX_HTTP_OK) {
        sendData.push_back(END_OF_BODY);
        sendData.push_back(HEAD_DEFAULT_LEN >> BIT_LEN_8);
        sendData.push_back(HEAD_DEFAULT_LEN);
    }
    SendResponseData(code, sendData, HEAD_DEFAULT_LEN);
    if (code == ResponseCodes::OBEX_HTTP_CONTINUE) {
        return HandleSendReplyContinue(hasOkResponse_);
    }
    return true;
}

__attribute__((no_sanitize("cfi")))
bool ObexServerPutAction::HandleSendReplyContinue(bool hasNextResponse)
{
    uint8_t requestType = 0;
    uint32_t requestPacketLength = 0;
    if (!ParseReplyContinueRequestType(requestType, requestPacketLength)) {
        return false;
    }
    requestFinished_ = (requestType == OBEX_OPCODE_PUT_FINAL);
    uint8_t putResponseCode = OBEX_HTTP_CONTINUE;
    while (putResponseCode == OBEX_HTTP_CONTINUE) {
        HILOGI("before putResponseCode is %{public}d, requestFinished %{public}d", putResponseCode, requestFinished_);
        if (requestPacketLength >= HEAD_DEFAULT_LEN) {
            ParseReqeustPackageData(requestPacketLength - HEAD_DEFAULT_LEN, requestType);
            if (requestHandler_) {
                putResponseCode = requestHandler_->OnPut(request_, reply_, privateInput_, requestFinished_);
            }
            privateInput_.clear();
        }
        HILOGI("srmEnabled_ is %{public}d hasNextResponse is %{public}d", srmEnabled_, hasNextResponse);
        if ((!srmEnabled_ && putResponseCode == OBEX_HTTP_CONTINUE) ||
            (putResponseCode != OBEX_HTTP_CONTINUE && !hasNextResponse)) {
            //需要continue确认即srm = 0，发送continue指令;非continue时，后续没有回复处理，发送对应指令。
            SendResponseData(putResponseCode, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        } else if (putResponseCode != OBEX_HTTP_CONTINUE && hasNextResponse) {
            //非continue时，后续有回复处理，break不影响下次发送。
            HILOGI("hasNextResponse break ");
            break;
        }
        //根据回复去接收数据 更新循环条件
        if (putResponseCode == OBEX_HTTP_CONTINUE) {
            if (!ParseReplyContinueRequestType(requestType, requestPacketLength)) {
                HILOGE("ParseReplyContinueRequestType break ");
                break;
            }
        }
        HILOGI("after putResponseCode is %{public}d, requestFinished %{public}d", putResponseCode, requestFinished_);
    }
    return true;
}

bool ObexServerPutAction::ParseReplyContinueRequestType(uint8_t &requestType, uint32_t &requestPacketLength)
{
    int32_t ret = transport_->Read(&requestType, sizeof(uint8_t));
    if (ret <= 0) {
        HILOGE("read return <= 0, error");
        return false;
    }
    HILOGE("action, requestType : %{public}x", requestType);
    // handle next get action
    if (requestType == OBEX_OPCODE_ABORT) {
        HILOGE("abort recieved");
        uint32_t packetLength = ReadRequestPackageLength();
        uint8_t dataBuf = 0;
        for (uint32_t i = HEAD_ABORT_LEN; i < packetLength; i++) {
            transport_->Read(&dataBuf, 1);
        }
        if (requestHandler_) {
            requestHandler_->OnAbort(request_, reply_);
        }
        SendResponseData(OBEX_HTTP_OK, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    if (requestType != OBEX_OPCODE_PUT && requestType != OBEX_OPCODE_PUT_FINAL) {
        HILOGE("invalid action, requestType : %{public}x", requestType);
        SendResponseData(OBEX_HTTP_BAD_REQUEST, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    requestPacketLength = ReadRequestPackageLength();
    if (requestPacketLength > transport_->GetMaxRxPacketSize()) {
        SendResponseData(OBEX_HTTP_REQ_TOO_LARGE, std::vector<uint8_t>{}, HEAD_DEFAULT_LEN);
        return false;
    }
    requestFinished_ = (requestType == OBEX_OPCODE_PUT_FINAL);
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS