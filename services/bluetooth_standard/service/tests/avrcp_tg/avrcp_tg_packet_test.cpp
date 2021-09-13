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

TEST(avrcp_tg_packet_test, PushOctets1_001)
{
    uint8_t array[10];
    AvrcTgPassPacket packet;
    packet.PushOctets1(array, 0x01);

    EXPECT_EQ(0x01, array[0]);
}

TEST(avrcp_tg_packet_test, PushOctets2_001)
{
    uint8_t array[10];
    AvrcTgPassPacket packet;
    packet.PushOctets2(array, 0x0101);

    EXPECT_EQ(0x01, array[0]);
    EXPECT_EQ(0x01, array[1]);
}

TEST(avrcp_tg_packet_test, PushOctets3_001)
{
    uint8_t array[10];
    AvrcTgPassPacket packet;
    packet.PushOctets3(array, 0x00010101);

    EXPECT_EQ(0x01, array[0]);
    EXPECT_EQ(0x01, array[1]);
    EXPECT_EQ(0x01, array[2]);
}

TEST(avrcp_tg_packet_test, PushOctets4_001)
{
    uint8_t array[10];
    AvrcTgPassPacket packet;
    packet.PushOctets4(array, 0x01010101);

    EXPECT_EQ(0x01, array[0]);
    EXPECT_EQ(0x01, array[1]);
    EXPECT_EQ(0x01, array[2]);
    EXPECT_EQ(0x01, array[3]);
}

TEST(avrcp_tg_packet_test, PushOctets8_001)
{
    uint8_t array[10];
    AvrcTgPassPacket packet;
    packet.PushOctets8(array, 0x0101010101010101);

    EXPECT_EQ(0x01, array[0]);
    EXPECT_EQ(0x01, array[1]);
    EXPECT_EQ(0x01, array[2]);
    EXPECT_EQ(0x01, array[3]);
    EXPECT_EQ(0x01, array[4]);
    EXPECT_EQ(0x01, array[5]);
    EXPECT_EQ(0x01, array[6]);
    EXPECT_EQ(0x01, array[7]);
}

TEST(avrcp_tg_packet_test, PushOctets_001)
{
    uint8_t array[10];
    uint8_t payload[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    packet.PushOctets(array, payload, 10);

    EXPECT_EQ(0x01, array[0]);
    EXPECT_EQ(0x01, array[1]);
    EXPECT_EQ(0x01, array[2]);
    EXPECT_EQ(0x01, array[3]);
    EXPECT_EQ(0x01, array[4]);
    EXPECT_EQ(0x01, array[5]);
    EXPECT_EQ(0x01, array[6]);
    EXPECT_EQ(0x01, array[7]);
    EXPECT_EQ(0x01, array[8]);
    EXPECT_EQ(0x01, array[9]);
}

TEST(avrcp_tg_packet_test, PopOctets1_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint64_t payload;
    packet.PopOctets1(pkt, payload);
    payload = static_cast<uint8_t>(payload);
    EXPECT_TRUE(payload == 0x01);
}

TEST(avrcp_tg_packet_test, PopOctets2_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint64_t payload;
    packet.PopOctets2(pkt, payload);
    payload = static_cast<uint16_t>(payload);
    EXPECT_TRUE(payload == 0x0101);
}

TEST(avrcp_tg_packet_test, PopOctets3_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint64_t payload;
    packet.PopOctets3(pkt, payload);
    payload = static_cast<uint32_t>(payload);
    EXPECT_TRUE(payload == 0x00010101);
}

TEST(avrcp_tg_packet_test, PopOctets4_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint64_t payload;
    packet.PopOctets4(pkt, payload);
    payload = static_cast<uint32_t>(payload);
    EXPECT_TRUE(payload == 0x01010101);
}

TEST(avrcp_tg_packet_test, PopOctets8_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint64_t payload;
    packet.PopOctets8(pkt, payload);
}

TEST(avrcp_tg_packet_test, PopOctets_001)
{
    uint8_t pkt[10] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    AvrcTgPassPacket packet;
    uint8_t payload[10];
    packet.PopOctets(pkt, payload, 4);

    EXPECT_EQ(payload[0], 0x01);
    EXPECT_EQ(payload[1], 0x01);
    EXPECT_EQ(payload[2], 0x01);
    EXPECT_EQ(payload[3], 0x01);
}

TEST(avrcp_tg_packet_test, GetOpCode_001)
{
    AvrcTgPassPacket packet;
    Packet *pkt = PacketMalloc(0x00, 0x00, 0x08);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));
    ASSERT(buffer != nullptr);
    uint16_t offset = 0x00;
    offset += packet.PushOctets1((buffer + offset), 0x30);
    offset += packet.PushOctets1((buffer + offset), 0xff);
    offset += packet.PushOctets1((buffer + offset), 0x05);
    uint8_t op = packet.GetOpCode(pkt);

    EXPECT_EQ(op, 0x05);
}

TEST(avrcp_tg_packet_test, GetVendorPdu_001)
{
    AvrcTgPassPacket packet;
    Packet *pkt = PacketMalloc(0x00, 0x00, 0x08);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));
    ASSERT(buffer != nullptr);
    uint16_t offset = 0x00;
    offset += packet.PushOctets1((buffer + offset), 0x30);
    offset += packet.PushOctets1((buffer + offset), 0xff);
    offset += packet.PushOctets1((buffer + offset), 0x05);
    offset += packet.PushOctets3((buffer + offset), 0x00001958);
    offset += packet.PushOctets1((buffer + offset), 0x08);
    uint8_t pdu = packet.GetVendorPdu(pkt);

    EXPECT_EQ(pdu, 0x08);
}

TEST(avrcp_tg_packet_test, GetBrowsePdu_001)
{
    AvrcTgPassPacket packet;
    Packet *pkt = PacketMalloc(0x00, 0x00, 0x08);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));
    ASSERT(buffer != nullptr);
    uint16_t offset = 0x00;
    offset += packet.PushOctets1((buffer + offset), 0x06);
    uint8_t pdu = packet.GetBrowsePdu(pkt);

    EXPECT_EQ(pdu, 0x06);
}

TEST(avrcp_tg_packet_test, IsValidOpCode_001)
{
    AvrcTgPassPacket packet;
    bool result = packet.IsValidOpCode(0x30);
    EXPECT_TRUE(result);
    result = packet.IsValidOpCode(0x31);
    EXPECT_TRUE(result);
    result = packet.IsValidOpCode(0x00);
    EXPECT_TRUE(result);
    result = packet.IsValidOpCode(0x7C);
    EXPECT_TRUE(result);
    result = packet.IsValidOpCode(0x23);
    EXPECT_TRUE(!result);
}
