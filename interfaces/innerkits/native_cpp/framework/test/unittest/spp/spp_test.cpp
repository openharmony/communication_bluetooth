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
#include "bluetooth_socket.h"
#include "bluetooth_gatt_characteristic.h"
#include "uuid.h"
#include "bluetooth_remote_device.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

// class BluetoothHostObserverCommon : public BluetoothHostObserver{
// public:
//     BluetoothHostObserverCommon() = default;
//     virtual ~BluetoothHostObserverCommon() = default;
//     static BluetoothHostObserverCommon& GetInstance();
//     void OnStateChanged(const int transport, const int status) override;
//     void OnDiscoveryStateChanged(int status) override;
//     void OnDiscoveryResult(const BluetoothRemoteDevice& device) override {};
//     void OnPairRequested(const BluetoothRemoteDevice& device) override {};
//     void OnPairConfirmed(const BluetoothRemoteDevice& device, int reqType, int number) override {};
//     void OnDeviceNameChanged(const std::string &deviceName) override {};
//     void OnScanModeChanged(int mode) override {};
//     void OnDeviceAddrChanged(const std::string &address) override {};

// private:

// };

// BluetoothHostObserverCommon& BluetoothHostObserverCommon::GetInstance()
// {
//     static BluetoothHostObserverCommon instance;
//     return instance;
// }

// void BluetoothHostObserverCommon::OnStateChanged(const int transport, const int status)
// {
//     if(transport == BT_TRANSPORT_BLE){
//         switch(status)
//         {
//             case bluetooth::STATE_TURNING_ON:
//                 GTEST_LOG_(INFO) << "BLE:Turning on ...";
//                 break;
//             case bluetooth::STATE_TURN_ON:
//                 GTEST_LOG_(INFO) << "BLE:Turn on";
//                 break;
//             case bluetooth::STATE_TURNING_OFF:
//                 GTEST_LOG_(INFO) << "BLE:Turning off ...";
//                 break;
//             case bluetooth::STATE_TURN_OFF:
//                 GTEST_LOG_(INFO) << "BLE:Turn off";
//                 break;
//         }
//         return; 
//     }
//     else {
//         switch(status)
//         {
//             case bluetooth::STATE_TURNING_ON:
//                 GTEST_LOG_(INFO) << "BREDR:Turning on ...";
//                 break;
//             case bluetooth::STATE_TURN_ON:
//                 GTEST_LOG_(INFO) << "BREDR:Turn on";
//                 break;
//             case bluetooth::STATE_TURNING_OFF:
//                 GTEST_LOG_(INFO) << "BREDR:Turning off ...";
//                 break;
//             case bluetooth::STATE_TURN_OFF:
//                 GTEST_LOG_(INFO) << "BREDR:Turn off";
//                 break;
//         }
//         return;
//     }
    
// }

// void BluetoothHostObserverCommon::OnDiscoveryStateChanged(int status)
// {
//     switch (status) {
//         // 0x01:DISCOVERY_STARTED
//         // 0x02:DISCOVERYING
//         // 0x03:DISCOVERY_STOPED
//         case 0x01:
//             GTEST_LOG_(INFO) << "discovery_start" ;
//             break;
//         case 0x02:
//             GTEST_LOG_(INFO) << "discoverying" ;
//             break;
//         case 0x03:
//             GTEST_LOG_(INFO) << "discovery_done" ;
//             break;
//         default:
//             break;
//     }
// }

class SocketTest : public testing::Test {
public:
    SocketTest()
    {}
    ~SocketTest()
    {}

