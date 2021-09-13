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
#include "log.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "securec.h"

using namespace std;
using namespace testing;
using namespace bluetooth;
static const uint32_t TEST_CONNECTION_ID_1234 = 1234;
static const uint32_t TEST_SLEEP_SEC_2 = 2;

namespace obex_mp_client_test {
class TestObexMpClientObserver : public ObexClientObserver {
public:
    struct Info {
        int callCountOnTransportFailed{0};
        int callCountOnConnected{0};
        int callCountOnConnectFailed{0};
        int callCountOnDisconnected{0};
        int callCountOnActionCompleted{0};
        std::unique_ptr<ObexHeader> lastRespHeader_{nullptr};
    };
    TestObexMpClientObserver() = default;
    virtual ~TestObexMpClientObserver() = default;

    void OnTransportFailed(ObexClient &client, int errCd) override
    {
        std::cout << "TestObexMpClientObserver::OnTransportFailed" << std::endl;
        data_.lastRespHeader_ = nullptr;
        data_.callCountOnTransportFailed++;
    }
    void OnConnected(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexMpClientObserver::OnConnected" << std::endl;
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnConnected++;
    }
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexMpClientObserver::OnConnectFailed" << std::endl;
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnConnectFailed++;
    }
    void OnDisconnected(ObexClient &client) override
    {
        std::cout << "TestObexMpClientObserver::OnDisconnected" << std::endl;
        data_.lastRespHeader_ = nullptr;
        data_.callCountOnDisconnected++;
    }
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override
    {
        int code = resp.GetFieldCode();
        std::cout << "TestObexMpClientObserver::OnActionCompleted :" << code << std::endl;
        LOG_DEBUG("TestObexMpClientObserver::OnActionCompleted : %02X", code);
        data_.lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        data_.callCountOnActionCompleted++;
    }
    void ResetCallCount()
    {
        data_.callCountOnTransportFailed = 0;
        data_.callCountOnConnected = 0;
        data_.callCountOnConnectFailed = 0;
        data_.callCountOnDisconnected = 0;
        data_.callCountOnActionCompleted = 0;
    }
    Info &GetData()
    {
        return data_;
    }

private:
    Info data_{};
    DISALLOW_COPY_AND_ASSIGN(TestObexMpClientObserver);
};

class TestObexMpServerObserverDummy : public ObexServerObserver {
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
        ObexIncomingConnect *lastIncomingConnect_{nullptr};
        ObexServerSession *lastServerSession_{nullptr};
    };
    TestObexMpServerObserverDummy() = default;
    virtual ~TestObexMpServerObserverDummy() = default;
    void OnTransportConnect(ObexIncomingConnect &incomingConnect) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.transportConnectHandler_) {
            data_.transportConnectHandler_(incomingConnect);
        } else {
            incomingConnect.AcceptConnection();
        }
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
            cout << "TestObexMpServerObserverDummy:SendConnectResponse:" << ret << endl;
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
            cout << "TestObexMpServerObserverDummy:SendDisconnectResponse:" << ret << endl;
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
            data_.callCountPut_++;
            return;
        }
        if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::PUT)) {
            auto headerBody = req.GetItemBody();
            if (headerBody != nullptr) {
                auto bodyBytes = headerBody->GetBytes();
                cout << "Receive Put body from client, size=" << (int)headerBody->GetHeaderDataSize() << ":\n"
                     << ObexUtils::ToDebugString(bodyBytes.get(), headerBody->GetHeaderDataSize(), false) << endl;
            }
            auto resp = ObexHeader::CreateResponse(ObexRspCode::CONTINUE);
            session.SendResponse(*resp);
        } else if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::PUT_FINAL)) {
            auto headerBody = req.GetItemEndBody();
            if (headerBody != nullptr) {
                auto bodyBytes = headerBody->GetBytes();
                cout << "Put end body size: " << headerBody->GetHeaderDataSize() << endl;
            }
            auto extendBody = req.GetExtendBodyObject();
            if (extendBody) {
                uint8_t buf[0x64];
                memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
                vector<uint8_t> results;
                size_t cnt = 0;
                while ((cnt = extendBody->Read(buf, 0x64)) > 0) {
                    results.insert(results.end(), buf, buf + cnt);
                }
                cout << "EXTEND_BODY size: " << results.size() << endl;
            }
            auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
            session.SendResponse(*resp);
        } else {
            session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
        }
        data_.callCountPut_++;
    }
    void OnGet(ObexServerSession &session, const ObexHeader &req) override
    {
        cout << __PRETTY_FUNCTION__ << endl;
        if (data_.getHandler_) {
            data_.getHandler_(session, req);
            data_.callCountGet_++;
            return;
        }
        if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::GET)) {
            auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
            session.SendResponse(*resp);
        } else if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::GET_FINAL)) {
            auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
            auto appParams = req.GetItemAppParams();
            uint16_t maxListCount = 1;
            if (appParams != nullptr) {
                const TlvTriplet *tlv = appParams->GetTlvtriplet(0x04);
                if (tlv != nullptr) {
                    maxListCount = ObexUtils::GetBufData16(tlv->GetVal(), 0);
                    cout << "maxListCount:" << maxListCount << endl;
                }
            }
            uint8_t body[0xC8];
            for (size_t i = 0; i < sizeof(body); i++) {
                body[i] = i % 0x64;
            }
            if (maxListCount < 0x02) {
                resp->AppendItemBody(body, sizeof(body));
                session.SendResponse(*resp);
            } else {
                auto sendBodyObject = std::make_shared<ObexArrayBodyObject>();
                for (int i = 0; i < maxListCount; i++) {
                    sendBodyObject->Write(body, sizeof(body));
                }
                auto respSuccess = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
                ObexTlvParamters appParamters;
                appParamters.AppendTlvtriplet(TlvTriplet(0x08, maxListCount));
                uint8_t databaseId[16];
                memset_s(databaseId, sizeof(databaseId), 0x00, sizeof(databaseId));
                databaseId[sizeof(databaseId) - 1] = 0x01;
                appParamters.AppendTlvtriplet(TlvTriplet(0x0D, sizeof(databaseId), databaseId));
                respSuccess->AppendItemAppParams(appParamters);
                session.SendGetResponse(req, *respSuccess, sendBodyObject);
            }
        } else {
            session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
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
            cout << "TestObexMpServerObserverDummy:SendAbortResponse:" << ret << endl;
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
            cout << "TestObexMpServerObserverDummy:SendSetPathResponse:" << ret << endl;
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

    DISALLOW_COPY_AND_ASSIGN(TestObexMpServerObserverDummy);
};

