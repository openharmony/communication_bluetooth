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

class PowerManagerTest : public testing::Test {
public:
    virtual void SetUp()
    {
        std::cout << "PowerManagerTest SetUP" << std::endl;
        addr = RawAddress::ConvertToString(btaddr.addr);
        registerNewMockBTM(&btmMock_);
        block_ = (BtmPmCallbackTest *)malloc(sizeof(BtmPmCallbackTest));
        registerPmCallback(block_);
        dispatcher_ = std::make_unique<utility::Dispatcher>("PowerManagerTest");
        dispatcher_->Initialize();
        IPowerManager::Initialize(*dispatcher_);
        IPowerManager::GetInstance().Enable();
    }
    virtual void TearDown()
    {
        std::cout << "PowerManagerTest TearDown" << std::endl;
        IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_OFF, PROFILE_NAME_A2DP_SINK, addr);
        usleep(TEST_TIMER_NORMAL);
        IPowerManager::GetInstance().Disable();
        IPowerManager::Uninitialize();
        free(block_);
        block_ = NULL;
        dispatcher_->Uninitialize();
    }

    RawAddress& GetRawAddr()
    {   
        return addr;
    }

    const BtAddr btaddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

private:
    StrictMock<Mock_BTM> btmMock_;
    std::unique_ptr<utility::Dispatcher> dispatcher_ = nullptr;
    BtmPmCallbackTest* block_;
    RawAddress addr;
};

TEST_F(PowerManagerTest, AclCallbacks)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);

    BtmAclConnectCompleteParam param{.status = 0x00,
        .connectionHandle = 0x0001,
        .addr = &btaddr,
        .classOfDevice = {0x01, 0x01, 0x01},
        .encyptionEnabled = true};

    getPmCallback()->aclcallbacks->connectionComplete(&param, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    getPmCallback()->aclcallbacks->disconnectionComplete(0x00, param.connectionHandle, 0x01, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
}

TEST_F(PowerManagerTest, StatusUpdate_1)
{
    EXPECT_EQ(BTPowerMode::MODE_INVALID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_A2DP_SINK, GetRawAddr());

    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
    sleep(TEST_POWER_DELAY_4);

    getPmCallback()->callbacks->setSniffSubratingComplete(0x00, &btaddr, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_2)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 800;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_3)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 600;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    getPmCallback()->callbacks->setSniffSubratingComplete(0x00, &btaddr, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    interval = 150;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_4)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    interval = 100;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_5)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_OFF, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);
}

TEST_F(PowerManagerTest, StatusUpdate_6)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 0;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_7)
{
    EXPECT_EQ(BTPowerMode::MODE_INVALID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_8)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 0;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_9)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_10)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_11)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_12)
{
    // HFP_AG:CONNECT_ON, A2DP_SINK:CONNECT_ON, power mode is LEVEL_LOW
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    sleep(TEST_POWER_DELAY_4);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    // HFP_AG:CONNECT_ON->SCO_ON, A2DP_SINK:CONNECT_ON->IDLE, power mode is LEVEL_MID
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    sleep(TEST_POWER_DELAY_4);

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    // HFP_AG:SCO_ON->IDLE, A2DP_SINK:IDLE->BUSY, power mode is ACTIVE
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    // HFP_AG:IDLE->SCO_ON, A2DP_SINK:BUSY->IDLE, power mode is LEVEL_MID
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    sleep(TEST_POWER_DELAY_4);

    interval = 50;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_MID, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_13)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_SNIFF_LEVEL_LOW, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_HFP_AG, GetRawAddr());
    sleep(TEST_POWER_DELAY_8);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}

TEST_F(PowerManagerTest, StatusUpdate_14)
{
    IPowerManager::GetInstance().StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);

    uint16_t interval = 400;
    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x02, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    getPmCallback()->callbacks->modeChange(0x00, &btaddr, 0x00, interval, getPmCallback()->context);
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    usleep(TEST_TIMER_NORMAL);

    IPowerManager::GetInstance().StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_A2DP_SINK, GetRawAddr());
    sleep(TEST_POWER_DELAY_4);
    EXPECT_EQ(BTPowerMode::MODE_ACTIVE, IPowerManager::GetInstance().GetPowerMode(GetRawAddr()));
}