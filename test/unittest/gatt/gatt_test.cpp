/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <securec.h>

#include "bluetooth_def.h"
#include "ohos_bt_def.h"
#include "ohos_bt_gatt.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
class GattTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattTest::SetUpTestCase(void) {}

void GattTest::TearDownTestCase(void) {}

void GattTest::SetUp() {}

void GattTest::TearDown() {}

/*
 * @tc.name: GattTest_001
 * @tc.desc: Test BleStartFilterScan when g_BleCentralManager is null
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattTest, GattTest_001, TestSize.Level1)
{
    BleScanParams params;
    BleScanNativeFilter filter[1];
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartFilterScan(&params, filter, filterSize);
    EXPECT_EQ(res, 1);
}

/*
 * @tc.name: GattTest_002
 * @tc.desc: Test BleStartFilterScan success status
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattTest, GattTest_002, TestSize.Level1)
{
    BtGattCallbacks btGattCallbacks;
    BleGattRegisterCallbacks(&btGattCallbacks);
    BleScanParams params;
    params.scanInterval = 3000;
    params.scanWindow = 60;
    params.scanPhy = PHY_LE_1M;

    BleScanNativeFilter filter[1];
    char testDeviceName[5] = "test";
    filter[0].deviceName = testDeviceName;
    memcpy_s(filter[0].serviceUuid, sizeof(filter[0].serviceUuid), "AABBCC", sizeof("AABBCC"));
    memcpy_s(filter[0].serviceUuidMask, sizeof(filter[0].serviceUuidMask), "AABBCC", sizeof("AABBCC"));
    unsigned char test1[1];
    unsigned char test2[1];
    unsigned char test3[1];
    unsigned char test4[1];
    filter[0].serviceData = test1;
    filter[0].serviceDataMask = test2;
    filter[0].manufactureData = test3;
    filter[0].manufactureDataMask = test4;
    filter[0].serviceData[0] = 1;
    filter[0].serviceDataMask[0] = 1;
    filter[0].manufactureData[0] = 1;
    filter[0].manufactureDataMask[0] = 1;
    filter[0].serviceDataLength = 1;
    filter[0].manufactureDataLength = 1;
    filter[0].manufactureId = 0;
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartFilterScan(&params, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    params.scanPhy = PHY_LE_ALL_SUPPORTED;
    res = BleStartFilterScan(&params, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);

    res = BleStartFilterScan(NULL, filter, filterSize);
    EXPECT_EQ(res, 1);
    
    res = BleStartFilterScan(&params, NULL, 0);
    EXPECT_EQ(res, 1);
}

/*
 * @tc.name: GattTest_003
 * @tc.desc: Test BleStartFilterScan when serviceData or manufactureData is null
 * @tc.type: FUNC
 * @tc.require: issueI5OH5C
*/
HWTEST_F(GattTest, GattTest_003, TestSize.Level1)
{
    BleScanParams params;
    params.scanInterval = 3000;
    params.scanWindow = 60;

    BleScanNativeFilter filter[1];
    char testDeviceName[5] = "test";
    filter[0].deviceName = testDeviceName;
    memcpy_s(filter[0].serviceUuid, sizeof(filter[0].serviceUuid), "AABBCC", sizeof("AABBCC"));
    memcpy_s(filter[0].serviceUuidMask, sizeof(filter[0].serviceUuidMask), "AABBCC", sizeof("AABBCC"));
    unsigned char test1[1];
    unsigned char test2[1];
    filter[0].serviceData = NULL;
    filter[0].serviceDataMask = test1;
    filter[0].manufactureData = NULL;
    filter[0].manufactureDataMask = test2;
    filter[0].serviceDataLength = 0;
    filter[0].manufactureDataLength = 0;
    filter[0].manufactureId = 0;
    int filterSize = sizeof(filter) / sizeof(BleScanNativeFilter);
    int res = BleStartFilterScan(&params, filter, filterSize);
    EXPECT_EQ(res, OHOS_BT_STATUS_SUCCESS);
}
}  // namespace Bluetooth
}  // namespace OHOS
