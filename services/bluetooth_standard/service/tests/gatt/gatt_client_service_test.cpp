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

#include "gatt_client_service.h"
#include "class_creator.h"
#include "gatt_cache.h"
#include "gatt_client_profile.h"
#include "gatt_connection_manager.h"
#include "gatt_service_base.h"
#include "mock/att_send_mock.h"
#include "mock/gatt_common.h"
#include "common_mock_all.h"
#include "src/att/att_connect.h"
#include "log.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "iostream"

using namespace bluetooth;
using namespace testing;

class GattClientServiceTest : public testing::Test {
public:
    void AdapterConfigMockSetUp()
    {
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
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

    void SetUp()
    {
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(),
            AdapterManager_GetState()).Times(AtLeast(0)).WillRepeatedly(Return(STATE_TURN_ON));

        AdapterConfigMockSetUp();

        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        gattClientService = new GattClientService();
        gattClientService->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(gattClientService));

        // Mock PowerManager & PowerManagerInstance
        registerNewMockPowerManager(&pminstanceMock_);
        EXPECT_CALL(pminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_, _, _)).Times(AtLeast(0)).WillRepeatedly(Return());
    }

    void TearDown()
    {
        if (gattClientService != nullptr) {
            gattClientService->GetContext()->Uninitialize();
            delete gattClientService;
        }
    }

protected:
    AdapterManagerMock amMock_;
    GattClientService *gattClientService = NULL;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
    StrictMock<PowerManagerInstanceMock> pminstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;
};

struct AsyncCallInfromation {
    bool isCompleted_;
    int state_;

    std::mutex mutex_;
    std::condition_variable condition_;
};

class GattClientProfileCallbackImplement : public GattClientProfileCallback {
public:
    void OnExchangeMtuEvent(int reqId, uint16_t connectHandle, uint16_t rxMtu, bool status) override{};
    void OnDiscoverAllPrimaryServicesEvent(int reqId, int result, uint16_t connectHandle,
        const std::map<uint16_t, GattCache::Service> &services) override{};
    void OnDiscoverPrimaryServicesByServiceUUIDEvent(int reqId, int result, uint16_t connectHandle,
        const std::map<uint16_t, GattCache::Service> &services) override{};
    void OnFindIncludedServicesEvent(int reqId, int result, uint16_t connectHandle, uint16_t serviceHandle,
        const std::vector<GattCache::IncludeService> &services) override{};
    void OnDiscoverAllCharacteristicOfServiceEvent(int reqId, int result, uint16_t connectHandle,
        uint16_t serviceHandle, const std::map<uint16_t, GattCache::Characteristic> &characteristics) override{};
    void OnDiscoverCharacteristicByUUIDEvent(int reqId, int result, uint16_t connectHandle, uint16_t serviceHandle,
        const std::map<uint16_t, GattCache::Characteristic> &characteristics) override{};
    void OnDiscoverAllCharacteristicDescriptorsEvent(int reqId, int result,
        uint16_t serviceHandle, uint16_t characteristicHandle,
        const std::map<uint16_t, GattCache::Descriptor> &descriptors) override{};
    void OnReadCharacteristicValueEvent(
        int reqId, uint16_t handle, GattValue &value, size_t len, int result) override{};
    void OnWriteCharacteristicValueEvent(int reqId, uint16_t connectHandle, uint16_t handle, int result) override{};
    void OnWriteLongCharacteristicValueEvent(int reqId, uint16_t connectHandle, uint16_t handle, int result) override{};
    void OnWriteLongValueOverageDataEvent(
        int reqId, uint16_t connectHandle, uint16_t handle, GattValue &value, size_t len) override{};
    void OnReadDescriptorValueEvent(
        int reqId, uint16_t handle, GattValue &value, size_t len, int result) override{};
    void OnWriteDescriptorValueEvent(int reqId, uint16_t connectHandle, uint16_t handle, int result) override{};
    void OnCharacteristicNotifyEvent(
        uint16_t connectHandle, uint16_t handle, GattValue &value, size_t len, bool needConfirm) override{};
    void OnReliableWriteCharacteristicValueEvent(
        int reqId, uint16_t handle, GattValue &value, size_t len, int result) override{};
    void OnExecuteWriteValueEvent(int reqId, uint16_t connectHandle, int result) override{};

    GattClientProfileCallbackImplement()
    {}
    ~GattClientProfileCallbackImplement()
    {}
};

AsyncCallInfromation asyncCallInfromation;

