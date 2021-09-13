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
#ifndef MCE_MOCK_H
#define MCE_MOCK_H

#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "obex_server.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "interface_profile_map_mce.h"
#include "obex_socket_transport.h"

extern "C" {
#include "btstack.h"
#include "btm.h"
#include "log.h"
}

using namespace bluetooth;
enum MceTestCtrlType {
    MCE_TEST_CTRL_NO_RESPONES,
    MCE_TEST_CTRL_RESPONES_OK,
    MCE_TEST_CTRL_RESPONES_NG,
    MCE_TEST_CTRL_CONNECT_OK,
    MCE_TEST_CTRL_CONNECT_FAILED,
    MCE_TEST_CTRL_DISCONNECT_OK,
    MCE_TEST_CTRL_TRANSPORT_ERRO,
    MCE_TEST_CTRL_ACTION_COMPLETE_OK,
    MCE_TEST_CTRL_ACTION_COMPLETE_NG,
    MCE_TEST_CTRL_SPECIAL_CODE1,
};

enum MceExcuteStatus {
    MCE_TEST_EXCUTE_STATUS_ON_CONNECTED = 1,
    MCE_TEST_EXCUTE_STATUS_ON_DISONCONNECTED = 2,
    MCE_TEST_EXCUTE_STATUS_ON_MAP_EVENT_REPORTED = 3,
    MCE_TEST_EXCUTE_STATUS_RET_NO_ERROR = 4,
    MCE_TEST_EXCUTE_STATUS_RET_OPRATION_FAILED = 5,
    MCE_TEST_EXCUTE_STATUS_IDLE = 0xFF
};

extern std::string mceProfileId_;
extern bool mceProfileIdRet_;
extern int obexConnectCtrl;
extern int obexDisconnectCtrl;
extern int obexPutGetCtrl;
extern int gapCtrl;
extern int sdpCtrl;

extern int callbackExcuteStatus;
extern int callbackConnectionStatus;

extern bluetooth::ObexServerObserver *obexServerObserver_;

extern std::unique_ptr<bluetooth::ObexHeader> header0;
extern std::unique_ptr<bluetooth::ObexHeader> header1;

extern std::unique_ptr<bluetooth::ObexHeader> mnsHeader0;
extern std::unique_ptr<bluetooth::ObexHeader> mnsHeader1;

extern bluetooth::ObexClient *obexClient;
extern int TEST_MasServiceSdpSearchCallback(uint8_t attributeId, uint32_t rfcomNo);

class MapMceUintTestCallback : public IProfileMapMceObserver {
public:
    void OnMapActionCompleted(
        const RawAddress &deviceAddress, const IProfileMapAction &action, MapExecuteStatus status) override;
    void OnMapEventReported(const RawAddress &deviceAddress, const IProfileMapEventReport &report) override;
    void OnConnectionStateChanged(const RawAddress &deviceAddress, int state) override;

    void OnBmessageCompleted(
        const RawAddress &deviceAddress, const IProfileBMessage &bmsg, MapExecuteStatus status) override;
    void OnMessagesListingCompleted(
        const RawAddress &deviceAddress, const IProfileMessagesListing &listing, MapExecuteStatus status) override;
    void OnConversationListingCompleted(
        const RawAddress &deviceAddress, const IProfileConversationListing &listing, MapExecuteStatus status) override;
};

class MapMceMnsTransTestCallback : public ObexTransportObserver {
public:
    void OnTransportIncomingDisconnected(const std::string &btAddr) override;
    void OnTransportConnected(ObexTransport &transport) override;
    void OnTransportDisconnected(ObexTransport &transport) override;
    void OnTransportDataBusy(ObexTransport &transport, uint8_t isBusy) override;
    void OnTransportDataAvailable(ObexTransport &transport, ObexPacket &obexPacket) override;
    void OnTransportError(ObexTransport &transport, int errCd) override;
    void OnTransportConnectIncoming(ObexIncomingConnect &incomingConnect) override;
};
class MapMceUintTestCallback2 : public IProfileMapMceObserver {
public:
};
class MapMceAdapterConfigMock {
public:
    static bool GetValueInt(const std::string &section, const std::string &property, int &value);
    static bool GetValueBool(const std::string &section, const std::string &property, bool &value);

private:
    MapMceAdapterConfigMock() = default;
    virtual ~MapMceAdapterConfigMock() = default;
};
#endif  // MCE_MOCK_H
