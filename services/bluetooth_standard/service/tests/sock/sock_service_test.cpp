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

#include <sys/socket.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "socket_service.h"
#include "socket_util.h"
#include "dispatcher.h"
#include "stack_mock.h"
#include "common_mock_all.h"
#include "transport_l2cap.h"
#include "securec.h"

using namespace testing;
using namespace bluetooth;

extern AdapterManagerMock *g_amMock_;
extern Mock_Stack *g_stackmocker;
extern SdpSearchCbParam g_sdpCb_param;
extern RfcommEventCb g_rfcCb;
extern L2capEventCb g_l2cCb;
extern L2capConnectReqCb g_l2cConReqCb;

class TestDataTransportObserver : public DataTransportObserver {
public:
    void OnConnectIncoming(const RawAddress &addr, uint16_t port) override;
    void OnIncomingDisconnected(const RawAddress &addr) override;
    void OnConnected(DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU) override;
    void OnDisconnected(DataTransport *transport) override;
    void OnDisconnectSuccess(DataTransport *transport) override;
    void OnDataAvailable(DataTransport *transport) override;
    void OnDataAvailable(DataTransport *transport, Packet *pkt) override;
    void OnDataBusy(DataTransport *transport, uint8_t isBusy) override;
    void OnTransportError(DataTransport *transport, int errType) override;
};
void TestDataTransportObserver::OnConnectIncoming(const RawAddress &addr, uint16_t port)
{
    std::cout << "TestDataTransportObserver::OnConnectIncoming" << std::endl;
}
void TestDataTransportObserver::OnIncomingDisconnected(const RawAddress &addr)
{
    std::cout << "TestDataTransportObserver::OnIncomingDisconnected" << std::endl;
}
void TestDataTransportObserver::OnConnected(DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU)
{
    std::cout << "TestDataTransportObserver::OnConnected" << std::endl;
}
void TestDataTransportObserver::OnDisconnected(DataTransport *transport)
{
    std::cout << "TestDataTransportObserver::OnDisconnected" << std::endl;
}
void TestDataTransportObserver::OnDisconnectSuccess(DataTransport *transport)
{
    std::cout << "TestDataTransportObserver::OnDisconnectSuccess" << std::endl;
}
void TestDataTransportObserver::OnDataAvailable(DataTransport *transport)
{
    std::cout << "TestDataTransportObserver::OnDataAvailable" << std::endl;
}
void TestDataTransportObserver::OnDataAvailable(DataTransport *transport, Packet *pkt)
{
    std::cout << "TestDataTransportObserver::OnDataAvailable" << std::endl;
}
void TestDataTransportObserver::OnTransportError(DataTransport *transport, int errType)
{
    std::cout << "TestDataTransportObserver::OnTransportError" << std::endl;
}
void TestDataTransportObserver::OnDataBusy(DataTransport *transport, uint8_t isBusy)
{
    std::cout << "TestDataTransportObserver::OnDataBusy" << std::endl;
}

class SockTest : public Test {
public:
    SocketService *sockServcie_ = NULL;
    SdpProtocolDescriptor protocol[1] = {0};
    int sendBufSize = 5;  //*1000
    std::string addr = "00:00:00:00:00:FF";
    uint16_t uuid = 0x0011;
    RfcommIncomingInfo rfcEventData_;
    L2capConnectionInfo l2cConnectInfo_;
    L2capConfigInfo l2cConfigInfo_;
    DataTransportObserver *observer_ = NULL;
    utility::Dispatcher *dispatcher_ = NULL;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    StrictMock<PowerManagerInstanceMock> pminstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;

    void SetSdpCallbackParam();

    void initRfcommIncomingInfo()
    {
        rfcEventData_.addr = g_sdpCb_param.btaddr;
        rfcEventData_.scn = 1;
    }

    void StartService()
    {
        sockServcie_->Enable();
        usleep(10000);
    }

    void initL2capIncomingInfo()
    {
        l2cConnectInfo_.addr = g_sdpCb_param.btaddr;
        l2cConnectInfo_.handle = 1;
    }

