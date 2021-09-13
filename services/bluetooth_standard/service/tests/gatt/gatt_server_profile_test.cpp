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

#include "gatt_connection_manager.h"
#include "gatt_server_profile.h"
#include "gatt_server_service.h"
#include "gatt_service_base.h"
#include "log.h"
#include "gatt_defines.h"
#include "string.h"
#include "bt_def.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "mock/att_connect_mock.h"
#include "mock/att_send_mock.h"
#include "mock/ble_adapter_mock.h"
#include "mock/classic_adapter_mock.h"
#include "mock/gatt_common.h"
#include "common_mock_all.h"
#include <string.h>
#include "log.h"
#include "src/att/att_receive.h"
#include "src/att/att_common.h"

using namespace bluetooth;
using namespace testing;

class GattServerProfiletest : public testing::Test {
public:
    void AdapterConfigMockSetUp()
    {
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_GATTSERVER_EXCHANGE_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(512), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(7), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_MIN_CONNECTION_INTERVAL, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(6), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_MAX_CONNECTION_INTERVAL, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(84), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_CONNECTION_LATENCY, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(0), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BLE_CONNECTION_SUPERVISION_TIMEOUT, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(252), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_CLASSIC_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(7), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_CLASSIC_CONNECTION_MTU, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(512), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_CLASSIC_CONNECTION_MODE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(0), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_CLASSIC_CONNECTION_FLUSH_TIMEOUT, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(65535), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_CLASSIC_CONNECTION_SECURITY_MODE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(36), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BALANCED_PRIORITY_TIMEOUT, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(500), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BALANCED_PRIORITY_LATENCY, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(0), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BALANCED_PRIORITY_MAX_INTERVAL, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(40), Return(true)));
        EXPECT_CALL(acMock_, GetValue(SECTION_GATT_SERVICE, PROPERTY_BALANCED_PRIORITY_MIN_INTERVAL, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(24), Return(true)));
    }

    void AdapterDeviceConfigMockSetUp()
    {
        std::vector<std::string> tempvec;
        tempvec.clear();

        registerNewMockAdapterDeviceConfig(&adcinstanceMock_);
        EXPECT_CALL(adcinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&adcMock_));
        EXPECT_CALL(adcMock_, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(adcMock_, GetValue(SECTION_HOST, PROPERTY_DEVICE_NAME, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("BluetoothDevice"), Return(true)));
        EXPECT_CALL(adcMock_, GetValue(SECTION_HOST, PROPERTY_BLE_APPEARANCE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(2), Return(true)));
        EXPECT_CALL(adcMock_, GetSubSections(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<std::vector<std::string> &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<1>(tempvec), Return(true)));
        EXPECT_CALL(adcMock_, GetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATTS_START_HANDLE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<3>(3), Return(true)));
        EXPECT_CALL(adcMock_, GetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATTS_END_HANDLE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<3>(3), Return(true)));
        EXPECT_CALL(adcMock_, GetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATT_TRANSPORT, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<3>(3), Return(true)));

        EXPECT_CALL(adcMock_, SetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATTS_START_HANDLE, An<const int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(Return(true)));
        EXPECT_CALL(adcMock_, SetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATTS_END_HANDLE, An<const int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(Return(true)));
        EXPECT_CALL(adcMock_, SetValue(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<const std::string &>(),
            PROPERTY_GATT_TRANSPORT, An<const int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(Return(true)));
    }

    void AdapterDeviceInfoMockSetUp()
    {
        registerNewMockAdapterDeviceInfo(&aiinstanceMock_);
        EXPECT_CALL(aiinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&aiMock_));
        EXPECT_CALL(aiMock_, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_SYSYTEM_ID, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("-"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_PNP_ID, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("-"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_IEEE_INFO, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("-"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_MANUFACTURER_NAME, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("Hikey960"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_MODEL_NUMBER, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("960"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_SERIAL_NUMBER, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("AAAA-S032-4321"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_HARDWARE_REVISION, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("1.0.0"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_FIRMWARE_REVISION, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("1.0.0"), Return(true)));
        EXPECT_CALL(aiMock_, GetValue(SECTION_BLE, PROPERTY_SOFTWARE_REVISION, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("1.0.0"), Return(true)));
    }

    void SetUp()
    {
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(),
            AdapterManager_GetState()).Times(AtLeast(0)).WillRepeatedly(Return(STATE_TURN_ON));
        EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));

        AdapterDeviceConfigMockSetUp();
        AdapterDeviceInfoMockSetUp();
        AdapterConfigMockSetUp();

        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        gattServerService = new GattServerService();
        gattServerService->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(gattServerService));

        // Mock PowerManager & PowerManagerInstance
        registerNewMockPowerManager(&pminstanceMock_);
        EXPECT_CALL(pminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_, _, _)).Times(AtLeast(0)).WillRepeatedly(Return());
    }

    void TearDown()
    {
        if (gattServerService != nullptr) {
            gattServerService->GetContext()->Uninitialize();
            delete gattServerService;
        }
        if (iAdapter != nullptr) {
            delete iAdapter;
            iAdapter = nullptr;
        }
    }

    struct bluetooth::Service ServiceUt()
    {
        uint16_t handle = 0x1F;
        uint16_t starthandle = 0x20;
        uint16_t endHandle = 0x21;
        const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
        struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

        const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
        uint16_t ccc_handle = 0x1F;
        int ccc_properties = 10;
        int ccc_permissions = static_cast<int>(GattPermission::READABLE) | static_cast<int>(GattPermission::WRITABLE);
        const int ccc_value = 10;
        bluetooth::Characteristic ccc(
            ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, (const uint8_t *)&ccc_value, sizeof(uint16_t));

        uint16_t desc_handle = 0x1F;
        const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
        int desc_permissions = static_cast<int>(GattPermission::READABLE) | static_cast<int>(GattPermission::WRITABLE);
        const int desc_value = 10;
        bluetooth::Descriptor desc(
            desc_uuid, desc_handle, desc_permissions, (const uint8_t *)&desc_value, sizeof(uint16_t));
        ccc.descriptors_.push_back(desc);
        service.characteristics_.push_back(ccc);
        return service;
    }

    struct bluetooth::Service UtServices()
    {
        uint16_t handle = 0x1F;
        uint16_t starthandle = 0x20;
        uint16_t endHandle = 0x21;
        const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
        struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

        const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
        uint16_t ccc_handle = 0x1F;
        int ccc_properties = 2;
        int ccc_permissions = static_cast<int>(GattPermission::READABLE);
        const int ccc_value = 10;
        bluetooth::Characteristic ccc(
            ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, (const uint8_t *)&ccc_value, sizeof(uint16_t));

        uint16_t desc_handle = 0x1F;
        const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
        int desc_permissions = static_cast<int>(GattPermission::READABLE);
        const int desc_value = 10;
        bluetooth::Descriptor desc(
            desc_uuid, desc_handle, desc_permissions, (const uint8_t *)&desc_value, sizeof(uint16_t));
        ccc.descriptors_.push_back(desc);
        service.characteristics_.push_back(ccc);
        return service;
    }

