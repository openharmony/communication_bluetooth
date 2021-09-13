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

#include "avrcp_ct_mock.h"

using namespace testing;
using namespace bluetooth;

class AvrcpCtConnection_test : public testing::Test {
public:
    virtual void SetUp()
    {
        RawAddress bda("0A:00:27:00:00:01");
        uint8_t connectId = 0x05;
        uint8_t role = 0x01;
        uint16_t controlMtu = 0x0050;
        uint16_t browseMtu = 0x0030;
        uint32_t companyId = 0x001958;
        uint32_t uidCounter = 0x00000001;
        AvrcCtConnectManager::GetInstance()->Add(
            bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter,false, nullptr, nullptr);
    }
    virtual void TearDown()
    {
        RawAddress bda("0A:00:27:00:00:01");
        AvrcCtConnectManager::GetInstance()->Delete(bda);
        AvrcCtConnectManager::GetInstance()->~AvrcCtConnectManager();
    }
};

void callback()
{}

TEST_F(AvrcpCtConnection_test, GetConnectId_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    auto connId = AvrcCtConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x05, connId);
}

TEST_F(AvrcpCtConnection_test, GetConnectId_002)
{
    RawAddress bda("0A:00:27:00:00:06");
    auto connId = AvrcCtConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x00, connId);
}

TEST_F(AvrcpCtConnection_test, SetConnectId_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    AvrcCtConnectManager::GetInstance()->SetConnectId(bda, 0x02);
    auto connId = AvrcCtConnectManager::GetInstance()->GetConnectId(bda);
    EXPECT_EQ(0x02, connId);
}

TEST_F(AvrcpCtConnection_test, PassPacket_1)
{
    RawAddress bda("0A:00:27:00:00:01");
    auto func = callback;

    AvrcCtConnectManager::GetInstance()->SetPassTimer(bda, func, 200, false);

    std::shared_ptr<AvrcCtPassPacket> packet = std::make_shared<AvrcCtPassPacket>(0x41, 0x01);
    AvrcCtConnectManager::GetInstance()->SetPassPacket(bda, packet);

    auto pkt = AvrcCtConnectManager::GetInstance()->GetPassPacket(bda);
    EXPECT_TRUE(pkt != nullptr);

    AvrcCtConnectManager::GetInstance()->ClearPassInfo(bda);
}

TEST_F(AvrcpCtConnection_test, PassPacket_2)
{
    RawAddress bda("0A:00:27:00:00:07");
    auto func = callback;

    AvrcCtConnectManager::GetInstance()->SetPassTimer(bda, func, 200, false);

    std::shared_ptr<AvrcCtPassPacket> packet = std::make_shared<AvrcCtPassPacket>(0x41, 0x01);
    AvrcCtConnectManager::GetInstance()->SetPassPacket(bda, packet);

    auto pkt = AvrcCtConnectManager::GetInstance()->GetPassPacket(bda);
    EXPECT_TRUE(pkt == nullptr);

    AvrcCtConnectManager::GetInstance()->ClearPassInfo(bda);
}

TEST_F(AvrcpCtConnection_test, PassQueue_1)
{
    RawAddress bda("0A:00:27:00:00:01");
    std::shared_ptr<AvrcCtPassPacket> pktin = std::make_shared<AvrcCtPassPacket>(0x41, 0x01);
    AvrcCtConnectManager::GetInstance()->PushPassQueue(bda, pktin);
    auto size = AvrcCtConnectManager::GetInstance()->GetSizeOfPassQueue(bda);
    EXPECT_EQ(size, 0x01);

    std::shared_ptr<AvrcCtPassPacket> pktout = AvrcCtConnectManager::GetInstance()->PopPassQueue(bda);
    EXPECT_TRUE(pktout != nullptr);

    size = AvrcCtConnectManager::GetInstance()->GetSizeOfPassQueue(bda);
    EXPECT_EQ(size, 0x00);
}

TEST_F(AvrcpCtConnection_test, PassQueue_2)
{
    RawAddress bda("0A:00:27:00:00:07");
    std::shared_ptr<AvrcCtPassPacket> pktin = std::make_shared<AvrcCtPassPacket>(0x41, 0x01);
    AvrcCtConnectManager::GetInstance()->PushPassQueue(bda, pktin);
    auto size = AvrcCtConnectManager::GetInstance()->GetSizeOfPassQueue(bda);
    EXPECT_EQ(size, 0x00);

    std::shared_ptr<AvrcCtPassPacket> pktout = AvrcCtConnectManager::GetInstance()->PopPassQueue(bda);
    EXPECT_TRUE(pktout == nullptr);
}

TEST_F(AvrcpCtConnection_test, GetVendorContinuePacket_1)
{
    RawAddress bda("0A:00:27:00:00:01");
    auto pkt = AvrcCtConnectManager::GetInstance()->GetVendorContinuePacket(bda);
}

TEST_F(AvrcpCtConnection_test, GetVendorContinuePacket_2)
{
    RawAddress bda("0A:00:27:00:00:07");
    auto pkt = AvrcCtConnectManager::GetInstance()->GetVendorContinuePacket(bda);
}

TEST_F(AvrcpCtConnection_test, ClearVendorInfo_001)
{
    RawAddress bda("0A:00:27:00:00:01");
    AvrcCtConnectManager::GetInstance()->ClearVendorInfo(bda);
    auto pkt = AvrcCtConnectManager::GetInstance()->GetVendorPacket(bda);

    EXPECT_TRUE(pkt == nullptr);
}

TEST_F(AvrcpCtConnection_test, VendorQueue_1)
{
    RawAddress bda("0A:00:27:00:00:01");
    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGpsPacket>();
    AvrcCtConnectManager::GetInstance()->PushVendorQueue(bda, packet);
    std::shared_ptr<AvrcCtVendorPacket> pkt = AvrcCtConnectManager::GetInstance()->PopVendorQueue(bda);
}

TEST_F(AvrcpCtConnection_test, VendorQueue_2)
{
    RawAddress bda("0A:00:27:00:00:07");
    std::shared_ptr<AvrcCtVendorPacket> packet = std::make_shared<AvrcCtGpsPacket>();
    AvrcCtConnectManager::GetInstance()->PushVendorQueue(bda, packet);
    std::shared_ptr<AvrcCtVendorPacket> pkt = AvrcCtConnectManager::GetInstance()->PopVendorQueue(bda);
}

TEST_F(AvrcpCtConnection_test, ClearVendorPackets_002)
{
    RawAddress bda("0A:00:27:00:00:07");
    AvrcCtConnectManager::GetInstance()->ClearVendorInfo(bda);
    auto pkt = AvrcCtConnectManager::GetInstance()->GetVendorPacket(bda);
    EXPECT_TRUE(pkt == nullptr);
}