    void initL2capConfigInfo()
    {
        L2capOptionRfc rfc = {.mode = 1,
            .maxTransmit = 0,
            .txWindowSize = 1,
            .rxWindowSize = 0,
            .retransmissionTimeout = 0,
            .monitorTimeout = 0,
            .mps = 1};

        l2cConfigInfo_.fcs = 1;
        l2cConfigInfo_.flushTimeout = 30;
        l2cConfigInfo_.mtu = 1500;
        l2cConfigInfo_.rfc = rfc;
    }

    void StopService()
    {
        sockServcie_->Disable();
        usleep(10000);
    }

    void listen()
    {
        EXPECT_EQ(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));
    }

    void connect()
    {
        EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Connect(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));
        usleep(10000);
    }

    void rfcCbConnectSuccess()
    {
        g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_SUCCESS, &rfcEventData_, g_rfcCb.context);
    }

    virtual void SetUp()
    {
        SetSdpCallbackParam();
        initRfcommIncomingInfo();
        initL2capIncomingInfo();
        initL2capConfigInfo();

        g_amMock_ = new AdapterManagerMock();
        EXPECT_CALL(*g_amMock_, AdapterManager_GetState()).Times(AtLeast(0)).WillRepeatedly(Return(STATE_TURN_ON));

        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        sockServcie_ = new SocketService();
        sockServcie_->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(sockServcie_));

        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_SOCKET_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));

        // Mock PowerManager & PowerManagerInstance
        registerNewMockPowerManager(&pminstanceMock_);
        EXPECT_CALL(pminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_,_,_)).Times(AtLeast(0)).WillRepeatedly(Return());

        g_stackmocker = new Mock_Stack;
        EXPECT_CALL(*g_stackmocker, RFCOMM_AssignServerNum()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, RFCOMM_RegisterServer()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, RFCOMM_ConnectChannel()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, RFCOMM_GetPeerAddress())
            .Times(AtLeast(0))
            .WillRepeatedly(Return(g_sdpCb_param.btaddr));
        EXPECT_CALL(*g_stackmocker, RFCOMM_DeregisterServer()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddServiceClassIdList()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddProtocolDescriptorList()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddBluetoothProfileDescriptorList()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddAttribute()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddBrowseGroupList()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_ServiceSearchAttribute()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_RegisterServiceRecord()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_DeregisterServiceRecord()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_DestroyServiceRecord()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, SDP_AddServiceName()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_ConnectReq()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_DisconnectionReq()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_RegisterService()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_DeregisterService()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_ConnectRsp()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_ConfigReq()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_SendData()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_DisconnectionRsp()).Times(AtLeast(0));
        EXPECT_CALL(*g_stackmocker, L2CIF_ConfigRsp()).Times(AtLeast(0));
        SocketService *socketService =
            (SocketService *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
        dispatcher_ = socketService->GetDispatcher();
    }

    virtual void TearDown()
    {
        if (sockServcie_ != nullptr) {
            sockServcie_->GetContext()->Uninitialize();
            delete sockServcie_;
        }
        if (g_amMock_) {
            delete g_amMock_;
            g_amMock_ = nullptr;
        }
        if (g_stackmocker) {
            delete g_stackmocker;
            g_stackmocker = NULL;
        }
    }
};