protected:
    AdapterManagerMock amMock_;
    GattServerService *gattServerService = NULL;
    IAdapter *iAdapter = NULL;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    StrictMock<AdapterDeviceConfigInstanceMock> adcinstanceMock_;
    StrictMock<AdapterDeviceConfigMock> adcMock_;
    StrictMock<AdapterDeviceInfoInstanceMock> aiinstanceMock_;
    StrictMock<AdapterDeviceInfoMock> aiMock_;
    StrictMock<PowerManagerInstanceMock> pminstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;
};

struct AsyncCallInfromation_ServerProfile {
    bool isCompleted_;
    int state_;

    std::mutex mutex_;
    std::condition_variable condition_;
};

AsyncCallInfromation_ServerProfile asyncCallInfromation_ServerProfile;
class GattServerProfileCallbackImplement : public GattServerProfileCallback {
public:
    void OnExchangeMtuEvent(uint16_t connectHandle, uint16_t rxMtu) override{};
    void OnReadCharacteristicValueEvent(uint16_t connectHandle, uint16_t valueHandle) override{};
    void OnReadUsingCharacteristicUuidEvent(uint16_t connectHandle, uint16_t valueHandle) override{};
    void OnWriteWithoutResponseEvent(
        uint16_t connectHandle, uint16_t valueHandle, GattValue &value, size_t len) override{};
    void OnWriteCharacteristicValueEvent(
        uint16_t connectHandle, uint16_t valueHandle, GattValue &value, size_t len) override{};
    void OnIndicationEvent(uint16_t connectHandle, uint16_t valueHandle, int result) override{};
    void OnReliableWriteEvent(uint16_t connectHandle, uint16_t valueHandle, GattValue &value, size_t len) override{};
    void OnExecuteWriteEvent(uint16_t connectHandle, uint16_t valueHandle, int state) override{};
    void OnDescriptorReadEvent(uint16_t connectHandle, uint16_t valueHandle) override{};
    void OnDescriptorWriteEvent(uint16_t connectHandle, uint16_t valueHandle, GattValue &value, size_t len) override{};

    GattServerProfileCallbackImplement()
    {}
    ~GattServerProfileCallbackImplement()
    {}
};

