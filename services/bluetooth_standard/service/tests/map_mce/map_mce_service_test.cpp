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
#include "mce_mock/mce_mock.h"
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

using namespace testing;
using namespace bluetooth;

#define MCE_MAX_OF_OSERVER_TEST 10
int actCompleteTimes = 0;

void MapMceUintTestCallback::OnMapActionCompleted(
    const RawAddress &deviceAddress, const IProfileMapAction &action, MapExecuteStatus status)
{
    LOG_INFO("%s excute,this=%p", __PRETTY_FUNCTION__, this);
    if (status == MapExecuteStatus::SUCCEED) {
        callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_RET_NO_ERROR;
        if (action.action_ == MapActionType::GET_SUPPORTED_FEATURES) {
            LOG_INFO("%s action=GET_SUPPORTED_FEATURES,value=%x", __PRETTY_FUNCTION__, action.supportedFeatures_);
        }
    } else {
        callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_RET_OPRATION_FAILED;
    }
    actCompleteTimes++;
}

void MapMceUintTestCallback::OnMapEventReported(const RawAddress &deviceAddress, const IProfileMapEventReport &report)
{
    LOG_INFO("%s excute,this=%p", __PRETTY_FUNCTION__, this);
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_ON_MAP_EVENT_REPORTED;
}

void MapMceUintTestCallback::OnConnectionStateChanged(const RawAddress &deviceAddress, int state)
{
    LOG_INFO("%s excute,this=%p", __PRETTY_FUNCTION__, this);
    callbackConnectionStatus = state;
}

void MapMceUintTestCallback::OnBmessageCompleted(
    const RawAddress &deviceAddress, const IProfileBMessage &bmsg, MapExecuteStatus status)
{
    actCompleteTimes++;
}

void MapMceUintTestCallback::OnMessagesListingCompleted(
    const RawAddress &deviceAddress, const IProfileMessagesListing &listing, MapExecuteStatus status)
{
    actCompleteTimes++;
}

void MapMceUintTestCallback::OnConversationListingCompleted(
    const RawAddress &deviceAddress, const IProfileConversationListing &listing, MapExecuteStatus status)
{
    actCompleteTimes++;
}

