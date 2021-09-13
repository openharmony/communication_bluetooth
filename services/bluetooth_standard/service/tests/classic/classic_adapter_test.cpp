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

#include "classic_mock.h"
#include "classic_data_type_defs.h"
#include <string>
#include <iostream>
#include "classic_battery_observer.h"
using namespace bluetooth;
using namespace testing;
using namespace std;

ClassicMock *g_mocker = nullptr;
AdapterManagerMock *g_amMock = nullptr;
ClassicAdapter *g_classicAdaper = nullptr;
ClassicBatteryObserverHf *g_batteryObserverHf = nullptr;
ClassicBatteryObserverAg *g_batteryObserverAg = nullptr;
HfpHfMock *g_HfpHfService = nullptr;
HfpAgMock *g_HfpAgService = nullptr;

ClassicAdapterObserver_MOCK *ClassicAdapterTest::classicAdapterObserver_ = new ClassicAdapterObserver_MOCK();
ClassicRemoteDeviceObserver_MOCK *ClassicAdapterTest::classicRemoteDeviceObserver_ =
    new ClassicRemoteDeviceObserver_MOCK();
IContextCallbackMock *g_callback = new IContextCallbackMock();

void HfpAgMock::RegisterObserver(HfpAgServiceObserver &observer)
{
    g_batteryObserverAg = (ClassicBatteryObserverAg *)&observer;
}

void HfpHfMock::RegisterObserver(HfpHfServiceObserver &observer)
{
    g_batteryObserverHf = (ClassicBatteryObserverHf *)&observer;
}

HfpHfMock::HfpHfMock()
{}
HfpHfMock::~HfpHfMock()
{}

void ClassicAdapterTest::SetUpTestCase()
{}

void ClassicAdapterTest::TearDownTestCase()
{
    delete classicAdapterObserver_;
    classicAdapterObserver_ = nullptr;
    delete classicRemoteDeviceObserver_;
    classicRemoteDeviceObserver_ = nullptr;
    delete g_callback;
    g_callback = nullptr;
}
void ClassicAdapterTest::SetUp()
{
    g_mocker = new ClassicMock();
    g_amMock = new AdapterManagerMock();
    registerBtmMock(g_mocker);
    registerGapMock(g_mocker);
    registerXmlMock(g_mocker);
    registerSdpMock(g_mocker);
    registerNewMockAdapterManager(g_amMock);
    g_classicAdaper = new ClassicAdapter();
    g_HfpHfService = new HfpHfMock();
    g_HfpAgService = new HfpAgMock();
    g_classicAdaper->GetContext()->RegisterCallback(*g_callback);
    EXPECT_CALL(*g_callback, OnEnable(ADAPTER_NAME_CLASSIC, true)).Times(1);
    EXPECT_CALL(*g_mocker, BTM_RegisterACLCallbacks()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterDiscoveryCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAuthenticationCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterSecurityCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_SetClassOfDevice()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, XmlGetValue())
        .Times(0x04)
        .WillOnce(Return("device"))
        .WillOnce(Return("device"))
        .WillOnce(Return("1234"))
        .WillOnce(Return("00000001213,12356464,"));
    EXPECT_CALL(*g_mocker, XmlSetValueString()).Times(0x02).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, XmlParse_Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
    g_classicAdaper->Enable();
    g_classicAdaper->RegisterRemoteDeviceObserver(*classicRemoteDeviceObserver_);
    g_classicAdaper->RegisterClassicAdapterObserver(*classicAdapterObserver_);
    RawAddress device("01:00:00:00:00:00");
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteNameChanged(device, _)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnPairStatusChanged(ADAPTER_BREDR, _, _)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteCodChanged(device, _)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteBatteryLevelChanged(device, 0)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteBatteryLevelChanged(device, 1)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteAliasChanged(device, testing::_)).Times(AtLeast(0));
    EXPECT_CALL(*classicRemoteDeviceObserver_, OnRemoteUuidChanged(device, testing::_)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnPairConfirmed(_, _, _, _)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnDiscoveryResult(device)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnScanModeChanged(_)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnDiscoveryStateChanged(testing::_)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnDeviceNameChanged(testing::_)).Times(AtLeast(0));
    EXPECT_CALL(*classicAdapterObserver_, OnPairRequested(ADAPTER_BREDR, device)).Times(AtLeast(0));
}
void ClassicAdapterTest::TearDown()
{
    EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_CLASSIC, true)).Times(AtLeast(1));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterDiscoveryCallback()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAuthenticationCallback())
        .Times(AtLeast(0))
        .WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterSecurityCallback()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_DeregisterACLCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    g_classicAdaper->DeregisterRemoteDeviceObserver(*classicRemoteDeviceObserver_);
    g_classicAdaper->DeregisterClassicAdapterObserver(*classicAdapterObserver_);
    g_classicAdaper->SetBtScanMode(SCAN_MODE_NONE, 30);
    GetDiscoveryCallback()->inquiryComplete(0x00, g_classicAdaper);
    g_classicAdaper->CancelBtDiscovery();
    g_classicAdaper->Disable();
    delete g_HfpHfService;
    g_HfpHfService = nullptr;
    delete g_HfpAgService;
    g_HfpAgService = nullptr;
    delete g_classicAdaper;
    g_classicAdaper = nullptr;
    delete g_amMock;
    g_amMock = nullptr;
    delete g_mocker;
    g_mocker = nullptr;
}

