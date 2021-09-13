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

#include "att/att_common.h"
#include "bt_def.h"
#include "dispatcher.h"
#include "gatt_cache.h"
#include "gatt_client_profile.h"
#include "gatt_client_service.h"
#include "gatt_connection_manager.h"
#include "gatt_defines.h"
#include "gatt_service_base.h"
#include "mock/att_receive_mock.h"
#include "mock/ble_adapter_mock.h"
#include "mock/att_send_mock.h"
#include "mock/gatt_common.h"
#include "common_mock_all.h"
#include "src/att/att_connect.h"
#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace bluetooth;
using namespace testing;

class Gatt_Client_Profile_Test : public testing::Test {
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

    void SendResponse_Discover(const AttConnectInfo *connect)
    {
        uint16_t valuestart = 1;
        uint16_t valueend = 6;
        uint16_t value = 0;
        uint8_t offset = 0;
        Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
        AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value, sizeof(uint16_t));
        AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&valuestart, sizeof(uint16_t));
        AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&valueend, sizeof(uint16_t));

        // find include service response
        AttSendRespCallback(0, 0);
        AttReadByTypeResponse_2(connect, NULL);
        usleep(100);
        AttSendRespCallback(0, 0);
        AttErrorResponse_3(connect, buffer1);
        sleep(1);

        // discover ccc response
        AttSendRespCallback(0, 0);
        AttReadByTypeResponse_1(connect, buffer1);
        usleep(100);
        AttSendRespCallback(0, 0);
        AttErrorResponse_3(connect, buffer1);
        sleep(1);

        AttSendRespCallback(0, 0);
        AttReadByGroupTypeResponse(connect, NULL);
        sleep(1);

        // discover desc response
        AttSendRespCallback(0, 0);
        usleep(100);
        AttFindInformationResponse(connect, buffer1);
        sleep(1);

        BufferFree(buffer1);
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
    IAdapter *iAdapter = NULL;
};

AsyncCallInfromation_2 asyncCallInfromation_2;

class IGattClientCallbackImplement_2 : public IGattClientCallback {
public:
    void OnServicesChanged(const std::vector<Service> &services) override
    {}
    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {}
    void OnCharacteristicWrite(int ret, const Characteristic &characteristic) override
    {}
    void OnCharacteristicChanged(const Characteristic &characteristic) override
    {}
    void OnDescriptorRead(int ret, const Descriptor &descriptor) override
    {}
    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override
    {}
    void OnConnectionStateChanged(int state, int newState) override
    {}
    void OnMtuChanged(int state, int mtu) override
    {}
    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {}
    void OnServicesDiscovered(int status) override
    {}

    IGattClientCallbackImplement_2()
    {}
    ~IGattClientCallbackImplement_2()
    {}
};
class GattClientProfileCallbackImplement : public GattClientProfileCallback {
public:
    void OnExchangeMtuEvent(int reqId, uint16_t connectHandle, uint16_t rxMtu, bool status) override{};
    void OnDiscoverAllPrimaryServicesEvent(
        int reqId, int result, uint16_t connectHandle,
        const std::map<uint16_t, GattCache::Service> &services) override{};
    void OnDiscoverPrimaryServicesByServiceUUIDEvent(
        int reqId, int result, uint16_t connectHandle,
        const std::map<uint16_t, GattCache::Service> &services) override{};
    void OnFindIncludedServicesEvent(int reqId, int result, uint16_t connectHandle, uint16_t serviceHandle,
        const std::vector<GattCache::IncludeService> &services) override{};
    void OnDiscoverAllCharacteristicOfServiceEvent(int reqId, int result, uint16_t connectHandle,
        uint16_t serviceHandle, const std::map<uint16_t, GattCache::Characteristic> &characteristics) override{};
    void OnDiscoverCharacteristicByUUIDEvent(int reqId, int result, uint16_t connectHandle,
        uint16_t serviceHandle, const std::map<uint16_t, GattCache::Characteristic> &characteristics) override{};
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

class TestGattClientProfileCallbackImplement : public GattClientProfileCallback {
public:
    TestGattClientProfileCallbackImplement()
    {}
    ~TestGattClientProfileCallbackImplement()
    {}
};

TEST_F(Gatt_Client_Profile_Test, GattClientProfileCallback_test)
{
    TestGattClientProfileCallbackImplement testcallback;
    Buffer *buffer = BufferMalloc(8 * sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));
    uint16_t connectHandle = 0x01;
    uint16_t rxMtu = 0x01;
    bool status = true;
    uint16_t handle = 0x01;
    uint16_t serviceHandle = 0x01;
    uint16_t characteristicHandle = 0x01;
    int ret = 1;
    int result = 1;
    size_t len = 1;
    bool needConfirm = false;
    int reqId = 1;
    std::map<uint16_t, GattCache::Service> services;
    std::vector<GattCache::IncludeService> IncludeService;
    std::map<uint16_t, GattCache::Characteristic> characteristics;
    std::map<uint16_t, GattCache::Descriptor> descriptors;
    testcallback.OnExchangeMtuEvent(reqId, connectHandle, rxMtu, status);
    testcallback.OnDiscoverAllPrimaryServicesEvent(reqId, ret, connectHandle, services);
    testcallback.OnDiscoverPrimaryServicesByServiceUUIDEvent(reqId, ret, connectHandle, services);
    testcallback.OnFindIncludedServicesEvent(reqId, ret, connectHandle, serviceHandle, IncludeService);
    testcallback.OnDiscoverAllCharacteristicOfServiceEvent(reqId, ret, connectHandle, serviceHandle, characteristics);
    testcallback.OnDiscoverCharacteristicByUUIDEvent(reqId, ret, connectHandle, serviceHandle, characteristics);
    testcallback.OnDiscoverAllCharacteristicDescriptorsEvent(reqId, ret, 
                                                             serviceHandle, characteristicHandle, descriptors);
    testcallback.OnWriteCharacteristicValueEvent(reqId, connectHandle, handle, ret);
    testcallback.OnWriteLongCharacteristicValueEvent(reqId, connectHandle, handle, ret);
    testcallback.OnExecuteWriteValueEvent(reqId, connectHandle, result);
    testcallback.OnReadCharacteristicValueEvent(reqId, handle, sharedPtr, len, result);
    testcallback.OnWriteLongValueOverageDataEvent(reqId, connectHandle, handle, sharedPtr, len);
    testcallback.OnReadDescriptorValueEvent(reqId, handle, sharedPtr, len, result);
    testcallback.OnWriteDescriptorValueEvent(reqId, connectHandle, handle, result);
    testcallback.OnCharacteristicNotifyEvent(connectHandle, handle, sharedPtr, len, needConfirm);
    testcallback.OnReliableWriteCharacteristicValueEvent(reqId, handle, sharedPtr, len, result);
}

