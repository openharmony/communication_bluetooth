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
#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "map_mce_types.h"
#include "map_mce_data_analyse.h"
#include "map_mce_instance_request.h"
#include "map_mce_service.h"
#include "map_mce_instance_stm.h"
#include "map_mce_instance_client.h"
#include "map_mce_device_ctrl.h"
#include "map_mce_service.h"
#include "map_mce_mns_server.h"
#include "common_mock_all.h"
#include "obex_socket_transport.h"
#include "mce_mock/mce_mock.h"

extern "C" {
#include "btstack.h"
#include "btm.h"
#include "log.h"
}

using namespace testing;
using namespace bluetooth;
std::unique_ptr<bluetooth::ObexHeader> mnsHeader0 = nullptr;
std::unique_ptr<bluetooth::ObexHeader> mnsHeader1 = nullptr;
static const uint8_t MNS_TARGET[16] = {
    0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};

class TestMceObexTransport : public ObexTransport {
public:
    TestMceObexTransport() = default;
    virtual ~TestMceObexTransport() = default;
    bool Write(Packet &pkt) override
    {
        return true;
    };
    // the maximum allowed OBEX packet that can be send over the transport
    int GetMaxSendPacketSize() override
    {
        return 1024;
    };
    // the maximum allowed OBEX packet that can be received over the transport
    int GetMaxReceivePacketSize() override
    {
        return 1024;
    };
    // This function is used to get the Bluetooth address of the peer of the connected channel
    virtual const RawAddress &GetRemoteAddress() override
    {
        return rawAddr_;
    };
    // is transport Connected
    virtual bool IsConnected() override
    {
        return true;
    };
    // get transport key
    const std::string &GetTransportKey() override
    {
        return transportKey_;
    };
    void SetBusy(ObexServerSession &session, bool isBusy)
    {

    };
private:
    RawAddress rawAddr_{};
    std::string transportKey_ = "";
};
class TestMceObexMpServerObserver : public ObexServerObserver {
public:
    TestMceObexMpServerObserver() = default;
    virtual ~TestMceObexMpServerObserver()
    {}
    void OnTransportConnect(ObexIncomingConnect &incomingConnect) override
    {}
    void OnTransportConnected(const std::string &btAddr) override
    {}
    void OnTransportDisconnected(const std::string &btAddr) override
    {}
    void OnTransportError(const std::string &btAddr, int errCd, const std::string &msg) override
    {}
    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnError(const int errCd, const std::string &msg) override
    {}
    void OnPut(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnGet(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnAbort(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnSetPath(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnAction(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnSession(ObexServerSession &session, const ObexHeader &req) override
    {}
    void OnBusy(ObexServerSession &session, bool isBusy) const override
    {}
};

TEST(MapMceDeviceCtrl, Construct)
{
    MapMceObserverManager observerMgr;
    MapMceUintTestCallback deviceObserver;  // create callback
    observerMgr.RegisterObserver(deviceObserver);

    mnsHeader1 = ObexHeader::CreateResponse(ObexRspCode::NON_AUTH);
    mnsHeader0 = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
    mnsHeader0->AppendItemTarget(MNS_TARGET, 16);

    RawAddress mockerDevice("00:00:01:00:00:7F");
    MapMceService *mapServcie = nullptr;

    StrictMock<ProfileConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> newInstanceMock;
    registerNewMockProfileConfig(&newInstanceMock);
    EXPECT_CALL(newInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&configMock));
    EXPECT_CALL(configMock, SetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Return(true));
    EXPECT_CALL(configMock, GetValue(_, _, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<3>((int)BTStrategyType::CONNECTION_UNKNOWN), Return(false)));

    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock;
    StrictMock<ProfileServiceManagerMock> psmMock;
    registerNewMockProfileServiceManager(&psminstanceMock);
    EXPECT_CALL(psminstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock));
    mapServcie = new MapMceService();
    mapServcie->GetContext()->Initialize();
    EXPECT_CALL(psmMock, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(mapServcie));

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    MasInstanceConfig config;
    config.l2capMtu_ = 1024;
    config.rfcommMtu_ = 1024;
    config.isSupportSrm_ = true;
    MapMceDeviceCtrl *mceDeviceCtrl =
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, observerMgr);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ("00:00:01:00:00:7F", mceDeviceCtrl->GetBtDevice());

    delete mceDeviceCtrl;
    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    observerMgr.DeregisterObserver(deviceObserver);

    delete mapServcie;
    usleep(500000);  // 200 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceDeviceCtrl, status_change)
{
    MapMceObserverManager observerMgr;
    MapMceUintTestCallback deviceObserver;  // create callback
    observerMgr.RegisterObserver(deviceObserver);

    mnsHeader1 = ObexHeader::CreateResponse(ObexRspCode::NON_AUTH);
    mnsHeader0 = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
    mnsHeader0->AppendItemTarget(MNS_TARGET, 16);

    RawAddress mockerDevice("00:00:01:00:00:7F");
    MapMceService *mapServcie = nullptr;

    StrictMock<ProfileConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> newInstanceMock;
    registerNewMockProfileConfig(&newInstanceMock);
    EXPECT_CALL(newInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&configMock));
    EXPECT_CALL(configMock, SetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Return(true));
    EXPECT_CALL(configMock, GetValue(_, _, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<3>((int)BTStrategyType::CONNECTION_UNKNOWN), Return(false)));

    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock;
    StrictMock<ProfileServiceManagerMock> psmMock;
    registerNewMockProfileServiceManager(&psminstanceMock);
    EXPECT_CALL(psminstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock));
    mapServcie = new MapMceService();
    mapServcie->GetContext()->Initialize();
    EXPECT_CALL(psmMock, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(mapServcie));

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    MasInstanceConfig config;
    config.l2capMtu_ = 1024;
    config.rfcommMtu_ = 1024;
    config.isSupportSrm_ = true;
    MapMceDeviceCtrl *mceDeviceCtrl =
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, observerMgr);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    EXPECT_EQ("00:00:01:00:00:7F", mceDeviceCtrl->GetBtDevice());

