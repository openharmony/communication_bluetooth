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
#include "data_test_util.h"
#include "common_mock_all.h"
#include "message.h"
#include "pbap_pce_service.h"
#include "pbap_pse_service.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace testing;
using namespace bluetooth;
namespace pbap_pse_test {
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
class TestPbapPceObserver : public IPbapPceObserver {
public:
    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pce test remoteAddr:" << remoteAddr.GetAddress() << " state:" << state << std::endl;
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
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        callCountOnPasswordRequired++;
    }
    void OnActionCompleted(
        const RawAddress &device, int respCode, int actionType, const IPbapPhoneBookData &result) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
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
        std::cout << "pse test remoteAddr:" << remoteAddr.GetAddress() << std::endl;
        callCountOnServicePermission++;
    }

    void OnServicePasswordRequired(const RawAddress &remoteAddr, const std::vector<uint8_t> &description,
        uint8_t charset, bool fullAccess) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pse test remoteAddr:" << remoteAddr.GetAddress() << std::endl;
        callCountOnPasswordRequired++;
    }

    void ResetCallCount()
    {
        callCountOnServiceConnected = 0;
        callCountOnServiceDisconnected = 0;
        callCountOnServicePermission = 0;
        callCountOnPasswordRequired = 0;
    }

    int callCountOnServiceConnected{0};
    int callCountOnServiceDisconnected{0};
    int callCountOnServicePermission{0};
    int callCountOnPasswordRequired{0};
};
void CloseServerAndClient(bluetooth::PbapPseService &server, bluetooth::PbapPceService &client,
    ContextCallback &callBackForServer, ContextCallback &callBackForClient)
{
    std::cout << "CloseServerAndClient start" << std::endl;
    callBackForClient.Reset();
    callBackForServer.Reset();
    client.Disable();
    std::cout << "client.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForClient.callCountOnDisable_ == 1; })) {
        return;
    };
    server.Disable();
    std::cout << "server.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.callCountOnDisable_ == 1; })) {
        return;
    };
    std::cout << "CloseServerAndClient end" << std::endl;
}
}  // namespace pbap_pse_test

TEST(pbap_pse_test, startup_shutdown)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    server.Enable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    EXPECT_THAT(server.GetMaxConnectNum(), -1);

    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnDisable_ == 1; })) {
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
}

TEST(pbap_pse_test, Connect_Disconnect_WithoutObserver)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    RawAddress rawAddress("00:00:01:00:00:7F");
    std::cout << "client.Connect(rawAddress)" << std::endl;

    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    std::cout << "server.Disable())" << std::endl;
    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnDisable_ == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    std::cout << "client.Disable())" << std::endl;
    client.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForClient->callCountOnDisable_ == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
}

TEST(pbap_pse_test, Connect_Disconnect_withObserver)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    observerForServer->ResetCallCount();

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);

    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    RawAddress rawAddress("00:00:01:00:00:7F");
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    std::cout << "start connect" << std::endl;
    EXPECT_THAT(client.Connect(rawAddress), 0);
    std::cout << "sleep start" << std::endl;
    if (!DataTestUtil::WaitFor([&]() {
            return observerForClient->callCountOnConnected == 1 && observerForServer->callCountOnServiceConnected == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    std::cout << "sleep end" << std::endl;
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 0);

    // Disconnect use not exists device
    EXPECT_THAT(server.Disconnect(RawAddress("00:00:01:00:00:7B")), RET_BAD_STATUS);

    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    std::cout << "server.Disconnect start" << std::endl;
    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() {
            return observerForClient->callCountOnDisconnected == 1 &&
                   observerForServer->callCountOnServiceDisconnected == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    std::cout << "server.Disconnect end" << std::endl;

    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 1);

    // Disconnect not connected device
    EXPECT_THAT(server.Disconnect(rawAddress), RET_BAD_STATUS);

    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}

TEST(pbap_pse_test, GetContext)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    EXPECT_THAT(server.GetContext(), &server);
}

