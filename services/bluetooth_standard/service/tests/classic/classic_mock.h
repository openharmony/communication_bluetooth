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

#ifndef CLASSIC_MOCK_H
#define CLASSIC_MOCK_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "interface_adapter.h"
#include "interface_adapter_classic.h"
#include "classic_adapter.h"
#include "interface_profile.h"
#include <optional>
#include "common_mock_all.h"

class ClassicMock {
public:
    MOCK_METHOD0(BTM_Enable, int());
    MOCK_METHOD0(BTM_Disable, int());
    MOCK_METHOD0(BTM_RegisterACLCallbacks, int());
    MOCK_METHOD0(BTM_DeregisterACLCallbacks, int());

    MOCK_METHOD0(SDP_CreateServiceRecord, int());
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
    MOCK_METHOD0(SDP_ServiceSearchAttribute, int());

    MOCK_METHOD0(GAPIF_RegisterDiscoveryCallback, int());
    MOCK_METHOD0(GAPIF_RegisterAuthenticationCallback, int());
    MOCK_METHOD0(GAPIF_RegisterSecurityCallback, int());
    MOCK_METHOD0(GAPIF_UserConfirmRsp, int());
    MOCK_METHOD0(GAPIF_UserPasskeyRsp, int());
    MOCK_METHOD0(GAPIF_PinCodeRsp, int());
    MOCK_METHOD0(GAPIF_SetScanMode, int());
    MOCK_METHOD0(GAPIF_SetClassOfDevice, int());
    MOCK_METHOD0(GAPIF_GetRemoteName, int());
    MOCK_METHOD0(GAPIF_Inquiry, int());
    MOCK_METHOD0(GAPIF_PairIsFromLocal, int());
    MOCK_METHOD0(GAPIF_IOCapabilityRsp, int());
    MOCK_CONST_METHOD0(XmlParse_Load, bool());
    MOCK_METHOD0(XmlGetValue, std::string());
    MOCK_METHOD0(XmlSetValueString, bool());
};

class ClassicAdapterObserver_MOCK : public bluetooth::IAdapterClassicObserver {
public:
    MOCK_METHOD1(OnDiscoveryStateChanged, void(int status));
    MOCK_METHOD1(OnDiscoveryResult, void(const bluetooth::RawAddress &device));
    MOCK_METHOD2(OnPairRequested, void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device));
    MOCK_METHOD4(OnPairConfirmed,
        void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device, int reqType, int number));
    MOCK_METHOD1(OnScanModeChanged, void(int mode));
    MOCK_METHOD1(OnDeviceNameChanged, void(const std::string &deviceName));
    MOCK_METHOD1(OnDeviceAddrChanged, void(const std::string &address));
};

class ClassicRemoteDeviceObserver_MOCK : public bluetooth::IClassicRemoteDeviceObserver {
public:
    MOCK_METHOD3(OnPairStatusChanged,
        void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device, int status));
    MOCK_METHOD2(
        OnRemoteUuidChanged, void(const bluetooth::RawAddress &device, const std::vector<bluetooth::Uuid> &uuids));
    MOCK_METHOD2(OnRemoteNameChanged, void(const bluetooth::RawAddress &device, const std::string &deviceName));
    MOCK_METHOD2(OnRemoteAliasChanged, void(const bluetooth::RawAddress &device, const std::string &alias));
    MOCK_METHOD2(OnRemoteCodChanged, void(const bluetooth::RawAddress &device, int cod));
    MOCK_METHOD2(OnRemoteBatteryLevelChanged, void(const bluetooth::RawAddress &device, int batteryLevel));
};

