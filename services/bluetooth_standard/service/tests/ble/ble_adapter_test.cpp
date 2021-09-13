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

#include "ble_mock.h"
#include <string>

using namespace bluetooth;
using namespace testing;
using namespace std;

BleMock *g_mocker = nullptr;
XmlMock *g_xmlmock = nullptr;
AdapterManagerMock *g_amMock;
GenResPriAddrResult g_genRPAcallback;
BleAdapter *g_bleAdapter = nullptr;
bool isExtendAdv = false;

BlePeripheralCallback_MOCK *blePeripheralCallback_ = new BlePeripheralCallback_MOCK();
AdapterBleObserver_MOCK *bleAdapterObserver_ = new AdapterBleObserver_MOCK();
BleAdvertiserCallback_MOCK *bleAdvertiseCallback_ = new BleAdvertiserCallback_MOCK();
BleCentralManagerCallback_MOCK *bleCentralManagerCallback_ = new BleCentralManagerCallback_MOCK();
IContextCallbackMock *g_callback = new IContextCallbackMock();
BaseObserverList<IAdapterBleObserver> g_observer{};
GattClientMock *g_gattClient = nullptr;

class BleAdapterTest : public testing::Test {
public:
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {
        delete bleAdvertiseCallback_;
        bleAdvertiseCallback_ = nullptr;
        delete bleCentralManagerCallback_;
        bleCentralManagerCallback_ = nullptr;
        delete bleAdapterObserver_;
        bleAdapterObserver_ = nullptr;
        delete blePeripheralCallback_;
        blePeripheralCallback_ = nullptr;
        delete g_callback;
        g_callback = nullptr;
    }
    void SetUp()
    {
        g_mocker = new BleMock();
        g_xmlmock = new XmlMock();
        g_amMock = new AdapterManagerMock();
        g_bleAdapter = new BleAdapter();
        g_gattClient = new GattClientMock();
        registerNewMockAdapterManager(g_amMock);
        registerBtmMock(g_mocker);
        registerGapMock(g_mocker);
        registerSdpMock(g_mocker);
        registerXmlMock(g_xmlmock);
        g_bleAdapter->GetContext()->RegisterCallback(*g_callback);
        g_bleAdapter->RegisterBleAdapterObserver(*bleAdapterObserver_);
        g_bleAdapter->RegisterBlePeripheralCallback(*blePeripheralCallback_);
        g_bleAdapter->RegisterBleAdvertiserCallback(*bleAdvertiseCallback_);
        g_bleAdapter->RegisterBleCentralManagerCallback(*bleCentralManagerCallback_);
        g_observer.Register(*bleAdapterObserver_);
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(g_gattClient));
        EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(BT_NO_ERROR));
        EXPECT_CALL(*g_callback, OnEnable(ADAPTER_NAME_BLE, true)).Times(1);
        EXPECT_CALL(*g_mocker, BTM_RegisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeSetSecurityMode()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeSetMinEncKeySize()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeSetBondMode()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*bleAdapterObserver_, OnDeviceAddrChanged("00:00:00:00:00:00")).Times(1);
        EXPECT_CALL(*g_mocker, GAPIF_LeSetRole()).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*bleAdapterObserver_, OnDeviceNameChanged("BluetoothDevice")).Times(AtLeast(0));
        EXPECT_CALL(*g_xmlmock, GetValue())
            .Times(0x05)
            .WillOnce(Return(0x03))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(0x03));
        g_bleAdapter->Enable();
        EXPECT_CALL(*bleAdvertiseCallback_, OnStartResultEvent(_, _, _)).Times(AtLeast(0));
        EXPECT_CALL(*bleCentralManagerCallback_, OnScanCallback(_)).Times(AtLeast(0));
        EXPECT_CALL(*bleCentralManagerCallback_, OnStartScanFailed(_)).Times(AtLeast(0));
        EXPECT_CALL(*bleCentralManagerCallback_, OnBleBatchScanResultsEvent(_)).Times(AtLeast(0));
        EXPECT_CALL(*blePeripheralCallback_, OnReadRemoteRssiEvent(_, 0, _)).Times(AtLeast(0));
        EXPECT_CALL(*blePeripheralCallback_, OnPairStatusChanged(ADAPTER_BLE, _, _)).Times(AtLeast(0));
        EXPECT_CALL(*bleAdapterObserver_, OnPairConfirmed(ADAPTER_BLE, _, 0, 0)).Times(AtLeast(0));
        EXPECT_CALL(*bleAdapterObserver_, OnPairRequested(ADAPTER_BLE, _)).Times(AtLeast(0));
        EXPECT_CALL(*bleAdapterObserver_, OnAdvertisingStateChanged(0)).Times(AtLeast(0));
        EXPECT_CALL(*bleAdapterObserver_, OnAdvertisingStateChanged(1)).Times(AtLeast(0));
    }
    void TearDown()
    {
        EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_BLE, true)).Times(AtLeast(0));
        EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(isExtendAdv));
        EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
        g_bleAdapter->DeregisterBlePeripheralCallback(*blePeripheralCallback_);
        g_bleAdapter->DeregisterBleAdapterObserver(*bleAdapterObserver_);
        g_bleAdapter->DeregisterBleCentralManagerCallback();
        g_bleAdapter->DeregisterBleSecurityCallback();
        EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeScanSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
        EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
        EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
        g_bleAdapter->Disable();
        delete g_bleAdapter;
        g_bleAdapter = nullptr;
        delete g_amMock;
        g_amMock = nullptr;
        delete g_mocker;
        g_mocker = nullptr;
        delete g_xmlmock;
        g_xmlmock = nullptr;
        delete g_gattClient;
        g_gattClient = nullptr;
    }
};

void LeConnectCom(BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00})
{
    uint8_t status = 0x00;
    uint16_t connectionHandle = 0x0001;
    uint8_t role = 0x01;
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(AtLeast(0)).WillOnce(Return(true));
    GetBtmCallbacks()->leConnectionComplete(status, connectionHandle, &addr, role, g_bleAdapter);
}
void TestStartScan(long reportDelayMillis = 0)
{
    BleScanSettingsImpl setting;
    setting.SetReportDelay(reportDelayMillis);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StartScan(setting);
}

