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

#include <memory>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "data_test_util.h"
#include "message.h"
#include "obex/obex_utils.h"
#include "obex/obex_client.h"
#include "pbap_pce_service.h"
#include "pbap_pse_service.h"
#include "socket_transport_mock.h"
#include "securec.h"

using namespace testing;
using namespace bluetooth;
namespace pbap_pse_legacy_test {
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

class TestPbapPseObserverImpl : public IPbapPseObserver {
public:
    TestPbapPseObserverImpl()
    {}

    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pse test remoteAddr:" << remoteAddr.GetAddress() << " state:" << state << std::endl;
        switch (state) {
            case static_cast<int>(BTConnectState::CONNECTING):
                break;
            case static_cast<int>(BTConnectState::CONNECTED):
                callCountOnServiceConnected++;
                break;
            case static_cast<int>(BTConnectState::DISCONNECTING):
                break;
            case static_cast<int>(BTConnectState::DISCONNECTED):
                callCountOnServiceDisconnected++;
                break;
            default:
                break;
        }
    }
    void OnServicePermission(const RawAddress &remoteAddr) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pbap_pse_legacy_test remoteAddr:" << remoteAddr.GetAddress() << std::endl;
        callCountOnServicePermission++;
    }

    void OnServicePasswordRequired(const RawAddress &remoteAddr, const std::vector<uint8_t> &description,
        uint8_t charset, bool fullAccess) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pbap_pse_legacy_test remoteAddr:" << remoteAddr.GetAddress() << std::endl;
        description_ = description;
        charset_ = charset;
        fullAccess_ = fullAccess;
        callCountOnPasswordRequired++;
    }

    void ResetCallCount()
    {
        callCountOnServiceConnected = 0;
        callCountOnServiceDisconnected = 0;
        callCountOnServicePermission = 0;
        callCountOnPasswordRequired = 0;
    }
    std::vector<uint8_t> description_{};
    uint8_t charset_{0};
    bool fullAccess_{false};
    int callCountOnServiceConnected{0};
    int callCountOnServiceDisconnected{0};
    int callCountOnServicePermission{0};
    int callCountOnPasswordRequired{0};
};

class TestObexClientObserver : public ObexClientObserver {
public:
    void OnTransportFailed(ObexClient &client, int errCd) override
    {
        std::cout << "pbap_pse_legacy_test TestObexClientObserver::OnTransportFailed" << std::endl;
        lastRespHeader_ = nullptr;
        callCountOnTransportFailed++;
    }
    void OnConnected(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "pbap_pse_legacy_test TestObexClientObserver::OnConnected" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnected++;
    }
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "pbap_pse_legacy_test TestObexClientObserver::OnConnectFailed" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnectFailed++;
    }
    void OnDisconnected(ObexClient &client) override
    {
        std::cout << "pbap_pse_legacy_test TestObexClientObserver::OnDisconnected" << std::endl;
        callCountOnDisconnected++;
    }
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "pbap_pse_legacy_test TestObexClientObserver::OnActionCompleted" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnActionCompleted++;
    }
    void ResetCallCount()
    {
        callCountOnTransportFailed = 0;
        callCountOnConnected = 0;
        callCountOnConnectFailed = 0;
        callCountOnDisconnected = 0;
        callCountOnActionCompleted = 0;
    }

    int callCountOnTransportFailed{0};
    int callCountOnConnected{0};
    int callCountOnConnectFailed{0};
    int callCountOnDisconnected{0};
    int callCountOnActionCompleted{0};
    std::unique_ptr<ObexHeader> lastRespHeader_ = nullptr;
};

std::unique_ptr<bluetooth::ObexClient> StartTestLegacyObexClient(
    ObexClientObserver &observer, utility::Dispatcher &dispatcher)
{
    uint8_t ip[4] = {127, 0, 0, 1};  // 127.0.0.1
    static constexpr uint8_t SERVICE_UUID[16] = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexClientConfig configRfcomm;
    memcpy_s(configRfcomm.addr_.addr, sizeof(configRfcomm.addr_.addr), ip, sizeof(ip));
    memcpy_s(configRfcomm.serviceUUID_.uuid128, sizeof(configRfcomm.serviceUUID_.uuid128), SERVICE_UUID, 16);

    configRfcomm.scn_ = 15;
    configRfcomm.mtu_ = 400;
    configRfcomm.isSupportReliableSession_ = false;
    configRfcomm.isSupportSrm_ = false;
    configRfcomm.isGoepL2capPSM_ = false;

    auto obexClient = std::make_unique<ObexClient>(configRfcomm, observer, dispatcher);
    std::cout << "StartTestLegacyObexClient:Rfcomm" << std::endl;
    return obexClient;
}