TEST_F(ClassicAdapterTest, Enable_case1)
{
    EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_CLASSIC, true)).Times(1);
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_DeregisterACLCallbacks()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_classicAdaper->Disable();
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_callback, OnEnable(ADAPTER_NAME_CLASSIC, false)).Times(1);
    EXPECT_CALL(*g_mocker, XmlParse_Load()).Times(2).WillRepeatedly(Return(false));
    g_classicAdaper->Enable();
}

TEST_F(ClassicAdapterTest, Enable_case2)
{
    EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_CLASSIC, true)).Times(1);
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_DeregisterACLCallbacks()).Times(1).WillOnce(Return(0));
    g_classicAdaper->Disable();
    EXPECT_CALL(*g_callback, OnEnable(ADAPTER_NAME_CLASSIC, false)).Times(1);
    EXPECT_CALL(*g_mocker, BTM_RegisterACLCallbacks()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterDiscoveryCallback()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAuthenticationCallback()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterSecurityCallback()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_SetClassOfDevice()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, BTM_DeregisterACLCallbacks()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, XmlGetValue())
        .Times(4)
        .WillOnce(Return("device"))
        .WillOnce(Return("device"))
        .WillOnce(Return("1234"))
        .WillOnce(Return("00000001213,12356464,"));
    EXPECT_CALL(*g_mocker, XmlSetValueString()).Times(2).WillRepeatedly(Return(false));
    g_classicAdaper->Enable();
    EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_CLASSIC, false)).Times(1);
    g_classicAdaper->Disable();
}

void PostEnable()
{
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock;
    StrictMock<ProfileServiceManagerMock> psmMock;
    registerNewMockProfileServiceManager(&psminstanceMock);
    EXPECT_CALL(psminstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock));
    std::vector<std::string> stringUuids;
    std::vector<std::string> uuids(1, "1234");
    EXPECT_CALL(psmMock, GetProfileServicesSupportedUuids(stringUuids)).WillOnce(DoAll(SetArgReferee<0>(uuids)));
    EXPECT_CALL(psmMock, GetProfileService(PROFILE_NAME_HFP_HF)).Times(1).WillOnce(Return(g_HfpHfService));
    EXPECT_CALL(psmMock, GetProfileService(PROFILE_NAME_HFP_AG)).Times(1).WillOnce(Return(g_HfpAgService));
    g_classicAdaper->PostEnable();
}

TEST_F(ClassicAdapterTest, SetLocalName_case)
{
    EXPECT_CALL(*g_mocker, XmlSetValueString()).Times(2).WillRepeatedly(Return(true));
    std::string name;
    name.insert(0, 249, 'a');
    EXPECT_TRUE(g_classicAdaper->SetLocalName(name));
    name = "Test";
    EXPECT_TRUE(g_classicAdaper->SetLocalName(name));
    EXPECT_TRUE(g_classicAdaper->SetLocalName(name));
    g_classicAdaper->GetLocalName();
    name = INVALID_NAME;
    EXPECT_FALSE(g_classicAdaper->SetLocalName(name));
}

