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

#include "gatt_server_service.h"
#include "gatt_connection_manager.h"
#include "gatt/dis/device_information_service.h"
#include "gatt/gatts/generic_attribute_service.h"
#include "mock/att_send_mock.h"
#include "mock/gatt_common.h"
#include "common_mock_all.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace bluetooth;
using namespace testing;

class GattServerServiceTest : public testing::Test {
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
        tempvec.push_back("07:00:00:00:00:00");

        registerNewMockAdapterDeviceConfig(&adcinstanceMock_);
        EXPECT_CALL(adcinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&adcMock_));
        EXPECT_CALL(adcMock_, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(adcMock_, Save()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(adcMock_, GetValue(SECTION_HOST, PROPERTY_DEVICE_NAME, An<std::string &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>("BluetoothDevice"), Return(true)));
        EXPECT_CALL(adcMock_, GetValue(SECTION_HOST, PROPERTY_BLE_APPEARANCE, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<GATT_UT_UINT16_LENGTH>(2), Return(true)));
        EXPECT_CALL(adcMock_, GetSubSections(SECTION_GENERIC_ATTRIBUTE_SERVICE, An<std::vector<std::string> &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<1>(tempvec), Return(true)));
        EXPECT_CALL(adcMock_, RemoveSection(SECTION_GENERIC_ATTRIBUTE_SERVICE,  "07:00:00:00:00:00"))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(true));
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

struct AsyncCallInfromationServer_ {
    bool isCompleted_;
    int state_;

    std::mutex mutex_;
    std::condition_variable condition_;
};
AsyncCallInfromationServer_ asyncCallInfromationServer_;
class IGattServerCallbackImplement : public IGattServerCallback {
public:
    void OnCharacteristicReadRequest(const GattDevice &device, const Characteristic &characteristic) override{};
    void OnCharacteristicWriteRequest(
        const GattDevice &device, const Characteristic &characteristic, bool needRespones) override{};
    void OnDescriptorReadRequest(const GattDevice &device, const Descriptor &descriptor) override{};
    void OnDescriptorWriteRequest(const GattDevice &device, const Descriptor &descriptor) override{};
    void OnNotifyConfirm(const GattDevice &device, const Characteristic &characteristic, int result) override{};
    void OnConnectionStateChanged(const GattDevice &device, int ret, int state) override{};
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

TEST_F(GattServerServiceTest, RegisterApplication_test1)
{
    GattServerService gattserverservice;
    IGattServerCallbackImplement callback;
    int result = gattserverservice.RegisterApplication(callback);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, generic_attribute_service1)
{
    GattServerService gattserverservice;
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GenericAttributeService genericAttributeService(gattserverservice, *dispatcher);
    int result = genericAttributeService.RegisterSDP();
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, generic_attribute_service2)
{
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattserverservice.RegisterApplication(callback);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);

    gattserverservice.AddService(appId, Service);
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GenericAttributeService genericAttributeService(gattserverservice, *dispatcher);
    genericAttributeService.RegisterService();
    int result = genericAttributeService.RegisterSDP();
    EXPECT_EQ(result, GATT_SUCCESS);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, StoreNotifyInformation_test1)
{
    GattServerService gattserverservice;
    bluetooth::DeviceInformationService service(gattserverservice);
    int result = service.RegisterSDP();
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, StoreNotifyInformation_test2)
{
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int appId = gattserverservice.RegisterApplication(callback);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);

    gattserverservice.AddService(appId, Service);
    bluetooth::DeviceInformationService deviceService(gattserverservice);
    deviceService.RegisterService();
    int result = deviceService.RegisterSDP();
    EXPECT_EQ(result, GATT_SUCCESS);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RegisterApplication_test3)
{
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    int result = gattserverservice.RegisterApplication(callback);
    EXPECT_GE(result, 0);

    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, DeregisterApplication_test1)
{
    GattServerService gattserverservice;
    int appId = 1;
    int result = gattserverservice.DeregisterApplication(appId);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, DeregisterApplication_test2)
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    int appId = gattServerService.RegisterApplication(callback);

    int result = gattServerService.DeregisterApplication(appId);
    EXPECT_EQ(GATT_SUCCESS, result);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, AddService_test1)
{
    int appId = 123;
    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    GattServerService gattserverservice;
    int result = gattserverservice.AddService(appId, Service);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, AddService_test2)  //
{
    GattServerService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    int appId = gattClientService.RegisterApplication(callback);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    GattServerService gattserverservice;
    gattserverservice.AddService(appId, Service);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RemoveService_test1)
{
    int appId = 123;
    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    GattServerService gattserverservice;
    int result = gattserverservice.RemoveService(appId, Service);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, RemoveService_test2)
{
    int appId = 123;
    uint16_t handle = (uint16_t)65536;
    struct bluetooth::Service Service(handle);
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RemoveService(appId, Service);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RemoveService_test3)  // RemoveService=============GATT_SUCCESS
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    int appId = gattServerService.RegisterApplication(callback);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    int result = gattServerService.RemoveService(appId, Service);
    EXPECT_EQ(GATT_SUCCESS, result);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, ClearServices_test1)
{
    int appId = 123;
    GattServerService gattserverservice;
    int result = gattserverservice.ClearServices(appId);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, ClearServices_test3)  // ClearServices==================GATT_SUCCESS
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    int appId = gattServerService.RegisterApplication(callback);

    int result = gattServerService.ClearServices(appId);
    EXPECT_EQ(GATT_SUCCESS, result);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, NotifyClient_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    bool needConfirm = false;
    GattServerService gattserverservice;
    int result = gattserverservice.NotifyClient(gattdevice, characteristic, needConfirm);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, NotifyClient_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = (uint16_t)65536;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    bool needConfirm = false;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.NotifyClient(gattdevice, characteristic, needConfirm);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, NotifyClient_test3)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 0x3;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    bool needConfirm = false;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.NotifyClient(gattdevice, characteristic, needConfirm);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, NotifyClient_test4)  // NotifyClient==================GATT_SUCCESS
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    gattServerService.RegisterApplication(callback);

    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 0x3;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    int permissions = 1;
    size_t length = 2;
    const uint8_t *value = new uint8_t[length];
    struct bluetooth::Characteristic characteristic(uuid, handle, properties, permissions, value, length);

    bool needConfirm = false;
    int result = gattServerService.NotifyClient(gattdevice, characteristic, needConfirm);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondCharacteristicRead_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 1;
    GattServerService gattserverservice;
    int result = gattserverservice.RespondCharacteristicRead(gattdevice, characteristic, ret);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
    int result2 = gattserverservice.RespondCharacteristicReadByUuid(gattdevice, characteristic, ret);
    EXPECT_EQ(result2, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, RespondCharacteristicRead_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = (uint16_t)65536;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 1;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RespondCharacteristicRead(gattdevice, characteristic, ret);
    EXPECT_EQ(result, INVALID_PARAMETER);
    int result2 = gattserverservice.RespondCharacteristicReadByUuid(gattdevice, characteristic, ret);
    EXPECT_EQ(result2, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondCharacteristicRead_test3)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 0;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RespondCharacteristicRead(gattdevice, characteristic, ret);
    EXPECT_EQ(result, INVALID_PARAMETER);
    int result2 = gattserverservice.RespondCharacteristicReadByUuid(gattdevice, characteristic, ret);
    EXPECT_EQ(result2, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondCharacteristicRead_test4)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 0x3;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    int permissions = 1;
    size_t length = 2;
    const uint8_t *value = new uint8_t[length];
    struct bluetooth::Characteristic characteristic(uuid, handle, properties, permissions, value, length);
    struct bluetooth::Characteristic characteristic2(uuid, handle, properties, permissions, value, length);

    int ret = 0;
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    gattServerService.RegisterApplication(callback);

    int result = gattServerService.RespondCharacteristicRead(gattdevice, characteristic, ret);
    EXPECT_EQ(result, GATT_SUCCESS);
    int result2 = gattServerService.RespondCharacteristicReadByUuid(gattdevice, characteristic2, ret);
    EXPECT_EQ(result2, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondCharacteristicWrite_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 1;
    GattServerService gattserverservice;
    int result = gattserverservice.RespondCharacteristicWrite(gattdevice, characteristic, ret);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, RespondCharacteristicWrite_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);

    int ret = 1;
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    gattServerService.RegisterApplication(callback);
    int result = gattServerService.RespondCharacteristicWrite(gattdevice, characteristic, ret);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondDescriptorRead_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(uuid, permissions);

    int ret = 1;
    GattServerService gattserverservice;
    int result = gattserverservice.RespondDescriptorRead(gattdevice, desc, ret);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, RespondDescriptorRead_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = (uint16_t)65536;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(handle, uuid, permissions);

    int ret = 1;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RespondDescriptorRead(gattdevice, desc, ret);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondDescriptorRead_test3)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 0x0001;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(handle, uuid, permissions);

    int ret = 0;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RespondDescriptorRead(gattdevice, desc, ret);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondDescriptorRead_test4)  // RespondDescriptorRead==================GATT_SUCCESS
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
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
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    gattServerService.RegisterApplication(callback);
    int result = gattServerService.RespondDescriptorRead(gattdevice, desc, ret);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondDescriptorWrite_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(uuid, permissions);

    int ret = 1;
    GattServerService gattserverservice;
    int result = gattserverservice.RespondDescriptorWrite(gattdevice, desc, ret);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, RespondDescriptorWrite_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = (uint16_t)65536;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(handle, uuid, permissions);

    int ret = 1;
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);
    int result = gattserverservice.RespondDescriptorWrite(gattdevice, desc, ret);
    EXPECT_EQ(result, INVALID_PARAMETER);
    gattserverservice.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RespondDescriptorWrite_test3)  // RespondDescriptorWrite==================GATT_SUCCESS
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    uint16_t handle = 10;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(handle, uuid, permissions);

    int ret = 1;
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;

    gattServerService.RegisterApplication(callback);
    int result = gattServerService.RespondDescriptorWrite(gattdevice, desc, ret);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, CancelConnection_test1)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    GattServerService gattserverservice;
    int result = gattserverservice.CancelConnection(gattdevice);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, GetContext)
{
    GattServerService gattserverservice;
    gattserverservice.GetContext();
}