TEST_F(Gatt_Client_Profile_Test, OnReadCharacteristicValueEvent_Test)
{
    GattClientService gattClientService1;
    gattClientService1.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr1 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr1);
    uint8_t transport = 0x2;

    int appId1 = gattClientService1.RegisterSharedApplication(callback, addr_, transport);

    gattClientService1.Connect(appId1);
    sleep(1);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, 1, 1, ctx);
    sleep(1);

    uint8_t offset = 0;
    uint16_t handle = 10;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&lcid, sizeof(uint16_t));
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1001-8000-00805F9B34FC");
    int properties = 1;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    gattClientService1.ReadCharacteristic(appId1, characteristic);
    usleep(100);

    AttSendRespCallback(lcid, 0);
    usleep(100);
    AttReadResponse1(connect, buffer1);
    usleep(100);

    BufferFree(buffer1);
    gattClientService1.Disconnect(appId1);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService1.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnReadCharacteristicValueEvent_Test_error)
{
    GattClientService gattClientService2;
    gattClientService2.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr2 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr2);
    uint8_t transport = 0x2;

    int appId2 = gattClientService2.RegisterSharedApplication(callback, addr_, transport);

    gattClientService2.Connect(appId2);
    sleep(1);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, 1, status, ctx);
    sleep(1);

    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&lcid, sizeof(uint16_t));
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B340C");
    int properties = 1;
    uint16_t handle = 10;
    struct bluetooth::Characteristic characteristic(uuid, handle, properties);
    gattClientService2.ReadCharacteristic(appId2, characteristic);
    usleep(100);

    AttSendRespCallback(lcid, 0);
    usleep(100);
    AttErrorResponse_18(connect, buffer1);
    usleep(100);

    BufferFree(buffer1);
    gattClientService2.Disconnect(appId2);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService2.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnReadDescriptorValueEvent_Test)
{
    GattClientService gattClientService3;
    gattClientService3.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr3 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr3);
    uint8_t transport = 0x2;

    int appId3 = gattClientService3.RegisterSharedApplication(callback, addr_, transport);

    gattClientService3.Connect(appId3);
    sleep(1);

    uint16_t lcid = 0;
    uint16_t result = 1;
    uint16_t status = 1;
    void *ctx = NULL;
    const L2capConnectionInfo *info = new L2capConnectionInfo();

    AttReceiveConnectionRsp(lcid, info, result, status, ctx);
    sleep(1);

    uint16_t handle = 10;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&lcid, sizeof(uint16_t));
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00005F9B34FC");
    int permissions = 1;
    struct bluetooth::Descriptor descriptor(handle, uuid, permissions);
    gattClientService3.ReadDescriptor(appId3, descriptor);
    usleep(100);

    AttSendRespCallback(lcid, 0);
    usleep(100);
    AttReadResponse1(connect, buffer1);
    usleep(100);

    BufferFree(buffer1);
    gattClientService3.Disconnect(appId3);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService3.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnReadDescriptorValueEvent_Test_error)
{
    GattClientService gattClientService4;
    gattClientService4.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr4 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr4);
    uint8_t transport = 0x2;

    int appId4 = gattClientService4.RegisterSharedApplication(callback, addr_, transport);

    gattClientService4.Connect(appId4);
    sleep(1);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    void *ctx = NULL;
    uint16_t lcid = 0;
    uint16_t result = 1;
    uint16_t status = 1;

    AttReceiveConnectionRsp(lcid, info, result, status, ctx);
    sleep(1);

    uint16_t handle = 10;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&lcid, sizeof(uint16_t));
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001831-0000-1000-8000-00805F9B34FC");
    int permissions = 1;
    struct bluetooth::Descriptor descriptor(handle, uuid, permissions);
    gattClientService4.ReadDescriptor(appId4, descriptor);
    usleep(100);

    AttSendRespCallback(lcid, 0);
    usleep(100);
    AttErrorResponse_18(connect, buffer1);
    usleep(100);

    BufferFree(buffer1);
    gattClientService4.Disconnect(appId4);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService4.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnCharacteristicNotifyEvent)
{
    GattClientService gattClientService5;
    gattClientService5.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr5 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr5);
    uint8_t transport = 0x2;

    int appId5 = gattClientService5.RegisterSharedApplication(callback, addr_, transport);

    gattClientService5.Connect(appId5);
    sleep(1);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t status = 1;
    void *ctx = NULL;
    uint16_t lcid = 0;
    uint16_t result = 1;

    AttReceiveConnectionRsp(lcid, info, result, status, ctx);
    sleep(1);

    uint8_t offset = 0;
    Buffer *buffer5 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer5), &offset, (uint8_t *)&lcid, sizeof(uint16_t));

    AttSendRespCallback(lcid, 0);
    AttHandleValueNotification1(connect, buffer5);
    usleep(100);

    BufferFree(buffer5);
    gattClientService5.Disconnect(appId5);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService5.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnWriteCharacteristicValueEvent)
{
    GattClientService gattClientService6;
    gattClientService6.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr6 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr6);
    uint8_t transport = 0x2;

    int appId6 = gattClientService6.RegisterSharedApplication(callback, addr_, transport);

    gattClientService6.Connect(appId6);
    sleep(1);

    uint16_t lcid = 0;
    uint16_t result = 1;
    uint16_t status = 1;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid, info, result, status, ctx);
    sleep(1);

    uint8_t offset = 0;
    Buffer *buffer6 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer6), &offset, (uint8_t *)&lcid, sizeof(uint16_t));

    uint16_t handle = 2;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B31FC");
    int properties = 1;
    int permissions = 1;
    size_t length = 3;
    uint8_t *value = new uint8_t[length];
    struct bluetooth::Characteristic characteristic(uuid, handle, properties, permissions, value, length);

    gattClientService6.WriteCharacteristic(appId6, characteristic, false);
    usleep(100);

    AttSendRespCallback(lcid, 0);
    usleep(100);
    AttWriteResponse(connect, buffer6);
    usleep(100);

    BufferFree(buffer6);
    gattClientService6.Disconnect(appId6);
    sleep(1);
    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);
    gattClientService6.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, OnWriteDescriptorValueEvent)
{
    GattClientService gattClientService7;
    gattClientService7.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr7 = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr7);
    uint8_t transport = 0x2;

    int appId7 = gattClientService7.RegisterSharedApplication(callback, addr_, transport);

    gattClientService7.Connect(appId7);
    sleep(1);

    uint16_t lcid7 = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid7, info, result, status, ctx);
    sleep(1);

    uint8_t offset = 0;
    Buffer *buffer7 = BufferMalloc(sizeof(uint16_t));
    const AttConnectInfo *connect = new AttConnectInfo();
    AssembleDataPackage((uint8_t *)BufferPtr(buffer7), &offset, (uint8_t *)&lcid7, sizeof(uint16_t));

    uint16_t handle = 2;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B14FC");
    int permissions = 1;
    size_t length = 3;
    uint8_t *value = new uint8_t[length];
    struct bluetooth::Descriptor descriptor(uuid, handle, permissions, value, length);

    gattClientService7.WriteDescriptor(appId7, descriptor);
    usleep(100);

    AttSendRespCallback(lcid7, 0);
    usleep(100);
    AttWriteResponse(connect, buffer7);
    usleep(100);

    BufferFree(buffer7);
    gattClientService7.Disconnect(appId7);
    sleep(1);
    AttRecvDisconnectionRsp(lcid7, ctx);
    sleep(1);
    gattClientService7.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllPrimaryServices_1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverAllPrimaryServices(reqId, connectHandle, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReliableWriteCharacteristicValue)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    gattclientprofile.ReliableWriteCharacteristicValue(reqId, connectHandle, connectHandle,
                                                       sharedPtr1, BufferGetSize(buffer1));

    BufferFree(buffer1);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, FindIncludedServices_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x15;
    uint16_t endHandle = 0x16;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.FindIncludedServices(reqId, connectHandle, startHandle, endHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, FindIncludedServices_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t startHandle = 0x15;
    uint16_t endHandle = 0x16;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.FindIncludedServices(reqId, connectHandle, startHandle, endHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, FindIncludedServices_test3)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    GattClientService service1;
    service1.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x2;

    int appId8 = service1.RegisterSharedApplication(callback, addr_, transport);

    service1.Connect(appId8);
    sleep(1);

    uint16_t lcid8 = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    uint16_t result = 1;
    uint16_t status = 1;
    void *ctx = NULL;

    AttReceiveConnectionRsp(lcid8, info, result, status, ctx);
    sleep(1);

    service1.DiscoveryServices(appId8);
    sleep(1);

    AttSendRespCallback(lcid8, 0);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttReadByGroupTypeResponse(connect, buffer);
    sleep(3);

    AttSendRespCallback(lcid8, 0);
    AttReadByTypeResponse_3(connect, buffer);
    usleep(100);
    AttSendRespCallback(lcid8, 0);
    AttReadResponse(connect, buffer);
    usleep(100);
    BufferFree(buffer);
    service1.Disconnect(appId8);
    sleep(1);
    AttRecvDisconnectionRsp(lcid8, ctx);
    sleep(1);
    service1.Disable();
    sleep(1);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllCharacteristicOfService_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverAllCharacteristicOfService(reqId, connectHandle, startHandle, endHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllCharacteristicOfService_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverAllCharacteristicOfService(reqId, connectHandle, startHandle, endHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverCharacteristicByUuid_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFrom16Bits(0x01);
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverCharacteristicByUuid(reqId, connectHandle, startHandle, endHandle, uuid);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverCharacteristicByUuid_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0x81");
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverCharacteristicByUuid(reqId, connectHandle, startHandle, endHandle, uuid);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverCharacteristicByUuid_test3)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.DiscoverCharacteristicByUuid(reqId, connectHandle, startHandle, endHandle, uuid);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllCharacteristicDescriptors_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t connectHandle2 = 0x16;
    uint16_t startHandle = 0xFFFF;
    uint16_t startHandle2 = 0xFFFF;
    uint16_t endHandle = 0;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);

    gattclientprofile.DiscoverAllCharacteristicDescriptors(reqId, connectHandle, startHandle, endHandle);

    gattclientprofile.DiscoverAllCharacteristicDescriptors(reqId, connectHandle2, startHandle2, endHandle);

    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllCharacteristicDescriptors_test2)
{
    int reqId = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();

    uint16_t handle = 0x13;
    uint16_t startHandle =  2;
    uint16_t endHandle = 0x13;
    gattclientprofile.DiscoverAllCharacteristicDescriptors(reqId, handle, startHandle, endHandle);

    AttSendRespCallback(handle, 0);

    const AttConnectInfo *attConnectInfo = new AttConnectInfo();
    uint16_t value = 0x13;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));
    AttFindInformationResponse(attConnectInfo, buffer);

    gattclientprofile.Disable();
    delete attConnectInfo;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverAllCharacteristicDescriptors_test3)
{
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle =  2;
    uint16_t endHandle = 65535;
    gattclientprofile.DiscoverAllCharacteristicDescriptors(reqId, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);

    const AttConnectInfo *connect1 = new AttConnectInfo();
    uint16_t value1 = 0x13;
    uint8_t offset1 = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset1, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    AttFindInformationResponse(connect1, buffer1);

    gattclientprofile.Disable();
    delete connect1;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ReadCharacteristicValue_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadCharacteristicValue(reqId, connectHandle, handle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadCharacteristicValue_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadCharacteristicValue(reqId, connectHandle, handle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadUsingCharacteristicByUuid_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000001");
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, connectHandle, uuid);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadUsingCharacteristicByUuid_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000002");
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, connectHandle, uuid);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadLongCharacteristicValueParsing)
{
    int reqId = 1;
    uint16_t connectHandle = 0;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadLongCharacteristicValue(reqId, connectHandle, handle);

    AttSendRespCallback(0, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttReadBlobResponse(connect, buffer);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, ReadMultipleCharacteristicValue_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    auto sharedPtr = GattValue(new std::unique_ptr<uint8_t[]>(nullptr));
    gattclientprofile.ReadMultipleCharacteristicValue(reqId, connectHandle, sharedPtr, 0);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadMultipleCharacteristicValue_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t connectHandle2 = 0x13;
    size_t BufferSize = 2;
    Buffer *handleList = BufferMalloc(BufferSize);
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(handleList), BufferGetSize(handleList));

    gattclientprofile.ReadMultipleCharacteristicValue(reqId, connectHandle, sharedPtr1, BufferGetSize(handleList));

    gattclientprofile.ReadMultipleCharacteristicValue(reqId, connectHandle2, sharedPtr1, BufferGetSize(handleList));

    BufferFree(handleList);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadDescriptorValue_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadDescriptorValue(reqId, connectHandle, handle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadDescriptorValue_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.ReadDescriptorValue(reqId, connectHandle, handle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadLongCharacteristicDescriptor)
{
    int reqId = 1;
    uint16_t connectHandle = 0;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadLongCharacteristicDescriptor(reqId, connectHandle, handle);

    AttSendRespCallback(0, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttReadBlobResponse(connect, buffer);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, WriteWithoutResponse_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    auto sharedPtr = GattValue(new std::unique_ptr<uint8_t[]>(nullptr));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteWithoutResponse(reqId, connectHandle, handle, sharedPtr, 0);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteWithoutResponse_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t handle = 0x13;
    size_t BufferSize = 2;
    Buffer *value = BufferMalloc(BufferSize);
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(value), BufferGetSize(value));
    gattclientprofile.WriteWithoutResponse(reqId, connectHandle, handle, sharedPtr1, BufferGetSize(value));
    BufferFree(value);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, DiscoverPrimaryServicesByServiceUuid)
{
    int reqId = 1;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    const Uuid uuid_1 = Uuid::ConvertFromString("00000");
    const Uuid uuid_2 = Uuid::ConvertFromString("01234");
    const Uuid uuid_3 = Uuid::ConvertFromString("999999999999");
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, 0, uuid_1);
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, 0, uuid_2);
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, 0, uuid_3);

    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ReadUsingCharacteristicByUuid)
{
    int reqId = 1;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    const Uuid uuid_1 = Uuid::ConvertFromString("00000");
    const Uuid uuid_2 = Uuid::ConvertFromString("01234");
    const Uuid uuid_3 = Uuid::ConvertFromString("999999999999");
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, 0, uuid_1);
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, 0, uuid_2);
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, 0, uuid_3);

    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteCharacteristicValue_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset1 = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset1, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteCharacteristicValue(reqId, connectHandle, handle, sharedPtr1, BufferGetSize(buffer1));
    BufferFree(buffer1);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteCharacteristicValue_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x19;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer), BufferGetSize(buffer));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteCharacteristicValue(reqId, connectHandle, handle, sharedPtr, BufferGetSize(buffer));
    BufferFree(buffer);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteLongCharacteristicValue_test1)
{
    uint16_t connectHandle = 0x13;
    int reqId = 1;
    uint16_t handle = 0x13;
    uint16_t value = 0x13;
    uint8_t offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattClientProfile(clientcallback, dispatcher);
    gattClientProfile.Enable();
    gattClientProfile.WriteLongCharacteristicValue(reqId, connectHandle, handle, sharedPtr, BufferGetSize(buffer2));

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect2 = new AttConnectInfo();
    AttExecuteWriteResponse(connect2, buffer2);
    usleep(100);

    gattClientProfile.Disable();
    BufferFree(buffer2);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, WriteLongCharacteristicValue_test2)
{
    uint16_t connectHandle = 0x19;
    uint16_t handle = 0x13;
    uint16_t value = 8;
    int reqId = 1;
    uint8_t offset = 0;
    Buffer *buffer_ccc = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_ccc), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr_ccc = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_ccc), BufferGetSize(buffer_ccc));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteLongCharacteristicValue(reqId, connectHandle, handle, sharedPtr_ccc, BufferGetSize(buffer_ccc));
    BufferFree(buffer_ccc);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, SplitPrepareWriteRsp_ccc)
{
    int reqId = 1;
    uint8_t offset = 0;
    uint16_t connectHandle = 0;
    uint16_t handle = 0x13;
    uint16_t value = 8;
    Buffer *buffer_rsp1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_rsp1), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr_ccc = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_rsp1), BufferGetSize(buffer_rsp1));
    GattClientProfileCallbackImplement *clientcallback_ccc = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback_ccc, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteLongCharacteristicValue(reqId, connectHandle, handle, sharedPtr_ccc, 0x26);

    AttSendRespCallback(0, 0);
    usleep(500);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttPrepareWriteResponse(connect, buffer);
    AttSendRespCallback(0, 0);
    usleep(500);
    AttPrepareWriteResponse(connect, buffer);
    AttSendRespCallback(0, 0);
    usleep(500);
    AttPrepareWriteResponse(connect, buffer);

    gattclientprofile.Disable();
    BufferFree(buffer_rsp1);
    delete clientcallback_ccc;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, WriteDescriptorValue_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value2 = 8;
    uint8_t offset = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr2 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteDescriptorValue(reqId, connectHandle, handle, sharedPtr2, BufferGetSize(buffer2));
    BufferFree(buffer2);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteDescriptorValue_test2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x16;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteDescriptorValue(reqId, connectHandle, handle, sharedPtr1, BufferGetSize(buffer1));
    BufferFree(buffer1);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteLongCharacteristicDescriptor_test1)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value3 = 8;
    uint8_t offset = 0;
    Buffer *buffer3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer3), &offset, (uint8_t *)&value3, sizeof(uint16_t));
    auto sharedPtr3 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer3), BufferGetSize(buffer3));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId, connectHandle, handle,
                                                        sharedPtr3, BufferGetSize(buffer3));
    BufferFree(buffer3);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, WriteLongCharacteristicDescriptor_test2)
{
    uint16_t connectHandle = 0x19;
    uint16_t handle = 0x13;
    uint16_t value = 8;
    uint8_t offset = 0;
    int reqId = 1;
    Buffer *buffer_desc = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_desc), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr_desc = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_desc), BufferGetSize(buffer_desc));
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId, connectHandle, handle,
                                                        sharedPtr_desc, BufferGetSize(buffer_desc));
    BufferFree(buffer_desc);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, SplitPrepareWriteRsp_desc)
{
    int reqId = 1;
    uint16_t connectHandle = 0;
    uint16_t handle = 0x13;
    uint16_t value = 8;
    uint8_t offset = 0;
    Buffer *buffer_rsp2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_rsp2), &offset, (uint8_t *)&value, sizeof(uint16_t));
    auto sharedPtr_desc = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_rsp2), BufferGetSize(buffer_rsp2));
    GattClientProfileCallbackImplement *clientcallback_desc = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback_desc, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteDescriptorValue(reqId, connectHandle, handle, sharedPtr_desc, 0x26);

    AttSendRespCallback(0, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttPrepareWriteResponse(connect, buffer);
    AttSendRespCallback(0, 0);
    usleep(100);
    AttPrepareWriteResponse(connect, buffer);
    AttSendRespCallback(0, 0);
    usleep(100);
    AttPrepareWriteResponse(connect, buffer);

    gattclientprofile.Disable();
    BufferFree(buffer_rsp2);
    delete clientcallback_desc;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, HandleValueConfirmation_test1)
{
    uint16_t connectHandle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.HandleValueConfirmation(connectHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, HandleValueConfirmation_test2)
{
    uint16_t connectHandle = 0x16;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    gattclientprofile.HandleValueConfirmation(connectHandle);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, GetService)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    const GattCache::Service *result = gattclientprofile.GetService(connectHandle, handle);
    EXPECT_EQ(nullptr, result);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, GetCharacteristic)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    const GattCache::Characteristic *result = gattclientprofile.GetCharacteristic(connectHandle, handle);
    EXPECT_EQ(nullptr, result);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, GetDescriptor)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    const GattCache::Descriptor *result = gattclientprofile.GetDescriptor(connectHandle, handle);
    EXPECT_EQ(nullptr, result);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_ReadByGroupType_AttAttributeNotFound)
{
    int reqId = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer = nullptr;
    AttErrorResponse_1(connect, buffer);

    gattclientprofile.Disable();
    delete connect;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_ReadByGroupType)
{
    int reqId1 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId1, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect1 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_2(connect1, buffer);

    gattclientprofile.Disable();
    delete connect1;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_ReadByType)
{
    int reqId = 1;
    uint16_t connectHandle = 0x23;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *clientcallbackIml1 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallbackIml1, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllCharacteristicOfService(reqId, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_3(connect, buffer1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer1);
    delete clientcallbackIml1;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_3_DISCOVER_CHARACTERISTIC_BY_UUID)
{
    int reqId = 1;
    uint16_t connectHandle = 0x23;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFrom16Bits(0x01);
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer_res1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res1), BufferGetSize(buffer_res1));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverCharacteristicByUuid(reqId, connectHandle, startHandle, endHandle, uuid);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_3(connect, buffer_res1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_3_READ_USING_CHARACTERISTIC_UUID)
{
    int reqId = 1;
    uint16_t connectHandle = 0x23;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000001");
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer_res2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res2), BufferGetSize(buffer_res2));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, connectHandle, uuid);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_3(connect, buffer_res2);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res2);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_3_FIND_INCLUDE_SERVICE)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x15;
    uint16_t endHandle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000001");
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_res3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res3), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res3), BufferGetSize(buffer_res3));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.FindIncludedServices(reqId, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_3(connect, buffer_res3);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res3);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_FindByTypeValue_AttAttributeNotFound)
{
    int reqId2 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId2, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect2 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_4(connect2, buffer);

    gattclientprofile.Disable();
    delete connect2;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_FindByTypeValue)
{
    int reqId3 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId3, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect3 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_5(connect3, buffer);

    gattclientprofile.Disable();
    delete connect3;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_FindInformation)
{
    int reqId4 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId4, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect4 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_7(connect4, buffer);

    gattclientprofile.Disable();
    delete connect4;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_Read)
{
    int reqId5 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId5, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect5 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_8(connect5, buffer);

    gattclientprofile.Disable();
    delete connect5;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ErrorResponse_Write)
{
    int reqId6 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId6, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect6 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_9(connect6, buffer);

    gattclientprofile.Disable();
    delete connect6;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ReadBlobResponse)
{
    int reqId7 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId7, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect7 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttReadBlobResponse(connect7, buffer);

    gattclientprofile.Disable();
    delete connect7;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, PrepareWriteResponse)
{
    int reqId8 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId8, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect8 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttPrepareWriteResponse(connect8, buffer);

    gattclientprofile.Disable();
    delete connect8;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, HandleValueNotification)
{
    int reqId9 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId9, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect9 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttHandleValueNotification(connect9, buffer);

    gattclientprofile.Disable();
    delete connect9;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ExchangeMTUResponse)
{
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.ExchangeMtu(reqId, 0, 64);
    AttSendRespCallback(0, 0);

    AttConnectInfo *connect = new AttConnectInfo();
    Buffer *buffer_mtu1 = BufferMalloc(sizeof(uint16_t));
    uint16_t value1 = 22;
    uint8_t offset = 0;
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_mtu1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttExchangeMTUResponse(connect, buffer_mtu1);

    gattclientprofile.Disable();
    BufferFree(buffer_mtu1);
    delete connect;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ExchangeMTUResponse_set)
{
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.ExchangeMtu(reqId, 0, 64);
    AttSendRespCallback(0, 0);

    AttConnectInfo *connect = new AttConnectInfo();
    uint16_t value1 = 64;
    uint8_t offset = 0;
    Buffer *buffer_mtu2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_mtu2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttExchangeMTUResponse(connect, buffer_mtu2);

    gattclientprofile.Disable();
    BufferFree(buffer_mtu2);
    delete connect;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, FindInformationResponse)
{
    int reqId = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect2 = new AttConnectInfo();
    uint16_t value2 = 0x13;
    uint8_t offset2 = 0;
    Buffer *buffer2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset2, (uint8_t *)&value2, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    AttFindInformationResponse(connect2, buffer2);

    gattclientprofile.Disable();
    delete connect2;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, FindByTypeValueResponse)
{
    uint16_t connectHandle = 1;
    uint16_t handle = 0x13;
    uint16_t value1 = 1;
    uint8_t offset = 0;
    Buffer *buffer_res4 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res4), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res4), BufferGetSize(buffer_res4));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    const Uuid uuid_1 = Uuid::ConvertFromString("012345");
    int reqId = 1;
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, connectHandle, uuid_1);
    AttSendRespCallback(connectHandle, 0);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttFindByTypeValueResponse(connect, buffer_res4);

    gattclientprofile.Disable();
    delete connect;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, HandleValueIndication)
{
    int reqId10 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId10, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect10 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttHandleValueIndication(connect10, buffer);

    gattclientprofile.Disable();
    delete connect10;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_DiscoverPrimaryServicesByServiceUuid)
{
    uint16_t connectHandle = 0x13;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    const Uuid uuid = Uuid::ConvertFromString("00000");
    int reqId = 1;
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, connectHandle, uuid);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_FindIncludedServices)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x15;
    uint16_t endHandle = 0x16;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.FindIncludedServices(reqId, connectHandle, startHandle, endHandle);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_DiscoverCharacteristicByUuid)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFrom16Bits(0x01);
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverCharacteristicByUuid(reqId, connectHandle, startHandle, endHandle, uuid);

    AttSendRespCallback(connectHandle, 0);

    const AttConnectInfo *connect = new AttConnectInfo();
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    AttReadByTypeResponse_1(connect, buffer1);

    gattclientprofile.Disable();
    BufferFree(buffer1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_ReadCharacteristicValue)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x15;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadCharacteristicValue(reqId, connectHandle, handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadMultipleResponse(connect, buffer1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_ReadUsingCharacteristicByUuid)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000001");
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId, connectHandle, uuid);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_ReadMultipleCharacteristicValue)
{
    int reqId = 1;
    uint16_t connectHandle2 = 0x13;
    size_t BufferSize = 2;
    Buffer *handleList = BufferMalloc(BufferSize);
    auto sharedPtr1 = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(handleList), BufferGetSize(handleList));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadMultipleCharacteristicValue(reqId, connectHandle2, sharedPtr1, BufferGetSize(handleList));

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    BufferFree(handleList);
    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_ReadDescriptorValue)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x15;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadDescriptorValue(reqId, connectHandle, handle);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_ReadLongCharacteristicDescriptor)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.ReadLongCharacteristicDescriptor(reqId, connectHandle, handle);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_WriteCharacteristicValue)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    auto sharedPtr = GattValue(new std::unique_ptr<uint8_t[]>(nullptr));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.WriteCharacteristicValue(reqId, connectHandle, handle, sharedPtr, 0);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_WriteLongCharacteristicValue)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *bufferValue1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(bufferValue1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(bufferValue1), BufferGetSize(bufferValue1));
    GattClientProfileCallbackImplement *clientcallback1 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback1, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.WriteLongCharacteristicValue(reqId, connectHandle, handle, sharedPtr, 0);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    BufferFree(bufferValue1);
    delete clientcallback1;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_WriteDescriptorValue)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *bufferValue2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(bufferValue2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(bufferValue2), BufferGetSize(bufferValue2));
    GattClientProfileCallbackImplement *clientcallback2 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback2, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.WriteDescriptorValue(reqId, connectHandle, handle, sharedPtr, 0);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    BufferFree(bufferValue2);
    delete clientcallback2;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_WriteLongCharacteristicDescriptor)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *bufferValue3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(bufferValue3), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(bufferValue3), BufferGetSize(bufferValue3));
    GattClientProfileCallbackImplement *clientcallback3 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback3, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId, connectHandle, handle, sharedPtr, 0);

    uint16_t lcid = 0;
    int result = 1;
    AttSendRespCallback(lcid, result);

    gattclientprofile.Disable();
    BufferFree(bufferValue3);
    delete clientcallback3;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SendRespCallback_RequestExchangeMtu)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    GattClientService service2;
    service2.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr = "00:00:00:00:00:00";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x2;

    int appId = service2.RegisterApplication(callback, addr_, transport);

    service2.Connect(appId);
    sleep(1);

    uint16_t lcid = 0;
    const L2capConnectionInfo *info = new L2capConnectionInfo();
    void *ctx = NULL;
    AttReceiveConnectionRsp(lcid, info, 1, 1, ctx);
    sleep(1);

    int mtu = 64;
    service2.RequestExchangeMtu(appId, mtu);
    sleep(1);

    AttSendRespCallback(lcid, 0);
    AttConnectInfo *connect = new AttConnectInfo();
    uint16_t value1 = 22;
    uint8_t offset = 0;
    Buffer *buffer = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    AttExchangeMTUResponse(connect, buffer);

    AttSendRespCallback(lcid, 1);

    service2.Disconnect(appId);
    sleep(1);

    AttRecvDisconnectionRsp(lcid, ctx);
    sleep(1);

    service2.Disable();
    sleep(1);
    BufferFree(buffer);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, ExecuteWriteRequest)
{
    GattClientProfileCallbackImplement *callback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(callback, dispatcher);
    uint16_t connectHandle = 0x11;
    uint8_t flag = 1;
    int reqId = 1;
    gattclientprofile.ExecuteWriteRequest(reqId, connectHandle, flag);
    delete dispatcher;
    delete callback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_6)
{
    int reqId11 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId11, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect11 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_6(connect11, buffer);

    gattclientprofile.Disable();
    delete connect11;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_10)
{
    int reqId12 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId12, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect12 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_10(connect12, buffer);

    gattclientprofile.Disable();
    delete connect12;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_11)
{
    int reqId13 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId13, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect13 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_11(connect13, buffer);

    gattclientprofile.Disable();
    delete connect13;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_12)
{
    int reqId14 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId14, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect14 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_12(connect14, buffer);

    gattclientprofile.Disable();
    delete connect14;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_13)
{
    int reqId15 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId15, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect15 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_13(connect15, buffer);

    gattclientprofile.Disable();
    delete connect15;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_14)
{
    int reqId16 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId16, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect16 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_14(connect16, buffer);

    gattclientprofile.Disable();
    delete connect16;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_15)
{
    int reqId17 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId17, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect17 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_15(connect17, buffer);

    gattclientprofile.Disable();
    delete connect17;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, AttErrorResponse_16)
{
    int reqId18 = 1;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllPrimaryServices(reqId18, 0, MIN_ATTRIBUTE_HANDLE, MAX_ATTRIBUTE_HANDLE);
    AttSendRespCallback(0, 0);

    const AttConnectInfo *connect18 = new AttConnectInfo();
    Buffer *buffer = NULL;
    AttErrorResponse_16(connect18, buffer);

    gattclientprofile.Disable();
    delete connect18;
    delete dispatcher;
    delete clientcallback;
}

