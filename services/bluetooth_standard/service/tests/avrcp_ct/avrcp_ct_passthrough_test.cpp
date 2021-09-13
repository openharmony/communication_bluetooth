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

#include "avrcp_ct_test.h"

using namespace testing;
using namespace bluetooth;

TEST(AvrcCtPassPacket_test, AvrcCtPassPacket_001)
{
    AvrcCtPassPacket pkt;
    AvrcCtPassPacket pktPass(0x41, 0x01);

    Packet *pkt_ = PacketMalloc(0x00, 0x00, 5);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt_)));
    ASSERT(buffer != nullptr);

    uint16_t offset = 0x00;
    offset += pkt.PushOctets1((buffer + offset), 0x00);
    offset += pkt.PushOctets1((buffer + offset), 0x09 << 3 | 0x00);
    offset += pkt.PushOctets1((buffer + offset), 0x7C);
    offset += pkt.PushOctets1((buffer + offset), (0x01 << 7) | 0x43);
    offset += pkt.PushOctets1((buffer + offset), 0x00);
    AvrcCtPassPacket pktdis(pkt_);
    pkt.~AvrcCtPassPacket();
}

TEST(AvrcCtPassPacket_test, DeAvrcCtPassPacket_001)
{
    AvrcCtPassPacket pkt;

    pkt.~AvrcCtPassPacket();
}

TEST(AvrcCtPassPacket_test, DisassemblePacket_001)
{
    AvrcCtPassPacket packet;
    Packet *pkt = PacketMalloc(0x00, 0x00, 0x05);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));
    ASSERT(buffer != nullptr);
    uint16_t offset = 0x00;
    offset += packet.PushOctets1((buffer + offset), 0x09);
    offset += packet.PushOctets1((buffer + offset), 0x09 << 3 | 0x00);
    offset += packet.PushOctets1((buffer + offset), 0x7C);
    offset += packet.PushOctets1((buffer + offset), (0x01 << 7) | 0x43);
    offset += packet.PushOctets1((buffer + offset), 0x00);
    bool result = packet.DisassemblePacket(pkt);

    EXPECT_TRUE(result);
    packet.~AvrcCtPassPacket();
}

TEST(AvrcCtPassPacket_test, IsSupportedKeyOperation_001)
{
    AvrcCtPassPacket packet;

    EXPECT_TRUE(!packet.IsSupportedKeyOperation());
    packet.~AvrcCtPassPacket();
}

TEST(AvrcCtPassPacket_test, IsValidKeyState_001)
{
    AvrcCtPassPacket packet;

    EXPECT_TRUE(!packet.IsValidKeyState());
    packet.~AvrcCtPassPacket();
}
