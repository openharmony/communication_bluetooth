/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "obex_session.h"
#include "log.h"
#include "obex_socket_transport.h"
#include "obex_utils.h"
#include <memory>

namespace bluetooth {
class ObexPrivateServer {};

ObexServerSession::ObexServerSession(ObexTransport &transport, bool isSupportSrm, utility::Dispatcher &dispatcher,
    std::function<int(ObexServerSession &)> removeFun, std::function<void(ObexServerSession &, bool)> setBusyFun)
    : ObexSession(transport.GetRemoteAddress()),
      transport_(transport),
      isSupportSrm_(isSupportSrm),
      dispatcher_(dispatcher)
{
    removeFun_ = std::move(removeFun);
    setBusyFun_ = std::move(setBusyFun);
}

int ObexServerSession::Disconnect()
{
    return 0;
}

int ObexServerSession::SendResponse(ObexHeader &resp) const
{
    // Reset connect response's mtu
    if (resp.GetFieldMaxPacketLength() != nullptr) {
        resp.SetFieldMaxPacketLength(this->maxPacketLength_);
    }
    if (resp.GetFieldPacketLength() > this->maxPacketLength_) {
        LOG_ERROR("Error: Response packet length[%d] > mtu[%d].", resp.GetFieldPacketLength(), this->maxPacketLength_);
        return 0;
    }
    bool ret = this->transport_.Write(resp.Build()->GetPacket());
    if (ret) {
        return 0;
    }
    return 0;
}

int ObexServerSession::SendGetResponse(const ObexHeader &req, ObexHeader &hdr, std::shared_ptr<ObexBodyObject> reader)
{
    if (hdr.HasHeader(ObexHeader::BODY) || hdr.HasHeader(ObexHeader::END_OF_BODY)) {
        LOG_ERROR("Error: Can't include body/end-of-body header in this method.");
        return -1;
    }
    if (hdr.GetFieldCode() != static_cast<uint8_t>(ObexRspCode::SUCCESS) &&
        hdr.GetFieldCode() != static_cast<uint8_t>(ObexRspCode::CONTINUE)) {
        return SendResponse(hdr);
    }
    this->sendObject_ = std::make_unique<ObexServerSendObject>(req, hdr, reader, this->maxPacketLength_, isSupportSrm_);
    auto resq = this->sendObject_->GetNextRespHeader();
    return SendResponse(*resq);
}

ObexSetPathObject::ObexSetPathObject(const std::vector<std::u16string> &paths)
{
    this->paths_ = paths;
    this->pos_ = 0;
}

std::u16string ObexSetPathObject::GetNextPath()
{
    if (this->pos_ >= this->paths_.size()) {
        return nullptr;
    }
    return this->paths_.at(this->pos_++);
}

bool ObexSetPathObject::IsDone() const
{
    return this->pos_ >= this->paths_.size();
}

/// ObexClientSendObject
ObexClientSendObject::ObexClientSendObject(
    const ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> bodyReader, uint16_t mtu)
{
    this->firstReq_ = std::make_unique<ObexHeader>(firstReq);
    this->bodyReader_ = bodyReader;
    this->mtu_ = mtu;
    this->reqIndex_ = 0;
    this->srmEnable_ = false;
    this->isDone_ = false;
}

std::unique_ptr<ObexHeader> ObexClientSendObject::GetNextReqHeader(bool useSrm)
{
    LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    if (this->isDone_) {
        return nullptr;
    }
    std::unique_ptr<ObexHeader> header = nullptr;
    if (this->reqIndex_ == 0) {
        header = std::make_unique<ObexHeader>(*this->firstReq_);
    } else {
        header = std::make_unique<ObexHeader>(*this->firstReq_, true);
        auto connectId = this->firstReq_->GetItemConnectionId();
        if (connectId != nullptr) {
            header->AppendItemConnectionId(connectId->GetWord());
        }
    }
    if (useSrm) {
        if (!header->HasHeader(ObexHeader::SRM)) {
            header->AppendItemSrm(useSrm);
        }
    }
    uint16_t packetLength = header->GetFieldPacketLength();
    if (this->reqIndex_ == 0 && packetLength == this->mtu_ - 3) {
        this->reqIndex_++;
        return header;
    }
    if (packetLength > this->mtu_) {
        LOG_ERROR("Error:packetLength[%d] can't large then this->mtu_[%d] in ObexClientSendObject::GetNextReqHeader",
            packetLength,
            this->mtu_);
        return nullptr;
    }

    if (this->reqIndex_ != 0 && packetLength >= this->mtu_ - 3) {
        LOG_ERROR("Error:can't send next packet, this->reqIndex_[%d] != 0 && packetLength[%d] >= this->mtu_[%d] - 3 in "
                  "ObexClientSendObject::GetNextReqHeader",
            this->reqIndex_,
            packetLength,
            this->mtu_);
        return nullptr;
    }
    uint16_t remainLength = this->mtu_ - packetLength - 3;  // 3: body header+len
    auto buf = std::make_unique<uint8_t[]>(remainLength);
    int cnt = this->bodyReader_->Read(buf.get(), remainLength);
    if (cnt < 0) {
        return nullptr;
    }
    if (cnt < remainLength) {
        this->isDone_ = true;
    }
    if (this->isDone_) {
        LOG_DEBUG("GetNextReqHeader Add End-Body count %d", cnt);
        header->SetFinalBit(true);
        header->AppendItemEndBody(buf.get(), cnt);
    } else {
        LOG_DEBUG("GetNextReqHeader Add Body count %d", cnt);
        header->SetFinalBit(false);
        header->AppendItemBody(buf.get(), cnt);
    }
    this->reqIndex_++;
    return header;
}

bool ObexClientSendObject::IsDone() const
{
    return this->isDone_;
}

void ObexClientSendObject::SetSrmEnable(bool srmEnable)
{
    this->srmEnable_ = srmEnable;
}

bool ObexClientSendObject::IsSrmEnable() const
{
    return this->srmEnable_;
}

void ObexClientSendObject::SetSrmWait(bool wait)
{
    this->srmWait_ = wait;
}

bool ObexClientSendObject::IsSrmWait() const
{
    return this->srmWait_;
}

// ObexClientReceivedObject
ObexClientReceivedObject::ObexClientReceivedObject(
    const ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> writer, bool supportSrm, int srmpCount)
{
    firstReq_ = std::make_unique<ObexHeader>(firstReq);
    bodyWriter_ = writer;
    supportSrmMode_ = supportSrm;
    srmpCount_ = srmpCount;
    srmEnable_ = false;
    isDone_ = false;
    if (supportSrmMode_) {
        if (!firstReq_->HasHeader(ObexHeader::SRM)) {
            firstReq_->AppendItemSrm(supportSrmMode_);
        }
        if (srmpCount_ < 1) {
            firstReq_->RemoveItem(ObexHeader::SRMP);
        } else {
            firstReq_->AppendItemSrmp();
            srmpCount_--;
            srmpSended_ = true;
        }
    } else {
        firstReq_->RemoveItem(ObexHeader::SRM);
        firstReq_->RemoveItem(ObexHeader::SRMP);
    }
}

ObexHeader &ObexClientReceivedObject::GetFirstReqHeader() const
{
    OBEX_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    return *firstReq_;
}

const ObexHeader *ObexClientReceivedObject::GetFirstRespHeader() const
{
    if (firstResp_) {
        return firstResp_.get();
    }
    return nullptr;
}

void ObexClientReceivedObject::CacheResp(const ObexHeader &resp)
{
    if (!firstResp_) {
        firstResp_ = std::make_unique<ObexHeader>(resp);
        firstResp_->RemoveItem(ObexHeader::SRM);
        firstResp_->RemoveItem(ObexHeader::SRMP);
        firstResp_->RemoveItem(ObexHeader::BODY);
        firstResp_->SetExtendBodyObject(nullptr);
    }
}

std::unique_ptr<ObexHeader> ObexClientReceivedObject::GetContinueReqHeader()
{
    OBEX_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    auto header = std::make_unique<ObexHeader>(*firstReq_, true);
    auto connectId = firstReq_->GetItemConnectionId();
    if (connectId != nullptr) {
        header->AppendItemConnectionId(connectId->GetWord());
    }
    if (srmpCount_ > 0) {
        header->AppendItemSrmp();
        srmpCount_--;
        srmpSended_ = true;
    } else {
        srmpSended_ = false;
    }
    header->SetFinalBit(true);
    return header;
}

std::shared_ptr<ObexBodyObject> &ObexClientReceivedObject::GetBodyWriter()
{
    return bodyWriter_;
}

void ObexClientReceivedObject::AppendBody(const uint8_t *body, const uint16_t bodyLen)
{
    bodyWriter_->Write(body, bodyLen);
}

bool ObexClientReceivedObject::IsDone() const
{
    return isDone_;
}

void ObexClientReceivedObject::SetDone(bool done)
{
    isDone_ = done;
}

void ObexClientReceivedObject::SetSrmEnable(bool srmEnable)
{
    srmEnable_ = srmEnable;
}

bool ObexClientReceivedObject::IsSrmEnable() const
{
    return srmEnable_;
}

void ObexClientReceivedObject::SetSrmWait(bool wait)
{
    srmWait_ = wait;
}

bool ObexClientReceivedObject::IsSrmWait() const
{
    return srmWait_;
}

bool ObexClientReceivedObject::IsSupportSrmMode() const
{
    return supportSrmMode_;
}

bool ObexClientReceivedObject::IsSendNextReq() const
{
    if (IsSrmEnable() && !IsSrmWait() && !srmpSended_) {
        return false;
    }
    return true;
}

// ObexServerReceivedObject
ObexServerReceivedObject::ObexServerReceivedObject(const ObexHeader &firstReq, std::shared_ptr<ObexBodyObject> writer)
{
    this->baseReq_ = std::make_unique<ObexHeader>(firstReq);
    this->baseReq_->RemoveItem(ObexHeader::SRM);
    this->baseReq_->RemoveItem(ObexHeader::SRMP);
    this->baseReq_->RemoveItem(ObexHeader::BODY);
    this->baseReq_->SetFinalBit(true);
    this->writer_ = writer;
}

void ObexServerReceivedObject::AppendBody(const uint8_t *body, const uint16_t bodyLen)
{
    this->writer_->Write(body, bodyLen);
}

std::shared_ptr<ObexBodyObject> &ObexServerReceivedObject::GetWriter()
{
    return this->writer_;
}

std::unique_ptr<ObexHeader> ObexServerReceivedObject::CreateReceivedHeader()
{
    auto header = std::make_unique<ObexHeader>(*this->baseReq_);
    header->SetExtendBodyObject(this->writer_);
    return header;
}

ObexServerSendObject::ObexServerSendObject(const ObexHeader &firstReq, const ObexHeader &firstResp,
    std::shared_ptr<ObexBodyObject> bodyReader, uint16_t mtu, bool supportSrm)
{
    firstReq_ = std::make_unique<ObexHeader>(firstReq);
    firstResp_ = std::make_unique<ObexHeader>(firstResp);
    firstResp_->RemoveItem(ObexHeader::SRM);
    firstResp_->RemoveItem(ObexHeader::SRMP);
    bodyReader_ = bodyReader;
    mtu_ = mtu;
    reqIndex_ = 0;
    srmEnable_ = false;
    isDone_ = false;
    supportSrmMode_ = supportSrm;
}

void ObexServerSendObject::SetSrmParam(ObexHeader &header)
{
    if (supportSrmMode_) {
        if (reqIndex_ == 0 && firstReq_->HasHeader(ObexHeader::SRM)) {
            OBEX_LOG_DEBUG("ObexServerSendObject send response with SRM set to true");
            SetSrmEnable(firstReq_->GetItemSrm());
            if (IsSrmEnable()) {
                header.AppendItemSrm(true);
            }
        }
        if (reqIndex_ == 0 && firstReq_->HasHeader(ObexHeader::SRMP)) {
            OBEX_LOG_DEBUG("ObexServerSendObject srmp_wait Set to true");
            SetSrmWait(firstReq_->GetItemSrmp());
        }
    }
}

bool ObexServerSendObject::SetBodyToHeader(ObexHeader &header, const uint16_t &remainLength)
{
    auto buf = std::make_unique<uint8_t[]>(remainLength);
    int cnt = bodyReader_->Read(buf.get(), remainLength);
    if (cnt < 0) {
        return false;
    }
    if (cnt < remainLength) {
        isDone_ = true;
    }
    if (isDone_) {
        OBEX_LOG_DEBUG("GetNextRespHeader Add End-Body count %d", cnt);
        header.SetRespCode(static_cast<uint8_t>(ObexRspCode::SUCCESS));
        header.AppendItemEndBody(buf.get(), cnt);
    } else {
        OBEX_LOG_DEBUG("GetNextRespHeader Add Body count %d", cnt);
        header.SetRespCode(static_cast<uint8_t>(ObexRspCode::CONTINUE));
        header.AppendItemBody(buf.get(), cnt);
    }
    return true;
}

std::unique_ptr<ObexHeader> ObexServerSendObject::GetNextRespHeader()
{
    if (isDone_) {
        return nullptr;
    }
    std::unique_ptr<ObexHeader> header = nullptr;
    if (reqIndex_ == 0) {
        header = std::make_unique<ObexHeader>(*firstResp_);
    } else {
        header = std::make_unique<ObexHeader>(*firstResp_, true);
    }
    // Set SRM
    SetSrmParam(*header);

    uint16_t packetLength = header->GetFieldPacketLength();
    if (reqIndex_ == 0 && packetLength == mtu_ - ObexHeader::MIN_PACKET_LENGTH) {
        reqIndex_++;
        return header;
    }
    if (packetLength > mtu_) {
        OBEX_LOG_ERROR("Error:packetLength[%d] can't large then mtu_[%d] in ObexServerSendObject::GetNextRespHeader",
            packetLength,
            mtu_);
        return nullptr;
    }

    if (reqIndex_ != 0 && packetLength >= mtu_ - ObexHeader::MIN_PACKET_LENGTH) {
        OBEX_LOG_ERROR("Error:can't send next packet, reqIndex_[%d] != 0 && packetLength[%d] >= mtu_[%d] - 3 in "
                       "ObexServerSendObject::GetNextRespHeader",
            reqIndex_,
            packetLength,
            mtu_);
        return nullptr;
    }
    if (!startBodyResp_) {
        header->SetRespCode(static_cast<uint8_t>(ObexRspCode::CONTINUE));
    } else {
        uint16_t remainLength = mtu_ - packetLength - ObexHeader::HDR_BYTES_PREFIX_LENGTH;  // 3: body header+len
        if (!SetBodyToHeader(*header, remainLength)) {
            return nullptr;
        }
    }
    reqIndex_++;
    return header;
}

bool ObexServerSendObject::IsDone() const
{
    return isDone_;
}

void ObexServerSendObject::SetSrmEnable(bool srmEnable)
{
    srmEnable_ = srmEnable;
}

bool ObexServerSendObject::IsSrmEnable() const
{
    return srmEnable_;
}

void ObexServerSendObject::SetSrmWait(bool wait)
{
    srmWait_ = wait;
}

bool ObexServerSendObject::IsSrmWait() const
{
    return srmWait_;
}

void ObexServerSendObject::SetStartBodyResp(bool start)
{
    startBodyResp_ = start;
}

void ObexServerSendObject::SetSrmSending()
{
    isSrmSending_ = true;
}

bool ObexServerSendObject::IsSrmSending() const
{
    return isDone_ ? false : isSrmSending_;
}

void ObexServerSendObject::SetBusy(bool isBusy)
{
    isBusy_ = isBusy;
}

bool ObexServerSendObject::IsBusy() const
{
    return isBusy_;
}

bool ObexServerSendObject::IsSupportSrmMode() const
{
    return supportSrmMode_;
}

ObexSession::ObexSession(const RawAddress &remoteAddr) : remoteAddr_(remoteAddr)
{}
ObexClientSession::ObexClientSession(const RawAddress &remoteAddr) : ObexSession(remoteAddr)
{}
}  // namespace bluetooth