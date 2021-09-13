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

#include "avrcp_ct_mock.h"

using namespace testing;
using namespace bluetooth;

class avrcpCtUintTestCallback : public IProfileAvrcpCt::IObserver {
public:
    std::string funcname_;
    int connstate_;
    uint8_t button_;
    avrcpCtUintTestCallback(){};
    ~avrcpCtUintTestCallback(){};
    void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
    {
        funcname_ = __func__;
        connstate_ = state;
    };

    void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) override
    {
        funcname_ = __func__;
        button_ = button;
    };

    void OnReleaseButton(const RawAddress &rawAddr, uint8_t button, int result) override
    {
        funcname_ = __func__;
        button_ = button;
    };
    void OnSetAddressedPlayer(const RawAddress &rawAddr, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayerAppSettingAttributes(
        const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayerAppSettingValues(
        const RawAddress &rawAddr, uint8_t attribute, const std::vector<uint8_t> &values, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result) override
    {
        funcname_ = __func__;
    };

    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayStatus(
        const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result) override
    {
        funcname_ = __func__;
    };

    void OnAddToNowPlaying(const RawAddress &rawAddr, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnPlayItem(const RawAddress &rawAddr, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnChangePath(const RawAddress &rawAddr, uint32_t numOfItems, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items,
        int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items,
        int result, int detail) override
    {
        funcname_ = __func__;
    };
    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnGetTotalNumberOfItems(
        const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, int result, int detail) override
    {
        funcname_ = __func__;
    };

    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &attrStr, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueStr, int result) override
    {
        funcname_ = __func__;
    };

    void OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result) override
    {
        funcname_ = __func__;
    };

    void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) override
    {
        funcname_ = __func__;
    };

    void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) override
    {
        funcname_ = __func__;
    };

    void OnTrackReachedEnd(const RawAddress &rawAddr, int result) override
    {
        funcname_ = __func__;
    };

    void OnTrackReachedStart(const RawAddress &rawAddr, int result) override
    {
        funcname_ = __func__;
    };

    void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) override
    {
        funcname_ = __func__;
    };

    void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result) override
    {
        funcname_ = __func__;
    };

    void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) override
    {
        funcname_ = __func__;
    };

    void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) override
    {
        funcname_ = __func__;
    };

    void OnAddressedPlayerChanged(
        const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) override
    {
        funcname_ = __func__;
    };

    void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) override
    {
        funcname_ = __func__;
    };

    void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) override
    {
        funcname_ = __func__;
    };
};

class CtTestContextCallback : public utility::IContextCallback {
public:
    void OnEnable(const std::string &name, bool ret) override
    {
        if (ret) {
            enablestate = 1;
        } else {
            enablestate = 0;
        }
    }
    void OnDisable(const std::string &name, bool ret) override
    {
        if (ret) {
            enablestate = 0;
        } else {
            enablestate = 1;
        }
    }
    int enablestate = -1;
};

class AvrcpCtService_test : public testing::Test {
public:
    void Enable();
    void Disable();
    void SetConnState(int state);
    void SetConnState(int state, RawAddress bda);
    void WaitEnableState(int state, CtTestContextCallback &tccb);

    static void SetUpTestCase()
    {
        printf("AvrcpCtService_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("AvrcpCtService_test TearDownTestCase\n");
        AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
    }
    virtual void SetUp()
    {
        registerGapMocker(&mockGap_);
        registerSdpMocker(&mockSdp_);
        registerAvctpMocker(&mockAvctp_);
        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        service = new AvrcpCtService();
        service->GetContext()->Initialize();
        ccb_ = std::make_unique<CtTestContextCallback>();
        service->GetContext()->RegisterCallback(*ccb_);
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(service));

        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_CT_SERVICE, PROPERTY_CONTROL_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(512), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_CT_SERVICE, PROPERTY_BROWSE_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(335), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_AVRCP_TG_SERVICE, A<bool &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(false), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_CT_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));

        //Mock PowerManager
        registerNewMockPowerManager(&pminstacnceMock_);
        EXPECT_CALL(pminstacnceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_,_,_)).Times(AtLeast(0)).WillRepeatedly(Return());

        AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
        Enable();
        service->RegisterObserver(&observer);
        EXPECT_EQ(1, service->GetConnectState());
        RawAddress rawAddr("00:00:00:00:00:11");
        AvrcCtStateMachineManager::GetInstance()->DeletePairOfStateMachine(rawAddr);
        EXPECT_EQ(0, service->Connect(rawAddr));
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }
    virtual void TearDown()
    {
        AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
        Disable();
        service->UnregisterObserver();

        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

    std::unique_ptr<CtTestContextCallback> ccb_{nullptr};
    avrcpCtUintTestCallback observer;
    AvrcpCtService *service;
    NiceMock<MockSdp> mockSdp_;
    NiceMock<MockGap> mockGap_;
    NiceMock<MockAvctp> mockAvctp_;

    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    StrictMock<PowerManagerInstanceMock> pminstacnceMock_;
    StrictMock<PowerManagerMock> pmMock_;
};