class IGattClientCallbackImplement : public IGattClientCallback {
public:
    void OnServicesChanged(const std::vector<Service> &services) override{};
    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {
        std::unique_lock<std::mutex> lock_readccc(asyncCallInfromation.mutex_);
        asyncCallInfromation.isCompleted_ = true;
        asyncCallInfromation.condition_.notify_all();
    };
    void OnCharacteristicWrite(int ret, const Characteristic &characteristic) override
    {
        std::unique_lock<std::mutex> lock_writeccc(asyncCallInfromation.mutex_);
        asyncCallInfromation.isCompleted_ = true;
        asyncCallInfromation.condition_.notify_all();
    };
    void OnCharacteristicChanged(const Characteristic &characteristic) override{};
    void OnDescriptorRead(int ret, const Descriptor &descriptor) override
    {
        std::unique_lock<std::mutex> lock_readdesc(asyncCallInfromation.mutex_);
        asyncCallInfromation.isCompleted_ = true;
        asyncCallInfromation.condition_.notify_all();
    };
    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override
    {
        std::unique_lock<std::mutex> lock_writedesc(asyncCallInfromation.mutex_);
        asyncCallInfromation.isCompleted_ = true;
        asyncCallInfromation.condition_.notify_all();
    };
    void OnConnectionStateChanged(int state, int newState) override
    {
        if (newState == 1) {
            std::unique_lock<std::mutex> lock_con(asyncCallInfromation.mutex_);
            asyncCallInfromation.isCompleted_ = true;
            asyncCallInfromation.condition_.notify_all();
        } else if (newState == GATT_UT_OFFSET_3) {
            std::unique_lock<std::mutex> lock_discon(asyncCallInfromation.mutex_);
            asyncCallInfromation.isCompleted_ = true;
            asyncCallInfromation.condition_.notify_all();
        }
    };
    void OnMtuChanged(int state, int mtu) override
    {
        std::unique_lock<std::mutex> lock_mtu(asyncCallInfromation.mutex_);
        asyncCallInfromation.isCompleted_ = true;
        asyncCallInfromation.condition_.notify_all();
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {}

    void OnServicesDiscovered(int status) override
    {}

    IGattClientCallbackImplement()
    {}
    ~IGattClientCallbackImplement()
    {}
};

TEST_F(GattClientServiceTest, Connect_and_Disconnect)
{
    int appId = -1;
    GattClientService gattClientService;
    int result_connect = gattClientService.Connect(appId);
    int result_disconnect = gattClientService.Disconnect(appId);

    EXPECT_EQ(result_connect, bluetooth::GattStatus::REQUEST_NOT_SUPPORT);
    EXPECT_EQ(result_disconnect, bluetooth::GattStatus::REQUEST_NOT_SUPPORT);
}

TEST_F(GattClientServiceTest, Connect_and_Disconnect_impl)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x2;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    int result_2 = gattClientService.Connect(appId);
    EXPECT_EQ(result_2, GATT_SUCCESS);
    sleep(1);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result = 1;
    uint16_t status = 1;
    void *ctx = NULL;
    AttReceiveConnectionRsp(lcid, info, result, status, ctx);
    sleep(1);

    int result_1 = gattClientService.Disconnect(appId);
    EXPECT_EQ(result_1, GATT_SUCCESS);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, GetConnectDevices_normal_1)
{
    std::list<RawAddress> result;
    GattClientService gattClientService;
    std::list<RawAddress> list = gattClientService.GetConnectDevices();

    EXPECT_EQ(result, list);
}

TEST_F(GattClientServiceTest, GetConnectState)
{
    GattClientService gattClientService;
    int result = gattClientService.GetConnectState();
    EXPECT_EQ(0, result);
}

TEST_F(GattClientServiceTest, GetMaxConnectNum)
{
    uint16_t a = 7;
    uint16_t b = 7;
    std::pair<uint16_t, uint16_t> pair(a, b);
    GattClientService gattClientService;
    int result = gattClientService.GetMaxConnectNum();
    EXPECT_EQ(14, result);
}