TEST_F(ClassicAdapterTest, SetBondMode_case1)
{
    EXPECT_FALSE(g_classicAdaper->SetBondableMode(2));
    EXPECT_TRUE(g_classicAdaper->SetBondableMode(BONDABLE_MODE_OFF));
    g_classicAdaper->GetBondableMode();
}

TEST_F(ClassicAdapterTest, GetLocalAddress_case)
{
    std::string res = g_classicAdaper->GetLocalAddress();
    EXPECT_EQ(res, "00:00:00:00:00:00");
}

TEST_F(ClassicAdapterTest, GetDeviceName_case_classicRemoteDevice_GetRemoteName)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    EXPECT_EQ("", g_classicAdaper->GetDeviceName(device));
}

TEST_F(ClassicAdapterTest, GetDeviceUuids_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    g_classicAdaper->GetDeviceUuids(device);
}

TEST_F(ClassicAdapterTest, IsAclConnected_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    EXPECT_FALSE(g_classicAdaper->IsAclConnected(device));
}

TEST_F(ClassicAdapterTest, IsAclEncrypted_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    EXPECT_FALSE(g_classicAdaper->IsAclEncrypted(device));
}

TEST_F(ClassicAdapterTest, GetDeviceType_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    int res = g_classicAdaper->GetDeviceType(device);
    EXPECT_EQ(REMOTE_TYPE_BREDR, res);
}

TEST_F(ClassicAdapterTest, GetPairedStatus_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    EXPECT_EQ(PAIR_PAIRING, g_classicAdaper->GetPairState(device));
}

TEST_F(ClassicAdapterTest, SetGetAliasName_case)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    std::string name;
    name.insert(0, 249, '1');
    g_classicAdaper->SetAliasName(device, name);
    std::string alias;
    alias.insert(0, 248, '1');
    EXPECT_EQ(alias, g_classicAdaper->GetAliasName(device));
}

TEST_F(ClassicAdapterTest, GetPairedDevices)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    std::vector<RawAddress> pairedList = g_classicAdaper->GetPairedDevices();
    string resAddr = pairedList[0].GetAddress();
    EXPECT_EQ(resAddr, "01:00:00:00:00:00");
}

TEST_F(ClassicAdapterTest, RepeatStartPair)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    EXPECT_FALSE(g_classicAdaper->StartPair(device));
}

TEST_F(ClassicAdapterTest, StartPair_RemovePair)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    RawAddress device("01:00:00:00:00:00");
    EXPECT_FALSE(g_classicAdaper->RemovePair(device));
    bool res = g_classicAdaper->StartPair(device);
    EXPECT_TRUE(res);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    g_classicAdaper->RemovePair(device);
}

TEST_F(ClassicAdapterTest, StartPair_RemoveAllPairs)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    BtmAclConnectCompleteParam param{.status = 0x00,
        .connectionHandle = 0x0001,
        .addr = &addr,
        .classOfDevice = {0x01, 0x01, 0x01},
        .encyptionEnabled = true};
    void *context = g_classicAdaper;
    GetAclCallbacks()->connectionComplete(&param, context);
    RawAddress device("01:00:00:00:00:00");
    bool res = g_classicAdaper->StartPair(device);
    EXPECT_TRUE(res);
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    g_classicAdaper->RemoveAllPairs();
}

TEST_F(ClassicAdapterTest, SavePairedDeviceInfo)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    RawAddress device("01:00:00:00:00:00");
    bool res = g_classicAdaper->StartPair(device);
    EXPECT_TRUE(res);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    uint8_t name[248] = {'t', 'e', 's', 't'};
    uint8_t status = 0x00;
    std::string alias = "test";
    g_classicAdaper->SetAliasName(device, alias);
    GetDiscoveryCallback()->remoteName(status, &addr, name, context);
}

TEST_F(ClassicAdapterTest, GetPairState_PAIRNONE)
{
    RawAddress device("01:00:00:00:00:00");
    EXPECT_EQ(PAIR_NONE, g_classicAdaper->GetPairState(device));
}