void TestStopScan()
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetEnable()).Times(AtLeast(1)).WillRepeatedly(Return(0));
    g_bleAdapter->StopScan();
}

TEST_F(BleAdapterTest, BleScurityEncryptionComplete)
{
    uint8_t status = 0x00;
    BtAddr peerAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->encryptionComplete(status, &peerAddr, context);
}

TEST_F(BleAdapterTest, LeLocalEncryptionKeyReqEvent)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint64_t rand = 0;
    uint16_t ediv = 0x0000;
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->leLocalEncryptionKeyReqEvent(&addr, rand, ediv, context);
}

TEST_F(BleAdapterTest, LeRemoteEncryptionKeyReqEvent)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->leRemoteEncryptionKeyReqEvent(&addr, context);
}

TEST_F(BleAdapterTest, LeSignCounterChangeNotification)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GAP_SignCounterType type = LOCAL_SIGN_COUNTER;
    uint32_t counter = 0;
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->leSignCounterChangeNotification(&addr, type, counter, context);
}

TEST_F(BleAdapterTest, LeSignCounterChangeNotification2)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GAP_SignCounterType type = REMOTE_SIGN_COUNTER;
    uint32_t counter = 0;
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->leSignCounterChangeNotification(&addr, type, counter, context);
}

TEST_F(BleAdapterTest, LePairFeatureReq1)
{
    BtAddr peerAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(2).WillOnce(Return(3));
    GetLePairCallback()->lePairFeatureReq(&peerAddr, true, context);
}

TEST_F(BleAdapterTest, LePairFeatureReq2)
{
    BtAddr peerAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairFeatureReq(&peerAddr, false, context);
}

TEST_F(BleAdapterTest, LePairFeatureInd)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GapLePairFeature remoteFrature{.ioCapability = 0x00,
        .oobDataFlag = 0x00,
        .authReq = 0x00,
        .maxEncKeySize = 0x00,
        .initKeyDis = 0x00,
        .respKeyDis = 0x00};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairFeatureInd(&addr, remoteFrature, context);
}

TEST_F(BleAdapterTest, LePairMethodNotify)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t pairMethod = 0x00;
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairMethodNotify(&addr, pairMethod, context);
}

TEST_F(BleAdapterTest, LePairKeyPressNotification)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t pressType = 0x00;
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairKeyPressNotification(&addr, pressType, context);
}

TEST_F(BleAdapterTest, LePairPassKeyReq)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairPassKeyReq(&addr, context);
}

TEST_F(BleAdapterTest, LePairOobReq)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairOobReq(&addr, context);
}

TEST_F(BleAdapterTest, LePairScOobReq)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairScOobReq(&addr, context);
}

TEST_F(BleAdapterTest, LePairScUserConfirmReq)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t number = 0;
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairScUserConfirmReq(&addr, number, context);
}

TEST_F(BleAdapterTest, LePairCompelete_security)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t result = 0x00;
    uint8_t keyType = 0x00;
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairComplete(&addr, result, keyType, context);
}

TEST_F(BleAdapterTest, LePairKeyNotify)
{
    LeConnectCom();
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    LeEncKey remoteEncKey{.ltk = {0x00, 0x01, 0x02}, .rand = 0x00, .ediv = 0x00, .keySize = 0x03};
    LeIdKey remoteIdKey{.identityAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00}, .irk = {0x00, 0x01, 0x02}};
    LeSignKey remoteSignKey{.csrk = {0x00, 0x01, 0x02}, .counter = 0x00000000};
    LeEncKey localEncKey{.ltk = {0x00, 0x01, 0x02}, .rand = 0x00, .ediv = 0x00, .keySize = 0x03};
    LeSignKey localSignKey{.csrk = {0x00, 0x01, 0x02}, .counter = 0x00000000};
    LePairedKeys keys = {.remoteEncKey = &remoteEncKey,
        .remoteIdKey = &remoteIdKey,
        .remoteSignKey = &remoteSignKey,
        .localEncKey = &localEncKey,
        .localSignKey = &localSignKey};
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairKeyNotify(&addr, keys, context);
}

TEST_F(BleAdapterTest, GapRequestSigningAlgorithmInfo)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    void *context = GetBleSecurity();
    GetLeSecurityCallback()->GapRequestSigningAlgorithmInfo(&addr, context);
}

TEST_F(BleAdapterTest, LePairPassKeyNotification)
{
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint32_t number = 0;
    void *context = GetBleSecurity();
    GetLePairCallback()->lePairPassKeyNotification(&addr, number, context);
}

TEST_F(BleAdapterTest, GetAdvertiserHandle1)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    g_bleAdapter->GetAdvertiserHandle();
}

TEST_F(BleAdapterTest, GetAdvertiserHandle2)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(true));
    g_bleAdapter->GetAdvertiserHandle();
}

TEST_F(BleAdapterTest, GetDeviceType1)
{
    RawAddress device("01:00:00:00:00:00");
    int res = g_bleAdapter->GetDeviceType(device);
    EXPECT_EQ(BLE_BT_DEVICE_TYPE_UNKNOWN, res);
}

TEST_F(BleAdapterTest, PostEnable1)
{
    EXPECT_CALL(*g_mocker, GAPIF_LeSetRole()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillRepeatedly(Return(3));
    g_bleAdapter->PostEnable();
}

TEST_F(BleAdapterTest, SetLocalName_GetLocalName)
{
    std::string name = "TestBluetooth_BleUnitTestByWxy";
    EXPECT_CALL(*bleAdapterObserver_, OnDeviceNameChanged("TestBluetooth_BleUnitTestB")).Times(1);
    bool res = g_bleAdapter->SetLocalName(name);
    EXPECT_TRUE(res);
    g_bleAdapter->GetLocalName();
}

TEST_F(BleAdapterTest, GetLocalAddress)
{
    g_bleAdapter->GetLocalAddress();
}

TEST_F(BleAdapterTest, SetBondableMode)
{
    int mode = GAP_BONDABLE_MODE;
    EXPECT_TRUE(g_bleAdapter->SetBondableMode(mode));
}

TEST_F(BleAdapterTest, SetDevicePairingConfirmation1)
{
    RawAddress device("01:00:00:00:00:00");
    bool accept = true;
    g_bleAdapter->SetDevicePairingConfirmation(device, accept);
}

TEST_F(BleAdapterTest, SetDevicePasskey1)
{
    RawAddress device("01:00:00:00:00:00");
    int passkey = 123456;
    bool accept = true;
    g_bleAdapter->SetDevicePasskey(device, passkey, accept);
}

TEST_F(BleAdapterTest, CancelPairing_false)
{
    RawAddress device("01:00:00:00:00:00");
    g_bleAdapter->CancelPairing(device);
}

TEST_F(BleAdapterTest, CancelPairing_true)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    g_bleAdapter->LePairingStatus(device);
    EXPECT_CALL(*g_mocker, GAPIF_LeCancelPair()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->CancelPairing(device);
}

TEST_F(BleAdapterTest, SetDevicePasskey2)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    int passkey = 123456;
    bool accept = true;
    g_bleAdapter->SetDevicePasskey(device, passkey, accept);
}

