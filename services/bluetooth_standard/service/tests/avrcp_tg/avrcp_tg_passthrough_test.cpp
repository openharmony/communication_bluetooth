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

#include "avrcp_tg_test.h"

using namespace testing;
using namespace bluetooth;

TEST(AvrcTgPassPacket_test, AvrcTgPassPacket_001)
{
    AvrcTgPassPacket pkt;
    EXPECT_EQ(0x02, pkt.GetKeyState());
    AvrcTgPassPacket pktPass(0x41, 0x01, 0x00);
}

TEST(AvrcTgPassPacket_test, DeAvrcTgPassPacket_001)
{
    AvrcTgPassPacket pkt;

    pkt.~AvrcTgPassPacket();
}

TEST(AvrcTgPassPacket_test, AssemblePacket_001)
{
    AvrcTgPassPacket pkt;

    pkt.~AvrcTgPassPacket();
}

TEST(AvrcTgPassPacket_test, DisassemblePacket_001)
{
    AvrcTgPassPacket packet;
    Packet *pkt = PacketMalloc(0x00, 0x00, 0x05);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));
    ASSERT(buffer != nullptr);

    uint16_t offset = 0x00;
    offset += packet.PushOctets1((buffer + offset), 0x00);
    offset += packet.PushOctets1((buffer + offset), 0x09 << 3 | 0x00);
    offset += packet.PushOctets1((buffer + offset), 0x7C);
    offset += packet.PushOctets1((buffer + offset), (0x01 << 7) | 0x43);
    offset += packet.PushOctets1((buffer + offset), 0x00);
    bool result = packet.DisassemblePacket(pkt);

    EXPECT_TRUE(result);
}

TEST(AvrcTgPassPacket_test, IsSupportedKeyOperation_001)
{
    AvrcTgPassPacket packet;

    EXPECT_TRUE(!packet.IsSupportedKeyOperation());
}

TEST(AvrcTgPassPacket_test, IsValidKeyState_001)
{
    AvrcTgPassPacket packet;

    EXPECT_TRUE(!packet.IsValidKeyState());
}

TEST(AvrcTgPassPacket_test, IsSupportedKeyOperation_002)
{
    AvrcTgPassPacket packet;

    EXPECT_TRUE(!packet.IsSupportedKeyOperation());
}