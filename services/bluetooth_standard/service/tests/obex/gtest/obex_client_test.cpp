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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "data_test_util.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "securec.h"

using namespace std;
using namespace testing;
using namespace bluetooth;
static const uint32_t TEST_CONNECTION_ID_1234 = 1234;
static const uint32_t TEST_SLEEP_SEC_2 = 2;
static const uint32_t TEST_SLEEP_SEC_4 = 4;

namespace obex_client_test {
class TestObexClientObserver : public ObexClientObserver {
public:
    struct Info {
        int callCountOnTransportFailed_{0};
        int callCountOnConnected_{0};
        int callCountOnConnectFailed_{0};
        int callCountOnDisconnected_{0};
        int callCountOnActionCompleted_{0};
        std::unique_ptr<ObexHeader> lastRespHeader_{nullptr};
        int errCd_ = 0xFFFF;
    };
    TestObexClientObserver() = default;
    virtual ~TestObexClientObserver() = default;
    void OnTransportFailed(ObexClient &client, int errCd) override
    {
        std::cout << "TestObexClientObserver::OnTransportFailed" << std::endl;
        data_.lastRespHeader_ = nullptr;
        data_.errCd_ = errCd;
        data_.callCountOnTransportFailed_++;
    }
    void OnConnected(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnected" << std::endl;
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnConnected_++;
    }
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnectFailed" << std::endl;
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnConnectFailed_++;
    }
    void OnDisconnected(ObexClient &client) override
    {
        std::cout << "TestObexClientObserver::OnDisconnected" << std::endl;
        data_.lastRespHeader_ = nullptr;
        data_.callCountOnDisconnected_++;
    }
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnActionCompleted" << std::endl;
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnActionCompleted_++;
    }
    void ResetCallCount()
    {
        data_.callCountOnTransportFailed_ = 0;
        data_.callCountOnConnected_ = 0;
        data_.callCountOnConnectFailed_ = 0;
        data_.callCountOnDisconnected_ = 0;
        data_.callCountOnActionCompleted_ = 0;
        data_.errCd_ = 0xFFFF;
    }

    Info &GetData()
    {
        return data_;
    }

private:
    Info data_{};
    DISALLOW_COPY_AND_ASSIGN(TestObexClientObserver);
};

class TestObexServerObserverDummy : public ObexServerObserver {
public:
    struct Info {
        std::function<void(ObexIncomingConnect &incomingConnect)> transportConnectHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> connectHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> disconnectHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> putHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> getHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> abortHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> setpathHandler_{nullptr};
        int callCountTransportConnect_{0};
        int callCountTransportConnected_{0};
        int callCountTransportDisconnected_{0};
        int callCountTransportError_{0};
        int callCountError_{0};
        int callCountConnect_{0};
        int callCountDisconnect_{0};
        int callCountPut_{0};
        int callCountGet_{0};
        int callCountAbort_{0};
        int callCountSetPath_{0};
        int callCountAction_{0};
        int callCountSession_{0};
        ObexIncomingConnect *lastIncomingConnect_ = nullptr;
        ObexServerSession *lastServerSession_ = nullptr;
    };
    TestObexServerObserverDummy() = default;
    virtual ~TestObexServerObserverDummy() = default;

