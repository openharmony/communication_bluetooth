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
#include <stdio.h>
#include <stdlib.h>
#include "securec.h"
#include "context.h"
#include "data_test_util.h"
#include "map_mse_service.h"
#include "mse_mock/mse_mock.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include <memory>
#include <sstream>
#include <string>
#include "log.h"
using namespace testing;
using namespace bluetooth;
using namespace stub;

class TestMnsObserver : public ObexServerObserver {
public:
    void OnConnect(ObexServerSession &session, const ObexHeader &req) override
    {
        std::cout << "TestMnsObserver::OnConnect" << std::endl;

        auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
        header->AppendItemConnectionId(connectId_++);
        header->AppendItemWho(MNS_TARGET, 16);
        session.SendResponse(*header);
        callCountOnConnected++;
    }
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override
    {
        std::cout << "TestMnsObserver::OnDisconnect" << std::endl;

        session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::SUCCESS));
        callCountOnDisconnected++;
    }
    void OnPut(ObexServerSession &session, const ObexHeader &req) override
    {
        std::cout << "TestMnsObserver::OnPut" << std::endl;
        reqHeader_ = std ::make_unique<ObexHeader>(req);
        session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::SUCCESS));
        callCountOnPut++;
    }
    void ResetCallCount()
    {

        callCountOnConnected = 0;
        callCountOnDisconnected = 0;
        callCountOnPut = 0;
    }

    int callCountOnConnected{0};
    int callCountOnDisconnected{0};
    int callCountOnPut{0};
    std::unique_ptr<ObexHeader> reqHeader_;
    std::atomic_uint32_t connectId_ = 0;
    const uint8_t MNS_TARGET[16] = {
        0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
};

std::unique_ptr<bluetooth::ObexMpServer> StartMnsServer(
    ObexServerObserver *observer, utility::Dispatcher *dispatcher, uint16_t psm = 0)
{
    ObexServerConfig obexSrvConfig = {.useRfcomm_ = false,
        .rfcommScn_ = 0,
        .rfcommMtu_ = 500,
        .useL2cap_ = true,
        .l2capPsm_ = psm,
        .l2capMtu_ = 400,
        .isSupportSrm_ = true,
        .isSupportReliableSession_ = false};
    std::string serviceName = "TestMnsServer";
    auto mnsServer = std::make_unique<ObexMpServer>(serviceName, obexSrvConfig, *observer, *dispatcher);
    return mnsServer;
}

class TestObexClientObserver : public ObexClientObserver {
public:
    void OnTransportFailed(ObexClient &client, int errCd) override
    {
        std::cout << "TestObexClientObserver::OnTransportFailed" << std::endl;
        respHeader_ = nullptr;
        callCountOnTransportFailed++;
    }
    void OnConnected(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnected" << std::endl;
        respHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnected++;
    }
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnectFailed" << std::endl;
        respHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnectFailed++;
    }
    void OnDisconnected(ObexClient &client) override
    {
        std::cout << "TestObexClientObserver::OnDisconnected" << std::endl;
        respHeader_ = nullptr;
        callCountOnDisconnected++;
    }
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnActionCompleted" << std::endl;
        respHeader_ = std ::make_unique<ObexHeader>(resp);
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
    std::unique_ptr<ObexHeader> respHeader_;
};