TEST_F(Gatt_Client_Profile_Test, ReliableWriteCharacteristicValue2)
{
    int reqId = 1;
    uint16_t connectHandle = 8;
    uint16_t handle = 0x13;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *bufferValue4 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(bufferValue4), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(bufferValue4), BufferGetSize(bufferValue4));
    GattClientProfileCallbackImplement *clientcallback4 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback4, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReliableWriteCharacteristicValue(reqId, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttPrepareWriteResponse1(connect, bufferValue4);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(bufferValue4);
    delete clientcallback4;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_DISCOVER_SERVICE_BY_UUID)
{
    uint16_t connectHandle = 8;
    uint16_t value1 = 8;
    uint8_t offset = 0;
    Buffer *buffer_res5 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res5), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res5), BufferGetSize(buffer_res5));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    const Uuid uuid_1 = Uuid::ConvertFromString("01234");
    int reqId = 1;
    gattclientprofile.DiscoverPrimaryServicesByServiceUuid(reqId, connectHandle, uuid_1);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_res5);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res5);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_FIND_INCLUDE_SERVICE)
{
    int reqId_error1 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x15;
    uint16_t endHandle = 0x16;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error1), BufferGetSize(buffer_error1));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.FindIncludedServices(reqId_error1, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTOR)
{
    int reqId_error2 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle =  2;
    uint16_t endHandle = 65535;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error2), BufferGetSize(buffer_error2));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllCharacteristicDescriptors(reqId_error2, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error2);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error2);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_READ_CHARACTERISTIC_VALUE)
{
    int reqId_error3 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t Handle =  2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error3), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error3), BufferGetSize(buffer_error3));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadCharacteristicValue(reqId_error3, connectHandle, Handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error3);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error3);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_READ_USING_CHARACTERISTIC_UUID)
{
    int reqId_error4 = 1;
    uint16_t connectHandle = 0x13;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("0000001");
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error4 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error4), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error4), BufferGetSize(buffer_error4));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadUsingCharacteristicByUuid(reqId_error4, connectHandle, uuid);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error4);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error4);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_READ_MULTIPLE_CHARACTERISTIC)
{
    int reqId_error5 = 1;
    uint16_t connectHandle2 = 0x13;
    size_t BufferSize = 2;
    Buffer *handleList = BufferMalloc(BufferSize);
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error5 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error5), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error5), BufferGetSize(buffer_error5));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadMultipleCharacteristicValue(reqId_error5, connectHandle2, sharedPtr, BufferGetSize(handleList));

    AttSendRespCallback(connectHandle2, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error5);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error5);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_READ_CHARACTERISTIC_DESCRIPTOR)
{
    int reqId_error6 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error6 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error6), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error6), BufferGetSize(buffer_error6));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadDescriptorValue(reqId_error6, connectHandle, handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error6);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error6);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_READ_LONG_CHARACTERISTIC_DESCRIPTOR)
{
    int reqId_error7 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error7 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error7), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error7), BufferGetSize(buffer_error7));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadLongCharacteristicDescriptor(reqId_error7, connectHandle, handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error7);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error7);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_WRITE_CHARACTERISTIC_VALUE)
{
    int reqId = 1;
    uint16_t connectHandle = 0x23;
    uint16_t handle = 2;
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer_res6 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res6), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res6), BufferGetSize(buffer_res6));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteCharacteristicValue(reqId, connectHandle, handle, sharedPtr, 0x03);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_res6);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res6);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_WRITE_CHARACTERISTIC_DESCRIPTOR)
{
    int reqId = 1;
    uint16_t handle = 2;
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    uint16_t connectHandle = 0x23;
    Buffer *buffer_res7 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_res7), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_res7), BufferGetSize(buffer_res7));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteDescriptorValue(reqId, connectHandle, handle, sharedPtr, 0x03);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_res7);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_res7);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_WRITE_LONG_CHARACTERISTIC_VALUE)
{
    int reqId_error8 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error8 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error8), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error8), BufferGetSize(buffer_error8));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteLongCharacteristicValue(reqId_error8, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error8);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error8);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_WRITE_LONG_CHARACTERISTIC_DESCRIPTOR)
{
    int reqId_error9 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error9 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error9), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error9), BufferGetSize(buffer_error9));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId_error9, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error9);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error9);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_EXCHANGE_MTU)
{
    uint16_t connectHandle = 0x13;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error10 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error10), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error10), BufferGetSize(buffer_error10));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId = 1;
    gattclientprofile.ExchangeMtu(reqId, connectHandle, 64);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer_error10);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error10);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, error_DISCOVER_ALL_CHARACTERISTIC)
{
    int reqId = 1;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *clientcallbackIml2 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallbackIml2, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllCharacteristicOfService(reqId, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttErrorResponse_18(connect, buffer1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer1);
    delete clientcallbackIml2;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitReadRsp1)
{
    int reqId_readRsp1 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t sHandle = 0x13;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_readRsp1 = BufferMalloc(22);
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_readRsp1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadCharacteristicValue(reqId_readRsp1, connectHandle, sHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadResponse1(connect, buffer_readRsp1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_readRsp1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitReadRsp2)
{
    int reqId_readRsp2 = 1;
    uint16_t connectHandle = 0x13;
    uint16_t sHandle = 0x13;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_readRsp2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_readRsp2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadCharacteristicValue(reqId_readRsp2, connectHandle, sHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadResponse1(connect, buffer_readRsp2);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_readRsp2);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitReadRsp3)
{
    int reqId_readRsp3 = 1;
    uint16_t connectHandle = 1;
    uint16_t rHandle = 0x13;
    uint16_t value1 = 1;
    uint8_t offset = 0;
    Buffer *buffer_readRsp3 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_readRsp3), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadDescriptorValue(reqId_readRsp3, connectHandle, rHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadResponse1(connect, buffer_readRsp3);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_readRsp3);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitReadRsp4)
{
    int reqId_readRsp4 = 1;
    uint16_t connectHandle = 1;
    uint16_t rHandle = 0x13;
    uint16_t value1 = 1;
    uint8_t offset = 0;
    Buffer *buffer_readRsp4 = BufferMalloc(22);
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_readRsp4), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadDescriptorValue(reqId_readRsp4, connectHandle, rHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadResponse1(connect, buffer_readRsp4);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_readRsp4);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitWriteRsp1)
{
    uint16_t connectHandle = 0x23;
    uint16_t handle = 0x13;
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer_writeRsp1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_writeRsp1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_writeRsp1), BufferGetSize(buffer_writeRsp1));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId_writeRsp1 = 1;
    gattclientprofile.WriteCharacteristicValue(reqId_writeRsp1, connectHandle, handle, sharedPtr, 0x03);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttWriteResponse(connect, buffer_writeRsp1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_writeRsp1);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, SplitWriteRsp2)
{
    uint16_t connectHandle = 0x23;
    uint16_t handle = 0x13;
    uint16_t value1 = 0x23;
    uint8_t offset = 0;
    Buffer *buffer_writeRsp2 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_writeRsp2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_writeRsp2), BufferGetSize(buffer_writeRsp2));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId_writeRsp2 = 1;
    gattclientprofile.WriteDescriptorValue(reqId_writeRsp2, connectHandle, handle, sharedPtr, 0x03);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttWriteResponse(connect, buffer_writeRsp2);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_writeRsp2);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, GetCharacteristicEndHandle)
{
    uint16_t connectHandle = 0x01;
    uint16_t serviceHandle = 0x01;
    uint16_t characteristicsHandle = 0x01;
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    int result = gattclientprofile.GetCharacteristicEndHandle(connectHandle, serviceHandle, characteristicsHandle);
    EXPECT_EQ(0, result);
}