std::unique_ptr<bluetooth::ObexClient> StartTestWrongObexClient(
    ObexClientObserver &observer, utility::Dispatcher &dispatcher)
{
    uint8_t ip[4] = {127, 0, 0, 1};  // 127.0.0.1
    static constexpr uint8_t SERVICE_UUID[16] = {
        0x00, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexClientConfig configRfcomm;
    memcpy_s(configRfcomm.addr_.addr, sizeof(configRfcomm.addr_.addr), ip, sizeof(ip));
    memcpy_s(configRfcomm.serviceUUID_.uuid128, sizeof(configRfcomm.serviceUUID_.uuid128), SERVICE_UUID, 16);

    configRfcomm.scn_ = 15;
    configRfcomm.mtu_ = 400;
    configRfcomm.isSupportReliableSession_ = false;
    configRfcomm.isSupportSrm_ = false;
    configRfcomm.isGoepL2capPSM_ = false;

    auto obexClient = std::make_unique<ObexClient>(configRfcomm, observer, dispatcher);
    std::cout << "StartTestWrongObexClient:Rfcomm" << std::endl;
    return obexClient;
}

void CloseServer(PbapPseService &server, ContextCallback &callBackForServer)
{
    callBackForServer.Reset();
    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.callCountOnDisable_ == 1; })) {
        return;
    };
}
}  // namespace pbap_pse_legacy_test

