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
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include "../obex/obex_utils.h"
#include "log.h"
#include "socket_transport_mock.h"
#include "securec.h"
#include "transport_def.h"

namespace bluetooth {
#define MK_LOG_DEBUG(fmt, ...) LOG_DEBUG("[MockSockTransport]:" fmt, ##__VA_ARGS__)
#define MK_LOG_INFO(fmt, ...) LOG_INFO("[MockSockTransport]:" fmt, ##__VA_ARGS__)
#define MK_LOG_ERROR(fmt, ...) LOG_ERROR("[MockSockTransport]:" fmt, ##__VA_ARGS__)
#define MK_LOG_WARN(fmt, ...) LOG_WARN("[MockSockTransport]:" fmt, ##__VA_ARGS__)

static const uint16_t MIN_SOCK_PORT = 1024;
static const int SLEEP_MS = 100;

static const std::string CLIENT_COLOR = "\033[1m\033[33m";
static const std::string SERVER_COLOR = "\033[1m\033[36m";
static const std::string RESET_COLOR = "\033[0m";

static const char *ACCEPT = "ACCEPT";
static const size_t ACCEPT_REJECT_SIZE = 6;
static const char *REJECT = "REJECT";
static const uint16_t MAX_RECEIVE_PACKET_SIZE = 500;
static const uint16_t MAX_SEND_PACKET_SIZE = 500;

static const uint16_t UNBUSY_WRITE_PKT_COUNT = 0;
static const uint16_t BUSY_WRITE_PKT_COUNT = 2;

int MockSockTransport::lastClientFd_ = -1;
int MockSockTransport::lastServerFd_ = -1;
MockSockTransport::MockSockTransport(const RawAddress *addr, uint16_t lpsm, uint16_t rpsm, uint16_t mtu,
    DataTransportObserver &observer, TransportType transportType)
    : observer_(observer), transportType_(transportType)
{
    if (addr != nullptr) {
        // client mode only
        remoteAddr_ = std::make_unique<RawAddress>(*addr);
        transportSubType_ = TransportSubType::CLIENT;
        remotePort_ = rpsm;
    } else {
        transportSubType_ = TransportSubType::SERVER_MAIN;
        remotePort_ = lpsm;
    }
    isCallObserver_ = false;

    if (remotePort_ < MIN_SOCK_PORT) {
        remotePort_ = remotePort_ + MIN_SOCK_PORT;
    }
    MK_LOG_DEBUG("remotePort=0x%04X", remotePort_);
    Init();
}

MockSockTransport::MockSockTransport(int newSocketFd, MockSockTransport &parentSockTransport,
    DataTransportObserver &observer, const TransportType &transportType,
    MockSockTransport::TransportContext &transportContext)
    : localSockfd_(newSocketFd),
      parentSockTransport_(&parentSockTransport),
      observer_(observer),
      transportType_(transportType),
      transportContext_(&transportContext)
{
    isConnected_ = true;
    isCallObserver_ = true;
    transportSubType_ = TransportSubType::SERVER_SUB;
}

MockSockTransport::~MockSockTransport()
{
    MK_LOG_DEBUG("[%d] Call %s Start at %p ", transportSubType_, __PRETTY_FUNCTION__, this);
    isCallObserver_ = false;
    threadRunning_ = false;
    if (transportSubType_ == TransportSubType::SERVER_MAIN) {
        MK_LOG_DEBUG("TransportSubType::SERVER_MAIN");
        if (isServerStarted_) {
            RemoveServer();
        }
        if (mainDispatcher_ != nullptr) {
            MK_LOG_DEBUG("delete mainDispatcher_");
            delete mainDispatcher_;
            mainDispatcher_ = nullptr;
        }
        if (mainTransportContext_ != nullptr) {
            delete mainTransportContext_;
            mainTransportContext_ = nullptr;
        }
    } else if (transportSubType_ == TransportSubType::CLIENT) {
        MK_LOG_DEBUG("TransportSubType::CLIENT");
        WaitReceiveToEnd();
        if (mainDispatcher_ != nullptr) {
            MK_LOG_DEBUG("delete mainDispatcher_");
            delete mainDispatcher_;
            mainDispatcher_ = nullptr;
        }
        if (mainTransportContext_ != nullptr) {
            delete mainTransportContext_;
            mainTransportContext_ = nullptr;
        }
    } else if (transportSubType_ == TransportSubType::SERVER_SUB) {
        MK_LOG_DEBUG("TransportSubType::SERVER_SUB");
    }
    if (localSockfd_ != 0) {
        close(localSockfd_);
    }
    MK_LOG_DEBUG("Call %s End at %p ", __PRETTY_FUNCTION__, this);
}

void MockSockTransport::Init()
{
    auto dispatcher = std::make_unique<utility::Dispatcher>("MockSockTransport dispatcher");
    mainDispatcher_ = dispatcher.release();
    auto context = std::make_unique<TransportContext>(*mainDispatcher_);
    mainTransportContext_ = context.release();
    transportContext_ = mainTransportContext_;
}

int MockSockTransport::Connect()
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    if (isConnected_) {
        MK_LOG_ERROR("Error Already Connected!");
        return -1;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        MK_LOG_ERROR("Error socket:%d", errno);
        return -1;
    }
    localSockfd_ = sockfd;
    MockSockTransport::lastClientFd_ = sockfd;
    transportContext_->PostTask(std::bind(&MockSockTransport::ConnectThread, this));
    return 0;
}