TEST(pbap_pse_test, Connect)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    RawAddress rawAddress("00:00:01:00:00:7F");
    EXPECT_THAT(server.Connect(rawAddress), RET_NO_SUPPORT);
}

TEST(pbap_pse_test, RegisterObserver_DeregisterObserver)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    observerForServer->ResetCallCount();

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    RawAddress rawAddress("00:00:01:00:00:7F");
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() {
            return observerForClient->callCountOnConnected == 1 && observerForServer->callCountOnServiceConnected == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 0);

    // Change RegisterObserver
    auto observerForServerNew = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.DeregisterObserver(*observerForServer);
    server.RegisterObserver(*observerForServerNew);

    observerForServerNew->ResetCallCount();
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() {
            return observerForClient->callCountOnDisconnected == 1 &&
                   observerForServerNew->callCountOnServiceDisconnected == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServerNew->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServerNew->callCountOnServiceDisconnected, 1);
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 0);

    // DeregisterObserver
    server.DeregisterObserver(*observerForServerNew);
    observerForServerNew->ResetCallCount();
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    server.RegisterObserver(*observerForServerNew);
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() {
            return observerForClient->callCountOnConnected == 1 &&
                   observerForServerNew->callCountOnServiceConnected == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServerNew->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForServerNew->callCountOnServiceDisconnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 0);

    server.DeregisterObserver(*observerForServerNew);
    observerForServerNew->ResetCallCount();
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    sleep(5);
    EXPECT_THAT(observerForServerNew->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServerNew->callCountOnServiceDisconnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServiceDisconnected, 0);

    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}

TEST(pbap_pse_test, GetConnectState_GetConnectDevices_GetDevicesByStates)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    RawAddress rawAddress("00:00:01:00:00:7F");
    std::cout << "client.Connect(rawAddress)" << std::endl;

    EXPECT_THAT(server.GetConnectState(), 0);
    EXPECT_THAT(server.GetDeviceState(rawAddress), static_cast<int>(BTConnectState::DISCONNECTED));

    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    EXPECT_THAT(server.GetConnectState(), PROFILE_STATE_CONNECTED);
    EXPECT_THAT(server.GetDeviceState(rawAddress), static_cast<int>(BTConnectState::CONNECTED));

    // GetConnectDevices
    std::list<RawAddress> connectDevices = server.GetConnectDevices();
    EXPECT_THAT(connectDevices.size(), 1);
    EXPECT_THAT(connectDevices.front().GetAddress(), rawAddress.GetAddress());
    std::vector<int> states = {1};  // Connected
    std::vector<RawAddress> devicesByStates = server.GetDevicesByStates(states);
    EXPECT_THAT(devicesByStates.size(), 1);
    EXPECT_THAT(devicesByStates.front().GetAddress(), rawAddress.GetAddress());

    std::cout << "server.Disconnect(rawAddress)" << std::endl;

    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnDisconnected == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(server.GetConnectState(), PROFILE_STATE_DISCONNECTED);
    EXPECT_THAT(server.GetDeviceState(rawAddress), static_cast<int>(BTConnectState::DISCONNECTED));

    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}

TEST(pbap_pse_test, Connect_GrantPermission)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    RawAddress rawAddress("00:00:01:00:00:7F");
    auto adapterManager = AdapterManager::GetInstance();
    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    std::cout << "client.Connect(rawAddress)" << std::endl;
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServicePermission, 1);
    std::cout << "server.GrantPermission(rawAddress, true)" << std::endl;
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(server.GetConnectState(), PROFILE_STATE_CONNECTING);
    EXPECT_THAT(server.GetDeviceState(rawAddress), static_cast<int>(BTConnectState::CONNECTING));

    server.GrantPermission(rawAddress, true, true);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    // not exists device for coverage 
    server.GrantPermission(RawAddress("00:00:01:00:00:7B"), false, true);
    // wrong state device for coverage 
    server.GrantPermission(rawAddress, true, true);

    EXPECT_THAT(AdapterManager::GetInstance()->GetPhonebookPermission(rawAddress.GetAddress()),
        BTPermissionType::ACCESS_ALLOWED);
    std::cout << "server.Disconnect(rawAddress)" << std::endl;
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() {
            return observerForServer->callCountOnServiceDisconnected == 1 &&
                   observerForClient->callCountOnDisconnected == 1;
        })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServicePermission, 1);
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    std::cout << "server.GrantPermission(rawAddress, false)" << std::endl;
    server.GrantPermission(rawAddress, false, true);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnDisconnected == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    EXPECT_THAT(adapterManager->GetPhonebookPermission(rawAddress.GetAddress()), BTPermissionType::ACCESS_FORBIDDEN);

    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    std::cout << "client.Connect" << std::endl;
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServiceDisconnected == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    std::cout << "SetPhonebookPermission ACCESS_UNKNOWN" << std::endl;
    // servier disabled while connecting
    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    std::cout << "client.Connect" << std::endl;
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    std::cout << "server.Disable" << std::endl;
    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnDisable_ == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}