class IGattServerCallbackImplement : public IGattServerCallback {
public:
    void OnCharacteristicReadRequest(const GattDevice &device, const Characteristic &characteristic) override{};
    void OnCharacteristicWriteRequest(
        const GattDevice &device, const Characteristic &characteristic, bool needRespones) override{};
    void OnDescriptorReadRequest(const GattDevice &device, const Descriptor &descriptor) override{};
    void OnDescriptorWriteRequest(const GattDevice &device, const Descriptor &descriptor) override{};
    void OnNotifyConfirm(const GattDevice &device, const Characteristic &characteristic, int result) override{};
    void OnConnectionStateChanged(const GattDevice &device, int ret, int state) override
    {
        if (state == 1) {
            std::unique_lock<std::mutex> lock_con(asyncCallInfromation_ServerProfile.mutex_);
            asyncCallInfromation_ServerProfile.isCompleted_ = true;
            asyncCallInfromation_ServerProfile.condition_.notify_all();
        } else if (state == GATT_UT_OFFSET_3) {
            std::unique_lock<std::mutex> lock_discon(asyncCallInfromation_ServerProfile.mutex_);
            asyncCallInfromation_ServerProfile.isCompleted_ = true;
            asyncCallInfromation_ServerProfile.condition_.notify_all();
        }
    };
    void OnMtuChanged(const GattDevice &device, int mtu) override{};
    void OnAddService(int ret, const Service &services) override{};
    void OnConnectionParameterChanged(
        const GattDevice &device, int interval, int latency, int timeout, int status) override
    {}
    void OnServiceChanged(const Service &services) override
    {}

    IGattServerCallbackImplement()
    {}
    ~IGattServerCallbackImplement()
    {}
};

class TestServerCallbackImpl : public IGattServerCallback {
public:
    TestServerCallbackImpl()
    {}
    ~TestServerCallbackImpl()
    {}

    void OnConnectionStateChanged(const GattDevice &device, int ret, int state) override
    {
        int called = 1;
    }
};

class TestGattServerProfileCallbackImplement : public GattServerProfileCallback {
public:
    TestGattServerProfileCallbackImplement()
    {}
    ~TestGattServerProfileCallbackImplement()
    {}
};

TEST_F(GattServerProfiletest, GattServerProfileCallback_Test)
{
    TestGattServerProfileCallbackImplement testcallback;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));
    uint16_t connectHandle = 0x01;
    uint16_t rxMtu = 0x01;
    uint16_t valueHandle = 0x01;
    size_t len = 0x01;
    bool flag = false;
    int result = 1;
    int state = 1;
    uint16_t offset = 0x01;
    testcallback.OnExchangeMtuEvent(connectHandle, rxMtu);
    testcallback.OnReadCharacteristicValueEvent(connectHandle, valueHandle);
    testcallback.OnReadUsingCharacteristicUuidEvent(connectHandle, valueHandle);
    testcallback.ReadBlobValueEvent(connectHandle, valueHandle);
    testcallback.OnMultipleCharacteristicValueEvent(connectHandle, sharedPtr, len);
    testcallback.OnWriteWithoutResponseEvent(connectHandle, valueHandle, sharedPtr, len);
    testcallback.OnWriteCharacteristicValueEvent(connectHandle, valueHandle, sharedPtr, len);
    testcallback.OnPrepareWriteValueEvent(connectHandle, valueHandle, offset, sharedPtr, len);
    testcallback.OnExecuteWriteValueEvent(connectHandle, flag);
    testcallback.OnIndicationEvent(connectHandle, valueHandle, result);
    testcallback.OnReliableWriteEvent(connectHandle, valueHandle, sharedPtr, len);
    testcallback.OnExecuteWriteEvent(connectHandle, valueHandle, state);
    testcallback.OnDescriptorReadEvent(connectHandle, valueHandle);
    testcallback.OnDescriptorWriteEvent(connectHandle, valueHandle, sharedPtr, len);
}

TEST_F(GattServerProfiletest, SendReadBlobValueResp_Test)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle = 0x03;
    uint16_t handle = 0x03;
    size_t len = 1;
    int result1 = 1;
    int result2 = 0;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));

    gattserverprofile.SendReadBlobValueResp(connectHandle, handle, sharedPtr, len, result1);
    gattserverprofile.SendReadBlobValueResp(connectHandle, handle, sharedPtr, len, result2);
}

TEST_F(GattServerProfiletest, SendPrepareWriteValueResp_Test)
{
    PrepareWriteParam param;
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    param.connectHandle_ = 0x03;
    param.handle_ = 0x03;
    param.offset_ = 1;
    size_t len = 1;
    int result1 = 1;
    int result2 = 0;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));

    gattserverprofile.SendPrepareWriteValueResp(param, sharedPtr, len, result1);
    gattserverprofile.SendPrepareWriteValueResp(param, sharedPtr, len, result2);
}

TEST_F(GattServerProfiletest, Connect_Test1)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattServerService.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    uint8_t context1 = 1;
    void *context = &context1;
    GapServiceSecurityInfo info;
    info.serviceId = GAP;
    GAPSecCallback(result, info, context);

    sleep(1);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
}