std::unique_ptr<bluetooth::ObexMpClient> StartTestObexMpClient(
    ObexClientObserver *observer, utility::Dispatcher *dispatcher, uint16_t scn = 0)
{
    uint8_t ip[4] = {127, 0, 0, 1};  // 127.0.0.1
    static constexpr uint8_t SERVICE_UUID[16] = {
        0xbb, 0x58, 0x2b, 0x40, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
    ObexClientConfig configRfcomm;
    memcpy_s(configRfcomm.addr_.addr, sizeof(configRfcomm.addr_.addr), ip, sizeof(ip));
    memcpy_s(configRfcomm.serviceUUID_.uuid128, sizeof(configRfcomm.serviceUUID_.uuid128), SERVICE_UUID, sizeof(SERVICE_UUID));

    configRfcomm.scn_ = 100 + scn;
    configRfcomm.mtu_ = 400;
    configRfcomm.isSupportReliableSession_ = false;
    configRfcomm.isSupportSrm_ = true;
    configRfcomm.isGoepL2capPSM_ = true;

    auto obexMpClient = std::make_unique<ObexMpClient>(configRfcomm, *observer, *dispatcher);
    return obexMpClient;
}

class TestContentObserver : public utility::IContextCallback {
public:
    void OnEnable(const std::string &name, bool ret) override
    {
        std::cout << "TestContentObserver::OnEnable" << std::endl;
        callCountOnEnable++;
    }
    void OnDisable(const std::string &name, bool ret) override
    {
        std::cout << "TestContentObserver::OnDisable" << std::endl;
        callCountOnDisable++;
    }
    void ResetCallCount()
    {
        callCountOnEnable = 0;
        callCountOnDisable = 0;
    }

    int callCountOnEnable{0};
    int callCountOnDisable{0};
};

namespace bluetooth {

class DefaultObserver : public IMapMseObserver {};

class MapMseObserver : public IMapMseObserver {
public:
    void OnConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        std::cout << "TestMapMseObserver::OnConnectionStateChanged" << std::endl;
        switch (state) {
            case 0:
                std::cout << "mse " << remoteAddr.GetAddress().c_str() << " OnConnecting" << std::endl;
                break;
            case 1:
                std::cout << "mse " << remoteAddr.GetAddress().c_str() << " OnConnected" << std::endl;
                break;
            case 2:
                std::cout << "mse " << remoteAddr.GetAddress().c_str() << " OnDisconnecting" << std::endl;
                break;
            case 3:
                std::cout << "mse " << remoteAddr.GetAddress().c_str() << " OnDisconnected" << std::endl;
                break;
            default:
                std::cout << "mse " << remoteAddr.GetAddress().c_str() << " OnStateChanged:status error" << std::endl;
                break;
        }
        callCountOnConnectionStateChanged++;
    }
    void OnPermission(const RawAddress &remoteAddr) override
    {
        std::cout << "TestMapMseObserver::OnPermission" << std::endl;
        callCountOnPermission++;
    }
    void ResetCallCount()
    {
        callCountOnConnectionStateChanged = 0;
        callCountOnPermission = 0;
    }

    int callCountOnConnectionStateChanged{0};
    int callCountOnPermission{0};
};

void MseCloseServer(MapMseService &server, TestContentObserver &callBackForServer)
{
    callBackForServer.ResetCallCount();
    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.callCountOnDisable == 1; })) {
        std::cout << "OnDisable CloseServer" << std::endl;
        return;
    };
}

void PrintObexBodyObject(const std::shared_ptr<ObexBodyObject> &extendBody)
{
    if (!extendBody) {
        return;
    }
    uint8_t buf[100];
    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    std::vector<uint8_t> results;
    size_t cnt = 0;
    while ((cnt = extendBody->Read(buf, 100)) > 0) {
        results.insert(results.end(), buf, buf + cnt);
    }
    std::cout << "EXTEND_BODY:\n" << ObexUtils::ToDebugString(results.data(), results.size(), true) << std::endl;
    std::cout << "EXTEND_BODY size: " << results.size() << std::endl;
    std::string vcardInfo((char *)results.data(), results.size());
    std::cout << "BODY STRING:\n" << vcardInfo << std::endl;
}

