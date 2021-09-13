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

#include "avrcp_tg_mock.h"

using namespace testing;
using namespace bluetooth;

class avrcpTgUintTestCallback : public IProfileAvrcpTg::IObserver {
public:
    std::string funcname_;
    int connstate_;
    avrcpTgUintTestCallback(){};
    ~avrcpTgUintTestCallback(){};
    void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
    {
        funcname_ = __func__;
        connstate_ = state;
    };
};

class TgTestContextCallback : public utility::IContextCallback {
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

class AvrcpTgService_test : public testing::Test {
public:
    void Enable();
    void Disable();
    void SetConnState(int state);
    void WaitEnableState(int state, TgTestContextCallback &tccb);

    static void SetUpTestCase()
    {
        printf("AvrcpTgService_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("AvrcpTgService_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerGapMocker(&mockGap_);
        registerSdpMocker(&mockSdp_);
        registerAvctpMocker(&mockAvctp_);
        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        service = new AvrcpTgService();
        service->GetContext()->Initialize();
        ccb_ = std::make_unique<TgTestContextCallback>();
        service->GetContext()->RegisterCallback(*ccb_);
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(service));

        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_TG_SERVICE, PROPERTY_CONTROL_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(512), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_TG_SERVICE, PROPERTY_BROWSE_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(335), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_AVRCP_TG_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(1), Return(true)));
        //Mock PowerManager
        registerNewMockPowerManager(&pminstacnceMock_);
        EXPECT_CALL(pminstacnceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_,_,_)).Times(AtLeast(0)).WillRepeatedly(Return());

        AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
        RawAddress bda("00:00:00:00:00:11");
        AvrcTgStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);
        Enable();
        service->RegisterObserver(&observer);
    }
    virtual void TearDown()
    {
        AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
        Disable();
        service->UnregisterObserver();

        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

    std::unique_ptr<TgTestContextCallback> ccb_{nullptr};
    avrcpTgUintTestCallback observer;
    AvrcpTgService *service;
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

void AvrcpTgService_test::Enable()
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
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(BT_NO_ERROR));

    service->Enable();
    WaitEnableState(1, *ccb_);
    EXPECT_TRUE(service->IsEnabled());
}

void AvrcpTgService_test::Disable()
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

void AvrcpTgService_test::SetConnState(int state = 0)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_GetPeerMtu).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).Times(AtLeast(0));

    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
    AvrcTgStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);

    switch (state) {
        case 1:
            GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
            usleep(AVRC_TG_TEST_TIMER_NORMAL);
            event = AVCT_BR_CONNECT_IND_EVT;
            GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
            usleep(AVRC_TG_TEST_TIMER_NORMAL);
            break;
        case 2:
            GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
            usleep(AVRC_TG_TEST_TIMER_NORMAL);
            event = AVCT_BR_CONNECT_IND_EVT;
            GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
            usleep(AVRC_TG_TEST_TIMER_NORMAL);
            EXPECT_EQ(0, service->Disconnect(bda));
            usleep(AVRC_TG_TEST_TIMER_NORMAL);
            break;
        default:
            break;
    }
}

void AvrcpTgService_test::WaitEnableState(int state, TgTestContextCallback &tccb)
{
    while (true) {
        if (tccb.enablestate == state) {
            break;
        }
    }
}

TEST_F(AvrcpTgService_test, GetContext)
{
    EXPECT_NE(nullptr, service->GetContext());
}

TEST_F(AvrcpTgService_test, Connect_001)
{
    RawAddress bda("00:00:00:00:00:11");
    EXPECT_EQ(0, service->Connect(bda));
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    //SetConnState(1);
    //EXPECT_EQ(9, service->GetConnectState());
}

TEST_F(AvrcpTgService_test, Disconnect_001)
{
    RawAddress addr("00:00:00:00:00:11");

    SetConnState(1);

    EXPECT_EQ(0, service->Disconnect(addr));
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
    EXPECT_EQ(3, service->GetConnectState());

    EXPECT_EQ(-3, service->Connect(addr));
    EXPECT_EQ(-3, service->Disconnect(addr));
}

