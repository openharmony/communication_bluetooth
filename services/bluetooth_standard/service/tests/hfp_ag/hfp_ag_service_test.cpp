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

#include "hfp_ag_service.h"
#include "hfp_ag_profile_event_sender.h"
#include "interface_profile_hfp_ag.h"
#include "common_mock_all.h"
#include "ag_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpAgServiceObserverTest : public HfpAgServiceObserver {
public:
    void OnConnectionStateChanged(const RawAddress &device, int state) override
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnScoStateChanged(const RawAddress &device, int state) override
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnActiveDeviceChanged(const RawAddress &device) override
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnHfEnhancedDriverSafetyChanged(const RawAddress &device, int indValue) override
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }

    void OnHfBatteryLevelChanged(const RawAddress &device, int indValue) override
    {
        LOG_INFO("%s excute", __PRETTY_FUNCTION__);
        return;
    }
};

static HfpAgService agService;
class HfpAgServiceTest : public testing::Test {
public:
    // HfpAgService agService{};
    const RawAddress agRawAddr = RawAddress("00:00:00:00:00:01");

    static void SetUpTestCase()
    {
        agService.GetContext()->Initialize();
    }

    static void TearDownTestCase()
    {
        agService.GetContext()->Uninitialize();
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
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(&agService));

        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acInstanceMock_);
        EXPECT_CALL(acInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_HFP_AG_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
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

        agService.RegisterObserver(agObs);
    }

    void TearDown()
    {
        agService.DeregisterObserver(agObs);
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
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_GetPeerAddress()).Times(AnyNumber()).WillRepeatedly(Return(agBtAddr));
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
        agService.Enable();
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_StopService()
    {
        agService.Disable();
        usleep(50000);  // 50ms wait for service thread processing the message

        int ret = HfpAgProfile::DeregisterService();
        EXPECT_EQ(0, ret);

        std::vector<int> states;
        states.push_back((int)BTConnectState::CONNECTING);
        states.push_back((int)BTConnectState::CONNECTED);
        states.push_back((int)BTConnectState::DISCONNECTING);
        states.push_back((int)BTConnectState::DISCONNECTED);
        auto devList = agService.GetDevicesByStates(states);
        HfpAgMessage eventRemove(HFP_AG_REMOVE_STATE_MACHINE_EVT);
        for (auto dev : devList) {
            eventRemove.dev_ = dev.GetAddress();
            agService.ProcessEvent(eventRemove);
        }

        agService.SetActiveDevice(RawAddress("00:00:00:00:00:00"));
    }

    void SetPremise_Connecting(RawAddress rawAddr)
    {
        // startup service
        SetPremise_StartService();

        // connect to make the device available
        int ret = agService.Connect(rawAddr);
        EXPECT_EQ(0, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ConnectSuccessful(RawAddress rawAddr)
    {
        // make connecting state
        SetPremise_Connecting(rawAddr);

        // send connect successful message
        HfpAgMessage curEvent(HFP_AG_SLC_ESTABLISHED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        agService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_Disconnecting(RawAddress rawAddr)
    {
        // make connected state
        SetPremise_ConnectSuccessful(rawAddr);

        // disconnect to make the device unavailable
        int ret = agService.Disconnect(rawAddr);
        EXPECT_EQ(0, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_DisconnectSuccessful(RawAddress rawAddr)
    {
        // make Disconnecting state
        SetPremise_Disconnecting(rawAddr);

        // send disconnect successful message
        HfpAgMessage curEvent(HFP_AG_DISCONNECTED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        agService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoConnecting(RawAddress rawAddr)
    {
        SetPremise_ConnectSuccessful(rawAddr);
        bool ret = agService.SetActiveDevice(rawAddr);
        EXPECT_EQ(true, ret);
        usleep(50000);  // 50ms wait for service thread processing the message

        ret = agService.ConnectSco();
        EXPECT_EQ(true, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoConnectSuccessful(RawAddress rawAddr)
    {
        SetPremise_ScoConnecting(rawAddr);

        HfpAgMessage curEvent(HFP_AG_AUDIO_CONNECTED_EVT);
        curEvent.dev_ = rawAddr.GetAddress();
        agService.PostEvent(curEvent);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    void SetPremise_ScoDisconnecting(RawAddress rawAddr)
    {
        SetPremise_ScoConnectSuccessful(rawAddr);

        bool ret = agService.DisconnectSco();
        EXPECT_EQ(true, ret);
        usleep(50000);  // 50ms wait for service thread processing the message
    }

    // Need state >= connecting
    int MakeDataAvailableEvent(std::string command)
    {
        RfcommIncomingInfo eventData{agBtAddr, 1};
        std::string fullCommand(command + "\r");
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
            array.profileDescriptor[1].profileUuid.uuid16 = HFP_AG_UUID_SERVCLASS_HFP_HF;
            array.profileDescriptor[1].versionNumber = HFP_AG_HFP_VERSION_1_7;

            array.attributeNumber = 1;
            array.attribute = new SdpAttribute[array.attributeNumber];
            array.attribute[0].attributeId = HFP_AG_SDP_ATTRIBUTE_SUPPORTED_FEATURES;
            array.attribute[0].attributeValueLength = 2;
            uint16_t feature = 0xFFFF;
            array.attribute[0].attributeValue = &feature;

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
    StrictMock<AgBtmMock> btmMock_;
    StrictMock<AgGapMock> gapMock_;
    StrictMock<AgRfcommMock> rfcommMock_;
    StrictMock<AgSdpMock> sdpMock_;
    HfpAgServiceObserverTest agObs{};
};

TEST_F(HfpAgServiceTest, GetDeviceState1)
{
    SetPremise_StartService();

    int state = agService.GetDeviceState(agRawAddr);
    EXPECT_EQ((int)BTConnectState::DISCONNECTED, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, GetDeviceState2)
{
    SetPremise_Connecting(agRawAddr);

    int state = agService.GetDeviceState(agRawAddr);
    EXPECT_EQ((int)BTConnectState::CONNECTING, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, GetDeviceState3)
{
    SetPremise_ConnectSuccessful(agRawAddr);

    int state = agService.GetDeviceState(agRawAddr);
    EXPECT_EQ((int)BTConnectState::CONNECTED, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, GetDeviceState4)
{
    SetPremise_Disconnecting(agRawAddr);

    int state = agService.GetDeviceState(agRawAddr);
    EXPECT_EQ((int)BTConnectState::DISCONNECTING, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, ConnectFail)
{
    SetPremise_ConnectSuccessful(agRawAddr);

    int ret = agService.Connect(agRawAddr);
    EXPECT_EQ(HFP_AG_FAILURE, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, Connect_request)
{
    SetPremise_StartService();

    HfpAgMessage event(HFP_AG_CONNECT_REQUEST_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    int state = agService.GetDeviceState(agRawAddr);
    EXPECT_EQ((int)BTConnectState::CONNECTING, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, GetMaxConnectedNum)
{
    int maxNum = agService.GetMaxConnectNum();
    EXPECT_EQ(6, maxNum);
}

TEST_F(HfpAgServiceTest, IsInbandRinging)
{
    bool ret = agService.IsInbandRinging();
    EXPECT_EQ(false, ret);
}

// shutdown when there is device in connected state
TEST_F(HfpAgServiceTest, ShutDown)
{
    SetPremise_ConnectSuccessful(agRawAddr);
    SetPremise_StopService();
}

// If input device is in connected status,result is successful.
TEST_F(HfpAgServiceTest, Disconnect_1)
{
    // set premise
    SetPremise_Disconnecting(agRawAddr);
    SetPremise_StopService();
}

// If not set any premise(such as device is not connected),result is failed.
TEST_F(HfpAgServiceTest, Disconnect_2)
{
    SetPremise_StartService();

    int ret = agService.Disconnect(agRawAddr);
    EXPECT_EQ(HFP_AG_FAILURE, ret);
    usleep(50000);  // 100 ms

    SetPremise_StopService();
}

// IF there is one device in connected state, device is 1.
TEST_F(HfpAgServiceTest, GetConnectDevices_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    std::list<RawAddress> deviceList = agService.GetConnectDevices();
    EXPECT_EQ(1UL, deviceList.size());

    SetPremise_StopService();
}

// IF there is no device in connected state, device is 0.
TEST_F(HfpAgServiceTest, GetConnectDevices_2)
{
    std::list<RawAddress> deviceList = agService.GetConnectDevices();
    EXPECT_EQ(0UL, deviceList.size());
}

// Get connected state device.
TEST_F(HfpAgServiceTest, GetDevicesByStates_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    std::vector<int> states;
    states.push_back((int)BTConnectState::CONNECTED);
    std::vector<RawAddress> deviceList = agService.GetDevicesByStates(states);
    EXPECT_EQ(1UL, deviceList.size());
    EXPECT_EQ(agRawAddr.GetAddress(), deviceList[0].GetAddress());

    SetPremise_StopService();
}

// Input strong test
TEST_F(HfpAgServiceTest, GetDevicesByStates_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    std::vector<int> states;
    for (int i = 0; i < 3; i++) {
        states.push_back((int)BTConnectState::CONNECTED);
    }
    std::vector<RawAddress> deviceList = agService.GetDevicesByStates(states);
    EXPECT_EQ(1UL, deviceList.size());
    EXPECT_EQ(agRawAddr.GetAddress(), deviceList[0].GetAddress());

    SetPremise_StopService();
}

// Input strong test
TEST_F(HfpAgServiceTest, GetDevicesByStates_3)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    std::vector<int> states;
    states.push_back(10);

    std::vector<RawAddress> deviceList = agService.GetDevicesByStates(states);
    EXPECT_EQ(0UL, deviceList.size());

    SetPremise_StopService();
}

// All states test
TEST_F(HfpAgServiceTest, GetDevicesByStates_4)
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
    states.push_back((int)BTConnectState::DISCONNECTED);
    states.push_back((int)BTConnectState::CONNECTING);
    states.push_back((int)BTConnectState::DISCONNECTING);
    states.push_back((int)BTConnectState::CONNECTED);

    std::vector<RawAddress> deviceList = agService.GetDevicesByStates(states);
    EXPECT_EQ(4UL, deviceList.size());

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, ProcessEvent)
{
    HfpAgMessage event(HFP_AG_SERVICE_SHUTDOWN_EVT);
    agService.ProcessEvent(event);
    usleep(50000);  // 100 ms

    event.what_ = HFP_AG_REMOVE_STATE_MACHINE_EVT;
    agService.ProcessEvent(event);
    usleep(50000);  // 100 ms

    event.what_ = HFP_AG_VOICE_RECOGNITION_TIME_OUT_EVT;
    agService.ProcessEvent(event);
    usleep(50000);  // 100 ms
}

// If input device is in connected status,result is successful.
TEST_F(HfpAgServiceTest, SetActiveDevice_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_TRUE(ret);
    usleep(50000);  // 100 ms

    SetPremise_StopService();
}

// If input device is NULL, result is successful.
TEST_F(HfpAgServiceTest, SetActiveDevice_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    RawAddress rawAddr1("00:00:00:00:00:02");
    bool ret = agService.SetActiveDevice(rawAddr1);
    EXPECT_FALSE(ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// IF not set any active device, device is NULL.
TEST_F(HfpAgServiceTest, GetActiveDevice_1)
{
    std::string addr;
    addr = agService.GetActiveDevice();

    EXPECT_EQ("", addr);
}

// IF active device is set, device is the set device.
TEST_F(HfpAgServiceTest, GetActiveDevice_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    std::string addr;
    addr = agService.GetActiveDevice();
    EXPECT_EQ(agRawAddr.GetAddress(), addr);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, DialOutCallByHf_1)
{
    std::string deviceAddr = "00:00:00:00:00:01";
    bool ret = agService.DialOutCallByHf(deviceAddr);
    EXPECT_EQ(false, ret);
}

TEST_F(HfpAgServiceTest, DialOutCallByHf_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    std::string deviceAddr = agRawAddr.GetAddress();
    ret = agService.DialOutCallByHf(deviceAddr);
    EXPECT_EQ(true, ret);

    SetPremise_StopService();
}

//
TEST_F(HfpAgServiceTest, DisconnectSco_2)
{
    bool ret = agService.DisconnectSco();
    EXPECT_EQ(false, ret);
}

// If active device not NULL and it is connected, result is successful.
TEST_F(HfpAgServiceTest, ConnectSco_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.ConnectSco();
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If active device is NULL, result is failed.
TEST_F(HfpAgServiceTest, ConnectSco_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    bool ret = agService.ConnectSco();
    EXPECT_EQ(false, ret);

    SetPremise_StopService();
}

// If not set any premise, result is failed.
TEST_F(HfpAgServiceTest, ConnectSco_3)
{
    bool ret = agService.ConnectSco();
    EXPECT_EQ(false, ret);
}

// If connect sco twice,result is failed.
TEST_F(HfpAgServiceTest, ConnectSco_4)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    bool ret = agService.ConnectSco();
    EXPECT_EQ(false, ret);

    SetPremise_StopService();
}

// If connect sco tin connecting state, result is failed.
TEST_F(HfpAgServiceTest, ConnectSco_5)
{}

// If device is in audio connected state, result is successful.
TEST_F(HfpAgServiceTest, DisconnectSco_1)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    bool ret = agService.DisconnectSco();
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// HFP_AG_AUDIO_STATE_DISCONNECTED state test
TEST_F(HfpAgServiceTest, GetScoState_1)
{
    int state = agService.GetScoState(agRawAddr);
    EXPECT_EQ(HFP_AG_AUDIO_STATE_DISCONNECTED, state);
}

// HFP_AG_AUDIO_STATE_CONNECTED state test
TEST_F(HfpAgServiceTest, GetScoState_2)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    int state = agService.GetScoState(agRawAddr);
    EXPECT_EQ(HFP_AG_AUDIO_STATE_CONNECTED, state);

    SetPremise_StopService();
}

// HFP_AG_AUDIO_STATE_CONNECTING state test
TEST_F(HfpAgServiceTest, GetScoState_3)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    int state = agService.GetScoState(agRawAddr);
    EXPECT_EQ(HFP_AG_AUDIO_STATE_CONNECTING, state);

    SetPremise_StopService();
}

// HFP_AG_AUDIO_STATE_DISCONNECTING
TEST_F(HfpAgServiceTest, GetScoState_4)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    int state = agService.GetScoState(agRawAddr);
    EXPECT_EQ(HFP_AG_AUDIO_STATE_DISCONNECTING, state);

    SetPremise_StopService();
}

// If input device is active device, result is successful.
TEST_F(HfpAgServiceTest, OpenVoiceRecognition_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.OpenVoiceRecognition(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.CloseVoiceRecognition(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If input device is NULL, result is failed.
TEST_F(HfpAgServiceTest, OpenVoiceRecognition_2)
{
    bool ret = agService.OpenVoiceRecognition(agRawAddr);
    EXPECT_EQ(false, ret);
}

// If vr is opened,result is successful.
TEST_F(HfpAgServiceTest, CloseVoiceRecognition_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message
    ret = agService.OpenVoiceRecognition(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.CloseVoiceRecognition(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If vr is not opened,result is failed.
TEST_F(HfpAgServiceTest, CloseVoiceRecognition_2)
{
    bool ret = agService.CloseVoiceRecognition(agRawAddr);
    EXPECT_EQ(false, ret);
}

// test function call
TEST_F(HfpAgServiceTest, DialOutCallTimeOut)
{
    agService.DialOutCallTimeOut();
}

// If input device is active device, result is successful.
TEST_F(HfpAgServiceTest, OpenVoiceRecognitionByHf_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.OpenVoiceRecognitionByHf(agRawAddr.GetAddress());
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If input device is NULL , result is failed.
TEST_F(HfpAgServiceTest, OpenVoiceRecognitionByHf_2)
{
    bool ret = agService.OpenVoiceRecognitionByHf(agRawAddr.GetAddress());
    EXPECT_EQ(false, ret);
}

// If vr is opened, result is failed.
TEST_F(HfpAgServiceTest, OpenVoiceRecognitionByHf_3)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message
    ret = agService.OpenVoiceRecognition(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.OpenVoiceRecognitionByHf(agRawAddr.GetAddress());
    EXPECT_EQ(false, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If input device is active device and vr is opened, result is successful.
TEST_F(HfpAgServiceTest, CloseVoiceRecognitionByHf_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    std::string address = agRawAddr.GetAddress();
    ret = agService.OpenVoiceRecognition(address);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.CloseVoiceRecognitionByHf(address);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If input device is not active device, result is failed.
TEST_F(HfpAgServiceTest, CloseVoiceRecognitionByHf_2)
{
    bool ret = agService.CloseVoiceRecognitionByHf(agRawAddr.GetAddress());
    EXPECT_EQ(false, ret);
}

// If vr is not be opened, result is failed.
TEST_F(HfpAgServiceTest, CloseVoiceRecognitionByHf_3)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    ret = agService.CloseVoiceRecognitionByHf(agRawAddr.GetAddress());
    EXPECT_EQ(false, ret);

    SetPremise_StopService();
}

// test function call
TEST_F(HfpAgServiceTest, VoiceRecognitionTimeOut)
{
    agService.VoiceRecognitionTimeOut();
}

// test function call
TEST_F(HfpAgServiceTest, SetResponseClccTimer)
{
    agService.SetResponseClccTimer(agRawAddr.GetAddress());
}

TEST_F(HfpAgServiceTest, SlcStateChanged_1)
{
    // set premise
    SetPremise_StartService();
    int toState = HFP_AG_STATE_CONNECTED;
    HfpAgMessage msg(HFP_AG_CONNECT_REQUEST_EVT, 0, nullptr);
    msg.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(msg);
    usleep(50000);  // 50ms wait for service thread processing the message

    // test
    agService.SlcStateChanged(agRawAddr.GetAddress(), toState);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, SlcStateChanged_2)
{
    // set premise
    RawAddress rawAddr("00:00:00:00:00:02");
    SetPremise_ConnectSuccessful(agRawAddr);
    SetPremise_ConnectSuccessful(rawAddr);
    bool ret = agService.SetActiveDevice(rawAddr);
    EXPECT_EQ(true, ret);
    usleep(50000);  // 50ms wait for service thread processing the message

    // test
    int toState = HFP_AG_STATE_CONNECTED;
    agService.SlcStateChanged(agRawAddr.GetAddress(), toState);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, SlcStateChanged_3)
{
    // set premise
    std::string address = agRawAddr.GetAddress();
    int toState = HFP_AG_STATE_DISCONNECTED;
    bool ret = agService.SetActiveDevice(agRawAddr);
    EXPECT_EQ(false, ret);

    // test
    agService.SlcStateChanged(address, toState);
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 1;
    int callState = HFP_AG_CALL_STATE_DIALING;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 0;
    int callState = HFP_AG_CALL_STATE_ALERTING;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_3)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 0;
    int callState = HFP_AG_CALL_STATE_INCOMING;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_4)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 0;
    int callState = HFP_AG_CALL_STATE_IDLE;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_5)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 0;
    int callState = HFP_AG_CALL_STATE_ALERTING;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    numActive = 1;
    callState = HFP_AG_CALL_STATE_IDLE;
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, PhoneStateChanged_6)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int numActive = 0;
    int numHeld = 0;
    int callState = HFP_AG_CALL_STATE_INCOMING;
    std::string number = "10086";
    int type = 0;
    std::string name = "uttest";
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    numActive = 1;
    numHeld = 1;
    callState = HFP_AG_CALL_STATE_IDLE;
    agService.PhoneStateChanged(numActive, numHeld, callState, number, type, name);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, ClccResponse)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int index = 1;
    int direction = 1;
    int status = 0;
    int mode = 1;
    bool mpty = 0;
    std::string number = "10086";
    int type = HFP_AG_CALL_NUMBER_INTERNATIONAL;
    agService.ClccResponse(index, direction, status, mode, mpty, number, type);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgDisconnected_SlcConnected)
{
    // set premise
    SetPremise_DisconnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_SLC_ESTABLISHED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgDisconnected_Disconnect)
{
    // set premise
    SetPremise_DisconnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_ScoConnectRequest)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECT_REQUEST_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_ScoConnected)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_ConnectSco)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_SdpDiscoverySuccess)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    // HFP_AG_SDP_DISCOVERY_RESULT_SUCCESS
    int ret = MakeSdpSearchCb(true);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_SdpDiscoveryFail)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    // HFP_AG_SDP_DISCOVERY_RESULT_FAIL
    int ret = MakeSdpSearchCb(false);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_Disconnected)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    // HFP_AG_DISCONNECTED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_DISCONNECTED, nullptr, nullptr);
    HfpAgMessage event(HFP_AG_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_Connect)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_Connected)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    // HFP_AG_CONNECTED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_SUCCESS, nullptr, nullptr);
    HfpAgMessage event(HFP_AG_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_ConnectFail)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    // HFP_AG_CONNECT_FAILED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_CONNECT_FAIL, nullptr, nullptr);
    HfpAgMessage event(HFP_AG_CONNECT_FAILED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_DataAvailable)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).WillOnce(DoAll(SetArgPointee<1>(nullptr), Return(1)));
    HfpAgMessage event(HFP_AG_DATA_AVAILABLE_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECTION_TIMEOUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnecting_MessageNone)
{
    // set premise
    SetPremise_Connecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = agRawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_TYPE_NONE;
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgDisconnecting_Connect)
{
    // set premise
    SetPremise_Disconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgDisconnecting_ScoConnectRequest)
{
    // set premise
    SetPremise_Disconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECT_REQUEST_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgDisconnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_Disconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECTION_TIMEOUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_Connect)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_DisconnectSco)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ScoConnectRequest)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int state = HFP_AG_AUDIO_CONNECT_REQUEST_EVT;
    uint8_t linkType = 1;
    HfpAgProfileEventSender::GetInstance().ScoConnectRequest(agRawAddr.GetAddress(), state, linkType);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ScoConnecting)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTING_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ScoConnected)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_Disconnected)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_DISCONNECTED_EVT
    GetRfcommMocker().rfcommCb(1, RFCOMM_CHANNEL_EV_DISCONNECTED, nullptr, nullptr);
    HfpAgMessage event(HFP_AG_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_DataAvailable)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).WillOnce(DoAll(SetArgPointee<1>(nullptr), Return(1)));
    HfpAgMessage event(HFP_AG_DATA_AVAILABLE_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_AnswerCall)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_ANSWER_CALL
    std::string cmd = "ATA";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_HangCall1)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_HANGUP_CALL
    std::string cmd = "AT+CHUP";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_HangCall2)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    agService.SetActiveDevice(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_HANGUP_CALL
    std::string cmd = "AT+CHUP";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_SetVolumeSpeaker)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_VOLUME_CHANGED
    std::string cmd = "AT+VGS=5";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_SetVolumeMic)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_VOLUME_CHANGED
    std::string cmd = "AT+VGM=10";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_SendDtmf)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_SEND_DTMF
    std::string cmd = "AT+VTS=3";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_NoiseReduction)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_NOISE_REDUCTION
    std::string cmd = "AT+NREC=0";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ProcessChld)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_CHLD
    std::string cmd = "AT+CHLD=1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_RequestSubscriberNumber)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST
    std::string cmd = "AT+CNUM";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_GetAgIndicator)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_CIND
    HfpAgProfileEventSender::GetInstance().GetAgIndicator(agRawAddr.GetAddress());
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_RequestOperatorName)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_COPS
    std::string cmd = "AT+COPS?";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_GetCurrentCalls)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_CLCC
    std::string cmd = "AT+CLCC";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ProcessBind)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_BIND
    std::string cmd = "AT+BIND=1,2";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ProcessBiev)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_BIEV
    std::string cmd = "AT+BIEV=1,1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ProcessBia)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_AT_BIA
    std::string cmd = "AT+BIA=1,1,1,1,1,1,1";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_UpdateConnectState)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    int state = 1;
    HfpAgProfileEventSender::GetInstance().UpdateConnectState(agRawAddr.GetAddress(), state);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ProcessAtBcc)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    agService.SetActiveDevice(agRawAddr);

    // HFP_AG_CONNECT_AUDIO_EVT
    std::string cmd = "AT+BCC";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_VoiceRecognitionStateChanged)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_VR_CHANGED
    std::string cmd = "AT+BVRA=0";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_VoiceRecognitionResult)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_VOICE_RECOGNITION_RESULT_EVT, 1);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    event.arg1_ = 0;
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_VoiceRecognitionTimeOut)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_VOICE_RECOGNITION_TIME_OUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_CallStateChange)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);
    HfpAgMessage event(HFP_AG_CALL_STATE_CHANGE);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ClccResponse)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_SEND_CCLC_RESPONSE);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_DialOutResult)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DIALING_OUT_RESULT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_ClccTimeOut)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_ProcessAtBcs)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    // HFP_AG_CODEC_NEGOTIATED_EVT
    HfpAgProfileEventSender::GetInstance().ProcessAtBcs(agRawAddr.GetAddress());
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_Connect)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_Disconnect)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_CodecNegotitionFailed)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CODEC_NEGOTIATION_FAILED);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_Disconnected)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_RetryConnectSco)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_RETRY_CONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_SetipCodecCvsd)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_SETUP_CODEC_CVSD);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_ScoConnected)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_ScoConnectFail)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECT_FAILED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_ScoConnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECTION_TIMEOUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_Disconnect)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_Disconnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_ScoConnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_DisconnectSco)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_ScoDisconnected)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_ScoDisconnecting)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_DISCONNECTING_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_SendVolume_1)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_SET_VOLUME_EVT);
    event.dev_ = agRawAddr.GetAddress();
    event.arg1_ = HFP_AG_VOLUME_TYPE_SPK;
    event.arg3_ = 1;
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoConnected_SendVolume_2)
{
    // set premise
    SetPremise_ScoConnectSuccessful(agRawAddr);

    HfpAgMessage event(HFP_AG_SET_VOLUME_EVT);
    event.dev_ = agRawAddr.GetAddress();
    event.arg1_ = HFP_AG_VOLUME_TYPE_MIC;
    event.arg3_ = 1;
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_Connect)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_Disconnect)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_ConnectSco)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_DisconnectSco)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_DISCONNECT_AUDIO_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_ScoDisconnected)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_DISCONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_DisconnectScoFail)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_DISCONNECT_FAILED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_ScoConnected)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECTED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_ConnectScoFail)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_AUDIO_CONNECT_FAILED_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_SetInbandRingTone)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_SET_INBAND_RING_TONE_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgScoDisconnecting_ConnectionTimeOut)
{
    // set premise
    SetPremise_ScoDisconnecting(agRawAddr);

    HfpAgMessage event(HFP_AG_CONNECTION_TIMEOUT_EVT);
    event.dev_ = agRawAddr.GetAddress();
    agService.PostEvent(event);
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

// If there is no device connected,state is 0.
TEST_F(HfpAgServiceTest, GetConnectState_1)
{
    int state = agService.GetConnectState();
    EXPECT_EQ(0, state);
}

// If there is device connected,state is PROFILE_STATE_CONNECTED.
TEST_F(HfpAgServiceTest, GetConnectState_2)
{
    // set premise:connect devices;
    SetPremise_ConnectSuccessful(agRawAddr);

    int state = agService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_CONNECTED, state);
    SetPremise_StopService();
}

