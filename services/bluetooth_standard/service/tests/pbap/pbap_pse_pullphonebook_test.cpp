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
#include "message.h"
#include "obex_mp_client.h"
#include "obex_utils.h"
#include "pbap_pse_service.h"
#include "securec.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace testing;
using namespace bluetooth;
namespace pbap_pse_pullphonebook_test {
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

class TestObexClientObserver : public ObexClientObserver {
public:
    void OnConnected(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnected" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnected++;
    }
    /**
     * @brief Called OnConnectFailed
     *
     * @param client ObexClient
     * @param resp The Response from Server
     */
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnConnectFailed" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnConnectFailed++;
    }
    void OnDisconnected(ObexClient &client) override
    {
        std::cout << "TestObexClientObserver::OnDisconnected" << std::endl;
        callCountOnDisconnected++;
    }
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override
    {
        std::cout << "TestObexClientObserver::OnActionCompleted" << std::endl;
        lastRespHeader_ = std ::make_unique<ObexHeader>(resp);
        callCountOnActionCompleted++;
    }
    void OnTransportFailed(ObexClient &client, int errCd) override
    {
        std::cout << "TestObexClientObserver::OnTransportFailed" << std::endl;
        callCountOnTransportFailed++;
    }
    void ResetCallCount()
    {
        callCountOnConnected = 0;
        callCountOnActionCompleted = 0;
        callCountOnDisconnected = 0;
        callCountOnTransportFailed = 0;
        callCountOnConnectFailed = 0;
        lastRespHeader_ = nullptr;
    }

    int callCountOnConnected{0};
    int callCountOnDisconnected{0};
    int callCountOnActionCompleted{0};
    int callCountOnTransportFailed{0};
    int callCountOnConnectFailed{0};
    std::unique_ptr<ObexHeader> lastRespHeader_{nullptr};
};

std::unique_ptr<bluetooth::ObexMpClient> StartTestPceObexMpClient(
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

    auto obexMpClient = std::make_unique<ObexMpClient>(configRfcomm, observer, dispatcher);
    std::cout << "pbap_pse_pullphonebook_test::StartTestPceObexMpClient:Rfcomm" << std::endl;
    return obexMpClient;
}

void CloseServer(bluetooth::PbapPseService &server, ContextCallback &callBackForServer)
{
    std::cout << "CloseServer" << std::endl;
    callBackForServer.Reset();
    server.Disable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer.callCountOnDisable_ == 1; })) {
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
}  // namespace pbap_pse_pullphonebook_test

