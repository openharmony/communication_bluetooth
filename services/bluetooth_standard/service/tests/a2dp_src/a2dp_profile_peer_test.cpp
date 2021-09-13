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

#include "a2dp_src_test.h"
#include "a2dp_sbc_param_ctrl.h"

using namespace testing;
using namespace bluetooth;

class A2dp_profile_peer_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dp_profile_peer_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dp_profile_peer_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvdtpMock(&mockAvdtp_);
        EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_CODEC_SBC, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(1), Return(true)));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_CODEC_AAC, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(2), Return(true)));
        service = new A2dpSrcService();
        service->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SRC))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(service));
    }
    virtual void TearDown()
    {
        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

protected:
    NiceMock<MockAvdtp> mockAvdtp_;
    A2dpSrcService *service;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(A2dp_profile_peer_test, SetPeerSepInfo)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x07}, 0x01};
    uint8_t peerSepType = A2DP_ROLE_SINK;
    A2dpProfilePeer *contruct = new A2dpProfilePeer(addr, peerSepType);
    AvdtDiscover seps;
    seps.numSeps = 1;
    seps.seps[0].isUsed = true;
    seps.seps[0].mediaType = 0;
    seps.seps[0].role = 0;
    seps.seps[0].seid = 0;
    seps.seps[0].sepType = 0;

    contruct->SetPeerSepInfo(seps);
    AvdtSepInfo ret = contruct->GetPeerSEPInformation();
    EXPECT_EQ(true, ret.isUsed);

    delete contruct;
    contruct = nullptr;
}

TEST_F(A2dp_profile_peer_test, ActivePeerAddress)
{
    BtAddr addr = {{0x02, 0x03, 0x04, 0x05, 0x06, 0x07}, 0x04};
    uint8_t peerSepType = A2DP_ROLE_SINK;
    A2dpProfilePeer *contruct = new A2dpProfilePeer(addr, peerSepType);

    BtAddr ret = contruct->GetPeerAddress();
    EXPECT_EQ(4, ret.type);

    delete contruct;
    contruct = nullptr;
}

TEST_F(A2dp_profile_peer_test, GetPeerCapabilityInfo)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x07}, 0x01};
    uint8_t peerSepType = A2DP_ROLE_SINK;
    A2dpProfilePeer *contruct = new A2dpProfilePeer(addr, peerSepType);

    AvdtSepConfig ret = contruct->GetPeerCapabilityInfo();
    EXPECT_EQ(0, ret.mediaType);
    EXPECT_EQ(0, ret.numCodec);
    EXPECT_EQ(0, ret.pscMask);

    delete contruct;
    contruct = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetStreamHandle)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x07}, 0x01};
    uint8_t peerSepType = A2DP_ROLE_SINK;
    uint16_t handle = 3;
    A2dpProfilePeer *contruct = new A2dpProfilePeer(addr, peerSepType);

    contruct->SetStreamHandle(handle);

    EXPECT_EQ(3, contruct->GetStreamHandle());
    delete contruct;
    contruct = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerCapInfo_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);
    uint8_t intSeid = 0;
    uint8_t acpSeid = 0;
    AvdtSepConfig cap{0};
    A2dpSBCCapability sbcCap;
    sbcCap.minBitpool = 100;
    sbcCap.maxBitpool = 200;
    BuildSbcInfo(&sbcCap, cap.codecInfo);
    uint8_t role = A2DP_ROLE_SOURCE;

    peer->SetPeerCapInfo(intSeid, acpSeid, cap, role);
    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerCapInfo_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);
    uint8_t intSeid = 0;
    uint8_t acpSeid = 0;
    AvdtSepConfig cap{0};
    A2dpSBCCapability sbcCap;
    sbcCap.minBitpool = 100;
    sbcCap.maxBitpool = 200;
    BuildSbcInfo(&sbcCap, cap.codecInfo);
    uint8_t role = A2DP_ROLE_SINK;

    peer->SetPeerCapInfo(intSeid, acpSeid, cap, role);
    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, A2dpEncoderObserver_Read)
{
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserver(0);
    uint8_t *buf = nullptr;
    uint32_t size = 0;
    stub::A2dpServiceObserver obs;

    stub::A2dpService::GetInstance()->RegisterObserver(&obs);
    observer->Read(&buf, size);
    delete observer;
    observer = nullptr;
}

TEST_F(A2dp_profile_peer_test, A2dpEncoderObserver_EnquePacket)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    Packet *packet = PacketMalloc(0, 0, 5);
    size_t frames = 0;
    uint32_t bytes = 0;
    uint32_t pktTimeStamp = 0;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserver(0);

    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SOURCE);
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    profile->SetActivePeer(addr);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_WriteReq).WillOnce(Return(0));
    EXPECT_TRUE(observer->EnqueuePacket(packet, frames, bytes, pktTimeStamp));

    profile->DeletePeer(addr);
    delete observer;
    observer = nullptr;
}