TEST_F(GattClientServiceTest, RegisterApplication_test1)
{
    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:03";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    GattClientService gattClientService;
    int result = gattClientService.RegisterApplication(callback, addr_, transport);

    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, RegisterApplication_test3)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:05";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    gattClientService.RegisterApplication(callback, addr_, transport);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, DeregisterApplication_test1)
{
    GattClientService gattClientService;
    int appId = 10;
    int result = gattClientService.DeregisterApplication(appId);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, DeregisterApplication_test2)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:07";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    int result = gattClientService.DeregisterApplication(appId);
    EXPECT_EQ(GATT_SUCCESS, result);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, DiscoveryServices_test1)
{
    int appId = 1;
    GattClientService gattClientService;
    int result = gattClientService.DiscoveryServices(appId);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, DiscoveryServices_test2)
{
    GattClientService service1;
    service1.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr1(addr);
    uint8_t transport = 0x1;

    int appId = service1.RegisterApplication(callback, addr1, transport);

    int result = service1.DiscoveryServices(appId);
    EXPECT_EQ(GATT_SUCCESS, result);

    service1.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, ReadCharacteristic_test1)
{
    int appId = 123;
    uint16_t handle = 10;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    GattClientService gattClientService;
    int result = gattClientService.ReadCharacteristic(appId, characteristic);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, ReadCharacteristic_test2)
{
    GattClientService service2;
    service2.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr2(addr);
    uint8_t transport = 0x1;

    int appId = service2.RegisterApplication(callback, addr2, transport);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid_;
    bluetooth::Uuid uuid_ccc(uuid_);
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid_ccc, handle, properties);

    int result = service2.ReadCharacteristic(appId, characteristic);
    EXPECT_EQ(GATT_SUCCESS, result);
    sleep(1);

    service2.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, ReadCharacteristicByUuid_test)
{
    GattClientService service3;
    service3.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr3(addr);
    uint8_t transport = 0x1;

    int appId = service3.RegisterApplication(callback, addr3, transport);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid_ = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    bluetooth::Uuid uuid(uuid_);

    int result = service3.ReadCharacteristicByUuid(appId, uuid);
    EXPECT_EQ(GATT_SUCCESS, result);
    sleep(1);

    service3.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, WriteCharacteristic_test1)
{
    int appId = 123;
    uint16_t handle = 10;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    bool withoutRespond = true;
    GattClientService gattClientService;
    int result = gattClientService.WriteCharacteristic(appId, characteristic, withoutRespond);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, WriteCharacteristic_test2)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    int appId = 123;
    uint16_t handle = 10;
    const bluetooth::Uuid uuid_;
    bluetooth::Uuid uuid(uuid_);
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    bool withoutRespond = true;

    int result = gattClientService.WriteCharacteristic(appId, characteristic, withoutRespond);
    EXPECT_EQ(INVALID_PARAMETER, result);

    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(0);
    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, WriteCharacteristic_test3)
{
    GattClientService service4;
    service4.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr4(addr);
    uint8_t transport = 0x1;

    int appId = service4.RegisterApplication(callback, addr4, transport);

    uint16_t handle = 0;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid;
    uuid.ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int properties = 1;
    int permissions = 0;
    uint8_t value[2] = "1";
    size_t length = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties, permissions, value, length);

    bool withoutRespond = false;;
    int result = service4.WriteCharacteristic(appId, characteristic, withoutRespond);
    EXPECT_EQ(GATT_SUCCESS, result);
    sleep(1);

    service4.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, ReadDescriptor_test1)
{
    int appId = 123;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    int permissions = 1;
    struct bluetooth::Descriptor desc(uuid, permissions);
    GattClientService gattClientService;
    int result = gattClientService.ReadDescriptor(appId, desc);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, ReadDescriptor_test2)
{
    GattClientService service5;
    service5.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr5(addr);
    uint8_t transport = 0x1;

    int appId = service5.RegisterApplication(callback, addr5, transport);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    int properties = 1;
    const bluetooth::Uuid uuid;
    uuid.ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    int permissions = 1;
    struct bluetooth::Descriptor desc(uuid, permissions);

    int result = service5.ReadDescriptor(appId, desc);
    EXPECT_EQ(GATT_SUCCESS, result);
    sleep(1);

    service5.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, WriteDescriptor_test1)
{
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00005F9B34FB");
    const int permissions = 0;
    struct bluetooth::Descriptor descriptor(uuid, permissions);
    GattClientService gattClientService;
    int appId = 1;
    int result = gattClientService.WriteDescriptor(appId, descriptor);

    EXPECT_EQ(bluetooth::GattStatus::REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, WriteDescriptor_test2)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:06";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    uint16_t handle = 10;
    struct bluetooth::Service Service(handle);
    const bluetooth::Uuid uuid_;
    bluetooth::Uuid uuid_desc(uuid_);
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid_desc, handle, properties);

    const bluetooth::Uuid uuidDes;
    uuidDes.ConvertFromString("00001830-0000-1000-8000-00805F9B34FB");
    const int permissions = 0;
    uint8_t value[2] = "1";
    size_t length = 1;
    struct bluetooth::Descriptor descriptor(uuidDes, handle, permissions, value, length);

    int result = gattClientService.WriteDescriptor(appId, descriptor);
    EXPECT_EQ(bluetooth::GattStatus::GATT_SUCCESS, result);
    sleep(1);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, WriteDescriptor_test3)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    const bluetooth::Uuid uuid_;
    bluetooth::Uuid uuid(uuid_);
    const int permissions = 10;
    struct bluetooth::Descriptor descriptor(uuid, permissions);
    int appId = 0;
    int result = gattClientService.WriteDescriptor(appId, descriptor);

    EXPECT_EQ(bluetooth::GattStatus::INVALID_PARAMETER, result);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, RequestExchangeMtu_test1)
{
    int appId = 0;
    int mtu = 0;
    GattClientService gattClientService;
    int result = gattClientService.RequestExchangeMtu(appId, mtu);

    EXPECT_EQ(bluetooth::GattStatus::REQUEST_NOT_SUPPORT, result);
}