int MockSockTransport::Disconnect()
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    if (!isConnected_) {
        return -1;
    }
    if (transportSubType_ == TransportSubType::CLIENT) {
        transportContext_->PostTask(std::bind(&MockSockTransport::DisconnectClientThread, this));
    } else if (transportSubType_ == TransportSubType::SERVER_SUB) {
        transportContext_->PostTask(
            std::bind(&MockSockTransport::RemoveSocketFd, parentSockTransport_, this->localSockfd_));
    }
    return 0;
}

void MockSockTransport::DisconnectClientThread()
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    if (isConnected_) {
        isConnected_ = false;
        WaitReceiveToEnd();
        CloseFd(localSockfd_, this);
        isConnectedReceivedData_ = false;
    }
}

void MockSockTransport::ClientDisconnectedThread()
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    if (isConnected_) {
        isConnected_ = false;
        WaitReceiveToEnd();
        isConnectedReceivedData_ = false;
        if (isCallObserver_) {
            isCallObserver_ = false;
            observer_.OnDisconnected(this);
        }
    }
}

// Only call by server mode
int MockSockTransport::RemoveSocketFd(int subSocketFd)
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    std::unique_lock<std::mutex> lck(threadMutex_);
    {
        std::lock_guard<std::recursive_mutex> incomingLock(incomingMutex_);
        for (auto it = incomingConnectMap_.begin(); it != incomingConnectMap_.end();) {
            if (it->second == subSocketFd) {
                it = incomingConnectMap_.erase(it);
                break;
            }
            ++it;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> subTranLock(subTransportMutex_);
        if (subTransportMap_.find(subSocketFd) != subTransportMap_.end()) {
            auto &socketTransport = subTransportMap_.at(subSocketFd);
            CloseFd(subSocketFd, socketTransport);
            MK_LOG_DEBUG("[%d] Remove subTransport %p", transportSubType_, socketTransport);
            subTransportMap_.erase(subSocketFd);
        } else {
            MK_LOG_WARN("[%d] Can't find subSocketFd %d", transportSubType_, subSocketFd);
        }
    }
    return 0;
}

