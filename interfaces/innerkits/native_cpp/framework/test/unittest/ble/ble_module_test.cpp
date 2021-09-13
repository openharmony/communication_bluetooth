/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ble_module_test.h"
#include <thread>

namespace OHOS {
namespace BluetoothStandard {

BleTest *BleTest::bleInstance_ = nullptr;

const static int defaultInt = 150;

// BLE advertiser flag
typedef enum {
    BLE_ADV_FLAG_LIMIT_DISC = (0x01 << 0),
    BLE_ADV_FLAG_GEN_DISC = (0x01 << 1),
    BLE_ADV_FLAG_BREDR_NOT_SPT = (0x01 << 2),
    BLE_ADV_FLAG_DMT_CONTROLLER_SPT = (0x01 << 3),
    BLE_ADV_FLAG_DMT_HOST_SPT = (0x01 << 4),
    BLE_ADV_FLAG_NON_LIMIT_DISC = (0x00)
} BLE_ADV_FLAG;

void BleTest::initAdvertiseSettings()
{
    BleTest::bleInstance_->bleAdvertiserSettings_.SetConnectable(true);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetLegacyMode(true);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetInterval(350);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetTxPower(
        bluetooth::BLE_ADV_TX_POWER_LEVEL::BLE_ADV_TX_POWER_MEDIUM);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetPrimaryPhy(bluetooth::PHY_TYPE::PHY_LE_ALL_SUPPORTED);
    BleTest::bleInstance_->bleAdvertiserSettings_.SetSecondaryPhy(bluetooth::PHY_TYPE::PHY_LE_2M);
}
void BleTest::initScanSettings()
{
    BleTest::bleInstance_->bleScanSettings_.SetReportDelay(defaultInt);
    BleTest::bleInstance_->bleScanSettings_.SetScanMode(bluetooth::SCAN_MODE::SCAN_MODE_LOW_POWER);
    BleTest::bleInstance_->bleScanSettings_.SetLegacy(true);
    BleTest::bleInstance_->bleScanSettings_.SetPhy(bluetooth::PHY_TYPE::PHY_LE_ALL_SUPPORTED);
}

void BleTest::SetUpTestCase(void)
{}
void BleTest::TearDownTestCase(void)
{}
void BleTest::SetUp()
{
    bleInstance_ = this;
}

void BleTest::TearDown()
{}

void BleAdvertiseCallbackTest::OnStartResultEvent(int result)
{
    if (0 == result) {
        // test::Completed(START_ADVERTISING_EVENT);
    } else {
        // test::Completed(START_ADVERTISING_FAILED);
    }
}

/**
 * @tc.number: BLE_ModuleTest_StartCentralManager_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartCentralManager_00100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BLE_ModuleTest_StartCentralManager_00100 start";

    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();
    GTEST_LOG_(INFO) << "BleCentralManager::GetDefaultHost ends";
    EXPECT_TRUE(host_->EnableBle());
    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_TRUE(host_->IsBleEnabled());
    EXPECT_EQ(1650, host_->GetBleMaxAdvertisingDataLength());

    BleTest::bleInstance_->initScanSettings();
    GTEST_LOG_(INFO) << "initScanSettings ends";
    Bluetooth::BleCentralManager bleCentralManager(bleCentralManagerCallbackTest_);
    bleCentralManager.StartScan(BleTest::bleInstance_->bleScanSettings_);
    EXPECT_TRUE(host_->IsBtDiscovering(1));
    EXPECT_EQ(defaultInt, BleTest::bleInstance_->bleScanSettings_.GetReportDelayMillisValue());
    EXPECT_EQ(bluetooth::SCAN_MODE::SCAN_MODE_LOW_POWER, BleTest::bleInstance_->bleScanSettings_.GetScanMode());
    EXPECT_TRUE(BleTest::bleInstance_->bleScanSettings_.GetLegacy());
    EXPECT_EQ(bluetooth::PHY_TYPE::PHY_LE_ALL_SUPPORTED, BleTest::bleInstance_->bleScanSettings_.GetPhy());

    bleCentralManager.StopScan();
    EXPECT_TRUE(host_->DisableBle());
    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_FALSE(host_->IsBleEnabled());
    EXPECT_EQ(0, host_->GetBleMaxAdvertisingDataLength());
    EXPECT_FALSE(host_->IsBtDiscovering(1));
    GTEST_LOG_(INFO) << "BLE_ModuleTest_StartCentralManager_00100 start";
}

/**
 * @tc.number: BLE_ModuleTest_StartAdvertising_00100
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ble_ModuleTest_StartAdvertising_00100 start";
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

    EXPECT_TRUE(host_->EnableBle());
    GTEST_LOG_(INFO) << "BluetoothHost::GetDefaultHost ends";
    BleTest::bleInstance_->initAdvertiseSettings();
    Bluetooth::BleAdvertiser bleAdvertise;
    Bluetooth::BleAdvertiserData advData;
    Bluetooth::BleAdvertiserData scanData;
    GTEST_LOG_(INFO) << "advData function";
    advData.AddServiceUuid(BleTest::g_uuid);
    advData.AddManufacturerData(BleTest::g_manufacturerId, g_manufacturerData);
    advData.AddServiceData(BleTest::BleTest::g_serviceDataUuid, BleTest::g_serviceData);
    advData.SetAdvFlag(BLE_ADV_FLAG::BLE_ADV_FLAG_GEN_DISC);
    bleAdvertise.StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, bleAdvertiseCallbackTest_);

    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsConnectable());
    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsLegacyMode());
    EXPECT_EQ((uint16_t)350, BleTest::bleInstance_->bleAdvertiserSettings_.GetInterval());
    EXPECT_EQ(bluetooth::BLE_ADV_TX_POWER_LEVEL::BLE_ADV_TX_POWER_MEDIUM,
        BleTest::bleInstance_->bleAdvertiserSettings_.GetTxPower());
    EXPECT_EQ(bluetooth::PHY_TYPE::PHY_LE_ALL_SUPPORTED, BleTest::bleInstance_->bleAdvertiserSettings_.GetPrimaryPhy());
    EXPECT_EQ(bluetooth::PHY_TYPE::PHY_LE_2M, BleTest::bleInstance_->bleAdvertiserSettings_.GetSecondaryPhy());
    GTEST_LOG_(INFO) << "advData function test";
    EXPECT_TRUE((bool)BleTest::bleInstance_->haveUuid(advData));
    EXPECT_TRUE((bool)BleTest::bleInstance_->ManufacturerData(advData));
    EXPECT_TRUE((bool)BleTest::bleInstance_->ServiceData(advData));
    EXPECT_EQ(BLE_ADV_FLAG::BLE_ADV_FLAG_GEN_DISC, advData.GetAdvFlag());

    bleAdvertise.StopAdvertising(bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    bleAdvertise.Close(bleAdvertiseCallbackTest_);
    GTEST_LOG_(INFO) << "Ble_ModuleTest_StartAdvertising_00100 end";
}

HWTEST_F(BleTest, BLE_ModuleTest_StartAdvertising_00101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ble_ModuleTest_StartAdvertising_00100 start";
    host_ = &Bluetooth::BluetoothHost::GetDefaultHost();

    EXPECT_TRUE(host_->EnableBle());
    GTEST_LOG_(INFO) << "BluetoothHost::GetDefaultHost ends";
    BleTest::bleInstance_->initAdvertiseSettings();
    Bluetooth::BleAdvertiser bleAdvertise;
    vector<uint_8> advData = {1,2,3};
    vector<uint_8> scanData = {3,2,1};
    GTEST_LOG_(INFO) << "advData function";
    bleAdvertise.StartAdvertising(
        BleTest::bleInstance_->bleAdvertiserSettings_, advData, scanData, bleAdvertiseCallbackTest_);

    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsConnectable());
    EXPECT_TRUE(BleTest::bleInstance_->bleAdvertiserSettings_.IsLegacyMode());
    EXPECT_EQ((uint16_t)350, BleTest::bleInstance_->bleAdvertiserSettings_.GetInterval());
    EXPECT_EQ(bluetooth::BLE_ADV_TX_POWER_LEVEL::BLE_ADV_TX_POWER_MEDIUM,
        BleTest::bleInstance_->bleAdvertiserSettings_.GetTxPower());
    EXPECT_EQ(bluetooth::PHY_TYPE::PHY_LE_ALL_SUPPORTED, BleTest::bleInstance_->bleAdvertiserSettings_.GetPrimaryPhy());
    EXPECT_EQ(bluetooth::PHY_TYPE::PHY_LE_2M, BleTest::bleInstance_->bleAdvertiserSettings_.GetSecondaryPhy());
    GTEST_LOG_(INFO) << "advData function test";
    EXPECT_TRUE((bool)BleTest::bleInstance_->haveUuid(advData));
    EXPECT_TRUE((bool)BleTest::bleInstance_->ManufacturerData(advData));
    EXPECT_TRUE((bool)BleTest::bleInstance_->ServiceData(advData));
    EXPECT_EQ(BLE_ADV_FLAG::BLE_ADV_FLAG_GEN_DISC, advData.GetAdvFlag());

    bleAdvertise.StopAdvertising(bleAdvertiseCallbackTest_);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    bleAdvertise.Close(bleAdvertiseCallbackTest_);
    GTEST_LOG_(INFO) << "Ble_ModuleTest_StartAdvertising_00100 end";
}
}  // namespace BluetoothStandard
}  // namespace OHOS
