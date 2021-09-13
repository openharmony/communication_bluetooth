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

#include "log.h"
#include "message.h"
#include "pbap_pse_vcard_manager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "common_mock_all.h"
#include "pbap_mock.h"

using namespace testing;
using namespace bluetooth;

extern int TestAppMainServer(int argc, char *argv[]);
extern int TestAppMainClient(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    if (argc > 1) {
        std::cout << "argv:" << argv[1] << std::endl;
        std::string argv1(argv[1]);
        if (argv1 == "server") {
            LOG_Initialize(LOG_LEVEL::LOG_LEVEL_DEBUG);
            return TestAppMainServer(argc, argv);
        } else if (argv1 == "client") {
            LOG_Initialize(LOG_LEVEL::LOG_LEVEL_DEBUG);
            return TestAppMainClient(argc, argv);
        }
    }
    PbapPseVcardManager::SetDbFile("pbap_pse_test.s3db");
    
    StrictMock<PowerManagerInstanceMock> pmInstanceMock;
    StrictMock<PowerManagerMock> pmMock;
    registerNewMockPowerManager(&pmInstanceMock);
    EXPECT_CALL(pmInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock));
    EXPECT_CALL(pmMock, StatusUpdate(_, _, _)).Times(AtLeast(0)).WillRepeatedly(Return());

    StrictMock<ProfileConfigInstanceMock> pInstanceMock;
    StrictMock<ProfileConfigMock> pMock;
    registerNewMockProfileConfig(&pInstanceMock);
    EXPECT_CALL(pInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&pMock));
    EXPECT_CALL(pMock, GetValue(_, _, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(PbapProfileConfigMock::GetValue));
    EXPECT_CALL(pMock, SetValue(_, _, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(PbapProfileConfigMock::SetValue));

    StrictMock<AdapterConfigInstanceMock> acInstanceMock;
    StrictMock<AdapterConfigMock> acMock;
    registerNewMockAdapterConfig(&acInstanceMock);
    EXPECT_CALL(acInstanceMock, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock));
    EXPECT_CALL(acMock, GetValue(_, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(PbapAdapterConfigMock::GetValueInt));
    EXPECT_CALL(acMock, GetValue(_, _, An<bool &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(PbapAdapterConfigMock::GetValueBool));

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
