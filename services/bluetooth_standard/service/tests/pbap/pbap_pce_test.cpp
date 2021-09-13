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
#include <memory>
#include <string>
#include <iostream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "adapter_manager.h"
#include "message.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "pbap_pce_def.h"
#include "data_test_util.h"
#include "pbap_pce_service.h"
#include "pbap_pse_service.h"

using namespace testing;
using namespace bluetooth;

namespace pbap_pce_test {
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
    TestPbapPseObserverImpl() = default;
    virtual ~TestPbapPseObserverImpl() = default;
    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
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
        callCountOnServicePermission++;
    }

    void OnServicePasswordRequired(const RawAddress &remoteAddr, const std::vector<uint8_t> &description,
        uint8_t charset, bool fullAccess) override
    {
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

class TestPbapPceObserverImpl : public IPbapPceObserver {
public:
    TestPbapPceObserverImpl() = default;
    virtual ~TestPbapPceObserverImpl() = default;
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
    }
    server.Disable();
    std::cout << "server.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.callCountOnDisable_ == 1; })) {
        return;
    }
    std::cout << "CloseServerAndClient end" << std::endl;
}
}  // namespace pbap_pce_test
#define CLOSE_PCE_SERVER() \
    [&]() { pbap_pce_test::CloseServerAndClient(server, client, *callBackForServer, *callBackForClient); }

#define EXPECT_CS_ENABLE (callBackForServer->callCountOnEnable_ == 1 && callBackForClient->callCountOnEnable_ == 1)

TEST(pbap_pce_test, Connect_Disconnect)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXPECT_CS_ENABLE, CLOSE_PCE_SERVER());

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    EXPECT_THAT(client.GetConnectDevices().size() > 0, true);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));
    retVal = client.Connect(rawAddr);
    EXPECT_THAT(retVal, RET_BAD_STATUS);

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);
    retVal = client.Disconnect(rawAddr);
    EXPECT_THAT(retVal, RET_BAD_STATUS);

    // start pce service
    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddr), 0);

    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    WAIT_FOR(observerForServer->callCountOnServiceConnected == 1, CLOSE_PCE_SERVER());
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);

    auto close_test = CLOSE_PCE_SERVER();
    close_test();
}

TEST(pbap_pce_test, SetConnectionStrategy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXPECT_CS_ENABLE, CLOSE_PCE_SERVER());

    // connect
    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);
    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    // SetConnectionStrategy
    observerForClient->ResetCallCount();
    retVal = client.SetConnectionStrategy(rawAddr, 100);
    EXPECT_THAT(retVal, RET_BAD_PARAM);

    observerForClient->ResetCallCount();
    client.SetConnectionStrategy(rawAddr, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);
    retVal = client.GetConnectionStrategy(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN));

    observerForClient->ResetCallCount();
    client.SetConnectionStrategy(rawAddr, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);
    retVal = client.GetConnectionStrategy(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));

    // disconnect
    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);

    // start pce service and change observer
    observerForClient->ResetCallCount();
    observerForServer->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddr), 0);
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    WAIT_FOR(observerForServer->callCountOnServiceConnected == 1, CLOSE_PCE_SERVER());
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 1);

    auto close_test = CLOSE_PCE_SERVER();
    close_test();
}

TEST(pbap_pce_test, RegisterObserver_DeregisterObserver)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXPECT_CS_ENABLE, CLOSE_PCE_SERVER());

    RawAddress rawAddress("00:00:01:00:00:7F");
    observerForClient->ResetCallCount();
    EXPECT_THAT(client.Connect(rawAddress), 0);
    WAIT_FOR(observerForClient->callCountOnConnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 0);

    // Change RegisterObserver
    client.DeregisterObserver(*observerForClient);
    auto observerForClientNew = std::make_unique<pbap_pce_test::TestPbapPceObserverImpl>();
    client.RegisterObserver(*observerForClientNew);

    observerForClientNew->ResetCallCount();
    observerForClient->ResetCallCount();
    EXPECT_THAT(server.Disconnect(rawAddress), 0);
    WAIT_FOR(observerForClientNew->callCountOnDisconnected == 1, CLOSE_PCE_SERVER());
    EXPECT_THAT(observerForClientNew->callCountOnConnected, 0);
    EXPECT_THAT(observerForClientNew->callCountOnDisconnected, 1);
    EXPECT_THAT(observerForClient->callCountOnConnected, 0);
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 0);

    // DeregisterObserver
    client.DeregisterObserver(*observerForClientNew);
    observerForClientNew->ResetCallCount();
    observerForClient->ResetCallCount();

    EXPECT_THAT(client.Connect(rawAddress), 0);
    sleep(5);
    EXPECT_THAT(observerForClientNew->callCountOnConnected, 0);
    EXPECT_THAT(observerForClientNew->callCountOnDisconnected, 0);
    EXPECT_THAT(observerForClient->callCountOnConnected, 0);
    EXPECT_THAT(observerForClient->callCountOnDisconnected, 0);
    auto close_test = CLOSE_PCE_SERVER();
    close_test();
}

TEST(pbap_pce_test, GetContext)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    PbapPceService server;
    EXPECT_THAT(server.GetContext(), &server);
}