TEST_F(BleAdapterTest, SetDevicePairingConfirmation2)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    bool accept = true;
    g_bleAdapter->SetDevicePairingConfirmation(device, accept);
}

TEST_F(BleAdapterTest, StartPair2)
{
    RawAddress device("01:00:00:00:00:00");
    g_bleAdapter->StartPair(device);
}

TEST_F(BleAdapterTest, GetDeviceName)
{
    RawAddress device("02:00:00:00:00:00");
    g_bleAdapter->GetDeviceName(device);
}

TEST_F(BleAdapterTest, GetDeviceUuids)
{
    RawAddress device("02:00:00:00:00:00");
    std::vector<Uuid> uuids = g_bleAdapter->GetDeviceUuids(device);
}

TEST_F(BleAdapterTest, GetPairedDevices)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    g_bleAdapter->GetPairedDevices();
}

TEST_F(BleAdapterTest, GetConnectedDevices)
{
    g_bleAdapter->GetConnectedDevices();
}

TEST_F(BleAdapterTest, IsBondedFromLocal)
{
    RawAddress device("02:00:00:00:00:00");
    g_bleAdapter->IsBondedFromLocal(device);
}

TEST_F(BleAdapterTest, PairRequestReply)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    bool accept = true;
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(2).WillOnce(Return(0));
    g_bleAdapter->PairRequestReply(device, accept);
}

TEST_F(BleAdapterTest, IsAclConnected)
{
    RawAddress device("02:00:00:00:00:00");
    g_bleAdapter->IsAclConnected(device);
}

TEST_F(BleAdapterTest, IsAclEncrypted)
{
    RawAddress device("02:00:00:00:00:00");
    g_bleAdapter->IsAclEncrypted(device);
}

TEST_F(BleAdapterTest, GetPairState)
{
    RawAddress device("01:00:00:00:00:00");
    g_bleAdapter->GetPairState(device);
    LeConnectCom();
    g_bleAdapter->GetPairState(device);
}

TEST_F(BleAdapterTest, GetBondableMode)
{
    g_bleAdapter->GetBondableMode();
}

TEST_F(BleAdapterTest, GetBleMaxAdvertisingDataLength1)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillOnce(Return(0));
    EXPECT_EQ(512, g_bleAdapter->GetBleMaxAdvertisingDataLength());
}

TEST_F(BleAdapterTest, GetBleMaxAdvertisingDataLength2)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillOnce(Return(1));
    g_bleAdapter->GetBleMaxAdvertisingDataLength();
}

TEST_F(BleAdapterTest, GetBleMaxAdvertisingDataLength3)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(false));
    g_bleAdapter->GetBleMaxAdvertisingDataLength();
}

TEST_F(BleAdapterTest, SetIoCapability)
{
    int ioCapability = GAP_IO_DISPLAYONLY;
    g_bleAdapter->SetIoCapability(ioCapability);
    g_bleAdapter->GetIoCapability();
}

TEST_F(BleAdapterTest, IsBleEnabled)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    g_bleAdapter->IsBleEnabled();
}

TEST_F(BleAdapterTest, IsBtDiscovering)
{
    g_bleAdapter->IsBtDiscovering();
}

TEST_F(BleAdapterTest, ReadRemoteRssiValue)
{
    RawAddress device("02:00:00:00:00:00");
    g_bleAdapter->ReadRemoteRssiValue(device);
}

TEST_F(BleAdapterTest, LePairComplete)
{
    RawAddress device1("03:00:00:00:00:00");
    BtAddr addr1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x03}, 0x00};
    RawAddress device2("04:00:00:00:00:00");
    BtAddr addr2 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x04}, 0x00};
    LeConnectCom(addr1);
    g_bleAdapter->LePairingStatus(device1);
    LeConnectCom(addr1);
    LeConnectCom(addr2);
    int status = 0x00;
    g_bleAdapter->LePairComplete(device1, status);
    g_bleAdapter->LePairComplete(device2, status);
    g_bleAdapter->GetPairedDevices();
    g_bleAdapter->RemovePair(device1);
    g_bleAdapter->RemoveAllPairs();
    RawAddress device3("05:00:00:00:00:00");
    BtAddr addr3 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x04}, 0x00};
    LeConnectCom(addr3);
    g_bleAdapter->LePairComplete(device3, 0x01);
    g_bleAdapter->LePairComplete(device3, status);
}

TEST_F(BleAdapterTest, EncryptionComplete)
{
    RawAddress device1("02:00:00:00:00:00");
    g_bleAdapter->EncryptionComplete(device1);
}

TEST_F(BleAdapterTest, LeConnectionComplete)
{
    uint8_t status = 0x00;
    uint16_t connectionHandle = 0x0001;
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t role = 0x01;
    GetBtmCallbacks()->leConnectionComplete(status, connectionHandle, &addr, role, g_bleAdapter);
    role = 0x01;

    RawAddress device("01:00:00:00:00:00");
    int pairStatus = 0x00;
    g_bleAdapter->LePairComplete(device, pairStatus);

    GetBtmCallbacks()->leConnectionComplete(status, connectionHandle, &addr, role, g_bleAdapter);
    status = 0x01;
    GetBtmCallbacks()->leConnectionComplete(status, connectionHandle, &addr, role, g_bleAdapter);
}