int MockSockTransport::RegisterServer()
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    if (isServerStarted_) {
        MK_LOG_DEBUG("Server Already Started!");
        return -1;
    }
    std::unique_lock<std::mutex> lck(threadMutex_);
    // Socket generation
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        MK_LOG_ERROR("Error socket:%d", errno);
        return -1;
    }
    int one = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0) {
        MK_LOG_ERROR("Error setsockopt:%d", errno);
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int)) < 0) {
        MK_LOG_ERROR("Error setsockopt:%d", errno);
        return -1;
    }
    struct sockaddr_in addr;
    memset_s(&addr, sizeof(sockaddr_in), 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(remotePort_);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        MK_LOG_ERROR("Error bind:%d, port[%d], fd[%d]", errno, remotePort_, sockfd);
        return -1;
    }
    if (listen(sockfd, SOMAXCONN) < 0) {
        MK_LOG_ERROR("Error listen:%d", errno);
        close(sockfd);
        return -1;
    }
    MockSockTransport::lastServerFd_ = sockfd;
    localSockfd_ = sockfd;
    isServerStarted_ = true;

    FD_ZERO(&readfds_);
    FD_SET(sockfd, &readfds_);

    receiveHandle_ = std::make_unique<std::thread>(std::bind(&MockSockTransport::ReceiveThread, this));
    receiveHandle_->detach();
    isCallObserver_ = true;
    MK_LOG_DEBUG("RegisterServer at port 0x%04X", remotePort_);
    return 0;
}

int MockSockTransport::RemoveServer(bool isDisable)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    std::unique_lock<std::mutex> lck(threadMutex_);

    if (isServerStarted_) {
        isServerStarted_ = false;
        threadRunning_ = false;
        WaitReceiveToEnd();
        {
            std::lock_guard<std::recursive_mutex> incomingLock(incomingMutex_);
            for (auto it = incomingConnectMap_.begin(); it != incomingConnectMap_.end();) {
                int ret = CloseFd(it->second, nullptr);
                MK_LOG_WARN("Remove incoming close:%d result:%d", it->second, ret);
                it = incomingConnectMap_.erase(it);
            }
        }
        {
            std::lock_guard<std::recursive_mutex> subTranLock(subTransportMutex_);
            for (auto it = subTransportMap_.begin(); it != subTransportMap_.end();) {
                int ret = CloseFd(it->first, it->second);
                MK_LOG_WARN("Remove subtransport close:%d result:%d", it->first, ret);
                MK_LOG_WARN("Remove subTransport %p", it->second);
                it = subTransportMap_.erase(it);
            }
        }

        CloseFd(localSockfd_, this);

        mainDispatcher_->Uninitialize();
    }
    return 0;
}

int MockSockTransport::AcceptConnection(const RawAddress &addr, uint16_t port)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    std::string ip = addr.GetAddress();
    transportContext_->PostTask(std::bind(&MockSockTransport::AcceptConnectionThread, this, ip, port));
    return 0;
}

void MockSockTransport::AcceptConnectionThread(std::string ip, uint8_t port)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    if (incomingConnectMap_.find(ip) != incomingConnectMap_.end()) {
        int newConnect = incomingConnectMap_.at(ip);
        MK_LOG_DEBUG("add new fd to readfds_:%d", newConnect);
        FD_SET(newConnect, &readfds_);
        {
            std::lock_guard<std::recursive_mutex> subTranLock(subTransportMutex_);
            // this pointer need be free in the call side
            auto newSockTransport =
                new MockSockTransport(newConnect, *this, observer_, transportType_, *transportContext_);
            subTransportMap_[newConnect] = newSockTransport;
        }

        send(newConnect, ACCEPT, ACCEPT_REJECT_SIZE, MSG_DONTWAIT);
        if (isCallObserver_) {
            observer_.OnConnected(subTransportMap_.at(newConnect), MAX_SEND_PACKET_SIZE, MAX_RECEIVE_PACKET_SIZE);
        }
    }
}

int MockSockTransport::RejectConnection(const RawAddress &addr, uint16_t port)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    std::string ip = addr.GetAddress();
    transportContext_->PostTask(std::bind(&MockSockTransport::RejectConnectionThread, this, ip, port));
    return 0;
}

void MockSockTransport::RejectConnectionThread(std::string ip, uint8_t port)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    {
        std::lock_guard<std::recursive_mutex> incomingLock(incomingMutex_);
        if (incomingConnectMap_.find(ip) != incomingConnectMap_.end()) {
            int newSockFd = incomingConnectMap_.at(ip);
            incomingConnectMap_.erase(ip);
            send(newSockFd, REJECT, ACCEPT_REJECT_SIZE, MSG_DONTWAIT);
            CloseFd(newSockFd, nullptr);
        }
    }
}