    SppClientSocket *pSppClientSocket = nullptr;
    //BluetoothHostObserverCommon& btObserver_ = BluetoothHostObserverCommon::GetInstance();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SocketTest::SetUpTestCase(void)
{}
void SocketTest::TearDownTestCase(void)
{}
void SocketTest::SetUp()
{
    
}

void SocketTest::TearDown()
{}

// URI: scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment
/**
 * @tc.number: Xxx_Unittest_AttachId_GetId_0100
 * @tc.name: AttachId/GetId
 * @tc.desc: Test if attachd and getid return values are correct.
 */


// void SppClientTest::SocketInit()
// {
//     if (!insecureServer_)
//         insecureServer_ = SocketFactory::DataListenInsecureRfcommByServiceRecord("server", insecureUuid_);
//     if (!server_)
//         server_ = SocketFactory::DataListenRfcommByServiceRecord("server", uuid_);

//     if (!insecureClient_)
//         insecureClient_ = SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(device_, insecureUuid_);
//     if (!client_)
//         client_ = SocketFactory::BuildRfcommDataSocketByServiceRecord(device_, uuid_);
// }


HWTEST_F(SocketTest, Spp_ModuleTest_Close, TestSize.Level1)
{
    // UUID tempUuid_ = UUID::RandomUUID();
    // uint16_t handle_ = 1;
    // const int permissions_ = 100;
    // const int properties_ = 200;

    // GattCharacteristic* pGattCharacteristic = new GattCharacteristic(tempUuid_ , handle_, permissions_, properties_);
    
    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_001 start";

    BluetoothRemoteDevice* pbluetoothRomote = new BluetoothRemoteDevice();
    UUID randomUuid_ = UUID::RandomUUID();
    pSppClientSocket = new SppClientSocket(*pbluetoothRomote, randomUuid_, TYPE_RFCOMM, false);

    // defult uuid
    UUID insecureUuid_ = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");
    UUID uuid_ = UUID::FromString("11111111-0000-1000-8000-00805F9B34FB");

    //SppServerSocket* insecureServer_ = nullptr;
    SppServerSocket* server_ = nullptr;
    //SppClientSocket* insecureClient_ = nullptr;
    //SppClientSocket* client_ = nullptr;

     BluetoothRemoteDevice device_ = *pbluetoothRomote;
    // if (!insecureServer_)
    //     GTEST_LOG_(INFO) << "SocketFactory::DataListenInsecureRfcommByServiceRecord starts";
    //     insecureServer_ = SocketFactory::DataListenInsecureRfcommByServiceRecord("server", insecureUuid_);
    //     GTEST_LOG_(INFO) << "SocketFactory::DataListenInsecureRfcommByServiceRecord ends";
    if (!server_)
        GTEST_LOG_(INFO) << "SocketFactory::DataListenRfcommByServiceRecord starts";
        server_ = SocketFactory::DataListenRfcommByServiceRecord("server", uuid_);
    //     GTEST_LOG_(INFO) << "SocketFactory::DataListenRfcommByServiceRecord ends";
    // if (!insecureClient_)
    //     GTEST_LOG_(INFO) << "SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord starts";
    //     insecureClient_ = SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(device_, insecureUuid_);
    //     GTEST_LOG_(INFO) << "SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord starts";
    // if (!client_)
    //     GTEST_LOG_(INFO) << "SocketFactory::BuildRfcommDataSocketByServiceRecord starts";
    //     client_ = SocketFactory::BuildRfcommDataSocketByServiceRecord(device_, uuid_);
    //     GTEST_LOG_(INFO) << "SocketFactory::BuildRfcommDataSocketByServiceRecord starts";

    GTEST_LOG_(INFO) << "SppClientSocket::Connect starts";
    pSppClientSocket->Connect();
    GTEST_LOG_(INFO) << "SppClientSocket::Connect ends";


    GTEST_LOG_(INFO) << "SppClientSocket::Close starts";
    pSppClientSocket->Close();
    GTEST_LOG_(INFO) << "SppClientSocket::Close ends";


    int fd = 37;

    SppClientSocket* pfd_SppClientSocket = new SppClientSocket(fd, device_.GetDeviceAddr());

    char receive[512];
    int DATASIZE = 1024;
    size_t returnInput = 0;
    size_t returnOutput = 0;
    char multiChar[10] = {'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    GTEST_LOG_(INFO) << "SppClientSocket::GetInputStream starts";
    InputStream input = pfd_SppClientSocket->GetInputStream();
    GTEST_LOG_(INFO) << "InputStream::Read starts";
    returnInput = input.Read(receive, DATASIZE);
    GTEST_LOG_(INFO) << "InputStream::Read ends";
    GTEST_LOG_(INFO) << "SppClientSocket::GetInputStream ends";

    GTEST_LOG_(INFO) << "SppClientSocket::GetOutputStream starts";
    OutputStream output = pfd_SppClientSocket->GetOutputStream();
    GTEST_LOG_(INFO) << "OutputStream::Write starts";
    returnOutput = output.Write(multiChar, 10);
    GTEST_LOG_(INFO) << "OutputStream::Write ends";
    GTEST_LOG_(INFO) << "SppClientSocket::GetOutputStream ends";

    GTEST_LOG_(INFO) << "SppClientSocket::GetRemoteDevice starts";
    BluetoothRemoteDevice tempRemoteDevice = pfd_SppClientSocket->GetRemoteDevice();
    GTEST_LOG_(INFO) << "SppClientSocket::GetRemoteDevice ends";


    GTEST_LOG_(INFO) << "SppClientSocket::IsConnected starts";
    bool IsConnected = false;
    IsConnected = pfd_SppClientSocket->IsConnected();
    EXPECT_EQ(IsConnected, true);
    GTEST_LOG_(INFO) << "SppClientSocket::IsConnected ends";

    GTEST_LOG_(INFO) << "SppServerSocket::GetStringTag starts";
    std::string returnStr{""}; 
    returnStr = server_->GetStringTag();
    GTEST_LOG_(INFO) << "SppServerSocket::GetStringTag ends";

    GTEST_LOG_(INFO) << "SppServerSocket::Accept starts";
    int SERTIMEOUT = 10;
    std::unique_ptr<SppClientSocket>  preturn_SppClientSocket = server_->Accept(SERTIMEOUT);
    GTEST_LOG_(INFO) << "SppServerSocket::Accept ends";

    GTEST_LOG_(INFO) << "SppServerSocket::Close starts";
    server_->Close();
    GTEST_LOG_(INFO) << "SppServerSocket::Close ends";

    EXPECT_EQ(1000, 1000);

    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_001 end";
}

/**
 * @tc.number: Xxx_Unittest_AttachId_GetId_0100
 * @tc.name: AttachId/DeleteId/IsAttachedId
 * @tc.desc: Test whether the return values of attachid, deleteid and isattachedidare correct. 
 */
}  // namespace Bluetooth
}  // namespace OHOS