TEST_F(BleAdapterTest, ReadRssiComplete)
{
    uint8_t status = 0x01;
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    int8_t rssi = 0x00;
    GetBtmCallbacks()->readRssiComplete(status, &addr, rssi, g_bleAdapter);
    status = 0x00;
    GetBtmCallbacks()->readRssiComplete(status, &addr, rssi, g_bleAdapter);
}

TEST_F(BleAdapterTest, LeDisconnectionComplete)
{
    uint8_t status = 0x00;
    uint16_t connectionHandle = 0x0001;
    uint8_t reason = 0x01;
    GetBtmCallbacks()->leDisconnectionComplete(status, connectionHandle, reason, g_bleAdapter);
    status = 0x01;
    GetBtmCallbacks()->leDisconnectionComplete(status, connectionHandle, reason, g_bleAdapter);
}

TEST_F(BleAdapterTest, CancelPairing_SetDevicePasskey3_SetDevicePairingConfirmation3)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    g_bleAdapter->CancelPairing(device);
    int passkey = 123456;
    bool accept = false;
    g_bleAdapter->SetDevicePasskey(device, passkey, accept);
    g_bleAdapter->SetDevicePairingConfirmation(device, accept);
}

TEST_F(BleAdapterTest, RemovePair_RemoveAllPairs_false)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    g_bleAdapter->RemovePair(device);
    g_bleAdapter->RemoveAllPairs();
}

TEST_F(BleAdapterTest, IsRemovePairedDevice)
{
    RawAddress device("01:00:00:00:00:00");
    g_bleAdapter->IsRemovePairedDevice(device);
    LeConnectCom();
    g_bleAdapter->IsRemovePairedDevice(device);
}

TEST_F(BleAdapterTest, StartAdvertising_false1)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_CODED);
    settings.SetSecondaryPhy(PHY_LE_1M);
    settings.SetConnectable(true);
    settings.IsConnectable();
    settings.SetInterval(4000);
    settings.SetTxPower(0);
    settings.SetTxPower(1);
    settings.SetTxPower(2);
    settings.SetTxPower(3);
    settings.GetTxPower();
    BleAdvertiserDataImpl advData;
    std::string str;
    str.insert(0, 32, '0');
    advData.AddData(str);
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    g_bleAdapter->GetAdvertisingStatus();
}

TEST_F(BleAdapterTest, StartAdvertising_false2)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_CODED);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    std::string str;
    str.insert(0, 32, '0');
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData(str);
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false3)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    BleAdvertiserDataImpl advData;
    int manufacturerId = 0;
    std::string data = "0";
    advData.AddManufacturerData(manufacturerId, data);
    std::string name = "TestBluetooth_BleUnitTestByWxy";
    advData.SetDeviceName(name);
    name = "TestBluetooth_BleUnitTest";
    advData.SetDeviceName(name);
    int txPowerLevel = 0;
    advData.SetTxPowerLevel(txPowerLevel);
    uint8_t flag = 0x04;
    advData.SetFlags(flag);
    Uuid uuidMain;
    uint16_t uuid1 = 0x00FF;
    Uuid uuidService1 = uuidMain.ConvertFrom16Bits(uuid1);
    advData.AddServiceUuid(uuidService1);

    uint32_t uuid2 = 0xFFFFFFFF;
    Uuid uuidService2 = uuidMain.ConvertFrom32Bits(uuid2);
    advData.AddServiceUuid(uuidService2);

    std::array<uint8_t, 16> uuid3 = {0x00};
    Uuid uuidService3 = uuidMain.ConvertFrom128Bits(uuid3);

    advData.AddServiceUuid(uuidService3);
    advData.AddServiceData(uuidService1, data);
    advData.AddServiceData(uuidService2, data);
    advData.AddServiceData(uuidService3, data);
    uint16_t appearance = 0x0000;
    advData.SetAppearance(appearance);
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false4)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    BleAdvertiserDataImpl advData;
    std::string str;
    str.insert(0, 32, '0');
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData(str);
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false5)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_CODED);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false6)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_1M);
    settings.SetSecondaryPhy(PHY_LE_CODED);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false7)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_1M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    std::string str;
    str.insert(0, 32, '0');
    BleAdvertiserDataImpl advData;
    advData.AddData(str);
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false8)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_1M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    std::string str;
    str.insert(0, 32, '0');
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData(str);
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false9)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillOnce(Return(1));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false10)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_OFF));
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_false11)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetParam()).Times(1).WillOnce(Return(0));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertising_true)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetParam()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(4).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}
void TestStartAdv()
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(0x02).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetParam()).Times(1).WillOnce(Return(0));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

void TestStopAdv()
{
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(AtLeast(0)).WillOnce(Return(false));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, RestartAdv)
{
    TestStartAdv();
    uint8_t status = 0x00;
    uint16_t connectionHandle = 0x0001;
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    uint8_t role = 0x01;
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    GetBtmCallbacks()->leConnectionComplete(status, connectionHandle, &addr, role, g_bleAdapter);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetParamResult1)
{
    TestStartAdv();
    uint8_t status = 0x00;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvReadTxPower()).Times(1).WillOnce(Return(1));
    GetAdvCallback()->advSetParamResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetParamResult2)
{
    TestStartAdv();
    uint8_t status = 0x01;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advSetParamResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvReadTxPower1)
{
    TestStartAdv();
    uint8_t status = 0x00;
    int8_t txPower = 0x00;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetData()).Times(1).WillOnce(Return(1));
    GetAdvCallback()->advReadTxPower(status, txPower, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvReadTxPower2)
{
    TestStartAdv();
    uint8_t status = 0x01;
    int8_t txPower = 0x00;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advReadTxPower(status, txPower, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetDataResult1)
{
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetScanRspData()).Times(1).WillOnce(Return(1));
    TestStartAdv();
    uint8_t status = 0x00;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advSetDataResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetDataResult2)
{
    TestStartAdv();
    uint8_t status = 0x01;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advSetDataResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetScanRspDataResult1)
{
    TestStartAdv();
    uint8_t status = 0x00;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillRepeatedly(Return(1));
    GetAdvCallback()->advSetScanRspDataResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetScanRspDataResult2)
{
    TestStartAdv();
    uint8_t status = 0x01;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advSetScanRspDataResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetEnableResult1)
{
    TestStartAdv();
    uint8_t status = 0x00;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(2).WillRepeatedly(Return(BLE_ADDR_TYPE_RPA));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    GetAdvCallback()->advSetEnableResult(status, context);
    uint8_t addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    g_genRPAcallback = *GetGenRPACallback();
    g_genRPAcallback(0, addr, context);
    GetAdvCallback()->advSetEnableResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetEnableResult2)
{
    TestStartAdv();
    uint8_t status = 0x01;
    void *context = GetBleAdvertiser();
    GetAdvCallback()->advSetEnableResult(status, context);
    TestStopAdv();
}

