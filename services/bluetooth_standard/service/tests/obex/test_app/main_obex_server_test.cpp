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
#ifndef IS_SOCK_MOCK
#include "../obex_gap/socket_gap_mock.h"
#include "adapter_manager.h"
#include <signal.h>
#include <unistd.h>
#endif
#include <iostream>
#include "log.h"
#include "obex_headers.h"
#include "obex_mp_client.h"
#include "obex_mp_server.h"
#include "obex_utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "securec.h"
#include <functional>
#include <thread>
#ifdef _WIN32  // Linux platform
#include "conio.h"
#define get_char getch
#else
#include "termios.h"
#define get_char getchar
#endif

using namespace std;
using namespace bluetooth;

class TestObexServerObserver : public ObexServerObserver {
public:
    virtual ~TestObexServerObserver();
    void OnTransportConnect(ObexIncomingConnect &incomingConnect) override;

    void OnConnect(ObexServerSession &session, const ObexHeader &req) override;
    void OnDisconnect(ObexServerSession &session, const ObexHeader &req) override;
    void OnPut(ObexServerSession &session, const ObexHeader &req) override;
    void OnGet(ObexServerSession &session, const ObexHeader &req) override;
    void OnSetPath(ObexServerSession &session, const ObexHeader &req) override;

    ObexIncomingConnect *GetLastIncomingConnect()
    {
        return lastIncomingConnect_;
    }
    ObexServerSession *GetLastServerSession()
    {
        return lastServerSession_;
    }

private:
    ObexIncomingConnect *lastIncomingConnect_;
    ObexServerSession *lastServerSession_;
};
TestObexServerObserver::~TestObexServerObserver()
{
    cout << "TestObexServerObserver::~TestObexServerObserver" << endl;
}

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
    auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
    int ret = session.SendResponse(*header);
    cout << "TestObexServerObserver:SendResponse:" << ret << endl;
}

void TestObexServerObserver::OnPut(ObexServerSession &session, const ObexHeader &req)
{
    cout << "TestObexServerObserver::OnPut" << endl;
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
            cout << "Receive Put end body from client:\n"
                 << ObexUtils::ToDebugString(bodyBytes.get(), headerBody->GetHeaderDataSize(), false) << endl;
        }
        auto extendBody = req.GetExtendBodyObject();
        if (extendBody) {
            uint8_t buf[100];
            memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
            vector<uint8_t> results;
            size_t cnt = 0;
            while ((cnt = extendBody->Read(buf, 100)) > 0) {
                results.insert(results.end(), buf, buf + cnt);
            }
            cout << "EXTEND_BODY:\n" << ObexUtils::ToDebugString(results.data(), results.size(), true) << endl;
            cout << "EXTEND_BODY size: " << results.size() << endl;
        }
        auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
        session.SendResponse(*resp);
    } else {
        session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
    }
}

void TestObexServerObserver::OnGet(ObexServerSession &session, const ObexHeader &req)
{
    cout << "TestObexServerObserver::OnGet" << endl;

    if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::GET)) {
        auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
        resp->AppendItemSrm(false);
        session.SendResponse(*resp);
    } else if (req.GetFieldCode() == static_cast<uint8_t>(ObexOpeId::GET_FINAL)) {
        try {
            auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
            auto appParams = req.GetItemAppParams();
            uint16_t maxListCount = 1;
            if (appParams) {
                const TlvTriplet *tlv = appParams->GetTlvtriplet(0x04);
                if (tlv) {
                    maxListCount = ObexUtils::GetBufData16(tlv->GetVal(), 0);
                    cout << "maxListCount:" << maxListCount << endl;
                }
            }
            uint8_t body[200];
            for (int i = 0; i < 200; i++) {
                body[i] = i % 100;
            }
            if (maxListCount < 2) {
                resp->AppendItemBody(body, sizeof(body));
                int ret = session.SendResponse(*resp);
                if (ret != 0) {
                    session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
                }
            } else {
                auto sendBodyObject = std::make_shared<ObexArrayBodyObject>();
                for (int i = 0; i < maxListCount; i++) {
                    sendBodyObject->Write(body, sizeof(body));
                }
                auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
                ObexTlvParamters appParamters;
                TlvTriplet phoneBookSize(0x08, maxListCount);
                appParamters.AppendTlvtriplet(phoneBookSize);
                uint8_t databaseId[16];
                memset_s(databaseId, sizeof(databaseId), 0x00, sizeof(databaseId));
                databaseId[15] = 0x01;
                TlvTriplet databaseIdTlv(0x0D, sizeof(databaseId), databaseId);
                appParamters.AppendTlvtriplet(databaseIdTlv);
                resp->AppendItemAppParams(appParamters);
                int ret = session.SendGetResponse(req, *resp, sendBodyObject);
                if (ret != 0) {
                    session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
                }
            }

        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
        }
    } else {
        session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::INTERNAL_SERVER_ERROR));
    }
}

