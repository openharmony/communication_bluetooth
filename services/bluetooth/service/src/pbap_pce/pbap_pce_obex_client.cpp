/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_obex_client"
#endif

#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <endian.h>
#include "pbap_pce_obex_client.h"
#include "pbap_pce_service_impl.h"
#include "interface_profile_socket.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "service_util.h"
#include "bt_uuid.h"
#include "bt_sock.h"
#include "log.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "pbap_pce_header_msg.h"
#include "obex_socket_device.h"
#include "obex_def.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_vcard_parser.h"


namespace OHOS {
namespace bluetooth {

size_t MemoryBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    if (buf == nullptr || bufLen == 0) {
        return 0;
    }
    data_.insert(data_.end(), buf, buf + bufLen);
    return bufLen;
}

PbapPceObexClient::PbapPceObexClient(const PbapPceObexConfig &config, PbapPceServiceImpl &pceService)
    : obexConfig_(config), pceService_(pceService)
{
    HILOGI("PbapPceObexClient Create addr=%{public}s",  GetEncryptAddr(obexConfig_.addr).c_str());
}

PbapPceObexClient::~PbapPceObexClient()
{
    CloseTransport();
    HILOGI("PbapPceObexClient destroy");
}

void PbapPceObexClient::CloseTransport()
{
    std::lock_guard<std::mutex> lock(socketFdLock_);
    if (clientSession_ != nullptr) {
        clientSession_->CloseTransport();
        clientSession_ = nullptr;
    }
    socketFd_ = -1;
    isObexConnected_ = false;
}


int PbapPceObexClient::CreateSocketConnection()
{
    HILOGI("[PbapPceObexClient] CreateSocketConnection, addr: %{public}s",
           GetEncryptAddr(obexConfig_.addr).c_str());

    IProfileSocket *socketService =
        (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
    CHECK_AND_RETURN_LOG_RET(socketService != nullptr, -1,
                              "SocketService is nullptr");

    Uuid pbapUuid = Uuid::ConvertFromString(BLUETOOTH_UUID_PBAP_PSE);

    int securityFlag = 3;
    int socketType = 0;
    int psm = 0;

    if (obexConfig_.isL2capPSM && obexConfig_.l2capPsm > 0) {
        socketType = 1;
        psm = obexConfig_.l2capPsm;
    } else {
        socketType = 0;
        psm = obexConfig_.rfCommChannel;
    }

    int socketFd = socketService->Connect(obexConfig_.addr, pbapUuid, securityFlag, socketType, psm);

    if (socketFd == -1) {
        HILOGE("[PbapPceObexClient] SocketService::Connect failed");
        return -1;
    }

    HILOGI("[PbapPceObexClient] SocketService::Connect success, fd: %{public}d", socketFd);
    return socketFd;
}

void PbapPceObexClient::CloseSocket()
{
    HILOGI("[PbapPceObexClient] CloseSocket, fd: %{public}d", socketFd_);
    isDisconnectRequested_ = true;
    if (socketFd_ == -1) {
        HILOGW("[PbapPceObexClient] CloseSocket, socket already invalid");
        return;
    }
    shutdown(socketFd_, SHUT_RDWR);
    close(socketFd_);
    socketFd_ = -1;
    isObexConnected_ = false;
    utility::Message msg(PCE_OBEX_DISCONNECTED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);

    HILOGI("[PbapPceObexClient] CloseSocket success");
}

void PbapPceObexClient::ForceClose()
{
    HILOGI("[PbapPceObexClient] ForceClose");
    CloseSocket();
}

std::shared_ptr<ObexSocketDevice> PbapPceObexClient::RecvSocketDevice(SocketType socketType)
{
    HILOGI("[PbapPceObexClient] RecvSocketDevice, socketType: %{public}d", socketType);
    int32_t channel = 0;
    auto ret = recv(socketFd_, &channel, sizeof(channel), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[PbapPceObexClient] recv channel error, ret=%{public}d", ret);
        return nullptr;
    }
    SockConnectSignal cs;
    ret = recv(socketFd_, &cs, sizeof(SockConnectSignal), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[PbapPceObexClient] recv connect_info error, ret=%{public}d", ret);
        return nullptr;
    }
    HILOGI("[PbapPceObexClient] channel=%{public}d, mtu tx=%{public}d rx=%{public}d",
        channel, cs.maxTxPacketSize, cs.maxRxPacketSize);

    std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
        socketFd_, obexConfig_.mtu, obexConfig_.mtu, socketType, obexConfig_.addr);
    return socketDevice;
}

void PbapPceObexClient::ObexConnectFailed()
{
    isObexConnected_ = false;
    utility::Message msg(PCE_OBEX_CONNECT_FAILED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
    CloseSocket();
    CloseTransport();
}

void PbapPceObexClient::BuildConnectHeader(ObexHeader& header, bool supported)
{
    HILOGI("[PbapPceObexClient] BuildConnectHeader, supported: %{public}d", supported);

    header.SetHeader(TARGET, std::vector<uint8_t>(PBAP_PCE_SERVICE_UUID,
        PBAP_PCE_SERVICE_UUID + PBAP_PCE_SERVICE_UUID_LEN));

    if (supported) {
        ObexTlvParamters appParams;
        uint32_t features = PBAP_PCE_FEATURES_DOWNLOAD | PBAP_PCE_FEATURES_BROWSING;
        uint32_t beFeatures = htobe32(features);
        TlvTriplet tlv(PbapPceAppParams::PBAP_SUPPORTED_FEATURES, beFeatures);
        appParams.AppendTlvtriplet(tlv);
        header.AppendItemAppParams(appParams);
    }

    header.SetConnectionId(0);

    HILOGI("[PbapPceObexClient] BuildConnectHeader success");
}

int PbapPceObexClient::Connect(std::shared_ptr<PbapPceObexClient> self, bool supported)
{
    HILOGI("[PbapPceObexClient] Connect enter, supported: %{public}d", supported);

    int socketFd = CreateSocketConnection();
    if (socketFd == -1) {
        HILOGE("[PbapPceObexClient] Socket connection failed");
        isObexConnected_ = false;
        utility::Message msg(PCE_OBEX_CONNECT_FAILED);
        pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
        return RET_BAD_STATUS;
    }
    socketFd_ = socketFd;

    SocketType transportType = obexConfig_.isL2capPSM ? SocketType::TYPE_L2CAP : SocketType::TYPE_RFCOMM;
    auto socketDevice = RecvSocketDevice(transportType);
    if (socketDevice == nullptr) {
        HILOGE("[PbapPceObexClient] RecvSocketDevice failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(socketDevice);
    if (transport == nullptr) {
        HILOGE("[PbapPceObexClient] Create ObexServerTransport failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    clientSession_ = std::make_shared<ObexClientSession>(transport, self);

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] Create ObexClientSession failed");
        ObexConnectFailed();
        return RET_BAD_STATUS;
    }

    ObexHeader connectHeader;
    BuildConnectHeader(connectHeader, supported);

    clientSession_->Connect(connectHeader, connectHeader_);

    HILOGI("[PbapPceObexClient] Connect request sent, waiting for response");
    return RET_NO_ERROR;
}

int PbapPceObexClient::Disconnect(bool withObexReq)
{
    HILOGI("PbapPceObexClient::Disconnect start withObexReq=%{public}d", withObexReq);
    if (clientSession_ == nullptr) {
        HILOGE("Disconnect: clientSession_ is null");
        return RET_BAD_STATUS;
    }

    if (withObexReq && isObexConnected_) {
        ObexHeader header;
        header.SetConnectionId(connectionId_);
        ObexHeader resp;
        clientSession_->DisConnect(header, resp);
    } else {
        CloseTransport();
        isObexConnected_ = false;
        utility::Message msg(PCE_OBEX_DISCONNECTED);
        pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
    }
    return RET_NO_ERROR;
}

int PbapPceObexClient::Get(const ObexHeader &req, int reqMsgType, std::shared_ptr<ObexBodyObject> writer)
{
    HILOGI("[PbapPceObexClient] Get enter, reqMsgType: %{public}d", reqMsgType);

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] Get failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    currentReqMsgType_ = reqMsgType;

    SetPhoneBookActionInfo(clientSession_->GetLastOpeId(), reqMsgType, u"", 0);

    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);

    ObexHeader resp;
    int ret = clientSession_->Get(const_cast<ObexHeader &>(req), resp, writer);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] Get failed, ret: %{public}d", ret);
        pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);
        return ret;
    }
    HILOGI("[PbapPceObexClient] Get success");
    return RET_NO_ERROR;
}