TEST_F(BleAdapterTest, AdvSetEnableResult3)
{
    TestStartAdv();
    uint8_t status = 0x00;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
    GetAdvCallback()->advSetEnableResult(status, context);
}

TEST_F(BleAdapterTest, AdvSetEnableResult4)
{
    TestStartAdv();
    uint8_t status = 0x01;
    void *context = GetBleAdvertiser();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
    GetAdvCallback()->advSetEnableResult(status, context);
}

TEST_F(BleAdapterTest, Close)
{
    g_bleAdapter->Close(BLE_LEGACY_ADVERTISING_HANDLE);
    TestStartAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->Close(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartScan1_1)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_OFF));
    g_bleAdapter->StartScan();
}

TEST_F(BleAdapterTest, StartScan1_2)
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->StartScan();
}

TEST_F(BleAdapterTest, StartScans2_1)
{
    BleScanSettingsImpl settings;
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_OFF));
    g_bleAdapter->StartScan(settings);
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_OFF));
    g_bleAdapter->StartScan();
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_OFF));
    g_bleAdapter->StartScan(settings);
    g_bleAdapter->StopScan();
}

TEST_F(BleAdapterTest, StartScans2_3)
{
    BleScanSettingsImpl setting;
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(1));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, StartScanAlready)
{
    TestStartScan();
    BleScanSettingsImpl setting;
    setting.SetReportDelay(0);
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, ScanSetParamResult1)
{
    TestStartScan(1);
    uint8_t status = 0x00;
    void *context = GetBleCentraManager();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1);
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetEnable()).Times(2).WillRepeatedly(Return(1));
    GetScanCallback()->scanSetParamResult(status, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, StartScans2_4)
{
    BleScanSettingsImpl setting;
    setting.SetReportDelay(1);
    setting.SetScanMode(1);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, StartScans2_5)
{
    BleScanSettingsImpl setting;
    setting.SetScanMode(1);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, StartScans3_1)
{
    BleScanSettingsImpl setting;
    setting.SetScanMode(2);
    setting.SetReportDelay(1);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, StartScans3_2)
{
    BleScanSettingsImpl setting;
    setting.SetScanMode(2);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StartScan(setting);
    TestStopScan();
}