TEST_F(ClassicAdapterTest, SetDevicePairingConfirmation_false1)
{
    RawAddress device("00:00:00:00:00:00");
    bool accept = false;
    bool res = g_classicAdaper->SetDevicePairingConfirmation(device, accept);
    EXPECT_FALSE(res);
}
TEST_F(ClassicAdapterTest, SetDevicePairingConfirmation_false2)
{
    RawAddress device("01:00:00:00:00:00");
    bool accept = false;
    bool res = g_classicAdaper->SetDevicePairingConfirmation(device, accept);
    EXPECT_FALSE(res);
}

TEST_F(ClassicAdapterTest, PairRequestReply)
{
    RawAddress device("01:00:00:00:00:00");
    bool accept = true;
    EXPECT_FALSE(g_classicAdaper->PairRequestReply(device, accept));
    g_classicAdaper->StartPair(device);
    EXPECT_CALL(*g_mocker, GAPIF_IOCapabilityRsp()).Times(1).WillOnce(Return(0));
    EXPECT_TRUE(g_classicAdaper->PairRequestReply(device, accept));
}

TEST_F(ClassicAdapterTest, UserConfirmReqCallback)
{
    RawAddress device("07:00:00:00:00:00");
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x07}, 0x00};
    g_classicAdaper->StartPair(device);
    uint32_t number = 0x0000;
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->userConfirmReq(&addr, number, context);
    EXPECT_FALSE(g_classicAdaper->IsBondedFromLocal(device));
}

TEST_F(ClassicAdapterTest, UserPasskeyReqCallback)
{
    RawAddress device("01:00:00:00:00:00");
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    g_classicAdaper->StartPair(device);
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->userPasskeyReq(&addr, context);
}

TEST_F(ClassicAdapterTest, UserPasskeyNotificationCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t number = 0x0000;
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->userPasskeyNotification(&addr, number, context);
}

TEST_F(ClassicAdapterTest, RemoteOobReqCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->remoteOobReq(&addr, context);
}

TEST_F(ClassicAdapterTest, PinCodeReqCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x07}, 0x00};
    void *context = g_classicAdaper;
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(1));
    GetAuthenticationCallback()->pinCodeReq(&addr, context);
}

TEST_F(ClassicAdapterTest, CancelPairing_false2)
{
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);

    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_PinCodeRsp()).Times(1).WillOnce(Return(BT_NO_ERROR));
    GetAuthenticationCallback()->pinCodeReq(&addr, context);
    g_classicAdaper->CancelPairing(device);
    bool res = g_classicAdaper->CancelPairing(device);
    EXPECT_FALSE(res);
}

TEST_F(ClassicAdapterTest, SetDevicePin1)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    std::string pinCode = "1234";
    EXPECT_CALL(*g_mocker, GAPIF_PinCodeRsp()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetDevicePin(device, pinCode);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetDevicePin2)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->StartPair(device);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    std::string pinCode = "";
    EXPECT_CALL(*g_mocker, GAPIF_PinCodeRsp()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetDevicePin(device, pinCode);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode1)
{
    int mode = SCAN_MODE_CONNECTABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode2)
{
    int mode = SCAN_MODE_NONE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    g_classicAdaper->SetBtScanMode(mode, duration);
}

TEST_F(ClassicAdapterTest, SetBtScanMode3)
{
    int mode = SCAN_MODE_GENERAL_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode4_GetBtScanMode)
{
    int mode = SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
    g_classicAdaper->GetBtScanMode();
}

TEST_F(ClassicAdapterTest, SetBtScanMode5)
{
    int mode = SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode6)
{
    int mode = SCAN_MODE_LIMITED_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode7)
{
    int mode = 0x06;
    int duration = 128000;
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_FALSE(res);
}

