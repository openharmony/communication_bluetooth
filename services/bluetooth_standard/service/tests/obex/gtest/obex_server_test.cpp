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
#include "obex_headers.h"
#include "obex_server.h"
#include "obex_utils.h"
#include "stub/message_digest.h"

using namespace std;
using namespace testing;
using namespace bluetooth;
using namespace stub;
namespace obex_server_test {
class TestObexServerObserver : public ObexServerObserver {
public:
    TestObexServerObserver() = default;

    virtual ~TestObexServerObserver();
    void OnTransportConnect(ObexIncomingConnect &incomingConnect) override;

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override;
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override;

    ObexIncomingConnect *GetLastIncomingConnect()
    {
        return lastIncomingConnect_;
    }
    ObexServerSession *GetLastServerSession()
    {
        return lastServerSession_;
    }

private:
    ObexIncomingConnect *lastIncomingConnect_ = nullptr;
    ObexServerSession *lastServerSession_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(TestObexServerObserver);
};
TestObexServerObserver::~TestObexServerObserver()
{}
void TestObexServerObserver::OnTransportConnect(ObexIncomingConnect &incomingConnect)
{
    cout << "TestObexServerObserver::OnTransportConnect" << endl;

    incomingConnect.AcceptConnection();
}

void TestObexServerObserver::OnConnect(ObexServerSession &session, const ObexHeader &req)
{
    auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
    int ret = session.SendResponse(*header);
    cout << "TestObexServerObserver:SendResponse:" << ret << endl;

    lastServerSession_ = &session;
}

void TestObexServerObserver::OnDisconnect(ObexServerSession &session, const ObexHeader &req)
{
    cout << "TestObexServerObserver::OnDisconnect" << endl;
}
}  // namespace obex_server_test

TEST(ObexServer_test, Startup_Rfcomm)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");

    ObexServerConfig config;
    config.useRfcomm_ = true;
    config.rfcommScn_ = 234;
    config.rfcommMtu_ = 500;
    config.useL2cap_ = false;
    config.l2capPsm_ = 345;
    config.l2capMtu_ = 500;
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;
    auto observer = std::make_unique<obex_server_test::TestObexServerObserver>();

    auto obexServer = std::make_unique<ObexServer>("test server", config, *observer, *serverDispatcher);
    int ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, 0);

    ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, -1);

    obexServer->Shutdown();
    sleep(2);
}

TEST(ObexServer_test, Startup_L2cap)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");

    ObexServerConfig config;
    config.useRfcomm_ = false;
    config.rfcommScn_ = 234;
    config.rfcommMtu_ = 500;
    config.useL2cap_ = true;
    config.l2capPsm_ = 345;
    config.l2capMtu_ = 500;
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;
    auto observer = std::make_unique<obex_server_test::TestObexServerObserver>();

    auto obexServer = std::make_unique<ObexServer>("test server", config, *observer, *serverDispatcher);
    int ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, 0);

    ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, -1);

    obexServer->Shutdown();
    sleep(2);
}

TEST(ObexServer_test, Startup_Both)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");

    ObexServerConfig config;
    config.useRfcomm_ = true;
    config.rfcommScn_ = 234;
    config.rfcommMtu_ = 500;
    config.useL2cap_ = true;
    config.l2capPsm_ = 345;
    config.l2capMtu_ = 500;
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;
    auto observer = std::make_unique<obex_server_test::TestObexServerObserver>();

    auto obexServer = std::make_unique<ObexServer>("test server", config, *observer, *serverDispatcher);
    int ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, 0);

    obexServer->Shutdown();
    sleep(2);
}

TEST(ObexServer_test, MD5)
{
    MessageDigest *messageDigest = MessageDigestFactory::GetInstance(DIGEST_TYPE_MD5);
    const char *input = "1234567890123456";
    std::vector<uint8_t> md5 = messageDigest->Digest((uint8_t *)input, 16);
    EXPECT_THAT(md5,
        ElementsAreArray(
            {0xab, 0xea, 0xc0, 0x7d, 0x3c, 0x28, 0xc1, 0xbe, 0xf9, 0xe7, 0x30, 0x00, 0x2c, 0x75, 0x3e, 0xd4}));
}

TEST(ObexServer_test, StartupWhenBothNotUsed)
{
    auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");
    ObexServerConfig config;
    config.useRfcomm_ = false;
    config.rfcommScn_ = 234;
    config.rfcommMtu_ = 500;
    config.useL2cap_ = false;
    config.l2capPsm_ = 345;
    config.l2capMtu_ = 500;
    config.isSupportReliableSession_ = false;
    config.isSupportSrm_ = true;
    auto observer = std::make_unique<obex_server_test::TestObexServerObserver>();

    auto obexServer = std::make_unique<ObexServer>("test server", config, *observer, *serverDispatcher);
    int ret = obexServer->Startup();
    sleep(2);
    EXPECT_THAT(ret, -1);

    obexServer->Shutdown();
    sleep(2);
}