TEST_F(BleAdapterTest, SetBleScanSettings2_ScanSetParamResult2)
{
    BleScanSettingsImpl settings;
    settings.SetReportDelay(0);
    TestStartScan();
    uint8_t status = 0x01;
    void *context = GetBleCentraManager();
    GetScanCallback()->scanSetParamResult(status, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, ScanSetEnableResult1)
{
    TestStartScan();
    uint8_t status = 0x01;
    void *context = GetBleCentraManager();
    GetScanCallback()->scanSetEnableResult(status, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, AdvertisingReport1)
{
    TestStartScan();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(GAP_LE_ROLE_OBSERVER));
    uint8_t advType = 0x00;
    BtAddr peerAddr{.addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, .type = 1};
    uint8_t dataLen = 0x01;
    uint8_t data[2] = {0x00, 0x09};
    int8_t rssi = 0x00;
    void *context = GetBleCentraManager();
    GapAdvReportParam reportParam{.dataLen = dataLen, .data = data, .rssi = rssi};
    GetScanCallback()->advertisingReport(advType, &peerAddr, reportParam, &peerAddr, context);

    BlePeripheralDevice dev;
    uint8_t payload1[] = {4, BLE_AD_TYPE_NAME_CMPL, 0x41, 0x41, 0x41};
    size_t total_len = 4;
    BlePeripheralDeviceParseAdvData parseAdvData = {
        .payload = payload1,
        .length = total_len,
    };
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload2[] = {4, BLE_AD_TYPE_TX_PWR, 0x42};
    parseAdvData.payload = payload2;
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload3[] = {4, BLE_AD_TYPE_APPEARANCE, 0x43, 0x42};
    parseAdvData.payload = payload3;
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload4[] = {4, BLE_AD_TYPE_FLAG, 0x41};
    parseAdvData.payload = payload4;
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload5[] = {4, BLE_AD_TYPE_16SRV_CMPL, 0x44, 0x43};
    parseAdvData.payload = payload5;
    dev.ParseAdvertiserment(parseAdvData);
    total_len = 8;
    uint8_t payload6[] = {8, BLE_AD_TYPE_32SRV_CMPL, 0x44, 0x43, 0x00, 0x00};
    dev.ParseAdvertiserment(parseAdvData);
    parseAdvData.payload = payload6;
    uint8_t payload7[18] = {8, BLE_AD_TYPE_128SRV_CMPL, 0x44, 0x43, 0x00, 0x00};
    parseAdvData.payload = payload7;
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload8[18] = {8, BLE_AD_TYPE_128SRV_PART, 0x44, 0x43, 0x00, 0x00};
    parseAdvData.payload = payload8;
    dev.ParseAdvertiserment(parseAdvData);
    // total_len = 4;
    uint8_t payload9[] = {4, BLE_AD_MANUFACTURER_SPECIFIC_TYPE, 0x41, 0x42, 0x43};
    parseAdvData.payload = payload9;
    parseAdvData.length = 4;
    dev.ParseAdvertiserment(parseAdvData);
    uint8_t payload10[] = {4, BLE_AD_TYPE_SERVICE_DATA, 0x44, 0x43, 0x42};
    parseAdvData.payload = payload10;
    dev.ParseAdvertiserment(parseAdvData);
    // total_len = 8;
    uint8_t payload11[10] = {8, BLE_AD_TYPE_32SERVICE_DATA, 0x44, 0x43, 0x00, 0x00, 0x00};
    parseAdvData.payload = payload11;
    parseAdvData.length = 8;
    dev.ParseAdvertiserment(parseAdvData);
    TestStopScan();
}

TEST_F(BleAdapterTest, BlePeripheralDevice)
{
    TestStartScan();
    BlePeripheralDevice dev;
    size_t total_len = 18;
    uint8_t payload12[] = {18,
        BLE_AD_TYPE_128SERVICE_DATA,
        0x44,
        0x43,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x41};
    BlePeripheralDeviceParseAdvData paraseData = {
        .payload = payload12,
        .length = total_len
    };
    dev.ParseAdvertiserment(paraseData);
    dev.SetBondedFromLocal(true);
    dev.GetName();
    dev.GetAppearance();
    dev.GetManufacturerData();
    dev.GetRSSI();
    dev.IsRSSI();
    dev.IsBondedFromLocal();
    dev.IsServiceData();
    dev.GetServiceData(0);
    dev.GetAdFlag();
    TestStopScan();
}

TEST_F(BleAdapterTest, AdvertisingReport2)
{
    TestStartScan();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(2).WillOnce(Return(BT_NO_ERROR)).WillOnce(Return(1));
    uint8_t advType = 0x00;
    BtAddr peerAddr{.addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, .type = 1};
    uint8_t dataLen = 0x01;
    uint8_t data[2] = {0x00, 0x09};
    int8_t rssi = 0x00;
    void *context = GetBleCentraManager();
    GapAdvReportParam reportParam{.dataLen = dataLen, .data = data, .rssi = rssi};
    GetScanCallback()->advertisingReport(advType, &peerAddr, reportParam, &peerAddr, context);
    GetScanCallback()->advertisingReport(advType, &peerAddr, reportParam, &peerAddr, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, AdvertisingReport3)
{
    TestStartScan();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(2).WillOnce(Return(2)).WillOnce(Return(GAP_LE_ROLE_OBSERVER));
    uint8_t advType = 0x00;
    BtAddr peerAddr{.addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, .type = 1};
    uint8_t dataLen = 0x01;
    uint8_t data[2] = {0x00, 0x09};
    int8_t rssi = 0x00;
    void *context = GetBleCentraManager();
    GapAdvReportParam reportParam{.dataLen = dataLen, .data = data, .rssi = rssi};
    GetScanCallback()->advertisingReport(advType, &peerAddr, reportParam, &peerAddr, context);
    GetScanCallback()->advertisingReport(advType, &peerAddr, reportParam, &peerAddr, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, GetPeerDeviceAddrType)
{
    RawAddress device("01:00:00:00:00:00");
    LeConnectCom();
    g_bleAdapter->GetDeviceType(device);
    g_bleAdapter->GetPeerDeviceAddrType(device);
}

TEST_F(BleAdapterTest, ScanSetEnableResult2)
{
    TestStartScan();
    uint8_t status = 0x00;
    void *context = GetBleCentraManager();
    GetScanCallback()->scanSetEnableResult(status, context);
    TestStopScan();
}

TEST_F(BleAdapterTest, StopScan)
{
    g_bleAdapter->StopScan();
    g_bleAdapter->StopScan();
}

TEST_F(BleAdapterTest, ScanSetEnableResult3)
{
    TestStartScan();
    uint8_t status = 0x01;
    void *context = GetBleCentraManager();
    TestStopScan();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(4).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->Disable();
    GetScanCallback()->scanSetEnableResult(status, context);
}

TEST_F(BleAdapterTest, ScanSetEnableResult4)
{
    TestStartScan();
    uint8_t status = 0x00;
    void *context = GetBleCentraManager();
    TestStopScan();
    GetScanCallback()->scanSetEnableResult(status, context);
}

TEST_F(BleAdapterTest, StartScans2_6)
{
    BleScanSettingsImpl setting;
    setting.SetScanMode(1);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetParam()).Times(1).WillOnce(Return(BT_NO_ERROR));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, ble_properties)
{
    BleProperties &bleProperties = BleProperties::GetInstance();
    EXPECT_CALL(*g_mocker, GAPIF_LeSetBondMode()).Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(3));
    bleProperties.SetBondableMode(BLE_BONDABLE_MODE_NONE);
    bleProperties.SetBondableMode(2);
    bleProperties.LoadBleConfigInfo();
    bleProperties.GetBondableMode();
    bleProperties.GetPasskey();
}

TEST_F(BleAdapterTest, EnableDisable_Error)
{
    BleAdapter bleAdapter;
    bleAdapter.Disable();
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(1));
    bleAdapter.Enable();
    EXPECT_CALL(*g_mocker, GAPIF_LeSetBondMode()).Times(1).WillOnce(Return(BT_NO_ERROR));
    int mode = BLE_BONDABLE_MODE_NONE;
    EXPECT_TRUE(g_bleAdapter->SetBondableMode(mode));
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(4).WillRepeatedly(Return(3));
    EXPECT_CALL(*bleAdapterObserver_, OnDeviceAddrChanged("00:00:00:00:00:00")).Times(1);
    EXPECT_CALL(*g_mocker, GAPIF_LeSetRole()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_RegisterAclCallbacks()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetSecurityMode()).Times(2).WillRepeatedly(Return(BT_NOT_SUPPORT));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetMinEncKeySize()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetBondMode()).Times(2).WillRepeatedly(Return(1));
    bleAdapter.Enable();
}

TEST_F(BleAdapterTest, GetAdvertisingStatus)
{
    g_bleAdapter->GetAdvertisingStatus();
    BleAdapter bleAdapter;
    bleAdapter.GetAdvertisingStatus();
}

TEST_F(BleAdapterTest, IsLlPrivacySupported)
{
    g_bleAdapter->IsLlPrivacySupported();
}

TEST_F(BleAdapterTest, AddCharacteristicValue)
{
    uint8_t adtype = 0x00;
    std::string data = "test";
    g_bleAdapter->AddCharacteristicValue(adtype, data);
}