TEST_F(GattServerProfiletest, Connect_Test2)
{
    const std::string addr_1 = "0001";
    bluetooth::RawAddress addr1(addr_1);
    uint8_t type = 0x0;
    uint8_t transport = 0x1;
    bluetooth::GattDevice device1(addr1, type, transport);
    const std::string addr_2 = "0002";
    bluetooth::RawAddress addr2(addr_2);
    bluetooth::GattDevice device2(addr2, type, transport);
    const std::string addr_3 = "0003";
    bluetooth::RawAddress addr3(addr_3);
    bluetooth::GattDevice device3(addr3, type, transport);
    GattConnectionManager::GetInstance().Connect(device1);
    GattConnectionManager::GetInstance().Connect(device2);
    GattConnectionManager::GetInstance().Connect(device3);

    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattServerService.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    uint16_t aclHandle = 1;
    uint8_t status = 1;
    uint8_t reason = 1;
    AttLeDisconnected(aclHandle, status, reason);

    sleep(1);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
}

TEST_F(GattServerProfiletest, Callback_Test)
{
    TestServerCallbackImpl testcallback;
    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x2;
    uint8_t transport = 0x2;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);
    const Characteristic characteristic;
    const Descriptor descriptor;
    const Service services;
    testcallback.OnCharacteristicReadRequest(gattdevice, characteristic);
    testcallback.OnCharacteristicWriteRequest(gattdevice, characteristic, true);
    testcallback.OnDescriptorReadRequest(gattdevice, descriptor);
    testcallback.OnDescriptorWriteRequest(gattdevice, descriptor);
    testcallback.OnNotifyConfirm(gattdevice, characteristic, 0);
    testcallback.OnAddService(0, services);
    testcallback.OnConnectionParameterChanged(gattdevice, 0, 0, 0, 0);
}