void CloseMpClientAndMpServer(bluetooth::ObexMpServer *server, bluetooth::ObexMpClient *client)
{
    client->Disconnect();
    sleep(TEST_SLEEP_SEC_2);
    ObexMpClient::DeregisterL2capLPsm(0x01);
    server->Shutdown();
    sleep(TEST_SLEEP_SEC_2);
}

std::unique_ptr<bluetooth::ObexMpServer> StartTestObexMpServer(
    ObexServerObserver &observer, utility::Dispatcher &dispatcher, bool srm = false)
{
    ObexServerConfig config;
    config.useRfcomm_ = true;
    config.rfcommScn_ = 0xEA;    // 234
    config.rfcommMtu_ = 0x012C;  // 300
    config.useL2cap_ = true;
    config.l2capPsm_ = 0x159;   // 345
    config.l2capMtu_ = 0x01F4;  // 500
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = srm;

    auto obexServer = std::make_unique<ObexMpServer>("test server", config, observer, dispatcher);
    int ret = obexServer->Startup();
    sleep(TEST_SLEEP_SEC_2);
    EXPECT_THAT(ret, 0);

    cout << "StartTestObexMpServer:startup" << ret << endl;
    return obexServer;
}

std::unique_ptr<bluetooth::ObexMpClient> StartTestObexMpClient(
    ObexClientObserver &observer, utility::Dispatcher &dispatcher, bool srm = false)
{
    uint8_t ip[4] = {127, 0, 0, 1};  // 127.0.0.1
    static constexpr uint8_t SERVICE_UUID[16] = {
        0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexClientConfig configL2cap;
    memcpy_s(configL2cap.addr_.addr, sizeof(configL2cap.addr_.addr), ip, sizeof(ip));
    memcpy_s(
        configL2cap.serviceUUID_.uuid128, sizeof(configL2cap.serviceUUID_.uuid128), SERVICE_UUID, sizeof(SERVICE_UUID));

    configL2cap.scn_ = 0x159;   // 345
    configL2cap.mtu_ = 0x012C;  // 300
    configL2cap.isSupportReliableSession_ = false;
    configL2cap.isSupportSrm_ = srm;
    configL2cap.isGoepL2capPSM_ = true;

    ObexMpClient::RegisterL2capLPsm(0x01);
    auto obexMpClient = std::make_unique<ObexMpClient>(configL2cap, observer, dispatcher);
    cout << "StartTestObexMpClient:L2cap" << endl;
    return obexMpClient;
}
}  // namespace obex_mp_client_test