TEST_F(BleAdapterTest, ExtentAdvertising1)
{
    isExtendAdv = true;
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetParam()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, ExtentAdvertising2)
{
    isExtendAdv = false;
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetParam()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, ExtentAdvertising3)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetParam()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, ExtentAdvertising4)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetParam()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(false));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(false);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, LegacyAdvertising1)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetParam()).Times(1).WillOnce(Return(1));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, LegacyAdvertising2)
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetParam()).Times(1).WillOnce(Return(0));
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData("0");
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData("0");
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

void TestStartExAdv(bool isConnect = true, bool isLegacy = false, string data = "0", string rspdata = "0")
{
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    if (!isLegacy) {
        EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension())
            .Times(1)
            .WillRepeatedly(Return(true));
    }
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(0x02).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExAdvCallback()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetParam()).Times(1).WillRepeatedly(Return(0));
    BleAdvertiserSettingsImpl settings;
    settings.SetConnectable(isConnect);
    settings.SetLegacyMode(isLegacy);
    settings.SetPrimaryPhy(PHY_LE_2M);
    settings.SetSecondaryPhy(PHY_LE_1M);
    BleAdvertiserDataImpl advData;
    advData.AddData(data);
    BleAdvertiserDataImpl scanResponse;
    scanResponse.AddData(rspdata);
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

void TestStopExAdv()
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(AtLeast(0)).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    g_bleAdapter->StopAdvertising(BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, StartAdvertisingAlready)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(true));
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
}

TEST_F(BleAdapterTest, ExAdvSetRandAddrResult)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetRandAddrResult(0x00, GetBleAdvertiser());
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(1));
    GetExAdvCallback()->exAdvSetRandAddrResult(0x00, GetBleAdvertiser());
    TestStartExAdv();
    GetExAdvCallback()->exAdvSetRandAddrResult(0x01, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvSetParamResult)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetData()).Times(1).WillOnce(Return(1));
    GetExAdvCallback()->exAdvSetParamResult(0x01, 0x00, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_ADDR_TYPE_RPA));
    GetExAdvCallback()->exAdvSetDataResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
    TestStartExAdv(false);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetScanRspData()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvDataFragment_ExResDataFragment1)
{
    std::string str;
    str.insert(0, 192, 'A');
    TestStartExAdv(true, false, str);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetData()).Times(2).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
    TestStopExAdv();
    TestStartExAdv(false, false, "0", str);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetScanRspData()).Times(2).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
}

TEST_F(BleAdapterTest, ExAdvDataFragment_ExResDataFragment2)
{
    std::string str;
    str.insert(0, 382, 'A');
    TestStartExAdv(true, false, str);
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetData()).Times(3).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
    TestStopExAdv();
    TestStartExAdv(false, false, "0", str);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvGetMaxDataLen()).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetScanRspData()).Times(3).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
}

TEST_F(BleAdapterTest, ExAdvSetDataResult)
{
    TestStartExAdv(true, true);
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetScanRspData()).Times(1).WillOnce(Return(1));
    GetExAdvCallback()->exAdvSetDataResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
    GetExAdvCallback()->exAdvSetDataResult(0x01, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_ADDR_TYPE_RPA));
    GetExAdvCallback()->exAdvSetDataResult(0x00, GetBleAdvertiser());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(1));
    GetExAdvCallback()->exAdvSetDataResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvSetScanRspDataResult)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetData()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeDataPacketLengthExtension()).Times(1).WillOnce(Return(false));
    GetExAdvCallback()->exAdvSetParamResult(0x00, 0x00, GetBleAdvertiser());
    TestStartExAdv();
    GetExAdvCallback()->exAdvSetScanRspDataResult(0x01, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_ADDR_TYPE_RPA));
    GetExAdvCallback()->exAdvSetScanRspDataResult(0x00, GetBleAdvertiser());
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetRandAddr()).Times(1).WillOnce(Return(1));
    uint8_t addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    g_genRPAcallback = *GetGenRPACallback();
    g_genRPAcallback(0, addr, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    GetExAdvCallback()->exAdvSetScanRspDataResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvSetEnableResult01)
{
    TestStartExAdv();
    GetExAdvCallback()->exAdvSetEnableResult(0x01, GetBleAdvertiser());
    TestStartExAdv();
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_ADDR_TYPE_RPA));
    GetExAdvCallback()->exAdvSetEnableResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
    GetExAdvCallback()->exAdvSetEnableResult(0x01, GetBleAdvertiser());
    GetExAdvCallback()->exAdvSetEnableResult(0x00, GetBleAdvertiser());
}

TEST_F(BleAdapterTest, ExAdvSetEnableResult02)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(2).WillRepeatedly(Return(0));
    g_bleAdapter->RemoveAllPairs();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(4).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->Disable();
    GetExAdvCallback()->exAdvSetEnableResult(0x01, GetBleAdvertiser());
    GetExAdvCallback()->exAdvSetEnableResult(0x00, GetBleAdvertiser());
}

TEST_F(BleAdapterTest, ExAdvSetEnableResult03)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(1).WillRepeatedly(Return(0));
    g_bleAdapter->Disable();
    GetExAdvCallback()->exAdvSetEnableResult(0x01, GetBleAdvertiser());
    GetExAdvCallback()->exAdvSetEnableResult(0x00, GetBleAdvertiser());
}

