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
#include "mse_mock/mse_mock.h"
#include "log.h"
#include <cstdio>

using namespace testing;

int main(int argc, char **argv)
{
    LOG_Initialize(LOG_LEVEL::LOG_LEVEL_DEBUG);
    printf("Running main() from %s\n", __FILE__);

    StrictMock<PowerManagerInstanceMock> pminstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;
    registerNewMockPowerManager(&pminstanceMock_);
    EXPECT_CALL(pminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
    EXPECT_CALL(pmMock_, StatusUpdate(_, _, _)).Times(AtLeast(0)).WillRepeatedly(Return());
    
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    registerNewMockAdapterConfig(&acinstanceMock_);
    EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
    EXPECT_CALL(acMock_, GetValue(SECTION_MAP_MSE_SERVICE, PROPERTY_MAP_VERSION, An<std::string &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<2>("map14"), Return(true)));
    EXPECT_CALL(acMock_, GetValue(SECTION_MAP_MSE_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));

    StrictMock<ProfileConfigInstanceMock> pinstanceMock_;
    StrictMock<ProfileConfigMock> pMock_;
    registerNewMockProfileConfig(&pinstanceMock_);
    EXPECT_CALL(pinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&pMock_));
    EXPECT_CALL(pMock_, RemoveProperty(_, _, _)).Times(AtLeast(0)).WillRepeatedly(Invoke(MseMock::RemoveProperty));
    EXPECT_CALL(pMock_, GetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Invoke(MseMock::GetValue));
    EXPECT_CALL(pMock_, SetValue(_, _, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Invoke(MseMock::SetValue));
    EXPECT_CALL(acMock_, GetValue(_, _, An<int &>())).Times(AtLeast(0)).WillRepeatedly(Invoke(MseMock::GetValueInt));
    EXPECT_CALL(acMock_, GetValue(_, _, An<bool &>())).Times(AtLeast(0)).WillRepeatedly(Invoke(MseMock::GetValueBool));
    testing::InitGoogleTest(&argc, argv);
    testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
    // }
}