void AvrcpCtService_test::Enable()
{
    EXPECT_CALL(GetGapMocker(), GAPIF_RegisterServiceSecurity).WillRepeatedly(Return(0));
    EXPECT_CALL(GetSdpMocker(), SDP_CreateServiceRecord).WillRepeatedly(Return(0x00));
    EXPECT_CALL(GetSdpMocker(), SDP_AddServiceClassIdList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddProtocolDescriptorList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddAdditionalProtocolDescriptorList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddBluetoothProfileDescriptorList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddAttribute).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddServiceName).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddBrowseGroupList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_RegisterServiceRecord).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(0));

    service->Enable();
    WaitEnableState(1, *ccb_);
    EXPECT_TRUE(service->IsEnabled());
}

void AvrcpCtService_test::Disable()
{
    EXPECT_CALL(GetGapMocker(), GAPIF_DeregisterServiceSecurity).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_DeregisterServiceRecord).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_DestroyServiceRecord).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0)).WillRepeatedly(Return(AVCT_FAILED));

    service->Disable();
    WaitEnableState(0, *ccb_);
    EXPECT_FALSE(service->IsEnabled());
}

void AvrcpCtService_test::SetConnState(int state = 0)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;

    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq()).Times(AtLeast(0));

    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).Times(AtLeast(0));

    AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
    AvrcCtConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    AvrcCtStateMachineManager *smManager = AvrcCtStateMachineManager::GetInstance();
    smManager->AddControlStateMachine(bda);
    smManager->AddBrowseStateMachine(bda);
    utility::Message msg(state);
    smManager->SendMessageToControlStateMachine(bda, msg);
    smManager->SendMessageToBrowseStateMachine(bda, msg);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
}

void AvrcpCtService_test::SetConnState(int state, RawAddress bda)
{
    EXPECT_EQ(0, service->Connect(bda));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;

    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq()).Times(AtLeast(0));

    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).Times(AtLeast(0));

    AvrcCtStateMachineManager *smManager = AvrcCtStateMachineManager::GetInstance();
    smManager->AddControlStateMachine(bda);
    smManager->AddBrowseStateMachine(bda);
    utility::Message msg(state);
    smManager->SendMessageToControlStateMachine(bda, msg);
    smManager->SendMessageToBrowseStateMachine(bda, msg);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
}

void AvrcpCtService_test::WaitEnableState(int state, CtTestContextCallback &tccb)
{
    while (true) {
        if (tccb.enablestate == state) {
            break;
        }
    }
}

TEST_F(AvrcpCtService_test, GetContext)
{
    EXPECT_NE(nullptr, service->GetContext());
}

TEST_F(AvrcpCtService_test, Connect)
{
    RawAddress bda("00:00:00:00:00:11");

    EXPECT_EQ(5, service->GetConnectState());

    EXPECT_EQ(-3, service->Connect(bda));
}

TEST_F(AvrcpCtService_test, Disconnect_1)
{
    RawAddress bda("0A:00:27:00:00:01");

    EXPECT_EQ(-3, service->Disconnect(bda));
}

TEST_F(AvrcpCtService_test, Disconnect_2)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);
    EXPECT_EQ(9, service->GetConnectState());

    EXPECT_EQ(service->Disconnect(bda), 0);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_EQ(3, service->GetConnectState());
}

TEST_F(AvrcpCtService_test, Disconnect_3)
{
    Disable();
    RawAddress bda("00:00:00:00:00:11");
    EXPECT_EQ(-3, service->Connect(bda));
    EXPECT_EQ(-3, service->Disconnect(bda));
}

TEST_F(AvrcpCtService_test, GetConnectDevices_1)
{
    std::list<RawAddress> device = service->GetConnectDevices();
}

TEST_F(AvrcpCtService_test, GetMaxConnectNum_1)
{
    EXPECT_EQ(0, service->GetMaxConnectNum());
}

