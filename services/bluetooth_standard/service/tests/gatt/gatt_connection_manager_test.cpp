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
#include "gatt_client_service.h"
#include "gatt_server_service.h"
#include "src/att/att_connect.h"
#include "src/att/att_receive.h"
#include "mock/ble_adapter_mock.h"
#include "mock/gatt_common.h"
#include "common_mock_all.h"
#include "bt_def.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace bluetooth;
using namespace testing;

class Gatt_Connection_Manager_Test : public testing::Test {
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
        EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));

        AdapterConfigMockSetUp();

        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        gattClientService = new GattClientService();
        gattClientService->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(gattClientService));
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
    IAdapter *iAdapter = NULL;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

struct AsyncCallInfromation_1 {
    bool isCompleted_;
    int state_;

    std::mutex mutex_;
    std::condition_variable condition_;
};

AsyncCallInfromation_1 asyncCallInfromation_1;

class IGattClientCallbackImplement_1 : public IGattClientCallback {
public:
    void OnServicesChanged(const std::vector<Service> &services) override{};
    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {
        std::unique_lock<std::mutex> lock_readccc(asyncCallInfromation_1.mutex_);
        asyncCallInfromation_1.isCompleted_ = true;
        asyncCallInfromation_1.condition_.notify_all();
    };
    void OnCharacteristicWrite(int ret, const Characteristic &characteristic) override
    {
        std::unique_lock<std::mutex> lock_writeccc(asyncCallInfromation_1.mutex_);
        asyncCallInfromation_1.isCompleted_ = true;
        asyncCallInfromation_1.condition_.notify_all();
    };
    void OnCharacteristicChanged(const Characteristic &characteristic) override{};
    void OnDescriptorRead(int ret, const Descriptor &descriptor) override{};
    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override
    {
        std::unique_lock<std::mutex> lock_writedesc(asyncCallInfromation_1.mutex_);
        asyncCallInfromation_1.isCompleted_ = true;
        asyncCallInfromation_1.condition_.notify_all();
    };
    void OnConnectionStateChanged(int state, int newState) override
    {
        if (newState == 1) {
            std::unique_lock<std::mutex> lock_con(asyncCallInfromation_1.mutex_);
            asyncCallInfromation_1.isCompleted_ = true;
            asyncCallInfromation_1.condition_.notify_all();
        } else if (newState == GATT_UT_OFFSET_3) {
            std::unique_lock<std::mutex> lock_discon(asyncCallInfromation_1.mutex_);
            asyncCallInfromation_1.isCompleted_ = true;
            asyncCallInfromation_1.condition_.notify_all();
        }
    };
    void OnMtuChanged(int state, int mtu) override
    {
        std::unique_lock<std::mutex> lock_mtu(asyncCallInfromation_1.mutex_);
        asyncCallInfromation_1.isCompleted_ = true;
        asyncCallInfromation_1.condition_.notify_all();
    }

    IGattClientCallbackImplement_1()
    {}
    ~IGattClientCallbackImplement_1()
    {}
};

class GattConnectionObserverImplement : public GattConnectionObserver {
public:
    GattConnectionObserverImplement()
    {}
    ~GattConnectionObserverImplement()
    {}
};

TEST_F(Gatt_Connection_Manager_Test, GattConnectionObserverTest)
{
    GattConnectionObserverImplement testCallback;
    const std::string addr_ = "0001";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x0;
    uint8_t transport = 0x1;
    bluetooth::GattDevice device(addr, type, transport);
    uint16_t connectionHandle = 0x01;
    int ret = 1;
    int interval = 1;
    int latency = 1;
    int timeout = 1;
    int status = 1;
    testCallback.OnConnect(device, connectionHandle, ret);
    testCallback.OnDisconnect(device, connectionHandle, ret);
    testCallback.OnConnectionParameterChanged(device, interval, latency, timeout, status);
}

TEST_F(Gatt_Connection_Manager_Test, ConnectTestDevice)
{
    iAdapter = new BleAdapter();
    EXPECT_CALL(GetAdapterManagerMock(), GetAdapter()).Times(AtLeast(0)).WillRepeatedly(Return(iAdapter));
    const std::string addr_1 = "00001";
    bluetooth::RawAddress addr1(addr_1);
    uint8_t type = 0x0;
    uint8_t transport = 0x1;
    bluetooth::GattDevice gattDevice1(addr1, type, transport);
    const std::string addr_2 = "00002";
    bluetooth::RawAddress addr2(addr_2);
    bluetooth::GattDevice gattDevice2(addr2, type, transport);
    const std::string addr_3 = "00003";
    bluetooth::RawAddress addr3(addr_3);
    bluetooth::GattDevice gattDevice3(addr3, type, transport);
    GattConnectionManager::GetInstance().Connect(gattDevice1);
    GattConnectionManager::GetInstance().Connect(gattDevice2);
    GattConnectionManager::GetInstance().Connect(gattDevice3);

    std::vector<GattDevice> dev;
    dev.push_back(gattDevice3);
    GattConnectionManager::GetInstance().GetDevices(dev);

    uint16_t handle = 0;
    int connPriority = 1;
    GattConnectionManager::GetInstance().RequestConnectionPriority(handle, connPriority);

    GattConnectionManager::GetInstance().GetEncryptionInfo(gattDevice3);
    bool autoConnect = false;
    GattConnectionManager::GetInstance().SetConnectionType(gattDevice3, autoConnect);
    GattConnectionManager::GetInstance().GetDeviceState(gattDevice3);
    GattConnectionManager::GetInstance().GetDeviceInformation(gattDevice3);
}

