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
#include "raw_address.h"
#include "securec.h"

#include <memory>
#include <signal.h>
#include <unistd.h>

#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <iostream>

#include "log.h"
#include "obex_client.h"
#include "obex_headers.h"
#include "obex_mp_client.h"
#include "obex_utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#ifdef _WIN32  // Linux platform
#include "conio.h"
#define get_char getch
#else
#include "termios.h"
#define get_char getchar
#endif

using namespace std;
using namespace bluetooth;

class TestObexClientObserver : public ObexClientObserver {
public:
    void OnTransportFailed(ObexClient &client, int errCd) override;
    void OnConnected(ObexClient &client, const ObexHeader &resp) override;
    void OnConnectFailed(ObexClient &client, const ObexHeader &resp) override;
    void OnDisconnected(ObexClient &client) override;
    void OnActionCompleted(ObexClient &client, const ObexHeader &resp) override;
};
void TestObexClientObserver::OnTransportFailed(ObexClient &client, int errCd)
{
    std::cout << client.GetClientId() << ":"
              << "TestObexClientObserver::OnTransportFailed" << std::endl;
}

void TestObexClientObserver::OnConnected(ObexClient &client, const ObexHeader &resp)
{
    std::cout << client.GetClientId() << ":"
              << "TestObexClientObserver::OnConnected" << std::endl;
}

void TestObexClientObserver::OnConnectFailed(ObexClient &client, const ObexHeader &resp)
{
    std::cout << client.GetClientId() << ":"
              << "TestObexClientObserver::OnConnectFailed" << std::endl;
}

void TestObexClientObserver::OnDisconnected(ObexClient &client)
{
    std::cout << client.GetClientId() << ":"
              << "TestObexClientObserver::OnDisconnected" << std::endl;
}

void TestObexClientObserver::OnActionCompleted(ObexClient &client, const ObexHeader &resp)
{
    std::cout << client.GetClientId() << ":"
              << "TestObexClientObserver::OnActionCompleted" << std::endl;
    auto extendBody = resp.GetExtendBodyObject();
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
}

int main(int argc, char *argv[])
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
    SocketGapMock socketGap;
#else
    cout << "MOCK SOCK MODE" << endl;