TEST_F(GattServerProfiletest, Connect_Test)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattServerService.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(2);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, RespondDescriptorRead)
{
    iAdapter = new ClassicAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x2;
    uint8_t transport = 0x2;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    uint16_t handle = 0x3;
    int permissions = 1;
    size_t length = 2;
    const uint8_t *value = new uint8_t[length];
    struct bluetooth::Descriptor desc(uuid, handle, permissions, value, length);

    int ret = 1;
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);
    sleep(1);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);
    
    int result = gattServerService.RespondDescriptorRead(gattdevice, desc, ret);
    EXPECT_EQ(result, GATT_SUCCESS);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattServerService.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0,
        .mtu = 512,
    };
    uint16_t value1 = 3;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttExchangeMTURequest(&ut, buffer);
    AttFindInformationRequest(&ut, buffer);
    AttFindByTypeValueRequest(&ut, buffer);
    AttReadBlobRequest(&ut, buffer);
    AttReadMultipleRequest(&ut, buffer);
    AttReadByGroupTypeRequest(&ut, buffer);
    AttWriteRequest(&ut, buffer);
    AttWriteCommand(&ut, buffer);
    AttPrepareWriteRequest(&ut, buffer);
    AttExecuteWriteRequest(&ut, buffer);
    AttHandleValueConfirmation(&ut, buffer);
    AttSignedWriteCommand(&ut, buffer);

    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    BufferFree(buffer);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test1)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattServerService.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t value1 = 0x17;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttExchangeMTURequest(&ut, buffer);
    sleep(2);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    BufferFree(buffer);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test1_MTUchanged)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0,
        .mtu = 0x17,
    };
    uint16_t value1 = 0x400;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttExchangeMTURequest(&ut, buffer);
    sleep(1);

    AttSendRespCallback_server(lcid, 0);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    BufferFree(buffer);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test2)
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttFindInformationRequest(&ut, buffer);

    BufferFree(buffer);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerProfiletest, ReceiveData_test3)
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x16,
        .mtu = 512,
    };
    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttFindByTypeValueRequest(&ut, buffer);

    BufferFree(buffer);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerProfiletest, ReceiveData_test4)
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t value1 = 0x1E;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttFindInformationRequest(&ut, buffer);

    BufferFree(buffer);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerProfiletest, ReceiveData_service_test3)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x16,
        .mtu = 512,
    };
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    AttFindByTypeValueRequest(&ut, buffer1);

    uint16_t value2 = 22;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));

    AttFindByTypeValueRequest(&ut, buffer2);

    uint16_t value3 = 33;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttFindByTypeValueRequest(&ut, buffer3);

    uint16_t value4 = 33;
    offset = 0;
    Buffer *buffer4 = BufferMalloc(8 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer4), &offset, (uint8_t *)&value4, sizeof(uint16_t));
    AttFindByTypeValueRequest(&ut, buffer4);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);
    BufferFree(buffer4);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, FindIncludedSerRes_none)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    // value null
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x02;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    sleep(1);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, FindIncludedSerRes_one)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    uint16_t include_handle = 0x1;
    uint16_t include_starthandle = 0x2;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    service.includeServices_.push_back(include_service);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x02;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    sleep(1);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, FindIncludedSerRes_error)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x02;
    uint16_t valuestart1 = 0;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, DiscoverCccRes_none)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x03;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, DiscoverCccRes_one)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    // value null
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x03;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    sleep(1);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, DiscoverCccRes_error)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x03;
    uint16_t valuestart1 = 0;
    uint16_t valueend1 = 0x1f;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(3 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_none)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x2800;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_one_ccc)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x03;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x1820;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    sleep(1);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_one_ccc_unread)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = 0;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = 0;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x03;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x1820;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_one_Desc)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x04;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x1830;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_one_Desc_unread)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = 0;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x04;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x1830;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadUsingCccByUuidRes_error)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x1820;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadValueResponse_UUid16_readable_all_test1)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattServerService.RegisterApplication(callback);

    bluetooth::Service servicesut = GattServerProfiletest::UtServices();
    gattServerService.AddService(appId, servicesut);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    AttReceiveConnectionRsp(0, info, 1, 1, NULL);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value1 = 0x1F;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttReadRequest(&ut, buffer1);

    uint16_t value2 = 0x20;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttReadRequest(&ut, buffer2);

    uint16_t value3 = 0x21;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttReadRequest(&ut, buffer3);

    uint16_t value4 = 0x22;
    offset = 0;
    Buffer *buffer4 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer4), &offset, (uint8_t *)&value4, sizeof(uint16_t));
    AttReadRequest(&ut, buffer4);
    sleep(1);

    AttRecvDisconnectionRsp(0, NULL);
    sleep(1);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);
    BufferFree(buffer4);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReadValueResponse_UUid16_readable_all_test2)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattServerService.RegisterApplication(callback);

    bluetooth::Service servicesut = GattServerProfiletest::UtServices();
    gattServerService.AddService(appId, servicesut);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value5 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer5 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer5), &offset, (uint8_t *)&value5, sizeof(uint16_t));
    AttReadRequest(&ut, buffer5);

    uint16_t value6 = 0x0A;
    offset = 0;
    Buffer *buffer6 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer6), &offset, (uint8_t *)&value6, sizeof(uint16_t));
    AttReadRequest(&ut, buffer6);

    uint16_t value7 = 0x5;
    offset = 0;
    Buffer *buffer7 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer7), &offset, (uint8_t *)&value7, sizeof(uint16_t));
    AttReadRequest(&ut, buffer7);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    BufferFree(buffer5);
    BufferFree(buffer6);
    BufferFree(buffer7);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReadValueResponse_UUid128_readable_ser_ccc)
{
    GattServerService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    int appId = gattClientService.RegisterApplication(callback);

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x19;
    uint16_t endHandle = 0x1E;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0010-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0010-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x1A;
    int ccc_properties = 2;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    const int ccc_value = 10;
    bluetooth::Characteristic ccc(
        ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, (const uint8_t *)&ccc_value, sizeof(uint16_t));

    uint16_t desc_handle = 0x1C;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    const int desc_value = 10;
    bluetooth::Descriptor desc(
        desc_uuid, desc_handle, desc_permissions, (const uint8_t *)&desc_value, sizeof(uint16_t));
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattClientService.AddService(appId, service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t value1 = 0x19;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttReadRequest(&ut, buffer1);
    BufferFree(buffer1);

    uint16_t value2 = 0x1A;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttReadRequest(&ut, buffer2);
    BufferFree(buffer2);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattServerProfiletest, ReadValueResponse_UUid32_readable_ser_ccc)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("01001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("01001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttReadRequest(&ut, buffer1);

    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttReadRequest(&ut, buffer2);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReadValueResponse_UUid16_unreadable_cccd_desc)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    uint16_t ccc_handle = 0x2;
    int ccc_properties = 10;
    int ccc_permissions = 0;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, NULL, 0);

    uint16_t desc_handle = 0x4;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = 0;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value3 = 0x03;
    uint8_t offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttReadRequest(&ut, buffer3);

    uint16_t value4 = 0x04;
    offset = 0;
    Buffer *buffer4 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer4), &offset, (uint8_t *)&value4, sizeof(uint16_t));
    AttReadRequest(&ut, buffer4);

    BufferFree(buffer3);
    BufferFree(buffer4);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_novalue_test5)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value2 = 0x04;
    uint8_t offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttReadRequest(&ut, buffer2);

    uint16_t value3 = 0x03;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttReadRequest(&ut, buffer3);

    BufferFree(buffer2);
    BufferFree(buffer3);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_test8)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttReadByGroupTypeRequest(&ut, buffer1);

    sleep(1);
    uint16_t value2 = 0x04;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttReadByGroupTypeRequest(&ut, buffer2);

    sleep(1);
    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, WriteValueRes_Writeable_all_test1)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattServerService.RegisterApplication(callback);

    bluetooth::Service serviceut = GattServerProfiletest::ServiceUt();
    gattServerService.AddService(appId, serviceut);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value1 = 0x1F;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer1);

    uint16_t value2 = 0x20;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer2);

    uint16_t value3 = 0x21;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer3);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, WriteValueRes_Writeable_all_test2)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattServerService.RegisterApplication(callback);

    bluetooth::Service serviceut = GattServerProfiletest::ServiceUt();
    gattServerService.AddService(appId, serviceut);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value4 = 0x22;
    uint8_t offset = 0;
    Buffer *buffer4 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer4), &offset, (uint8_t *)&value4, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer4), &offset, (uint8_t *)&value4, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer4);

    uint16_t value5 = 0x23;
    offset = 0;
    Buffer *buffer5 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer5), &offset, (uint8_t *)&value5, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer5), &offset, (uint8_t *)&value5, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer5);

    uint16_t value6 = 0x24;
    offset = 0;
    Buffer *buffer6 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer6), &offset, (uint8_t *)&value6, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer6), &offset, (uint8_t *)&value6, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer6);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    BufferFree(buffer4);
    BufferFree(buffer5);
    BufferFree(buffer6);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_int_test9)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = static_cast<int>(GattPermission::WRITABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer1);

    uint16_t value2 = 0x04;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer2);

    uint16_t value3 = 0x02;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer3);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_test10)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = static_cast<int>(GattPermission::READABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer1);

    uint16_t value2 = 0x04;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer2);

    uint16_t value3 = 0x08;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer3);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_int_test10)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = static_cast<int>(GattPermission::WRITABLE);
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattserverprofile.AddService(service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value1 = 0x01;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer1);

    uint16_t value2 = 0x04;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer2);

    uint16_t value3 = 0x02;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer3);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_test11)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 500,
    };

    uint16_t value1 = 0x05;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttPrepareWriteRequest(&ut, buffer);

    BufferFree(buffer);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_test12)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;

    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    gattserverprofile.SendIndication(connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1));

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value2 = 0x01;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttExecuteWriteRequest(&ut, buffer2);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, ReceiveData_test13callback) 
{
    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x2;
    uint8_t transport = 0x2;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 0x1F;
    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int ccc_properties = 1;
    size_t ccc_length = 1;
    int ccc_permissions = 2;
    uint8_t* ccc_value = new uint8_t[ccc_length];
    struct bluetooth::Characteristic characteristic(
        ccc_uuid, handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    bool needConfirm = true;

    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    gattServerService.NotifyClient(gattdevice, characteristic, needConfirm);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test13callback_true) 
{
    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    bluetooth::GattDevice gattdevice(addr, GATT_UT_UINT16_LENGTH, GATT_UT_UINT16_LENGTH, 1);

    uint16_t handle = 0x1C;
    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    size_t ccc_length = 1;
    uint8_t* ccc_value = new uint8_t[ccc_length];
    struct bluetooth::Characteristic characteristic(
        ccc_uuid, 0x1C, 1, GATT_UT_UINT16_LENGTH, ccc_value, ccc_length);

    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    AttReceiveConnectionRsp(0, info, 1, 1, NULL);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value1 = 0x1E;
    uint16_t value2 = 2;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer1);

    gattServerService.NotifyClient(gattdevice, characteristic, true);

    AttSendRespCallback_server(0, 0);

    uint16_t value3 = 0x01;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttHandleValueConfirmation(&ut, buffer2);
    sleep(1);

    AttRecvDisconnectionRsp(0, NULL);
    sleep(1);
    gattServerService.Disable();
    sleep(1);
    BufferFree(buffer1);
    BufferFree(buffer2);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test13callback_false) 
{
    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    bluetooth::GattDevice gattdevice(addr, GATT_UT_UINT16_LENGTH, GATT_UT_UINT16_LENGTH, 1);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    size_t ccc_length = 1;
    uint8_t* ccc_value = new uint8_t[ccc_length];
    struct bluetooth::Characteristic characteristic(
        ccc_uuid, 0x1C, 1, GATT_UT_UINT16_LENGTH, ccc_value, ccc_length);

    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    AttReceiveConnectionRsp(0, info, 1, 1, NULL);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value1 = 0x1E;
    uint16_t value2 = 1;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(2 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteRequest(&ut, buffer1);

    gattServerService.NotifyClient(gattdevice, characteristic, false);

    AttSendRespCallback_server(0, 0);

    uint16_t value3 = 0x01;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    AttHandleValueConfirmation(&ut, buffer2);
    sleep(1);

    AttRecvDisconnectionRsp(0, NULL);
    sleep(1);
    gattServerService.Disable();
    sleep(1);
    BufferFree(buffer1);
    BufferFree(buffer2);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, ReceiveData_test13)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;

    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    gattserverprofile.SendIndication(connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1));

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };

    uint16_t value2 = 0x01;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttHandleValueConfirmation(&ut, buffer2);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, AddService)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);

    uint16_t handle = 0x13;
    uint16_t starthandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0x80");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    gattserverprofile.AddService(service);
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, CheckLegalityOfServiceDefinition)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t handle = 65535;
    uint16_t starthandle = 65535;
    uint16_t endHandle = 65535;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0x80");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    gattserverprofile.CheckLegalityOfServiceDefinition(service);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, RemoveService)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t serviceHandle = 0x13;
    gattserverprofile.RemoveService(serviceHandle);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetServices)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    gattserverprofile.GetServices();
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetService)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t serviceHandle = 0x13;
    gattserverprofile.GetService(serviceHandle);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetCharacteristic)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t serviceHandle = 0x13;
    gattserverprofile.GetCharacteristic(serviceHandle);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetDescriptor)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t serviceHandle = 0x13;
    gattserverprofile.GetDescriptor(serviceHandle);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SetAttributeValue)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    size_t length = 2;
    uint8_t *value = new uint8_t[length];
    GattDatabase::AttributeValue attributevalue;
    attributevalue.SetValue(value, length);

    uint16_t serviceHandle = 0x13;
    gattserverprofile.SetAttributeValue(serviceHandle, attributevalue);
    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
    delete[] value;
}