// PROFILE_STATE_CONNECTING.
TEST_F(HfpAgServiceTest, GetConnectState_3)
{
    // set premise:connect devices;
    SetPremise_Connecting(agRawAddr);

    int state = agService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_CONNECTING, state);
    SetPremise_StopService();
}

// PROFILE_STATE_DISCONNECTING.
TEST_F(HfpAgServiceTest, GetConnectState_4)
{
    // set premise:connect devices and disconnect it;
    SetPremise_Disconnecting(agRawAddr);

    int state = agService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_DISCONNECTING, state);
    SetPremise_StopService();
}

// If device is connected, and transmit to disconnected.
TEST_F(HfpAgServiceTest, GetConnectState_5)
{
    // set premise:connect devices and disconnect it(disconnected);
    SetPremise_DisconnectSuccessful(agRawAddr);
    SetPremise_DisconnectSuccessful(agRawAddr);

    int state = agService.GetConnectState();
    EXPECT_EQ(PROFILE_STATE_DISCONNECTED, state);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyHfEnhancedDriverSafety)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyHfEnhancedDriverSafety(agRawAddr, 1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyHfBatteryLevel)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyHfBatteryLevel(agRawAddr, 3);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyRegistrationStatusChanged)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyRegistrationStatusChanged(1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifySignalStrengthChanged)
{
    // set premise
    SetPremise_StartService();

    agService.NotifySignalStrengthChanged(1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyRoamingStatusChanged)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyRoamingStatusChanged(1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyBatteryLevelChanged)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyBatteryLevelChanged(1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, NotifyVolumeChanged)
{
    // set premise
    SetPremise_StartService();

    agService.NotifyVolumeChanged(1, 1);

    SetPremise_StopService();
}

TEST_F(HfpAgServiceTest, HfpAgConnected_DialCall)
{
    // set premise
    SetPremise_ConnectSuccessful(agRawAddr);

    // HFP_AG_CONTROL_OTHER_MODULES_EVT - HFP_AG_MSG_TYPE_DIAL_CALL
    std::string cmd = "ATD10086;";
    int ret = MakeDataAvailableEvent(cmd);
    EXPECT_EQ(0, ret);

    agService.PhoneStateChanged(0, 0, HFP_AG_CALL_STATE_DIALING, "10086", 1, "test");
    usleep(50000);  // 50ms wait for service thread processing the message

    SetPremise_StopService();
}

TEST(HfpAgServiceObserverTest, VirtualFunctionTest)
{
    HfpAgServiceObserverTest testCallback;
    HfpAgServiceObserver virtCallback = (HfpAgServiceObserver)testCallback;
    RawAddress agRawAddr = RawAddress("00:00:00:00:00:01");

    virtCallback.OnActiveDeviceChanged(agRawAddr);
    virtCallback.OnConnectionStateChanged(agRawAddr, 1);
    virtCallback.OnHfBatteryLevelChanged(agRawAddr, 1);
    virtCallback.OnHfEnhancedDriverSafetyChanged(agRawAddr, 1);
    virtCallback.OnScoStateChanged(agRawAddr, 1);
}