TEST(pbap_pse_pullphonebook_test, PullPhoneBook)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_pullphonebook_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    auto observerForClient = std::make_unique<pbap_pse_pullphonebook_test::TestObexClientObserver>();

    // start pce test obex client
    auto client = pbap_pse_pullphonebook_test::StartTestPceObexMpClient(*observerForClient, *clientDispatcher);
    ObexTlvParamters appParamters;
    uint32_t pceSupportedFeature = 0x03FF;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)pceSupportedFeature));
    {
        ObexConnectParams connectParams;
        connectParams.appParams_ = &appParamters;
        EXPECT_THAT(client->Connect(connectParams), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
        EXPECT_THAT(observerForClient->callCountOnConnected, 1);
    }
    auto connectIdItem = observerForClient->lastRespHeader_->GetItemConnectionId();
    auto pbReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    pbReq->AppendItemConnectionId(connectIdItem->GetWord());
    {
        // no type item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // no name item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->AppendItemType("x-bt/phonebook");
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // wrong name item
        pbReq->AppendItemName(u"pb.txt");
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    pbReq->RemoveItem(ObexHeader::NAME);
    pbReq->AppendItemName(u"telecom/pb.vcf");
    {
        // wrong app params
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0xFF, (uint16_t)0));
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // maxListCount 0
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::cout << "GetPhoneBookSize start" << std::endl;
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)0));
        appParamters1.AppendTlvtriplet(TlvTriplet(0x05, (uint16_t)1));  // ListStartOffset

        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        EXPECT_THAT(client->Get(*pbReq), 0);
        std::cout << "GetPhoneBookSize request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        auto &lastResp1 = observerForClient->lastRespHeader_;
        EXPECT_THAT(lastResp1 != nullptr, true);
        EXPECT_THAT(lastResp1->GetFieldCode(), 0xA0);
        EXPECT_THAT(lastResp1->GetItemAppParams() != nullptr, true);
        if (lastResp1->GetItemAppParams()) {
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x08) != nullptr, true);  // PhonebookSize
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0A) != nullptr, true);  // PrimaryFolderVersion
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0B) != nullptr, true);  // SecondaryFolderVersion
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0D) != nullptr, true);  // DatabaseIdentifier
        }
    }
    {
        // maxListCount > 0
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::cout << "Get more item start" << std::endl;
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        ObexTlvParamters appParamters2;
        appParamters2.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)3));  // MaxListCount
        uint64_t propertyMask = 1 << 2 | 1 << 5 | 1 << 8;               // N,ADR,EMAIL
        appParamters2.AppendTlvtriplet(TlvTriplet(0x06, propertyMask));

        uint64_t vCardSelector = 1 << 5 | 1 << 8;  // ADR,EMAIL
        appParamters2.AppendTlvtriplet(TlvTriplet(0x0C, vCardSelector));
        appParamters2.AppendTlvtriplet(TlvTriplet(0x0E, (uint8_t)0x01));  // AND
        // Format 3.0
        appParamters2.AppendTlvtriplet(TlvTriplet(0x07, (uint8_t)0x01));

        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters2);
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        std::cout << "Get more item end" << std::endl;

        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        auto &lastResp2 = observerForClient->lastRespHeader_;
        EXPECT_THAT(lastResp2->GetFieldCode(), 0xA0);
        EXPECT_THAT(lastResp2->GetItemAppParams() != nullptr, true);

        auto &extendBody = lastResp2->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_pullphonebook_test, PullvCardListing)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_pullphonebook_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    auto observerForClient = std::make_unique<pbap_pse_pullphonebook_test::TestObexClientObserver>();

    // start pce test obex client
    auto client = pbap_pse_pullphonebook_test::StartTestPceObexMpClient(*observerForClient, *clientDispatcher);
    ObexConnectParams connectParams;
    ObexTlvParamters appParamters;
    uint32_t pceSupportedFeature = 0x03FF;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)pceSupportedFeature));
    connectParams.appParams_ = &appParamters;
    EXPECT_THAT(client->Connect(connectParams), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);

    auto connectIdItem = observerForClient->lastRespHeader_->GetItemConnectionId();
    auto pbReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    pbReq->AppendItemConnectionId(connectIdItem->GetWord());
    {
        // no type item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    pbReq->AppendItemType("x-bt/vcard-listing");
    {
        // setpath
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::vector<std::u16string> paths;
        paths.push_back(u"telecom");
        paths.push_back(u"pb");
        EXPECT_THAT(client->SetPath(paths), 0);
        std::cout << "SetPath request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
    }
    {
        // no name item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // wrong name : has path information
        pbReq->AppendItemName(u"telecom/pb");
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // wrong name : not folder
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"pb.vcf");
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable

        // empty name : get all vcard in /telecom/pb
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"");
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    {
        // wrong app params
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0xFF, (uint16_t)0));
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // maxListCount 0
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::cout << "PullvCardListingSize start" << std::endl;
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)0));  // maxListCount
        std::string searchValue = "34";
        appParamters1.AppendTlvtriplet(
            TlvTriplet(0x02, searchValue.size(), (const uint8_t *)searchValue.c_str()));  // SearchValue
        appParamters1.AppendTlvtriplet(TlvTriplet(0x03, (uint8_t)0x01));                  // SearchProperty 0x01= Number
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        EXPECT_THAT(client->Get(*pbReq), 0);
        std::cout << "PullvCardListingSize request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        auto &lastResp1 = observerForClient->lastRespHeader_;
        EXPECT_THAT(lastResp1 != nullptr, true);
        EXPECT_THAT(lastResp1->GetFieldCode(), 0xA0);
        EXPECT_THAT(lastResp1->GetItemAppParams() != nullptr, true);
        if (lastResp1->GetItemAppParams()) {
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x08) != nullptr, true);  // PullvCardListingSize
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0A) != nullptr, true);  // PrimaryFolderVersion
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0B) != nullptr, true);  // SecondaryFolderVersion
            EXPECT_THAT(lastResp1->GetItemAppParams()->GetTlvtriplet(0x0D) != nullptr, true);  // DatabaseIdentifier
        }
    }
    {
        // setpath
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::vector<std::u16string> paths;
        paths.push_back(u"telecom");
        EXPECT_THAT(client->SetPath(paths), 0);
        std::cout << "SetPath request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
    }
    {
        // not empty name but not exists
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"pbtest");
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC4);
    }
    {
        // not empty name : get all vcard in /telecom/pb
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"pb");
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0x01, (uint8_t)0x00));  // order by indexed
        std::string searchValue = "Wang";
        appParamters1.AppendTlvtriplet(
            TlvTriplet(0x02, searchValue.size(), (const uint8_t *)searchValue.c_str()));  // SearchValue
        appParamters1.AppendTlvtriplet(TlvTriplet(0x03, (uint8_t)0x00));                  // SearchProperty 0x00= Name

        appParamters1.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)20));  // maxListCount

        uint64_t vCardSelector = 1 << 5 | 1 << 8;  // ADR,EMAIL
        appParamters1.AppendTlvtriplet(TlvTriplet(0x0C, vCardSelector));
        appParamters1.AppendTlvtriplet(TlvTriplet(0x0E, (uint8_t)0x01));  // AND

        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    {
        // order by alphanumeric
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0x01, (uint8_t)0x01));  // order by alphanumeric
        std::string searchValue = "34";
        appParamters1.AppendTlvtriplet(
            TlvTriplet(0x02, searchValue.size(), (const uint8_t *)searchValue.c_str()));  // SearchValue
        appParamters1.AppendTlvtriplet(TlvTriplet(0x03, (uint8_t)0x01));                  // SearchProperty 0x01= Number

        appParamters1.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)20));  // maxListCount
        appParamters1.AppendTlvtriplet(TlvTriplet(0x05, (uint16_t)0));   // ListStartOffset
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }

    {
        // order by phonetic
        ObexTlvParamters appParamters1;
        appParamters1.AppendTlvtriplet(TlvTriplet(0x01, (uint8_t)0x02));  // order by phonetic
        std::string searchValue = "Bai";
        appParamters1.AppendTlvtriplet(
            TlvTriplet(0x02, searchValue.size(), (const uint8_t *)searchValue.c_str()));  // SearchValue
        appParamters1.AppendTlvtriplet(TlvTriplet(0x03, (uint8_t)0x02));                  // SearchProperty 0x02= Sound

        appParamters1.AppendTlvtriplet(TlvTriplet(0x04, (uint16_t)20));  // maxListCount
        appParamters1.AppendTlvtriplet(TlvTriplet(0x05, (uint16_t)0));   // ListStartOffset

        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        pbReq->AppendItemAppParams(appParamters1);
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_pullphonebook_test, PullvCardEntry)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_pullphonebook_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    auto observerForClient = std::make_unique<pbap_pse_pullphonebook_test::TestObexClientObserver>();

    // start pce test obex client
    auto client = pbap_pse_pullphonebook_test::StartTestPceObexMpClient(*observerForClient, *clientDispatcher);
    ObexConnectParams connectParams;
    ObexTlvParamters appParamters;
    uint32_t pceSupportedFeature = 0x03FF;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)pceSupportedFeature));
    connectParams.appParams_ = &appParamters;
    EXPECT_THAT(client->Connect(connectParams), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);

    auto connectIdItem = observerForClient->lastRespHeader_->GetItemConnectionId();
    auto pbReq = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    pbReq->AppendItemConnectionId(connectIdItem->GetWord());
    {
        // no type item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // wrong type item
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::TYPE);
        pbReq->AppendItemType("unknow-type");
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        // no name
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::TYPE);
        pbReq->AppendItemType("x-bt/vcard");
        EXPECT_THAT(client->Get(*pbReq), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Implemented
    }
    {
        std::vector<std::u16string> wrongNames = {
            u"", u"1.xxx", u"telecom/pb/1.vcf", u"12345.vcf", u".vcf", u"GHIJ.vcf", u"X-BT-UID:"};
        // wrong name item
        for (auto &tmpName : wrongNames) {
            pbReq->RemoveItem(ObexHeader::NAME);
            pbReq->AppendItemName(tmpName);
            observerForClient->ResetCallCount();
            observerForServer->ResetCallCount();
            EXPECT_THAT(client->Get(*pbReq), 0);
            if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
                pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
                GTEST_FATAL_FAILURE_("time out");
                return;
            };
            EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
            EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
        }
    }

    {
        // setpath
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::vector<std::u16string> paths;
        paths.push_back(u"telecom");
        paths.push_back(u"pb");
        EXPECT_THAT(client->SetPath(paths), 0);
        std::cout << "SetPath request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
    }
    {
        std::cout << "search use no exists vcf" << std::endl;
        // search use no exists vcf
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"FFFF.vcf");
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC4);  // NOT FOUND
    }
    {
        std::cout << "wrong app params" << std::endl;
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"0.vcf");
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        ObexTlvParamters appParamters2;
        appParamters2.AppendTlvtriplet(TlvTriplet(0xFF, (uint8_t)0x01));  // unknow param
        pbReq->AppendItemAppParams(appParamters2);
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC6);  // Not Acceptable
    }
    {
        std::cout << "search use exists vcf" << std::endl;
        // search use vcf
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"0.vcf");
        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        ObexTlvParamters appParamters2;
        // Format 3.0
        appParamters2.AppendTlvtriplet(TlvTriplet(0x07, (uint8_t)0x01));
        pbReq->AppendItemAppParams(appParamters2);
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetItemAppParams() != nullptr, true);
        if (observerForClient->lastRespHeader_->GetItemAppParams()) {
            EXPECT_THAT(observerForClient->lastRespHeader_->GetItemAppParams()->GetTlvtriplet(0x0D) != nullptr, true);
        }

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    {
        std::cout << "search use X-BT-UID" << std::endl;
        // search use X-BT-UID
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        pbReq->RemoveItem(ObexHeader::NAME);
        pbReq->AppendItemName(u"X-BT-UID:A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6");

        pbReq->RemoveItem(ObexHeader::APP_PARAMETERS);
        ObexTlvParamters appParamters2;
        uint64_t propertyMask = 1 << 2 | 1 << 5 | 1 << 8;                // N,ADR,EMAIL
        appParamters2.AppendTlvtriplet(TlvTriplet(0x06, propertyMask));  // PropertySelector
        // Format 2.1
        appParamters2.AppendTlvtriplet(TlvTriplet(0x07, (uint8_t)0x00));
        pbReq->AppendItemAppParams(appParamters2);
        auto receivedObj = std::make_shared<ObexArrayBodyObject>();
        EXPECT_THAT(client->Get(*pbReq, receivedObj), 0);
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        };
        EXPECT_THAT(observerForClient->callCountOnActionCompleted, 1);
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);

        auto &extendBody = observerForClient->lastRespHeader_->GetExtendBodyObject();
        EXPECT_THAT(extendBody != nullptr, true);
        EXPECT_THAT(extendBody.get(), receivedObj.get());
        pbap_pse_pullphonebook_test::PrintObexBodyObject(extendBody);
    }
    pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
}