static constexpr size_t DATA_SPLIT_SIZE = 20;
static const uint8_t *DATA_SPLIT_START = (const uint8_t *)"{DATA_SPLIT_000000}";
static const uint8_t *DATA_SPLIT_END = (const uint8_t *)"{DATA_SPLIT_FFFFFF}";

void MockSockTransport::AddToPacketsMap()
{
    MK_LOG_DEBUG("Call %s socketFD %d START", __PRETTY_FUNCTION__, localSockfd_);
    std::unique_lock<std::mutex> lck(receivedPacketsMutex_);
    uint8_t buf[1];
    while (true) {
        int cnt = recv(localSockfd_, buf, sizeof(buf), MSG_DONTWAIT);
        if (cnt <= 0) {
            break;
        }
        receivedBuf_.push_back(buf[0]);
    }
    transportContext_->PostTask(std::bind(&MockSockTransport::FindNewDataThread, this));
    MK_LOG_DEBUG("Call %s socketFD %d END", __PRETTY_FUNCTION__, localSockfd_);
}

void MockSockTransport::FindNewDataThread()
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    if (FindNewData(receivedBuf_)) {
        transportContext_->PostTask(std::bind(&MockSockTransport::FindNewDataThread, this));
    }
}

bool MockSockTransport::FindNewData(std::vector<uint8_t> &receivedBuf)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    size_t size = receivedBuf.size();
    if (size < (DATA_SPLIT_SIZE + DATA_SPLIT_SIZE)) {
        MK_LOG_DEBUG("size < (DATA_SPLIT_SIZE + DATA_SPLIT_SIZE)");
        return false;
    }
    uint8_t *dataBuf = receivedBuf.data();
    int packetStartIdx = -1;
    int packetEndIdx = -1;
    for (size_t i = 0; i < size; i++) {
        const uint8_t *tmpBuf = dataBuf + i;
        if (packetStartIdx != -1 && memcmp(tmpBuf, DATA_SPLIT_END, DATA_SPLIT_SIZE) == 0) {
            // new packet end
            packetEndIdx = i + DATA_SPLIT_SIZE - 1;
            break;
        } else if (packetEndIdx == -1 && memcmp(tmpBuf, DATA_SPLIT_START, DATA_SPLIT_SIZE) == 0) {
            // new packet start
            packetStartIdx = i;
            continue;
        }
    }
    if (packetStartIdx != -1 && packetEndIdx != -1 && packetEndIdx > packetStartIdx) {
        MK_LOG_DEBUG("find packet %d - %d ", packetStartIdx, packetEndIdx);
        size_t dataSize = packetEndIdx - packetStartIdx + 1 - (DATA_SPLIT_SIZE + DATA_SPLIT_SIZE);
        Packet *obexPacket = PacketMalloc(0, 0, dataSize);
        PacketPayloadWrite(obexPacket, dataBuf + packetStartIdx + DATA_SPLIT_SIZE, 0, dataSize);
        receivedPackets.push_back(obexPacket);
        receivedBuf.erase(receivedBuf.begin(), receivedBuf.begin() + packetEndIdx + 1);
        uint32_t index = availableIndex_++;
        std::cout << GetColor() << "Add Packet To Read Job List!" << index << RESET_COLOR << std::endl;
        MK_LOG_DEBUG("Add Packet To Read List!:%d", index);
        NewDataAvailableThread(index);
        return true;
    }
    MK_LOG_DEBUG("not find packet");
    return false;
}

void MockSockTransport::NewDataAvailableThread(uint32_t index)
{
    MK_LOG_DEBUG("[%d] Call %s socketFd %d START", transportSubType_, __PRETTY_FUNCTION__, localSockfd_);
    MK_LOG_DEBUG("observer_.OnDataAvailable at %d", index);
    if (isCallObserver_) {
        if (transportType_ == TransportType::L2CAP) {
            Packet *pkt = nullptr;
            Read(&pkt);
            if (pkt != nullptr) {
                observer_.OnDataAvailable(this, pkt);
            }
        } else {
            observer_.OnDataAvailable(this);
        }
    }
    MK_LOG_DEBUG("Call %s socketFd %d END", __PRETTY_FUNCTION__, localSockfd_);
}