TEST(pbap_pse_test, Connect_GrantPermission_Timeout)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    RawAddress rawAddress("00:00:01:00:00:7F");
    auto adapterManager = AdapterManager::GetInstance();
    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    std::cout << "client.Connect(rawAddress)" << std::endl;
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 0);
    EXPECT_THAT(observerForServer->callCountOnServicePermission, 1);
    std::cout << "server.GrantPermission(rawAddress, true)" << std::endl;
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    std::cout << "waiting timeout disconnect" << std::endl;
    if (!DataTestUtil::WaitFor(
            [&]() {
                return observerForClient->callCountOnDisconnected == 1 &&
                       observerForServer->callCountOnServiceDisconnected == 1;
            },
            60)) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    std::cout << "waiting disconnect ok" << std::endl;

    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}

TEST(pbap_pse_test, SetConnectionStrategy_GetConnectionStrategy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    // start pce service
    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pse_test::ContextCallback>();
    client.RegisterCallback(*callBackForClient);
    auto observerForClient = std::make_unique<pbap_pse_test::TestPbapPceObserver>();
    client.RegisterObserver(*observerForClient);
    client.Enable();
    if (!DataTestUtil::WaitFor([&]() {
            return callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1;
        })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    RawAddress rawAddress("00:00:01:00:00:7F");
    std::cout << "client.Connect(rawAddress)" << std::endl;

    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(server.SetConnectionStrategy(rawAddress, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)), 0);
    EXPECT_THAT(server.GetConnectionStrategy(rawAddress), static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnDisconnected == 1; })) {
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    EXPECT_THAT(server.SetConnectionStrategy(rawAddress, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)), 0);
    EXPECT_THAT(server.GetConnectionStrategy(rawAddress), static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    // SetConnectionStrategy int server Permissioning: no accept
    auto adapterManager = AdapterManager::GetInstance();
    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);

    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(server.SetConnectionStrategy(rawAddress, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)), 0);
    EXPECT_THAT(server.GetConnectionStrategy(rawAddress), static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnDisconnected == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };

    // SetConnectionStrategy int server Permissioning: accepted but no connected
    observerForServer->ResetCallCount();
    observerForClient->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddress), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForServer->callCountOnServicePermission == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    // accept connect
    server.GrantPermission(rawAddress, true, false);

    EXPECT_THAT(server.SetConnectionStrategy(rawAddress, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)), 0);
    EXPECT_THAT(server.GetConnectionStrategy(rawAddress), static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnDisconnected == 1; })) {
        adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
        pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    // can't set unknown strategy
    EXPECT_THAT(
        server.SetConnectionStrategy(rawAddress, static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN)), RET_BAD_PARAM);

    RawAddress notExistsRawAddress("00:00:01:00:00:11");
    EXPECT_THAT(server.SetConnectionStrategy(notExistsRawAddress, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)),
        RET_NO_SUPPORT);
    EXPECT_THAT(
        server.GetConnectionStrategy(notExistsRawAddress), static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN));

    adapterManager->SetPhonebookPermission(rawAddress.GetAddress(), BTPermissionType::ACCESS_ALLOWED);
    pbap_pse_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient);
}