TEST_F(AvrcpCtService_test, GetDevicesByStates_1)
{
    std::vector<int> list = {0};
    service->GetDevicesByStates(list);

    EXPECT_EQ(0, list.front());
}

TEST_F(AvrcpCtService_test, GetConnectedDevices_1)
{
    auto result = service->GetConnectedDevices().empty();
    EXPECT_TRUE(result);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_CONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq).WillRepeatedly(Return(0));

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_CONNECT_CFM_EVT_2)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_CFM_EVT;
    uint16_t result = AVCT_FAILED;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    RawAddress rawAddr(RawAddress::ConvertToString(btAddr.addr));
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), service->GetDeviceState(rawAddr));
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_DISCONNECT_IND_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_DISCONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), observer.connstate_);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_DISCONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_DISCONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    RawAddress rawAddr(RawAddress::ConvertToString(btAddr.addr));

    AvrcCtStateMachineManager *smManager = AvrcCtStateMachineManager::GetInstance();
    utility::Message msg(AVRC_CT_SM_EVENT_TO_DISABLE_STATE);
    smManager->SendMessageToAllBrowseStateMachine(msg);
    smManager->SendMessageToAllControlStateMachine(msg);

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_DISCONNECT_CFM_EVT_2)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_DISCONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), observer.connstate_);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_CONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), observer.connstate_);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_CONNECT_CFM_EVT_2)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CONNECT_CFM_EVT;
    uint16_t result = AVCT_FAILED;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_DISCONNECT_IND_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_DISCONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_DISCONNECT_CFM_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_DISCONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_CONNECT_IND_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
    RawAddress rawAddr("00:00:00:00:00:11");
    AvrcCtStateMachineManager::GetInstance()->DeletePairOfStateMachine(rawAddr);

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), observer.connstate_);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_CONNECT_IND_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelEventCallback_AVCT_BR_CHANNEL_BUSY_EVT)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CHANNEL_BUSY_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_PASS_THROUGH_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t oper = AVRC_KEY_OPERATION_VOLUME_UP;
    uint8_t state = AVRC_KEY_STATE_PRESS;

    std::shared_ptr<AvrcTgPassPacket> packet = std::make_shared<AvrcTgPassPacket>(oper, state, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnPressButton", observer.funcname_.c_str());
    EXPECT_EQ(oper, observer.button_);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_PASS_THROUGH_2)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t oper = AVRC_KEY_OPERATION_BACKWARD;
    uint8_t state = AVRC_KEY_STATE_RELEASE;

    std::shared_ptr<AvrcTgPassPacket> packet = std::make_shared<AvrcTgPassPacket>(oper, state, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnReleaseButton", observer.funcname_.c_str());
    EXPECT_EQ(oper, observer.button_);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_UNIT_INFO_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    std::shared_ptr<AvrcTgUnitPacket> packet = std::make_shared<AvrcTgUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcCtUnitPacket> subpkt = std::make_shared<AvrcCtUnitPacket>();
    AvrcCtConnectManager::GetInstance()->PushUnitQueue(rawAddr, subpkt);

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_UNIT_INFO_2)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    std::shared_ptr<AvrcTgUnitPacket> packet = std::make_shared<AvrcTgUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcCtUnitPacket> subpkt = std::make_shared<AvrcCtSubUnitPacket>();
    AvrcCtConnectManager::GetInstance()->PushUnitQueue(rawAddr, subpkt);

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_UNIT_INFO_3)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    std::shared_ptr<AvrcTgUnitPacket> packet = std::make_shared<AvrcTgUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcCtVendorPacket> subpkt = std::make_shared<AvrcCtGpsPacket>();
    AvrcCtConnectManager::GetInstance()->PushVendorQueue(rawAddr, subpkt);

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AVRC_CT_OP_CODE_SUB_UNIT_INFO_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    std::shared_ptr<AvrcTgSubUnitPacket> packet = std::make_shared<AvrcTgSubUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcCtVendorPacket> subpkt = std::make_shared<AvrcCtGpsPacket>();
    AvrcCtConnectManager::GetInstance()->PushVendorQueue(rawAddr, subpkt);

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gc_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    std::vector<uint32_t> companyies;
    companyies.push_back(100);

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgGcPacket>(crCode, companyies, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnGetCapabilities", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Lpasa_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::deque<uint8_t> attributes;
    for (int i = 0; i < 255; i++) {
        int j = i;
        if (j == 255) {
            j = 0;
        }
        attributes.push_back(j);
    }

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgLpasaPacket>(crCode, attributes, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnGetPlayerAppSettingAttributes", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Lpasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t attr = 0x00;

    std::deque<uint8_t> values;
    for (int i = 0; i < AVRC_TG_LPASV_MAX_NUM_OF_VALUES * AVRC_CT_TEST_FRAGMENTS; i++) {
        values.push_back(i % 256);
    }

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgLpasvPacket>(crCode, values, label);

    SetConnState(1);
    EXPECT_EQ(0, service->GetPlayerAppSettingValues(rawAddr, attr));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }
    EXPECT_STREQ("OnGetPlayerAppSettingValues", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gcpasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    std::vector<uint8_t> attributes = {0x00};
    uint16_t mtu = 100;
    std::deque<uint8_t> attrs;
    std::deque<uint8_t> vals;
    for (int i = 0; i <= AVRC_TG_GCPASV_MAX_NUM_OF_ATTRIBUTES * AVRC_CT_TEST_FRAGMENTS; i++) {
        attrs.push_back(i);
        vals.push_back(bluetooth::AVRC_EQUALIZER_OFF);
    }

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgGcpasvPacket>(mtu, crCode, attrs, vals, label);

    SetConnState(1);
    EXPECT_EQ(0, service->GetPlayerAppSettingCurrentValue(rawAddr, attributes));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }
    EXPECT_STREQ("OnGetPlayerAppSettingCurrentValue", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Spasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgSpasvPacket>(crCode, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnSetPlayerAppSettingCurrentValue", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gpasat_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::vector<uint8_t> attrs;
    std::vector<std::string> attrStr;

    for (int i = 0; i <= AVRC_TG_GPASAT_ASSEMBLE_MAX_SIZE * 5; i++) {
        attrs.push_back(i);
        attrStr.push_back("test");
    }

    RawAddress bda("00:00:00:00:00:11");
    std::vector<uint8_t> attributes = {0x00};

    SetConnState(1);
    EXPECT_EQ(0, service->GetPlayerAppSettingAttributeText(bda, attributes));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgGpasatPacket>(crCode, attrs, attrStr, label);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }

    EXPECT_STREQ("OnGetPlayerAppSettingAttributeText", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gpasvt_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::vector<uint8_t> values;
    std::vector<std::string> valueStr;

    for (int i = 0; i <= AVRC_TG_GPASVT_ASSEMBLE_MAX_SIZE * 5; i++) {
        values.push_back(i);
        valueStr.push_back("test");
    }

    RawAddress bda("00:00:00:00:00:11");
    uint8_t attributeId = 0x00;
    std::vector<uint8_t> vals = {0x00};

    SetConnState(1);
    EXPECT_EQ(0, service->GetPlayerAppSettingValueText(bda, attributeId, vals));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgGpasvtPacket>(crCode, values, valueStr, label);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }

    EXPECT_STREQ("OnGetPlayerAppSettingValueText", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gea_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::vector<uint32_t> attrs;
    std::vector<std::string> values;

    for (int i = 0; i <= AVRC_TG_GEA_ASSEMBLE_MAX_SIZE * 3; i++) {
        attrs.push_back(i);
        values.push_back("test");
    }

    RawAddress bda("00:00:00:00:00:11");
    uint64_t identifier = 1;
    std::vector<uint32_t> attributes = {0};

    SetConnState(1);
    EXPECT_EQ(0, service->GetElementAttributes(bda, identifier, attributes));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgGeaPacket>(crCode, attrs, values, label);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }

    EXPECT_STREQ("OnGetElementAttributes", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Gps_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    uint32_t songLength = 210000;
    uint32_t songPosition = 60000;
    uint32_t playStatus = 0x01;

    std::shared_ptr<AvrcTgVendorPacket> packet =
        std::make_shared<AvrcTgGpsPacket>(crCode, songLength, songPosition, playStatus, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnGetPlayStatus", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Acr_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t attr = 0x00;

    std::deque<uint8_t> values;
    for (int i = 0; i < 600; i++) {
        int j = i;
        if (j == 255) {
            j = 0;
        }
        values.push_back(j);
    }

    RawAddress rawAddr("00:00:00:00:00:11");
    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgLpasvPacket>(crCode, values, label);

    SetConnState(1);
    EXPECT_EQ(0, service->GetPlayerAppSettingValues(rawAddr, attr));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    Packet *pkt = packet->PopAssembledPacket();
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    uint8_t pduId = AVRC_TG_PDU_ID_LIST_PLAYER_APPLICATION_SETTING_VALUES;

    std::shared_ptr<AvrcTgVendorPacket> packet2 = std::make_shared<AvrcTgAcrPacket>(pduId, label);
    Packet *pkt2 = packet2->PopAssembledPacket();
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Sap_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgSapPacket>(crCode, status, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnSetAddressedPlayer", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Pi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgPiPacket>(crCode, status, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnPlayItem", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Atnp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgAtnpPacket>(crCode, status, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnAddToNowPlaying", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VENDOR_Sav_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t vol = 0x7F;

    std::shared_ptr<AvrcTgVendorPacket> packet = std::make_shared<AvrcTgSavPacket>(crCode, vol, label);
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnSetAbsoluteVolume", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_Browse_Sbp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    uint16_t mtu = 100;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;
    uint16_t uidCounter = 0;
    uint32_t numOfItems = 10;
    std::vector<std::string> folderNames;
    folderNames.push_back("火曜日のトレーニング");
    folderNames.push_back("星期二的课程");
    folderNames.push_back("C++");

    std::shared_ptr<AvrcTgBrowsePacket> packet =
        std::make_shared<AvrcTgSbpPacket>(mtu, status, uidCounter, numOfItems, folderNames, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnSetBrowsedPlayer", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_Browse_Cp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    uint8_t status = AVRC_ES_CODE_NO_ERROR;
    uint32_t numOfItems = 10;

    std::shared_ptr<AvrcTgBrowsePacket> packet = std::make_shared<AvrcTgCpPacket>(status, numOfItems, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnChangePath", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_Browse_Gfi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    uint16_t mtu = 512;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;
    uint16_t uidCounter = 0;
    std::vector<AvrcMpItem> itemsp;

    for (int i = 0; i < 10; i++) {
        uint8_t itemType = AVRC_MEDIA_TYPE_MEDIA_PLAYER_ITEM;
        uint16_t playerId = 0;
        uint8_t majorType = 0;
        uint32_t subType = 0;
        uint8_t playStatus = 0;
        std::vector<uint8_t> features = {0};
        std::string name = "test";
        AvrcMpItem item_(itemType, playerId, majorType, subType, playStatus, features, name);
        itemsp.push_back(item_);
    }

    std::shared_ptr<AvrcTgBrowsePacket> packet =
        std::make_shared<AvrcTgGfiPacket>(mtu, status, uidCounter, itemsp, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_LONG);
    EXPECT_STREQ("OnGetMediaPlayers", observer.funcname_.c_str());

    std::vector<AvrcMeItem> itemse;

    for (int i = 0; i < 10; i++) {
        uint8_t itemType = 0;
        if (i == 0) {
            itemType = AVRC_MEDIA_TYPE_FOLDER_ITEM;
        } else if (i == 1) {
            itemType = AVRC_MEDIA_TYPE_MEDIA_ELEMENT_ITEM;
        }
        uint64_t uid = 0;
        uint8_t type = 0;
        uint8_t playable = 0;
        std::string name = "test";
        std::vector<uint32_t> attributes = {0};
        std::vector<std::string> values = {"value"};
        AvrcMeItem item_(itemType, uid, type, playable, name, attributes, values);
        itemse.push_back(item_);
    }

    std::shared_ptr<AvrcTgBrowsePacket> packet2 =
        std::make_shared<AvrcTgGfiPacket>(mtu, status, uidCounter, itemse, label);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_CT_TEST_TIMER_LONG);
    EXPECT_STREQ("OnGetFolderItems", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_Browse_Gia_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    uint16_t mtu = 100;
    uint8_t status = AVRC_ES_CODE_NO_ERROR;
    std::vector<uint32_t> attrs;
    std::vector<std::string> vals;
    for (int i = 0; i < 10; i++) {
        attrs.push_back(i);
        vals.push_back("ldasjkgoiIEGN897624976");
    }

    std::shared_ptr<AvrcTgBrowsePacket> packet = std::make_shared<AvrcTgGiaPacket>(mtu, status, attrs, vals, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_LONG);

    EXPECT_STREQ("OnGetItemAttributes", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_Browse_Gtnoi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    uint8_t status = AVRC_ES_CODE_NO_ERROR;
    uint16_t uidCounter = 0;
    uint32_t numOfItems = 10;

    std::shared_ptr<AvrcTgBrowsePacket> packet =
        std::make_shared<AvrcTgGtnoiPacket>(status, uidCounter, numOfItems, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnGetTotalNumberOfItems", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, PressButton_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t button = 0x50;

    SetConnState(1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->PressButton(bda, button);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->PressButton(bda, button));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->PressButton(bda, button));
}

TEST_F(AvrcpCtService_test, ReleaseButton_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t button = 0x41;

    SetConnState(1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->ReleaseButton(bda, button);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    EXPECT_EQ(-3, service->ReleaseButton(bda, button));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    EXPECT_EQ(-3, service->ReleaseButton(bda, button));
}

TEST_F(AvrcpCtService_test, GetUnitInfo_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetUnitInfo(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetUnitInfo(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetUnitInfo(bda));
}

TEST_F(AvrcpCtService_test, GetSubUnitInfo_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetSubUnitInfo(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetSubUnitInfo(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetSubUnitInfo(bda));
}

TEST_F(AvrcpCtService_test, SetAddressedPlayer_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint16_t playerId = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->SetAddressedPlayer(bda, playerId);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->SetAddressedPlayer(bda, playerId));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->SetAddressedPlayer(bda, playerId));
}

TEST_F(AvrcpCtService_test, SetBrowsedPlayer_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint16_t playerId = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->SetBrowsedPlayer(bda, playerId);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->SetBrowsedPlayer(bda, playerId));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->SetBrowsedPlayer(bda, playerId));
}

TEST_F(AvrcpCtService_test, GetSupportedCompanies_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetSupportedCompanies(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetSupportedCompanies(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetSupportedCompanies(bda));
}

TEST_F(AvrcpCtService_test, GetSupportedEvents_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetSupportedEvents(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetSupportedEvents(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetSupportedEvents(bda));
}

TEST_F(AvrcpCtService_test, ListPlayerApplicationSettingAttributes_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingAttributes(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributes(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributes(bda));
}

TEST_F(AvrcpCtService_test, GetPlayerAppSettingAttributes_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingAttributes(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributes(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributes(bda));
}

TEST_F(AvrcpCtService_test, GetPlayerAppSettingValues_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t attr = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingValues(bda, attr);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingValues(bda, attr));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingValues(bda, attr));
}

TEST_F(AvrcpCtService_test, GetPlayerAppSettingCurrentValue_1)
{
    RawAddress bda("00:00:00:00:00:11");
    std::vector<uint8_t> attributes = {0x00};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingCurrentValue(bda, attributes);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingCurrentValue(bda, attributes));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingCurrentValue(bda, attributes));
}

TEST_F(AvrcpCtService_test, SetPlayerAppSettingCurrentValue_1)
{
    RawAddress bda("00:00:00:00:00:11");
    std::vector<uint8_t> attribtues = {0};
    std::vector<uint8_t> values = {0};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->SetPlayerAppSettingCurrentValue(bda, attribtues, values);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->SetPlayerAppSettingCurrentValue(bda, attribtues, values));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->SetPlayerAppSettingCurrentValue(bda, attribtues, values));
}

TEST_F(AvrcpCtService_test, GetPlayStatus_1)
{
    RawAddress bda("00:00:00:00:00:11");

    SetConnState(1);

    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayStatus(bda);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayStatus(bda));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayStatus(bda));
}

TEST_F(AvrcpCtService_test, PlayItem_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t scope = 0x00;
    uint64_t uid = 0;
    uint16_t uidCounter = 0;

    SetConnState(1);

    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));
    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->PlayItem(bda, scope, uid, uidCounter);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->PlayItem(bda, scope, uid, uidCounter));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->PlayItem(bda, scope, uid, uidCounter));
}

