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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <iostream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "adapter_manager.h"
#include "message.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "securec.h"
#include "pbap_pce_def.h"
#include "data_test_util.h"
#include "pbap_pce_service.h"


using namespace testing;
using namespace bluetooth;

namespace pbap_pce_legacy_test {
class ContextCallback : public utility::IContextCallback {
public:
    ContextCallback() = default;
    virtual ~ContextCallback() = default;
    void OnEnable(const std::string &name, bool ret) override
    {
        if (ret) {
            lastRet_ = 1;
        } else {
            lastRet_ = 0;
        }
        callCountOnEnable_++;
    }
    void OnDisable(const std::string &name, bool ret) override
    {
        if (ret) {
            lastRet_ = 1;
        } else {
            lastRet_ = 0;
        }
        callCountOnDisable_++;
    }

    void Reset()
    {
        callCountOnEnable_ = 0;
        callCountOnDisable_ = 0;
        lastRet_ = -1;
    }
    int callCountOnEnable_{0};
    int callCountOnDisable_{0};
    int lastRet_ = -1;
};

class TestPbapObserverForClientImpl : public IPbapPceObserver {
public:
    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        switch (state) {
            case static_cast<int>(BTConnectState::CONNECTING):
                break;
            case static_cast<int>(BTConnectState::CONNECTED):
                callCountOnConnected++;
                break;
            case static_cast<int>(BTConnectState::DISCONNECTING):
                break;
            case static_cast<int>(BTConnectState::DISCONNECTED):
                callCountOnDisconnected++;
                break;
            default:
                break;
        }
    }
    void OnServicePasswordRequired(
        const RawAddress &device, const std::vector<uint8_t> &description, uint8_t charset, bool fullAccess) override
    {
        callCountOnPasswordRequired++;
    }
    void OnActionCompleted(
        const RawAddress &device, int respCode, int actionType, const IPbapPhoneBookData &result) override
    {
        callCountOnActionCompleted++;
    }

    void ResetCallCount()
    {
        callCountOnConnected = 0;
        callCountOnActionCompleted = 0;
        callCountOnDisconnected = 0;
        callCountOnPasswordRequired = 0;
    }
    int callCountOnConnected{0};
    int callCountOnActionCompleted{0};
    int callCountOnDisconnected{0};
    int callCountOnPasswordRequired{0};
};

std::unique_ptr<bluetooth::ObexMpServer> StartTestObexMpServer(
    ObexServerObserver &observer, utility::Dispatcher &dispatcher)
{
    ObexServerConfig config;
    config.useRfcomm_ = true;
    config.rfcommScn_ = 15;
    config.rfcommMtu_ = 300;
    config.useL2cap_ = false;
    config.l2capPsm_ = 456;
    config.l2capMtu_ = 500;
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;

    auto obexServer = std::make_unique<ObexMpServer>("test server", config, observer, dispatcher);
    int ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, 0);

    return obexServer;
}

class TestBaseObexServerObserverImpl : public ObexServerObserver {
public:
    ~TestBaseObexServerObserverImpl()
    {}

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {
        auto response = req.GetItemAuthResponse();
        std::unique_ptr<ObexHeader> header;
        if (response) {
            header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
        } else {
            header = ObexHeader::CreateResponse(ObexRspCode::UNAUTHORIZED, true);
            std::vector<uint8_t> nonce = ObexUtils::MakeNonce(1111);
            ObexDigestChallenge challenge;
            challenge.AppendNonce(&nonce[0], nonce.size());

            // add option
            challenge.AppendOptions(0x01 | 0x02);
            // add realm
            uint8_t realm[11];
            memset_s(realm, sizeof(realm), 0x00, sizeof(realm));
            // charset UNICODE
            realm[0] = 0xFF;
            std::u16string descr = u"descr";

            memcpy_s(realm + 1, sizeof(realm) - 1, descr.data(), sizeof(realm) - 1);
            challenge.AppendRealm(realm, sizeof(realm));

            header->AppendItemAuthChallenges(challenge);
        }

        session.SendResponse(*header);
    }
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {
        auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
        session.SendResponse(*header);
    }
};

class TestBadObexServerObserverImpl : public ObexServerObserver {
public:
    ~TestBadObexServerObserverImpl()
    {}

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {
        auto response = req.GetItemAuthResponse();
        std::unique_ptr<ObexHeader> header;
        if (response) {
            header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
        } else {
            header = ObexHeader::CreateResponse(ObexRspCode::BAD_REQUEST, true);
        }

        session.SendResponse(*header);
    }
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {
        auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
        session.SendResponse(*header);
    }
};

void CloseClient(bluetooth::PbapPceService &client, ContextCallback &callBackForClient)
{
    std::cout << "CloseClient start" << std::endl;
    callBackForClient.Reset();
    client.Disable();
    std::cout << "client.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForClient.callCountOnDisable_ == 1; })) {
        return;
    };
    std::cout << "CloseClient end" << std::endl;
}
}  // namespace pbap_pce_legacy_test

#define CLOSE_LEGACY_PCE_SERVER()                                      \
    [&]() {                                                            \
        pbap_pce_legacy_test::CloseClient(client, *callBackForClient); \
        sleep(3);                                                      \
        obexServer->Shutdown();                                        \
        sleep(3);                                                      \
    }

TEST(pbap_pce_legacy_test, Connect_to_legacy_pce)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    // bt enable
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_legacy_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_legacy_test::TestBaseObexServerObserverImpl>();
    auto observerForClient = std::make_unique<pbap_pce_legacy_test::TestPbapObserverForClientImpl>();
    auto obexServer = pbap_pce_legacy_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(callBackForClient->callCountOnEnable_ == 1, CLOSE_LEGACY_PCE_SERVER());

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnPasswordRequired == 1, CLOSE_LEGACY_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnPasswordRequired, 1);
    std::string pw = "123abc";
    client.SetDevicePassword(RawAddress("00:00:01:00:00:7F"), pw, "uid1");
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_LEGACY_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_LEGACY_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);

    auto close_test = CLOSE_LEGACY_PCE_SERVER();
    close_test();
}

TEST(pbap_pce_legacy_test, Connect_to_legacy_pce_bad_response)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto observerForClient = std::make_unique<pbap_pce_legacy_test::TestPbapObserverForClientImpl>();
    auto observerForServer = std::make_unique<pbap_pce_legacy_test::TestBadObexServerObserverImpl>();
    auto callBackForClient = std::make_unique<pbap_pce_legacy_test::ContextCallback>();
    auto obexServer = pbap_pce_legacy_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);

    PbapPceService client;
    client.RegisterObserver(*observerForClient);
    client.RegisterCallback(*callBackForClient);
    client.Enable();
    WAIT_FOR(callBackForClient->callCountOnEnable_ == 1, CLOSE_LEGACY_PCE_SERVER());

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_LEGACY_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnPasswordRequired, 0);
    EXPECT_THAT(observerForClient->callCountOnConnected, 0);
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);

    auto close_test = CLOSE_LEGACY_PCE_SERVER();
    close_test();
}