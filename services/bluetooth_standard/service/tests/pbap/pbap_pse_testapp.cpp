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

#include "message.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include "pbap_pse_service.h"

using namespace bluetooth;
namespace pbap_pse_testapp {
class TestPbapPseObserverImpl : public IPbapPseObserver {
public:
    TestPbapPseObserverImpl()
    {}

    void OnServiceConnectionStateChanged(const RawAddress &remoteAddr, int state) override
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "pbap_pse_testapp remoteAddr:" << remoteAddr.GetAddress() << " state:" << state << std::endl;
        switch (state) {
            case static_cast<int>(BTConnectState::CONNECTING):
                break;
            case static_cast<int>(BTConnectState::CONNECTED):
                std::cout << "pbap_pse_testapp remoteAddr:" << remoteAddr.GetAddress() << std::endl;
                break;
            case static_cast<int>(BTConnectState::DISCONNECTING):
                break;
            case static_cast<int>(BTConnectState::DISCONNECTED):
                std::cout << "pbap_pse_testapp remoteAddr:" << remoteAddr.GetAddress() << std::endl;
                break;
            default:
                break;
        }
    }

    void OnServicePermission(const RawAddress &remoteAddr) override
    {}

    void OnServicePasswordRequired(const RawAddress &remoteAddr, const std::vector<uint8_t> &description,
        uint8_t charset, bool fullAccess) override
    {}
};
}  // namespace pbap_pse_testapp
int TestAppMainServer(int argc, char *argv[])
{
    std::cout << "TestAppMainServer" << std::endl;
    PbapPseService ser;
    auto observer = std::make_unique<pbap_pse_testapp::TestPbapPseObserverImpl>();
    ser.RegisterObserver(*observer);
    ser.Enable();
    getchar();
    return 0;
}
