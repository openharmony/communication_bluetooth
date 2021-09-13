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

using namespace bluetooth;
using namespace testing;
using namespace utility;

class ProfileServiceManagerTest : public testing::Test {
public:
    virtual void SetUp()
    {
        std::cout << "ProfileServiceManagerTest SetUP" << std::endl;
        registerNewMockAdapterConfig(&instanceMock_);
        registerNewMockProfileConfig(&profileinstanceMock_);
        registerNewMockBTM(&btmMock_);
        block_ = (BtmPmCallbackTest *)malloc(sizeof(BtmPmCallbackTest));
        registerPmCallback(block_);

        bool value = false;

        EXPECT_CALL(instanceMock_, GetInstance()).WillRepeatedly(Return(&configMock));
        EXPECT_CALL(profileinstanceMock_, GetInstance()).WillRepeatedly(Return(&profileMock));

        EXPECT_CALL(configMock, Load()).WillRepeatedly(Return(true));
        EXPECT_CALL(profileMock, Load()).WillRepeatedly(Return(true));

        EXPECT_CALL(configMock, GetValue(_, _, A<bool &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<0x02>(false), Return(false)));

        EXPECT_CALL(configMock, GetValue(SECTION_CLASSIC_ADAPTER, PROPERTY_IS_VALID, value))
            .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));
        EXPECT_CALL(configMock, GetValue(SECTION_BLE_ADAPTER, PROPERTY_IS_VALID, value))
            .WillOnce(DoAll(SetArgReferee<0x02>(true), Return(true)));

        EXPECT_CALL(btmMock_, BTM_Initialize()).WillOnce(Return(BT_NO_ERROR));

        TestObserver_ = std::make_unique<TestObserver>();
        EXPECT_TRUE(AdapterManager::GetInstance()->RegisterStateObserver(*TestObserver_));
        TestSysObserver_ = std::make_unique<TestSysObserver>();
        EXPECT_TRUE(AdapterManager::GetInstance()->RegisterSystemStateObserver(*TestSysObserver_));

        AdapterManager::GetInstance()->Start();
        TestSysObserver_->WaitSysStateChange((int)BTSystemState::ON);
    }

    virtual void TearDown()
    {
        EXPECT_CALL(btmMock_, BTM_Close()).WillOnce(Return(BT_NO_ERROR));

        AdapterManager::GetInstance()->Stop();
        TestSysObserver_->WaitSysStateChange((int)BTSystemState::OFF);

        EXPECT_TRUE(AdapterManager::GetInstance()->DeregisterStateObserver(*TestObserver_));
        EXPECT_TRUE(AdapterManager::GetInstance()->DeregisterSystemStateObserver(*TestSysObserver_));
        free(block_);
        block_ = NULL;
        std::cout << "ProfileServiceManagerTest TearDown" << std::endl;
    }

    StrictMock<AdapterConfigMock>& GetAdapterConfigMock()
    {
        return configMock;
    }

    StrictMock<Mock_BTM>& GetBtmMock()
    {
        return btmMock_;
    }

    std::unique_ptr<TestObserver>& GetTestObserver()
    {
        return TestObserver_;
    }

private:
    StrictMock<AdapterConfigInstanceMock> instanceMock_;
    StrictMock<AdapterConfigMock> configMock;
    StrictMock<ProfileConfigInstanceMock> profileinstanceMock_;
    StrictMock<ProfileConfigMock> profileMock;
    StrictMock<Mock_BTM> btmMock_;
    std::unique_ptr<TestObserver> TestObserver_;
    std::unique_ptr<TestSysObserver> TestSysObserver_;
    BtmPmCallbackTest* block_;
};

TEST_F(ProfileServiceManagerTest, GetInstance)
{
    IProfileManager *instance = nullptr;
    instance = IProfileManager::GetInstance();
    EXPECT_NE(nullptr, instance);
}

