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
#include "mce_mock/mce_mock.h"
#include "common_mock_all.h"

extern "C" {
#include "btstack.h"
#include "btm.h"
#include "log.h"
}

using namespace testing;
using namespace bluetooth;
MapMceObserverManager tempObserverMgr2{};

TEST(MapMceInstanceStm, Construct)
{
    int instanceId = 8;
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
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, tempObserverMgr2);
    EXPECT_EQ(MAP_MCE_DEV_STATE_DISCONNECTED, mceDeviceCtrl->GetCurrentDeviceState());
    EXPECT_EQ(MAP_MCE_DEV_STATE_CONNECTED, mceDeviceCtrl->GetTargetDeviceState());
    MapMceInstanceStm *mceInstanceStm =
        new MapMceInstanceStm(*mceDeviceCtrl, *(mapServcie->GetDispatcher()), instanceId, config);
    mceInstanceStm->CreateStm();
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());

    delete mceInstanceStm;
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

TEST(MapMceInstanceStm, status_change_test)
{
    ObexConnectParams connectParams;
    int instanceId = 8;
    RawAddress mockerDevice("00:00:01:00:00:7F");
    obexConnectCtrl = MCE_TEST_CTRL_NO_RESPONES;
    obexDisconnectCtrl = MCE_TEST_CTRL_NO_RESPONES;
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;

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
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, tempObserverMgr2);
    MapMceInstanceStm *mceInstanceStm =
        new MapMceInstanceStm(*mceDeviceCtrl, *(mapServcie->GetDispatcher()), instanceId, config);
    mceInstanceStm->CreateStm();
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());

    // disconnected status test
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_RECEIVE_REQUEST_COMPLETED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_TRANSPORT_FAILED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED_FAILED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_SEND_REQUEST);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());

    // connecting status test
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);  // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_RECEIVE_REQUEST_COMPLETED);  // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_SEND_REQUEST);  // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    utility::Message outMsg(MSG_MASSTM_OBEX_TRANSPORT_FAILED);
    outMsg.arg1_ = 6;
    mceInstanceStm->MceProcessMessage(outMsg);  // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_TRANSPORT_FAILED);  // test case excute
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                       // connecting again
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());       // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED_FAILED);             // test case excute
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                       // connecting again
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());       // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);                 // test case excute
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                       // connecting again
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());       // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);                    // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connecting, target disconnect
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);
    usleep(100000);  // 100 ms
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connecting again , target connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);                   // connecting again
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());      // connecting again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                         // connecting again
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());    // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());         // connecting again
    // target connect
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);  // test case excute
    usleep(100000);                                                // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());

    obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;
    obexConnectCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;

    mceInstanceStm->GetMasClient().ClientSendReqGetMasInstanceInformation();        // test case excute
    usleep(100000);                                                                 // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE_S_REQSENDING, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_GET_MASINSTANCE));
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_ALL));
    EXPECT_EQ(0, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_IDLE));
    // connected again
    connectParams.appParams_ = nullptr;
    obexClient->Connect(connectParams);                                             // OnActionCompleted  //debug code
    usleep(100000);                                                                 // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connected again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());      // connected again
    int setpathFlag = MASSTM_SETPATH_DOWN;
    std::u16string setpathPaths = u"MSG";
    std::vector<std::u16string> setpathPathList;
    mceInstanceStm->GetMasClient().ClientSendReqSetPath(setpathFlag, setpathPaths, setpathPathList);
    usleep(100000);  // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE_S_REQSENDING, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_SET_PATH));
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_ALL));
    EXPECT_EQ(0, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_IDLE));

    // connected again
    connectParams.appParams_ = nullptr;
    obexClient->Connect(connectParams);  // OnActionCompleted   //debug code, res actinog commlite
    usleep(100000);                      // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connected again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());      // connected again
    mceInstanceStm->GetMasClient().ClientSendReqSetNotificationRegistration(true);  // excute test case
    usleep(100000);                                                                 // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE_S_REQSENDING, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_SET_NOTIFICATION_REGIST));
    EXPECT_EQ(1, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_ALL));
    EXPECT_EQ(0, mceInstanceStm->GetMasClient().ClientCountSendingRequest(MCE_REQUEST_TYPE_IDLE));

    // connected again
    connectParams.appParams_ = nullptr;
    obexClient->Connect(connectParams);  // OnActionCompleted   //debug code, res actinog commlite
    usleep(100000);                      // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connected again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());      // connected again
    obexConnectCtrl = MCE_TEST_CTRL_NO_RESPONES;                                    // connect control recover
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;

    // normal event test
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_RECEIVE_REQUEST_COMPLETED);
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);
    usleep(100000);  // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED_FAILED);
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);     // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);  // connected again
    usleep(100000);                                                // 100 ms wait connect send request finish
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connected again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());      // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_TRANSPORT_FAILED);            // test case excute
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);     // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);  // connected again
    usleep(100000);                                                // 100 ms wait connect send request finish
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connected again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());      // connected again
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);                   // test case excute
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());

    // disconnectting status test
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_RECEIVE_REQUEST_COMPLETED);
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_SEND_REQUEST);  // test case excute
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // disconnectting agian, target connected
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);     // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);  // disconnectting agian
    usleep(100000);                                                // 100 ms wait connect send request finish
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);  // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);     // disconnectting agian
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // disconnectting agian
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());          // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_DISCONNECTED);                    // test case excute
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);  // disconnectting agian
    usleep(100000);                                                // 100 ms wait connect send request finish
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);  // disconnectting agian
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // disconnectting agian
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());       // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_TRANSPORT_FAILED);                // test case excute
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());
    // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                          // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);                       // disconnectting agian
    usleep(100000);                                                                     // 100 ms
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_DISCONNECT);                       // disconnectting agian
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // disconnectting agian
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());       // disconnectting agian
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED_FAILED);                // test case excute
    EXPECT_THAT(MCE_DISCONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());

    delete mceInstanceStm;
    delete mceDeviceCtrl;

    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 200 ms
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;
    EXPECT_EQ(0, 0);
}