TEST_F(Gatt_Connection_Manager_Test, Connect1)
{
    const std::string addr_ = "0";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0;
    uint8_t transport = 0;
    bluetooth::GattDevice device(addr, type, transport);

    int result = GattConnectionManager::GetInstance().Connect(device);

    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);
}

TEST_F(Gatt_Connection_Manager_Test, Connect2)
{
    const std::string addr_ = "0001";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x1;
    bluetooth::GattDevice device(addr, type, transport);

    int result = GattConnectionManager::GetInstance().Connect(device);

    EXPECT_EQ(result, INTERNAL_ERROR);
}

TEST_F(Gatt_Connection_Manager_Test, Connect4)
{
    const std::string addr_1 = "0001";
    bluetooth::RawAddress addr1(addr_1);
    uint8_t type = 0x0;
    uint8_t transport = 0x1;
    bluetooth::GattDevice gattDevice1(addr1, type, transport);
    const std::string addr_2 = "0002";
    bluetooth::RawAddress addr2(addr_2);
    bluetooth::GattDevice gattDevice2(addr2, type, transport);
    const std::string addr_3 = "0003";
    bluetooth::RawAddress addr3(addr_3);
    bluetooth::GattDevice gattDevice3(addr3, type, transport);
    const std::string addr_4 = "0004";
    bluetooth::RawAddress addr4(addr_4);
    bluetooth::GattDevice gattDevice4(addr4, type, transport);
    const std::string addr_5 = "0005";
    bluetooth::RawAddress addr5(addr_5);
    bluetooth::GattDevice gattDevice5(addr5, type, transport);
    const std::string addr_6 = "0006";
    bluetooth::RawAddress addr6(addr_6);
    bluetooth::GattDevice gattDevice6(addr6, type, transport);
    const std::string addr_7 = "0007";
    bluetooth::RawAddress addr7(addr_7);
    bluetooth::GattDevice gattDevice7(addr7, type, transport);
    const std::string addr_8 = "0008";
    bluetooth::RawAddress addr8(addr_8);
    bluetooth::GattDevice gattDevice8(addr8, type, transport);
    const std::string addr_9 = "0009";
    bluetooth::RawAddress addr9(addr_9);
    bluetooth::GattDevice gattDevice9(addr9, type, transport);
    GattConnectionManager::GetInstance().Connect(gattDevice1);
    GattConnectionManager::GetInstance().Connect(gattDevice2);
    GattConnectionManager::GetInstance().Connect(gattDevice3);
    GattConnectionManager::GetInstance().Connect(gattDevice4);
    GattConnectionManager::GetInstance().Connect(gattDevice5);
    GattConnectionManager::GetInstance().Connect(gattDevice6);
    GattConnectionManager::GetInstance().Connect(gattDevice7);
    GattConnectionManager::GetInstance().Connect(gattDevice8);
    GattConnectionManager::GetInstance().Connect(gattDevice9);
    int result = GattConnectionManager::GetInstance().Connect(gattDevice9);

    EXPECT_EQ(result, MAX_CONNECTIONS);
}

TEST_F(Gatt_Connection_Manager_Test, Disconnect1)
{
    const std::string addr;
    bluetooth::RawAddress addr_(addr);
    uint8_t type = 0;
    uint8_t transport = 0;
    bluetooth::GattDevice device(addr_, type, transport);
    int ret = bluetooth::GattConnectionManager::GetInstance().Disconnect(device);

    EXPECT_EQ(ret, INVALID_PARAMETER);
}