// This function is used to get the data sent by the peer from RFCOMM/L2cap
int MockSockTransport::Read(Packet **pkt)
{
    MK_LOG_DEBUG("Call %s socketFd %d", __PRETTY_FUNCTION__, localSockfd_);
    std::unique_lock<std::mutex> lck(receivedPacketsMutex_);
    if (receivedPackets.size() < 1) {
        MK_LOG_ERROR("Read Error!");
        return -1;
    }
    uint32_t index = readIndex_++;
    Packet *packet = receivedPackets.front();
    MK_LOG_DEBUG("Read Packet at %d", index);
    if (packet != nullptr) {
        std::cout << GetColor() << "Read at " << index << "\n"
                  << ObexUtils::ToDebugString(*packet) << RESET_COLOR << std::endl;
    }
    receivedPackets.pop_front();
    MK_LOG_DEBUG("left receivedPackets size %d", receivedPackets.size());

    if (packet != nullptr) {
        *pkt = packet;
        return 0;
    }
    return -1;
}

// This function is used to write the data to RFCOMM/L2cap
int MockSockTransport::Write(Packet *pkt)
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    Packet *newPacket = PacketRefMalloc(pkt);
    uint32_t index = writeIndex_++;
    uint32_t count = transportContext_->AddWritePkgCount();
    MK_LOG_DEBUG("Add Packet To Write Job List!:%d , remain pkt size %d", index, count);
    if (isCallObserver_ && count == BUSY_WRITE_PKT_COUNT) {
        transportContext_->PostTask(std::bind(&DataTransportObserver::OnDataBusy, &observer_, this, 1));
    }
    Sleep(SLEEP_MS);
    transportContext_->PostTask(std::bind(&MockSockTransport::WriteThread, this, newPacket, index));
    return 0;
}

void MockSockTransport::WriteThread(Packet *pkt, uint32_t index)
{
    MK_LOG_DEBUG("[%d] Call %s", transportSubType_, __PRETTY_FUNCTION__);
    MK_LOG_DEBUG("[%d] Send data at sockfd:%d", transportSubType_, localSockfd_);

    Buffer *tmpBuffer = PacketContinuousPayload(pkt);
    uint8_t *packetBuf = (uint8_t *)BufferPtr(tmpBuffer);
    size_t packetBufSize = BufferGetSize(tmpBuffer);
    size_t totalSize = packetBufSize + (DATA_SPLIT_SIZE + DATA_SPLIT_SIZE);
    MK_LOG_DEBUG("[%d] PacketPayloadSize:%d", transportSubType_, packetBufSize);
    auto buf = std::make_unique<uint8_t[]>(totalSize);
    memcpy_s(buf.get(), DATA_SPLIT_SIZE, DATA_SPLIT_START, DATA_SPLIT_SIZE);
    memcpy_s(buf.get() + DATA_SPLIT_SIZE, packetBufSize, packetBuf, packetBufSize);
    memcpy_s(buf.get() + packetBufSize + DATA_SPLIT_SIZE, DATA_SPLIT_SIZE, DATA_SPLIT_END, DATA_SPLIT_SIZE);
    MK_LOG_DEBUG("[%d] Write at %d", transportSubType_, index);
    std::cout << GetColor() << "Write at " << index << ":\n"
              << ObexUtils::ToDebugString(packetBuf, packetBufSize, true).c_str() << RESET_COLOR << std::endl;
    PacketFree(pkt);
    int cnt = send(localSockfd_, buf.get(), totalSize, MSG_DONTWAIT);
    uint32_t count = transportContext_->MinusWritePkgCount();
    MK_LOG_DEBUG("[%d] Remain pkt size %d", transportSubType_, count);
    if (isCallObserver_ && count == UNBUSY_WRITE_PKT_COUNT) {
        observer_.OnDataBusy(this, 0);
    }
    MK_LOG_INFO("[%d] Sended data size:%d", transportSubType_, cnt);
}