TEST_F(AvrcpCtService_test, AddToNowPlaying_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t scope = 0x03;
    uint64_t uid = 0x00000002;
    uint16_t uidCounter = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->AddToNowPlaying(bda, scope, uid, uidCounter);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->AddToNowPlaying(bda, scope, uid, uidCounter));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->AddToNowPlaying(bda, scope, uid, uidCounter));
}

TEST_F(AvrcpCtService_test, RequestContinuingResponse_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t pduId = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->RequestContinuingResponse(bda, pduId);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->RequestContinuingResponse(bda, pduId));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->RequestContinuingResponse(bda, pduId));
}

TEST_F(AvrcpCtService_test, AbortContinuingResponse_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t pduId = 0x00;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->AbortContinuingResponse(bda, pduId);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->AbortContinuingResponse(bda, pduId));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->AbortContinuingResponse(bda, pduId));
}

TEST_F(AvrcpCtService_test, ChangePath_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint16_t uidCounter = 0x00;
    uint8_t direction = 0x01;
    uint64_t folderUid = 0x03;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->ChangePath(bda, uidCounter, direction, folderUid);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->ChangePath(bda, uidCounter, direction, folderUid));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->ChangePath(bda, uidCounter, direction, folderUid));
}

TEST_F(AvrcpCtService_test, GetFolderItems_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t scope = 0x01;
    uint32_t start = 2;
    uint32_t end = 4;
    std::vector<uint32_t> attributes{0};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetFolderItems(bda, scope, start, end, attributes);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetFolderItems(bda, scope, start, end, attributes));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetFolderItems(bda, scope, start, end, attributes));
}

