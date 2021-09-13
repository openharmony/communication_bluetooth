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

#include <gtest/gtest.h>
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"

namespace OHOS {
namespace BluetoothStandard {

using namespace testing::ext;

class BleAdvertiseCallbackTest : public Bluetooth::BleAdvertiseCallback{
public:
    BleAdvertiseCallbackTest(){};
    ~BleAdvertiseCallbackTest(){};

private:
    void OnStartResultEvent(int result);
};

class BleCentralManagerCallbackTest : public Bluetooth::BleCentralManagerCallback {
public:
    BleCentralManagerCallbackTest(){};
    ~BleCentralManagerCallbackTest(){};

private:
    void OnScanCallback(const Bluetooth::BleScanResult &result)
    {}
    void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results)
    {}
    void OnStartScanFailed(int resultCode)
    {}
};

class BleTest : public testing::Test {
public:
    BleTest()
    {}
    ~BleTest()
    {}

    Bluetooth::BluetoothHost *host_ = nullptr;
    static BleTest *bleInstance_;
    Bluetooth::UUID g_uuid = Bluetooth::UUID::FromString("00000000-0000-1000-8000-00805F9B34FB");
    Bluetooth::UUID g_serviceDataUuid = Bluetooth::UUID::FromString("00000000-0000-1000-8000-00805F9B34FA");
    std::string g_serviceData = "123";
    int g_manufacturerId = 24;
    std::string g_manufacturerData = "1567";
    Bluetooth::BleAdvertiserSettings bleAdvertiserSettings_{};
    BleCentralManagerCallbackTest bleCentralManagerCallbackTest_{};
    BleAdvertiseCallbackTest bleAdvertiseCallbackTest_{};
    Bluetooth::BleScanSettings bleScanSettings_{};

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void initAdvertiseSettings();
    void initScanSettings();
    bool haveUuid(Bluetooth::BleAdvertiserData advData)
    {
        for (Bluetooth::UUID uuid : advData.GetServiceUuids()) {
            if (uuid.Equals(g_uuid)) {
                isHaveUuid = true;
            }
        }
        return isHaveUuid;
    }
    bool ManufacturerData(Bluetooth::BleAdvertiserData advData)
    {
        if (advData.GetManufacturerData().empty()) {
            return false;
        } else {
            std::map<uint16_t, std::string> data = advData.GetManufacturerData();
            std::map<uint16_t, std::string>::iterator iter = data.begin();
            for (; iter != data.end(); ++iter) {
                if (g_manufacturerId == iter->first) {
                    EXPECT_STREQ(g_manufacturerData.c_str(), iter->second.c_str());
                    
                }
            }
        }
        return true; 
    }
    bool ServiceData(Bluetooth::BleAdvertiserData advData)
    {
        if (advData.GetServiceData().empty()) {
            return false;
        } else {
            for (auto &iter : advData.GetServiceData()) {
                if (BleTest::g_serviceData == iter.second) {
                    EXPECT_STREQ(BleTest::g_serviceData.c_str(), iter.second.c_str());
                    
                }
            }
        }
        return true;
    }

private:
    bool isHaveUuid = false;
};

}  // namespace BluetoothStandard
}  // namespace OHOS