TEST_F(Gatt_Connection_Manager_Test, RegisterObserver)
{
    bluetooth::GattConnectionObserver *observer1 = new bluetooth::GattConnectionObserver();
    bluetooth::GattConnectionObserver *observer2 = new bluetooth::GattConnectionObserver();
    bluetooth::GattConnectionObserver *observer3 = new bluetooth::GattConnectionObserver();
    bluetooth::GattConnectionObserver *observer4 = new bluetooth::GattConnectionObserver();
    std::pair<bool, GattConnectionObserver *> observers_1(false, observer1);
    std::pair<bool, GattConnectionObserver *> observers_2(false, observer2);
    std::pair<bool, GattConnectionObserver *> observers_3(false, observer3);
    std::pair<bool, GattConnectionObserver *> observers_4(false, observer4);

    int result = bluetooth::GattConnectionManager::GetInstance().RegisterObserver(*observer1);
    EXPECT_EQ(2, result);

    bluetooth::GattConnectionManager::GetInstance().RegisterObserver(*observer2);
    bluetooth::GattConnectionManager::GetInstance().RegisterObserver(*observer3);

    int id = bluetooth::GattConnectionManager::GetInstance().RegisterObserver(*observer4);
    EXPECT_EQ(-1, id);

    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(1);
    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(2);
    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(3);
    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(4);
    delete observer1;
    delete observer2;
    delete observer3;
    delete observer4;
}

TEST_F(Gatt_Connection_Manager_Test, DeregisterObserver1)
{
    int registerId = 0;
    static const uint8_t num = 4;  // bluetooth::GattConnectionManager::MAX_OBSERVER_NUM
    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(registerId);
    EXPECT_GT(num, registerId);

    bluetooth::GattConnectionObserver *observer = new bluetooth::GattConnectionObserver();
    std::pair<bool, GattConnectionObserver *> observers_(false, observer);
    bool a = observers_.first;
    EXPECT_EQ(a, false);
    delete observer;
}

TEST_F(Gatt_Connection_Manager_Test, DeregisterObserver2)
{
    int registerId = 4;
    static const uint8_t num = 4;  // bluetooth::GattConnectionManager::MAX_OBSERVER_NUM
    bluetooth::GattConnectionManager::GetInstance().DeregisterObserver(registerId);

    EXPECT_EQ(num, registerId);
}

TEST_F(Gatt_Connection_Manager_Test, GetDeviceState1)
{
    const std::string addr_ = "00000003";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0;
    uint8_t transport = 0x1;
    bluetooth::GattDevice device(addr, type, transport);

    std::string device_ = bluetooth::GattConnectionManager::GetInstance().GetDeviceState(device);
    EXPECT_EQ(bluetooth::GattConnectionManager::Device::STATE_IDLE, device_);
}

TEST_F(Gatt_Connection_Manager_Test, GetMaximumNumberOfConnections)
{
    std::pair<uint16_t, uint16_t> pair1 = GattConnectionManager::GetInstance().GetMaximumNumberOfConnections();
    uint16_t a = 7;
    uint16_t b = 7;
    std::pair<uint16_t, uint16_t> pair2(a, b);

    EXPECT_EQ(pair2, pair1);
}

TEST_F(Gatt_Connection_Manager_Test, shutdown)
{
    int result = bluetooth::GattConnectionManager::GetInstance().ShutDown();
    EXPECT_EQ(result, GATT_SUCCESS);
}

TEST_F(Gatt_Connection_Manager_Test, AttLeConnected)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);
    const BtAddr *addr = NULL;
    uint16_t aclHandle = 10;
    uint8_t role = 0;
    uint8_t status = 0;
    AttLeConnected(addr, aclHandle, role, status);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(Gatt_Connection_Manager_Test, AttLeDisconnected)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);
    uint16_t aclHandle = 10;
    uint8_t status = 0;
    uint8_t reason = 0;
    AttLeDisconnected(aclHandle, status, reason);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(Gatt_Connection_Manager_Test, RequestConnectionPriority)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);
    uint16_t handle = 0x01;
    int connPriority = 1;
    int result = bluetooth::GattConnectionManager::GetInstance().RequestConnectionPriority(handle, connPriority);
    EXPECT_EQ(result, REQUEST_NOT_SUPPORT);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(Gatt_Connection_Manager_Test, GetEncryptionInfo)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);
    const std::string addr_ = "00000001";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x2;
    bluetooth::GattDevice device(addr, type, transport);
    bool result = bluetooth::GattConnectionManager::GetInstance().GetEncryptionInfo(device);
    EXPECT_EQ(result, false);

    gattClientService.Disable();
    sleep(1);
}

TEST_F(Gatt_Connection_Manager_Test, SetConnectionType)
{
    GattClientService gattClientService;
    gattClientService.Enable();
    sleep(1);
    const std::string addr_ = "00000001";
    bluetooth::RawAddress addr(addr_);
    uint8_t type = 0x1;
    uint8_t transport = 0x2;
    bluetooth::GattDevice device(addr, type, transport);
    bool autoConnect = true;
    int result = bluetooth::GattConnectionManager::GetInstance().SetConnectionType(device, autoConnect);
    EXPECT_EQ(result, GATT_FAILURE);

    gattClientService.Disable();
    sleep(1);
}