TEST(pbap_pse_legacy_test, connect_with_wrong_target)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_legacy_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_legacy_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.Enable();
    auto observerForClient = std::make_unique<pbap_pse_legacy_test::TestObexClientObserver>();
    auto obexClient = pbap_pse_legacy_test::StartTestWrongObexClient(*observerForClient, *clientDispatcher);
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    int ret = obexClient->Connect();
    EXPECT_THAT(ret, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnectFailed == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    auto &lastRespHeader = observerForClient->lastRespHeader_;
    EXPECT_THAT(lastRespHeader != nullptr, true);
    if (lastRespHeader) {
        EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xC6);
    }

    pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_legacy_test, connected_but_transport_down)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_legacy_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_legacy_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.Enable();
    auto observerForClient = std::make_unique<pbap_pse_legacy_test::TestObexClientObserver>();
    auto obexClient = pbap_pse_legacy_test::StartTestLegacyObexClient(*observerForClient, *clientDispatcher);
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    int ret = obexClient->Connect();
    EXPECT_THAT(ret, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }

    auto &lastRespHeader = observerForClient->lastRespHeader_;
    EXPECT_THAT(lastRespHeader != nullptr, true);
    if (lastRespHeader) {
        EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);  // succeeded
    }

    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();

    auto reqPut = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    obexClient->Put(*reqPut);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(lastRespHeader != nullptr, true);
    if (lastRespHeader) {
        EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xC0);  // BAD_REQUEST
    }

    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    EXPECT_THAT(bluetooth::MockSockTransport::CloseClient(), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServiceDisconnected == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_legacy_test, connect_with_challenge_nonce)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    std::string str = "123abc";

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_legacy_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_legacy_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.Enable();
    auto observerForClient = std::make_unique<pbap_pse_legacy_test::TestObexClientObserver>();
    auto obexClient = pbap_pse_legacy_test::StartTestLegacyObexClient(*observerForClient, *clientDispatcher);
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    {
        // has authChallenges but no nonce
        ObexConnectParams connectParams;
        ObexDigestChallenge authChallenges;
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        connectParams.authChallenges_ = &authChallenges;
        int ret = obexClient->Connect(connectParams);
        EXPECT_THAT(ret, 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnectFailed == 1; })) {
            pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);
    }
    {
        ObexConnectParams connectParams;
        ObexDigestChallenge authChallenges;
        std::default_random_engine e(time(0));
        uint32_t privateKey = static_cast<uint32_t>(e());
        auto nonce = ObexUtils::MakeNonce(privateKey);
        // add nonce
        authChallenges.AppendNonce(nonce.data(), nonce.size());
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        connectParams.authChallenges_ = &authChallenges;
        int ret = obexClient->Connect(connectParams);
        EXPECT_THAT(ret, 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnPasswordRequired == 1; })) {
            pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->callCountOnPasswordRequired, 1);
        server.SetDevicePassword(RawAddress("00:00:01:00:00:7F"), str);
        sleep(2);
        auto &lastRespHeader = observerForClient->lastRespHeader_;
        EXPECT_NE(lastRespHeader, nullptr);
        EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);  // succeeded
        auto authResponse = lastRespHeader->GetItemAuthResponse();

        auto requestDigestTlv = authResponse->GetTlvtriplet(0x00);
        EXPECT_THAT(requestDigestTlv->GetLen(), 16);
        std::vector<uint8_t> actualRdVector(
            requestDigestTlv->GetVal(), requestDigestTlv->GetVal() + requestDigestTlv->GetLen());
        std::vector<uint8_t> exceptRdVector = ObexUtils::MakeRequestDigest(nonce.data(), nonce.size(), str);
        EXPECT_THAT(actualRdVector, ElementsAreArray(exceptRdVector));

        auto userIdTlv = authResponse->GetTlvtriplet(0x01);
        EXPECT_THAT(userIdTlv, nullptr);

        auto nonceTlv = authResponse->GetTlvtriplet(0x02);
        EXPECT_NE(nonceTlv, nullptr);
        std::vector<uint8_t> exceptNonceVector = ObexUtils::MakeRequestDigest(nonce.data(), nonce.size(), str);
        std::vector<uint8_t> actualNonceVector(nonceTlv->GetVal(), nonceTlv->GetVal() + nonceTlv->GetLen());
        EXPECT_THAT(actualNonceVector, ElementsAreArray(nonce));
    }
    pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_legacy_test, connect_with_challenge_nonce_option_realm)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    std::string userId = "userId";
    std::string str = "abc123";
    RawAddress rawAddress("00:00:01:00:00:7F");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_legacy_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_legacy_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.Enable();
    auto observerForClient = std::make_unique<pbap_pse_legacy_test::TestObexClientObserver>();
    auto obexClient = pbap_pse_legacy_test::StartTestLegacyObexClient(*observerForClient, *clientDispatcher);
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    ObexConnectParams connectParams;
    ObexDigestChallenge authChallenges;
    std::default_random_engine e(time(0));
    uint32_t privateKey = static_cast<uint32_t>(e());
    auto nonce = ObexUtils::MakeNonce(privateKey);
    // add nonce
    authChallenges.AppendNonce(nonce.data(), nonce.size());
    // add option
    authChallenges.AppendOptions(0x01 | 0x02);
    // add realm
    uint8_t realm[11];
    memset_s(realm, sizeof(realm), 0x00, sizeof(realm));
    // charset UNICODE
    realm[0] = 0xFF;
    std::u16string descr = u"descr";
    memcpy_s(realm + 1, sizeof(realm) - 1, descr.data(), descr.size() * 2);
    if (!ObexUtils::SysIsBigEndian()) {
        ObexUtils::DataReverse(&realm[1], 10, 2);
    }
    authChallenges.AppendRealm(realm, sizeof(realm));

    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    connectParams.authChallenges_ = &authChallenges;
    int ret = obexClient->Connect(connectParams);
    EXPECT_THAT(ret, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnPasswordRequired == 1; })) {
        pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnPasswordRequired, 1);

    EXPECT_THAT(observerForServer->charset_, 0xFF);
    std::vector<uint8_t> exceptDescrVector(realm + 1, realm + sizeof(realm));
    EXPECT_THAT(observerForServer->description_, ElementsAreArray(exceptDescrVector));
    EXPECT_THAT(observerForServer->fullAccess_, false);

    server.SetDevicePassword(rawAddress, str, userId);
    sleep(2);
    auto &lastRespHeader = observerForClient->lastRespHeader_;
    EXPECT_NE(lastRespHeader, nullptr);
    EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);  // succeeded
    auto authResponse = lastRespHeader->GetItemAuthResponse();

    auto requestDigestTlv = authResponse->GetTlvtriplet(0x00);
    EXPECT_THAT(requestDigestTlv->GetLen(), 16);
    std::vector<uint8_t> actualRdVector(
        requestDigestTlv->GetVal(), requestDigestTlv->GetVal() + requestDigestTlv->GetLen());
    std::vector<uint8_t> exceptRdVector = ObexUtils::MakeRequestDigest(nonce.data(), nonce.size(), str);
    EXPECT_THAT(actualRdVector, ElementsAreArray(exceptRdVector));

    auto userIdTlv = authResponse->GetTlvtriplet(0x01);
    EXPECT_NE(userIdTlv, nullptr);
    EXPECT_THAT(userIdTlv->GetLen(), userId.size());
    EXPECT_THAT(std::string(userIdTlv->GetVal(), userIdTlv->GetVal() + userIdTlv->GetLen()), userId);

    auto nonceTlv = authResponse->GetTlvtriplet(0x02);
    EXPECT_NE(nonceTlv, nullptr);
    std::vector<uint8_t> exceptNonceVector = ObexUtils::MakeRequestDigest(nonce.data(), nonce.size(), str);
    std::vector<uint8_t> actualNonceVector(nonceTlv->GetVal(), nonceTlv->GetVal() + nonceTlv->GetLen());
    EXPECT_THAT(actualNonceVector, ElementsAreArray(nonce));

    // wrong password for coverage
    EXPECT_THAT(server.SetDevicePassword(rawAddress, "", userId), RET_BAD_PARAM);
    // wrong userId for coverage
    EXPECT_THAT(server.SetDevicePassword(rawAddress, str, "123456789012345678901"), RET_BAD_PARAM);
    // not exists device for coverage
    EXPECT_THAT(server.SetDevicePassword(RawAddress("00:00:01:00:00:7B"), str, userId), RET_BAD_STATUS);
    // wrong device's state for coverage
    EXPECT_THAT(server.SetDevicePassword(rawAddress, str, userId), RET_BAD_STATUS);
    sleep(2);
    pbap_pse_legacy_test::CloseServer(server, *callBackForServer);
}