/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "bluetooth_log.h"
#include "uuid.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {

class UuidTest : public testing::Test {
public:
    UuidTest() = default;
    ~UuidTest() = default;
    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}
    void SetUp() {}
    void TearDown() {}
};

static const std::string VALID_UUIDS[] = {
    "12345678-1234-1234-1234-123456789abc",
    "ABCDEF12-3456-7890-ABCD-EF1234567890",
    "00000000-0000-0000-0000-000000000000",
    "ffffffff-ffff-ffff-ffff-ffffffffffff",
    "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
};

static const std::string INVALID_UUIDS[] = {
    "",
    "1234567-1234-1234-1234-123456789abc",
    "123456789-1234-1234-1234-123456789abc",
    "12345678-123-1234-1234-123456789abc",
    "12345678-12345-1234-1234-123456789abc",
    "12345678-1234-123-1234-123456789abc",
    "12345678-1234-12345-1234-123456789abc",
    "12345678-1234-1234-123-123456789abc",
    "12345678-1234-1234-12345-123456789abc",
    "12345678-1234-1234-1234-123456789ab",
    "12345678-1234-1234-1234-123456789abcd",
    "123456781234-1234-1234-123456789abc",
    "g2345678-1234-1234-1234-123456789abc",
    "12345678-1234-1234-1234-123456789abg",
    " 12345678-1234-1234-1234-123456789abc",
    "12345678-1234-1234-1234-123456789abc ",
    "12345678-1234-1234-1234-123456789ABC\n",
};

/**
 * @tc.name: IsValidUuid_001
 * @tc.desc: Test valid UUID format
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_001, TestSize.Level1)
{
    HILOGI("IsValidUuid_001 enter");
    for (const auto &uuid : VALID_UUIDS) {
        EXPECT_TRUE(IsValidUuid(uuid)) << "Expected valid for: " << uuid;
    }
    HILOGI("IsValidUuid_001 end");
}

/**
 * @tc.name: IsValidUuid_002
 * @tc.desc: Test invalid UUID format
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_002, TestSize.Level1)
{
    HILOGI("IsValidUuid_002 enter");
    for (const auto &uuid : INVALID_UUIDS) {
        EXPECT_FALSE(IsValidUuid(uuid)) << "Expected invalid for: " << uuid;
    }
    HILOGI("IsValidUuid_002 end");
}

/**
 * @tc.name: IsValidUuid_003
 * @tc.desc: Test empty string UUID
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_003, TestSize.Level1)
{
    HILOGI("IsValidUuid_003 enter");
    EXPECT_FALSE(IsValidUuid(""));
    HILOGI("IsValidUuid_003 end");
}

/**
 * @tc.name: IsValidUuid_004
 * @tc.desc: Test UUID with missing dash
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_004, TestSize.Level1)
{
    HILOGI("IsValidUuid_004 enter");
    EXPECT_FALSE(IsValidUuid("12345678123412341234123456789abc"));
    HILOGI("IsValidUuid_004 end");
}

/**
 * @tc.name: IsValidUuid_005
 * @tc.desc: Test UUID with extra dash
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_005, TestSize.Level1)
{
    HILOGI("IsValidUuid_005 enter");
    EXPECT_FALSE(IsValidUuid("12345678--1234-1234-1234-123456789abc"));
    HILOGI("IsValidUuid_005 end");
}

/**
 * @tc.name: IsValidUuid_006
 * @tc.desc: Test UUID with invalid hex character
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_006, TestSize.Level1)
{
    HILOGI("IsValidUuid_006 enter");
    EXPECT_FALSE(IsValidUuid("g2345678-1234-1234-1234-123456789abc"));
    HILOGI("IsValidUuid_006 end");
}

/**
 * @tc.name: IsValidUuid_007
 * @tc.desc: Test UUID with upper and lower mixed hex characters
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_007, TestSize.Level1)
{
    HILOGI("IsValidUuid_007 enter");
    EXPECT_TRUE(IsValidUuid("AbCdEf12-3456-7890-aBcD-eF1234567890"));
    HILOGI("IsValidUuid_007 end");
}

/**
 * @tc.name: IsValidUuid_008
 * @tc.desc: Test UUID too short
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_008, TestSize.Level1)
{
    HILOGI("IsValidUuid_008 enter");
    EXPECT_FALSE(IsValidUuid("12345678-1234-1234-1234-123456789ab"));
    HILOGI("IsValidUuid_008 end");
}

/**
 * @tc.name: IsValidUuid_009
 * @tc.desc: Test UUID too long
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_009, TestSize.Level1)
{
    HILOGI("IsValidUuid_009 enter");
    EXPECT_FALSE(IsValidUuid("12345678-1234-1234-1234-123456789abcd"));
    HILOGI("IsValidUuid_009 end");
}

/**
 * @tc.name: IsValidUuid_010
 * @tc.desc: Test UUID with leading whitespace
 * @tc.type: FUNC
 */
HWTEST_F(UuidTest, IsValidUuid_010, TestSize.Level1)
{
    HILOGI("IsValidUuid_010 enter");
    EXPECT_FALSE(IsValidUuid(" 12345678-1234-1234-1234-123456789abc"));
    HILOGI("IsValidUuid_010 end");
}

}  // namespace Bluetooth
}  // namespace OHOS