TEST(ObexMpClient_test, PutAndPutFinal)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    ObexTransport &transport = observerForServer->GetData().lastServerSession_->GetTransport();
    EXPECT_THAT(transport.IsConnected(), true);

    // PUT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
    putHeader->AppendItemConnectionId(TEST_CONNECTION_ID_1234);
    uint8_t body[500];
    for (int i = 0; i < 500; i++) {
        body[i] = i % 256;
    }
    putHeader->AppendItemBody(body, 287);
    int putRet = obexClientL2cap->Put(*putHeader);  // Send obex put request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted == 1; })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);
        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x02);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x02);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0x90);
        EXPECT_THAT(
            observerForClient->GetData().lastRespHeader_->GetItemConnectionId()->GetWord(), TEST_CONNECTION_ID_1234);
    }

    // PUT_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);  // Set put_final header
    putHeader->AppendItemEndBody(body + 287, 500 - 287);
    putRet = obexClientL2cap->Put(*putHeader);  // Send obex put_final request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountPut_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);
        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x82);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, PutBodyObject)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);
    uint8_t body[20000];
    for (int i = 0; i < 20000; i++) {
        body[i] = i % 256;
    }
    {
        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, 610);
        auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET);  // Set get header
        // Error: Please connect first.
        int putRet = obexClientL2cap->Put(*getHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, -1);

        // CONNECT TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        int connRet = obexClientL2cap->Connect();
        EXPECT_THAT(connRet, 0);
        if (!DataTestUtil::WaitFor([&]() {
                return (observerForServer->GetData().callCountConnect_ == 1) &&
                       (observerForClient->GetData().callCountOnConnected == 1);
            })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
        EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
        EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);

        // PUT TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        // Error: Opcode is wrong.
        putRet = obexClientL2cap->Put(*getHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, -1);
    }
    {
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, 610);
        // Error: Can't include body/end-of-body header in this method.
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
        putHeader->AppendItemBody(body, 610);
        int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, -1);
        putHeader->RemoveItem(ObexHeader::BODY);
    }
    {
        // put auto continue
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, 1000);
        int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, 0);
        // Error: Another operation is being processed, please try again later.
        int ret = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(ret, -1);
        if (!DataTestUtil::WaitFor([&]() {
                return (observerForServer->GetData().callCountPut_ == 1) &&
                       (observerForClient->GetData().callCountOnActionCompleted == 1);
            })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x82);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    {
        // put auto continue, but abort
        std::cout << " put auto continue, but abort" << std::endl;
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);  // Set put header
        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, sizeof(body));
        int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, 0);
        const ObexClientSession &session = obexClientL2cap->GetClientSession();
        if (!DataTestUtil::WaitFor(
                [&]() { return session.GetLastRespCd() == static_cast<uint8_t>(ObexRspCode::CONTINUE); })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        EXPECT_THAT(obexClientL2cap->Abort(), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted == 1; })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 0);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        EXPECT_THAT(session.GetLastOpeId(), 0xFF);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, PutBodyObjectWithSrm)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher, true);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher, true);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    uint8_t body[5000];
    for (int i = 0; i < 5000; i++) {
        body[i] = i % 256;
    }
    {
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        // PUT TEST
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);
        putHeader->AppendItemConnectionId(6144);
        putHeader->AppendItemType("x-bt/MAP-event-report");
        ObexTlvParamters appParamters;
        TlvTriplet masInstanceID(0x0F, (uint8_t)0);
        appParamters.AppendTlvtriplet(masInstanceID);
        putHeader->AppendItemAppParams(appParamters);

        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, sizeof(body));
        int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, 0);
        if (!DataTestUtil::WaitFor([&]() {
                return (observerForServer->GetData().callCountPut_ == 1) &&
                       (observerForClient->GetData().callCountOnActionCompleted == 1);
            })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x82);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, PutBodyObjectWithSrmAbort)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher, true);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher, true);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    uint8_t body[20000];
    for (int i = 0; i < 20000; i++) {
        body[i] = i % 256;
    }
    {
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        // PUT TEST
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);
        putHeader->AppendItemConnectionId(6144);
        putHeader->AppendItemType("x-bt/MAP-event-report");
        ObexTlvParamters appParamters;
        TlvTriplet masInstanceID(0x0F, (uint8_t)0);
        appParamters.AppendTlvtriplet(masInstanceID);
        putHeader->AppendItemAppParams(appParamters);

        auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, sizeof(body));
        int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
        EXPECT_THAT(putRet, 0);
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        const ObexClientSession &session = obexClientL2cap->GetClientSession();
        if (!DataTestUtil::WaitFor(
                [&]() { return session.GetLastRespCd() == static_cast<uint8_t>(ObexRspCode::CONTINUE); })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(obexClientL2cap->Abort(), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted == 1; })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountPut_, 0);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        EXPECT_THAT(session.GetLastOpeId(), 0xFF);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }
    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, PutFinalBodyObject)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher, true);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher, true);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);

    // PUT TEST
    auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    putHeader->AppendItemConnectionId(6144);
    putHeader->AppendItemType("x-bt/MAP-event-report");
    ObexTlvParamters appParamters;
    TlvTriplet masInstanceID(0x0F, (uint8_t)0);
    appParamters.AppendTlvtriplet(masInstanceID);
    putHeader->AppendItemAppParams(appParamters);
    uint8_t body[100];
    for (int i = 0; i < 100; i++) {
        body[i] = i;
    }
    auto readerForPut = std::make_shared<ObexArrayBodyObject>(body, sizeof(body));
    int putRet = obexClientL2cap->Put(*putHeader, readerForPut);  // Send obex put request
    EXPECT_THAT(putRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountPut_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountPut_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

    const ObexClientSession &session = obexClientL2cap->GetClientSession();

    EXPECT_THAT(session.GetLastOpeId(), 0x82);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x82);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, GetAndGetFinal)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    // FOR Session invalid response
    auto serverSession = observerForServer->GetData().lastServerSession_;
    {
        auto obexReqHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
        auto obexRespHeader = ObexHeader::CreateResponse(ObexRspCode::BAD_REQUEST);
        uint8_t dummyBody[1] = {0};
        obexRespHeader->AppendItemBody(dummyBody, 1);
        // Can't include body/end-of-body header in this method.
        EXPECT_THAT(serverSession->SendGetResponse(*obexReqHeader, *obexRespHeader, nullptr), -1);
    }
    {
        auto obexReqHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
        auto obexRespHeader = ObexHeader::CreateResponse(ObexRspCode::BAD_REQUEST);
        std::string str = "0";
        for (size_t i = 0; i < serverSession->GetMaxPacketLength(); i++) {
            str += "0";
        }
        obexRespHeader->AppendItemType(str);
        // length > mtu continue
        EXPECT_THAT(serverSession->SendGetResponse(*obexReqHeader, *obexRespHeader, nullptr), -1);
    }

    // GET TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET);  // Set get header
    getHeader->AppendItemConnectionId(1);
    int getRet = obexClientL2cap->Get(*getHeader);  // Send obex get request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountGet_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);
        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x03);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x03);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }

    // GET_FINAL TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get_final header
    getRet = obexClientL2cap->Get(*getHeader);                    // Send obex get_final request
    EXPECT_THAT(getRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountGet_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    {
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);
        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x83);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x83);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);
    }

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, GetFinalAutoContinue)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    {
        auto writerForGet = std::make_shared<ObexArrayBodyObject>();
        // Error: Opcode is wrong.
        auto putHeader = ObexHeader::CreateRequest(ObexOpeId::PUT);   // Set get header
        int putRet = obexClientL2cap->Get(*putHeader, writerForGet);  // Send obex get request
        EXPECT_THAT(putRet, -1);
    }
    {
        auto writerForGet = std::make_shared<ObexArrayBodyObject>();
        // Get TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get header
        getHeader->AppendItemConnectionId(1);
        getHeader->AppendItemName(u"telecom/pb.vcf");
        getHeader->AppendItemType("x-bt/phonebook");
        ObexTlvParamters appParamters;
        TlvTriplet maxListCount(0x04, (uint16_t)20);
        appParamters.AppendTlvtriplet(maxListCount);
        TlvTriplet listStartOffset(0x05, (uint16_t)0);
        appParamters.AppendTlvtriplet(listStartOffset);
        uint8_t propertyMask[] = {0, 0, 0, 0, 0, 0, 0, 134};
        TlvTriplet propertySelector(0x08, 8, propertyMask);
        appParamters.AppendTlvtriplet(propertySelector);
        getHeader->AppendItemAppParams(appParamters);
        int getRet = obexClientL2cap->Get(*getHeader, writerForGet);  // Send obex get request
        EXPECT_THAT(getRet, 0);
        if (!DataTestUtil::WaitFor([&]() {
                return (observerForServer->GetData().callCountGet_ == 1) &&
                       (observerForClient->GetData().callCountOnActionCompleted == 1);
            })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x83);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x83);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

        auto extendBody = observerForClient->GetData().lastRespHeader_->GetExtendBodyObject();
        vector<uint8_t> results;
        if (extendBody) {
            uint8_t buf[100];
            memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
            size_t cnt = 0;
            while ((cnt = extendBody->Read(buf, 100)) > 0) {
                results.insert(results.end(), buf, buf + cnt);
            }
            cout << "EXTEND_BODY size: " << results.size() << endl;
        }
        EXPECT_THAT(results.size(), 4000);
    }

    {
        // continue but abort
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        std::cout << "continue but abort" << std::endl;
        auto writerForGet = std::make_shared<ObexArrayBodyObject>();
        // Get TEST
        auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get header
        getHeader->AppendItemConnectionId(1);
        getHeader->AppendItemName(u"telecom/pb.vcf");
        getHeader->AppendItemType("x-bt/phonebook");
        ObexTlvParamters appParamters;
        TlvTriplet maxListCount(0x04, (uint16_t)100);
        appParamters.AppendTlvtriplet(maxListCount);
        getHeader->AppendItemAppParams(appParamters);
        std::cout << "call get" << std::endl;
        int getRet = obexClientL2cap->Get(*getHeader, writerForGet);  // Send obex get request
        EXPECT_THAT(getRet, 0);

        if (!DataTestUtil::WaitFor([&]() { return observerForServer->GetData().callCountGet_ == 1; })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        std::cout << "call abort" << std::endl;

        EXPECT_THAT(obexClientL2cap->Abort(), 0);

        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted == 1; })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 0);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0xFF);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

        auto extendBody = observerForClient->GetData().lastRespHeader_->GetExtendBodyObject();

        EXPECT_THAT(extendBody == nullptr, true);
    }

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, GetFinalAutoContinueSrm)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher, true);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher, true);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);
    {
        // Get TEST
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        auto writerForGet = std::make_shared<ObexArrayBodyObject>();
        auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get header
        getHeader->AppendItemConnectionId(1);
        getHeader->AppendItemName(u"telecom/pb.vcf");
        getHeader->AppendItemType("x-bt/phonebook");
        ObexTlvParamters appParamters;
        TlvTriplet maxListCount(0x04, (uint16_t)20);
        appParamters.AppendTlvtriplet(maxListCount);
        TlvTriplet listStartOffset(0x05, (uint16_t)0);
        appParamters.AppendTlvtriplet(listStartOffset);
        uint8_t propertyMask[] = {0, 0, 0, 0, 0, 0, 0, 134};
        TlvTriplet propertySelector(0x08, 8, propertyMask);
        appParamters.AppendTlvtriplet(propertySelector);
        getHeader->AppendItemAppParams(appParamters);
        int getRet = obexClientL2cap->Get(*getHeader, writerForGet, 2);  // Send obex get request
        EXPECT_THAT(getRet, 0);
        if (!DataTestUtil::WaitFor([&]() {
                return (observerForServer->GetData().callCountGet_ == 1) &&
                       (observerForClient->GetData().callCountOnActionCompleted == 1);
            })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 1);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0x83);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
        EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x83);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

        auto extendBody = observerForClient->GetData().lastRespHeader_->GetExtendBodyObject();
        vector<uint8_t> results;
        if (extendBody) {
            uint8_t buf[100];
            memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
            size_t cnt = 0;
            while ((cnt = extendBody->Read(buf, 100)) > 0) {
                results.insert(results.end(), buf, buf + cnt);
            }
            cout << "EXTEND_BODY size: " << results.size() << endl;
        }
        EXPECT_THAT(results.size(), 4000);
    }
    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, GetFinalAutoContinueSrmAbort)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher, true);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher, true);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);

    {
        // continue but abort
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        LOG_DEBUG("continue but abort");
        std::cout << "continue but abort" << std::endl;
        auto writerForGet = std::make_shared<ObexArrayBodyObject>();
        // Get TEST
        auto getHeader = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);  // Set get header
        getHeader->AppendItemConnectionId(1);
        getHeader->AppendItemName(u"telecom/pb.vcf");
        getHeader->AppendItemType("x-bt/phonebook");
        ObexTlvParamters appParamters;
        TlvTriplet maxListCount(0x04, (uint16_t)0xFF);
        appParamters.AppendTlvtriplet(maxListCount);
        getHeader->AppendItemAppParams(appParamters);
        std::cout << "call get" << std::endl;
        LOG_DEBUG("call get");

        EXPECT_THAT(obexClientL2cap->Get(*getHeader, writerForGet), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForServer->GetData().callCountGet_ == 1; })) {
            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            LOG_ERROR("time out");
            return;
        };
        std::cout << "call abort" << std::endl;
        LOG_DEBUG("call abort");

        EXPECT_THAT(obexClientL2cap->Abort(), 0);
        observerForServer->ResetCallCount();
        observerForClient->ResetCallCount();
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->GetData().callCountOnActionCompleted == 1; })) {
            std::cout << "observerForClient->GetData().callCountOnActionCompleted :"
                      << observerForClient->GetData().callCountOnActionCompleted << std::endl;

            obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->GetData().callCountGet_, 0);
        EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

        const ObexClientSession &session = obexClientL2cap->GetClientSession();

        EXPECT_THAT(session.GetLastOpeId(), 0xFF);
        EXPECT_NE(session.GetLastReqHeader(), nullptr);
        EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

        auto extendBody = observerForClient->GetData().lastRespHeader_->GetExtendBodyObject();

        EXPECT_THAT(extendBody == nullptr, true);
    }
    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, SetPathWithFlag)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);

    // SETPATH TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int setpathRet = obexClientL2cap->SetPath(OBEX_SETPATH_NOCREATE, u"");  // Send obex setpath request
    EXPECT_THAT(setpathRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountSetPath_ == 1) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountSetPath_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

    const ObexClientSession &session = obexClientL2cap->GetClientSession();

    EXPECT_THAT(session.GetLastOpeId(), 0x85);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x85);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}