TEST_F(BleAdapterTest, ExAdvRemoveHandleResult)
{
    TestStartExAdv();
    GetExAdvCallback()->exAdvRemoveHandleResult(0x01, GetBleAdvertiser());
    GetExAdvCallback()->exAdvRemoveHandleResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvClearHandleResult)
{
    TestStartExAdv();
    GetExAdvCallback()->exAdvClearHandleResult(0x01, GetBleAdvertiser());
    GetExAdvCallback()->exAdvClearHandleResult(0x00, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvScanRequestReceived)
{
    TestStartExAdv();
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GetExAdvCallback()->exAdvScanRequestReceived(0x01, &addr, GetBleAdvertiser());
    GetExAdvCallback()->exAdvScanRequestReceived(0x00, &addr, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExAdvTerminatedAdvSet)
{
    TestStartExAdv();
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    GetExAdvCallback()->exAdvTerminatedAdvSet(0x00, 0x00, 0x0001, 0x01, GetBleAdvertiser());
    TestStopExAdv();
}

TEST_F(BleAdapterTest, ExScan_false1)
{
    BleScanSettingsImpl setting;
    setting.SetReportDelay(1);
    setting.SetLegacy(false);
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExScanCallback()).Times(1).WillOnce(Return(1));
    g_bleAdapter->StartScan(setting);
}

TEST_F(BleAdapterTest, ExScan_false2)
{
    BleScanSettingsImpl setting;
    setting.SetReportDelay(1);
    setting.SetLegacy(false);
    setting.SetPhy(PHY_LE_1M);
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExScanCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetParam()).Times(1).WillOnce(Return(1));
    g_bleAdapter->StartScan(setting);
}

void TestStartExScan(long reportDelayMillis = 1, bool isLegacy = false, int phy = PHY_LE_ALL_SUPPORTED)
{
    BleScanSettingsImpl setting;
    setting.SetReportDelay(reportDelayMillis);
    setting.SetLegacy(isLegacy);
    setting.SetPhy(phy);
    EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(1).WillOnce(Return(STATE_TURN_ON));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterExScanCallback()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetParam()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StartScan(setting);
}

void TestStopExScan()
{
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetEnable()).Times(1).WillOnce(Return(0));
    g_bleAdapter->StopScan();
}

TEST_F(BleAdapterTest, ScanExSetParamResult)
{
    TestStartExScan(0);
    EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetEnable()).Times(1).WillOnce(Return(1));
    GetExScanCallback()->scanExSetParamResult(0x01, GetBleCentraManager());
    GetExScanCallback()->scanExSetParamResult(0x00, GetBleCentraManager());
    TestStopExScan();
    GetExScanCallback()->scanExSetParamResult(0x00, GetBleCentraManager());
}

TEST_F(BleAdapterTest, ExAdvertisingReport)
{
    TestStartExScan();
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GapExAdvReportParam param;
    uint8_t data[2] = {0x02, 0x01};
    param.data = data;
    param.dataLen = 0x01;
    param.rssi = 0x01;
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_GENERAL));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_NON_DISC));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_NON_DISC));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_GENERAL));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());

    GetExScanCallback()->exAdvertisingReport(0x20, &addr, param, &addr, GetBleCentraManager());
    GetExScanCallback()->exAdvertisingReport(0x20, &addr, param, &addr, GetBleCentraManager());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_GENERAL));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    GetExScanCallback()->exAdvertisingReport(0x20, &addr, param, &addr, GetBleCentraManager());
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_GENERAL));
    GetExScanCallback()->exAdvertisingReport(0x40, &addr, param, &addr, GetBleCentraManager());
    TestStopExScan();
}

TEST_F(BleAdapterTest, ExAdvertisingReport_Legacy)
{
    TestStartExScan(1, true);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    GapExAdvReportParam param;
    uint8_t data[2] = {0x02, 0x01};
    param.data = data;
    param.dataLen = 0x01;
    param.rssi = 0x01;
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    TestStopExScan();
}

TEST_F(BleAdapterTest, ExAdvertisingReport_NoData)
{
    TestStartExScan(1, false, PHY_LE_CODED);
    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, 0x00};
    static GapExAdvReportParam param{};
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(1).WillOnce(Return(BLE_SCAN_MODE_GENERAL));
    GetExScanCallback()->exAdvertisingReport(0x00, &addr, param, &addr, GetBleCentraManager());
    TestStopExScan();
}

TEST_F(BleAdapterTest, ScanExSetEnableResult)
{
    BleAdapter bleAdapter;
    EXPECT_CALL(*g_mocker, BTM_Enable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_RegisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetSecurityMode()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetMinEncKeySize()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetBondMode()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeSetRole()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*bleAdapterObserver_, OnDeviceAddrChanged("00:00:00:00:00:00")).Times(1);
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(5).WillOnce(Return(3)).WillOnce(Return(0)).WillRepeatedly(Return(3));
    bleAdapter.Enable();
    TestStartExScan(0);
    GetExScanCallback()->scanExSetEnableResult(0x01, GetBleCentraManager());
    GetExScanCallback()->scanExSetEnableResult(0x00, GetBleCentraManager());
    TestStopExScan();
    EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    bleAdapter.Disable();
    GetExScanCallback()->scanExSetEnableResult(0x01, GetBleCentraManager());
    GetExScanCallback()->scanExSetEnableResult(0x00, GetBleCentraManager());
}

TEST_F(BleAdapterTest, ExAdvSetEnableResult_stopAlladv)
{
    TestStartExAdv();
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising()).Times(1).WillRepeatedly(Return(true));
    BleAdvertiserSettingsImpl settings;
    BleAdvertiserDataImpl advData;
    BleAdvertiserDataImpl scanResponse;
    g_bleAdapter->StartAdvertising(settings, advData, scanResponse, BLE_LEGACY_ADVERTISING_HANDLE);
    EXPECT_CALL(*g_callback, OnDisable(ADAPTER_NAME_BLE, true)).Times(AtLeast(0));
    EXPECT_CALL(*g_mocker, BTM_IsControllerSupportLeExtendedAdvertising())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(isExtendAdv));
    EXPECT_CALL(*g_mocker, GAPIF_RegisterAdvCallback()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    g_bleAdapter->DeregisterBlePeripheralCallback(*blePeripheralCallback_);
    g_bleAdapter->DeregisterBleAdapterObserver(*bleAdapterObserver_);
    g_bleAdapter->DeregisterBleCentralManagerCallback();
    g_bleAdapter->DeregisterBleSecurityCallback();
    EXPECT_CALL(*g_mocker, GAPIF_LeAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeScanSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExAdvSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, GAPIF_LeExScanSetEnable()).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_mocker, BTM_DeregisterAclCallbacks()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_mocker, BTM_Disable()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
    g_bleAdapter->Disable();
    GetExAdvCallback()->exAdvSetEnableResult(0x01, GetBleAdvertiser());
}