void TestSentEvent(ObexMpServer &mnsServer, TestMnsObserver &callbackServer, int type)
{
    callbackServer.ResetCallCount();
    stub::MapService::GetInstance()->SendEvent("00:00:01:00:00:7F", type);
    if (!DataTestUtil::WaitFor([&]() { return callbackServer.callCountOnPut == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        mnsServer.Shutdown();
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto &extendBody = callbackServer.reqHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    PrintObexBodyObject(extendBody);
}
}  // namespace bluetooth
enum InstanceType { IM, EMAIL, SMS };

TEST(MapMseService_test, Construct)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);

    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);

    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    stub::MapService::GetInstance()->SetMseInstance(12);
    stub::MapService::GetInstance()->ReceiveMessage(5);
    stub::MapService::GetInstance()->ReceiveMessage(2);

    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, Enable_Disable)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_FORBIDDEN);
    obsClient->ResetCallCount();
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnTransportFailed == 1; })) {
        std::cout << "OnDisconnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, Client_Disconnect)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    obsClient->ResetCallCount();
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "OnConnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_EQ(0, obexClientRfcomm->Disconnect());
    obsClient->ResetCallCount();
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnDisconnected == 1; })) {
        std::cout << "OnDisconnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, IsConnect)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(4);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress mockerDevice("00:1A:7D:DA:71:B4");
    EXPECT_EQ(false, mseService->IsConnected(mockerDevice));
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, RegisterObserver)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(5);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->Connect(rawAddr);
    sleep(3);
    mseService->GetConnectionState(rawAddr);
    sleep(3);
    mseService->GrantPermission(rawAddr, true);
    sleep(2);
    mseService->Disconnect(rawAddr);
    sleep(2);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, DefaultRegisterObserver)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<DefaultObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(5);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "OnConnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, DefaultRegisterObserver2)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<DefaultObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(5);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    sleep(3);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, DeRegisterObserver)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(6);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    sleep(3);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    mseService->DeregisterObserver(*observer);
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, Other)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    mseService->GetConnectDevices();
    sleep(1);
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->SetConnectionStrategy(rawAddr, 0);
    sleep(3);
    EXPECT_EQ(2, mseService->GetConnectionStrategy(rawAddr));
    sleep(3);
    mseService->SetConnectionStrategy(rawAddr, 1);
    sleep(3);
    EXPECT_EQ(1, mseService->GetConnectionStrategy(rawAddr));
    mseService->SetConnectionStrategy(rawAddr, 2);
    sleep(5);
    EXPECT_EQ(2, mseService->GetConnectionStrategy(rawAddr));
    mseService->Connect(rawAddr);
    mseService->GetConnectionState(rawAddr);
    mseService->GetConnectState();
    std::vector<int> states;
    states.push_back(0);
    states.push_back(1);
    states.push_back(2);
    states.push_back(3);
    mseService->GetDevicesByStates(states);
    mseService->GetMaxConnectNum();
    mseService->GetState();
    mseService->GrantPermission(rawAddr, true);
    mseService->GetContext();
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, Error)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, callback)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto obsClient = std::make_unique<TestObexClientObserver>();
    obsClient->ResetCallCount();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    RawAddress rawAddr("00:00:01:00:00:7F");
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    mseService->GetState();
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "OnConnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    mseService->GetState();
    sleep(2);
    EXPECT_EQ(0, obexClientRfcomm->Disconnect());
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnDisconnected == 1; })) {
        std::cout << "OnDisconnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    mseService->GetState();
    mseService->GetConnectionState(rawAddr);
    sleep(2);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, callbackError)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, false);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnTransportFailed == 1; })) {
        std::cout << "OnDisconnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, server_disconnect)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    obsClient->ResetCallCount();
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "OnConnected CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obexClientRfcomm->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, connectTimeout)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto obexClientRfcomm = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    obsClient->ResetCallCount();
    EXPECT_EQ(0, obexClientRfcomm->Connect());
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnTransportFailed == 1; }, 50)) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, SetNotificationRegistration)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    sleep(2);
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    mseReq->AppendItemType("x-bt/MAP-NotificationRegistration");
    ObexTlvParamters appParamters1;
    appParamters1.AppendTlvtriplet(TlvTriplet(0x0E, (uint8_t)0x00));
    mseReq->AppendItemAppParams(appParamters1);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    obsClient->ResetCallCount();
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, SetNotificationFilter)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    mseReq->AppendItemType("x-bt/MAP-notification-filter");
    ObexTlvParamters appParamters1;
    appParamters1.AppendTlvtriplet(TlvTriplet(0x25, (uint32_t)0x01));
    mseReq->AppendItemAppParams(appParamters1);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, SetMessageStatus)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_EQ(2, observer->callCountOnConnectionStateChanged);
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    std::u16string name = u"1020210114030526";
    mseReq->AppendItemName(name);
    mseReq->AppendItemType("x-bt/messageStatus");
    ObexTlvParamters appParamters1;
    appParamters1.AppendTlvtriplet(TlvTriplet(0x17, (uint8_t)0x01));
    appParamters1.AppendTlvtriplet(TlvTriplet(0x18, (uint8_t)0x01));
    mseReq->AppendItemAppParams(appParamters1);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    obsClient->ResetCallCount();
    auto mseReq2 = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq2->AppendItemConnectionId(connectId);
    mseReq2->AppendItemName(name);
    mseReq2->AppendItemType("x-bt/messageStatus");
    ObexTlvParamters appParamters2;
    appParamters2.AppendTlvtriplet(TlvTriplet(0x17, (uint8_t)0x00));
    appParamters2.AppendTlvtriplet(TlvTriplet(0x18, (uint8_t)0x01));
    mseReq->AppendItemAppParams(appParamters2);
    mseReq2->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq2);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, SetMessageStatus2)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_EQ(2, observer->callCountOnConnectionStateChanged);
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    std::u16string name = u"0220210114030711";
    mseReq->AppendItemName(name);
    mseReq->AppendItemType("x-bt/messageStatus");
    ObexTlvParamters appParamters1;
    appParamters1.AppendTlvtriplet(TlvTriplet(0x17, (uint8_t)0x00));
    appParamters1.AppendTlvtriplet(TlvTriplet(0x18, (uint8_t)0x01));
    std::string extData = "8:mseut";
    appParamters1.AppendTlvtriplet(TlvTriplet(0x28, (uint8_t)extData.size(), (uint8_t *)extData.data()));
    mseReq->AppendItemAppParams(appParamters1);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, SetPath)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_EQ(2, observer->callCountOnConnectionStateChanged);
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::SETPATH);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());

    obsClient->ResetCallCount();
    std::u16string name;
    uint8_t notFlag = 0b01;
    mapClient->SetPath(notFlag, name);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xC6);  // NOT_ACCEPTABLE

    obsClient->ResetCallCount();
    uint8_t root = 0b11;
    mapClient->SetPath(root, name);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xC4);  // Not NOT_FOUND

    obsClient->ResetCallCount();
    name = u"telecom";
    mapClient->SetPath(root, name);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xC4);  // Not NOT_FOUND

    uint8_t flags = 0b10;
    name = u"telecom";
    mapClient->SetPath(flags, name);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 2; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 2);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);

    name = u"msg";
    mapClient->SetPath(flags, name);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 3; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 3);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);

    std::u16string nullName;
    mapClient->SetPath(flags, nullName);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 4; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 4);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    EXPECT_EQ(0, 0);
}