TEST_F(Gatt_Client_Profile_Test, SplitReadByTypeRsp1)
{
    int reqId = 1;
    uint16_t value1 = 0x13;
    uint16_t connectHandle = 0x13;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x13;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *clientcallbackIml3 = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallbackIml3, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.DiscoverAllCharacteristicOfService(reqId, connectHandle, startHandle, endHandle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadByTypeResponse_1(connect, buffer1);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer1);
    delete clientcallbackIml3;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, ReadLongCharacteristicValueParsing_2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x80;
    uint16_t handle = 0x13;
    uint16_t value1 = 0x80;
    uint8_t offset = 0;
    Buffer *buffer1 = BufferMalloc(22);
    AssembleDataPackage((uint8_t *)BufferPtr(buffer1), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer1), BufferGetSize(buffer1));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadLongCharacteristicValue(reqId, connectHandle, handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadBlobResponse1(connect, buffer1);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, ReadLongCharacteristicDescriptor_2)
{
    int reqId = 1;
    uint16_t connectHandle = 0x81;
    uint16_t handle = 0x13;
    uint16_t value1 = 0x81;
    uint8_t offset = 0;
    Buffer *buffer2 = BufferMalloc(22);
    AssembleDataPackage((uint8_t *)BufferPtr(buffer2), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer2), BufferGetSize(buffer2));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    gattclientprofile.ReadLongCharacteristicDescriptor(reqId, connectHandle, handle);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadBlobResponse1(connect, buffer2);

    gattclientprofile.Disable();
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, WriteLongCharacteristicDescriptor_test)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error11 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error11), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error11), BufferGetSize(buffer_error11));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientProfile(clientcallback, dispatcher);
    gattclientProfile.Enable();
    int reqId_error11 = 1;
    gattclientProfile.WriteLongCharacteristicDescriptor(reqId_error11, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect1 = new AttConnectInfo();
    AttExecuteWriteResponse(connect1, buffer_error11);
    usleep(100);

    gattclientProfile.Disable();
    BufferFree(buffer_error11);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, AttTransactionTimeOutId_test)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error12 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error12), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error12), BufferGetSize(buffer_error12));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId_error12 = 1;
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId_error12, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttTransactionTimeOutId(connect, buffer_error12);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error12);
    delete clientcallback;
    delete dispatcher;
}