    void OnTransportConnect(ObexIncomingConnect &incomingConnect) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.transportConnectHandler_) {
            data_.transportConnectHandler_(incomingConnect);
        } else {
            incomingConnect.AcceptConnection();
        }
        data_.lastIncomingConnect_ = &incomingConnect;
        data_.callCountTransportConnect_++;
    }
    void OnTransportConnected(const std::string &btAddr) override
    {
        cout << __PRETTY_FUNCTION__ << endl;

        data_.callCountTransportConnected_++;
    }
    void OnTransportDisconnected(const std::string &btAddr) override
    {
        cout << __PRETTY_FUNCTION__ << endl;

        data_.callCountTransportDisconnected_++;
    }
    void OnTransportError(const std::string &btAddr, int errCd, const std::string &msg) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        data_.callCountTransportError_++;
    }

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.connectHandler_) {
            data_.connectHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
            header->AppendItemConnectionId(TEST_CONNECTION_ID_1234);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendConnectResponse:" << ret << endl;
        }
        data_.lastServerSession_ = &session;
        data_.callCountConnect_++;
    }
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.disconnectHandler_) {
            data_.disconnectHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendDisconnectResponse:" << ret << endl;
        }
        data_.callCountDisconnect_++;
    }
    void OnError(const int errCd, const std::string &msg) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        data_.callCountError_++;
    }
    void OnPut(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.putHandler_) {
            data_.putHandler_(session, req);
        } else {
            ObexRspCode rspCode = ObexRspCode::NOT_ACCEPTABLE;
            if (req.GetFieldCode() == 0x02) {
                rspCode = ObexRspCode::CONTINUE;
            } else if (req.GetFieldCode() == 0x82) {
                rspCode = ObexRspCode::SUCCESS;
            }
            auto header = ObexHeader::CreateResponse(rspCode, false);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendPutResponse:" << ret << endl;
        }
        data_.callCountPut_++;
    }
    void OnGet(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.getHandler_) {
            data_.getHandler_(session, req);
        } else {
            ObexRspCode rspCode = ObexRspCode::NOT_ACCEPTABLE;
            if (req.GetFieldCode() == 0x03) {
                rspCode = ObexRspCode::CONTINUE;
            } else if (req.GetFieldCode() == 0x83) {
                rspCode = ObexRspCode::SUCCESS;
            }
            auto header = ObexHeader::CreateResponse(rspCode, false);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendGetResponse:" << ret << endl;
        }
        data_.callCountGet_++;
    }
    void OnAbort(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.abortHandler_) {
            data_.abortHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendAbortResponse:" << ret << endl;
        }
        data_.callCountAbort_++;
    }
    void OnSetPath(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.setpathHandler_) {
            data_.setpathHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
            int ret = session.SendResponse(*header);
            cout << "TestObexServerObserverDummy:SendSetPathResponse:" << ret << endl;
        }
        data_.callCountSetPath_++;
    }
    void OnAction(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;

        data_.callCountAction_++;
    }
    void OnSession(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;

        data_.callCountSession_++;
    }
    void ResetCallCount()
    {
        data_.callCountTransportConnect_ = 0;
        data_.callCountTransportConnected_ = 0;
        data_.callCountTransportDisconnected_ = 0;
        data_.callCountTransportError_ = 0;

        data_.callCountError_ = 0;
        data_.callCountConnect_ = 0;
        data_.callCountDisconnect_ = 0;
        data_.callCountPut_ = 0;
        data_.callCountGet_ = 0;
        data_.callCountAbort_ = 0;
        data_.callCountSetPath_ = 0;
        data_.callCountAction_ = 0;
        data_.callCountSession_ = 0;
    }

    Info &GetData()
    {
        return data_;
    }

private:
    Info data_{};
    DISALLOW_COPY_AND_ASSIGN(TestObexServerObserverDummy);
};

class TestBaseObexServerObserver : public ObexServerObserver {
public:
    struct Info {
        std::function<void(ObexServerSession &session, const ObexHeader &req)> connectHandler_{nullptr};
        std::function<void(ObexServerSession &session, const ObexHeader &req)> disconnectHandler_{nullptr};
        int callCountConnect_{0};
        int callCountDisconnect_{0};
        ObexServerSession *lastServerSession_{nullptr};
    };
    TestBaseObexServerObserver() = default;
    virtual ~TestBaseObexServerObserver() = default;

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        data_.lastServerSession_ = &session;
        if (data_.connectHandler_) {
            data_.connectHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
            int ret = session.SendResponse(*header);
            cout << "TestBaseObexServerObserver:SendConnectResponse:" << ret << endl;
        }
        data_.callCountConnect_++;
    }
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.disconnectHandler_) {
            data_.disconnectHandler_(session, req);
        } else {
            auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
            int ret = session.SendResponse(*header);
            cout << "TestBaseObexServerObserver:SendDisconnectResponse:" << ret << endl;
        }
        data_.callCountDisconnect_++;
    }

    void ResetCallCount()
    {
        data_.callCountConnect_ = 0;
        data_.callCountDisconnect_ = 0;
    }

    Info &GetData()
    {
        return data_;
    }

private:
    Info data_{};
    DISALLOW_COPY_AND_ASSIGN(TestBaseObexServerObserver);
};