void TestObexServerObserver::OnSetPath(ObexServerSession &session, const ObexHeader &req)
{
    auto resp = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
    session.SendResponse(*resp);
}

int main()
{
    LOG_Initialize(LOG_LEVEL::LOG_LEVEL_DEBUG);

#ifdef _WIN32
// Do nothing
#else
    struct termios stored_settings;
    struct termios new_settings;
    tcgetattr(0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
#endif
#ifndef IS_SOCK_MOCK
    cout << "SOCK MODE" << endl;
    SocketGapMock SocketGapMock;
#else
    cout << "MOCK SOCK MODE" << endl;
#endif
    ObexServerConfig config;
    try {
        config.useRfcomm_ = true;
        config.rfcommScn_ = 1;
        config.rfcommMtu_ = 300;
#ifdef IS_SOCK_MOCK
        config.useL2cap_ = true;
#else
        config.useL2cap_ = false;
#endif
        config.l2capPsm_ = 1234;
        config.l2capMtu_ = 300;
        config.isSupportReliableSession_ = false;
        config.isSupportSrm_ = true;
        auto observer = std::make_unique<TestObexServerObserver>();
        auto serverDispatcher = std::make_unique<utility::Dispatcher>("obex_server");

        auto obexServer = std::make_unique<ObexMpServer>("test server", config, *observer, *serverDispatcher);

#ifndef IS_SOCK_MOCK
        bluetooth::AdapterManager::GetInstance()->Start();
        sleep(1);
        bluetooth::AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
        sleep(2);
        if (config.useRfcomm_) {
            SocketGapMock.RegisterServiceSecurity(nullptr,
                GAP_SecMultiplexingProtocol::SEC_PROTOCOL_RFCOMM,
                config.rfcommScn_,
                true,
                0,
                GAP_Service::PBAP_SERVER);
        }
        if (config.useL2cap_) {
            SocketGapMock.RegisterServiceSecurity(nullptr,
                GAP_SecMultiplexingProtocol::SEC_PROTOCOL_L2CAP,
                config.l2capPsm_,
                true,
                0,
                GAP_Service::PBAP_SERVER);
        }
        sleep(1);
#endif
        int ret = obexServer->Startup();
        cout << "obexServer->Startup():" << ret << endl;

        bool isContinue = true;
        while (isContinue) {
            cout << "---------------------------------" << endl;
            cout << "0: Accept last connection" << endl;
            cout << "1: Reject last connection" << endl;
            cout << "2: Disconnect last server session" << endl;
            cout << "q: Shutdown" << endl;
            int ch = getchar();
            switch (ch) {
                case '0':
                    std::cout << "Accept last connection..." << std::endl;
                    if (observer->GetLastIncomingConnect() != nullptr) {
                        observer->GetLastIncomingConnect()->AcceptConnection();
                    }
                    break;
                case '1':
                    std::cout << "Reject last connection..." << std::endl;
                    if (observer->GetLastIncomingConnect() != nullptr) {
                        observer->GetLastIncomingConnect()->RejectConnection();
                    }
                    break;
                case '2':
                    std::cout << "Disconnect last server session..." << std::endl;
                    if (observer->GetLastServerSession() != nullptr) {
                        observer->GetLastServerSession()->Disconnect();
                    }
                    break;
                case 'q':
                    std::cout << "Shutdown..." << std::endl;
                    // shutdown
                    obexServer->Shutdown();
                    isContinue = false;
                    break;
                default:
                    break;
            }
        }

    } catch (const std::logic_error &e) {
        std::cerr << "logic_error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
#ifdef _WIN32
// Do nothing
#else
    tcsetattr(0, TCSANOW, &stored_settings);
#endif
#ifndef IS_SOCK_MOCK
    if (config.useRfcomm_) {
        SocketGapMock.UnregisterSecurity(nullptr, config.rfcommScn_, GAP_Service::PBAP_SERVER, true);
    }
    if (config.useL2cap_) {
        SocketGapMock.UnregisterSecurity(nullptr, config.l2capPsm_, GAP_Service::PBAP_SERVER, true);
    }
#endif
    std::cout << "Obex Server End." << std::endl;
    return 0;
}