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

#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "common_mock_all.h"
#include "obex_socket_transport.h"
#include "mce_mock/mce_mock.h"

using namespace testing;
using namespace bluetooth;

int main(int argc, char **argv)
{
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    registerNewMockAdapterConfig(&acinstanceMock_);
    EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
    EXPECT_CALL(acMock_, GetValue(_, _, An<int &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(MapMceAdapterConfigMock::GetValueInt));
    EXPECT_CALL(acMock_, GetValue(_, _, An<bool &>()))
        .Times(AtLeast(0))
        .WillRepeatedly(Invoke(MapMceAdapterConfigMock::GetValueBool));

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