void CloseClientAndServer(bluetooth::ObexServer *server, bluetooth::ObexClient *client)
{
    client->Disconnect();
    sleep(TEST_SLEEP_SEC_2);
    server->Shutdown();
    sleep(TEST_SLEEP_SEC_2);
}

void CloseServerAndClient(bluetooth::ObexServer *server, bluetooth::ObexClient *client)
{
    server->Shutdown();
    sleep(TEST_SLEEP_SEC_4);
    client->Disconnect(false);
    sleep(TEST_SLEEP_SEC_2);
}

std::unique_ptr<bluetooth::ObexServer> StartTestObexServer(
    ObexServerObserver &observer, utility::Dispatcher &dispatcher)
{
    ObexServerConfig config;
    config.useRfcomm_ = true;
    config.rfcommScn_ = 0xEA;    // 234
    config.rfcommMtu_ = 0x012C;  // 300
    config.useL2cap_ = true;
    config.l2capPsm_ = 0x159;   // 345
    config.l2capMtu_ = 0x01F4;  // 500
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;

    auto obexServer = std::make_unique<ObexServer>("test server", config, observer, dispatcher);
    int ret = obexServer->Startup();
    sleep(TEST_SLEEP_SEC_2);
    EXPECT_THAT(ret, 0);

    cout << "StartTestObexServer:startup:" << ret << endl;
    return obexServer;
}

std::unique_ptr<bluetooth::ObexClient> StartTestObexClient(
    ObexClientObserver &observer, utility::Dispatcher &dispatcher)
{
    uint8_t ip[4] = {127, 0, 0, 1};  // 127.0.0.1
    static constexpr uint8_t SERVICE_UUID[16] = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexClientConfig configRfcomm;
    memcpy_s(configRfcomm.addr_.addr, sizeof(configRfcomm.addr_.addr), ip, sizeof(ip));
    memcpy_s(configRfcomm.serviceUUID_.uuid128,
        sizeof(configRfcomm.serviceUUID_.uuid128),
        SERVICE_UUID,
        sizeof(SERVICE_UUID));

    configRfcomm.scn_ = 0xEA;    // 234
    configRfcomm.mtu_ = 0x0190;  // 400
    configRfcomm.isSupportReliableSession_ = false;
    configRfcomm.isSupportSrm_ = false;
    configRfcomm.isGoepL2capPSM_ = false;

    auto obexClient = std::make_unique<ObexClient>(configRfcomm, observer, dispatcher);
    cout << "StartTestObexClient:Rfcomm" << endl;
    return obexClient;
}
}  // namespace obex_client_test

