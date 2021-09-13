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
#include <iostream>
#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "adapter_manager.h"
#include "message.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "data_test_util.h"
#include "pbap_pce_service.h"
#include "pbap_pse_service.h"

using namespace testing;
using namespace bluetooth;

namespace pbap_pce_pullphonebook_test {
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

    int GetCallCountOnEnable()
    {
        return callCountOnEnable_;
    }
    int GetCallCountOnDisable()
    {
        return callCountOnDisable_;
    }
    int GetLastRet()
    {
        return lastRet_;
    }

private:
    int callCountOnEnable_{0};
    int callCountOnDisable_{0};
    int lastRet_ = -1;
};
class TestPbapPceObserverImpl : public IPbapPceObserver {
public:
    explicit TestPbapPceObserverImpl()
    {
    }
    virtual ~TestPbapPceObserverImpl() = default;
    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        LOG_INFO("pce test remoteAddr:%s state:%d", remoteAddr.GetAddress().c_str(), state);
        switch (state) {
            case static_cast<int>(BTConnectState::CONNECTING):
                break;
            case static_cast<int>(BTConnectState::CONNECTED):
                callCountOnConnected_++;
                break;
            case static_cast<int>(BTConnectState::DISCONNECTING):
                break;
            case static_cast<int>(BTConnectState::DISCONNECTED):
                callCountOnDisconnected_++;
                break;
            default:
                break;
        }
    }
    void OnServicePasswordRequired(
        const RawAddress &device, const std::vector<uint8_t> &description, uint8_t charset, bool fullAccess) override
    {
        callCountOnPasswordRequired_++;
    }
    void OnActionCompleted(
        const RawAddress &device, int respCode, int actionType, const IPbapPhoneBookData &result) override
    {
        callCountOnActionCompleted_++;
        auto &res = (IPbapPhoneBookData &)(result);
        res.Load(res.result_);
        respCode_ = respCode;
    }
    void ResetCallCount()
    {
        callCountOnConnected_ = 0;
        callCountOnActionCompleted_ = 0;
        callCountOnDisconnected_ = 0;
        callCountOnPasswordRequired_ = 0;
        respCode_ = 0;
    }
    int GetCallCountOnConnected()
    {
        return callCountOnConnected_;
    }
    int GetCallCountOnActionCompleted()
    {
        return callCountOnActionCompleted_;
    }
    int GetCallCountOnDisconnected()
    {
        return callCountOnDisconnected_;
    }
    int GetCallCountOnPasswordRequired()
    {
        return callCountOnPasswordRequired_;
    }
    int GetRespCode()
    {
        return respCode_;
    }

private:
    int callCountOnConnected_{0};
    int callCountOnActionCompleted_{0};
    int callCountOnDisconnected_{0};
    int callCountOnPasswordRequired_{0};
    int respCode_ = 0;
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
                callCountOnConnected_++;
                break;
            case static_cast<int>(BTConnectState::DISCONNECTING):
                break;
            case static_cast<int>(BTConnectState::DISCONNECTED):
                callCountOnDisconnected_++;
                break;
            default:
                break;
        }
    }
    void OnServicePermission(const RawAddress &remoteAddr) override
    {
        callCountOnPermission_++;
    }

    void OnServicePasswordRequired(const RawAddress &remoteAddr, const std::vector<uint8_t> &description,
        uint8_t charset, bool fullAccess) override
    {
        callCountOnPasswordRequired_++;
    }

    void ResetCallCount()
    {
        callCountOnConnected_ = 0;
        callCountOnDisconnected_ = 0;
        callCountOnPermission_ = 0;
        callCountOnPasswordRequired_ = 0;
    }
    int GetCallCountOnServiceConnected()
    {
        return callCountOnConnected_;
    }
    int GetCallCountOnServiceDisconnected()
    {
        return callCountOnDisconnected_;
    }
    int GetCallCountOnServicePermission()
    {
        return callCountOnPermission_;
    }
    int GetCallCountOnPasswordRequired()
    {
        return callCountOnPasswordRequired_;
    }

private:
    int callCountOnConnected_{0};
    int callCountOnDisconnected_{0};
    int callCountOnPermission_{0};
    int callCountOnPasswordRequired_{0};
};
void CloseServerAndClient(bluetooth::PbapPseService &server, bluetooth::PbapPceService &client,
    ContextCallback &callBackForServer, ContextCallback &callBackForClient)
{
    std::cout << "CloseServerAndClient start" << std::endl;
    callBackForClient.Reset();
    callBackForServer.Reset();
    client.Disable();
    std::cout << "client.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForClient.GetCallCountOnDisable() == 1; })) {
        return;
    }
    server.Disable();
    std::cout << "server.Disable" << std::endl;
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.GetCallCountOnDisable() == 1; })) {
        return;
    }
    std::cout << "CloseServerAndClient end" << std::endl;
}
};  // namespace pbap_pce_pullphonebook_test