int PbapPceObexClient::Abort()
{
    HILOGI("PbapPceObexClient::Abort start");
    if (clientSession_ == nullptr) {
        HILOGE("Abort: clientSession_ is null");
        return RET_BAD_STATUS;
    }

    ObexHeader header;
    header.SetConnectionId(connectionId_);
    ObexHeader resp;
    clientSession_->Abort(header, resp);
    SetAbort(true);
    return RET_NO_ERROR;
}

ObexClientSession *PbapPceObexClient::GetClientSession() const
{
    return clientSession_.get();
}

const std::string &PbapPceObexClient::GetRemoteAddress() const
{
    return obexConfig_.addr;
}

void PbapPceObexClient::SetBusy(bool isBusy)
{
    isBusy_ = isBusy;
}

bool PbapPceObexClient::IsBusy() const
{
    return isBusy_ || isBatchDownloading_.load();
}

void PbapPceObexClient::SetAbort(bool isAbort)
{
    isAbort_ = isAbort;
}

bool PbapPceObexClient::GetPhoneBookActionInfo(
    int operationId, int &retReqMsgType, std::u16string &retPath, uint8_t &retFlags) const
{
    retReqMsgType = phoneBookActionInfo_.reqMsgType_;
    retPath = phoneBookActionInfo_.path_;
    retFlags = phoneBookActionInfo_.flags_;
    return true;
}