TEST_F(GattClientServiceTest, GetAllDevice)
{
    GattClientService gattClientService;
    std::vector<GattDevice> vector = gattClientService.GetAllDevice();
}

TEST_F(GattClientServiceTest, SignedWriteCharacteristic_test1)
{
    uint16_t handle = 1;
    struct bluetooth::Service Service(handle);
    int properties = 1;
    int permissions = 1;
    size_t length = 2;
    const uint8_t *value = new uint8_t[length];

    struct bluetooth::Characteristic characteristic(
        Uuid::ConvertFrom16Bits(0x2A24), handle, properties, permissions, value, length);
    GattClientService service6;
    service6.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr6(addr);
    uint8_t transport = 0x1;

    int appId = service6.RegisterApplication(callback, addr6, transport);

    int result1 = service6.SignedWriteCharacteristic(appId, characteristic);
    EXPECT_EQ(GATT_SUCCESS, result1);
    usleep(500);

    service6.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, SignedWriteCharacteristic_test2)
{
    int appId = 1;
    uint16_t handle = 1;
    struct bluetooth::Service Service(handle);
    int properties = 1;
    struct bluetooth::Characteristic characteristic(Uuid::ConvertFrom16Bits(0x2A24), handle, properties);
    GattClientService gattClientService;
    int result2 = gattClientService.SignedWriteCharacteristic(appId, characteristic);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result2);
}

TEST_F(GattClientServiceTest, SignedWriteCharacteristic_test3)
{
    uint16_t handle = 1;
    struct bluetooth::Service Service(handle);
    int properties = 1;

    struct bluetooth::Characteristic characteristic(Uuid::ConvertFrom16Bits(0x2A24), handle, properties);
    GattClientService service7;
    service7.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:01";
    bluetooth::RawAddress addr7(addr);
    uint8_t transport = 0x1;

    int appId = service7.RegisterApplication(callback, addr7, transport);

    int result3 = service7.SignedWriteCharacteristic(appId, characteristic);
    EXPECT_EQ(INVALID_PARAMETER, result3);
    usleep(500);

    service7.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, RequestConnectionPriority_test1)
{
    int connPriority = 1;
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:02";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    int result1 = gattClientService.RequestConnectionPriority(appId, connPriority);
    EXPECT_EQ(GATT_SUCCESS, result1);
    usleep(500);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, RequestConnectionPriority_test2)
{
    int appId = 1;
    int connPriority = 1;
    GattClientService gattClientService;
    int result2 = gattClientService.RequestConnectionPriority(appId, connPriority);
    EXPECT_EQ(REQUEST_NOT_SUPPORT, result2);
}

TEST_F(GattClientServiceTest, RequestConnectionPriority_test3)
{
    int connPriority = 3;
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:12";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    int result3 = gattClientService.RequestConnectionPriority(appId, connPriority);
    EXPECT_EQ(INVALID_PARAMETER, result3);
    usleep(500);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, GetServices_test1)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement callback;
    const std::string addr = "00:00:00:03";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x1;

    int appId = gattClientService.RegisterApplication(callback, addr_, transport);

    gattClientService.GetServices(appId);
    usleep(500);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(GattClientServiceTest, GetServices_test2)
{
    int appId = 3;
    GattClientService gattClientService;
    gattClientService.GetServices(appId);
}