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

#include "hfp_hf_service.h"
#include "hfp_hf_profile_event_sender.h"
#include "interface_profile_hfp_hf.h"
#include "common_mock_all.h"
#include "hf_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpHfServiceObserverTest : public HfpHfServiceObserver {
public:
    void OnConnectionStateChanged(const RawAddress &device, int state)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnScoStateChanged(const RawAddress &device, int state)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnBatteryLevelChanged(const RawAddress &device, int batteryLevel)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnSignalStrengthChanged(const RawAddress &device, int signal)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnRegistrationStatusChanged(const RawAddress &device, int status)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnRoamingStatusChanged(const RawAddress &device, int status)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnOperatorSelectionChanged(const RawAddress &device, const std::string &name)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnSubscriberNumberChanged(const RawAddress &device, const std::string &number)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnVoiceRecognitionStatusChanged(const RawAddress &device, int status)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnInBandRingToneChanged(const RawAddress &device, int status)
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }
};

static HfpHfService hfService;
class HfpHfServiceTest : public testing::Test {
public:
    const RawAddress hfRawAddr = RawAddress("00:00:00:00:00:02");

    static void SetUpTestCase()
    {
        hfService.GetContext()->Initialize();
    }

    static void TearDownTestCase()
    {
        hfService.GetContext()->Uninitialize();
    }