void PbapPceObexClient::SetPhoneBookActionInfo(
    int operationId, int reqMsgType, const std::u16string &path, uint8_t flags, bool isBusy)
{
    phoneBookActionInfo_.operationId_ = operationId;
    phoneBookActionInfo_.reqMsgType_ = reqMsgType;
    phoneBookActionInfo_.path_ = path;
    phoneBookActionInfo_.flags_ = flags;
    isBusy_ = isBusy;
}

void PbapPceObexClient::OnConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("[PbapPceObexClient] OnConnectResp enter, responseCode: 0x%02x", responseCode);

    SetPhoneBookActionInfo(clientSession_->GetLastOpeId(), PCE_OBEX_CONNECTED, u"", 0, false);

    switch (responseCode) {
        case OBEX_HTTP_OK:
            HandleConnectSuccess(reply);
            break;
        case OBEX_HTTP_UNAUTHORIZED:
        case OBEX_HTTP_FORBIDDEN:
        case OBEX_HTTP_UNAVAILABLE:
            HILOGE("[PbapPceObexClient] Service unavailable");
            NotifyConnectFailed();
            break;
        default:
            HILOGE("[PbapPceObexClient] Connect failed, responseCode: 0x%02x", responseCode);
            NotifyConnectFailed();
            break;
    }

    HILOGI("[PbapPceObexClient] OnConnectResp end");
}

void PbapPceObexClient::HandleConnectSuccess(ObexHeader &reply)
{
    HILOGI("[PbapPceObexClient] HandleConnectSuccess");
    int32_t connectionId = reply.GetConnectionId();
    if (connectionId != 0) {
        connectionId_ = connectionId;
        reply.SetConnectionId(connectionId);
        HILOGI("[PbapPceObexClient] Connection ID: %d", connectionId);
    }

    isObexConnected_ = true;
    std::unique_ptr<PbapPceObexMessage> obexMsg =
        std::make_unique<PbapPceObexMessage>(obexConfig_.addr, connectHeader_);
    utility::Message msg(PCE_OBEX_CONNECTED, 0, static_cast<void*>(obexMsg.release()));
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}


void PbapPceObexClient::NotifyConnectFailed()
{
    HILOGI("[PbapPceObexClient] NotifyConnectFailed");
    isObexConnected_ = false;
    utility::Message msg(PCE_OBEX_CONNECT_FAILED, 0, nullptr);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}


void PbapPceObexClient::OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("OnPutDateAvailableResp responseCode=%{public}0x", responseCode);
}