RawAddress MockSockTransport::GetRemoteAddress()
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);

    if (remoteAddr_ == nullptr) {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        getpeername(localSockfd_, (struct sockaddr *)&addr, &addr_size);
        uint8_t btAddr[6];
        memset_s(btAddr, sizeof(btAddr), 0x00, sizeof(btAddr));
        memcpy_s(btAddr, sizeof(btAddr), &addr.sin_addr, sizeof(addr.sin_addr));
        remoteAddr_ = std::make_unique<RawAddress>(RawAddress::ConvertToString(btAddr, sizeof(btAddr)));
    }
    return *remoteAddr_;
}

void MockSockTransport::ConnectThread()
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);

    // Address generation
    struct sockaddr_in addr;
    memset_s(&addr, sizeof(sockaddr_in), 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;  // Address family(ipv4)
    addr.sin_port = htons(remotePort_);
    uint8_t btAddr[RawAddress::BT_ADDRESS_BYTE_LEN];
    memset_s(btAddr, sizeof(btAddr), 0x00, sizeof(btAddr));
    remoteAddr_->ConvertToUint8(btAddr, sizeof(btAddr));
    memcpy_s(&addr.sin_addr.s_addr, sizeof(addr.sin_addr.s_addr), btAddr, sizeof(addr.sin_addr.s_addr));
    uint8_t *ip = (uint8_t *)static_cast<uint32_t *>(&addr.sin_addr.s_addr);
    MK_LOG_DEBUG("Connect to IP:%s", bluetooth::ObexUtils::ToDebugString(ip, sizeof(addr.sin_addr.s_addr)).c_str());
    isConnected_ = false;

    int ret = connect(localSockfd_, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret == 0) {
        MK_LOG_DEBUG("Add new fd to readfds_:%d", localSockfd_);
        FD_SET(localSockfd_, &readfds_);
        ioctl(localSockfd_, FIONBIO, 1);  // no-block mode
        isConnected_ = true;
        receiveHandle_ = std::make_unique<std::thread>(std::bind(&MockSockTransport::ReceiveThread, this));
        receiveHandle_->detach();
        return;
    }
    perror("Socket Connect fail");
}

bool MockSockTransport::HandleReceiveClient(fd_set &fds, struct timeval &tv)
{
    int ret = select(localSockfd_ + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0) {
        return true;
    }
    if (FD_ISSET(localSockfd_, &fds)) {
        uint8_t buf[ACCEPT_REJECT_SIZE];
        memset_s(buf, sizeof(buf), 0x00, ACCEPT_REJECT_SIZE);
        int cnt = recv(localSockfd_, buf, ACCEPT_REJECT_SIZE, MSG_PEEK);
        if (cnt == 0) {
            MK_LOG_DEBUG("Receive close from server:%d, at transport %p", localSockfd_, this);
            FD_CLR(localSockfd_, &readfds_);
            transportContext_->PostTask(std::bind(&MockSockTransport::ClientDisconnectedThread, this));
            return false;
        }
        if (cnt < 0) {
            perror("Socket2");
            MK_LOG_ERROR("Socket2 :localSockfd_=%d", localSockfd_);
            if (isCallObserver_) {
                observer_.OnTransportError(this, cnt);
            }
            return true;
        }
        if (!isConnectedReceivedData_ && cnt == ACCEPT_REJECT_SIZE && memcmp(buf, ACCEPT, ACCEPT_REJECT_SIZE) == 0) {
            MK_LOG_DEBUG("Receive data from server:%d", localSockfd_);
            recv(localSockfd_, buf, ACCEPT_REJECT_SIZE, MSG_DONTWAIT);
            isConnectedReceivedData_ = true;
            isCallObserver_ = true;
            if (isCallObserver_) {
                observer_.OnConnected(this, MAX_SEND_PACKET_SIZE, MAX_RECEIVE_PACKET_SIZE);
            }
        } else if (!isConnectedReceivedData_ && cnt == ACCEPT_REJECT_SIZE &&
                   memcmp(buf, REJECT, ACCEPT_REJECT_SIZE) == 0) {
            MK_LOG_DEBUG("Receive data from server:%d", localSockfd_);
            recv(localSockfd_, buf, ACCEPT_REJECT_SIZE, MSG_DONTWAIT);
            observer_.OnTransportError(this, CONNECT_FAIL);
            isCallObserver_ = false;
            CloseFd(localSockfd_, nullptr);
        } else if (isConnectedReceivedData_) {
            MK_LOG_DEBUG("Receive data from server:%d", localSockfd_);
            AddToPacketsMap();
        } else {
            MK_LOG_ERROR("Not Received data before accept!");
            int bufLen = MAX_RECEIVE_PACKET_SIZE;
            auto maxBuf = std::make_unique<uint8_t[]>(bufLen);
            recv(localSockfd_, maxBuf.get(), bufLen, MSG_DONTWAIT);
        }
    }
    return true;
}

