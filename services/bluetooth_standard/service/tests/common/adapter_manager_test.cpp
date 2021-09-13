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

#include "common_mock.h"
#include "compat.h"

using namespace bluetooth;
using namespace testing;
using namespace utility;

class AdapterManagerTest : public testing::Test {
public:
    virtual void SetUp()
    {
        std::cout << "AdapterManagerTest SetUP" << std::endl;
        registerNewMockAdapterConfig(&instanceMock_);
        registerNewMockProfileConfig(&profileinstanceMock_);
        registerNewMockAdapterDeviceConfig(&adapterdeviceinstanceMock_);
        registerNewMockAdapterDeviceInfo(&adapterinfoinstanceMock_);
        registerNewMockBTM(&btmMock_);
        block_ = (BtmPmCallbackTest *)malloc(sizeof(BtmPmCallbackTest));
        registerPmCallback(block_);

        EXPECT_CALL(instanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&configMock));
        EXPECT_CALL(profileinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&profileMock));
        EXPECT_CALL(adapterdeviceinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&adapterdeviceMock));
        EXPECT_CALL(adapterinfoinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&adapterinfoMock));

        EXPECT_CALL(configMock, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(profileMock, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(adapterdeviceMock, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));
        EXPECT_CALL(adapterinfoMock, Load()).Times(AtLeast(0)).WillRepeatedly(Return(true));

        EXPECT_CALL(configMock, GetValue(_, _, A<bool &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<0x02>(false), Return(false)));

        EXPECT_CALL(GetBtmMock(), BTM_Initialize()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));
        EXPECT_CALL(GetBtmMock(), BTM_Close()).Times(AtLeast(0)).WillRepeatedly(Return(BT_NO_ERROR));

        TestObserver_ = std::make_unique<TestObserver>();
        EXPECT_TRUE(AdapterManager::GetInstance()->RegisterStateObserver(*TestObserver_));
        TestSysObserver_ = std::make_unique<TestSysObserver>();
        EXPECT_TRUE(AdapterManager::GetInstance()->RegisterSystemStateObserver(*TestSysObserver_));
    }

    virtual void TearDown()
    {
        EXPECT_TRUE(AdapterManager::GetInstance()->DeregisterStateObserver(*TestObserver_));
        EXPECT_TRUE(AdapterManager::GetInstance()->DeregisterSystemStateObserver(*TestSysObserver_));
        std::cout << "AdapterManagerTest TearDown" << std::endl;
        free(block_);
        block_ = NULL;
    }

    void SetConfigEnableDisable1();
    void SetConfigEnableDisable2();
    void SetConfigReset();

    StrictMock<AdapterConfigMock>& GetAdapterConfigMock()
    {
        return configMock;
    }

    StrictMock<ProfileConfigMock>& GetProfileConfigMock()
    {
        return profileMock;
    }

    StrictMock<Mock_BTM>& GetBtmMock()
    {
        return btmMock_;
    }

    std::unique_ptr<TestObserver>& GetTestObserver()
    {
        return TestObserver_;
    }

    std::unique_ptr<TestSysObserver>& GetTestSysObserver()
    {
        return TestSysObserver_;
    }

private:
    StrictMock<AdapterConfigInstanceMock> instanceMock_;
    StrictMock<AdapterConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> profileinstanceMock_;
    StrictMock<ProfileConfigMock> profileMock;
    StrictMock<AdapterDeviceConfigInstanceMock> adapterdeviceinstanceMock_;
    StrictMock<AdapterDeviceConfigMock> adapterdeviceMock;
    StrictMock<AdapterDeviceInfoInstanceMock> adapterinfoinstanceMock_;
    StrictMock<AdapterDeviceInfoMock> adapterinfoMock;
    StrictMock<Mock_BTM> btmMock_;
    std::unique_ptr<TestObserver> TestObserver_;
    std::unique_ptr<TestSysObserver> TestSysObserver_;
    BtmPmCallbackTest* block_;
};