void PbapPceObexClient::OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("OnGetDateAvailableResp responseCode=%{public}0x", responseCode);
    if (isBatchDownloading_) {
        HILOGI("OnGetDateAvailableResp suppressed during batch download, code=0x%02x", responseCode);
        return;
    }
    isObexConnected_ = true;
    switch (currentReqMsgType_) {
        case PCE_REQ_PULLPHONEBOOK:
            pceService_.ProcessPhonebookMessage(RawAddress(obexConfig_.addr),
                utility::Message(PCE_PULLPHONEBOOK_COMPLETED));
            break;
        case PCE_REQ_PULLPHONEBOOKSIZE:
            pceService_.ProcessPhonebookMessage(RawAddress(obexConfig_.addr),
                utility::Message(PCE_PULLPHONEBOOKSIZE_COMPLETED));
            break;
        default:
            break;
    }
}

void PbapPceObexClient::OnDisConnectResp(ObexHeader &reply, uint8_t responseCode)
{
    HILOGI("OnDisConnectResp responseCode=%{public}0x", responseCode);
    isBusy_ = false;
    isObexConnected_ = false;
    CloseTransport();
    utility::Message msg(PCE_OBEX_DISCONNECTED);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}

void PbapPceObexClient::OnAbortResp()
{
    HILOGI("OnAbortResp");
    isBusy_ = false;
}

void PbapPceObexClient::BuildGetHeader(ObexHeader &header, const std::string &path, const PbapPceAppParams &appParams)
{
    HILOGI("[PbapPceObexClient] BuildGetHeader, path: %{public}s", path.c_str());

    header.SetConnectionId(connectionId_);
    header.SetType("x-bt/phonebook");

    std::u16string u16Path(path.begin(), path.end());
    header.SetNameU16(u16Path);

    appParams.AddToObexHeader(header);
}

void PbapPceObexClient::ExtractBodyFromResponse(ObexHeader &resp, MemoryBodyObject &writer)
{
    if (resp.HasHeader(BODY)) {
        uint16_t bodyLen = 0;
        std::vector<uint8_t> bodyData = resp.GetBody(bodyLen);
        if (bodyLen > 0) {
            writer.Write(bodyData.data(), bodyLen);
        }
    }
    if (resp.HasHeader(END_OF_BODY)) {
        uint16_t endBodyLen = 0;
        std::vector<uint8_t> endBodyData = resp.GetEndOfBody(endBodyLen);
        if (endBodyLen > 0) {
            writer.Write(endBodyData.data(), endBodyLen);
        }
    }
}

uint16_t PbapPceObexClient::PullPhoneBookSize(const std::string &path)
{
    HILOGI("[PbapPceObexClient] PullPhoneBookSize, path: %{public}s", path.c_str());

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] PullPhoneBookSize failed, clientSession_ is null");
        return 0;
    }

    std::string fullPath = path + VCARD_FILE_SUFFIX;

    ObexHeader reqHeader;
    PbapPceAppParams appParams;
    appParams.SetMaxListCount(0);
    BuildGetHeader(reqHeader, fullPath, appParams);
    reqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);

    currentReqMsgType_ = PCE_REQ_PULLPHONEBOOKSIZE;
    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);

    ObexHeader resp;
    int ret = clientSession_->Get(reqHeader, resp, nullptr);

    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullPhoneBookSize failed, ret: %{public}d", ret);
        return 0;
    }

    PbapPceAppParams respParams = PbapPceAppParams::FromObexHeader(resp);
    uint16_t phonebookSize = respParams.HasPhonebookSize() ? respParams.GetPhonebookSize() : 0;

    HILOGI("[PbapPceObexClient] PullPhoneBookSize success, size: %{public}u", phonebookSize);
    return phonebookSize;
}

int PbapPceObexClient::PullCalllog(const std::string &path, uint16_t offset, uint16_t count)
{
    HILOGI("[PbapPceObexClient] PullPhoneBookBatch, path: %{public}s, offset: %{public}u, count: %{public}u",
        path.c_str(), offset, count);

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    std::string fullPath = path + VCARD_FILE_SUFFIX;

    ObexHeader reqHeader;
    PbapPceAppParams appParams;
    appParams.SetMaxListCount(count);
    // appParams.SetListStartOffset(offset);
    appParams.SetFormat(PBAP_PCE_VCARD_FORMAT_30);
    BuildGetHeader(reqHeader, fullPath, appParams);
    reqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);

    auto writer = std::make_shared<MemoryBodyObject>();

    currentReqMsgType_ = PCE_REQ_PULLPHONEBOOK;
    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);

    ObexHeader resp;
    int ret = clientSession_->Get(reqHeader, resp, writer);

    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, ret: %{public}d", ret);
        return ret;
    }

    ExtractBodyFromResponse(resp, *writer);

    const std::vector<uint8_t> &bodyData = writer->GetData();
    HILOGI("[PbapPceObexClient] PullCalllog success, path=%{public}s, body size: %{public}zu",
        path.c_str(), bodyData.size());

    // Parse vCard data and insert call logs into database
    int parseRet = ParseAndInsertCallLogs(bodyData, path);
    if (parseRet != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] ParseAndInsertCallLogs failed, path=%{public}s, ret=%{public}d",
            path.c_str(), parseRet);
    }

    return RET_NO_ERROR;
}