TEST(pbap_pse_pullphonebook_test, SetPhoneBook)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    // start pse service
    auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

    PbapPseService server;
    auto callBackForServer = std::make_unique<pbap_pse_pullphonebook_test::ContextCallback>();
    server.RegisterCallback(*callBackForServer);
    auto observerForServer = std::make_unique<pbap_pse_pullphonebook_test::TestPbapPseObserverImpl>();
    server.RegisterObserver(*observerForServer);

    server.Enable();
    if (!DataTestUtil::WaitFor([&]() { return callBackForServer->callCountOnEnable_ == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    auto observerForClient = std::make_unique<pbap_pse_pullphonebook_test::TestObexClientObserver>();

    // start pce test obex client
    auto client = pbap_pse_pullphonebook_test::StartTestPceObexMpClient(*observerForClient, *clientDispatcher);
    ObexConnectParams connectParams;
    ObexTlvParamters appParamters;
    uint32_t pceSupportedFeature = 0x03FF;
    appParamters.AppendTlvtriplet(TlvTriplet(0x10, (uint32_t)pceSupportedFeature));
    connectParams.appParams_ = &appParamters;
    EXPECT_THAT(client->Connect(connectParams), 0);
    if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnConnected == 1; })) {
        pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
        GTEST_FATAL_FAILURE_("time out");
        return;
    };
    EXPECT_THAT(observerForServer->callCountOnServiceConnected, 1);
    EXPECT_THAT(observerForClient->callCountOnConnected, 1);
    {
        // setpath create=true
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::u16string tmpPath = u"telecom";
        EXPECT_THAT(client->SetPath(0x01, tmpPath), 0);
        std::cout << "SetPath request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        }
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC3);  // FORBIDDEN
    }
    {
        std::vector<std::vector<std::u16string>> okPaths = {{u"telecom", u"cch"},
            {u"SIM1", u"telecom", u"cch"},
            {u"telecom", u"fav"},
            {u"telecom", u"ich"},
            {u"SIM1", u"telecom", u"ich"},
            {u"telecom", u"mch"},
            {u"SIM1", u"telecom", u"mch"},
            {u"telecom", u"och"},
            {u"SIM1", u"telecom", u"och"},
            {u"telecom", u"pb"},
            {u"SIM1", u"telecom", u"pb"},
            {u"telecom", u"spd"}};

        for (auto &paths : okPaths) {
            // setpath
            observerForClient->ResetCallCount();
            observerForServer->ResetCallCount();
            EXPECT_THAT(client->SetPath(paths), 0);
            std::cout << "SetPath request " << std::endl;
            if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
                pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
                GTEST_FATAL_FAILURE_("time out");
                return;
            }
            EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
        }
    }
    {
        // setpath backup = true
        observerForClient->ResetCallCount();
        observerForServer->ResetCallCount();
        std::u16string tmpPath = u"";
        EXPECT_THAT(client->SetPath(0x03, tmpPath), 0);
        std::cout << "SetPath request " << std::endl;
        if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
            pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
            GTEST_FATAL_FAILURE_("time out");
            return;
        }
        EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xA0);
    }
    {
        std::vector<std::vector<std::u16string>> ngPaths = {
            {u"SIM2"}, {u"telecom1"}, {u"telecom", u"test"}, {u"SIM1", u"telecom1"}, {u"SIM1", u"telecom", u"test"}};
        for (auto &paths : ngPaths) {
            // setpath
            observerForClient->ResetCallCount();
            observerForServer->ResetCallCount();
            EXPECT_THAT(client->SetPath(paths), 0);
            std::cout << "SetPath request " << std::endl;
            if (!DataTestUtil::WaitFor([&]() { return observerForClient->callCountOnActionCompleted == 1; })) {
                pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
                GTEST_FATAL_FAILURE_("time out");
                return;
            }
            EXPECT_THAT(observerForClient->lastRespHeader_->GetFieldCode(), 0xC4);
        }
    }
    pbap_pse_pullphonebook_test::CloseServer(server, *callBackForServer);
}