void SockTest::SetSdpCallbackParam()
{
    // Param of SdpSearchCallback
    protocol[0].protocolUuid.uuid16 = UUID_PROTOCOL_RFCOMM;
    protocol[0].parameter[0].type = SDP_TYPE_UINT_8;
    g_sdpCb_param.btaddr = {{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    g_sdpCb_param.serviceArray.descriptorNumber = 1;
    g_sdpCb_param.serviceArray.descriptor = &protocol[0];
    g_sdpCb_param.serviceNum = 1;
}

int RecvSocketFd(int socketFd_)
{
    struct msghdr msg;
    memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    int rv = 0;
    int cfd = -1;
    int clientFd = -1;
    char ccmsg[CMSG_SPACE(sizeof(cfd))];
    struct cmsghdr *cmptr;
    char buffer[10];

    msg.msg_control = ccmsg;
    msg.msg_controllen = sizeof(ccmsg);
    struct iovec io;
    io.iov_base = buffer;
    io.iov_len = 10;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    rv = recvmsg(socketFd_, &msg, MSG_NOSIGNAL);
    printf("RecvSocketFd ret: %d", rv);
    if (rv == -1) {
        if (errno == EAGAIN) {
            return 0;
        } else {
            printf("sendmsg error, file fd is %d", socketFd_);
            return -1;
        }
    }

    cmptr = CMSG_FIRSTHDR(&msg);
    if ((cmptr != NULL) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int)))) {
        if (cmptr->cmsg_level != SOL_SOCKET) {
            printf("control level != SOL_SOCKET");
            return -1;
        }
        if (cmptr->cmsg_type != SCM_RIGHTS) {
            printf("control type != SCM_RIGHTS");
            return -1;
        }

        clientFd = *((int *)CMSG_DATA(cmptr));
    } else {
        if (cmptr == NULL) {
            printf("null cmptr, fd not passed");
        } else {
            printf("message len: if incorrect. %ld", cmptr->cmsg_len);
        }
        return -1;
    }
    printf("receive client socket fd: %d", clientFd);

    uint8_t recvBuf[rv];
    memset_s(recvBuf, sizeof(recvBuf), 0, sizeof(recvBuf));
    memcpy_s(recvBuf, rv, (uint8_t *)msg.msg_iov[0].iov_base, rv);

    uint8_t recvAddrBuf[6];
    memset_s(recvAddrBuf, sizeof(recvAddrBuf), 0, sizeof(recvAddrBuf));
    memcpy_s(recvAddrBuf, 6, &recvBuf[1], 6);

    RawAddress rawAddr = RawAddress::ConvertToString(recvAddrBuf);
    std::string address = rawAddr.GetAddress().c_str();
    printf("accept address: %s", address.c_str());
    return clientFd;
}

void setSockSendBufSize(int dataFd_)
{
    int optVal = 0;
    socklen_t optLen = sizeof(optVal);

    int err = getsockopt(dataFd_, SOL_SOCKET, SO_SNDBUF, &optVal, &optLen);
    if (err < 0) {
        printf("Get sockets' send buf error!\n");
    }
    printf("dataFd_:%d, send buf is %d\n", dataFd_, optVal);

    optVal = 5120;
    err = setsockopt(dataFd_, SOL_SOCKET, SO_SNDBUF, &optVal, optLen);
    if (err < 0) {
        printf("Set sockets' send buf error!\n");
    }

    err = getsockopt(dataFd_, SOL_SOCKET, SO_SNDBUF, &optVal, &optLen);
    if (err < 0) {
        printf("Get sockets' send buf error!\n");
    }
    printf("dataFd_:%d, send buf is %d\n", dataFd_, optVal);
}

TEST_F(SockTest, Enable_Disable1)
{
    StartService();
    StopService();
}

TEST_F(SockTest, Enable_Disable2)
{
    StartService();

    int fd = sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    EXPECT_NE(SOCK_INVALID_FD, fd);
    if (fd > 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }

    EXPECT_CALL(*g_stackmocker, RFCOMM_DeregisterServer()).Times(AtLeast(1));
    StopService();
    usleep(10000);
}

TEST_F(SockTest, Connect1)
{
    StartService();

    connect();

    StopService();
}

TEST_F(SockTest, Connect2)
{
    protocol[0].protocolUuid.uuid16 = UUID_PROTOCOL_SDP;
    g_sdpCb_param.serviceArray.descriptor = &protocol[0];

    StartService();

    connect();

    StopService();
}