TEST_F(GattServerProfiletest, SendNotification)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0;
    uint16_t handle1 = 0;

    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    gattserverprofile.SendNotification(connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1));

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;

    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    gattserverprofile.SendNotification(connectHandle2, handle2, sharedPtr2, BufferGetSize(buffer2));

    uint16_t connectHandle3 = 0x13;
    uint16_t handle3 = 0x13;
    auto sharedPtr3 = GattValue(new std::unique_ptr<uint8_t[]>(nullptr));
    gattserverprofile.SendNotification(connectHandle3, handle3, sharedPtr3, 0);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendIndication)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;

    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    gattserverprofile.SendIndication(connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1));

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;

    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    gattserverprofile.SendIndication(connectHandle2, handle2, sharedPtr2, BufferGetSize(buffer2));

    uint16_t connectHandle3 = 0x13;
    uint16_t handle3 = 0x13;
    auto sharedPtr3 = GattValue(new std::unique_ptr<uint8_t[]>(nullptr));
    gattserverprofile.SendIndication(connectHandle3, handle3, sharedPtr3, 0);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendReadCharacteristicValueResp)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;

    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    int ret1 = 0;
    gattserverprofile.SendReadCharacteristicValueResp(
        connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1), ret1);

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;

    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    int ret2 = 1;
    gattserverprofile.SendReadCharacteristicValueResp(
        connectHandle2, handle2, sharedPtr2, BufferGetSize(buffer2), ret2);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendReadUsingCharacteristicValueResp)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;
    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    int ret1 = 0;
    gattserverprofile.SendReadUsingCharacteristicValueResp(
        connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1), ret1);

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;
    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    int ret2 = 1;
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    gattserverprofile.SendReadUsingCharacteristicValueResp(
        connectHandle2, handle2, sharedPtr2, BufferGetSize(buffer2), ret2);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendWriteCharacteristicValueResp)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;
    int ret1 = 0;
    gattserverprofile.SendWriteCharacteristicValueResp(connectHandle1, handle1, ret1);

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;
    int ret2 = 1;
    gattserverprofile.SendWriteCharacteristicValueResp(connectHandle2, handle2, ret2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendReadDescriptorResp)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;
    uint16_t value1 = 0x02;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    int ret1 = 0;
    gattserverprofile.SendReadDescriptorResp(connectHandle1, handle1, sharedPtr1, BufferGetSize(buffer1), ret1);

    uint16_t connectHandle2 = 0x13;
    uint16_t handle2 = 0x13;
    uint16_t value2 = 0x02;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    int ret2 = 1;
    gattserverprofile.SendReadDescriptorResp(connectHandle2, handle2, sharedPtr2, BufferGetSize(buffer2), ret2);

    uint16_t connectHandle3 = 0x16;
    uint16_t handle3 = 0x13;
    uint16_t value3 = 0x02;
    offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    auto sharedPtr3 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer3), BufferGetSize(buffer3));
    int ret3 = 0;
    gattserverprofile.SendReadDescriptorResp(connectHandle3, handle3, sharedPtr3, BufferGetSize(buffer3), ret3);

    BufferFree(buffer1);
    BufferFree(buffer2);
    BufferFree(buffer3);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, SendWriteDescriptorResp)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();

    uint16_t connectHandle1 = 0x13;
    uint16_t handle1 = 0x13;
    int ret1 = 0;
    gattserverprofile.SendWriteDescriptorResp(connectHandle1, handle1, ret1);

    uint16_t connectHandle2 = 0x16;
    uint16_t handle2 = 0x13;
    int ret2 = 1;
    gattserverprofile.SendWriteDescriptorResp(connectHandle2, handle2, ret2);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetAttributeValue_null)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();
    uint16_t handle = 0x13;
    gattserverprofile.GetAttributeValue(handle);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, GetAttributeValue_hasvalue)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);
    service.characteristics_.push_back(ccc);

    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();
    gattserverprofile.AddService(service);
    uint16_t valuehandle = 3;
    gattserverprofile.GetAttributeValue(valuehandle);

    gattserverprofile.Disable();
    delete dispatcher;
    delete callback;
}