TEST(MapMseService_test, PushMessage)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(2);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_UNKNOWN);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return observer->callCountOnPermission == 1; })) {
        std::cout << "OnPermission CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    RawAddress rawAddr("00:00:01:00:00:7F");
    mseService->GrantPermission(rawAddr, true);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.0" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:EMAIL" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Mat" << std::endl;
    data << "EMAIL:ma@abc.edu" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Tanaka" << std::endl;
    data << "EMAIL:tanaka@def.edu" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Laurent" << std::endl;
    data << "EMAIL:laurent@ghi.edu" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:8BIT" << std::endl;
    data << "LENGTH:125" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "Date: 20 Jun 96" << std::endl;
    data << "Subject: Fish" << std::endl;
    data << "From: tanaka@def.edu" << std::endl;
    data << "To: laurent@ghi.edu" << std::endl;
    data << std::endl;
    data << std::endl;
    data << "Let's go fishing!" << std::endl;
    data << "BR, Mat" << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, PushMessageVcard3)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.1" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:IM" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:3.0" << std::endl;
    data << "N:L;Marry" << std::endl;
    data << "FN:Mrt.Marry" << std::endl;
    data << "X-BT-UID:012345679EB2345432345678912ABCDE" << std::endl;
    data << "X-BT-UCI:whateverapp:497654321@s.whateverapp.net" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:3.0" << std::endl;
    data << "N:S.;Alois" << std::endl;
    data << "X-BT-UID:022345679EB2345432345678912ABCDE" << std::endl;
    data << "X-BT-UCI:whateverapp:4912345@s.whateverapp.net;unit@test.com" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:8BIT" << std::endl;
    data << "LENGTH:" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "Date: Fri, 1 Aug 2014 01:29:12 +01:00" << std::endl;
    data << "From:whateverapp:497654321@s.whateverapp.net" << std::endl;
    data << "To:whateverapp:4912345@s.whateverapp.net;unit@test.com" << std::endl;
    data << "Content-Type:text/plain;" << std::endl;
    data << std::endl;
    data << std::endl;
    data << "Happy birthady!" << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, PushMessageGsm)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.0" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:SMS_GSM" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Joachim" << std::endl;
    data << "TEL:00498912345678" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Mr.Liu" << std::endl;
    data << "TEL:13354011188" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:G-7BIT" << std::endl;
    data << "LENGTH:96" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "0191000E9100949821436587000011303231" << std::endl;
    data << "12928211CC32FD34079DDF20737A8E4EBBCF21" << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, PushMessageIm)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::IM);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    std::string convoid = "E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4";
    appParas.AppendTlvtriplet(TlvTriplet(0x22, convoid.size(), (const uint8_t *)convoid.data()));  // conversationId
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.1" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:IM" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "EXTENDEDDATA:0:3" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:L;Marry" << std::endl;
    data << "X-BT-UCI:whateverrapp:497654321@s.whateverapp.net" << std::endl;
    data << "X-BT-UID:44101410@s.whateverapp.net" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:S.;Alois" << std::endl;
    data << "X-BT-UCI:whateverrapp:49712345@s.whateverapp.net" << std::endl;
    data << "X-BT-UID:88578450@s.whateverapp.net" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:8BIT" << std::endl;
    data << "LENGTH:" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "Date:Fri, 1 Aug 2014 01:29:12 +01:00" << std::endl;
    data << "From:whateverapp:497654321@s.whateverapp.net" << std::endl;
    data << "To:whateverapp:4912345@s.whateverapp.net" << std::endl;
    data << "Content-Type:text/plain;" << std::endl;
    data << std::endl;
    data << "Happy birthday to you." << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, PushMessageCdma)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.0" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:SMS_CDMA" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:CdmaClark" << std::endl;
    data << "TEL:00123412345678" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Mr.Liu" << std::endl;
    data << "TEL:15554011188" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:G-7BIT" << std::endl;
    data << "LENGTH:96" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "0191000E9100949821436587000011303231" << std::endl;
    data << "12928211CC32FD34079DDF20737A8E4EBBCF21" << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, PushMessageMms)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    sleep(2);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    std::u16string folderName = u"outbox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x1));  // attachment
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x0B, (uint8_t)0x1));  // transparent
    appParas.AppendTlvtriplet(TlvTriplet(0x0C, (uint8_t)0x1));  // retry
    mseReq->AppendItemAppParams(appParas);
    std::stringstream data;
    data << "BEGIN:BMSG" << std::endl;
    data << "VERSION:1.0" << std::endl;
    data << "STATUS:UNREAD" << std::endl;
    data << "TYPE:MMS" << std::endl;
    data << "FOLDER:TELECOM/MSG/INBOX" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Joachim" << std::endl;
    data << "TEL:1234512345678" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BENV" << std::endl;
    data << "BEGIN:VCARD" << std::endl;
    data << "VERSION:2.1" << std::endl;
    data << "N:Mr.Liu" << std::endl;
    data << "TEL:18854011188" << std::endl;
    data << "END:VCARD" << std::endl;
    data << "BEGIN:BBODY" << std::endl;
    data << "ENCODING:G-7BIT" << std::endl;
    data << "LENGTH:96" << std::endl;
    data << "BEGIN:MSG" << std::endl;
    data << "0191000E9100949821436587000011303231" << std::endl;
    data << "12928211CC32FD34079DDF20737A8E4EBBCF21" << std::endl;
    data << "369852CC32FD34079DDF20737A8E4EBBCF21" << std::endl;
    data << "END:MSG" << std::endl;
    data << "END:BBODY" << std::endl;
    data << "END:BENV" << std::endl;
    data << "END:BMSG" << std::endl;
    std::string strData = data.str();
    printf("%ld : %s", strData.size(), strData.c_str());
    auto sendedObj = std::make_shared<ObexArrayBodyObject>();
    sendedObj->Write((uint8_t *)strData.c_str(), strData.size());
    obsClient->ResetCallCount();
    EXPECT_THAT(mapClient->Put(*mseReq, sendedObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetMessageListing)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/MAP-msg-listing");
    std::u16string folderName = u"INBox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0x010));  // maxlistcount
    appParas.AppendTlvtriplet(TlvTriplet(0x02, (uint16_t)0x003));  // listStartOffset
    std::string srch = "tan*";
    appParas.AppendTlvtriplet(TlvTriplet(0x07, srch.size(), (const uint8_t *)srch.data()));  // filterRecipient
    std::string org = "M";
    appParas.AppendTlvtriplet(TlvTriplet(0x08, org.size(), (const uint8_t *)org.data()));  // filterOriginator
    appParas.AppendTlvtriplet(TlvTriplet(0x06, (uint8_t)0x1));                             // filterReadStatus
    appParas.AppendTlvtriplet(TlvTriplet(0x13, (uint8_t)0xA));                             // subjectLength
    appParas.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)0x0));                            // parameterMask
    appParas.AppendTlvtriplet(TlvTriplet(0x03, (uint8_t)0x1B));                            // filterMessageType
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetMessageListing2)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/MAP-msg-listing");
    std::u16string folderName = u"INBox";
    mseReq->AppendItemName(folderName);
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0x005));  // maxlistcount
    appParas.AppendTlvtriplet(TlvTriplet(0x02, (uint16_t)0x003));  // listStartOffset
    std::string srch = "tan";
    appParas.AppendTlvtriplet(TlvTriplet(0x07, srch.size(), (const uint8_t *)srch.data()));  // filterRecipient
    std::string org = "M*";
    appParas.AppendTlvtriplet(TlvTriplet(0x08, org.size(), (const uint8_t *)org.data()));  // filterOriginator
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetConversationListing)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::IM);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/MAP-convo-listing");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0x05));  // maxlistcount
    std::string begin = "20131201T112030";
    std::string end = "20201210T112030";
    appParas.AppendTlvtriplet(TlvTriplet(0x02, (uint16_t)0x00));  // listStartOffset
    appParas.AppendTlvtriplet(TlvTriplet(0x1F, begin.size(), (const uint8_t *)begin.data()));  // filterLastActityBegin
    appParas.AppendTlvtriplet(TlvTriplet(0x20, end.size(), (const uint8_t *)end.data()));      // filterLastActityEnd
    appParas.AppendTlvtriplet(TlvTriplet(0x06, (uint8_t)0x0));                                 // filterReadStatus
    std::string srch = "*B4";
    appParas.AppendTlvtriplet(TlvTriplet(0x07, srch.size(), (const uint8_t *)srch.data()));  // filterRecipient
    std::string convoid = "E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4";
    appParas.AppendTlvtriplet(TlvTriplet(0x22, convoid.size(), (const uint8_t *)convoid.data()));  // conversationId
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetConversationListing2)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();
    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::IM);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/MAP-convo-listing");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0x0));  // maxlistcount
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    auto listingSize = obsClient->respHeader_->GetItemAppParams()->GetTlvtriplet(0x12)->GetUint16();
    EXPECT_THAT(listingSize >= 2, IsTrue);
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetFolderListing)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-obex/folder-listing");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0xFF));  // maxlistcount
    appParas.AppendTlvtriplet(TlvTriplet(0x02, (uint16_t)0x00));  // listStartOffset
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    sleep(1);
}

