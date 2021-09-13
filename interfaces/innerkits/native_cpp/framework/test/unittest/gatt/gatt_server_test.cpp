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

#include <gtest/gtest.h>
#include "bluetooth_gatt_server.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
class BluetoothGattServerCallbackCommon : public GattServerCallback {
public:
    BluetoothGattServerCallbackCommon() = default;
    virtual ~BluetoothGattServerCallbackCommon() = default;
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override{}
    void OnServiceAdded(GattService *Service, int ret) override {}
    void OnCharacteristicReadRequest(
                    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override{}
    void OnCharacteristicWriteRequest(
                    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)override{}
    void OnDescriptorReadRequest(
                    const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override {}
    void OnDescriptorWriteRequest(
                    const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override {}
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override {}
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override {}
    void OnConnectionParameterChanged(
                    const BluetoothRemoteDevice &device, int interval, int latency, int timeout, int status) override{}
private:
};
static BluetoothGattServerCallbackCommon callback_;

class GattServerTest : public testing::Test {
public:
    GattServerTest()
    {}
    ~GattServerTest()
    {}

    int tempData_ = 0;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattServerTest::SetUpTestCase(void)
{}
void GattServerTest::TearDownTestCase(void)
{}
void GattServerTest::SetUp()
{
    tempData_ = 0;
}

void GattServerTest::TearDown()
{}

// URI: scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment

//  * @tc.number: Xxx_Unittest_AttachId_GetId_0100
//  * @tc.name: AttachId/GetId
//  * @tc.desc: Test if attachd and getid return values are correct.
 
HWTEST_F(GattServerTest, GattServer_ModuleTest_AddService, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattServer_ModuleTest_AddService start";
    GattServer server(callback_);
    UUID uuidSerPer;
    uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
    int ret = server.AddService(serviceOne);

    EXPECT_EQ(ret, bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattServer_ModuleTest_AddService end";
}

// HWTEST_F(GattServerTest, GattServer_ModuleTest_CancelConnection, TestSize.Level1)
// {
    
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_CancelConnection start";
//     GattServer server(callback_);

//     BluetoothRemoteDevice deviceBle_;
    
//     server.CancelConnection(deviceBle_);
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_CancelConnection end";
// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_RemoveService, TestSize.Level1)
// {
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_RemoveService start";
//     GattServer server(callback_);
 
//     std::list<GattService> &list = server.GetServices();
//     int ret = server.RemoveGattService(list.back());
//     EXPECT_EQ(ret, bluetooth::GattStatus::GATT_SUCCESS);
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_RemoveService end";
// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_ClearServices, TestSize.Level1)
// {

//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_ClearServices start";
//     GattServer server(callback_);
  
//     std::list<GattService>& list = server.GetServices();
//     GTEST_LOG_(INFO) << (int)list.size();
//     server.ClearServices();
//     list = server.GetServices();
//     EXPECT_EQ((int)list.size(), 0);
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_ClearServices end";
// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_Notify, TestSize.Level1)
// {
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_Notify start";
//     GattServer server(callback_);

//     BluetoothRemoteDevice deviceBle_;
//     UUID uuidSerPer;
//     uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
//     GattCharacteristic* aa = new GattCharacteristic(uuidSerPer,1,1);
//     int res = server.NotifyCharacteristicChanged(deviceBle_, *aa, false);
//     EXPECT_EQ(res, (int)bluetooth::GattStatus::GATT_SUCCESS);

//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_Notify end";
// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_SendResponse, TestSize.Level1)
// {
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_SendResponse start";
//     GattServer server(callback_);

//     BluetoothRemoteDevice deviceBle_;
//     string valueChrTwo = "2";
//     int ret = server.SendResponse(deviceBle_,
//         0,
//         (int)GattStatus::GATT_SUCCESS,
//         1,
//         (uint8_t *)valueChrTwo.c_str(),
//         valueChrTwo.size());
//     EXPECT_EQ(ret, bluetooth::GattStatus::GATT_SUCCESS);
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_SendResponse end";

// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_GetServices, TestSize.Level1)
// {
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_GetServices start";
//     GattServer server(callback_);

//     UUID uuidSerPer;
//     uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");

//     GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
//     int ret = server.AddService(serviceOne);

//     EXPECT_EQ(ret, bluetooth::GattStatus::GATT_SUCCESS);
//     std::list<GattService> list = server.GetServices();
//     EXPECT_EQ((int)list.size(), 1);

//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_GetServices end";
// }

// HWTEST_F(GattServerTest, GattServer_ModuleTest_GetService, TestSize.Level1)
// {
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_GetService start";
//     GattServer server(callback_);

//     UUID uuidSerPer;
//     uuidSerPer = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");

//     GattService serviceOne(uuidSerPer, GattServiceType::PRIMARY);
//     int ret = server.AddService(serviceOne);

//     EXPECT_EQ(ret, bluetooth::GattStatus::GATT_SUCCESS);

//     std::optional<GattService> listSecondary = server.GetService(uuidSerPer, false);
//     EXPECT_FALSE(listSecondary->IsPrimary());
//     GTEST_LOG_(INFO) << "GattServer_ModuleTest_GetService end";
// }
}  // namespace Bluetooth
}  // namespace OHOS