int PbapPceObexClient::ParseAndInsertCallLogs(const std::vector<uint8_t> &bodyData, const std::string &path)
{
    HILOGI("[PbapPceObexClient] ParseAndInsertCallLogs enter, path=%{public}s, bodySize=%{public}zu",
        path.c_str(), bodyData.size());

    if (bodyData.empty()) {
        HILOGW("[PbapPceObexClient] ParseAndInsertCallLogs: body data is empty, path=%{public}s", path.c_str());
        return RET_NO_ERROR;
    }

    // Determine call type from PBAP path
    PbapPceCallType callType = PBAP_PCE_CALL_TYPE_MISSED;
    if (path == PATH_MCH) {
        callType = PBAP_PCE_CALL_TYPE_MISSED;
    } else if (path == PATH_ICH) {
        callType = PBAP_PCE_CALL_TYPE_INCOMING;
    } else if (path == PATH_OCH) {
        callType = PBAP_PCE_CALL_TYPE_OUTGOING;
    } else {
        HILOGW("[PbapPceObexClient] ParseAndInsertCallLogs: unknown path=%{public}s, default to MISSED", path.c_str());
    }
    HILOGI("[PbapPceObexClient] call type determined: path=%{public}s, callType=%{public}d",
        path.c_str(), static_cast<int32_t>(callType));

    // Parse vCard data in-memory
    std::vector<PbapPceVCardContact> contacts =
        PbapPceVCardParser::Parse(bodyData, PbapPceParseScenario::CALL_LOGS);
    HILOGI("[PbapPceObexClient] vCard parsed: path=%{public}s, contacts count=%{public}zu",
        path.c_str(), contacts.size());

    // Log each parsed call log entry for debugging (before DB insert)
    for (size_t i = 0; i < contacts.size(); i++) {
        const PbapPceVCardContact &c = contacts[i];
        std::string phoneStr = c.phones.empty() ? "" : c.phones[0];
        HILOGI("[PbapPceObexClient] parsed calllog[%{public}zu]: name=%{public}s, phone=%{public}s, "
            "phonesCount=%{public}zu, xDataCount=%{public}zu, version=%{public}s",
            i,
            c.displayName.c_str(),
            phoneStr.c_str(),
            c.phones.size(),
            c.unknownXData.size(),
            c.version.c_str());
        // Log X-IRMC-CALL-DATETIME entries for timestamp debugging
        for (size_t j = 0; j < c.unknownXData.size(); j++) {
            HILOGI("[PbapPceObexClient]   xData[%{public}zu]: prop=%{public}s, value=%{public}s",
                j, c.unknownXData[j].first.c_str(), c.unknownXData[j].second.c_str());
        }
    }

    if (contacts.empty()) {
        HILOGW("[PbapPceObexClient] ParseAndInsertCallLogs: no contacts parsed, skip insert, path=%{public}s",
            path.c_str());
        return RET_NO_ERROR;
    }

    // Insert into calllog database
    if (dataShareHelper_ == nullptr) {
        dataShareHelper_ = std::make_unique<PbapPceDataShareHelper>();
    }
    int32_t insertRet = dataShareHelper_->BatchInsertCallLogs(contacts, callType);
    HILOGI("[PbapPceObexClient] BatchInsertCallLogs result: path=%{public}s, ret=%{public}d",
        path.c_str(), insertRet);

    return insertRet;
}

