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

#include "avrcp_ct_mock.h"

using namespace testing;
using namespace bluetooth;


class AvrcpCtGap_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("AvrcpCtGap_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("AvrcpCtGap_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerGapMocker(&mockGap_);
    }
    virtual void TearDown()
    {}
    StrictMock<MockGap> mockGap_;
};

TEST_F(AvrcpCtGap_test, RegisterSecurity_001)
{
    AvrcCtGapManager gapManager;
    EXPECT_CALL(GetGapMocker(), GAPIF_RegisterServiceSecurity).Times(4).WillRepeatedly(Return(0));
    int result = gapManager.RegisterSecurity();

    EXPECT_EQ(0, result);
}

TEST_F(AvrcpCtGap_test, UnregisterSecurity_001)
{
    AvrcCtGapManager gapManager;
    EXPECT_CALL(GetGapMocker(), GAPIF_DeregisterServiceSecurity).Times(4).WillRepeatedly(Return(0));
    int result = gapManager.UnregisterSecurity();

    EXPECT_EQ(0, result);
}