TEST_F(Gatt_Client_Profile_Test, FindIncludedServices_all)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    const std::string addr_1 = "0001";
    bluetooth::RawAddress addr1(addr_1);
    uint8_t type = 0x0;
    uint8_t transport1 = 0x1;
    bluetooth::GattDevice device1(addr1, type, transport1);
    const std::string addr_2 = "0002";
    bluetooth::RawAddress addr2(addr_2);
    bluetooth::GattDevice device2(addr2, type, transport1);
    const std::string addr_3 = "0003";
    bluetooth::RawAddress addr3(addr_3);
    bluetooth::GattDevice device3(addr3, type, transport1);
    GattConnectionManager::GetInstance().Connect(device1);
    GattConnectionManager::GetInstance().Connect(device2);
    GattConnectionManager::GetInstance().Connect(device3);

    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);

    IGattClientCallbackImplement_2 callback;
    const std::string addr = "00:00:00:00:00:01";
    bluetooth::RawAddress addr_(addr);
    uint8_t transport = 0x2;
    int appId9 = gattClientService.RegisterSharedApplication(callback, addr_, transport);

    gattClientService.Connect(appId9);
    sleep(1);

    const L2capConnectionInfo *info = new L2capConnectionInfo();
    AttReceiveConnectionRsp(0, info, 1, 1, NULL);
    sleep(1);

    gattClientService.DiscoveryServices(appId9);
    sleep(1);

    AttSendRespCallback(0, 0);
    sleep(1);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttReadByGroupTypeResponse(connect, NULL);
    sleep(1);

    SendResponse_Discover(connect);

    gattClientService.GetServices(appId9);
    sleep(1);

    gattClientService.Disconnect(appId9);
    sleep(1);
    AttRecvDisconnectionRsp(0, NULL);
    sleep(1);
    gattClientService.Disable();
    sleep(1);
    GattConnectionManager::GetInstance().Disconnect(device1);
    GattConnectionManager::GetInstance().Disconnect(device2);
    GattConnectionManager::GetInstance().Disconnect(device3);
    delete connect;
}

