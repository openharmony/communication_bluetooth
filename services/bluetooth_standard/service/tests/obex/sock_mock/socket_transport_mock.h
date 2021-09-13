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
#ifndef SOCKET_TRANSPORT_MOCK_H
#define SOCKET_TRANSPORT_MOCK_H

#include <atomic>
#include <list>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "dispatcher.h"
#include "rfcomm.h"
#include "transport.h"

namespace bluetooth {
class MockSockTransport : public DataTransport {
public:
    class TransportContext {
    public:
        TransportContext(utility::Dispatcher &dispatcher);
        virtual ~TransportContext() = default;
        void PostTask(const std::function<void()> &task);
        uint32_t AddWritePkgCount();
        uint32_t MinusWritePkgCount();

    private:
        utility::Dispatcher &dispatcher_;
        std::mutex mutex_{};
        uint32_t writePktCount_{0};
    };
    enum class TransportType : uint8_t { RFCOMM, L2CAP };
    enum class TransportSubType : uint8_t { SERVER_MAIN, SERVER_SUB, CLIENT };
    explicit MockSockTransport(const RawAddress *addr, uint16_t lpsm, uint16_t rpsm, uint16_t mtu,
        DataTransportObserver &observer, TransportType TransportType);
    explicit MockSockTransport(int newSocketFd, MockSockTransport &parentSockTransport, DataTransportObserver &observer,
        const TransportType &transportType, TransportContext &transportContext);
    virtual ~MockSockTransport();

    void Init();
    int Connect() override;
    int Disconnect() override;

    int RegisterServer() override;
    int RemoveServer(bool isDisable = true) override;

    int AcceptConnection(const RawAddress &addr, uint16_t port) override;
    int RejectConnection(const RawAddress &addr, uint16_t port) override;

    int Read(Packet **pkt) override;
    int Write(Packet *pkt) override;
    RawAddress GetRemoteAddress() override;

    void AddToPacketsMap();

    static int CloseClient();
    static int CloseServer();

protected:
private:
    void ConnectThread();
    void DisconnectClientThread();
    void ClientDisconnectedThread();

    void ReceiveThread();
    void WaitReceiveToEnd();
    void WriteThread(Packet *pkt, uint32_t index);
    void AcceptConnectionThread(std::string ip, uint8_t port);
    void RejectConnectionThread(std::string ip, uint8_t port);

    bool HandleReceiveClient(fd_set &fds, struct timeval &tv);
    bool HandleReceiveServer(fd_set &fds, struct timeval &tv);
    bool HandleReceiveServerMainFd();
    int HandleReceiveServerSubFd(fd_set &fds, int fd, DataTransport *transport);

    int RemoveSocketFd(int subSocketFd);
    bool FindNewData(std::vector<uint8_t> &receivedBuf);
    void FindNewDataThread();
    void NewDataAvailableThread(uint32_t index);

    int CloseFd(int socketFd, DataTransport *transport);
    void CloseFdThread(int socketFd, DataTransport *transport);
    static void Sleep(int ms);
    const std::string &GetColor();
    std::atomic_bool threadRunning_{false};
    std::atomic_bool threadEnd_{false};
    std::atomic_bool isConnected_{false};
    std::atomic_bool isConnectedReceivedData_{false};
    std::atomic_bool isServerStarted_{false};
    std::atomic_bool isCallObserver_{false};
    fd_set readfds_{};

    int localSockfd_{0};
    MockSockTransport *parentSockTransport_{nullptr};
    std::unique_ptr<RawAddress> remoteAddr_{nullptr};
    uint16_t remotePort_{0};
    DataTransportObserver &observer_;
    TransportType transportType_ = TransportType::RFCOMM;
    TransportSubType transportSubType_ = TransportSubType::SERVER_MAIN;
    std::unique_ptr<std::thread> receiveHandle_{nullptr};

    std::recursive_mutex incomingMutex_{};
    std::unordered_map<std::string, int> incomingConnectMap_{};
    std::recursive_mutex subTransportMutex_{};
    std::unordered_map<int, DataTransport *> subTransportMap_{};

    std::mutex receivedPacketsMutex_{};
    std::vector<uint8_t> receivedBuf_{};
    std::list<Packet *> receivedPackets{};

    TransportContext *mainTransportContext_ = nullptr;
    TransportContext *transportContext_ = nullptr;
    utility::Dispatcher *mainDispatcher_ = nullptr;

    std::mutex threadMutex_{};

    static int lastClientFd_;
    static int lastServerFd_;
    std::atomic_uint32_t availableIndex_{0};
    std::atomic_uint32_t readIndex_{0};
    std::atomic_uint32_t writeIndex_{0};
    MockSockTransport(const bluetooth::MockSockTransport &) = delete;
    MockSockTransport operator=(const bluetooth::MockSockTransport &) = delete;
};
}  // namespace bluetooth
#endif  // SOCKET_TRANSPORT_MOCK_H