    void SetUp()
    {
        // Mock AdapterManager::GetState()
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(AtLeast(0))
            .WillRepeatedly(Return(STATE_TURN_ON));

        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psmInstanceMock_);
        EXPECT_CALL(psmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(&hfService));

        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acInstanceMock_);
        EXPECT_CALL(acInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_HFP_HF_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));

        // Mock PowerManager & PowerManagerInstance
        registerNewMockPowerManager(&pmInstanceMock_);
        EXPECT_CALL(pmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_,_,_)).Times(AtLeast(0)).WillRepeatedly(Return());

        // Mock BT Manager
        registerBtmMocker(&btmMock_);
        initBtmMock();

        // Mock Gap
        registerGapMocker(&gapMock_);
        initGapMock();

        // Mock Rfcomm
        registerRfcommMocker(&rfcommMock_);
        initRfcommMock();

        // Mock Sdp
        registerSdpMocker(&sdpMock_);
        initSdpMock();

        hfService.RegisterObserver(hfObs);
    }

    void TearDown()
    {
        hfService.DeregisterObserver(hfObs);
    }

    void initBtmMock()
    {
        EXPECT_CALL(GetBtmMocker(), BTM_RegisterScoCallbacks()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_DeregisterScoCallbacks()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_WriteVoiceSetting()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_IsSecureConnection()).Times(AnyNumber()).WillRepeatedly(Return(false));
        EXPECT_CALL(GetBtmMocker(), BTM_CreateEscoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_DisconnectScoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_AcceptEscoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_CreateScoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_AcceptScoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_RejectScoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_GetLocalSupportedCodecs()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_IsControllerSupportEsco()).Times(AnyNumber()).WillRepeatedly(Return(false));
    }

    void initGapMock()
    {
        EXPECT_CALL(GetGapMocker(), GAPIF_RegisterServiceSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetGapMocker(), GAPIF_DeregisterServiceSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetGapMocker(), GAP_RequestSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

    void initRfcommMock()
    {
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_RegisterServer()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_DeregisterServer()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_FreeServerNum()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_AcceptConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_RejectConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_AssignServerNum()).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_ConnectChannel()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_DisconnectChannel()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Write()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_GetPeerAddress()).Times(AnyNumber()).WillRepeatedly(Return(hfBtAddr));
    }

    void initSdpMock()
    {
        EXPECT_CALL(GetSdpMocker(), SDP_CreateServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetSdpMocker(), SDP_RegisterServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_DeregisterServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_DestroyServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddServiceClassIdList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddProtocolDescriptorList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddBluetoothProfileDescriptorList())
            .Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddBrowseGroupList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddAttribute()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_ServiceSearchAttribute()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddServiceName()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

    void SetPremise_StartService()
    {
        hfService.Enable();
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetEnd_StopService()
    {
        hfService.Disable();
        usleep(50000);  // 50ms wait for service thread processing the message

        int ret = HfpHfProfile::DeregisterService();
        EXPECT_EQ(0, ret);

        std::vector<int> states;
        states.push_back((int)BTConnectState::CONNECTING);
        states.push_back((int)BTConnectState::CONNECTED);
        states.push_back((int)BTConnectState::DISCONNECTING);
        states.push_back((int)BTConnectState::DISCONNECTED);
        auto devList = hfService.GetDevicesByStates(states);
        HfpHfMessage eventRemove(HFP_HF_REMOVE_STATE_MACHINE_EVT);
        for (auto dev : devList) {
            eventRemove.dev_ = dev.GetAddress();
            hfService.ProcessEvent(eventRemove);
        }
    }

    void SetPremise_SendDisconnect(RawAddress rawAddr)
    {
        // send disconnect successful message
        if (hfService.GetDeviceState(rawAddr) == (int)BTConnectState::CONNECTED) {
            std::string address = rawAddr.GetAddress();
            HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
            curEvent.dev_ = address;
            hfService.PostEvent(curEvent);
            usleep(50000);  // 50ms wait for service thread processing the message
        }
    }

    void SetPremise_Connecting(RawAddress rawAddr)
    {
        // startup service
        SetPremise_StartService();

        // connect to make the device available
        int ret = hfService.Connect(rawAddr);
        EXPECT_EQ(0, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ConnectSuccessful(RawAddress rawAddr)
    {
        // make connecting state
        SetPremise_Connecting(rawAddr);

        // send connect successful message
        HfpHfMessage curEvent(HFP_HF_SLC_ESTABLISHED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        hfService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_Disconnecting(RawAddress rawAddr)
    {
        // make connected state
        SetPremise_ConnectSuccessful(rawAddr);

        // disconnect to make the device unavailable
        int ret = hfService.Disconnect(rawAddr);
        EXPECT_EQ(0, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_DisconnectSuccessful(RawAddress rawAddr)
    {
        // make Disconnecting state
        SetPremise_Disconnecting(rawAddr);

        // send disconnect successful message
        HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        hfService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoConnecting(RawAddress rawAddr)
    {
        SetPremise_ConnectSuccessful(rawAddr);

        bool ret = hfService.ConnectSco(rawAddr);
        EXPECT_EQ(true, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoConnectSuccessful(RawAddress rawAddr)
    {
        SetPremise_ScoConnecting(rawAddr);

        HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        hfService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoDisconnecting(RawAddress rawAddr)
    {
        SetPremise_ScoConnectSuccessful(rawAddr);

        bool ret = hfService.DisconnectSco(rawAddr);
        EXPECT_EQ(true, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    // Need state >= connecting
    int MakeDataAvailableEvent(std::string command)
    {
        RfcommIncomingInfo eventData{hfBtAddr, 1};
        std::string fullCommand("\r\n" + command + "\r\n");
        std::size_t CmdLength = fullCommand.length();
        Packet *packet = PacketMalloc(0, 0, CmdLength);
        Buffer *buf = PacketContinuousPayload(packet);
        void *data = BufferPtr(buf);
        (void)memcpy_s(data, CmdLength, fullCommand.c_str(), CmdLength);
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _))
            .WillOnce(DoAll(SetArgPointee<1>(packet), Return(0)))
            .WillRepeatedly(Return(1));

        GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_INCOMING, &eventData, nullptr);
        GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_REV_DATA, nullptr, nullptr);
        usleep(50000);  // 50ms wait for service thread processing the message
        return 0;
    }

    int MakeSdpSearchCb(bool isSuccessCb)
    {
        if (isSuccessCb) {
            SdpService array;
            (void)memset_s(&array, sizeof(SdpService), 0x00, sizeof(SdpService));

            array.descriptorNumber = 2;
            array.descriptor = new SdpProtocolDescriptor[array.descriptorNumber];
            array.descriptor[1].protocolUuid.uuid16 = UUID_PROTOCOL_RFCOMM;
            array.descriptor[1].parameter[0].type = SDP_TYPE_UINT_8;
            array.descriptor[1].parameter[0].value = 1;

            array.profileDescriptorNumber = 2;
            array.profileDescriptor = new SdpProfileDescriptor[array.profileDescriptorNumber];
            array.profileDescriptor[1].profileUuid.uuid16 = HFP_HF_UUID_SERVCLASS_HFP_AG;
            array.profileDescriptor[1].versionNumber = HFP_HF_HFP_VERSION_1_7;

            array.attributeNumber = 2;
            array.attribute = new SdpAttribute[array.attributeNumber];
            array.attribute[0].attributeId = HFP_HF_SDP_ATTRIBUTE_SUPPORTED_FEATURES;
            array.attribute[0].attributeValueLength = 2;
            uint16_t feature = 0xFFFF;
            array.attribute[0].attributeValue = &feature;
            array.attribute[1].attributeId = HFP_HF_SDP_ATTRIBUTE_DATA_STORES_OR_NETWORK;
            array.attribute[1].attributeValueLength = 2;
            uint16_t network = HFP_HF_NETWORK_VALUE_HAVE;
            array.attribute[1].attributeValue = &network;

            GetSdpMocker().sdpCb(&(GetSdpMocker().addr), &array, 1, nullptr);

            delete[] array.descriptor;
            delete[] array.profileDescriptor;
            delete[] array.attribute;
        } else {
            GetSdpMocker().sdpCb(&(GetSdpMocker().addr), nullptr, 0, nullptr);
        }
        usleep(50000);  // 50ms wait for service thread processing the message
        return 0;
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acInstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    StrictMock<PowerManagerInstanceMock> pmInstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;
    StrictMock<HfBtmMock> btmMock_;
    StrictMock<HfGapMock> gapMock_;
    StrictMock<HfRfcommMock> rfcommMock_;
    StrictMock<HfSdpMock> sdpMock_;
    HfpHfServiceObserverTest hfObs{};
};

TEST_F(HfpHfServiceTest, StartUp_ShutDown)
{
    SetPremise_StartService();
    SetEnd_StopService();
}

// Test:if service is started.
TEST_F(HfpHfServiceTest, StartUp)
{
    // startup service
    SetPremise_StartService();

    hfService.Enable();
    usleep(50000);

    SetEnd_StopService();
}

// Connect failed if there are 6 devices in connected.
TEST_F(HfpHfServiceTest, Connect)
{
    SetPremise_StartService();

    RawAddress rawAddr1("00:00:00:00:00:01");
    RawAddress rawAddr2("00:00:00:00:00:02");
    RawAddress rawAddr3("00:00:00:00:00:03");
    RawAddress rawAddr4("00:00:00:00:00:04");
    RawAddress rawAddr5("00:00:00:00:00:05");
    RawAddress rawAddr6("00:00:00:00:00:06");
    std::vector<RawAddress> rawAddrList;
    rawAddrList.push_back(rawAddr1);
    rawAddrList.push_back(rawAddr2);
    rawAddrList.push_back(rawAddr3);
    rawAddrList.push_back(rawAddr4);
    rawAddrList.push_back(rawAddr5);
    rawAddrList.push_back(rawAddr6);

    for (size_t i = 0; i < rawAddrList.size(); i++) {
        SetPremise_ConnectSuccessful(rawAddrList[i]);
    }

    RawAddress rawAddr7("00:00:00:00:00:07");
    int ret = hfService.Connect(rawAddr7);
    EXPECT_EQ(HFP_HF_FAILURE, ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return FAILURE.
TEST_F(HfpHfServiceTest, Disconnect)
{
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    int ret = hfService.Disconnect(rawAddr1);
    EXPECT_EQ(HFP_HF_FAILURE, ret);
    usleep(50000);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, GetMaxConnectNum)
{
    int num = hfService.GetMaxConnectNum();
    EXPECT_EQ(6, num);
}

// If there are two devices connected, device is 2.
TEST_F(HfpHfServiceTest, GetConnectDevices_1)
{
    // set premise:connect devices;
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);
    RawAddress rawAddr1("00:00:00:00:00:02");
    SetPremise_ConnectSuccessful(rawAddr1);

    std::list<RawAddress> deviceList;
    deviceList = hfService.GetConnectDevices();
    EXPECT_EQ(2UL, deviceList.size());
    SetEnd_StopService();
}

// If there is no device connected, device is 0.
TEST_F(HfpHfServiceTest, GetConnectDevices_2)
{
    std::list<RawAddress> deviceList;
    deviceList = hfService.GetConnectDevices();
    EXPECT_EQ(0UL, deviceList.size());
}

// If there is no device connected,state is 0.
TEST_F(HfpHfServiceTest, GetConnectState_1)
{
    int state = hfService.GetConnectState();
    EXPECT_EQ(0, state);
}

// If there is device connected,state is PROFILE_STATE_CONNECTED.
TEST_F(HfpHfServiceTest, GetConnectState_2)
{
    // set premise:connect devices;
    SetPremise_ConnectSuccessful(hfRawAddr);

    int state = hfService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_CONNECTED, state);
    SetEnd_StopService();
}

// PROFILE_STATE_CONNECTING.
TEST_F(HfpHfServiceTest, GetConnectState_3)
{
    // set premise:connect devices;
    SetPremise_Connecting(hfRawAddr);

    int state = hfService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_CONNECTING, state);
    SetEnd_StopService();
}

// PROFILE_STATE_DISCONNECTING.
TEST_F(HfpHfServiceTest, GetConnectState_4)
{
    // set premise:connect devices and disconnect it;
    SetPremise_Disconnecting(hfRawAddr);

    int state = hfService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_DISCONNECTING, state);
    SetEnd_StopService();
}

// If device is connected, and transmit to disconnected.
TEST_F(HfpHfServiceTest, GetConnectState_5)
{
    // set premise:connect devices and disconnect it(disconnected);
    SetPremise_DisconnectSuccessful(hfRawAddr);
    SetPremise_DisconnectSuccessful(hfRawAddr);

    int state = hfService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_DISCONNECTED, state);

    SetEnd_StopService();
}

// All states test
TEST_F(HfpHfServiceTest, GetConnectState_6)
{
    // set premise
    // connected
    RawAddress rawAddr1("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr1);
    // connecting
    RawAddress rawAddr2("00:00:00:00:00:02");
    SetPremise_Connecting(rawAddr2);
    // disconnecting
    RawAddress rawAddr3("00:00:00:00:00:03");
    SetPremise_Disconnecting(rawAddr3);
    // disconnected
    RawAddress rawAddr4("00:00:00:00:00:04");
    SetPremise_DisconnectSuccessful(rawAddr4);

    int state = hfService.GetConnectState();
    EXPECT_EQ(
        PROFILE_STATE_CONNECTED | PROFILE_STATE_CONNECTING | PROFILE_STATE_DISCONNECTING | PROFILE_STATE_DISCONNECTED,
        state);
    SetEnd_StopService();
}

// If device is connected, return true.
TEST_F(HfpHfServiceTest, ConnectSco_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.ConnectSco(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// IF device is not connected, return false.
TEST_F(HfpHfServiceTest, ConnectSco_2)
{
    // set premise
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    bool ret = hfService.ConnectSco(rawAddr1);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If device is not connected, return false.
TEST_F(HfpHfServiceTest, DisconnectSco_1)
{
    // set premise
    SetPremise_StartService();

    bool ret = hfService.DisconnectSco(hfRawAddr);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If device is in sco connected states, return true.
TEST_F(HfpHfServiceTest, DisconnectSco_2)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    bool ret = hfService.DisconnectSco(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// Get connected state device.
TEST_F(HfpHfServiceTest, GetDevicesByStates_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    std::vector<int> states;
    states.push_back((int)BTConnectState::CONNECTED);
    std::vector<RawAddress> deviceList = hfService.GetDevicesByStates(states);
    EXPECT_EQ(1UL, deviceList.size());
    EXPECT_EQ(hfRawAddr.GetAddress(), deviceList[0].GetAddress());

    SetEnd_StopService();
}

// Input strong test
TEST_F(HfpHfServiceTest, GetDevicesByStates_2)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);
    std::vector<int> states;
    states.push_back(10);

    std::vector<RawAddress> deviceList = hfService.GetDevicesByStates(states);
    EXPECT_EQ(0UL, deviceList.size());

    SetEnd_StopService();
}

// All states test
TEST_F(HfpHfServiceTest, GetDevicesByStates_3)
{
    // set premise
    // make a connected device
    RawAddress rawAddr1("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr1);
    // make a connecting device
    RawAddress rawAddr2("00:00:00:00:00:02");
    SetPremise_Connecting(rawAddr2);
    // make a disconnecting device
    RawAddress rawAddr3("00:00:00:00:00:03");
    SetPremise_Disconnecting(rawAddr3);
    // make a disconnected device
    RawAddress rawAddr4("00:00:00:00:00:04");
    SetPremise_DisconnectSuccessful(rawAddr4);

    std::vector<int> states;
    states.push_back((int)BTConnectState::CONNECTING);
    states.push_back((int)BTConnectState::CONNECTED);
    states.push_back((int)BTConnectState::DISCONNECTING);
    states.push_back((int)BTConnectState::DISCONNECTED);

    std::vector<RawAddress> deviceList = hfService.GetDevicesByStates(states);
    EXPECT_EQ(4UL, deviceList.size());

    SetEnd_StopService();
}

// get DISCONNECTED state
TEST_F(HfpHfServiceTest, GetDeviceState_1)
{
    int state = hfService.GetDeviceState(hfRawAddr);
    EXPECT_EQ((int)BTConnectState::DISCONNECTED, state);
}

// get DISCONNECTING state
TEST_F(HfpHfServiceTest, GetDeviceState_2)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    int state = hfService.GetDeviceState(hfRawAddr);
    EXPECT_EQ((int)BTConnectState::DISCONNECTING, state);

    SetEnd_StopService();
}

// get CONNECTING state
TEST_F(HfpHfServiceTest, GetDeviceState_3)
{
    // set premise:connect devices(but not connected)
    SetPremise_StartService();

    int ret = hfService.Connect(hfRawAddr);
    EXPECT_EQ(HFP_HF_SUCCESS, ret);
    usleep(50000);

    int state = hfService.GetDeviceState(hfRawAddr);
    EXPECT_EQ((int)BTConnectState::CONNECTING, state);

    SetEnd_StopService();
}

// get CONNECTED state
TEST_F(HfpHfServiceTest, GetDeviceState_4)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    int state = hfService.GetDeviceState(hfRawAddr);
    EXPECT_EQ((int)BTConnectState::CONNECTED, state);

    SetEnd_StopService();
}

// HFP_HF_AUDIO_STATE_DISCONNECTED
TEST_F(HfpHfServiceTest, GetScoState_1)
{
    int state = hfService.GetScoState(hfRawAddr);
    EXPECT_EQ(HFP_HF_AUDIO_STATE_DISCONNECTED, state);
}

// HFP_HF_AUDIO_STATE_CONNECTED
TEST_F(HfpHfServiceTest, GetScoState_2)
{
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    int state = hfService.GetScoState(hfRawAddr);
    EXPECT_EQ(HFP_HF_AUDIO_STATE_CONNECTED, state);

    SetEnd_StopService();
}

// HFP_HF_AUDIO_STATE_CONNECTING
TEST_F(HfpHfServiceTest, GetScoState_3)
{
    SetPremise_ScoConnecting(hfRawAddr);

    int state = hfService.GetScoState(hfRawAddr);
    EXPECT_EQ(HFP_HF_AUDIO_STATE_CONNECTING, state);

    SetEnd_StopService();
}

// HFP_HF_AUDIO_STATE_DISCONNECTING
TEST_F(HfpHfServiceTest, GetScoState_4)
{
    SetPremise_ScoDisconnecting(hfRawAddr);

    int state = hfService.GetScoState(hfRawAddr);
    EXPECT_EQ(HFP_HF_AUDIO_STATE_DISCONNECTING, state);

    SetEnd_StopService();
}

// If device is connected, return true.
TEST_F(HfpHfServiceTest, SendDTMFTone_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.SendDTMFTone(hfRawAddr, '3');
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If device is not connected, return false.
TEST_F(HfpHfServiceTest, SendDTMFTone_2)
{
    bool ret = hfService.SendDTMFTone(hfRawAddr, '3');
    EXPECT_FALSE(ret);
}

// Make a call, get call list number is 1.
TEST_F(HfpHfServiceTest, GetCurrentCallList_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CURRENT_CALLS
    std::string cmd = "+CLCC: 1,0,0,0,0,\"+10086\",145";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE
    HfpHfProfileEventSender::GetInstance().CurrentCallsReplyDone(hfRawAddr.GetAddress());
    usleep(50000);  // 50ms wait for service thread processing the message

    std::vector<HandsFreeUnitCalls> callList;
    callList = hfService.GetCurrentCallList(hfRawAddr);
    EXPECT_EQ(1UL, callList.size());

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, AcceptIncomingCall_2)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.AcceptIncomingCall(hfRawAddr, 0);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, AcceptIncomingCall_3)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.AcceptIncomingCall(hfRawAddr, 1);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// Input parameter is out of range.
TEST_F(HfpHfServiceTest, AcceptIncomingCall_4)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    RawAddress rawAddr = RawAddress("00:00:00:00:00:03");
    bool ret = hfService.AcceptIncomingCall(rawAddr, 1);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, AcceptIncomingCall_5)
{
    // set premise
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    bool ret = hfService.AcceptIncomingCall(rawAddr1, 0);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, HoldActiveCall_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.HoldActiveCall(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, HoldActiveCall_2)
{
    // set premise
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    bool ret = hfService.HoldActiveCall(rawAddr1);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, RejectIncomingCall_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.RejectIncomingCall(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, RejectIncomingCall_2)
{
    // set premise
    RawAddress rawAddr("00:00:00:00:00:01");
    SetPremise_ConnectSuccessful(rawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    bool ret = hfService.RejectIncomingCall(rawAddr1);
    EXPECT_FALSE(ret);

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, FinishActiveCall_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);
    HandsFreeUnitCalls call(hfRawAddr.GetAddress(), 1, 0, "10086", 0, 0, 0);

    bool ret = hfService.FinishActiveCall(hfRawAddr, call);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, FinishActiveCall_2)
{
    // set premise
    HandsFreeUnitCalls call(hfRawAddr.GetAddress(), 1, 0, "10086", 0, 0, 0);

    bool ret = hfService.FinishActiveCall(hfRawAddr, call);
    EXPECT_FALSE(ret);
}

// If input device is connected, return a call.
TEST_F(HfpHfServiceTest, StartDial_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    auto call = hfService.StartDial(hfRawAddr, "10086");
    EXPECT_EQ("10086", call.value().GetNumber());
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return null.
TEST_F(HfpHfServiceTest, StartDial_2)
{
    auto call = hfService.StartDial(hfRawAddr, "10086");
    EXPECT_EQ(std::nullopt, call);
}

// If input device is connected
TEST_F(HfpHfServiceTest, SetHfVolume_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    hfService.SetHfVolume(10, 0);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected
TEST_F(HfpHfServiceTest, SetHfVolume_2)
{
    // set premise
    SetPremise_DisconnectSuccessful(hfRawAddr);

    hfService.SetHfVolume(10, 0);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, OpenVoiceRecognition_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.OpenVoiceRecognition(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, OpenVoiceRecognition_2)
{
    bool ret = hfService.OpenVoiceRecognition(hfRawAddr);
    EXPECT_FALSE(ret);
}

// If input device is connected, return true.
TEST_F(HfpHfServiceTest, CloseVoiceRecognition_1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    bool ret = hfService.CloseVoiceRecognition(hfRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);

    SetEnd_StopService();
}

// If input device is not connected, return false.
TEST_F(HfpHfServiceTest, CloseVoiceRecognition_2)
{
    bool ret = hfService.CloseVoiceRecognition(hfRawAddr);
    EXPECT_FALSE(ret);
}

// just function call
TEST_F(HfpHfServiceTest, BatteryLevelChanged)
{
    hfService.BatteryLevelChanged(10);
}

// just function call
TEST_F(HfpHfServiceTest, EnhancedDriverSafety)
{
    hfService.EnhancedDriverSafety(0);
}

TEST_F(HfpHfServiceTest, ProcessEvent)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage eventStartup(HFP_HF_SERVICE_STARTUP_EVT);
    hfService.ProcessEvent(eventStartup);
    usleep(50000);

    HfpHfMessage eventShutdown(HFP_HF_SERVICE_SHUTDOWN_EVT);
    hfService.ProcessEvent(eventShutdown);
    usleep(50000);

    HfpHfMessage eventRemove(HFP_HF_REMOVE_STATE_MACHINE_EVT);
    hfService.ProcessEvent(eventRemove);
    usleep(50000);

    HfpHfMessage event_other(40);
    hfService.ProcessEvent(event_other);
    usleep(50000);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, notify)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    hfService.NotifyStateChanged(hfRawAddr, HFP_HF_STATE_CONNECTED);
    usleep(50000);

    hfService.NotifyScoStateChanged(hfRawAddr, HFP_HF_AUDIO_STATE_DISCONNECTED);
    usleep(50000);

    HandsFreeUnitCalls call(hfRawAddr.GetAddress(), 1, 0, "10086", 0, 0, 0);
    ;
    hfService.NotifyCallChanged(hfRawAddr, call);
    usleep(50000);

    hfService.NotifyBatteryLevelChanged(hfRawAddr, 100);
    usleep(50000);

    hfService.NotifySignalStrengthChanged(hfRawAddr, 20);
    usleep(50000);

    hfService.NotifyRegistrationStatusChanged(hfRawAddr, 1);
    usleep(50000);

    hfService.NotifyRoamingStatusChanged(hfRawAddr, 0);
    usleep(50000);

    hfService.NotifyOperatorSelectionChanged(hfRawAddr, "mobile");
    usleep(50000);

    hfService.NotifySubscriberNumberChanged(hfRawAddr, "10086");
    usleep(50000);

    hfService.NotifyVoiceRecognitionStatusChanged(hfRawAddr, 0);
    usleep(50000);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev1)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_NETWORK_STATE
    std::string cmd1 = "+CIEV: 0,1";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev2)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CALL_STATE
    std::string cmd1 = "+CIEV: 1,0";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev3)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CALL_SETUP_STATE
    std::string cmd1 = "+CIEV: 2,0";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev4)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CALL_HELD_STATE
    std::string cmd1 = "+CIEV: 3,0";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev5)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_NETWORK_SIGNAL
    std::string cmd1 = "+CIEV: 4,4";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev6)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_NETWORK_ROAM
    std::string cmd1 = "+CIEV: 5,1";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessCiev7)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // Init remote AG Indicators
    std::string cmd = "+CIND: ";
    cmd.append("(\"service\",(0-1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2)),");
    cmd.append("(\"signal\",(0-5)),(\"roam\",(0,1)),(\"battchg\",(0-5)))");
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_BATTERY_LEVEL
    std::string cmd1 = "+CIEV: 6,3";
    ret = MakeDataAvailableEvent(cmd1);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_CurrentOperatorReply)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CURRENT_OPERATOR
    std::string cmd = "+COPS: 3,0,\"test1\"";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_HoldResultReply)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_HOLD_RESULT
    std::string cmd = "+BTRH: 1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_Callinglineidentification)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION
    std::string cmd = "+CLIP: \"+10086\",145";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_CallWaitingNotify)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CALL_WAITING
    std::string cmd = "+CCWA: \"+10086\",145";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_CurrentCallsReply)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CURRENT_CALLS
    std::string cmd = "+CLCC: 1,0,0,0,0,\"+10086\",145";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessVgm)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_SET_VOLUME
    std::string cmd = "+VGM: 5";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessVgs)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_SET_VOLUME
    std::string cmd = "+VGS: 10";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessBusy)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_AT_CMD_RESULT
    std::string cmd = "BUSY";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessDelayed)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_AT_CMD_RESULT
    std::string cmd = "DELAYED";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessNoCarrier)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_AT_CMD_RESULT
    std::string cmd = "NO CARRIER";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessNoAnswer)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_AT_CMD_RESULT
    std::string cmd = "NO ANSWER";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ProcessBlocklisted)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_AT_CMD_RESULT
    std::string cmd = "BLOCKLISTED";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_SubscriberNumberReply)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_SUBSCRIBER_NUMBER
    std::string cmd = "+CNUM: ,\"10086\",129,,1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_SetInBandRing)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_INBAND_RING
    std::string cmd = "+BSIR: 1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_RingIndicationNotify)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_RING_ALERT
    std::string cmd = "RING";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_UnknownEventNotify)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_UNKNOWN
    std::string str = "UNKNOWN";
    HfpHfProfileEventSender::GetInstance().UnknownEventNotify(hfRawAddr.GetAddress(), str);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnected_ConnectRequest)
{
    // set premise
    SetPremise_StartService();

    // HFP_HF_CONNECT_REQUEST_EVT
    RfcommIncomingInfo eventData{hfBtAddr, 1};
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_INCOMING, &eventData, nullptr);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_UpdateConnectState)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    int state = 1;
    HfpHfProfileEventSender::GetInstance().UpdateConnectState(hfRawAddr.GetAddress(), state);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ScoConnectRequest)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    int what = HFP_HF_AUDIO_CONNECT_REQUEST_EVT;
    uint8_t linkType = 1;
    HfpHfProfileEventSender::GetInstance().ScoConnectRequest(hfRawAddr.GetAddress(), what, linkType);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_UpdateScoConnectState)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    int state = 4;
    HfpHfProfileEventSender::GetInstance().UpdateScoConnectState(hfRawAddr.GetAddress(), state);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_SdpDiscoverySuccess)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS
    int ret = MakeSdpSearchCb(true);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_SdpDiscoveryFail)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_SDP_DISCOVERY_RESULT_FAIL
    int ret = MakeSdpSearchCb(false);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_CurrentCallsReplyDone)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE
    HfpHfProfileEventSender::GetInstance().CurrentCallsReplyDone(hfRawAddr.GetAddress());
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_OpenVoiceRecognitionResult)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_OPEN_VR_RESULT
    int result = 1;
    HfpHfProfileEventSender::GetInstance().OpenVoiceRecognitionResult(hfRawAddr.GetAddress(), result);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_CloseVoiceRecognitionResult)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_CLOSE_VR_RESULT
    int result = 1;
    HfpHfProfileEventSender::GetInstance().CloseVoiceRecognitionResult(hfRawAddr.GetAddress(), result);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_VoiceRecognitionChanged)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED
    std::string cmd = "+BVRA: 1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnected_ScoConnectRequest)
{
    // set premise
    SetPremise_DisconnectSuccessful(hfRawAddr);

    int what = HFP_HF_AUDIO_CONNECT_REQUEST_EVT;
    uint8_t linkType = 1;
    HfpHfProfileEventSender::GetInstance().ScoConnectRequest(hfRawAddr.GetAddress(), what, linkType);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_Connect)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_DataAvailable)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DATA_AVAILABLE_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_Connected)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_CONNECTED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_SUCCESS, nullptr, nullptr);
    HfpHfMessage curEvent(HFP_HF_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_Disconnected)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_DISCONNECTED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_DISCONNECTED, nullptr, nullptr);
    HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_ConnectSco)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_ScoRequest)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECT_REQUEST_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_ScoConnected)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_ConnectFail)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_CONNECT_FAILED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_FAIL, nullptr, nullptr);
    HfpHfMessage curEvent(HFP_HF_CONNECT_FAILED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_BatteryLevelChanged)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_BATTERY_LEVEL_CHANGED_EVT
    hfService.BatteryLevelChanged(3);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_EnhancedDriverSafety)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT
    hfService.EnhancedDriverSafety(1);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_RingIndicationNotify)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    // HFP_HF_INTERACTIVE_EVT - HFP_HF_TYPE_RING_ALERT
    std::string cmd = "RING";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_Connecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECTION_TIMEOUT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnecting_Connect)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnecting_DisconnectFail)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    // HFP_HF_DISCONNECT_FAILED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_DISCONNECT_FAIL, nullptr, nullptr);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECTION_TIMEOUT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnecting_Disconnect)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfDisconnecting_ScoConnectRequest)
{
    // set premise
    SetPremise_Disconnecting(hfRawAddr);

    int what = HFP_HF_AUDIO_CONNECT_REQUEST_EVT;
    uint8_t linkType = 1;
    HfpHfProfileEventSender::GetInstance().ScoConnectRequest(hfRawAddr.GetAddress(), what, linkType);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_Connect)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ConnectSco)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_DisconnectSco)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ScoConnecting)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTING_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_ScoConnected)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_Disconnected)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_BatteryChanged)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_BATTERY_LEVEL_CHANGED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfConnected_DriverSafetyChanged)
{
    // set premise
    SetPremise_ConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_Connect)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_Disconnect)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_Disconnected)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_ScoConnectReauest)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECT_REQUEST_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_ScoConnected)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_ScoConnectFail)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECT_FAILED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECTION_TIMEOUT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnecting_ScoDisconnected)
{
    // set premise
    SetPremise_ScoConnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_Connect)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_ConnectSco)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_DisconnectSco)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_Disconnect)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_ScoDisconnected)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_ScoDisconnectFail)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_DISCONNECT_FAILED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_CONNECTION_TIMEOUT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoDisconnecting_ScoConnected)
{
    // set premise
    SetPremise_ScoDisconnecting(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_ScoConnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_CONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_ScoDisconnecting)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_DISCONNECTING_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_DisconnectSco)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_AUDIO_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_Disconnect)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECT_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_ScoDisconnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_AUDIO_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST_F(HfpHfServiceTest, HfpHfScoConnected_Disconnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(hfRawAddr);

    HfpHfMessage curEvent(HFP_HF_DISCONNECTED_EVT);
    curEvent.dev_ = hfRawAddr.GetAddress();
    hfService.PostEvent(curEvent);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetEnd_StopService();
}

TEST(HfpHfServiceObserverTest, VirtualFunctionTest)
{
    HfpHfServiceObserverTest testCallback;
    HfpHfServiceObserver virtCallback = (HfpHfServiceObserverTest)testCallback;
    RawAddress hfRawAddr = RawAddress("00:00:00:00:00:02");
    HandsFreeUnitCalls call;

    virtCallback.OnBatteryLevelChanged(hfRawAddr, 3);
    virtCallback.OnCallChanged(hfRawAddr, call);
    virtCallback.OnConnectionStateChanged(hfRawAddr, 1);
    virtCallback.OnInBandRingToneChanged(hfRawAddr, 1);
    virtCallback.OnOperatorSelectionChanged(hfRawAddr, "test_user");
    virtCallback.OnRegistrationStatusChanged(hfRawAddr, 1);
    virtCallback.OnRoamingStatusChanged(hfRawAddr, 1);
    virtCallback.OnScoStateChanged(hfRawAddr, 1);
    virtCallback.OnSignalStrengthChanged(hfRawAddr, 1);
    virtCallback.OnSubscriberNumberChanged(hfRawAddr, "10086");
    virtCallback.OnVoiceRecognitionStatusChanged(hfRawAddr, 1);
}