TEST_F(ClassicAdapterTest, SetLocalDeviceClass1)
{
    int deviceClass = 1;
    EXPECT_CALL(*g_mocker, GAPIF_SetClassOfDevice()).Times(1).WillOnce(Return(1));
    bool res = g_classicAdaper->SetLocalDeviceClass(deviceClass);
    EXPECT_FALSE(res);
}
TEST_F(ClassicAdapterTest, SetLocalDeviceClass2)
{
    int deviceClass = 0;
    EXPECT_CALL(*g_mocker, GAPIF_SetClassOfDevice()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetLocalDeviceClass(deviceClass);
    EXPECT_TRUE(res);
    EXPECT_EQ(0, g_classicAdaper->GetLocalDeviceClass());
}

TEST_F(ClassicAdapterTest, SetBtScanMode8)
{
    int mode = SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE;
    int duration = 0;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode9)
{
    int mode = SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_TRUE(res);
}

TEST_F(ClassicAdapterTest, SetBtScanMode10)
{
    int mode = 6;
    int duration = 128000;
    bool res = g_classicAdaper->SetBtScanMode(mode, duration);
    EXPECT_FALSE(res);
}

TEST_F(ClassicAdapterTest, StartBtDiscovery2)
{
    EXPECT_CALL(*g_mocker, GAPIF_Inquiry()).Times(1).WillOnce(Return(0));
    bool res = g_classicAdaper->StartBtDiscovery();
    EXPECT_TRUE(res);
    g_classicAdaper->StartBtDiscovery();
    g_classicAdaper->IsBtDiscovering();
}

TEST_F(ClassicAdapterTest, CancelBtDiscovery_true)
{
    EXPECT_CALL(*g_mocker, GAPIF_Inquiry()).Times(1).WillOnce(Return(0));
    g_classicAdaper->StartBtDiscovery();
    g_classicAdaper->CancelBtDiscovery();
    g_classicAdaper->IsBtDiscovering();
}

TEST_F(ClassicAdapterTest, GetBtDiscoveryEndMillis)
{
    EXPECT_EQ(0, g_classicAdaper->GetBtDiscoveryEndMillis());
}

TEST_F(ClassicAdapterTest, InquiryResultCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    int classOfDevice = 0x0002;
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->inquiryResult(&addr, classOfDevice, context);
    GetDiscoveryCallback()->inquiryResult(&addr, classOfDevice, context);
    RawAddress device("01:00:00:00:00:00");
    EXPECT_EQ(2, g_classicAdaper->GetDeviceClass(device));
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
}

TEST_F(ClassicAdapterTest, InquiryResultRssiCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0001;
    int8_t rssi = 0x00;
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->inquiryResultRssi(&addr, classOfDevice, rssi, context);
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
}

TEST_F(ClassicAdapterTest, ExtendedInquiryResultCallback2)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0001;
    int8_t rssi = 0x00;
    uint8_t eir[240] = {0x05, 0x04, 0x04, 0x00, 0x00, 0x00, 0x05, 0x05, 0x04, 0x00, 0x00, 0x00};
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->extendedInquiryResult(&addr, classOfDevice, rssi, eir, context);
    RawAddress device("01:00:00:00:00:00");
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    g_classicAdaper->StartPair(device);
    g_classicAdaper->GetDeviceUuids(device);
        const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
}

TEST_F(ClassicAdapterTest, ExtendedInquiryResultCallback3)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0001;
    int8_t rssi = 0x00;
    uint8_t eir[240] = {
        0x11, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->extendedInquiryResult(&addr, classOfDevice, rssi, eir, context);
}

TEST_F(ClassicAdapterTest, ExtendedInquiryResultCallback4)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0001;
    int8_t rssi = 0x00;
    uint8_t eir[240] = {
        0x11, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->extendedInquiryResult(&addr, classOfDevice, rssi, eir, context);
}

TEST_F(ClassicAdapterTest, ExtendedInquiryResultCallback5)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0001;
    int8_t rssi = 0x00;
    uint8_t eir[240] = {0x03, 0x0a, 0x02, 0x00, 0x03, 0xff, 0x00, 0x00, 0x03, 0x24, 0x02, 0x00};
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->extendedInquiryResult(&addr, classOfDevice, rssi, eir, context);
}