TEST_F(SockTest, Connect3)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_ServiceSearchAttribute()).WillOnce(Return(-1));
    EXPECT_EQ(SOCK_INVALID_FD, sockServcie_->Connect(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    StopService();
}

TEST_F(SockTest, Nonsense)
{
    RawAddress rawAddr("00:00:00:00:00:FF");
    EXPECT_EQ(0, sockServcie_->Connect(rawAddr));
    EXPECT_EQ(0, sockServcie_->Disconnect(rawAddr));
    std::list<RawAddress> devList;
    devList = sockServcie_->GetConnectDevices();
    EXPECT_EQ(0, (int)devList.size());
    EXPECT_EQ(0, sockServcie_->GetConnectState());
    EXPECT_EQ(0, sockServcie_->GetMaxConnectNum());
}

TEST_F(SockTest, RfcommEventCallback_CONNECT_INCOMING)
{
    StartService();

    connect();

    initRfcommIncomingInfo();
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_INCOMING, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_CONNECT_SUCCESS1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    rfcCbConnectSuccess();
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_CONNECT_SUCCESS2)
{
    StartService();

    connect();

    rfcCbConnectSuccess();
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_CONNECT_FAIL1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_FAIL, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_FAIL, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_CONNECT_FAIL2)
{
    StartService();

    connect();

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_FAIL, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECTED1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECTED, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECTED, &rfcEventData_, g_rfcCb.context);
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECTED2)
{
    StartService();

    connect();

    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECTED, &rfcEventData_, g_rfcCb.context);
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECT_SUCCESS1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_SUCCESS, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_SUCCESS, &rfcEventData_, g_rfcCb.context);
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECT_SUCCESS2)
{
    StartService();

    connect();

    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_SUCCESS, &rfcEventData_, g_rfcCb.context);
    usleep(10000);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECT_SUCCESS3)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));
    initRfcommIncomingInfo();
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_CONNECT_INCOMING, &rfcEventData_, g_rfcCb.context);
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECTED, &rfcEventData_, g_rfcCb.context);
    usleep(10000);
    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECT_FAIL1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_FAIL, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_FAIL, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_DISCONNECT_FAIL2)
{
    StartService();

    connect();

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_DISCONNECT_FAIL, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_FC_ON1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_FC_ON, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_FC_ON, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_FC_ON2)
{
    StartService();

    connect();

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_FC_ON, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_REV_DATA1)
{
    Packet *pkt1 = PacketMalloc(5, 5, 100);
    EXPECT_CALL(*g_stackmocker, RFCOMM_Read(_, _)).WillOnce(DoAll(SetArgPointee<1>(pkt1), Return(RFCOMM_SUCCESS)));

    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0));

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_REV_DATA, &rfcEventData_, g_rfcCb.context);
    rfcCbConnectSuccess();
    usleep(10000);
    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_REV_DATA, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, RfcommEventCallback_REV_DATA2)
{
    Packet *pkt1 = PacketMalloc(5, 5, 100);
    EXPECT_CALL(*g_stackmocker, RFCOMM_Read(_, _)).WillOnce(DoAll(SetArgPointee<1>(pkt1), Return(RFCOMM_SUCCESS)));

    StartService();

    connect();

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_REV_DATA, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, OnSocketReadReady1)
{
    StartService();

    int socketFd_ = sockServcie_->Connect(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    EXPECT_NE(SOCK_INVALID_FD, socketFd_);
    usleep(10000);

    rfcCbConnectSuccess();
    EXPECT_CALL(*g_stackmocker, RFCOMM_Write()).WillOnce(Return(0));

    int wbytes = write(socketFd_, "test", 4);
    if (wbytes <= 0) {
        printf("write %d error!\n", socketFd_);
    }
    usleep(10000);

    StopService();
}

TEST_F(SockTest, OnSocketReadReady2)
{
    StartService();

    int socketFd_ = sockServcie_->Connect(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    EXPECT_NE(SOCK_INVALID_FD, socketFd_);
    usleep(10000);

    EXPECT_CALL(*g_stackmocker, RFCOMM_Write()).WillRepeatedly(Return(0));

    uint8_t buffer[1000];
    int wbytes = write(socketFd_, buffer, 1000);
    if (wbytes <= 0) {
        printf("write %d error!\n", socketFd_);
    }
    usleep(10000);

    StopService();
}

TEST_F(SockTest, OnSocketReadReady3)
{
    StartService();

    int socketFd_ = sockServcie_->Connect(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    EXPECT_NE(SOCK_INVALID_FD, socketFd_);
    usleep(10000);

    EXPECT_CALL(*g_stackmocker, RFCOMM_Write()).WillRepeatedly(Return(-1));

    int wbytes = write(socketFd_, "test", 4);
    if (wbytes <= 0) {
        printf("write %d error!\n", socketFd_);
    }
    usleep(10000);

    g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_FC_ON, &rfcEventData_, g_rfcCb.context);

    StopService();
}

TEST_F(SockTest, OnSocketReadReady4)
{
    StartService();

    int socketFd_ = sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    EXPECT_NE(SOCK_INVALID_FD, socketFd_);

    rfcCbConnectSuccess();
    usleep(10000);

    int dataFd_ = RecvSocketFd(socketFd_);

    EXPECT_CALL(*g_stackmocker, RFCOMM_Write()).WillOnce(Return(0));

    int wbytes = write(dataFd_, "test", 4);
    if (wbytes <= 0) {
        printf("write %d error!\n", dataFd_);
    }
    usleep(10000);

    StopService();
}

TEST_F(SockTest, OnSocketWriteReady1)
{
    StartService();

    sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 0, 0);
    rfcCbConnectSuccess();
    usleep(10000);

    for (int i = 0; i < sendBufSize; i++) {
        Packet *pkt1 = PacketMalloc(5, 5, 1000);
        EXPECT_CALL(*g_stackmocker, RFCOMM_Read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<1>(pkt1), Return(RFCOMM_SUCCESS)));

        g_rfcCb.callBack(1, RFCOMM_CHANNEL_EV_REV_DATA, &rfcEventData_, g_rfcCb.context);
        usleep(10000);
    }
    StopService();
}