int PbapPceObexClient::PullPhoneBookBatch(const std::string &path, uint16_t offset, uint16_t count, bool isFinish)
{
    HILOGI("[PbapPceObexClient] PullPhoneBookBatch, path: %{public}s, offset: %{public}u, count: %{public}u, "
        "isFinish: %{public}d",
        path.c_str(), offset, count, static_cast<int>(isFinish));

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, clientSession_ is null");
        return RET_BAD_STATUS;
    }

    std::string fullPath = path + VCARD_FILE_SUFFIX;

    ObexHeader reqHeader;
    PbapPceAppParams appParams;
    appParams.SetMaxListCount(count);
    appParams.SetListStartOffset(offset);
    appParams.SetFormat(PBAP_PCE_VCARD_FORMAT_30);
    BuildGetHeader(reqHeader, fullPath, appParams);
    reqHeader.SetRecvTimeoutMs(PBAP_PCE_GET_TIMEOUT_MS);

    auto writer = std::make_shared<MemoryBodyObject>();

    currentReqMsgType_ = PCE_REQ_PULLPHONEBOOK;
    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), true);

    ObexHeader resp;
    int ret = clientSession_->Get(reqHeader, resp, writer);

    pceService_.SetPowerStatusBusy(RawAddress(obexConfig_.addr), false);

    if (ret != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] PullPhoneBookBatch failed, ret: %{public}d", ret);
        return ret;
    }

    ExtractBodyFromResponse(resp, *writer);

    const std::vector<uint8_t> &bodyData = writer->GetData();
    HILOGI("[PbapPceObexClient] PullPhoneBookBatch success, path=%{public}s, offset=%{public}u, body size: %{public}zu",
        path.c_str(), offset, bodyData.size());

    // Parse vCard data and insert contacts into database
    int parseRet = ParseAndInsertContacts(bodyData, isFinish);
    if (parseRet != RET_NO_ERROR) {
        HILOGE("[PbapPceObexClient] ParseAndInsertContacts failed, offset=%{public}u, ret=%{public}d",
            offset, parseRet);
    }

    return RET_NO_ERROR;
}

int PbapPceObexClient::ParseAndInsertContacts(const std::vector<uint8_t> &bodyData, bool isFinish)
{
    HILOGI("[PbapPceObexClient] ParseAndInsertContacts enter, bodySize=%{public}zu, isFinish=%{public}d",
        bodyData.size(), static_cast<int>(isFinish));

    if (bodyData.empty()) {
        HILOGW("[PbapPceObexClient] ParseAndInsertContacts: body data is empty");
        return RET_NO_ERROR;
    }

    // Parse vCard data in-memory
    std::vector<PbapPceVCardContact> contacts = PbapPceVCardParser::Parse(bodyData);
    HILOGI("[PbapPceObexClient] vCard parsed: contacts count=%{public}zu", contacts.size());

    // Log each parsed contact for debugging (before DB insert)
    for (size_t i = 0; i < contacts.size(); i++) {
        const PbapPceVCardContact &c = contacts[i];
        // Build phone numbers string for logging
        std::string phoneStr;
        for (size_t j = 0; j < c.phones.size(); j++) {
            if (j > 0) {
                phoneStr += ",";
            }
            phoneStr += c.phones[j];
        }
        HILOGI("[PbapPceObexClient] parsed contact[%{public}zu]: name=%{public}s, family=%{public}s, "
            "given=%{public}s, middle=%{public}s, phones=%{public}s, phonesCount=%{public}zu, "
            "emailsCount=%{public}zu, version=%{public}s",
            i,
            c.displayName.c_str(),
            c.familyName.c_str(),
            c.givenName.c_str(),
            c.middleName.c_str(),
            phoneStr.c_str(),
            c.phones.size(),
            c.emails.size(),
            c.version.c_str());
    }

    if (contacts.empty()) {
        HILOGW("[PbapPceObexClient] ParseAndInsertContacts: no contacts parsed, skip insert");
        return RET_NO_ERROR;
    }

    // Insert into contacts database
    if (dataShareHelper_ == nullptr) {
        dataShareHelper_ = std::make_unique<PbapPceDataShareHelper>();
    }
    int32_t insertRet = dataShareHelper_->BatchInsertContacts(contacts, isFinish);
    HILOGI("[PbapPceObexClient] BatchInsertContacts result: ret=%{public}d", insertRet);

    return insertRet;
}