TEST_F(ClassicAdapterTest, RemoteNameCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t classOfDevice = 0x0002;
    void *context = g_classicAdaper;
    RawAddress remoteAddr("01:00:00:00:00:00");
    GetDiscoveryCallback()->inquiryResult(&addr, classOfDevice, context);
    uint8_t status = 0x01;
    uint8_t name[248] = {'t', 'e', 's', 't'};
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
    GetDiscoveryCallback()->inquiryComplete(0x00, g_classicAdaper);
    g_classicAdaper->CancelBtDiscovery();
    GetDiscoveryCallback()->remoteName(status, &addr, name, context);
    GetDiscoveryCallback()->inquiryComplete(0x00, g_classicAdaper);
}

TEST_F(ClassicAdapterTest, LinkKeyReqCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->linkKeyReq(&addr, context);
}

TEST_F(ClassicAdapterTest, LinkKeyReqCallback_else)
{
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    GetAuthenticationCallback()->linkKeyReq(&addr, context);
}

TEST_F(ClassicAdapterTest, IoCapabilityReqCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->IOCapabilityReq(&addr, context);
}

TEST_F(ClassicAdapterTest, IoCapabilityRspCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t ioCapability = 0x03;
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->IOCapabilityRsp(&addr, ioCapability, context);
}

TEST_F(ClassicAdapterTest, SimplePairCompleteCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    uint8_t status = 0x01;
    GetAuthenticationCallback()->simplePairComplete(&addr, status, context);
}

TEST_F(ClassicAdapterTest, AuthenticationCompleteCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t status = 0x01;
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->authenticationComplete(&addr, status, context);
}

TEST_F(ClassicAdapterTest, EncryptionChangeCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t status = 0x00;
    void *context = g_classicAdaper;
    GetAuthenticationCallback()->encryptionChangeCallback(&addr, status, context);
}

TEST_F(ClassicAdapterTest, AuthorizeIndCallback)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GAP_Service service = GAP;
    void *context = g_classicAdaper;
    GetSecurityCallback()->authorizeInd(&addr, service, context);
}

TEST_F(ClassicAdapterTest, ConnectionComplete1)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, g_classicAdaper);
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->pinCodeReq(&addr, g_classicAdaper);
    BtmAclConnectCompleteParam param{.status = 0x01,
        .connectionHandle = 0x0001,
        .addr = &addr,
        .classOfDevice = {0x01, 0x01, 0x01},
        .encyptionEnabled = true};
    GetAclCallbacks()->connectionComplete(&param, g_classicAdaper);
}

TEST_F(ClassicAdapterTest, ConnectionComplete2)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    BtmAclConnectCompleteParam param{.status = 0x00,
        .connectionHandle = 0x0001,
        .addr = &addr,
        .classOfDevice = {0x01, 0x01, 0x01},
        .encyptionEnabled = true};
    void *context = g_classicAdaper;
    GetAclCallbacks()->connectionComplete(&param, context);
}

TEST_F(ClassicAdapterTest, PinCodeReqCallback2)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->pinCodeReq(&addr, context);
}

TEST_F(ClassicAdapterTest, DisconnectionComplete)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    BtmAclConnectCompleteParam param{.status = 0x00,
        .connectionHandle = 0x0001,
        .addr = &addr,
        .classOfDevice = {0x01, 0x01, 0x01},
        .encyptionEnabled = true};
    void *context = g_classicAdaper;
    GetAclCallbacks()->connectionComplete(&param, context);

    const uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(BT_NO_ERROR));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, GAPIF_PairIsFromLocal()).Times(1).WillOnce(Return(BT_NO_ERROR));
    GetAuthenticationCallback()->pinCodeReq(&addr, context);
    uint8_t status = 0x00;
    uint16_t connectionHandle = 0x0001;
    uint8_t reason = 0x00;
    GetAclCallbacks()->disconnectionComplete(status, connectionHandle, reason, context);
}