TEST_F(SockTest, Listen1)
{
    StartService();

    EXPECT_NE(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 1, 0));

    StopService();
}

TEST_F(SockTest, Listen2)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_AddServiceClassIdList()).WillOnce(Return(-1));
    EXPECT_EQ(SOCK_INVALID_FD, sockServcie_->Listen(addr, Uuid::ConvertFrom16Bits(uuid), 1, 0));

    StopService();
}

TEST_F(SockTest, Listen3)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_AddProtocolDescriptorList()).WillOnce(Return(-1));
    listen();

    StopService();
}

TEST_F(SockTest, Listen4)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_AddBluetoothProfileDescriptorList()).WillOnce(Return(-1));
    listen();

    StopService();
}

TEST_F(SockTest, Listen5)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_AddServiceName()).WillOnce(Return(-1));
    listen();

    StopService();
}

TEST_F(SockTest, Listen6)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_AddBrowseGroupList()).WillOnce(Return(-1));
    listen();

    StopService();
}

TEST_F(SockTest, Listen7)
{
    StartService();

    EXPECT_CALL(*g_stackmocker, SDP_RegisterServiceRecord()).WillOnce(Return(-1));
    listen();

    StopService();
}

TEST_F(SockTest, CreateRfcommTransport)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    sockTransport_ = TransportFactory::CreateRfcommTransport(&rawAddr, 1, 200, *observer_, *dispatcher_);
    EXPECT_EQ(0, sockTransport_->RejectConnection(rawAddr, 1));
}

TEST_F(SockTest, RfcommGetRemoteAddress)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    sockTransport_ = TransportFactory::CreateRfcommTransport(&rawAddr, 1, 200, *observer_, *dispatcher_);
    EXPECT_EQ(rawAddr, sockTransport_->GetRemoteAddress());
    usleep(10000);
}

TEST_F(SockTest, CreateL2capTransport)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, *observer_, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);
    EXPECT_EQ(0, sockTransport_->Connect());
    EXPECT_EQ(0, sockTransport_->Disconnect());
    EXPECT_EQ(0, sockTransport_->RegisterServer());
    EXPECT_EQ(0, sockTransport_->RemoveServer(false));
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    EXPECT_EQ(0, sockTransport_->RejectConnection(rawAddr, 1));
    EXPECT_EQ(0, sockTransport_->Read(nullptr));
    EXPECT_EQ(0, sockTransport_->Write(nullptr));
}