TEST(MapMceServiceTest, startup_shutdown)
{
    MapMceService *mapServcie = nullptr;

    StrictMock<ProfileConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> newInstanceMock;
    registerNewMockProfileConfig(&newInstanceMock);
    EXPECT_CALL(newInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&configMock));
    EXPECT_CALL(configMock, SetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Return(false));
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

    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    RawAddress mockerDevice("00:00:01:00:00:7F");
    MapMceUintTestCallback mapMceCallback;  // create callback
    mapServcie->RegisterObserver(mapMceCallback);
    char tempPtr[] = "temp";
    IProfileGetMessagesListingParameters para1;
    para1.FilterPeriodBegin = "test";
    para1.FilterPeriodEnd = tempPtr;
    para1.FilterRecipient = tempPtr;
    para1.FilterOriginator = tempPtr;
    para1.ConversationID = "testConversationID";
    para1.FilterMessageHandle = "testFilterMessageHandle";
    para1.FilterReadStatus = 1;

    IProfileGetMessageParameters para2;
    IProfileGetConversationListingParameters para3;
    para3.FilterRecipient = tempPtr;
    para3.FilterLastActivityBegin = tempPtr;
    para3.FilterLastActivityEnd = tempPtr;
    para3.ConversationID = "testConversationID";

    IProfileSetOwnerStatusParameters para4;
    para4.ownerStatus_.PresenceText = "temp";
    para4.ownerStatus_.LastActivity = "temp2";
    para4.ConversationID = "testConversationID";

    // default return test
    MapStatusIndicatorType statusIndicator = MapStatusIndicatorType::DELETED_STATUS;
    MapStatusValueType statusValue = MapStatusValueType::YES;
    IProfileSetMessageStatus mapStatus;
    mapStatus.msgHandle = u"test";
    mapStatus.statusIndicator = statusIndicator;
    mapStatus.statusValue = statusValue;
    mapStatus.extendedData = "";

    int strategy = (int)BTStrategyType::CONNECTION_ALLOWED;

    IProfileSendMessageParameters msg;
    msg.bmessage_.version_property = "tempVersion";
    msg.bmessage_.folder_property = u"tempfolder";
    msg.bmessage_.envelope_.msgBody_.body_charset = "tempcharset";
    msg.bmessage_.envelope_.msgBody_.body_language = "templanguage";
    msg.bmessage_.envelope_.msgBody_.body_content = "tempmessage body";
    msg.bmessage_.envelope_.maxLevelOfEnvelope_ = 1;
    msg.Charset = MapCharsetType::UTF_8;
    msg.bmessage_.folder_property = u"inbox";
    MapMessageType type = MapMessageType::SMS_GSM;

    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetMessagesListing(mockerDevice, para1));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetMessage(mockerDevice, type, u"test", para2));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->UpdateInbox(mockerDevice, type));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetConversationListing(mockerDevice, para3));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetMessageStatus(mockerDevice, type, mapStatus));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetOwnerStatus(mockerDevice, para4));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetOwnerStatus(mockerDevice, "test"));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetConnectionStrategy(mockerDevice, strategy));
    EXPECT_EQ((int)BTStrategyType::CONNECTION_UNKNOWN, mapServcie->GetConnectionStrategy(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetUnreadMessages(mockerDevice, type, 9));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SendMessage(mockerDevice, msg));

    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
    actCompleteTimes = 0;
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetSupportedFeatures(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(BT_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(1, actCompleteTimes);
    EXPECT_EQ(MCE_TEST_EXCUTE_STATUS_RET_NO_ERROR, callbackExcuteStatus);
    // shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    // bad status test
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Connect(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Connect(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetNotificationRegistration(mockerDevice, true));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetMessagesListing(mockerDevice, para1));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetMessage(mockerDevice, type, u"test", para2));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->UpdateInbox(mockerDevice, type));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetConversationListing(mockerDevice, para3));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetMessageStatus(mockerDevice, type, mapStatus));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetOwnerStatus(mockerDevice, para4));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetOwnerStatus(mockerDevice, "test"));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SetConnectionStrategy(mockerDevice, strategy));
    EXPECT_EQ((int)BTStrategyType::CONNECTION_FORBIDDEN, mapServcie->GetConnectionStrategy(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetUnreadMessages(mockerDevice, type, 8));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->GetSupportedFeatures(mockerDevice));
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->SendMessage(mockerDevice, msg));

    delete mapServcie;
    usleep(200000);  // 200 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connected_occor_obexerro)
{
    ObexConnectParams connectParams;
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(BT_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }

    // occor transport erro
    obexConnectCtrl = MCE_TEST_CTRL_TRANSPORT_ERRO;
    connectParams.appParams_ = nullptr;
    obexClient->Connect(connectParams);  // OnActionCompleted   //debug code, res actinog commlite
    usleep(200000);                      // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // test setup
    delete mapServcie;
    usleep(200000);  // 200 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connect_disconnect)
{
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
    callbackConnectionStatus = (int)BTConnectState::DISCONNECTED;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(PROFILE_STATE_DISCONNECTED, mapServcie->GetConnectState());
    EXPECT_EQ((int)BTConnectState::DISCONNECTED, mapServcie->GetDeviceConnectState(mockerDevice));
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // regist callbcack
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    MapMceUintTestCallback mapMceCallback;  // create callback
    mapServcie->RegisterObserver(mapMceCallback);

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Connect(mockerDevice));
    EXPECT_EQ(PROFILE_STATE_CONNECTING, mapServcie->GetConnectState());
    EXPECT_EQ((int)BTConnectState::CONNECTING, mapServcie->GetDeviceConnectState(mockerDevice));

    LOG_INFO("%s TEST_MasServiceSdpSearchCallback debug", __FUNCTION__);

    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Connect(mockerDevice));
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }
    EXPECT_EQ(PROFILE_STATE_CONNECTED, mapServcie->GetConnectState());
    EXPECT_EQ((int)BTConnectState::CONNECTED, mapServcie->GetDeviceConnectState(mockerDevice));
    EXPECT_EQ((int)BTConnectState::CONNECTED, callbackConnectionStatus);  // test call backexcute
    std::vector<int> statusList;
    statusList.push_back((int)BTConnectState::CONNECTED);
    std::vector<RawAddress> deviceList = mapServcie->GetDevicesByStates(statusList);
    if (deviceList.size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", deviceList.begin()->GetAddress());
    } else {
        EXPECT_EQ(1, 0);
    }
    EXPECT_EQ(1, mapServcie->GetMaxConnectNum());
    // disconnect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    EXPECT_EQ((int)BTConnectState::DISCONNECTED, callbackConnectionStatus);  // test call backexcute

    // deregist
    mapServcie->DeregisterObserver(mapMceCallback);

    // test another connect disconnect
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }
    // disconnect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    delete mapServcie;
    usleep(200000);  // 200 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connecting_gap_failed)
{
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_NG;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms

    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // disconnect test
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // release memory
    delete mapServcie;
    usleep(200000);  // 500 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connecting_sdp_failed)
{
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_NG;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms

    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // disconnect test
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // release memory
    delete mapServcie;
    usleep(200000);  // 500 ms
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connecting_obex_failed)
{
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_FAILED;  // connect fail control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms

    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // disconnect test
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // release memory
    delete mapServcie;
    usleep(200000);  // 500 ms
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connecting_obex_transporterro)
{
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_TRANSPORT_ERRO;  // connect transport erro control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
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

    // startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms

    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // disconnect test
    EXPECT_EQ(RET_BAD_STATUS, mapServcie->Disconnect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));
    EXPECT_EQ((const unsigned long int)0, mapServcie->GetConnectDevices().size());

    // release memory
    delete mapServcie;
    usleep(200000);  // 500 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connected_shutdown)
{
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
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

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }

    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 500 ms
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connected_sendrequestOkNg)
{
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;  // send request ok
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;

    char tempPtr[] = "temp";
    IProfileGetMessagesListingParameters para1;
    para1.FilterPeriodBegin = "test";
    para1.FilterPeriodEnd = tempPtr;
    para1.FilterRecipient = tempPtr;
    para1.FilterOriginator = tempPtr;
    para1.ConversationID = "testConversationID";
    para1.FilterMessageHandle = "testFilterMessageHandle";
    para1.FilterReadStatus = 1;

    IProfileGetMessageParameters para2;
    IProfileGetConversationListingParameters para3;
    para3.FilterRecipient = tempPtr;
    para3.FilterLastActivityBegin = tempPtr;
    para3.FilterLastActivityEnd = tempPtr;
    para3.ConversationID = "testConversationID";

    IProfileSetOwnerStatusParameters para4;
    para4.ownerStatus_.PresenceText = "temp";
    para4.ownerStatus_.LastActivity = "temp2";
    para4.ConversationID = "testConversationID";

    // SetMessageStatus
    MapStatusIndicatorType statusIndicator = MapStatusIndicatorType::DELETED_STATUS;
    MapStatusValueType statusValue = MapStatusValueType::YES;
    IProfileSetMessageStatus mapStatus;
    mapStatus.msgHandle = u"test";
    mapStatus.statusIndicator = statusIndicator;
    mapStatus.statusValue = statusValue;
    mapStatus.extendedData = "";

    IProfileSendMessageParameters msg;
    msg.bmessage_.version_property = "tempVersion";
    msg.bmessage_.folder_property = u"tempfolder";
    msg.bmessage_.envelope_.msgBody_.body_charset = "tempcharset";
    msg.bmessage_.envelope_.msgBody_.body_language = "templanguage";
    msg.bmessage_.envelope_.msgBody_.body_content = "tempmessage body";
    msg.bmessage_.envelope_.maxLevelOfEnvelope_ = 1;
    msg.Charset = MapCharsetType::UTF_8;
    msg.bmessage_.folder_property = u"inbox";
    MapMessageType type = MapMessageType::SMS_GSM;

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

    // regist callbcack
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    MapMceUintTestCallback mapMceCallback;  // create callback
    mapServcie->RegisterObserver(mapMceCallback);

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }

    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;  // send request ok
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    actCompleteTimes = 0;
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x01));
    usleep(200000);  // 200 ms
    EXPECT_EQ(MCE_TEST_EXCUTE_STATUS_RET_NO_ERROR, callbackExcuteStatus);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xff));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfefefefe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x00));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x78));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfff));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xeee));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x777));
    usleep(300000);  // 300 ms
    EXPECT_EQ(8, actCompleteTimes);

    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_NG;  // send request ng
    actCompleteTimes = 0;
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfff));
    usleep(200000);  // 200 ms
    EXPECT_EQ(MCE_TEST_EXCUTE_STATUS_RET_OPRATION_FAILED, callbackExcuteStatus);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xeee));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x00));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xeee));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x78));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfefefefe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x88));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x77));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x44));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x11));
    usleep(300000);  // 300 ms
    EXPECT_EQ(11, actCompleteTimes);

    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessagesListing(mockerDevice, para1));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessage(mockerDevice, type, u"test", para2));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->UpdateInbox(mockerDevice, type));
    usleep(200000);  // 200 ms
    EXPECT_EQ(14, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetConversationListing(mockerDevice, para3));
    usleep(100000);  // 100 ms
    EXPECT_EQ(15, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetMessageStatus(mockerDevice, type, mapStatus));
    usleep(100000);  // 100 ms
    EXPECT_EQ(16, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetOwnerStatus(mockerDevice, para4));
    usleep(100000);  // 100 ms
    EXPECT_EQ(17, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetOwnerStatus(mockerDevice, "test"));
    usleep(100000);  // 100 ms
    EXPECT_EQ(18, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SendMessage(mockerDevice, msg));
    usleep(100000);  // 100 ms
    EXPECT_EQ(19, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetUnreadMessages(mockerDevice, type, 8));
    usleep(100000);  // 100 ms
    EXPECT_EQ(20, actCompleteTimes);
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationRegistration(mockerDevice, true));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetFolderListing(mockerDevice, 10, 0));
    usleep(200000);  // 200 ms
    std::vector<std::u16string> folderList;
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetPath(mockerDevice, 2, u"folder", folderList));
    usleep(200000);  // 200 ms
    IProfileMasInstanceInfoList instanceInfo = mapServcie->GetMasInstanceInfo(mockerDevice);
    usleep(100000);                   // 100 ms
    EXPECT_EQ(20, actCompleteTimes);  // intenal command , do not excute callback observer

    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 500 ms
    obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connected_sendrequestNoResponse)
{
    ObexConnectParams connectParams;
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;  // request ok
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;

    char tempPtr[] = "temp";
    IProfileGetMessagesListingParameters para1;
    para1.FilterPeriodBegin = "test";
    para1.FilterPeriodEnd = tempPtr;
    para1.FilterRecipient = tempPtr;
    para1.FilterOriginator = tempPtr;
    para1.ConversationID = "testConversationID";
    para1.FilterMessageHandle = "testFilterMessageHandle";
    para1.FilterReadStatus = 1;

    IProfileGetMessageParameters para2;
    IProfileGetConversationListingParameters para3;
    para3.FilterRecipient = tempPtr;
    para3.FilterLastActivityBegin = tempPtr;
    para3.FilterLastActivityEnd = tempPtr;
    para3.ConversationID = "testConversationID";

    IProfileSetOwnerStatusParameters para4;
    para4.ownerStatus_.PresenceText = "temp";
    para4.ownerStatus_.LastActivity = "temp2";
    para4.ConversationID = "testConversationID";

    MapStatusIndicatorType statusIndicator = MapStatusIndicatorType::DELETED_STATUS;
    MapStatusValueType statusValue = MapStatusValueType::YES;
    MapMessageType type = MapMessageType::SMS_GSM;
    IProfileSetMessageStatus mapStatus;
    mapStatus.msgHandle = u"test";
    mapStatus.statusIndicator = statusIndicator;
    mapStatus.statusValue = statusValue;
    mapStatus.extendedData = "";

    IProfileSendMessageParameters msg;
    msg.bmessage_.version_property = "tempVersion";
    msg.bmessage_.folder_property = u"tempfolder";
    msg.bmessage_.envelope_.msgBody_.body_charset = "tempcharset";
    msg.bmessage_.envelope_.msgBody_.body_language = "templanguage";
    msg.bmessage_.envelope_.msgBody_.body_content = "tempmessage body";
    msg.bmessage_.envelope_.maxLevelOfEnvelope_ = 1;
    msg.Charset = MapCharsetType::UTF_8;
    msg.bmessage_.folder_property = u"inbox";

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

    // regist callbcack
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    MapMceUintTestCallback mapMceCallback;  // create callback
    mapServcie->RegisterObserver(mapMceCallback);

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }

    obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(MCE_TEST_EXCUTE_STATUS_IDLE, callbackExcuteStatus);
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_NOTIFICATION_FILTER));
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(6, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_NOTIFICATION_FILTER));
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0x55));
    usleep(300000);  // 300 ms
    EXPECT_EQ(18, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_NOTIFICATION_FILTER));
    EXPECT_EQ(18, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_ALL));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_IDLE));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_FOLDERLISTING));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_MESSAGE));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SEND_MESSAGE));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_MESSAGE_STATUS));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_UNREAD_MESSAGES));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_MASINSTANCE));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_UPDATE_INBOX));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_OWNER_STATUS));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_OWNER_STATUS));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_CONVERSATION_LISTING));

    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetSupportedFeatures(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessagesListing(mockerDevice, para1));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessage(mockerDevice, type, u"test", para2));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->UpdateInbox(mockerDevice, type));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetConversationListing(mockerDevice, para3));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetMessageStatus(mockerDevice, type, mapStatus));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetOwnerStatus(mockerDevice, para4));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetOwnerStatus(mockerDevice, "test"));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetUnreadMessages(mockerDevice, type, 8));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SendMessage(mockerDevice, msg));
    usleep(300000);  // 300 ms

    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_IDLE));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_FOLDERLISTING));
    EXPECT_EQ(0, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_MASINSTANCE));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_MESSAGE));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_MESSAGELISTING));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SEND_MESSAGE));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_MESSAGE_STATUS));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_UNREAD_MESSAGES));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_UPDATE_INBOX));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_SET_OWNER_STATUS));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_OWNER_STATUS));
    EXPECT_EQ(1, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_GET_CONVERSATION_LISTING));
    EXPECT_EQ(27, mapServcie->CountSendingRequest(mockerDevice, MCE_REQUEST_TYPE_ALL));
    // client shutdown test
    mapServcie->Disable();
    usleep(100000);                                                        // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWNING, mapServcie->GetServiceStates());  // when request sending callback
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    // release memory
    for (int i = 0; i < 27; i++) {
        // occor send complete ok
        obexConnectCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
        connectParams.appParams_ = nullptr;
        obexClient->Connect(connectParams);  // OnActionCompleted   //debug code, res actinog commlite
    }
    usleep(300000);                                                     // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());  // when request sending callback
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(100000);  // 500 ms
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
    EXPECT_EQ(0, 0);
}