TEST(MapMseService_test, GetFolderListing2)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::EMAIL);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-obex/folder-listing");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x01, (uint16_t)0xFF));  // maxlistcount
    appParas.AppendTlvtriplet(TlvTriplet(0x02, (uint16_t)0x00));  // listStartOffset
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    sleep(1);
}

TEST(MapMseService_test, SetOwnerStatus)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::IM);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/ownerStatus");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x1C, (uint8_t)0xFF));  // presenceAvailability
    std::string presenceText = "运行";
    appParas.AppendTlvtriplet(
        TlvTriplet(0x1D, (uint8_t)presenceText.size(), (uint8_t *)presenceText.data()));  // presenceText
    std::string lastActivity = "20201208T174820+100";
    appParas.AppendTlvtriplet(
        TlvTriplet(0x1E, (uint8_t)lastActivity.size(), (uint8_t *)lastActivity.data()));  // lastActivity
    appParas.AppendTlvtriplet(TlvTriplet(0x21, (uint8_t)0x02));                           // chatState
    std::string conversationId = "C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4";
    appParas.AppendTlvtriplet(
        TlvTriplet(0x22, (uint8_t)conversationId.size(), (uint8_t *)conversationId.data()));  // conversationId
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetOwnerStatus)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(3);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get(), InstanceType::IM);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/ownerStatus");
    ObexTlvParamters appParas;
    std::string conversationId = "C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4";
    appParas.AppendTlvtriplet(
        TlvTriplet(0x22, (uint8_t)conversationId.size(), (uint8_t *)conversationId.data()));  // conversationId
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    mapClient->Get(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto resp = obsClient->respHeader_->GetItemAppParams();
    EXPECT_THAT(resp->GetTlvtriplet(0x1C) != nullptr, true);
    EXPECT_THAT(resp->GetTlvtriplet(0x1D) != nullptr, true);
    EXPECT_THAT(resp->GetTlvtriplet(0x1E) != nullptr, true);
    EXPECT_THAT(resp->GetTlvtriplet(0x21) != nullptr, true);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetMessage)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    paths.push_back(u"inbox");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    ObexTlvParamters appParas;
    std::u16string handle = u"0220201223134517";
    mseReq->AppendItemName(handle);
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x0));  // attachment 0:OFF 1:ON
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x15, (uint8_t)0x0));  // fractionRequest 0:first 1:next
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto resp = obsClient->respHeader_->GetItemAppParams();
    if (resp != nullptr) {
        EXPECT_THAT(obsClient->respHeader_->GetItemAppParams()->GetTlvtriplet(0x16) != nullptr, true);
    }
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetMessageIm)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    uint32_t connectId = connectIdItem->GetWord();
    std::vector<std::u16string> paths;
    paths.push_back(u"telecom");
    paths.push_back(u"msg");
    paths.push_back(u"inbox");
    obsClient->ResetCallCount();
    mapClient->SetPath(paths);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnActionCompleted CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectId);
    mseReq->AppendItemType("x-bt/message");
    ObexTlvParamters appParas;
    std::u16string handle = u"1020210121061916";
    mseReq->AppendItemName(handle);
    appParas.AppendTlvtriplet(TlvTriplet(0x0A, (uint8_t)0x0));  // attachment 0:OFF 1:ON
    appParas.AppendTlvtriplet(TlvTriplet(0x14, (uint8_t)0x1));  // charset
    appParas.AppendTlvtriplet(TlvTriplet(0x15, (uint8_t)0x0));  // fractionRequest 0:first 1:next
    mseReq->AppendItemAppParams(appParas);

    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto resp = obsClient->respHeader_->GetItemAppParams();
    if (resp != nullptr) {
        EXPECT_THAT(obsClient->respHeader_->GetItemAppParams()->GetTlvtriplet(0x16) != nullptr, true);
    }
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, UpdateInbox)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(2);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    obsClient->ResetCallCount();
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    mseReq->AppendItemType("x-bt/MAP-messageUpdate");
    ObexTlvParamters appParas;
    std::u16string name = u"inbox";
    mseReq->AppendItemName(name);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, GetMASInstanceInformation)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    stub::MapService::GetInstance()->SetMseInstance(2);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();

    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    mseReq->AppendItemType("x-bt/MASInstanceInformation");
    ObexTlvParamters appParas;
    appParas.AppendTlvtriplet(TlvTriplet(0x0F, (uint8_t)0x1));  // MASInstanceId
    mseReq->AppendItemAppParams(appParas);
    obsClient->ResetCallCount();
    auto bodyObj = std::make_shared<ObexArrayBodyObject>();
    EXPECT_THAT(mapClient->Get(*mseReq, bodyObj), 0);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    EXPECT_THAT(obsClient->callCountOnActionCompleted, 1);
    EXPECT_THAT(obsClient->respHeader_->GetFieldCode(), 0xA0);
    auto &extendBody = obsClient->respHeader_->GetExtendBodyObject();
    EXPECT_THAT(extendBody != nullptr, true);
    EXPECT_THAT(extendBody.get(), bodyObj.get());
    PrintObexBodyObject(extendBody);
    MseCloseServer(*mseService, *contentObserver);
}