TEST_F(Gatt_Client_Profile_Test, AttNull_test)
{
    uint16_t connectHandle = 0x13;
    uint16_t handle = 2;
    uint16_t value1 = 0x13;
    uint8_t offset = 0;
    Buffer *buffer_error13 = BufferMalloc(sizeof(uint16_t));
    AssembleDataPackage((uint8_t *)BufferPtr(buffer_error13), &offset, (uint8_t *)&value1, sizeof(uint16_t));
    auto sharedPtr = GattServiceBase::BuildGattValue((uint8_t *)BufferPtr(buffer_error13), BufferGetSize(buffer_error13));
    GattClientProfileCallbackImplement *clientcallback = new GattClientProfileCallbackImplement();
    utility::Dispatcher *dispatcher = new utility::Dispatcher("bt-dispatcher");
    bluetooth::GattClientProfile gattclientprofile(clientcallback, dispatcher);
    gattclientprofile.Enable();
    int reqId_error13 = 1;
    gattclientprofile.WriteLongCharacteristicDescriptor(reqId_error13, connectHandle, handle, sharedPtr, 0x26);

    AttSendRespCallback(connectHandle, 0);
    usleep(100);

    const AttConnectInfo *connect = new AttConnectInfo();
    AttNull(connect, buffer_error13);
    usleep(100);

    gattclientprofile.Disable();
    BufferFree(buffer_error13);
    delete clientcallback;
    delete dispatcher;
}
