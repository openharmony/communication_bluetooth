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

class AvrcpCtProfile_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("AvrcpCtProfile_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("AvrcpCtProfile_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

TEST_F(AvrcpCtProfile_test, ConnectBr_001)
{
    RawAddress bda("00:00:00:00:00:01");
    uint16_t features = 0x0000;
    uint16_t controlMut = 0x0010;
    uint16_t browseMtu = 0x0010;
    uint32_t companyId = 0x00001958;
    uint8_t maxFragments = 0x03;

    AvrcCtProfile profile(features, companyId, controlMut, browseMtu, maxFragments, nullptr, nullptr, nullptr);
    EXPECT_EQ(profile.ConnectBr(bda), -2);
}

TEST_F(AvrcpCtProfile_test, DisconnectBr_001)
{
    RawAddress bda("00:00:00:00:00:00");
    uint16_t features = 0x0000;
    uint16_t controlMut = 0x0010;
    uint16_t browseMtu = 0x0010;
    uint32_t companyId = 0x00001958;
    uint8_t maxFragments = 0x03;

    AvrcCtProfile profile(features, companyId, controlMut, browseMtu, maxFragments, nullptr, nullptr, nullptr);
    EXPECT_EQ(profile.DisconnectBr(bda), 0);
}
