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
#include "bluetooth_connection_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {

class CjBluetoothConnectionCommonTest : public testing::Test {
public:
    CjBluetoothConnectionCommonTest() = default;
    ~CjBluetoothConnectionCommonTest() = default;
    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}
    void SetUp() {}
    void TearDown() {}
};

static const std::string VALID_MAC_ADDRESSES[] = {
    "00:11:22:33:44:55",
    "aa:bb:cc:dd:ee:ff",
    "AA:BB:CC:DD:EE:FF",
    "01:23:45:67:89:ab",
    "AB:CD:EF:01:23:45",
    "ff:ff:ff:ff:ff:ff",
    "00:00:00:00:00:00",
    "a1:b2:c3:d4:e5:f6",
};

static const std::string INVALID_MAC_ADDRESSES[] = {
    "",
    "00:11:22:33:44",
    "00:11:22:33:44:55:66",
    "001122334455",
    "00-11-22-33-44-55",
    "00:11:22:33:44:5",
    "00:11:22:33:44:555",
    "GG:11:22:33:44:55",
    "00:11:22:33:44:5g",
    "00:11:22:33:44:55 ",
    " 00:11:22:33:44:55",
    "00:11:22:33::44:55",
    "::11:22:33:44:55",
    "00:11:22:33:44:55\n",
    "0g:11:22:33:44:55",
    "00:11:22:33:44:5G",
};

/**
 * @tc.name: IsValidAddress_001
 * @tc.desc: Test valid MAC format addresses
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_001, TestSize.Level1)
{
    HILOGI("IsValidAddress_001 enter");
    for (const auto &addr : VALID_MAC_ADDRESSES) {
        EXPECT_TRUE(IsValidAddress(addr)) << "Expected valid for: " << addr;
    }
    HILOGI("IsValidAddress_001 end");
}

/**
 * @tc.name: IsValidAddress_002
 * @tc.desc: Test invalid MAC format addresses
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_002, TestSize.Level1)
{
    HILOGI("IsValidAddress_002 enter");
    for (const auto &addr : INVALID_MAC_ADDRESSES) {
        EXPECT_FALSE(IsValidAddress(addr)) << "Expected invalid for: " << addr;
    }
    HILOGI("IsValidAddress_002 end");
}

/**
 * @tc.name: IsValidAddress_003
 * @tc.desc: Test empty string address
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_003, TestSize.Level1)
{
    HILOGI("IsValidAddress_003 enter");
    EXPECT_FALSE(IsValidAddress(""));
    HILOGI("IsValidAddress_003 end");
}

/**
 * @tc.name: IsValidAddress_004
 * @tc.desc: Test MAC with separator not colon
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_004, TestSize.Level1)
{
    HILOGI("IsValidAddress_004 enter");
    EXPECT_FALSE(IsValidAddress("00-11-22-33-44-55"));
    HILOGI("IsValidAddress_004 end");
}

/**
 * @tc.name: IsValidAddress_005
 * @tc.desc: Test MAC with invalid hex character
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_005, TestSize.Level1)
{
    HILOGI("IsValidAddress_005 enter");
    EXPECT_FALSE(IsValidAddress("GG:11:22:33:44:55"));
    HILOGI("IsValidAddress_005 end");
}

/**
 * @tc.name: IsValidAddress_006
 * @tc.desc: Test MAC with upper and lower mixed hex characters
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_006, TestSize.Level1)
{
    HILOGI("IsValidAddress_006 enter");
    EXPECT_TRUE(IsValidAddress("Aa:Bb:Cc:Dd:Ee:Ff"));
    HILOGI("IsValidAddress_006 end");
}

/**
 * @tc.name: IsValidAddress_007
 * @tc.desc: Test MAC address too short
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_007, TestSize.Level1)
{
    HILOGI("IsValidAddress_007 enter");
    EXPECT_FALSE(IsValidAddress("00:11:22:33:44"));
    HILOGI("IsValidAddress_007 end");
}

/**
 * @tc.name: IsValidAddress_008
 * @tc.desc: Test MAC address too long
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_008, TestSize.Level1)
{
    HILOGI("IsValidAddress_008 enter");
    EXPECT_FALSE(IsValidAddress("00:11:22:33:44:55:66"));
    HILOGI("IsValidAddress_008 end");
}

/**
 * @tc.name: IsValidAddress_009
 * @tc.desc: Test MAC with no separators
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_009, TestSize.Level1)
{
    HILOGI("IsValidAddress_009 enter");
    EXPECT_FALSE(IsValidAddress("001122334455"));
    HILOGI("IsValidAddress_009 end");
}

/**
 * @tc.name: IsValidAddress_010
 * @tc.desc: Test MAC with consecutive separators
 * @tc.type: FUNC
 */
HWTEST_F(CjBluetoothConnectionCommonTest, IsValidAddress_010, TestSize.Level1)
{
    HILOGI("IsValidAddress_010 enter");
    EXPECT_FALSE(IsValidAddress("00:11:22:33::55"));
    HILOGI("IsValidAddress_010 end");
}

}  // namespace CJBluetoothConnection
}  // namespace CJSystemapi
}  // namespace OHOS