TEST_F(AvrcpCtService_test, GetItemAttributes_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t scope = 0x03;
    uint64_t uid = 0x02;
    uint16_t uidCounter = 0x00;
    std::vector<uint32_t> attributes{0};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetItemAttributes(bda, scope, uid, uidCounter, attributes);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetItemAttributes(bda, scope, uid, uidCounter, attributes));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetItemAttributes(bda, scope, uid, uidCounter, attributes));
}

TEST_F(AvrcpCtService_test, GetTotalNumberOfItems_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t scope = 0x01;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetTotalNumberOfItems(bda, scope);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetTotalNumberOfItems(bda, scope));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetTotalNumberOfItems(bda, scope));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_1)
{
    RawAddress bda("00:00:00:00:00:11");
    int volume = 20;

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->SetAbsoluteVolume(bda, volume);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->SetAbsoluteVolume(bda, volume));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_2)
{
    RawAddress bda("A0:E9:DB:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_3)
{
    RawAddress bda("00:14:02:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_4)
{
    RawAddress bda("44:5E:F3:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_5)
{
    RawAddress bda("D4:9C:28:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_6)
{
    RawAddress bda("00:18:6B:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_7)
{
    RawAddress bda("B8:AD:3E:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_8)
{
    RawAddress bda("00:24:1C:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_9)
{
    RawAddress bda("00:11:B1:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_10)
{
    RawAddress bda("A4:15:66:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_11)
{
    RawAddress bda("00:14:F1:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_12)
{
    RawAddress bda("00:24:1C:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_13)
{
    RawAddress bda("00:26:7E:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_14)
{
    RawAddress bda("90:03:B7:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_15)
{
    RawAddress bda("0C:A6:94:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_16)
{
    RawAddress bda("94:16:25:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, SetAbsoluteVolume_17)
{
    RawAddress bda("9C:64:8B:00:00:11");
    int volume = 20;

    SetConnState(1, bda);
    EXPECT_EQ(true, AvrcCtConnectManager::GetInstance()->IsDisableAbsoluteVolume(bda));
    EXPECT_EQ(-1, service->SetAbsoluteVolume(bda, volume));
}

TEST_F(AvrcpCtService_test, GetPlayerApplicationSettingAttributeText_1)
{
    RawAddress bda("00:00:00:00:00:11");
    std::vector<uint8_t> attributes = {0x00};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingAttributeText(bda, attributes);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributeText(bda, attributes));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingAttributeText(bda, attributes));
}

TEST_F(AvrcpCtService_test, GetPlayerApplicationSettingValueText_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t attributeId = 0x00;
    std::vector<uint8_t> values = {0x00};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetPlayerAppSettingValueText(bda, attributeId, values);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetPlayerAppSettingValueText(bda, attributeId, values));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetPlayerAppSettingValueText(bda, attributeId, values));
}

TEST_F(AvrcpCtService_test, GetElementAttributes_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint64_t identifier = 1;
    std::vector<uint32_t> attributes = {0};

    SetConnState(1);

    for (int i = 0; i < AVRC_CT_TEST_SIZE_OF_QUEUE; i++) {
        service->GetElementAttributes(bda, identifier, attributes);
    }
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->GetElementAttributes(bda, identifier, attributes));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->GetElementAttributes(bda, identifier, attributes));
}

TEST_F(AvrcpCtService_test, EnableDisableNotification_1)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t interval = 0;
    std::vector<uint8_t> events;
    for (uint8_t event = 0x01; event < 0x0E + 1; event++) {
        events.push_back(event);
    }

    SetConnState(1);
    EXPECT_EQ(0, service->DisableNotification(bda, events));
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_EQ(0, service->EnableNotification(bda, events, interval));
    usleep(AVRC_CT_TEST_TIMEOUT);

    SetConnState(2);
    EXPECT_EQ(-3, service->DisableNotification(bda, events));
    EXPECT_EQ(-3, service->EnableNotification(bda, events, interval));

    AvrcCtConnectManager::GetInstance()->Delete(bda);
    Disable();
    EXPECT_EQ(-3, service->DisableNotification(bda, events));
    EXPECT_EQ(-3, service->EnableNotification(bda, events, interval));
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_PlaybackStatusChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_PLAYBACK_STATUS_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint8_t playStatus = AVRC_PLAY_STATUS_PLAYING;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_PLAYBACK_STATUS_CHANGED, crCode, label);
    notifyPkt->InitPlaybackStatusChanged(playStatus);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnPlaybackStatusChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_TrackChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_TRACK_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint64_t Identifier = 0;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_TRACK_CHANGED, crCode, label);
    notifyPkt->InitTrackChanged(Identifier);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnTrackChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_TrackReachedEnd)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_TRACK_REACHED_END;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_TRACK_REACHED_END, crCode, label);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnTrackReachedEnd", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_TrackReachedStart)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_TRACK_REACHED_START;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_TRACK_REACHED_START, crCode, label);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnTrackReachedStart", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_PlaybackPosChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_PLAYBACK_POS_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint32_t playbackPos = 0;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_PLAYBACK_POS_CHANGED, crCode, label);
    notifyPkt->InitPlaybackPosChanged(playbackPos);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnPlaybackPosChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_PlayerApplicationSettingChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    std::deque<uint8_t> attrs;
    std::deque<uint8_t> vals;

    for (int i = 0; i < AVRC_TG_GCPASV_MAX_NUM_OF_ATTRIBUTES * 3; i++) {
        attrs.push_back(AVRC_PLAYER_ATTRIBUTE_EQUALIZER);
        vals.push_back(AVRC_EQUALIZER_ON);
    }

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED, crCode, label);
    notifyPkt->InitPlayerApplicationSettingChanged(attrs, vals);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;

    SetConnState(1);

    while (true) {
        Packet *pkt = packet->PopAssembledPacket();
        if (pkt == nullptr) {
            break;
        }
        GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
        usleep(AVRC_CT_TEST_TIMER_NORMAL);
    }

    EXPECT_STREQ("OnPlayerAppSettingChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_NowPlayingContentChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED, crCode, label);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnNowPlayingContentChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AvailablePlayersChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_AVAILABLE_PLAYERS_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_AVAILABLE_PLAYERS_CHANGED, crCode, label);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnAvailablePlayersChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_AddressedPlayerChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_ADDRESSED_PLAYER_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint16_t playerId = 1;
    uint16_t uidCounter = 1;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_ADDRESSED_PLAYER_CHANGED, crCode, label);
    notifyPkt->InitAddressedPlayerChanged(playerId, uidCounter);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnAddressedPlayerChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_UidChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_UIDS_CHANGED;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;
    uint16_t uidCounter = 2;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_UIDS_CHANGED, crCode, label);
    notifyPkt->InitUidsChanged(uidCounter);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnUidChanged", observer.funcname_.c_str());
}

TEST_F(AvrcpCtService_test, ChannelMessageCallback_VolumeChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    uint8_t eventId = AVRC_TG_EVENT_ID_VOLUME_CHANGED;
    uint8_t volume = AVRC_ABSOLUTE_VOLUME_PERCENTAGE_100;
    uint8_t crCode = AVRC_TG_RSP_CODE_ACCEPTED;

    std::shared_ptr<AvrcTgNotifyPacket> notifyPkt =
        std::make_shared<AvrcTgNotifyPacket>(AVRC_TG_EVENT_ID_VOLUME_CHANGED, crCode, label);
    notifyPkt->InitVolumeChanged(volume);
    notifyPkt->AssemblePackets();
    std::shared_ptr<AvrcTgVendorPacket> packet = notifyPkt;
    Packet *pkt = packet->PopAssembledPacket();

    SetConnState(1);
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_CT_TEST_TIMER_NORMAL);
    EXPECT_STREQ("OnVolumeChanged", observer.funcname_.c_str());
}