class ClassicAdapterTest : public testing::Test {
public:
    static ClassicAdapterObserver_MOCK *classicAdapterObserver_;
    static ClassicRemoteDeviceObserver_MOCK *classicRemoteDeviceObserver_;
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class HfpAgMock : public bluetooth::IProfileHfpAg {
public:
    ~HfpAgMock()
    {}
    MOCK_METHOD1(GetDevicesByStates, std::vector<bluetooth::RawAddress>(std::vector<int> states));
    MOCK_METHOD1(GetDeviceState, int(const bluetooth::RawAddress &device));
    MOCK_METHOD1(GetScoState, int(const bluetooth::RawAddress &device));
    MOCK_METHOD0(ConnectSco, bool());
    MOCK_METHOD0(DisconnectSco, bool());
    MOCK_METHOD6(PhoneStateChanged,
        void(int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name));
    MOCK_METHOD7(ClccResponse,
        void(int index, int direction, int status, int mode, bool mpty, const std::string &number, int type));
    MOCK_METHOD1(OpenVoiceRecognition, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(CloseVoiceRecognition, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(SetActiveDevice, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD0(GetActiveDevice, std::string());
    void RegisterObserver(bluetooth::HfpAgServiceObserver &observer) override;
    void DeregisterObserver(bluetooth::HfpAgServiceObserver &observer)
    {}
    MOCK_METHOD1(Connect, int(const bluetooth::RawAddress &device));
    MOCK_METHOD1(Disconnect, int(const bluetooth::RawAddress &device));
    MOCK_METHOD0(GetConnectDevices, std::list<bluetooth::RawAddress>());
    MOCK_METHOD0(GetConnectState, int());
    MOCK_METHOD0(GetMaxConnectNum, int());
    MOCK_METHOD0(GetContext, utility::Context *());
};

class HfpHfMock : public bluetooth::IProfileHfpHf {
public:
    HfpHfMock();
    ~HfpHfMock();
    MOCK_METHOD0(GetContext, utility::Context *());
    MOCK_METHOD1(Connect, int(const bluetooth::RawAddress &device));
    MOCK_METHOD1(Disconnect, int(const bluetooth::RawAddress &device));
    MOCK_METHOD0(GetConnectDevices, std::list<bluetooth::RawAddress>());
    MOCK_METHOD0(GetConnectState, int());
    MOCK_METHOD0(GetMaxConnectNum, int());
    MOCK_METHOD1(ConnectSco, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(DisconnectSco, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(GetDevicesByStates, std::vector<bluetooth::RawAddress>(std::vector<int> states));
    MOCK_METHOD1(GetDeviceState, int(const bluetooth::RawAddress &device));
    MOCK_METHOD1(GetScoState, int(const bluetooth::RawAddress &device));
    MOCK_METHOD2(SendDTMFTone, bool(const bluetooth::RawAddress &device, uint8_t code));
    MOCK_METHOD1(
        GetScoGetCurrentCallListState, std::vector<bluetooth::HandsFreeUnitCalls>(const bluetooth::RawAddress &device));
    MOCK_METHOD1(GetCurrentCallList, std::vector<bluetooth::HandsFreeUnitCalls>(const bluetooth::RawAddress &device));
    MOCK_METHOD2(AcceptIncomingCall, bool(const bluetooth::RawAddress &device, int flag));
    MOCK_METHOD1(HoldActiveCall, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(RejectIncomingCall, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD2(
        FinishActiveCall, bool(const bluetooth::RawAddress &device, const bluetooth::HandsFreeUnitCalls &call));
    MOCK_METHOD2(StartDial,
        std::optional<bluetooth::HandsFreeUnitCalls>(const bluetooth::RawAddress &device, const std::string &number));
    void DeregisterObserver(bluetooth::HfpHfServiceObserver &observer)
    {}
    void RegisterObserver(bluetooth::HfpHfServiceObserver &observer) override;
    MOCK_METHOD2(SetHfVolume, void(int volume, int type));
    MOCK_METHOD1(OpenVoiceRecognition, bool(const bluetooth::RawAddress &device));
    MOCK_METHOD1(CloseVoiceRecognition, bool(const bluetooth::RawAddress &device));
};

class IContextCallbackMock : public utility::IContextCallback {
public:
    ~IContextCallbackMock() = default;
    MOCK_METHOD2(OnEnable, void(const std::string &name, bool ret));
    MOCK_METHOD2(OnDisable, void(const std::string &name, bool ret));
};
void registerBtmMock(ClassicMock *instance);
void registerGapMock(ClassicMock *instance);
void registerXmlMock(ClassicMock *instance);
void registerSdpMock(ClassicMock *instance);
GapDiscoveryCallback *GetDiscoveryCallback();
GapAuthenticationCallback *GetAuthenticationCallback();
GapSecurityCallback *GetSecurityCallback();
BtmAclCallbacks *GetAclCallbacks();
typedef void (*SearchAttributeCb)(
    const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context);
SearchAttributeCb GetSdpServiceSearchAttCb();
GapSetScanModeResultCallback GetSetScanModeResultCallback();
#endif