TEST_F(AdapterManagerTest, GetInstance)
{
    IAdapterManager *instance = nullptr;
    instance = IAdapterManager::GetInstance();
    EXPECT_NE(nullptr, instance);
}

// Start and Stop BR/EDR->A2DP_SRC and BLE->GATT_CLIENT
TEST_F(AdapterManagerTest, StartStop_normal_1)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    AdapterManager::GetInstance()->Stop();

    EXPECT_TRUE(AdapterManager::GetInstance()->Start());
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_NE(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
    EXPECT_NE(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));

    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0));
    EXPECT_EQ(BTConnectState::DISCONNECTED, AdapterManager::GetInstance()->GetAdapterConnectState());

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
}

// Start and Stop BR/EDR->GATT_CLIENT and BR/EDR->A2DP_SRC
TEST_F(AdapterManagerTest, StartStop_normal_2)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_TRUE(AdapterManager::GetInstance()->Start());
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_NE(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
}

// Start and Stop with no adapter
TEST_F(AdapterManagerTest, StartStop_normal_3)
{
    EXPECT_TRUE(AdapterManager::GetInstance()->Start());
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(nullptr, AdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

// Test the error cases
TEST_F(AdapterManagerTest, Start_error)
{
    EXPECT_CALL(GetAdapterConfigMock(), Load()).WillOnce(Return(false));

    EXPECT_FALSE(AdapterManager::GetInstance()->Start());

    EXPECT_CALL(GetAdapterConfigMock(), Load()).WillOnce(Return(true));
    EXPECT_CALL(GetProfileConfigMock(), Load()).WillOnce(Return(false));

    EXPECT_FALSE(AdapterManager::GetInstance()->Start());

    EXPECT_CALL(GetAdapterConfigMock(), Load()).WillOnce(Return(true));
    EXPECT_CALL(GetProfileConfigMock(), Load()).WillOnce(Return(true));
    EXPECT_CALL(GetBtmMock(), BTM_Initialize()).WillOnce(Return(BT_OPERATION_FAILED));

    EXPECT_FALSE(AdapterManager::GetInstance()->Start());
}

// Test Enable and Disable with BR/EDR->no profile and BLE->no profile
TEST_F(AdapterManagerTest, EnableDisable_normal_1)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the adapter, adapter return false, then disable
    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the adapter, adapter return true, enable successfully
    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable again will do nothing
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the adapter, adapter return false or true, disable successfully
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // disable again will do nothing
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

// Test Enable and Disable with BR/EDR->A2DP_SRC&A2DP_SINK and BLE->GATT_CLIENT&GATT_SERVER
void AdapterManagerTest::SetConfigEnableDisable1()
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SINK, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));    
}

TEST_F(AdapterManagerTest, EnableDisable_normal_2)
{
    SetConfigEnableDisable1();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the adapter, adapter return true, but some profile services return false, then disable
    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SINK)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_CLIENT)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SINK)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_CLIENT)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_OPERATION_FAILED));

    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_normal_3)
{
    SetConfigEnableDisable1();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    // Enable the adapter, adapter return true, all profile services return true, enable successfully
    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SINK)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_CLIENT)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Stop immediately, some profile services return false, disable successfully
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SINK)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_CLIENT)).WillOnce(Return(BT_OPERATION_FAILED));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));
}

// Test Enable and Disable with both BR/EDR&BLE->GATT_CLIENT&GATT_SERVER
void AdapterManagerTest::SetConfigEnableDisable2()
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_CLIENT)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_SERVER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_CLIENT)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_SERVER)).WillRepeatedly(Return(BT_NO_ERROR));
}