TEST_F(GattServerServiceTest, GetConnectDevices)
{
    GattServerService gattserverservice;
    gattserverservice.GetConnectDevices();
}

TEST_F(GattServerServiceTest, GetConnectState)
{
    GattServerService gattserverservice;
    gattserverservice.Enable();
    sleep(1);

    gattserverservice.GetConnectState();
}

TEST_F(GattServerServiceTest, GetMaxConnectNum)
{
    GattServerService gattserverservice;
    gattserverservice.GetMaxConnectNum();
}

TEST_F(GattServerServiceTest, Enable_mutitimes)
{
    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    gattServerService.Enable();
    sleep(1);

    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, CancelConnection_test2)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    int result = gattServerService.CancelConnection(gattdevice);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}

TEST_F(GattServerServiceTest, RegisterApplicationSync_REQUEST_NOT_SUPPORT)
{
    GattServerService gattserverservice;
    IGattServerCallbackImplement callback;
    int result = gattserverservice.RegisterApplicationSync(callback);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, DeregisterApplicationSync_REQUEST_NOT_SUPPORT)
{
    GattServerService gattserverservice;
    int appId = 1;
    int result = gattserverservice.DeregisterApplicationSync(appId);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(GattServerServiceTest, GetDatabaseHash)
{
    GattServerService gattserverservice;
    gattserverservice.GetDatabaseHash();
}

TEST_F(GattServerServiceTest, CancelConnection_test3)
{
    const std::string addr_ = "00:00:00:00:00:02";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x2;
    int state = 1;
    bluetooth::GattDevice gattdevice(addr, type, transport, state);

    GattServerService gattServerService;
    gattServerService.Enable();
    sleep(1);

    IGattServerCallbackImplement callback;
    gattServerService.RegisterApplication(callback);

    int result = gattServerService.CancelConnection(gattdevice);
    EXPECT_EQ(result, GATT_SUCCESS);
    gattServerService.Disable();
    sleep(1);
}