#endif

    ObexClientConfig configRfcomm;
    ObexClientConfig configL2cap;
    try {
#ifdef IS_SOCK_MOCK
        std::string ipaddr = "127.0.0.1";
        if (argc > 1) {
            ipaddr = argv[1];
        }
        cout << "\nconnect to ipaddr:" << ipaddr << endl;

        uint32_t ip32 = inet_addr(ipaddr.c_str());
        uint8_t ip[4] = {0};  // 127.0.0.1
        memcpy_s(ip, sizeof(ip), &ip32, 4);
#else
        bluetooth::AdapterManager::GetInstance()->Start();
        sleep(1);
        bluetooth::AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
        sleep(3);
        string btAddr = "00:1A:7D:DA:71:B4";
        if (argc > 1) {
            btAddr = argv[1];
        }
        cout << "\nconnect to btAddr:" << btAddr << endl;
        RawAddress rawAddr(btAddr);

        uint8_t ip[6] = {0};

        rawAddr.ConvertToUint8(ip, sizeof(ip));
#endif
        memcpy_s(configRfcomm.addr_.addr, sizeof(configRfcomm.addr_.addr), ip, sizeof(ip));
        configRfcomm.addr_.type = 0;
#ifndef IS_SOCK_MOCK
        configRfcomm.addr_.type = BT_PUBLIC_DEVICE_ADDRESS;
#endif
        configRfcomm.scn_ = 1;
        configRfcomm.mtu_ = 400;
        configRfcomm.isSupportReliableSession_ = false;
        configRfcomm.isSupportSrm_ = false;
        configRfcomm.isGoepL2capPSM_ = false;

        memcpy_s(configL2cap.addr_.addr, sizeof(configL2cap.addr_.addr), ip, sizeof(ip));
        configL2cap.addr_.type = 0;

        configL2cap.scn_ = 1234;
        configL2cap.mtu_ = 400;
        configL2cap.isSupportReliableSession_ = false;
        configL2cap.isSupportSrm_ = true;
        configL2cap.isGoepL2capPSM_ = true;

#ifndef IS_SOCK_MOCK
        socketGap.RegisterServiceSecurity(&configRfcomm.addr_,
            GAP_SecMultiplexingProtocol::SEC_PROTOCOL_RFCOMM,
            configRfcomm.scn_,
            false,
            0,
            GAP_Service::PBAP_CLIENT);
        sleep(1);
#endif
        auto clientDispatcher = std::make_unique<utility::Dispatcher>("obex_client");

        auto observer = std::make_unique<TestObexClientObserver>();
        auto obexClientRfcomm = std::make_unique<ObexMpClient>(configRfcomm, *observer, *clientDispatcher);
        auto obexClientL2cap = std::make_unique<ObexMpClient>(configL2cap, *observer, *clientDispatcher);

        bool isContinue = true;
        std::string typeName;
        while (isContinue) {
            ObexMpClient *client = nullptr;

            std::cout << "---------------------------Transport type-------------------------" << std::endl;
            std::cout << "1: Rfcomm" << std::endl;
            std::cout << "2: L2cap" << std::endl;
            std::cout << "q: Exit" << std::endl;
            std::cout << "------------------------------------------------------------------" << std::endl;
            std::cout << "Please input transport type:";
            char type = get_char();
            if (type != '\n') {
                std::cout << std::endl;
                std::cout << "/*************************** input type[" << type << "]****************************/"
                          << std::endl;
            }
            switch (type) {
                case '1': {
                    client = obexClientRfcomm.get();
                    typeName = "Rfcomm";
                } break;
                case '2': {
                    client = obexClientL2cap.get();
                    typeName = "L2cap";
                } break;
                case 'q':
                    isContinue = false;
                    continue;
                default:
                    continue;
            }
            while (isContinue) {
                std::cout << "-------Operation---------------" << std::endl;
                std::cout << "q: Back to menu" << std::endl;
                std::cout << "1: Connect use " << typeName << std::endl;
                std::cout << "2: Put use " << typeName << std::endl;
                std::cout << "3: Put with final bit use " << typeName << std::endl;
                std::cout << "4: Put with auto continue use " << typeName << std::endl;
                std::cout << "5: Get use " << typeName << std::endl;
                std::cout << "6: Get with final Bit use " << typeName << std::endl;
                std::cout << "7: Get with auto continue use " << typeName << std::endl;
                std::cout << "8: SetPath use " << typeName << std::endl;
                std::cout << "9: SetPath list use " << typeName << std::endl;
                std::cout << "a: Abort " << typeName << std::endl;
                std::cout << "0: Disconnect use " << typeName << std::endl;
                std::cout << "-------------------------------" << std::endl;
                std::cout << "Please input Operation:";
                char ch = get_char();
                if (ch != '\n') {
                    std::cout << std::endl;
                }
                bool skipWhile = false;
                switch (ch) {
                    case 'q': {
                        skipWhile = true;
                    } break;
                    case '0':
                        std::cout << "Disconnect..." << std::endl;
                        client->Disconnect();
                        break;
                    case '1': {
                        std::cout << "Connect..." << std::endl;
                        client->Connect();
                    } break;
                    case '2': {
                        std::cout << "Put..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::PUT);
                        header->AppendItemConnectionId(6144);
                        header->AppendItemType("x-bt/MAP-event-report");

                        ObexTlvParamters appParamters;
                        TlvTriplet masInstanceID(0x0F, (uint8_t)0);
                        appParamters.AppendTlvtriplet(masInstanceID);
                        header->AppendItemAppParams(appParamters);
                        uint8_t body[100];
                        for (int i = 0; i < 100; i++) {
                            body[i] = i % 100;
                        }
                        header->AppendItemBody(body, sizeof(body));
                        client->Put(*header);
                    } break;
                    case '3': {
                        std::cout << "Put Final..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::PUT_FINAL);
                        header->AppendItemConnectionId(6144);

                        const uint8_t body[] = {};
                        header->AppendItemEndBody(body, sizeof(body));
                        client->Put(*header);
                    } break;
                    case '4': {
                        std::cout << "Put with auto continue use..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::PUT);
                        header->AppendItemConnectionId(6144);
                        header->AppendItemType("x-bt/MAP-event-report");

                        ObexTlvParamters appParamters;
                        TlvTriplet masInstanceID(0x0F, (uint8_t)0);
                        appParamters.AppendTlvtriplet(masInstanceID);
                        header->AppendItemAppParams(appParamters);
                        if (typeName == "L2cap") {
                            header->AppendItemSrm(true);
                        }
                        uint8_t body[5000];
                        for (int i = 0; i < 5000; i++) {
                            body[i] = i % 256;
                        }
                        auto bodyObject = std::make_shared<ObexArrayBodyObject>(body, sizeof(body));
                        client->Put(*header, bodyObject);
                    } break;
                    case '5': {
                        std::cout << "Get..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::GET);
                        header->AppendItemConnectionId(1);
                        header->AppendItemSrm(true);
                        header->AppendItemSrmp();
                        client->Get(*header);
                    } break;
                    case '6': {
                        std::cout << "Get with final bit ..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
                        header->AppendItemConnectionId(1);
                        header->AppendItemName(u"telecom/pb.vcf");
                        header->AppendItemType("x-bt/phonebook");
                        ObexTlvParamters appParamters;
                        TlvTriplet maxListCount(0x04, (uint16_t)1);
                        appParamters.AppendTlvtriplet(maxListCount);
                        TlvTriplet listStartOffset(0x05, (uint16_t)0);
                        appParamters.AppendTlvtriplet(listStartOffset);
                        uint8_t propertyMask[] = {0, 0, 0, 0, 0, 0, 0, 134};
                        TlvTriplet propertySelector(0x08, 8, propertyMask);
                        appParamters.AppendTlvtriplet(propertySelector);
                        header->AppendItemAppParams(appParamters);
                        client->Get(*header);
                    } break;
                    case '7': {
                        std::cout << "Get with auto continue use ..." << std::endl;
                        auto header = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
                        header->AppendItemConnectionId(1);
                        header->AppendItemName(u"telecom/pb.vcf");
                        header->AppendItemType("x-bt/phonebook");
                        ObexTlvParamters appParamters;
                        TlvTriplet maxListCount(0x04, (uint16_t)50);
                        appParamters.AppendTlvtriplet(maxListCount);
                        TlvTriplet listStartOffset(0x05, (uint16_t)0);
                        appParamters.AppendTlvtriplet(listStartOffset);
                        uint8_t propertyMask[] = {0, 0, 0, 0, 0, 0, 0, 134};
                        TlvTriplet propertySelector(0x08, 8, propertyMask);
                        appParamters.AppendTlvtriplet(propertySelector);
                        header->AppendItemAppParams(appParamters);
                        if (typeName == "L2cap") {
                            header->AppendItemSrm(true);
                        }
                        auto recvBody = std::make_shared<ObexArrayBodyObject>();
                        client->Get(*header, recvBody);
                    } break;
                    case '8': {
                        std::cout << "SetPath..." << std::endl;
                        client->SetPath(0x02, u"1234567890");
                    } break;
                    case '9': {
                        std::cout << "SetPath List..." << std::endl;
                        vector<u16string> paths = {u"0123456789", u"9876543210"};
                        client->SetPath(paths);
                    } break;
                    case 'a': {
                        std::cout << "Abort ..." << std::endl;
                        client->Abort();
                    } break;
                    default:
                        std::cout << "UNKNOW CMD..." << std::endl;
                        break;
                }
                if (skipWhile) {
                    break;
                }
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
    socketGap.UnregisterSecurity(&configRfcomm.addr_, configRfcomm.scn_, GAP_Service::PBAP_CLIENT, false);
#endif
    std::cout << "Obex Client End." << std::endl;
    return 0;
}