    // disconncet status test
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_SEND_REQUEST);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FINISH);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECT_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_TRANSPORT_ERRO);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // connceting status test
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_SEND_REQUEST);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FINISH);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connceting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connceting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connceting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECT_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connceting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_TRANSPORT_ERRO);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connceting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // connceted status test
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FINISH);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECT_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connected again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_TRANSPORT_ERRO);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // connected again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_SEND_REQUEST);  // test case excute
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);  // test case excute
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // disconnceting status test
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_SEND_REQUEST);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FINISH);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_SDP_GET_INSTANCE_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // disconnecting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_TRANSPORT_ERRO);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    // disconnecting again
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECT_FAILED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    observerMgr.DeregisterObserver(deviceObserver);

    delete mceDeviceCtrl;
    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 200 ms
    EXPECT_EQ(0, 0);
}

const char *ctrlTestEndbody = "";

TEST(MapMceDeviceCtrl, mns_status_change)
{
    MapMceObserverManager observerMgr;
    MapMceUintTestCallback deviceObserver;  // create callback
    observerMgr.RegisterObserver(deviceObserver);

    //const char *endbody = "endbody";
    mnsHeader1 = ObexHeader::CreateResponse(ObexRspCode::NON_AUTH);
    mnsHeader0 = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
    mnsHeader0->AppendItemTarget(MNS_TARGET, 16);
    mnsHeader0->SetFieldObexVersionNum(0x66);
    // make writer object
    std::shared_ptr<ObexBodyObject> read = std::make_shared<ObexArrayBodyObject>();
    read->Write((const uint8_t *)ctrlTestEndbody, sizeof(ctrlTestEndbody));
    mnsHeader0->SetExtendBodyObject(read);

    RawAddress mockerDevice("00:00:01:00:00:7F");
    MapMceService *mapServcie = nullptr;

    StrictMock<ProfileConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> newInstanceMock;
    registerNewMockProfileConfig(&newInstanceMock);
    EXPECT_CALL(newInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&configMock));
    EXPECT_CALL(configMock, SetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Return(true));
    EXPECT_CALL(configMock, GetValue(_, _, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<3>((int)BTStrategyType::CONNECTION_UNKNOWN), Return(false)));

    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock;
    StrictMock<ProfileServiceManagerMock> psmMock;
    registerNewMockProfileServiceManager(&psminstanceMock);
    EXPECT_CALL(psminstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock));
    mapServcie = new MapMceService();
    mapServcie->GetContext()->Initialize();
    EXPECT_CALL(psmMock, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(mapServcie));

    MapMceUintTestCallback serviceDeviceObserver;  // create callback
    mapServcie->RegisterObserver(serviceDeviceObserver);

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    MasInstanceConfig config;
    config.l2capMtu_ = 1024;
    config.rfcommMtu_ = 1024;
    config.isSupportSrm_ = true;
    MapMceDeviceCtrl *mceDeviceCtrl =
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, observerMgr);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    EXPECT_EQ("00:00:01:00:00:7F", mceDeviceCtrl->GetBtDevice());

    // client disconncet status test
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    utility::Message msg(MSG_MCEDEVICE_MNS_INFO_CONNECT);

    ObexPrivateServer::ObexPrivateServerConfig option;
    TestMceObexMpServerObserver observer;
    ObexPrivateServer privateServer(option, observer, *mapServcie->GetDispatcher());
    TestMceObexTransport transport;

    ObexServerSession *mnsObexSession = new ObexServerSession(transport,
        true,
        *mapServcie->GetDispatcher(),
        std::bind(&ObexPrivateServer::RemoveSession, &privateServer, std::placeholders::_1),
        std::bind(&TestMceObexTransport::SetBusy, &transport, std::placeholders::_1, std::placeholders::_2));

    ObexServerSocketTransport::Option incomingOption;
    MapMceMnsTransTestCallback transObserver;
    ObexServerSocketTransport obexL2capTrans(incomingOption, transObserver, *mapServcie->GetDispatcher());
    ObexIncomingConnect *incomingPtr = new ObexServerIncomingConnect(obexL2capTrans, mockerDevice, 1);

    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;  // connect again
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    msg = MSG_MCEDEVICE_MNS_INFO_DISCONNECT;
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_ERROR);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_PUT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_DEVICE_DISCONNECTED_FINISH);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_CONNECT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // client connecting status test
    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetMnsState());
    msg = MSG_MCEDEVICE_MNS_INFO_DISCONNECT;
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());

    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;  // connect again
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_ERROR);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_PUT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_CONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // client connceted status test
    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetMnsState());
    // test on event report
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    msg = MSG_MCEDEVICE_MNS_INFO_PUT;
    bluetooth::ObexHeader *tempObexHeader = new bluetooth::ObexHeader(*mnsHeader0);
    msg.arg2_ = (void *)tempObexHeader;
    mceDeviceCtrl->ProcessMessage(msg);
    EXPECT_EQ(MCE_TEST_EXCUTE_STATUS_ON_MAP_EVENT_REPORTED, callbackExcuteStatus);
    // disconnect
    msg = MSG_MCEDEVICE_MNS_INFO_DISCONNECT;
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());

    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;  // connect again
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_ERROR);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_PUT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());

    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;  // connect again
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_REQ_DEVICE_DISCONNECT);  // test case excute
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    // client disconnceting status test
    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    msg = MSG_MCEDEVICE_MNS_INFO_CONNECT;  // connect again
    msg.arg2_ = (void *)mnsObexSession;
    mceDeviceCtrl->ProcessMnsObexObserverMessage(*mnsHeader0, msg);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTING, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_DISCONNECTED);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());

    msg = MSG_MCEDEVICE_MNS_INFO_TRANSPORT_INCOMING;
    msg.arg2_ = (void *)incomingPtr;
    mceDeviceCtrl->ProcessMessage(msg);
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_CONNECT);  // connect again
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_TRANSPORT_ERROR);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_MNS_INFO_PUT);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetMnsState());
    mceDeviceCtrl->ProcessMessage(MSG_MCEDEVICE_INSCLIENT_TRANSPORT_ERRO);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetTargetDeviceState());

    observerMgr.DeregisterObserver(deviceObserver);

    read = nullptr;
    delete tempObexHeader;
    delete incomingPtr;
    delete mnsObexSession;
    delete mceDeviceCtrl;
    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 200 ms
    EXPECT_EQ(0, 0);
}