#define CLOSE_FUNC pbap_pce_pullphonebook_test::CloseServerAndClient
#define CLOSE_PB_SERVER [&]() { CLOSE_FUNC(server, client, *callBackForServer, *callBackForClient); }

#define IS_PCE_ENABLE (callBackForClient->GetCallCountOnEnable() == 1)
#define IS_PSE_ENABLE (callBackForServer->GetCallCountOnEnable() == 1)
#define EXP_PB_CS_ENABLE (IS_PCE_ENABLE && IS_PSE_ENABLE)

#define PB_ACTION_EXPECT                                                                                             \
    do {                                                                                                             \
        WAIT_FOR(observerForClient->GetCallCountOnActionCompleted() == 1, CLOSE_PB_SERVER);                          \
        EXPECT_THAT(observerForClient->GetCallCountOnActionCompleted(), 1);                                          \
        EXPECT_THAT(                                                                                                 \
            (observerForClient->GetRespCode() == PBAP_SUCCESS || observerForClient->GetRespCode() == PBAP_CONTINUE), \
            true);                                                                                                   \
    } while (0)

void SetPullPhoneBookSizeParam(IPbapPullPhoneBookParam &param)
{
    param.SetName(u"telecom/pb.vcf");
    param.SetPropertySelector(1 << 2 | 1 << 5 | 1 << 8);  // N,ADR,EMAIL
    param.SetFormat(PBAP_FORMAT_VCARD3_0);
    param.SetMaxListCount(0);
    param.SetListStartOffset(0);
    param.SetResetNewMissedCalls(0);
    param.SetvCardSelector(1 << 5 | 1 << 8);  // ADR,EMAIL
    param.SetvCardSelectorOp(PBAP_SELECTOR_OPERATOR_AND);
}

void SetPullPhoneBookParam(IPbapPullPhoneBookParam &param)
{
    param.SetName(u"telecom/pb.vcf");
    param.SetPropertySelector(1 << 2 | 1 << 5 | 1 << 8);  // N,ADR,EMAIL
    param.SetFormat(PBAP_FORMAT_VCARD3_0);
    param.SetMaxListCount(3);
    param.SetListStartOffset(0);
    param.SetResetNewMissedCalls(0);
    param.SetvCardSelector(1 << 5 | 1 << 8);  // ADR,EMAIL
    param.SetvCardSelectorOp(PBAP_SELECTOR_OPERATOR_AND);
}

TEST(pbap_pce_pullphonebook_test, PullPhoneBook)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXP_PB_CS_ENABLE, CLOSE_PB_SERVER);

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnConnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnConnected(), 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    // phonebooksize
    IPbapPullPhoneBookParam param;
    SetPullPhoneBookSizeParam(param);
    observerForClient->ResetCallCount();
    retVal = client.PullPhoneBook(rawAddr, param);
    PB_ACTION_EXPECT;

    // pull phonebook
    observerForClient->ResetCallCount();
    SetPullPhoneBookParam(param);
    retVal = client.PullPhoneBook(rawAddr, param);
    PB_ACTION_EXPECT;
    EXPECT_THAT(client.IsDownloading(rawAddr), false);

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnDisconnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnDisconnected(), 1);

    CLOSE_PB_SERVER();
}

void SetPullPhoneBookParamForAbort(IPbapPullPhoneBookParam &param)
{
    param.SetName(u"telecom/pb.vcf");
    param.SetPropertySelector(1 << 2 | 1 << 5 | 1 << 8);  // N,ADR,EMAIL
    param.SetFormat(PBAP_FORMAT_VCARD3_0);
    param.SetMaxListCount(3);
    param.SetListStartOffset(0);
    param.SetResetNewMissedCalls(0);
    param.SetvCardSelector(1 << 5 | 1 << 8);  // ADR,EMAIL
    param.SetvCardSelectorOp(PBAP_SELECTOR_OPERATOR_AND);
}