bool MockSockTransport::HandleReceiveServerMainFd()
{
    // Waiting for connection
    struct sockaddr_in get_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int connect = accept(localSockfd_, (struct sockaddr *)&get_addr, &len);
    if (connect < 0) {
        return true;
    }
    uint8_t addr[RawAddress::BT_ADDRESS_BYTE_LEN];
    memset_s(addr, sizeof(addr), 0x00, sizeof(addr));
    memcpy_s(addr, sizeof(addr), &get_addr.sin_addr, sizeof(get_addr.sin_addr));
    RawAddress rawAddr = RawAddress::ConvertToString(addr, sizeof(addr));
    std::string ip = rawAddr.GetAddress();
    MK_LOG_DEBUG("Connect from IP %s:%d with sockfd=%d", ip.c_str(), get_addr.sin_port, connect);
    {
        std::lock_guard<std::recursive_mutex> incomingLock(incomingMutex_);
        if (incomingConnectMap_.find(ip) != incomingConnectMap_.end()) {
            MK_LOG_DEBUG("Close duplicate connected %s:%d", ip.c_str(), get_addr.sin_port);
            CloseFd(connect, nullptr);
            return true;
        }
        isCallObserver_ = true;
        ioctl(connect, FIONBIO, 1);  // no-block mode
        incomingConnectMap_[ip] = connect;
    }
    if (isCallObserver_) {
        observer_.OnConnectIncoming(rawAddr, get_addr.sin_port);
    }
    return true;
}

int MockSockTransport::HandleReceiveServerSubFd(fd_set &fds, int fd, DataTransport *transport)
{
    uint8_t buf[1] = {0};
    int cnt = recv(fd, buf, sizeof(buf), MSG_PEEK);
    if (cnt == 0) {
        MK_LOG_DEBUG("Receive close from client:%d", fd);
        FD_CLR(fd, &readfds_);
        {
            std::lock_guard<std::recursive_mutex> incomingLock(incomingMutex_);
            for (auto icm = incomingConnectMap_.begin(); icm != incomingConnectMap_.end();) {
                if (icm->second == fd) {
                    icm = incomingConnectMap_.erase(icm);
                    break;
                }
                ++icm;
            }
        }
    } else if (cnt < 0) {
        perror("Socket3");
        if (isCallObserver_) {
            observer_.OnTransportError(transport, cnt);
        }
    } else {
        MK_LOG_DEBUG("Receive data from client:%d, cnt=%d", fd, cnt);
        ((MockSockTransport *)transport)->AddToPacketsMap();
    }
    return cnt;
}

bool MockSockTransport::HandleReceiveServer(fd_set &fds, struct timeval &tv)
{
    int maxFd = localSockfd_;
    {
        std::lock_guard<std::recursive_mutex> subTranLock(subTransportMutex_);
        for (auto &pair : subTransportMap_) {
            int fd = pair.first;
            if (fd > maxFd) {
                maxFd = fd;
            }
        }
    }
    int ret = select(maxFd + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0) {
        return true;
    }
    if (FD_ISSET(localSockfd_, &fds)) {
        MK_LOG_DEBUG("Accppt Connect...");
        return HandleReceiveServerMainFd();
    }
    MK_LOG_DEBUG("Select FD with return:%d", ret);
    std::lock_guard<std::recursive_mutex> subTranLock(subTransportMutex_);
    MK_LOG_DEBUG("transportMap_ size:%d", subTransportMap_.size());
    for (auto it = subTransportMap_.begin(); it != subTransportMap_.end();) {
        if (it->first <= 0) {
            MK_LOG_DEBUG("it->first < 0 break");
            break;
        }
        if (!FD_ISSET(it->first, &fds)) {
            ++it;
            continue;
        }
        int cnt = HandleReceiveServerSubFd(fds, it->first, it->second);
        if (cnt != 0) {
            ++it;
            continue;
        }
        DataTransport *transport = it->second;
        MK_LOG_DEBUG("Remove subTransport %p", transport);
        it = subTransportMap_.erase(it);
        if (isCallObserver_) {
            isCallObserver_ = false;
            observer_.OnDisconnected(transport);
        }
    }
    return true;
}