TEST_F(GattServerProfiletest, OnWriteWithoutResponseEvent_test)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattServerService.RegisterApplication(callback);

    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, 0x1F, 0x20, 0x21);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("00001820-0000-1000-8000-00805F9B34FB");
    int ccc_permissions = static_cast<int>(GattPermission::READABLE);
    const int ccc_value = 10;
    bluetooth::Characteristic ccc(
        ccc_uuid, 0x1F, GATT_UT_UINT16_LENGTH, ccc_permissions, (const uint8_t *)&ccc_value, sizeof(uint16_t));

    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int desc_permissions = static_cast<int>(GattPermission::WRITABLE);
    const int desc_value = 10;
    bluetooth::Descriptor desc(
        desc_uuid, 0x1F, desc_permissions, (const uint8_t *)&desc_value, sizeof(uint16_t));
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);
    gattServerService.AddService(appId, service);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    AttReceiveConnectionRsp(0, info, 1, 1, NULL);
    sleep(1);

    AttConnectInfo ut;
    ut.aclHandle = 0x03;
    ut.retGattConnectHandle = 0;
    ut.mtu = 512;

    uint16_t value1 = 0x21;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer1);

    uint16_t value2 = 0x22;
    offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    AttWriteCommand(&ut, buffer2);

    AttRecvDisconnectionRsp(0, NULL);
    sleep(1);

    BufferFree(buffer1);
    BufferFree(buffer2);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, RespondCharacteristicWrite)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;
    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0,
        .mtu = 0x17,
    };

    uint16_t value1 = 0x400;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 220;
    uint8_t transport = 0x2;
    int state = 0;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 0;
    gattServerService.RespondCharacteristicWrite(gattdevice, characteristic, ret);
    sleep(1);
    AttSendRespCallback_server(lcid, 0);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    BufferFree(buffer);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, RespondDescriptorWrite)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result_1 = 1;
    uint16_t status = 1;
    void *ctx = NULL;
    AttReceiveConnectionRsp(lcid, info, result_1, status, ctx);
    sleep(1);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0,
        .mtu = 0x17,
    };

    uint16_t value1 = 0x400;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));

    const std::string addr_ = "00:00:00:00:00:00";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 220;
    uint8_t transport = 0x2;
    int state = 0;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(handle, uuid, permissions);

    int ret = 0;
    gattServerService.RespondDescriptorWrite(gattdevice, desc, ret);
    sleep(1);

    AttSendRespCallback_server(lcid, 0);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    BufferFree(buffer);
    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
    delete info;
}