TEST_F(AdapterManagerTest, EnableDisable_normal4)
{
    SetConfigEnableDisable2();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    // Enable the classic adapter and ble adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the classic adapter and ble adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the classic adapter, then enable the ble adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the classic adapter, then disable the ble adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_normal5)
{
    SetConfigEnableDisable2();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    // Enable the ble adapter and classic adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the ble adapter and classic adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the ble adapter, then enable the classic adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the ble adapter, then disable the classic adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_normal6)
{
    SetConfigEnableDisable2();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    // Enable the classic adapter firstly
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));

    // Disable the classic adapter, but enable the ble adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the ble adapter successfully
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Enable the ble adapter firstly
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    // Disable the ble adapter, but enable the classic adapter
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));

    // Disable the classic adapter successfully
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_timeout1)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_TIMEOUT));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));

    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURNING_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));

    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_timeout2)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_TIMEOUT));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));

    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    EXPECT_TRUE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_OFF);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));
    EXPECT_EQ(BTStateID::STATE_TURNING_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));

    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_timeout3)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(_)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(_)).WillRepeatedly(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileEnable(_)).WillRepeatedly(Return(BT_TIMEOUT));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(_)).WillRepeatedly(Return(BT_TIMEOUT));

    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_TRUE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURNING_ON);
    EXPECT_EQ(BTStateID::STATE_TURNING_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURNING_ON, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_error1)
{
    AdapterManager::GetInstance()->Start();
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, EnableDisable_error2)
{
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BREDR));
    EXPECT_FALSE(AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE));
    EXPECT_FALSE(AdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE));
}

// Test reset functions
void AdapterManagerTest::SetConfigReset()
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_CLIENT, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillRepeatedly(Return(BT_NO_ERROR));

    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_CLIENT)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SRC)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_CLIENT)).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SRC)).WillRepeatedly(Return(BT_NO_ERROR));
}

TEST_F(AdapterManagerTest, Reset_normal1)
{
    SetConfigReset();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);

    AdapterManager::GetInstance()->Reset();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, Reset_normal2)
{
    SetConfigReset();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURNING_ON);

    AdapterManager::GetInstance()->Reset();

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, Reset_normal3)
{
    SetConfigReset();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURNING_ON);

    AdapterManager::GetInstance()->Reset();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, FactoryReset_normal1)
{
    SetConfigReset();
    EXPECT_FALSE(AdapterManager::GetInstance()->FactoryReset());

    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->FactoryReset());
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->FactoryReset());
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, FactoryReset_normal2)
{
    SetConfigReset();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURNING_ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->FactoryReset());

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, FactoryReset_normal3)
{
    SetConfigReset();
    AdapterManager::GetInstance()->Start();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURNING_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURNING_ON);

    EXPECT_TRUE(AdapterManager::GetInstance()->FactoryReset());
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR));
    EXPECT_EQ(BTStateID::STATE_TURN_OFF, AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE));

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, GetMaxNumConnectedAudioDevices_normal)
{
    int value = 0;
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(5), Return(true)));

    EXPECT_EQ(5, AdapterManager::GetInstance()->GetMaxNumConnectedAudioDevices());
}

TEST_F(AdapterManagerTest, GetMaxNumConnectedAudioDevices_error)
{
    int value = 0;
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(0), Return(false)));

    EXPECT_EQ(0, AdapterManager::GetInstance()->GetMaxNumConnectedAudioDevices());
}

TEST_F(AdapterManagerTest, SetGetPhonebookPermission_normal1)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_ALLOWED;
    bool tmp = true;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), SetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, tmp))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetPhonebookPermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(true), Return(true)));

    EXPECT_EQ(BTPermissionType::ACCESS_ALLOWED, AdapterManager::GetInstance()->GetPhonebookPermission(address));
}

TEST_F(AdapterManagerTest, SetGetPhonebookPermission_normal2)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_UNKNOWN;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), RemoveProperty(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetPhonebookPermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(true), Return(false)));

    EXPECT_EQ(BTPermissionType::ACCESS_UNKNOWN, AdapterManager::GetInstance()->GetPhonebookPermission(address));
}

TEST_F(AdapterManagerTest, SetGetPhonebookPermission_normal3)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_FORBIDDEN;
    bool tmp = false;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), SetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, tmp))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetPhonebookPermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(false), Return(true)));

    EXPECT_EQ(BTPermissionType::ACCESS_FORBIDDEN, AdapterManager::GetInstance()->GetPhonebookPermission(address));
}