void MockSockTransport::ReceiveThread()
{
    MK_LOG_DEBUG("Call %s", __PRETTY_FUNCTION__);
    threadEnd_ = false;
    fd_set fds;
    struct timeval tv;
    tv.tv_sec = 0x03;
    tv.tv_usec = 0;
    MK_LOG_DEBUG("ReceiveThread/Accect start...");
    threadRunning_ = true;
    while (true) {
        if (!threadRunning_) {
            break;
        }
        memcpy_s(&fds, sizeof(fd_set), &readfds_, sizeof(fd_set));
        if (isConnected_) {  // CLient monitor
            if (!HandleReceiveClient(fds, tv)) {
                break;
            }
        } else if (isServerStarted_) {  // Server monitor
            if (!HandleReceiveServer(fds, tv)) {
                break;
            }
        }
    }
    threadEnd_ = true;
    MK_LOG_DEBUG("ReceiveThread/Accect end...");
}

void MockSockTransport::WaitReceiveToEnd()
{
    threadRunning_ = false;
    if (receiveHandle_ != nullptr) {
        while (!threadEnd_) {
            MK_LOG_DEBUG("Wait receive thread to end!");
            Sleep(SLEEP_MS);
        }
        receiveHandle_ = nullptr;
    }
}

void MockSockTransport::Sleep(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int MockSockTransport::CloseClient()
{
    MK_LOG_DEBUG("Call %s ,%d", __PRETTY_FUNCTION__, lastClientFd_);
    return close(lastClientFd_);
}

int MockSockTransport::CloseServer()
{
    MK_LOG_DEBUG("Call %s ,%d", __PRETTY_FUNCTION__, lastServerFd_);
    return close(lastServerFd_);
}

const std::string &MockSockTransport::GetColor()
{
    switch (transportSubType_) {
        case TransportSubType::SERVER_MAIN:
        case TransportSubType::SERVER_SUB:
            return SERVER_COLOR;
        default:
            return CLIENT_COLOR;
    }
}

int MockSockTransport::CloseFd(int socketFd, DataTransport *transport)
{
    MK_LOG_DEBUG("[%d] Call %s ,%d", transportSubType_, __PRETTY_FUNCTION__, socketFd);
    FD_CLR(socketFd, &readfds_);
    transportContext_->PostTask(std::bind(&MockSockTransport::CloseFdThread, this, socketFd, transport));
    return 0;
}

void MockSockTransport::CloseFdThread(int socketFd, DataTransport *transport)
{
    MK_LOG_DEBUG("[%d] Call %s ,closeFd %d", transportSubType_, __PRETTY_FUNCTION__, socketFd);
    close(socketFd);
    if (transport != nullptr) {
        observer_.OnDisconnectSuccess(transport);
    }
}

MockSockTransport::TransportContext::TransportContext(utility::Dispatcher &dispatcher) : dispatcher_(dispatcher)
{}

void MockSockTransport::TransportContext::PostTask(const std::function<void()> &task)
{
    dispatcher_.PostTask(task);
}

uint32_t MockSockTransport::TransportContext::AddWritePkgCount()
{
    mutex_.lock();
    writePktCount_++;
    uint32_t count = writePktCount_;
    mutex_.unlock();
    return count;
}

uint32_t MockSockTransport::TransportContext::MinusWritePkgCount()
{
    mutex_.lock();
    writePktCount_--;
    uint32_t count = writePktCount_;
    mutex_.unlock();
    return count;
}
}  // namespace bluetooth