TEST(ObexClient_test, ConnectWithBaseObexServerObserver)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x80);
    EXPECT_THAT(session.GetMaxPacketLength(), 300);
    auto &lastReqHeader = session.GetLastReqHeader();
    auto &lastRespHeader = observerForClient->GetData().lastRespHeader_;
    EXPECT_THAT(lastReqHeader->GetFieldCode(), 0x80);
    EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);

    // FOR Session invalid response
    auto serverSession = observerForServer->GetData().lastServerSession_;
    {
        auto obexHeader = ObexHeader::CreateResponse(ObexRspCode::BAD_REQUEST);
        auto dummyBody = std::make_unique<uint8_t[]>(serverSession->GetMaxPacketLength() + 1);
        obexHeader->AppendItemBody(dummyBody.get(), serverSession->GetMaxPacketLength() + 1);
        // length > mtu
        EXPECT_THAT(serverSession->SendResponse(*obexHeader), -1);
    }

    // DISCONNECT TEST
    obex_client_test::CloseServerAndClient(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, PutAndPutFinalWithBaseObexServerObserver)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // PUT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
    int putRet = obexClientRfcomm->Put(*putHeader);              // Send obex put request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x02);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x02);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);
    }

    // PUT_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);  // Set put_final header
    putRet = obexClientRfcomm->Put(*putHeader);                   // Send obex put_final request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x82);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);
    }

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, GetAndGetFinalWithBaseObexServerObserver)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // GET TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET);  // Set get header
    int getRet = obexClientRfcomm->Get(*getHeader);              // Send obex get request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x03);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x03);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);

    // GET_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get_final header
    getRet = obexClientRfcomm->Get(*getHeader);                   // Send obex get_final request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x83);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x83);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);
    }

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, AbortWithBaseObexServerObserver)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // Abort TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int abortRet = obexClientRfcomm->Abort();  // Send obex abort request
    EXPECT_THAT(abortRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0xFF);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0xFF);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, SetPathWithBaseObexServerObserver)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // SETPATH TEST With path information
    {
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        int setpathRet = obexClientRfcomm->SetPath(OBEX_SETPATH_NOCREATE, u"aaaa/bbbb");
        EXPECT_THAT(setpathRet, -1);
    }

    // SETPATH TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int setpathRet = obexClientRfcomm->SetPath(OBEX_SETPATH_NOCREATE, u"");  // Send obex setpath request
    EXPECT_THAT(setpathRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x85);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x85);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, ActionAndSessionWithTODO)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestBaseObexServerObserver>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // Action TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    const std::u16string srcName = u"";
    const std::u16string destName = u"";
    int ret = obexClientRfcomm->Copy(srcName, destName);
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->Move(srcName, destName);
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->SetPermissions(srcName, (uint32_t)111);
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->CreateSession();
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->SuspendSession();
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->ResumeSession();
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->CloseSession();
    EXPECT_THAT(ret, 0);
    ret = obexClientRfcomm->SetSessionTimeout(OBEX_SESSION_MAX_TIMEOUT_SEC);
    EXPECT_THAT(ret, 0);
    {
        // SESSION REQUEST TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        {
            auto sessionHeader = ObexHeader::CreateRequest(ObexOpeId::SESSION);
            EXPECT_THAT(obexClientRfcomm->SendRequest(*sessionHeader), -1);
            ObexSessionParameters params;
            sessionHeader->AppendItemSessionParams(params);
            EXPECT_THAT(obexClientRfcomm->SendRequest(*sessionHeader), -1);
        }

        auto sessionHeader = ObexHeader::CreateRequest(ObexOpeId::SESSION);
        ObexSessionParameters params;
        params.AppendSessionOpcode(ObexSessionParameters::SessionOpcode::CREATE);
        sessionHeader->AppendItemSessionParams(params);
        EXPECT_THAT(obexClientRfcomm->SendRequest(*sessionHeader), 0);

        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
            obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);
    }
    {
        // ACTION REQUEST TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto actionHeader = ObexHeader::CreateRequest(ObexOpeId::ACTION);

        EXPECT_THAT(obexClientRfcomm->SendRequest(*actionHeader), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted_ == 1; })) {
            obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xD1);
    }
    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, Connect)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountError_, 0);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x80);
    EXPECT_THAT(session.GetMaxPacketLength(), 300);
    EXPECT_THAT(session.GetConnectId(), TEST_CONNECTION_ID_1234);
    auto &lastReqHeader = session.GetLastReqHeader();
    auto &lastRespHeader = observerForClient->GetData().lastRespHeader_;
    EXPECT_THAT(lastReqHeader->GetFieldCode(), 0x80);
    EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);
    auto connectId = lastRespHeader->GetItemConnectionId();
    EXPECT_NE(connectId, nullptr);
    EXPECT_THAT(connectId->GetWord(), TEST_CONNECTION_ID_1234);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, ConnectWithParam)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    ObexConnectParams connectParams;
    ObexTlvParamters appParams;
    TlvTriplet item1(0x01, (uint16_t)0xFFFF);
    appParams.AppendTlvtriplet(item1);
    connectParams.appParams_ = &appParams;
    int connRet = obexClientRfcomm->Connect(connectParams);
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x80);
    EXPECT_THAT(session.GetMaxPacketLength(), 300);
    EXPECT_THAT(session.GetConnectId(), TEST_CONNECTION_ID_1234);
    auto &lastReqHeader = session.GetLastReqHeader();
    auto &lastRespHeader = observerForClient->GetData().lastRespHeader_;
    EXPECT_THAT(lastReqHeader->GetFieldCode(), 0x80);
    EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);
    auto connectId = lastRespHeader->GetItemConnectionId();
    EXPECT_NE(connectId, nullptr);
    EXPECT_THAT(connectId->GetWord(), TEST_CONNECTION_ID_1234);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, ConnectWithParams)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    ObexConnectParams connectParams;
    ObexTlvParamters appParams;
    TlvTriplet item1(0x01, (uint16_t)0xFFFF);
    appParams.AppendTlvtriplet(item1);
    ObexDigestChallenge authChallenges;
    authChallenges.AppendNonce((const uint8_t *)"NONCE1234567890A", 16);
    authChallenges.AppendOptions(uint8_t(0x00));
    authChallenges.AppendRealm((const uint8_t *)"REALM", 5);
    ObexDigestResponse authResponses;
    authResponses.AppendRequestDigest((const uint8_t *)"REQUEST_DIGEST", 14);
    authResponses.AppendUserId((const uint8_t *)"userid", 6);
    authResponses.AppendNonce((const uint8_t *)"NONCE1234567890A", 16);
    connectParams.appParams_ = &appParams;
    connectParams.authChallenges_ = &authChallenges;
    connectParams.authResponses_ = &authResponses;
    int connRet = obexClientRfcomm->Connect(connectParams);
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x80);
    EXPECT_THAT(session.GetMaxPacketLength(), 300);
    EXPECT_THAT(session.GetConnectId(), TEST_CONNECTION_ID_1234);
    auto &lastReqHeader = session.GetLastReqHeader();
    auto &lastRespHeader = observerForClient->GetData().lastRespHeader_;
    EXPECT_THAT(lastReqHeader->GetFieldCode(), 0x80);
    EXPECT_THAT(lastRespHeader->GetFieldCode(), 0xA0);
    auto connectId = lastRespHeader->GetItemConnectionId();
    EXPECT_NE(connectId, nullptr);
    EXPECT_THAT(connectId->GetWord(), TEST_CONNECTION_ID_1234);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, PutAndPutFinal)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // PUT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    uint8_t body[500];
    for (int i = 0; i < 500; i++) {
        body[i] = i % 256;
    }
    auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
    putHeader->AppendItemBody(body, 294);
    int putRet = obexClientRfcomm->Put(*putHeader);  // Send obex put request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountPut_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x02);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x02);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0x90);
    }

    // PUT_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);  // Set put_final header
    putHeader->AppendItemEndBody(body + 294, 500 - 294);
    putRet = obexClientRfcomm->Put(*putHeader);  // Send obex put_final request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountPut_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x82);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, GetAndGetFinal)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // GET TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET);  // Set get header
    int getRet = obexClientRfcomm->Get(*getHeader);              // Send obex get request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountGet_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x03);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x03);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0x90);
    }
    // GET_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get_final header
    getRet = obexClientRfcomm->Get(*getHeader);                   // Send obex get_final request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountGet_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
        const ObexClientSession &session = obexClientRfcomm->GetClientSession();
        EXPECT_THAT(session.GetLastOpeId(), 0x83);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x83);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, Abort)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // ABORT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int abortRet = obexClientRfcomm->Abort();  // Send obex abort request
    EXPECT_THAT(abortRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountAbort_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountAbort_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0xFF);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0xFF);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, SetPath)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 1);

    // SETPATH TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int setpathRet = obexClientRfcomm->SetPath(OBEX_SETPATH_NOCREATE, u"");  // Send obex setpath request
    EXPECT_THAT(setpathRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountSetPath_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted_ == 1);
        })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountSetPath_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted_, 1);
    const ObexClientSession &session = obexClientRfcomm->GetClientSession();
    EXPECT_THAT(session.GetLastOpeId(), 0x85);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x85);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

    // DISCONNECT TEST
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}

TEST(ObexClient_test, ConnectWhenServerReject)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_client_test::TestObexServerObserverDummy>();
    auto obexServer = obex_client_test::StartTestObexServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_client_test::TestObexClientObserver>();
    auto obexClientRfcomm = obex_client_test::StartTestObexClient(*observerForClient, *clientDispatcher);

    observerForServer->GetData().transportConnectHandler_ = [](ObexIncomingConnect &incomingConnect) {
        incomingConnect.RejectConnection();
    };
    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientRfcomm->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnTransportFailed_ == 1; })) {
        obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 0);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected_, 0);
    EXPECT_THAT(observerForClient->GetData().errCd_, 1);
    obex_client_test::CloseClientAndServer(obexServer.get(), obexClientRfcomm.get());
}