TEST_F(SockTest, CreateClientL2capTransport)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    Packet *pkt = PacketMalloc(0, 0, 5);
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvConnectionRsp(1, &l2cConnectInfo_, 1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvConfigReq(1, 1, &l2cConfigInfo_, g_l2cCb.context);
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvDisconnectionReq(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvDisconnectionRsp(1, g_l2cCb.context);
    g_l2cCb.callBack.disconnectAbnormal(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.remoteBusy(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvData(1, pkt, g_l2cCb.context);
    L2capTransport::DeregisterClientPsm(1);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RecvConnectionRsp_Success)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConnectionRsp(1, &l2cConnectInfo_, 0, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RecvConnectionRsp_Pending)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConnectionRsp(1, &l2cConnectInfo_, 2, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, CreateServerL2capTransport)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvDisconnectionReq(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvDisconnectionRsp(1, g_l2cCb.context);
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    g_l2cCb.callBack.disconnectAbnormal(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvConfigReq(1, 1, &l2cConfigInfo_, g_l2cCb.context);
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 1, g_l2cCb.context);
    g_l2cCb.callBack.remoteBusy(1, 1, g_l2cCb.context);
    g_l2cCb.callBack.recvData(1, pkt, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RecvConfigRsp_Success)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 0, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_GetRemoteAddress)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(rawAddr, sockTransport_->GetRemoteAddress());
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RemoveServer)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    EXPECT_EQ(0, sockTransport_->RemoveServer(false));

    usleep(10000);
}

TEST_F(SockTest, L2capTransport_DataBusy)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.remoteBusy(1, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RecvData)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvData(1, pkt, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_DisconnectAbnormal1)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.disconnectAbnormal(1, 255, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_DisconnectAbnormal2)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.disconnectAbnormal(1, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_DisconnectReq)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvDisconnectionReq(1, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_DisconnectRsp)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvDisconnectionRsp(1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_RecvConfigRsp)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    TestDataTransportObserver observer;
    Packet *pkt = PacketMalloc(0, 0, 5);
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    L2capTransportInfo createInfo{nullptr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    EXPECT_EQ(0, sockTransport_->RegisterServer());
    g_l2cCb.callBack.recvConnectionReq(1, 1, &l2cConnectInfo_, 1, g_l2cCb.context);
    usleep(10000);
    EXPECT_EQ(0, sockTransport_->AcceptConnection(rawAddr, 1));
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 1, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_Connect1)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    Packet *pkt = PacketMalloc(0, 0, 5);
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConnectionRsp(1, &l2cConnectInfo_, 0, 1, g_l2cCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConfigReq(1, 1, &l2cConfigInfo_, g_l2cCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 0, g_l2cCb.context);
    usleep(10000);
}

TEST_F(SockTest, L2capTransport_Connect2)
{
    std::unique_ptr<DataTransport> sockTransport_{nullptr};
    Packet *pkt = PacketMalloc(0, 0, 5);
    TestDataTransportObserver observer;
    RawAddress rawAddr = RawAddress::ConvertToString(g_sdpCb_param.btaddr.addr);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;

    L2capTransportInfo createInfo{&rawAddr, 1, 1, 200, observer, *dispatcher_};
    sockTransport_ = TransportFactory::CreateL2capTransport(createInfo);

    L2capTransport::RegisterClientPsm(1);
    EXPECT_EQ(0, sockTransport_->Connect());
    g_l2cConReqCb.cb(&btAddr, 1, 0, g_l2cConReqCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConnectionRsp(1, &l2cConnectInfo_, 0, 1, g_l2cCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConfigRsp(1, &l2cConfigInfo_, 0, g_l2cCb.context);
    usleep(10000);
    g_l2cCb.callBack.recvConfigReq(1, 1, &l2cConfigInfo_, g_l2cCb.context);
    usleep(10000);
}