TEST(MapMceServiceTest, connected_sendrequestSendOprationFailed)
{
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;  // request ok
    uint8_t masInstanceId = 1;
    uint32_t masRfcomId = 0;
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;  // connect control
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
    sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;

    char tempPtr[] = "temp";
    IProfileGetMessagesListingParameters para1;
    para1.FilterPeriodBegin = "test";
    para1.FilterPeriodEnd = tempPtr;
    para1.FilterRecipient = tempPtr;
    para1.FilterOriginator = tempPtr;
    para1.ConversationID = "testConversationID";
    para1.FilterMessageHandle = "testFilterMessageHandle";
    para1.FilterReadStatus = 1;

    IProfileGetMessageParameters para2;
    IProfileGetConversationListingParameters para3;
    para3.FilterRecipient = tempPtr;
    para3.FilterLastActivityBegin = tempPtr;
    para3.FilterLastActivityEnd = tempPtr;
    para3.ConversationID = "testConversationID";

    IProfileSetOwnerStatusParameters para4;
    para4.ownerStatus_.PresenceText = "temp";
    para4.ownerStatus_.LastActivity = "temp2";
    para4.ConversationID = "testConversationID";

    // default return test
    MapStatusIndicatorType statusIndicator = MapStatusIndicatorType::DELETED_STATUS;
    MapStatusValueType statusValue = MapStatusValueType::YES;
    IProfileSetMessageStatus mapStatus;
    mapStatus.msgHandle = u"test";
    mapStatus.statusIndicator = statusIndicator;
    mapStatus.statusValue = statusValue;
    mapStatus.extendedData = "";

    MapMessageType type = MapMessageType::SMS_GSM;

    IProfileSendMessageParameters msg;
    msg.bmessage_.version_property = "tempVersion";
    msg.bmessage_.folder_property = u"tempfolder";
    msg.bmessage_.envelope_.msgBody_.body_charset = "tempcharset";
    msg.bmessage_.envelope_.msgBody_.body_language = "templanguage";
    msg.bmessage_.envelope_.msgBody_.body_content = "tempmessage body";
    msg.bmessage_.envelope_.maxLevelOfEnvelope_ = 1;
    msg.Charset = MapCharsetType::UTF_8;
    msg.bmessage_.folder_property = u"inbox";

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

    // regist callbcack
    callbackExcuteStatus = MCE_TEST_EXCUTE_STATUS_IDLE;
    MapMceUintTestCallback mapMceCallback;  // create callback
    mapServcie->RegisterObserver(mapMceCallback);

    // client startup test
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    mapServcie->Enable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_STARTUP, mapServcie->GetServiceStates());

    // connect test
    EXPECT_EQ(RET_NO_ERROR, mapServcie->Connect(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, TEST_MasServiceSdpSearchCallback(masInstanceId, masRfcomId));
    usleep(200000);  // 200 ms
    EXPECT_EQ(true, mapServcie->IsConnected(mockerDevice));
    EXPECT_NE((const unsigned long int)0, mapServcie->GetConnectDevices().size());
    if (mapServcie->GetConnectDevices().size() != 0) {
        EXPECT_EQ("00:00:01:00:00:7F", mapServcie->GetConnectDevices().begin()->GetAddress());
    }
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationFilter(mockerDevice, 0xfe));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetSupportedFeatures(mockerDevice));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessagesListing(mockerDevice, para1));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetMessage(mockerDevice, type, u"test", para2));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->UpdateInbox(mockerDevice, type));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetConversationListing(mockerDevice, para3));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetMessageStatus(mockerDevice, type, mapStatus));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetOwnerStatus(mockerDevice, para4));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetOwnerStatus(mockerDevice, "test"));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->GetUnreadMessages(mockerDevice, type, 2));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SendMessage(mockerDevice, msg));
    usleep(200000);  // 200 ms
    EXPECT_EQ(RET_NO_ERROR, mapServcie->SetNotificationRegistration(mockerDevice, true));
    usleep(200000);  // 200 ms
    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 500 ms
    obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;
    EXPECT_EQ(0, 0);
}