TEST_F(ClassicAdapterTest, ServiceSearchAttributeCb)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    SdpService serviceArry[1];
    SdpService service;
    SdpProtocolDescriptor param;
    param.protocolUuid.type = BT_UUID_32;
    param.protocolUuid.uuid32 = 0x0002;
    param.parameterNumber = 1;
    SdpAdditionalProtocolDescriptor ptor;
    ptor.protocolDescriptorNumber = 1;
    ptor.parameter[0] = param;
    service.descriptorListNumber = 1;
    SdpAdditionalProtocolDescriptor addDesptor[1];
    addDesptor[0] = ptor;
    service.descriptorList = addDesptor;
    service.descriptorNumber = 1;
    SdpProtocolDescriptor param1;
    param1.protocolUuid.type = BT_UUID_16;
    param1.protocolUuid.uuid16 = 0x0001;
    param1.parameterNumber = 1;
    SdpProtocolDescriptor desptor[1];
    desptor[0] = param1;
    service.descriptor = desptor;
    service.classIdNumber = 3;
    BtUuid inUuid1;
    inUuid1.type = BT_UUID_16;
    BtUuid inUuid2;
    inUuid2.type = BT_UUID_32;
    BtUuid inUuid3;
    inUuid3.type = 0;
    BtUuid inUuid[3] = {inUuid1, inUuid2, inUuid3};
    service.classId = inUuid;
    serviceArry[0] = service;
    uint16_t serviceNum = 1;
    void *context = g_classicAdaper;
    uint8_t linkKey[GAP_LINKKEY_SIZE] = {0x00};
    uint8_t keyType = 0x00;
    g_classicAdaper->SetBondableMode(BONDABLE_MODE_ON);
    EXPECT_CALL(*g_mocker, GAPIF_GetRemoteName()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetAuthenticationCallback()->linkKeyNotification(&addr, linkKey, keyType, context);
    EXPECT_CALL(*g_mocker, SDP_ServiceSearchAttribute()).Times(1).WillOnce(Return(0));
    GetSdpServiceSearchAttCb()(&addr, serviceArry, serviceNum, context);
}

TEST_F(ClassicAdapterTest, SetScanModeResultCallback)
{
    int mode = SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE;
    int duration = 128000;
    EXPECT_CALL(*g_mocker, GAPIF_SetScanMode()).Times(1).WillOnce(Return(0));
    g_classicAdaper->SetBtScanMode(mode, duration);
    uint8_t status = 0x00;
    void *context = g_classicAdaper;
    GetSetScanModeResultCallback()(status, context);
}

TEST_F(ClassicAdapterTest, SimplePairCompleteCallback2)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = g_classicAdaper;
    uint8_t status = 0x01;
    RawAddress device("01:00:00:00:00:00");
    GetAuthenticationCallback()->simplePairComplete(&addr, status, context);
}

TEST_F(ClassicAdapterTest, RemoteNameCallback2)
{
    uint8_t status = 0x00;
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t name[248] = {'t', 'e', 's', 't'};
    void *context = g_classicAdaper;
    GetDiscoveryCallback()->remoteName(status, &addr, name, context);
}

TEST_F(ClassicAdapterTest, SetBondMode_case3)
{
    g_classicAdaper->SetBondableMode(2);
}

TEST_F(ClassicAdapterTest, ClassicRemoteDevice_GetTxPower_GetRssi)
{
    RawAddress remoteAddr("01:00:00:00:00:00");
    ClassicRemoteDevice *remoteDevice = new ClassicRemoteDevice(remoteAddr.GetAddress());
    remoteDevice->SetTxPower(0x0041);
    remoteDevice->SetRssi(6);
    delete remoteDevice;
    ClassicRemoteDevice dev;
    dev.SetAddress(remoteAddr.GetAddress());
}

TEST_F(ClassicAdapterTest, OnBatteryLevelChanged)
{
    PostEnable();
    RawAddress device("01:00:00:00:00:00");
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(1).WillOnce(Return(g_classicAdaper));
    g_batteryObserverHf->OnBatteryLevelChanged(device, 0);
}

TEST_F(ClassicAdapterTest, OnHfBatteryLevelChanged_GetDeviceBatteryLevel)
{
    PostEnable();
    RawAddress device("01:00:00:00:00:00");
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(1).WillOnce(Return(g_classicAdaper));
    g_batteryObserverAg->OnHfBatteryLevelChanged(device, 1);
    EXPECT_EQ(0, g_classicAdaper->GetDeviceBatteryLevel(device));
}

TEST_F(ClassicAdapterTest, ClassicDataStructure)
{
    ClassicDataStructure dataStru;
}