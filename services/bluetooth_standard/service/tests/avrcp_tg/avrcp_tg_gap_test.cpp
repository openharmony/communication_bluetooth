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

#include "avrcp_tg_mock.h"

using namespace testing;
using namespace bluetooth;

class AvrcpTgGap_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("AvrcpGap SetUpTestCase\n");

    }
    static void TearDownTestCase()
    {
        printf("AvrcpGap TearDownTestCase\n");

    }
    virtual void SetUp()
    {
        registerGapMocker(&mockGap_);
    }
    virtual void TearDown()
    {}
    NiceMock<MockGap> mockGap_;
};

TEST_F(AvrcpTgGap_test, RegisterSecurity_001)
{
    AvrcTgGapManager gapManager;

    EXPECT_CALL(GetGapMocker(), GAPIF_RegisterServiceSecurity).Times(4).WillRepeatedly(Return(0));
    int result = gapManager.RegisterSecurity();
    printf("output is %d\n", result);

    EXPECT_EQ(0, result);
}

TEST_F(AvrcpTgGap_test, UnregisterSecurity_001)
{
    AvrcTgGapManager gapManager;

    EXPECT_CALL(GetGapMocker(), GAPIF_DeregisterServiceSecurity).Times(4).WillRepeatedly(Return(0));
    int result = gapManager.UnregisterSecurity();

    EXPECT_EQ(0, result);
}