TEST(IProfileMapMceService, VirtualFunctionTest)
{
    MapMceUintTestCallback2 *testCallback = new MapMceUintTestCallback2();
    IProfileMapMceObserver *virtCallback = (IProfileMapMceObserver *)testCallback;
    RawAddress mockerDevice("00:00:01:00:00:7F");
    IProfileBMessage bmsg;
    IProfileMessagesListing msgList;
    IProfileMapEventReport eventReport;
    IProfileConversationListing conversationListing;
    IProfileMapEventReport report;

    IProfileMapAction action;
    MapRequestResponseAction retAction;

    virtCallback->OnMapActionCompleted(mockerDevice, action, MapExecuteStatus::SUCCEED);
    virtCallback->OnBmessageCompleted(mockerDevice, bmsg, MapExecuteStatus::SUCCEED);
    virtCallback->OnConnectionStateChanged(mockerDevice, 0);
    virtCallback->OnMessagesListingCompleted(mockerDevice, msgList, MapExecuteStatus::SUCCEED);
    virtCallback->OnConversationListingCompleted(mockerDevice, conversationListing, MapExecuteStatus::SUCCEED);
    virtCallback->OnMapEventReported(mockerDevice, report);

    delete virtCallback;
    EXPECT_EQ(0, 0);
}