TEST_F(GattServerProfiletest, continue_test)
{
    GattServerService gattServerService;
    GattConnectionManager::GetInstance().StartUp(*gattServerService.GetDispatcher());
    gattServerService.Enable();
    sleep(1);

    int appId = 1;
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x1;
    uint16_t endHandle = 0x5;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001810-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    gattServerService.AddService(appId, service);

    AttConnectInfo ut = {
        .aclHandle = 0x03,
        .retGattConnectHandle = 0x13,
        .mtu = 512,
    };
    uint16_t type1 = 0x01;
    uint16_t valuestart1 = 0x01;
    uint16_t valueend1 = 0x1f;
    uint16_t valueuuid1 = 0x2800;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(4 * sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&type1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valuestart1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueend1, sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&valueuuid1, sizeof(uint16_t));

    AttReadByTypeRequest(&ut, buffer);
    BufferFree(buffer);

    gattServerService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().ShutDown();
}

TEST_F(GattServerProfiletest, ConvertResponseErrorCode)
{
    GattServerProfileCallbackImplement *callback = new GattServerProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    uint16_t maxMtu = 0x1;
    bluetooth::GattServerProfile gattserverprofile(callback, dispatcher, maxMtu);
    gattserverprofile.Enable();
    int errorCode1 = -13;
    int errorCode2 = -12;
    int errorCode3 = -11;
    int errorCode4 = -10;
    int errorCode5 = -9;
    int errorCode6 = -8;
    int errorCode7 = -7;
    int errorCode8 = -6;
    int errorCode9 = -5;
    int errorCode10 = -4;
    int errorCode11 = -3;
    int errorCode12 = -2;
    int errorCode13 = -21;
    int errorCode = 3;
    gattserverprofile.ConvertResponseErrorCode(errorCode1);
    gattserverprofile.ConvertResponseErrorCode(errorCode2);
    gattserverprofile.ConvertResponseErrorCode(errorCode3);
    gattserverprofile.ConvertResponseErrorCode(errorCode4);
    gattserverprofile.ConvertResponseErrorCode(errorCode5);
    gattserverprofile.ConvertResponseErrorCode(errorCode6);
    gattserverprofile.ConvertResponseErrorCode(errorCode7);
    gattserverprofile.ConvertResponseErrorCode(errorCode8);
    gattserverprofile.ConvertResponseErrorCode(errorCode9);
    gattserverprofile.ConvertResponseErrorCode(errorCode10);
    gattserverprofile.ConvertResponseErrorCode(errorCode11);
    gattserverprofile.ConvertResponseErrorCode(errorCode12);
    gattserverprofile.ConvertResponseErrorCode(errorCode13);
    gattserverprofile.ConvertResponseErrorCode(errorCode);
    gattserverprofile.Disable();
}