TEST(pbap_pce_pullphonebook_test, AbortDownloading)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXP_PB_CS_ENABLE, CLOSE_PB_SERVER);

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnConnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnConnected(), 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    // phonebook
    observerForClient->ResetCallCount();
    IPbapPullPhoneBookParam param;
    SetPullPhoneBookParamForAbort(param);
    retVal = client.PullPhoneBook(rawAddr, param);
    retVal = client.AbortDownloading(rawAddr);
    PB_ACTION_EXPECT;

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnDisconnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnDisconnected(), 1);

    CLOSE_PB_SERVER();
}

TEST(pbap_pce_pullphonebook_test, SetPhoneBook)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXP_PB_CS_ENABLE, CLOSE_PB_SERVER);

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnConnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnConnected(), 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = 0;
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    // absolute path
    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"telecom/pb", PBAP_FLAG_GO_DOWN);

    WAIT_FOR(observerForClient->GetCallCountOnActionCompleted() == 1, CLOSE_PB_SERVER);
    PB_ACTION_EXPECT;

    // go up
    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"", PBAP_FLAG_GO_UP);
    PB_ACTION_EXPECT;

    // go down
    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"ich", PBAP_FLAG_GO_DOWN);
    PB_ACTION_EXPECT;

    // go back to root
    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"", PBAP_FLAG_GO_TO_ROOT);
    PB_ACTION_EXPECT;

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnDisconnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnDisconnected(), 1);

    CLOSE_PB_SERVER();
}

void setPullvCardListingParam(IPbapPullvCardListingParam &param)
{
    param.SetName(u"");
    param.SetOrder(PBAP_ORDER_ALPHANUMERIC);
    param.SetSearchValue("34");
    param.SetSearchProperty(PBAP_SEARCH_PROPERTY_NUMBER);
    param.SetMaxListCount(0);
    param.SetListStartOffset(0);
    param.SetResetNewMissedCalls(0);
    param.SetvCardSelector(0);
    param.SetvCardSelectorOp(0);
}

TEST(pbap_pce_pullphonebook_test, PullvCardListing)
{
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXP_PB_CS_ENABLE, CLOSE_PB_SERVER);

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnConnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnConnected(), 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"telecom/pb", PBAP_FLAG_GO_DOWN);
    PB_ACTION_EXPECT;

    observerForClient->ResetCallCount();
    IPbapPullvCardListingParam param;
    setPullvCardListingParam(param);
    retVal = client.PullvCardListing(rawAddr, param);
    PB_ACTION_EXPECT;

    observerForClient->ResetCallCount();
    param.SetMaxListCount(20);
    retVal = client.PullvCardListing(rawAddr, param);
    PB_ACTION_EXPECT;

    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnDisconnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnDisconnected(), 1);

    CLOSE_PB_SERVER();
}

TEST(pbap_pce_pullphonebook_test, PullvCardEntry)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // bt enable
    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForServer = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);
    server.RegisterCallback(*callBackForServer);
    server.Enable();

    PbapPceService client;
    auto callBackForClient = std::make_unique<pbap_pce_pullphonebook_test::ContextCallback>();
    auto observerForClient = std::make_unique<pbap_pce_pullphonebook_test::TestPbapPceObserverImpl>();
    client.RegisterCallback(*callBackForClient);
    client.RegisterObserver(*observerForClient);
    client.Enable();
    WAIT_FOR(EXP_PB_CS_ENABLE, CLOSE_PB_SERVER);

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);
    observerForClient->ResetCallCount();
    client.Connect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnConnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnConnected(), 1);

    int retVal = client.GetConnectState();
    EXPECT_THAT(retVal, PROFILE_STATE_CONNECTED);
    retVal = client.GetDeviceState(rawAddr);
    EXPECT_THAT(retVal, static_cast<int>(BTConnectState::CONNECTED));

    // Set Path
    observerForClient->ResetCallCount();
    retVal = client.SetPhoneBook(rawAddr, u"telecom/pb", PBAP_FLAG_GO_DOWN);
    PB_ACTION_EXPECT;

    // PullvCardEntry
    observerForClient->ResetCallCount();
    IPbapPullvCardEntryParam param;
    param.SetName(u"X-BT-UID:A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6");
    param.SetPropertySelector(0);
    param.SetFormat(PBAP_FORMAT_VCARD2_1);
    retVal = client.PullvCardEntry(rawAddr, param);
    PB_ACTION_EXPECT;

    // Disconnect
    observerForClient->ResetCallCount();
    client.Disconnect(rawAddr);
    WAIT_FOR(observerForClient->GetCallCountOnDisconnected() == 1, CLOSE_PB_SERVER);
    EXPECT_THAT(observerForClient->GetCallCountOnDisconnected(), 1);

    CLOSE_PB_SERVER();
}