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

#include <random>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "obex_utils.h"
#include "securec.h"

using namespace std;
using namespace testing;
using namespace bluetooth;
TEST(ObexUtils_test, DataReverse)
{
    uint8_t buf[] = {0x00, 0x01, 0x00, 0x02, 0x00, 0x03};
    auto array = std::make_unique<uint8_t[]>(sizeof(buf));
    memset_s(array.get(), sizeof(buf), 0x00, sizeof(buf));
    memcpy_s(array.get(), sizeof(buf), buf, sizeof(buf));
    bluetooth::ObexUtils::DataReverse(array.get(), sizeof(buf), 2);

    std::vector<uint8_t> actualVector(array.get(), array.get() + 6);
    std::vector<uint8_t> expectedVector({0x01, 0x00, 0x02, 0x00, 0x03, 0x00});
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));
}

TEST(ObexUtils_test, SetBufData16)
{
    uint16_t data = 0x0400;  // 1024, BigEndian{0x04, 0x00}, LittleEndian{0x00, 0x04}
    uint8_t seq[2] = {0x00, 0x00};
    ObexUtils::SetBufData16(seq, 0, data);  // 1024, BigEndian{0x04, 0x00}, LittleEndian{0x04, 0x00}
    std::vector<uint8_t> vector(seq, seq + 2);
    EXPECT_THAT(vector, ElementsAreArray({0x04, 0x00}));
}

TEST(ObexUtils_test, SetBufData32)
{
    uint32_t data = 0x12345678;
    uint8_t seq[4] = {0x00, 0x00, 0x00, 0x00};
    ObexUtils::SetBufData32(seq, 0, data);
    std::vector<uint8_t> vector(seq, seq + 4);
    EXPECT_THAT(vector, ElementsAreArray({0x12, 0x34, 0x56, 0x78}));
}

TEST(ObexUtils_test, SetBufData64)
{
    uint64_t data = 0x123456789abcdef0;
    uint8_t seq[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ObexUtils::SetBufData64(seq, 0, data);
    std::vector<uint8_t> vector(seq, seq + 8);
    EXPECT_THAT(vector, ElementsAreArray({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0}));
}

TEST(ObexUtils_test, GetBufData16)
{
    uint16_t data = 1024;
    uint8_t buf[2] = {0x04, 0x00};
    uint16_t length = ObexUtils::GetBufData16(buf, 0);
    EXPECT_THAT(length, 1024);
    EXPECT_THAT(length, data);
}

TEST(ObexUtils_test, GetBufData32)
{
    uint32_t data = 305419896;  // 0x12345678
    uint8_t buf[4] = {0x12, 0x34, 0x56, 0x78};
    uint32_t length = ObexUtils::GetBufData32(buf, 0);
    EXPECT_THAT(length, data);
}

TEST(ObexUtils_test, GetBufData64)
{
    uint64_t data = 1311768467463790320;  // 0x123456789abcdef0
    uint8_t buf[8] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    uint64_t length = ObexUtils::GetBufData64(buf, 0);
    EXPECT_THAT(length, data);
}

TEST(ObexUtils_test, MakeNonce)
{
    std::default_random_engine e(time(0));
    uint32_t privateKey = static_cast<uint32_t>(e());
    auto nonce = ObexUtils::MakeNonce(privateKey);

    EXPECT_THAT(nonce.size(), 16);
}

TEST(ObexUtils_test, MakeRequestDigest)
{
    std::string str = "123abc";
    std::default_random_engine e(time(0));
    uint32_t privateKey = static_cast<uint32_t>(e());
    auto nonce = ObexUtils::MakeNonce(privateKey);

    std::vector<uint8_t> digest = ObexUtils::MakeRequestDigest(nonce.data(), nonce.size(), str);

    EXPECT_THAT(digest.size(), 16);
}

TEST(ObexUtils_test, UnicodeToUtf8)
{
    std::string strUtf8 = "ABCD";
    std::u16string str16 = u"ABCD";
    EXPECT_THAT(ObexUtils::UnicodeToUtf8(str16), strUtf8);
}

TEST(ObexUtils_test, Utf8ToUnicode)
{
    std::string strUtf8 = "ABCD";
    std::u16string str16 = u"ABCD";
    EXPECT_THAT(ObexUtils::Utf8ToUnicode(strUtf8), str16);
}
