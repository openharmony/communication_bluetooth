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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_map_mse_client"
#endif

#include "map_mse_mns_client.h"
#include "bt_sock.h"
#include "service_util.h"
#include "map_mse_appparams.h"
#include "map_mse_service_impl.h"
#include "../obex/obex_server_transport.h"
#include "../obex/obex_header.h"
#include "log.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {
const uint8_t NOTIFICATION_STATUS_NO = 0;
const uint8_t NOTIFICATION_STATUS_YES = 1;
constexpr uint8_t MAC_ADDR_LEN = 6;
// MNS header中的的uuid
const std::vector<uint8_t> MNS_TARGET = {
    0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
const std::string TYPE_EVENT_REPORT = "x-bt/MAP-event-report";
MapMseMnsClient::MapMseMnsClient(const MapMnsSdpFoundRecord &record, MapMseServiceImpl *mapMseServiceImpl)
    : mnsRecord_(record), mapMseServiceImpl_(mapMseServiceImpl)
{
}

void MapMseMnsClient::HandleRegistration(int32_t masId, uint8_t notificationStatus)
{
    HILOGI("masId %{public}d. notificationStatus is %{public}u.", masId, notificationStatus);
    if (notificationStatus == NOTIFICATION_STATUS_NO) {
        auto iter = std::find(registerMasIds_.begin(), registerMasIds_.end(), masId);
        if (iter != registerMasIds_.end()) {
            registerMasIds_.erase(iter);
        }
    }
    if (notificationStatus == NOTIFICATION_STATUS_YES) {
        auto iter = std::find(registerMasIds_.begin(), registerMasIds_.end(), masId);
        if (iter == registerMasIds_.end()) {
            registerMasIds_.push_back(masId);
        }
        if (mnsRecord_.l2capPsm > 0 || mnsRecord_.channel > 0) {
            Connect();
        } else {
            HILOGI("recon map client deviceAddr %{public}s, uuid %{public}s, l2capPsm %{public}d, channel %{public}d.",
                GET_ENCRYPT_STR_ADDR(mnsRecord_.deviceAddr), mnsRecord_.uuid.c_str(),
                mnsRecord_.l2capPsm, mnsRecord_.channel);
            mapMseServiceImpl_->setIsNeedReconnClientFunc_(true);
        }
    }

    if (registerMasIds_.empty()) {
        DisConnect();
    }
}

void MapMseMnsClient::Connect()
{
    std::unique_lock<std::mutex> lock(connectLock_);
    if (connected_.load() || socketFd_ != -1) {
        HILOGI("deviceAddr %{public}s socketFd_ is %{public}d connected, so return",
            GET_ENCRYPT_STR_ADDR(mnsRecord_.deviceAddr), socketFd_);
        return;
    }
    int securityFlags = 3;  // SOCK_FLAG_AUTHENTICATION | SOCK_FLAG_ENCRYPTION;

    HILOGI("deviceAddr %{public}s, uuid %{public}s, l2capPsm %{public}d, channel %{public}d.",
        GET_ENCRYPT_STR_ADDR(mnsRecord_.deviceAddr),
        mnsRecord_.uuid.c_str(),
        mnsRecord_.l2capPsm,
        mnsRecord_.channel);
    CHECK_AND_RETURN_LOG(mapMseServiceImpl_, "mapMseServiceImpl_ is null");
    CHECK_AND_RETURN_LOG(mapMseServiceImpl_->connectSocketFunc_, "connect socket is null");
    SocketType type;
    auto uuid = Uuid::ConvertFromString(mnsRecord_.uuid);
    if (mnsRecord_.l2capPsm > 0) {
        type = SocketType::TYPE_L2CAP;
        socketFd_ = mapMseServiceImpl_->connectSocketFunc_(mnsRecord_.deviceAddr, uuid, securityFlags, 1,
            mnsRecord_.l2capPsm);
    } else if (mnsRecord_.channel > 0) {
        type = SocketType::TYPE_RFCOMM;
        socketFd_ = mapMseServiceImpl_->connectSocketFunc_(mnsRecord_.deviceAddr, uuid, securityFlags, 0,
            mnsRecord_.channel);
    } else {
        HILOGE("mnsRecord_ invalid");
        return;
    }
    HILOGI("socketFd_ is %{public}d", socketFd_);

    if (socketFd_ <= 0) {
        HILOGE("connect error");
        return;
    }
    auto device = RecvSocketDevice(type);
    if (!device) {
        lock.unlock();
        DisConnect();
        return;
    }
    std::shared_ptr<ObexServerTransport> transport = std::make_shared<ObexServerTransport>(device);
    clientSession_ = std::make_shared<ObexClientSession>(transport, nullptr);
    connected_ = true;

    ObexHeader header;
    header.SetHeader(TARGET, MNS_TARGET);
    clientSession_->Connect(header, connectHeader_);
}

void MapMseMnsClient::DisConnect()
{
    std::lock_guard<std::mutex> lock(connectLock_);
    if (!connected_.load()) {
        HILOGI("deviceAddr %{public}s no connect, so return", GET_ENCRYPT_STR_ADDR(mnsRecord_.deviceAddr));
        return;
    }
    if (clientSession_ != nullptr) {
        HILOGI("map start disconnect");
        ObexHeader header;
        header.SetConnectionId(connectHeader_.GetConnectionId());
        clientSession_->DisConnect(header, connectHeader_);
        clientSession_->CloseTransport();
    }
    connected_ = false;
    HILOGI("socketFd_ is %{public}d", socketFd_);
    //socketFd close manager at ObexSocketDevice(socketFd be copied to there to use)
    clientSession_ = nullptr;
    socketFd_ = -1;
}

void MapMseMnsClient::SendEvent(int32_t masId, const std::string &eventData)
{
    {
        std::lock_guard<std::mutex> lock(connectLock_);
        if (!connected_.load()) {
            HILOGE("mns client not connected");
            return;
        }
    }
    std::vector<uint8_t> appParamsRaw{};
    std::unique_ptr<MapMseAppParams> appParams = std::make_unique<MapMseAppParams>();
    appParams->SetMasInsanceId(masId);
    appParams->GetEncodeData(appParamsRaw);

    ObexHeader request;
    request.SetHeader(TYPE, std::vector<uint8_t>(TYPE_EVENT_REPORT.begin(), TYPE_EVENT_REPORT.end()));
    request.SetHeader(APPLICATION_PARAMETER, appParamsRaw);
    request.SetConnectionId(connectHeader_.GetConnectionId());

    clientSession_->SendEvent(request, eventData);
}

bool MapMseMnsClient::IsConnect()
{
    return connected_.load();
}

static RawAddress ConvertRfcommAddr(OHOS::bluetooth::RawAddress &addr)
{
    uint8_t addrBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    addr.ConvertToUint8(addrBytes);
    uint8_t reversedBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    for (int i = 0; i < MAC_ADDR_LEN; i++) {
        reversedBytes[i] = addrBytes[MAC_ADDR_LEN - 1 - i];
    }
    OHOS::bluetooth::RawAddress newAddr =
        OHOS::bluetooth::RawAddress::ConvertToString(reversedBytes);
    return newAddr;
}

std::shared_ptr<ObexSocketDevice> MapMseMnsClient::RecvSocketDevice(SocketType socketType)
{
    int32_t channel = 0;
    struct timeval time = {5, 0}; //设置超时时间5s
    setsockopt(socketFd_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&time), sizeof(time));
    auto ret = recv(socketFd_, &channel, sizeof(channel), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[sock] recv error, ret is %{public}d", ret);
        return nullptr;
    }
    SockConnectSignal cs;
    ret = recv(socketFd_, &cs, sizeof(SockConnectSignal), MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[sock] recv error, ret is %{public}d", ret);
        return nullptr;
    }
    RawAddress newAddr = ConvertRfcommAddr(cs.bdAddr);

    std::shared_ptr<ObexSocketDevice> socketDevice = nullptr;
    if (socketType == SocketType::TYPE_L2CAP) {
        socketDevice = std::make_shared<ObexSocketDevice>(
            socketFd_, cs.maxTxPacketSize, cs.maxRxPacketSize, socketType, newAddr.GetAddress());
    } else if (socketType == SocketType::TYPE_RFCOMM) {
        socketDevice = std::make_shared<ObexSocketDevice>(
            socketFd_, MAX_PACKET_SIZE, MAX_PACKET_SIZE, socketType, newAddr.GetAddress());
    } else {
        HILOGE("[sock] socketType error");
    }

    return socketDevice;
}
}  // namespace bluetooth
}  // namespace OHOS