TEST(MapMceInstanceStm, internal_sendrequestok)
{
    int instanceId = 8;
    // bool compareResult = false;
    RawAddress mockerDevice("00:00:01:00:00:7F");

    // disable obex_mock callback
    obexConnectCtrl = MCE_TEST_CTRL_NO_RESPONES;
    obexDisconnectCtrl = MCE_TEST_CTRL_NO_RESPONES;
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;

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
        new MapMceDeviceCtrl(mockerDevice.GetAddress(), *mapServcie, true, config, tempObserverMgr2);
    MapMceInstanceStm *mceInstanceStm =
        new MapMceInstanceStm(*mceDeviceCtrl, *(mapServcie->GetDispatcher()), instanceId, config);
    mceInstanceStm->CreateStm();
    EXPECT_THAT(MCE_DISCONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_DISCONNECTED, mceInstanceStm->GetTargetState());

    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_CONNECT);                       // connecting again
    EXPECT_THAT(MCE_CONNECTING_STATE, mceInstanceStm->GetCurrentMceStmStateName());  // connecting again
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());       // connecting again
    // target connect
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_OBEX_CONNECTED);
    usleep(100000);  // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());  // connected again

    // MSG_MASSTM_REQ_GET_INSTANCE internal request message test
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;               // send request ok
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_GET_INSTANCE);  // test case excute
    usleep(100000);                                                  // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_NG;               // send request ng
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_GET_INSTANCE);  // test case excute
    usleep(100000);                                                  // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());

    // MSG_MASSTM_REQ_GET_FOLDER_LISTING internal request message test
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;                     // send request ok
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_GET_FOLDER_LISTING);  // test case excute
    usleep(100000);                                                        // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_NG;                     // send request ng
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_GET_FOLDER_LISTING);  // test case excute
    usleep(100000);                                                        // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());

    // MSG_MASSTM_REQ_SET_PATH internal request message test
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_OK;           // send request ok
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_SET_PATH);  // test case excute
    usleep(100000);                                              // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());
    obexPutGetCtrl = MCE_TEST_CTRL_ACTION_COMPLETE_NG;           // send request ng
    mceInstanceStm->MceProcessMessage(MSG_MASSTM_REQ_SET_PATH);  // test case excute
    usleep(100000);                                              // 100 ms
    EXPECT_THAT(MCE_CONNECTED_STATE, mceInstanceStm->GetCurrentMceStmStateName());
    EXPECT_EQ(MCE_INSTANCE_STATE_CONNECTED, mceInstanceStm->GetTargetState());

    delete mceInstanceStm;
    delete mceDeviceCtrl;

    // client shutdown test
    mapServcie->Disable();
    usleep(200000);  // 200 ms
    EXPECT_EQ(MAP_MCE_STATE_SHUTDOWN, mapServcie->GetServiceStates());
    EXPECT_EQ(false, mapServcie->IsConnected(mockerDevice));

    delete mapServcie;
    usleep(200000);  // 200 ms
    obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;
    obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
    obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;
    EXPECT_EQ(0, 0);
}
