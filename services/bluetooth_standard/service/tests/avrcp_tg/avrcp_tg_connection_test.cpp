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

#include "avrcp_tg_mock.h"

using namespace testing;
using namespace bluetooth;

class AvrcpTgConnection_test : public testing::Test {
public:
    virtual void SetUp()
    {
        RawAddress bda("0A:00:27:00:00:01");
        uint8_t connectId = 0x05;
        uint8_t role = 0x01;
        uint16_t controlMtu = 0x0050;
        uint16_t browseMtu = 0x0030;
        uint32_t companyId = 0x001958;
        uint16_t uidCounter = 0x00000001;
        AvrcTgConnectManager::GetInstance()->Add(
            bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
    }
    virtual void TearDown()
    {
        RawAddress bda("0A:00:27:00:00:01");
        AvrcTgConnectManager::GetInstance()->Delete(bda);
        AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
    }
};

void callback()
{}

TEST_F(AvrcpTgConnection_test, GetInstance)
{
    auto connManager = AvrcTgConnectManager::GetInstance();

    EXPECT_TRUE(connManager != nullptr);
}

TEST_F(AvrcpTgConnection_test, constGetConnectInfo_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    const AvrcTgConnectInfo *info = AvrcTgConnectManager::GetInstance()->GetConnectInfo(bda);

    EXPECT_TRUE(info != nullptr);
}

TEST_F(AvrcpTgConnection_test, GetRawAddress_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    auto connId = AvrcTgConnectManager::GetInstance()->GetConnectId(bda);
    auto raw = AvrcTgConnectManager::GetInstance()->GetRawAddress(connId);
}

TEST_F(AvrcpTgConnection_test, GetDeviceAddresses_1)
{
    RawAddress bda("0A:00:27:00:00:01");
    AvrcTgConnectManager::GetInstance()->GetConnectId(bda);
    auto addr = AvrcTgConnectManager::GetInstance()->GetDeviceAddresses();
}

TEST_F(AvrcpTgConnection_test, IsConnectInfoEmpty_1)
{
    EXPECT_FALSE(AvrcTgConnectManager::GetInstance()->IsConnectInfoEmpty());
}

TEST_F(AvrcpTgConnection_test, GetConnectId_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    auto connId = AvrcTgConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x05, connId);
}

TEST_F(AvrcpTgConnection_test, GetConnectId_002)
{
    RawAddress bda("0A:00:27:00:00:06");
    auto connId = AvrcTgConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x00, connId);
}

TEST_F(AvrcpTgConnection_test, SetConnectId_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    AvrcTgConnectManager::GetInstance()->SetConnectId(bda, 0x02);
    auto connId = AvrcTgConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x02, connId);
}

TEST_F(AvrcpTgConnection_test, PassPacket_001)
{
    RawAddress bda("0A:00:27:00:00:01");

    auto func = callback;
    AvrcTgConnectManager::GetInstance()->SetPassTimer(bda, func, 200, false);

    std::shared_ptr<AvrcTgPassPacket> packet = std::make_shared<AvrcTgPassPacket>(0x41, 0x01, 0x00);
    AvrcTgConnectManager::GetInstance()->SetPassPacket(bda, packet);

    auto pkt = AvrcTgConnectManager::GetInstance()->GetPassPacket(bda);
    EXPECT_TRUE(pkt != nullptr);

    AvrcTgConnectManager::GetInstance()->ClearPassInfo(bda);
}

TEST_F(AvrcpTgConnection_test, PassPacket_002)
{
    RawAddress bda("0A:00:27:00:00:07");

    auto func = callback;
    AvrcTgConnectManager::GetInstance()->SetPassTimer(bda, func, 200, false);

    std::shared_ptr<AvrcTgPassPacket> packet = std::make_shared<AvrcTgPassPacket>(0x41, 0x01, 0x00);
    AvrcTgConnectManager::GetInstance()->SetPassPacket(bda, packet);

    auto pkt = AvrcTgConnectManager::GetInstance()->GetPassPacket(bda);
    EXPECT_TRUE(pkt == nullptr);

    AvrcTgConnectManager::GetInstance()->ClearPassInfo(bda);
}

TEST_F(AvrcpTgConnection_test, IsPassTimerEmpty_2)
{
    RawAddress bda("0A:00:27:00:00:07");
    AvrcTgConnectManager::GetInstance()->IsPassTimerEmpty(bda);
}

TEST_F(AvrcpTgConnection_test, ClearVendorInfo_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    AvrcTgConnectManager::GetInstance()->ClearVendorInfo(bda);
    auto pkt = AvrcTgConnectManager::GetInstance()->GetVendorPacket(bda);
    EXPECT_TRUE(pkt == nullptr);
}

TEST_F(AvrcpTgConnection_test, ClearVendorInfo_002)
{
    RawAddress bda("0A:00:27:00:00:07");
    AvrcTgConnectManager::GetInstance()->ClearVendorInfo(bda);
    auto pkt = AvrcTgConnectManager::GetInstance()->GetVendorPacket(bda);
    EXPECT_TRUE(pkt == nullptr);
}

TEST_F(AvrcpTgConnection_test, IsConnectInfoEmpty_001)
{
    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
    auto result = AvrcTgConnectManager::GetInstance()->IsConnectInfoEmpty();
    EXPECT_EQ(true, result);
}