TEST_F(ProfileServiceManagerTest, StartStop_normal)
{
    bool value = false;

    EXPECT_EQ(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    EXPECT_EQ(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_SERVER));

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    // Test Start function
    ProfileServiceManager::GetInstance()->Start();
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    EXPECT_NE(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_SERVER));

    EXPECT_CALL(GetBtmMock(), BTM_Enable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Enable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileEnable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_NO_ERROR));
    AdapterManager::GetInstance()->Enable(ADAPTER_BREDR);
    AdapterManager::GetInstance()->Enable(ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_ON);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_ON);

    // Test GetProfileServicesSupportedUuids function
    std::vector<std::string> uuids;
    ProfileServiceManager::GetInstance()->GetProfileServicesSupportedUuids(uuids);
    EXPECT_STREQ(BLUETOOTH_UUID_GATT.c_str(), uuids[0].c_str());
    EXPECT_STREQ(BLUETOOTH_UUID_A2DP_SRC.c_str(), uuids[1].c_str());

    EXPECT_CALL(GetBtmMock(), BTM_Disable(BREDR_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), BTM_Disable(LE_CONTROLLER)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_A2DP_SRC)).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetBtmMock(), MockProfileDisable(PROFILE_ID_GATT_SERVER)).WillOnce(Return(BT_NO_ERROR));
    AdapterManager::GetInstance()->Disable(ADAPTER_BREDR);
    AdapterManager::GetInstance()->Disable(ADAPTER_BLE);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BREDR, BTStateID::STATE_TURN_OFF);
    GetTestObserver()->WaitStateChange(BTTransport::ADAPTER_BLE, BTStateID::STATE_TURN_OFF);

    // Test Stop function
    ProfileServiceManager::GetInstance()->Stop();
    EXPECT_EQ(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    EXPECT_EQ(nullptr, ProfileServiceManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_SERVER));
    EXPECT_EQ(0, (int)ProfileServiceManager::GetInstance()->GetProfileServicesList().size());
}

TEST_F(ProfileServiceManagerTest, GetProfileServicesList)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_BLE_ADAPTER, PROFILE_NAME_GATT_SERVER, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    ProfileServiceManager::GetInstance()->Start();

    std::vector<uint32_t> profileList = ProfileServiceManager::GetInstance()->GetProfileServicesList();
    EXPECT_EQ(PROFILE_ID_GATT_SERVER, profileList[0]);
    EXPECT_EQ(PROFILE_ID_A2DP_SRC, profileList[1]);

    ProfileServiceManager::GetInstance()->Stop();
}

TEST_F(ProfileServiceManagerTest, GetProfileServiceConnectState)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SINK, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    ProfileServiceManager::GetInstance()->Start();

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b1111));
    EXPECT_EQ(BTConnectState::CONNECTED,
        ProfileServiceManager::GetInstance()->GetProfileServiceConnectState(PROFILE_ID_A2DP_SRC));

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0111));
    EXPECT_EQ(BTConnectState::CONNECTING,
        ProfileServiceManager::GetInstance()->GetProfileServiceConnectState(PROFILE_ID_A2DP_SINK));

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b0011));
    EXPECT_EQ(BTConnectState::DISCONNECTING,
        ProfileServiceManager::GetInstance()->GetProfileServiceConnectState(PROFILE_ID_A2DP_SRC));

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0000));
    EXPECT_EQ(BTConnectState::DISCONNECTED,
        ProfileServiceManager::GetInstance()->GetProfileServiceConnectState(PROFILE_ID_A2DP_SINK));

    EXPECT_EQ(BTConnectState::DISCONNECTED,
        ProfileServiceManager::GetInstance()->GetProfileServiceConnectState(PROFILE_ID_GATT_CLIENT));

    ProfileServiceManager::GetInstance()->Stop();
}

TEST_F(ProfileServiceManagerTest, GetProfileServicesConnectState)
{
    bool value = false;

    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SRC, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));
    EXPECT_CALL(GetAdapterConfigMock(), GetValue(SECTION_CLASSIC_ADAPTER, PROFILE_NAME_A2DP_SINK, value))
        .WillRepeatedly(DoAll(SetArgReferee<0x02>(true), Return(true)));

    ProfileServiceManager::GetInstance()->Start();

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b1111));
    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0111));
    EXPECT_EQ(BTConnectState::CONNECTED, ProfileServiceManager::GetInstance()->GetProfileServicesConnectState());

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b0011));
    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0111));
    EXPECT_EQ(BTConnectState::CONNECTING, ProfileServiceManager::GetInstance()->GetProfileServicesConnectState());

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b0011));
    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0001));
    EXPECT_EQ(BTConnectState::DISCONNECTING, ProfileServiceManager::GetInstance()->GetProfileServicesConnectState());

    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SRC)).WillOnce(Return(0b0001));
    EXPECT_CALL(GetBtmMock(), MockGetConnectState(PROFILE_ID_A2DP_SINK)).WillOnce(Return(0b0000));
    EXPECT_EQ(BTConnectState::DISCONNECTED, ProfileServiceManager::GetInstance()->GetProfileServicesConnectState());

    ProfileServiceManager::GetInstance()->Stop();
}