TEST_F(AvrcpTgService_test, GetConnectedDevices_001)
{
    SetConnState(1);
    auto device = service->GetConnectedDevices();
}

TEST_F(AvrcpTgService_test, GetConnectDevices_001)
{
    std::list<RawAddress> device = service->GetConnectDevices();
}

TEST_F(AvrcpTgService_test, GetMaxConnectNum_001)
{
    SetConnState(1);
    service->GetMaxConnectNum();
}

TEST_F(AvrcpTgService_test, GetDevicesByStates_001)
{
    std::vector<int> states;
    states.push_back(static_cast<int>(BTConnectState::CONNECTING));
    states.push_back(static_cast<int>(BTConnectState::CONNECTED));
    states.push_back(static_cast<int>(BTConnectState::DISCONNECTING));
    states.push_back(static_cast<int>(BTConnectState::DISCONNECTED));

    SetConnState(1);
    std::vector<RawAddress> addrs = service->GetDevicesByStates(states);

    EXPECT_STREQ("00:00:00:00:00:11", addrs[0].GetAddress().c_str());
}

TEST_F(AvrcpTgService_test, GetDeviceState_001)
{
    RawAddress bda("00:00:00:00:00:11");

    service->GetDeviceState(bda);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnConnectionStateChanged)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    int state = 1;
    obs.OnConnectionStateChanged(addr, state);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnPressButton)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t button = 0x00;
    obs.OnPressButton(addr, button);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnReleaseButton)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t button = 0x00;
    obs.OnReleaseButton(addr, button);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnSetAddressedPlayer)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t label = 0x00;
    int status = 1;

    SetConnState(1);
    obs.OnSetAddressedPlayer(addr, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnSetAddressedPlayer(addr, label, status);

    Disable();
    obs.OnSetAddressedPlayer(addr, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnSetBrowsedPlayer)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint16_t uidCounter = 0x0001;
    uint32_t numOfItems = 0x00000001;
    std::vector<std::string> folderNames;
    folderNames.push_back("testfolder");
    uint8_t label = 0x00;
    int status = AVRC_ES_CODE_NO_ERROR;

    EXPECT_CALL(GetAvctpMocker(), AVCT_BrGetPeerMtu).WillRepeatedly(Return(100));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrSendMsgReq).WillRepeatedly(Return(100));

    SetConnState(1);
    obs.OnSetBrowsedPlayer(addr, uidCounter, numOfItems, folderNames, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnSetBrowsedPlayer(addr, uidCounter, numOfItems, folderNames, label, status);

    Disable();
    obs.OnSetBrowsedPlayer(addr, uidCounter, numOfItems, folderNames, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayerAppSettingAttributes)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> attributes;
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_EQUALIZER);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_REPEAT);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SHUFFLE);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SCAN);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX);
    attributes.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_EXTENSION);

    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnGetPlayerAppSettingAttributes(addr, attributes, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayerAppSettingAttributes(addr, attributes, label);

    Disable();
    obs.OnGetPlayerAppSettingAttributes(addr, attributes, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayerAppSettingValues)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> values;
    values.push_back(0x01);
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnGetPlayerAppSettingValues(addr, values, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayerAppSettingValues(addr, values, label);

    Disable();
    obs.OnGetPlayerAppSettingValues(addr, values, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayerAppSettingCurrentValue)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> attributes;
    attributes.push_back(0x01);
    std::deque<uint8_t> values;
    values.push_back(0x01);
    uint8_t label = 0x00;
    uint8_t context = 0x00;

    SetConnState(1);
    obs.OnGetPlayerAppSettingCurrentValue(addr, attributes, values, label, context);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayerAppSettingCurrentValue(addr, attributes, values, label, context);

    Disable();
    obs.OnGetPlayerAppSettingCurrentValue(addr, attributes, values, label, context);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnSetPlayerAppSettingCurrentValue)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnSetPlayerAppSettingCurrentValue(addr, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnSetPlayerAppSettingCurrentValue(addr, label);

    Disable();
    obs.OnSetPlayerAppSettingCurrentValue(addr, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayerAppSettingAttributeText)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::vector<uint8_t> attrs = {0x00};
    std::vector<std::string> attrStr = {"test0"};
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnGetPlayerAppSettingAttributeText(addr, attrs, attrStr, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayerAppSettingAttributeText(addr, attrs, attrStr, label);

    Disable();
    obs.OnGetPlayerAppSettingAttributeText(addr, attrs, attrStr, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayerAppSettingValueText)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::vector<uint8_t> values = {0x00};
    std::vector<std::string> valueStr = {"test0"};
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnGetPlayerAppSettingValueText(addr, values, valueStr, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayerAppSettingValueText(addr, values, valueStr, label);

    Disable();
    obs.OnGetPlayerAppSettingValueText(addr, values, valueStr, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetElementAttributes)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::vector<uint32_t> attrs = {0};
    std::vector<std::string> values = {"test0"};
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnGetElementAttributes(addr, attrs, values, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetElementAttributes(addr, attrs, values, label);

    Disable();
    obs.OnGetElementAttributes(addr, attrs, values, label);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetPlayStatus)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint32_t songLength = 100;
    uint32_t songPosition = 50;
    uint8_t playStatus = 0x01;
    uint8_t label = 0x00;
    uint8_t context = 0x00;

    SetConnState(1);
    obs.OnGetPlayStatus(addr, songLength, songPosition, playStatus, label, context);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetPlayStatus(addr, songLength, songPosition, playStatus, label, context);

    Disable();
    obs.OnGetPlayStatus(addr, songLength, songPosition, playStatus, label, context);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnPlayItem)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t label = 0x00;
    int status = 1;

    SetConnState(1);
    obs.OnPlayItem(addr, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnPlayItem(addr, label, status);

    Disable();
    obs.OnPlayItem(addr, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnAddToNowPlaying)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t label = 0x00;
    int status = 1;

    SetConnState(1);
    obs.OnAddToNowPlaying(addr, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnAddToNowPlaying(addr, label, status);

    Disable();
    obs.OnAddToNowPlaying(addr, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnChangePath)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint32_t numOfItems = 3;
    uint8_t label = 0x00;
    int status = 1;

    SetConnState(1);
    obs.OnChangePath(addr, numOfItems, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnChangePath(addr, numOfItems, label, status);

    Disable();
    obs.OnChangePath(addr, numOfItems, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetMediaPlayers)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint16_t uidCounter = 1;
    std::vector<stub::MediaService::MediaPlayer> items;
    stub::MediaService::MediaPlayer testitem;
    items.push_back(testitem);
    uint8_t label = 0x00;
    int status = AVRC_ES_CODE_NO_ERROR;

    SetConnState(1);
    obs.OnGetMediaPlayers(addr, uidCounter, items, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetMediaPlayers(addr, uidCounter, items, label, status);

    Disable();
    obs.OnGetMediaPlayers(addr, uidCounter, items, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetFolderItems)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint16_t uidCounter = 1;
    std::vector<stub::MediaService::MediaItem> items;
    stub::MediaService::MediaItem testitem;
    testitem.playable_ = AVRC_MEDIA_FOLDER_PLAYABLE_RESERVED;
    items.push_back(testitem);
    uint8_t label = 0x00;
    int status = AVRC_ES_CODE_NO_ERROR;

    SetConnState(1);
    obs.OnGetFolderItems(addr, uidCounter, items, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetFolderItems(addr, uidCounter, items, label, status);

    Disable();
    obs.OnGetFolderItems(addr, uidCounter, items, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetItemAttributes)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    std::vector<uint32_t> attributes;
    attributes.push_back(5);
    std::vector<std::string> values;
    values.push_back("testvalue");
    uint8_t label = 0x00;
    int status = 1;

    SetConnState(1);
    obs.OnGetItemAttributes(addr, attributes, values, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetItemAttributes(addr, attributes, values, label, status);

    Disable();
    obs.OnGetItemAttributes(addr, attributes, values, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnGetTotalNumberOfItems)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint16_t uidCounter = 1;
    uint32_t numOfItems = 1;
    uint8_t label = 0x00;
    int status = AVRC_ES_CODE_NO_ERROR;

    SetConnState(1);
    obs.OnGetTotalNumberOfItems(addr, uidCounter, numOfItems, label, status);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnGetTotalNumberOfItems(addr, uidCounter, numOfItems, label, status);

    Disable();
    obs.OnGetTotalNumberOfItems(addr, uidCounter, numOfItems, label, status);
}

TEST_F(AvrcpTgService_test, ObserverImpl_OnSetAbsoluteVolume)
{
    AvrcpTgService::ObserverImpl obs;
    std::string addr = "00:00:00:00:00:11";
    uint8_t volume = 0x05;
    uint8_t label = 0x00;

    SetConnState(1);
    obs.OnSetAbsoluteVolume(addr, volume, label);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    SetConnState(2);
    obs.OnSetAbsoluteVolume(addr, volume, label);

    Disable();
    obs.OnSetAbsoluteVolume(addr, volume, label);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_CONNECT_IND_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    AvrcTgStateMachineManager *smManager = AvrcTgStateMachineManager::GetInstance();
    RawAddress bda("00:00:00:00:00:11");
    smManager->DeletePairOfStateMachine(bda);
    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_DISCONNECT_IND_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_DISCONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    SetConnState(1);

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), observer.connstate_);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_DISCONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_DISCONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    SetConnState(1);

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    EXPECT_STREQ("OnConnectionStateChanged", observer.funcname_.c_str());
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), observer.connstate_);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_BR_CONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_BR_DISCONNECT_IND_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_DISCONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_BR_DISCONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_DISCONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_CONNECT_CFM_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_CONNECT_CFM_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};

    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelEventCallback_AVCT_BR_CONNECT_IND_EVT_1)
{
    uint8_t connectId = 0x05;
    uint8_t event = AVCT_BR_CONNECT_IND_EVT;
    uint16_t result = AVCT_SUCCESS;
    BtAddr btAddr = {{0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    RawAddress bda("00:00:00:00:00:11");
    EXPECT_EQ(0, service->Connect(bda));
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
    
    GetAvctConnectParam().chEvtCallback(connectId, event, result, &btAddr, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_PASS_THROUGH_press_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t oper = AVRC_KEY_OPERATION_VOLUME_UP;
    uint8_t state = AVRC_KEY_STATE_PRESS;

    std::shared_ptr<AvrcCtPassPacket> packet = std::make_shared<AvrcCtPassPacket>(oper, state);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    uint8_t oper2 = AVRC_KEY_OPERATION_VOLUME_DOWN;

    std::shared_ptr<AvrcCtPassPacket> packet2 = std::make_shared<AvrcCtPassPacket>(oper2, state);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);

    uint8_t oper3 = AVRC_KEY_OPERATION_VOLUME_DOWN;

    std::shared_ptr<AvrcCtPassPacket> packet3 = std::make_shared<AvrcCtPassPacket>(oper3, state);
    Packet *pkt3 = (Packet *)packet3->AssemblePacket();
    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt3, nullptr);
    usleep(AVRC_TG_TEST_TIMEOUT);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_PASS_THROUGH_release_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t oper = AVRC_KEY_OPERATION_BACKWARD;
    uint8_t state = AVRC_KEY_STATE_RELEASE;

    std::shared_ptr<AvrcCtPassPacket> packet = std::make_shared<AvrcCtPassPacket>(oper, state);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_PASS_THROUGH_else_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t oper = 0xFF;
    uint8_t state = AVRC_KEY_STATE_PRESS;

    std::shared_ptr<AvrcCtPassPacket> packet = std::make_shared<AvrcCtPassPacket>(oper, state);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_UNIT_INFO_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtUnitPacket> packet = std::make_shared<AvrcCtUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_SUB_UNIT_INFO_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtSubUnitPacket> packet = std::make_shared<AvrcCtSubUnitPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_NORMAL);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gc_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t capabilityId = 0x03;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGcPacket>(capabilityId);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    capabilityId = 0x02;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtGcPacket>(capabilityId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Lpasa_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtLpasaPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Lpasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t attributes = 0x01;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtLpasvPacket>(attributes);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gcpasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::vector<uint8_t> attrs;
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_EQUALIZER);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_REPEAT);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SHUFFLE);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SCAN);
    for (int i = 0x05; i <= 0xFF; i++) {
        attrs.push_back(i);
    }

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGcpasvPacket>(attrs);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Spasv_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::vector<uint8_t> attrs;
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_EQUALIZER);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_REPEAT);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SHUFFLE);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SCAN);
    for (int i = 0x05; i <= 0xFF; i++) {
        attrs.push_back(i);
    }

    std::vector<uint8_t> vals;
    vals.push_back(bluetooth::AVRC_EQUALIZER_ON);
    vals.push_back(bluetooth::AVRC_REPEAT_SINGLE_TRACK);
    vals.push_back(bluetooth::AVRC_SHUFFLE_ALL_TRACKS);
    vals.push_back(bluetooth::AVRC_SCAN_ALL_TRACKS);
    for (int i = 0x05; i <= 0xFF; i++) {
        vals.push_back(bluetooth::AVRC_EQUALIZER_OFF);
    }

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtSpasvPacket>(attrs, vals);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gpasat_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::vector<uint8_t> attributes = {AVRC_PLAYER_ATTRIBUTE_REPEAT};

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGpasatPacket>(attributes);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gpasvt_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t attribtueId = AVRC_PLAYER_ATTRIBUTE_EQUALIZER;
    std::vector<uint8_t> values = {AVRC_EQUALIZER_OFF};

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGpasvtPacket>(attribtueId, values);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gea_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint64_t identifier = 1;
    std::vector<uint32_t> attributes = {0};

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGeaPacket>(identifier, attributes);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Sap_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint16_t playerId = 1;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtSapPacket>(playerId);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Gps_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGpsPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Rcr_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t pduId = 0x01;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtRcrPacket>(pduId);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Rcr_2)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtLpasaPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t pduId = 0x01;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtRcrPacket>(pduId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Rcr_3)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> values;
    for (int i = 0; i < 1000; i++) {
        values.push_back(i % 256);
    }
    AvrcpTgService::ObserverImpl obs;
    obs.OnGetPlayerAppSettingValues(addr, values, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t pduId = 0x01;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtRcrPacket>(pduId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Rcr_4)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> values;
    for (int i = 0; i < 1000; i++) {
        values.push_back(i % 256);
    }
    AvrcpTgService::ObserverImpl obs;
    obs.OnGetPlayerAppSettingValues(addr, values, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t pduId = AVRC_CT_PDU_ID_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtRcrPacket>(pduId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Acr_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t pduId = 0x02;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtAcrPacket>(pduId);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Acr_2)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtLpasaPacket>();
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t pduId = 0x02;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtAcrPacket>(pduId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Acr_3)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);
    std::string addr = "00:00:00:00:00:11";
    std::deque<uint8_t> values;
    for (int i = 0; i < 1000; i++) {
        values.push_back(i % 256);
    }
    AvrcpTgService::ObserverImpl obs;
    obs.OnGetPlayerAppSettingValues(addr, values, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t pduId = 0xFF;

    std::shared_ptr<AvrcCtVendorPacket> packet2 = std::make_shared<AvrcCtAcrPacket>(pduId);
    Packet *pkt2 = (Packet *)packet2->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt2, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Pi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t scope = AVRC_MEDIA_SCOPE_NOW_PLAYING;
    uint64_t uid = 0;
    uint16_t uidCounter = 0;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtPiPacket>(scope, uid, uidCounter);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Atnp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t scope = 0x00;
    uint64_t uid = 0;
    uint16_t uidCounter = 0;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtAtnpPacket>(scope, uid, uidCounter);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_VENDOR_Sav_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t volume = 0x7F;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtSavPacket>(volume);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_Browse_Sbp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    EXPECT_CALL(GetAvctpMocker(), AVCT_BrGetPeerMtu).WillRepeatedly(Return(100));

    SetConnState(1);

    uint16_t playerId = 1;

    std::shared_ptr<AvrcCtBrowsePacket> packet = std::make_shared<AvrcCtSbpPacket>(playerId);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_Browse_Cp_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    SetConnState(1);

    uint16_t uidCounter = 0;
    uint8_t direction = AVRC_FOLDER_DIRECTION_DOWN;
    uint64_t folderUid = 0;

    std::shared_ptr<AvrcCtBrowsePacket> packet = std::make_shared<AvrcCtCpPacket>(uidCounter, direction, folderUid);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_Browse_Gfi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    SetConnState(1);

    uint8_t scope = 0x01;
    uint32_t start = 2;
    uint32_t end = 4;
    std::vector<uint32_t> attributes{0};

    std::shared_ptr<AvrcCtBrowsePacket> packet = std::make_shared<AvrcCtGfiPacket>(scope, start, end, attributes);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_Browse_Gia_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    SetConnState(1);

    uint8_t scope = AVRC_MEDIA_SCOPE_NOW_PLAYING;
    uint64_t uid = 0;
    uint16_t uidCounter = 0;
    std::vector<uint32_t> attributes;

    std::shared_ptr<AvrcCtBrowsePacket> packet = std::make_shared<AvrcCtGiaPacket>(scope, uid, uidCounter, attributes);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, ChannelMessageCallback_Browse_Gtnoi_1)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_BR;

    SetConnState(1);

    uint8_t scope = 0x02;

    std::shared_ptr<AvrcCtBrowsePacket> packet = std::make_shared<AvrcCtGtnoiPacket>(scope);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_PlaybackStatusChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_PLAYBACK_STATUS_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t playStatus = AVRC_PLAY_STATUS_PAUSED;
    uint32_t playbackPos = 0;

    service->NotifyPlaybackStatusChanged(playStatus, playbackPos, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_TrackChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_TRACK_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint64_t uid = 0;
    uint32_t playbackPos = 0;

    service->NotifyTrackChanged(uid, playbackPos, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_TrackReachedEnd)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_TRACK_REACHED_END;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint32_t playbackPos = 0;

    service->NotifyTrackReachedEnd(playbackPos, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_TrackReachedStart)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_TRACK_REACHED_START;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint32_t playbackPos = 0;

    service->NotifyTrackReachedStart(playbackPos, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_PlaybackPosChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_PLAYBACK_POS_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint32_t playbackPos = 0;

    service->NotifyPlaybackPosChanged(playbackPos, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_PlayerAppSettingChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    std::deque<uint8_t> attrs;
    std::deque<uint8_t> vals;
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_EQUALIZER);
    vals.push_back(bluetooth::AVRC_EQUALIZER_ON);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_REPEAT);
    vals.push_back(bluetooth::AVRC_REPEAT_SINGLE_TRACK);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SHUFFLE);
    vals.push_back(bluetooth::AVRC_SHUFFLE_OFF);
    attrs.push_back(bluetooth::AVRC_PLAYER_ATTRIBUTE_SCAN);
    vals.push_back(bluetooth::AVRC_SCAN_OFF);

    service->NotifyPlayerAppSettingChanged(attrs, vals, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_NowPlayingContentChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    service->NotifyNowPlayingContentChanged(label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_AvailablePlayersChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_AVAILABLE_PLAYERS_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    service->NotifyAvailablePlayersChanged(label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_AddressedPlayerChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_ADDRESSED_PLAYER_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint16_t playerId = 1;
    uint16_t uidCounter = 1;

    service->NotifyAddressedPlayerChanged(playerId, uidCounter, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_UidChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_UIDS_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint16_t uidCounter = 2;

    service->NotifyUidChanged(uidCounter, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}

TEST_F(AvrcpTgService_test, RegisterNotification_VolumeChanged)
{
    uint8_t connectId = 0x05;
    uint8_t label = 0x00;
    uint8_t crType = 0x00;
    uint8_t chType = AVCT_DATA_CTRL;

    SetConnState(1);

    uint8_t eventId = AVRC_CT_EVENT_ID_VOLUME_CHANGED;

    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtNotifyPacket>(eventId, label);
    Packet *pkt = (Packet *)packet->AssemblePacket();

    GetAvctConnectParam().msgCallback(connectId, label, crType, chType, pkt, nullptr);
    usleep(AVRC_TG_TEST_TIMER_LONG);

    uint8_t volume = AVRC_ABSOLUTE_VOLUME_PERCENTAGE_100;

    service->NotifyVolumeChanged(volume, label);
    usleep(AVRC_TG_TEST_TIMER_LONG);
}