void PbapPceObexClient::DownloadContactsAndCallLogs()
{
    HILOGI("[PbapPceObexClient] DownloadContactsAndCallLogs start, addr: %{public}s",
        GetEncryptAddr(obexConfig_.addr).c_str());

    if (clientSession_ == nullptr) {
        HILOGE("[PbapPceObexClient] DownloadContactsAndCallLogs failed, clientSession_ is null");
        NotifyDownloadComplete();
        return;
    }

    isBatchDownloading_ = true;
    isDisconnectRequested_ = false;



    // 2. Get phonebook total size
    uint16_t totalSize = PullPhoneBookSize(PATH_PB);
    HILOGI("[PbapPceObexClient] Phonebook total size: %{public}u", totalSize);

    // 3. Start a new contact sync session (generates traceId, resets batchId)
    if (dataShareHelper_ == nullptr) {
        dataShareHelper_ = std::make_unique<PbapPceDataShareHelper>();
    }
    dataShareHelper_->StartContactSync();

    // 4. Download contacts in batches of DEFAULT_BATCH_SIZE (250)
    uint32_t totalBatches = (totalSize + PBAP_PCE_DEFAULT_BATCH_SIZE - 1) / PBAP_PCE_DEFAULT_BATCH_SIZE;
    uint32_t batchIdx = 0;
    for (uint16_t offset = 0; offset < totalSize; offset += PBAP_PCE_DEFAULT_BATCH_SIZE) {
        if (isDisconnectRequested_.load()) {
            HILOGI("[PbapPceObexClient] disconnect requested, abort contacts download loop");
            break;
        }
        uint16_t count = std::min(PBAP_PCE_DEFAULT_BATCH_SIZE, static_cast<uint16_t>(totalSize - offset));
        bool isFinish = (batchIdx == totalBatches - 1);
        HILOGI("[PbapPceObexClient] Downloading contacts batch: offset=%{public}u, count=%{public}u, "
            "batchIdx=%{public}u/%{public}u, isFinish=%{public}d",
            offset, count, batchIdx, totalBatches, static_cast<int>(isFinish));

        int batchRet = PullPhoneBookBatch(PATH_PB, offset, count, isFinish);
        if (batchRet != RET_NO_ERROR) {
            HILOGE("[PbapPceObexClient] Contacts batch download failed at offset=%{public}u, ret=%{public}d",
                offset, batchRet);
            break;
        }
        HILOGI("[PbapPceObexClient] Contacts batch downloaded: offset=%{public}u", offset);
        batchIdx++;
    }

    // 1. Download call logs: MCH -> ICH -> OCH (each max 100 entries)
    HILOGI("[PbapPceObexClient] Downloading call logs (MCH/ICH/OCH, max %{public}u each)",
        PBAP_PCE_CALLLOG_MAX_COUNT);

    if (isDisconnectRequested_.load()) {
        HILOGI("[PbapPceObexClient] disconnect requested, skip MCH");
    } else {
        int mchRet = PullCalllog(PATH_MCH, 0, PBAP_PCE_CALLLOG_MAX_COUNT);
        HILOGI("[PbapPceObexClient] MCH download result: %{public}d", mchRet);
    }

    if (isDisconnectRequested_.load()) {
        HILOGI("[PbapPceObexClient] disconnect requested, skip ICH");
    } else {
        int ichRet = PullCalllog(PATH_ICH, 0, PBAP_PCE_CALLLOG_MAX_COUNT);
        HILOGI("[PbapPceObexClient] ICH download result: %{public}d", ichRet);
    }

    if (isDisconnectRequested_.load()) {
        HILOGI("[PbapPceObexClient] disconnect requested, skip OCH");
    } else {
        int ochRet = PullCalllog(PATH_OCH, 0, PBAP_PCE_CALLLOG_MAX_COUNT);
        HILOGI("[PbapPceObexClient] OCH download result: %{public}d", ochRet);
    }

    isBatchDownloading_ = false;
    isDisconnectRequested_ = false;

    HILOGI("[PbapPceObexClient] DownloadContactsAndCallLogs completed, addr: %{public}s",
        GetEncryptAddr(obexConfig_.addr).c_str());

    NotifyDownloadComplete();
}

void PbapPceObexClient::NotifyDownloadComplete()
{
    utility::Message msg(PCE_DOWNLOAD_COMPLETE);
    pceService_.OnObexResponse(RawAddress(obexConfig_.addr), msg);
}

}  // namespace bluetooth
}  // namespace OHOS