TEST(MapMseService_test, SendEvent)
{
    LOG_INFO("%s Enter", __PRETTY_FUNCTION__);
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("map_mse_client");
    bluetooth::MseMock::ReSetRfcomm();
    std::unique_ptr<MapMseService> mseService = std::make_unique<MapMseService>();

    auto observer = std::make_unique<MapMseObserver>();
    mseService->RegisterObserver(*observer);
    auto contentObserver = std::make_unique<TestContentObserver>();
    contentObserver->ResetCallCount();
    mseService->RegisterCallback(*contentObserver);
    mseService->Enable();
    if (!DataTestUtil::WaitFor([&]() { return contentObserver->callCountOnEnable == 1; })) {
        std::cout << "OnEnable CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    AdapterManager::GetInstance()->SetMessagePermission("00:00:01:00:00:7F", BTPermissionType::ACCESS_ALLOWED);
    auto obsClient = std::make_unique<TestObexClientObserver>();
    auto mapClient = StartTestObexMpClient(obsClient.get(), clientDispatcher.get());

    auto serverDispatcher = std::make_unique<utility::Dispatcher>("map_mns_server");
    auto mnsObserver = std::make_unique<TestMnsObserver>();
    mnsObserver->ResetCallCount();
    auto mnsServer = StartMnsServer(mnsObserver.get(), serverDispatcher.get(), 0x1021);
    EXPECT_EQ(0, mnsServer->Startup());
    sleep(3);
    ObexTlvParamters appParamters;
    uint32_t mapSupportedFeature = 0x07F;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)mapSupportedFeature));
    ObexConnectParams obxPara = {.appParams_ = &appParamters};
    EXPECT_EQ(0, mapClient->Connect(obxPara));
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnConnected == 1; })) {
        std::cout << "CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    auto connectIdItem = obsClient->respHeader_->GetItemConnectionId();
    auto mseReq = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
    mseReq->AppendItemConnectionId(connectIdItem->GetWord());
    mseReq->AppendItemType("x-bt/MAP-NotificationRegistration");
    ObexTlvParamters appParamters1;
    appParamters1.AppendTlvtriplet(TlvTriplet(0x0E, (uint8_t)0x01));
    mseReq->AppendItemAppParams(appParamters1);
    uint8_t body = 0x30;
    mseReq->AppendItemEndBody(&body, 1);
    mapClient->Put(*mseReq);
    if (!DataTestUtil::WaitFor([&]() { return obsClient->callCountOnActionCompleted == 1; })) {
        std::cout << "OnPut CloseServer" << std::endl;
        MseCloseServer(*mseService, *contentObserver);
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    if (!DataTestUtil::WaitFor([&]() { return mnsObserver->callCountOnConnected == 1; })) {
        std::cout << "Mns OnConnected CloseServer" << std::endl;
        mnsServer->Shutdown();
        GTEST_FATAL_FAILURE_("time out");
        return;
    }
    for (int i = 0; i < 15; i++) {
        TestSentEvent(*mnsServer, *mnsObserver, i);
    }
    sleep(1);
    MseCloseServer(*mseService, *contentObserver);
    mnsServer->Shutdown();
    sleep(2);
}