TEST_F(A2dp_profile_peer_test, A2dpCodecDecoderObserver_DataAvailable)
{
    A2dpCodecDecoderObserver *observer = new A2dpCodecDecoderObserver();
    uint8_t *buf = nullptr;
    uint32_t size = 0;
    stub::A2dpServiceObserver obs;

    stub::A2dpService::GetInstance()->RegisterObserver(&obs);
    observer->DataAvailable(buf, size);
    delete observer;
    observer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SDPServiceCallback)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint16_t serviceNum = 2;
    SdpService serviceArray[2];
    BtUuid classidsnk[1];
    classidsnk[0].uuid16 = A2DP_SINK_SERVICE_CLASS_UUID;
    serviceArray[0].classId = classidsnk;
    SdpProtocolDescriptor des1[2];
    des1[0].protocolUuid.uuid16 = A2DP_PROTOCOL_UUID_L2CAP;
    des1[0].parameter[0].value = A2DP_PROTOCOL_UUID_AVDTP;
    des1[1].protocolUuid.uuid16 = A2DP_PROTOCOL_UUID_AVDTP;
    des1[1].parameter[0].value = A2DP_PROFILE_REV_1_3;
    serviceArray[0].descriptor = des1;
    SdpProfileDescriptor fdes1[1];
    fdes1[0].profileUuid.uuid16 = A2DP_PROFILE_UUID;
    fdes1[0].versionNumber = A2DP_PROFILE_REV_1_3;
    serviceArray[0].profileDescriptor = fdes1;

    BtUuid classidsrc[1];
    classidsrc[0].uuid16 = A2DP_SERVICE_CLASS_UUID;
    serviceArray[1].classId = classidsrc;
    SdpProtocolDescriptor des2[2] = {0};
    serviceArray[1].descriptor = des2;
    SdpProfileDescriptor fdes2[1] = {0};
    serviceArray[1].profileDescriptor = fdes2;

    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SOURCE);
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ConnectReq).WillOnce(Return(0));
    peer->SDPServiceCallback(&addr, serviceArray, serviceNum, profile);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    profile->DeletePeer(addr);
    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, EnableOptionalCodec_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);

    peer->EnableOptionalCodec(true);
    peer->EnableOptionalCodec(true);
    peer->EnableOptionalCodec(false);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, EnableOptionalCodec_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    peer->EnableOptionalCodec(true);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, NotifyAudioConfigChanged)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    peer->NotifyAudioConfigChanged();

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, UpdateConfigure)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigReq).WillOnce(Return(0));
    peer->UpdateConfigure();

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetOptionalCodecsSupportState)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    StrictMock<ProfileConfigInstanceMock> pcinstanceMock_;
    StrictMock<ProfileConfigMock> pcMock_;
    registerNewMockProfileConfig(&pcinstanceMock_);
    EXPECT_CALL(pcinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&pcMock_));

    EXPECT_CALL(pcMock_, RemoveProperty(_, SECTION_CODE_CS_SUPPORT, PROPERTY_A2DP_SUPPORTS_OPTIONAL_CODECS))
        .WillOnce(Return(true));

    peer->SetOptionalCodecsSupportState(A2DP_OPTIONAL_SUPPORT_UNKNOWN);

    EXPECT_CALL(pcMock_, SetValue(_, SECTION_CODE_CS_SUPPORT, PROPERTY_A2DP_SUPPORTS_OPTIONAL_CODECS, An<int &>()))
        .WillOnce(Return(true));

    peer->SetOptionalCodecsSupportState(A2DP_OPTIONAL_NOT_SUPPORT);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetAudioConfigure)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);
    uint8_t intSeid = 0;
    uint8_t acpSeid = 0;
    AvdtSepConfig cap{0};
    A2dpSBCCapability sbcCap;
    sbcCap.minBitpool = 100;
    sbcCap.maxBitpool = 200;
    BuildSbcInfo(&sbcCap, cap.codecInfo);
    uint8_t role = A2DP_ROLE_SOURCE;

    peer->SetPeerCapInfo(intSeid, acpSeid, cap, role);

    peer->EnableOptionalCodec(false);
    peer->SetAudioConfigure(0, 0, 0);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerSinkCodecCapabilities_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    uint8_t codecInfo[AVDT_CODEC_SIZE] = {0};
    uint8_t *codecInformation = codecInfo;
    A2dpSBCCapability sbcCap;
    sbcCap.minBitpool = 100;
    sbcCap.maxBitpool = 200;

    BuildSbcInfo(&sbcCap, codecInformation);

    peer->GetCodecConfigure()->SetPeerSinkCodecCapabilities(codecInformation);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerSinkCodecCapabilities_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SINK);

    uint8_t codecInfo[AVDT_CODEC_SIZE] = {0};
    uint8_t *codecInformation = codecInfo;
    A2dpAACCapability aacCap;

    BuildAacInfo(&aacCap, codecInformation);

    peer->GetCodecConfigure()->SetPeerSinkCodecCapabilities(codecInformation);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerSourceCodecCapabilities_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);

    uint8_t codecInfo[AVDT_CODEC_SIZE] = {0};
    uint8_t *codecInformation = codecInfo;
    A2dpSBCCapability sbcCap;
    sbcCap.minBitpool = 100;
    sbcCap.maxBitpool = 200;

    BuildSbcInfo(&sbcCap, codecInformation);

    peer->GetCodecConfigure()->SetPeerSourceCodecCapabilities(codecInformation);

    delete peer;
    peer = nullptr;
}

TEST_F(A2dp_profile_peer_test, SetPeerSourceCodecCapabilities_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfilePeer *peer = new A2dpProfilePeer(addr, A2DP_ROLE_SOURCE);

    uint8_t codecInfo[AVDT_CODEC_SIZE] = {0};
    uint8_t *codecInformation = codecInfo;
    A2dpAACCapability aacCap;

    BuildAacInfo(&aacCap, codecInformation);

    peer->GetCodecConfigure()->SetPeerSourceCodecCapabilities(codecInformation);

    delete peer;
    peer = nullptr;
}