TEST_F(AdapterManagerTest, SetGetMessagePermission_normal1)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_ALLOWED;
    bool tmp = true;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), SetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, tmp))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetMessagePermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(true), Return(true)));

    EXPECT_EQ(BTPermissionType::ACCESS_ALLOWED, AdapterManager::GetInstance()->GetMessagePermission(address));
}

TEST_F(AdapterManagerTest, SetGetMessagePermission_normal2)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_UNKNOWN;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), RemoveProperty(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetMessagePermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(true), Return(false)));

    EXPECT_EQ(BTPermissionType::ACCESS_UNKNOWN, AdapterManager::GetInstance()->GetMessagePermission(address));
}

TEST_F(AdapterManagerTest, SetGetMessagePermission_normal3)
{
    std::string address = "00:00:00:00";
    BTPermissionType permission = BTPermissionType::ACCESS_FORBIDDEN;
    bool tmp = false;
    bool value = 0;

    EXPECT_CALL(GetProfileConfigMock(), SetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, tmp))
        .WillOnce(Return(true));

    EXPECT_TRUE(AdapterManager::GetInstance()->SetMessagePermission(address, permission));

    EXPECT_CALL(GetProfileConfigMock(), GetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, value))
        .WillOnce(DoAll(SetArgReferee<3>(false), Return(true)));

    EXPECT_EQ(BTPermissionType::ACCESS_FORBIDDEN, AdapterManager::GetInstance()->GetMessagePermission(address));
}

TEST_F(AdapterManagerTest, OnPairDevicesRemoved)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
        .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

    EXPECT_TRUE(AdapterManager::GetInstance()->Start());
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::ON);

    std::vector<RawAddress> deviceList;
    deviceList.push_back(RawAddress("00:00:00:00:00:00"));
    deviceList.push_back(RawAddress("00:00:00:00:00:11"));
    deviceList.push_back(RawAddress("00:00:00:00:00:22"));

    EXPECT_CALL(GetProfileConfigMock(), RemoveAddr(_)).WillRepeatedly(Return(true));
    AdapterManager::GetInstance()->OnPairDevicesRemoved(BTTransport::ADAPTER_BREDR, deviceList);
    AdapterManager::GetInstance()->OnPairDevicesRemoved(BTTransport::ADAPTER_BLE, deviceList);
    usleep(TEST_TIMER_NORMAL);

    AdapterManager::GetInstance()->Stop();
    GetTestSysObserver()->WaitSysStateChange((int)BTSystemState::OFF);
}

TEST_F(AdapterManagerTest, CompatCheck)
{
    EXPECT_TRUE(Compat::CompatCheck(CompatType::COMPAT_DISABLE_BLE_SECURE_CONNECTIONS, "08:62:66:00:11:22"));
    EXPECT_FALSE(Compat::CompatCheck(CompatType::COMPAT_REJECT_NAME_REQUEST, "08:62:66:00:11:22"));
    EXPECT_FALSE(Compat::CompatCheck(CompatType::COMPAT_REJECT_ABSOLUTE_VOLUME, "A0:E9:D"));
    EXPECT_TRUE(Compat::CompatCheck(CompatType::COMPAT_REJECT_AUTO_PAIRING, "Parrot"));
    EXPECT_TRUE(Compat::CompatCheck(CompatType::COMPAT_REJECT_AVDTP_RECONFIGURE, "KMM-BT51*HD 1"));
    EXPECT_FALSE(Compat::CompatCheck(CompatType::COMPAT_GATTC_NO_SERVICE_CHANGED_IND, "Pixel C Keyboar"));
    EXPECT_FALSE(Compat::CompatCheck(CompatType::COMPAT_KEYBOARD_REQUIRES_FIXED_PIN, "000FF6"));
    EXPECT_FALSE(Compat::CompatCheck(CompatType::COMPAT_KEYBOARD_REQUIRES_FIXED_PIN, ""));
    EXPECT_FALSE(Compat::CompatCheck((CompatType)8, ""));
}