TEST(ObexMpClient_test, SetPathWithParms)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");
    auto observerForServer = std::make_unique<obex_mp_client_test::TestObexMpServerObserverDummy>();
    auto obexServer = obex_mp_client_test::StartTestObexMpServer(*observerForServer, *serverDispatcher);
    auto observerForClient = std::make_unique<obex_mp_client_test::TestObexMpClientObserver>();
    auto obexClientL2cap = obex_mp_client_test::StartTestObexMpClient(*observerForClient, *clientDispatcher);

    // CONNECT TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    int connRet = obexClientL2cap->Connect();
    EXPECT_THAT(connRet, 0);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountConnect_ == 1) &&
                   (observerForClient->GetData().callCountOnConnected == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnect_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountTransportConnected_, 1);
    EXPECT_THAT(observerForServer->GetData().callCountConnect_, 1);
    EXPECT_THAT(observerForClient->GetData().callCountOnConnected, 1);

    // SETPATH TEST
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    std::vector<std::u16string> paths;
    // Error: SetPath Job's size is 0.
    int setpathRet = obexClientL2cap->SetPath(paths);
    EXPECT_THAT(setpathRet, -1);

    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    paths.push_back(u"inbox");
    setpathRet = obexClientL2cap->SetPath(paths);  // Send obex setpath request
    EXPECT_THAT(setpathRet, 0);
    // Error: Another SetPath Job is being processing
    int ret = obexClientL2cap->SetPath(paths);
    EXPECT_THAT(ret, -1);
    if (!DataTestUtil::WaitFor([&]() {
            return (observerForServer->GetData().callCountSetPath_ == 4) &&
                   (observerForClient->GetData().callCountOnActionCompleted == 1);
        })) {
        obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->GetData().callCountSetPath_, 4);
    EXPECT_THAT(observerForClient->GetData().callCountOnActionCompleted, 1);

    const ObexClientSession &session = obexClientL2cap->GetClientSession();

    EXPECT_THAT(session.GetLastOpeId(), 0x85);
    EXPECT_NE(session.GetLastReqHeader(), nullptr);
    EXPECT_NE(observerForClient->GetData().lastRespHeader_, nullptr);
    EXPECT_THAT(session.GetLastReqHeader()->GetFieldCode(), 0x85);
    EXPECT_THAT(observerForClient->GetData().lastRespHeader_->GetFieldCode(), 0xA0);

    // DISCONNECT TEST
    obex_mp_client_test::CloseMpClientAndMpServer(obexServer.get(), obexClientL2cap.get());
}
