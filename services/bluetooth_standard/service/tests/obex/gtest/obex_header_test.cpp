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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "obex_headers.h"
#include "obex_utils.h"

using namespace testing;
using namespace bluetooth;

const uint8_t *TARGET = (const uint8_t *)"TARGET0123456789";
const uint16_t TARGET_LEN = 16;
const uint8_t *NONCE = (const uint8_t *)"NONCE1234567890A";
const uint8_t NONCE_LEN = 16;
const uint8_t *REALM = (const uint8_t *)"REALM";
const uint8_t REALM_LEN = 5;
const uint8_t *REQUEST_DIGEST = (const uint8_t *)"REQUEST_DIGEST";
const uint8_t REQUEST_DIGEST_LEN = 14;
const uint8_t *USER_ID = (const uint8_t *)"userid";
const uint8_t USER_ID_LEN = 6;

void AddData2List(std::vector<uint8_t> &vector, uint16_t data)
{
    uint8_t seq[2] = {0x00, 0x00};
    ObexUtils::SetBufData16(seq, 0, data);
    vector.insert(std::end(vector), seq, seq + 2);
}

void AddData2List(std::vector<uint8_t> &vector, uint32_t data)
{
    uint8_t seq[4] = {0x00, 0x00, 0x00, 0x00};
    ObexUtils::SetBufData32(seq, 0, data);
    vector.insert(std::end(vector), seq, seq + 4);
}

void AddData2List(std::vector<uint8_t> &vector, uint64_t data)
{
    uint8_t seq[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ObexUtils::SetBufData64(seq, 0, data);
    vector.insert(std::end(vector), seq, seq + 8);
}

void SetData2List(std::vector<uint8_t> &vector, int pos, uint16_t data)
{
    ObexUtils::SetBufData16(vector.data(), pos, data);
}

typedef void (*ObexHeaderAddItemFun)(ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen);

void HeaderAppendItem(ObexHeaderAddItemFun addFunction)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::PUT);
    uint16_t expectedLen = 3;
    expectedVector.insert(std::end(expectedVector), {0x02, 0x00, 0x00});

    addFunction(header.get(), expectedVector, expectedLen);

    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();

    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseResponse(obexPacket->GetBuffer(), obexPacket->GetSize(), false);
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
}

void GetParameterParseTlvtriplet(const TlvTriplet *actualParameterPtr, std::vector<uint8_t> expectedParameterVector)
{
    EXPECT_NE(actualParameterPtr, nullptr);
    EXPECT_THAT(actualParameterPtr->GetLen(), expectedParameterVector.size());
    std::vector<uint8_t> actualParameterVector(
        actualParameterPtr->GetVal(), actualParameterPtr->GetVal() + actualParameterPtr->GetLen());
    EXPECT_THAT(actualParameterVector, ElementsAreArray(expectedParameterVector));
}

TEST(ObexHeader_test, CreateParseRequest_CONNECT)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::CONNECT);
    uint16_t expectedLen = 7;
    EXPECT_THAT(header->GetFieldCode(), 0x80);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(*header->GetFieldObexVersionNum(), bluetooth::OBEX_VERSION);
    EXPECT_THAT(*header->GetFieldFlags(), 0x00);
    EXPECT_THAT(*header->GetFieldMaxPacketLength(), bluetooth::OBEX_DEFAULT_MTU);
    EXPECT_THAT(header->GetFieldConstants(), nullptr);
    expectedVector.insert(std::end(expectedVector), {0x80, 0x00, 0x07, 0x10, 0x00, 0x04, 0x00});

    header->AppendItemSessionSeqNum(0x01);
    expectedLen += (1 + 1);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x93, 0x01});

    header->AppendItemTarget(TARGET, TARGET_LEN);
    expectedLen += (3 + TARGET_LEN);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x46, 0x00, 0x13});
    expectedVector.insert(std::end(expectedVector), TARGET, TARGET + TARGET_LEN);
    header->AppendItemCount(3);
    expectedLen += (1 + 4);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0xC0});
    AddData2List(expectedVector, (uint32_t)3);

    // Auth. Challenge
    uint16_t subStartPos = expectedVector.size();
    expectedVector.insert(std::end(expectedVector), {0x4D, 0x00, 0x00});
    uint16_t beforeLen = expectedLen;
    ObexDigestChallenge authChallenges;
    expectedLen += 3;  // header + len
    authChallenges.AppendNonce(NONCE, NONCE_LEN);
    expectedLen += (2 + NONCE_LEN);
    expectedVector.insert(std::end(expectedVector), {0x00, NONCE_LEN});
    expectedVector.insert(std::end(expectedVector), NONCE, NONCE + NONCE_LEN);

    authChallenges.AppendOptions(uint8_t(0x00));
    expectedLen += (2 + 1);
    expectedVector.insert(std::end(expectedVector), {0x01, 0x01, 0x00});

    authChallenges.AppendRealm(REALM, REALM_LEN);
    expectedLen += (2 + REALM_LEN);
    expectedVector.insert(std::end(expectedVector), {0x02, REALM_LEN});
    expectedVector.insert(std::end(expectedVector), REALM, REALM + REALM_LEN);

    header->AppendItemAuthChallenges(authChallenges);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

    // Auth. Response
    subStartPos = expectedVector.size();
    expectedVector.insert(std::end(expectedVector), {0x4E, 0x00, 0x00});
    beforeLen = expectedLen;
    ObexDigestResponse digestResponse;
    expectedLen += 3;  // header + len
    digestResponse.AppendRequestDigest(REQUEST_DIGEST, REQUEST_DIGEST_LEN);
    expectedLen += (2 + REQUEST_DIGEST_LEN);
    expectedVector.insert(std::end(expectedVector), {0x00, REQUEST_DIGEST_LEN});
    expectedVector.insert(std::end(expectedVector), REQUEST_DIGEST, REQUEST_DIGEST + REQUEST_DIGEST_LEN);

    digestResponse.AppendUserId(USER_ID, USER_ID_LEN);
    expectedLen += (2 + USER_ID_LEN);
    expectedVector.insert(std::end(expectedVector), {0x01, USER_ID_LEN});
    expectedVector.insert(std::end(expectedVector), USER_ID, USER_ID + USER_ID_LEN);

    digestResponse.AppendNonce(NONCE, NONCE_LEN);
    expectedLen += (2 + NONCE_LEN);
    expectedVector.insert(std::end(expectedVector), {0x02, NONCE_LEN});
    expectedVector.insert(std::end(expectedVector), NONCE, NONCE + NONCE_LEN);

    header->AppendItemAuthResponse(digestResponse);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

    // set expected packet len
    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();

    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseRequest(obexPacket->GetBuffer(), obexPacket->GetSize());
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));

    // ObexHeader Copy true
    ObexHeader obexHeaderTrue((const ObexHeader)*header, true);
    EXPECT_THAT(obexHeaderTrue.GetFieldCode(), 0x80);
    EXPECT_THAT(obexHeaderTrue.GetFieldPacketLength(), 7);
    EXPECT_THAT(*obexHeaderTrue.GetFieldObexVersionNum(), bluetooth::OBEX_VERSION);
    EXPECT_THAT(*obexHeaderTrue.GetFieldFlags(), 0x00);
    EXPECT_THAT(*obexHeaderTrue.GetFieldMaxPacketLength(), bluetooth::OBEX_DEFAULT_MTU);
    EXPECT_THAT(obexHeaderTrue.GetFieldConstants(), nullptr);
    // ObexHeader Copy false
    ObexHeader obexHeaderFalse((const ObexHeader)*header, false);
    EXPECT_THAT(obexHeaderFalse.GetFieldCode(), 0x80);
    EXPECT_THAT(obexHeaderFalse.GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(*obexHeaderFalse.GetFieldObexVersionNum(), bluetooth::OBEX_VERSION);
    EXPECT_THAT(*obexHeaderFalse.GetFieldFlags(), 0x00);
    EXPECT_THAT(*obexHeaderFalse.GetFieldMaxPacketLength(), bluetooth::OBEX_DEFAULT_MTU);
    EXPECT_THAT(obexHeaderFalse.GetFieldConstants(), nullptr);

    ObexUtils::ObexHeaderDebug(*header);
}

TEST(ObexHeader_test, CreateParseRequest_DISCONNECT)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::DISCONNECT);

    uint16_t expectedLen = 3;
    EXPECT_THAT(header->GetFieldCode(), 0x81);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x81, 0x00, 0x03});

    header->AppendItemSessionSeqNum(0x01);
    expectedLen += (1 + 1);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x93, 0x01});

    header->AppendItemConnectionId(1234);
    expectedLen += (1 + 4);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0xCB});
    AddData2List(expectedVector, (uint32_t)1234);

    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();
    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseRequest(obexPacket->GetBuffer(), obexPacket->GetSize());
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
}

TEST(ObexHeader_test, CreateParseRequest_SETPATH)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::SETPATH);

    uint16_t expectedLen = 5;
    EXPECT_THAT(header->GetFieldCode(), 0x85);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(*header->GetFieldFlags(), 0x00);
    EXPECT_THAT(*header->GetFieldConstants(), 0x00);
    expectedVector.insert(std::end(expectedVector), {0x85, 0x00, 0x03, 0x00, 0x00});

    std::u16string path = u"";
    header->AppendItemName(path);
    expectedLen += (3 + 0);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(header->GetItemName()->GetUnicodeText(), path);
    expectedVector.insert(std::end(expectedVector), {0x01, 0x00, 0x03});
    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();
    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseRequest(obexPacket->GetBuffer(), obexPacket->GetSize());
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));

    // ObexHeader Copy true
    ObexHeader obexHeaderTrue((const ObexHeader)*header, true);
    EXPECT_THAT(obexHeaderTrue.GetFieldCode(), 0x85);
    EXPECT_THAT(obexHeaderTrue.GetFieldPacketLength(), 5);
    EXPECT_THAT(obexHeaderTrue.GetFieldObexVersionNum(), nullptr);
    EXPECT_THAT(*obexHeaderTrue.GetFieldFlags(), 0x00);
    EXPECT_THAT(obexHeaderTrue.GetFieldMaxPacketLength(), nullptr);
    EXPECT_THAT(*obexHeaderTrue.GetFieldConstants(), 0x00);
    // ObexHeader Copy false
    ObexHeader obexHeaderFalse((const ObexHeader)*header, false);
    EXPECT_THAT(obexHeaderFalse.GetFieldCode(), 0x85);
    EXPECT_THAT(obexHeaderFalse.GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(obexHeaderFalse.GetFieldObexVersionNum(), nullptr);
    EXPECT_THAT(*obexHeaderFalse.GetFieldFlags(), 0x00);
    EXPECT_THAT(obexHeaderFalse.GetFieldMaxPacketLength(), nullptr);
    EXPECT_THAT(*obexHeaderFalse.GetFieldConstants(), 0x00);
    ObexUtils::ObexHeaderDebug(*header);
}

TEST(ObexHeader_test, CreateParseRequest_PUT)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::PUT);

    // SetFinalBit()
    header->SetFinalBit(true);
    EXPECT_THAT(header->GetFieldCode(), 0x82);
    header->SetFinalBit(false);

    uint16_t expectedLen = 3;
    EXPECT_THAT(header->GetFieldCode(), 0x02);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x02, 0x00, 0x03});

    header->AppendItemConnectionId(6144);
    expectedLen += (1 + 4);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0xCB});
    AddData2List(expectedVector, (uint32_t)6144);

    std::string type = "1234567890";
    header->AppendItemType(type);
    expectedLen += (3 + 10 + 1);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x42});
    AddData2List(expectedVector, (uint16_t)(10 + 1 + 3));
    expectedVector.insert(std::end(expectedVector), type.c_str(), type.c_str() + 11);

    uint8_t body[498];
    for (int i = 0; i < 498; i++) {
        body[i] = i % 256;
    }

    header->AppendItemBody(body, 498);
    expectedLen += (3 + 498);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    expectedVector.insert(std::end(expectedVector), {0x48});
    AddData2List(expectedVector, (uint16_t)(3 + 498));
    expectedVector.insert(std::end(expectedVector), body, body + sizeof(body));

    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();
    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseRequest(obexPacket->GetBuffer(), obexPacket->GetSize());
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
}

TEST(ObexHeader_test, CreateParseResponse_CONNECT)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, true);
    uint16_t expectedLen = 7;
    EXPECT_THAT(header->GetFieldCode(), 0xA0);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(*header->GetFieldObexVersionNum(), bluetooth::OBEX_VERSION);
    EXPECT_THAT(*header->GetFieldFlags(), 0x00);
    EXPECT_THAT(*header->GetFieldMaxPacketLength(), bluetooth::OBEX_DEFAULT_MTU);
    EXPECT_THAT(header->GetFieldConstants(), nullptr);
    expectedVector.insert(std::end(expectedVector), {0xA0, 0x00, 0x07, 0x10, 0x00, 0x04, 0x00});

    auto obexPacket = header->Build();

    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseResponse(obexPacket->GetBuffer(), obexPacket->GetSize(), true);
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
}

TEST(ObexHeader_test, CreateParseResponse_OTHER)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateResponse(ObexRspCode::SUCCESS, false);
    uint16_t expectedLen = 3;
    EXPECT_THAT(header->GetFieldCode(), 0xA0);
    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    EXPECT_THAT(header->GetFieldObexVersionNum(), nullptr);
    EXPECT_THAT(header->GetFieldFlags(), nullptr);
    EXPECT_THAT(header->GetFieldMaxPacketLength(), nullptr);
    EXPECT_THAT(header->GetFieldConstants(), nullptr);
    expectedVector.insert(std::end(expectedVector), {0xA0, 0x00, 0x03});

    auto obexPacket = header->Build();

    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseResponse(obexPacket->GetBuffer(), obexPacket->GetSize(), false);
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
}

// bytes header
TEST(ObexHeader_test, AppendItemTarget)
{
    // HN:Target HI:0x46
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "1234567890123456";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x46;
        header->AppendItemTarget((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemTarget
        EXPECT_NE(header->GetItemTarget(), nullptr);
        EXPECT_NE(header->GetItemTarget()->GetBytes(), nullptr);
        EXPECT_THAT(header->GetItemTarget()->GetHeaderClassType(), ObexHeaderDataType::BYTES);
        EXPECT_THAT(header->GetItemTarget()->GetHeaderClassTypeName(), "ObexOptionalBytesHeader");
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemTarget()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemTarget()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));

        // Clone
        std::unique_ptr<ObexOptionalHeader> cloneItemTarget = header->GetItemTarget()->Clone();
        EXPECT_NE(cloneItemTarget, nullptr);
        EXPECT_THAT(cloneItemTarget->GetHeaderId(), 0x46);
        EXPECT_THAT(header->GetItemTarget()->GetHeaderId(), 0x46);
        EXPECT_THAT(cloneItemTarget->GetHeaderDataSize(), dataLen);
        std::unique_ptr<uint8_t[]> actualClonePtr = cloneItemTarget->GetBytes();
        std::vector<uint8_t> actualCloneVector(
            actualClonePtr.get(), actualClonePtr.get() + cloneItemTarget->GetHeaderDataSize());
        EXPECT_THAT(actualCloneVector, ElementsAreArray(actualItemVector));
    });
}

TEST(ObexHeader_test, AppendItemTimeIso8601)
{
    // HN:TimeIso8601 HI:0x44
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemTimeIso8601";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x44;
        header->AppendItemTimeIso8601((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemTimeIso8601
        EXPECT_NE(header->GetItemTimeIso8601(), nullptr);
        EXPECT_NE(header->GetItemTimeIso8601()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemTimeIso8601()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemTimeIso8601()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

TEST(ObexHeader_test, AppendItemHttp)
{
    // HN:Http HI:0x47
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemHttp";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x47;
        header->AppendItemHttp((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemHttp
        EXPECT_NE(header->GetItemHttp(), nullptr);
        EXPECT_NE(header->GetItemHttp()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemHttp()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemHttp()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

TEST(ObexHeader_test, AppendItemBody)
{
    // HN:Body HI:0x48
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemBody";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x48;
        header->AppendItemBody((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemBody
        EXPECT_NE(header->GetItemBody(), nullptr);
        EXPECT_NE(header->GetItemBody()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemBody()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemBody()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

TEST(ObexHeader_test, AppendItemEndBody)
{
    // HN:EndBody HI:0x49
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemEndBody";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x49;
        header->AppendItemEndBody((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemEndBody
        EXPECT_NE(header->GetItemEndBody(), nullptr);
        EXPECT_NE(header->GetItemEndBody()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemEndBody()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemEndBody()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

TEST(ObexHeader_test, AppendItemWho)
{
    // HN:Who HI:0x4A
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemWho";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x4A;
        header->AppendItemWho((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemWho
        EXPECT_NE(header->GetItemWho(), nullptr);
        EXPECT_NE(header->GetItemWho()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemWho()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemWho()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

TEST(ObexHeader_test, AppendItemObjectClass)
{
    // HN:ObjectClass HI:0x51
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "AppendItemObjectClass";
        int dataLen = strlen(data);  // has not '\0'
        uint8_t tagId = 0x51;
        header->AppendItemObjectClass((const uint8_t *)data, dataLen);

        expectedLen += (3 + dataLen);
        expectedVector.insert(std::end(expectedVector), {tagId});
        AddData2List(expectedVector, (uint16_t)(dataLen + 3));
        expectedVector.insert(std::end(expectedVector), (const uint8_t *)data, (const uint8_t *)data + dataLen);

        // GetItemObjectClass
        EXPECT_NE(header->GetItemObjectClass(), nullptr);
        EXPECT_NE(header->GetItemObjectClass()->GetBytes(), nullptr);
        std::unique_ptr<uint8_t[]> actualItemPtr = header->GetItemObjectClass()->GetBytes();
        std::vector<uint8_t> actualItemVector(
            actualItemPtr.get(), actualItemPtr.get() + header->GetItemObjectClass()->GetHeaderDataSize());
        std::vector<uint8_t> expectedItemVector((const uint8_t *)data, (const uint8_t *)data + dataLen);
        EXPECT_THAT(actualItemVector, ElementsAreArray(expectedItemVector));
    });
}

// tlv header
TEST(ObexHeader_test, ObexOptionalTlvHeaderNullptr)
{
    ObexOptionalTlvHeader tlvHeader(0x4C);
    // GetTlvtriplet return nullprt
    EXPECT_THAT(tlvHeader.GetTlvtriplet(0xFF), Eq(nullptr));
    // GetBytes return nullptr
    EXPECT_THAT(tlvHeader.GetBytes(), Eq(nullptr));
}

TEST(ObexHeader_test, AppendItemAppParams)
{
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::PUT);
    uint16_t expectedLen = 3;
    expectedVector.insert(std::end(expectedVector), {0x02, 0x00, 0x00});

    // HN:AppParameters HI:0x4C
    uint16_t subStartPos = expectedVector.size();
    expectedVector.insert(std::end(expectedVector), {0x4C, 0x00, 0x00});
    uint16_t beforeLen = expectedLen;
    ObexTlvParamters appParamters;
    expectedLen += 3;  // header + len

    TlvTriplet item1(0x01, (uint16_t)0x0001);
    appParamters.AppendTlvtriplet(item1);
    expectedLen += 4;
    expectedVector.insert(std::end(expectedVector), {0x01, 0x02});
    AddData2List(expectedVector, (uint16_t)(0x0001));

    TlvTriplet item2(0x02, (uint32_t)(0x00000001));
    appParamters.AppendTlvtriplet(item2);
    expectedLen += 6;
    expectedVector.insert(std::end(expectedVector), {0x02, 0x04});
    AddData2List(expectedVector, (uint32_t)(0x00000001));

    TlvTriplet item3(0x03, (uint64_t)(0x0000000000000001));
    appParamters.AppendTlvtriplet(item3);
    expectedLen += 10;
    expectedVector.insert(std::end(expectedVector), {0x03, 0x08});
    AddData2List(expectedVector, (uint64_t)(0x0000000000000001));

    header->AppendItemAppParams(appParamters);

    EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
    SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

    EXPECT_NE(header->GetItemAppParams(), nullptr);
    EXPECT_THAT(header->GetItemAppParams()->GetHeaderClassType(), ObexHeaderDataType::TLV);
    EXPECT_THAT(header->GetItemAppParams()->GetHeaderClassTypeName(), "ObexOptionalTlvHeader");
    // TagId does not exist
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x00), Eq(nullptr));

    // GetItemAppParamsParseTlvItem1
    GetParameterParseTlvtriplet(header->GetItemAppParams()->GetTlvtriplet(0x01), {0x01, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x01)->GetUnitLen(), 2);
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x01)->GetUint16(), 1);
    // GetItemAppParamsParseTlvItem2
    GetParameterParseTlvtriplet(header->GetItemAppParams()->GetTlvtriplet(0x02), {0x01, 0x00, 0x00, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x02)->GetUnitLen(), 4);
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x02)->GetUint32(), 1);
    // GetItemAppParamsParseTlvItem3
    GetParameterParseTlvtriplet(
        header->GetItemAppParams()->GetTlvtriplet(0x03), {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x03)->GetUnitLen(), 8);
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x03)->GetUint64(), 1);
    // GetTlvtriplet(item1)
    GetParameterParseTlvtriplet(appParamters.GetTlvtriplet(0x01), {0x01, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x01)->GetUnitLen(), 2);
    // GetTlvtriplet(item2)
    GetParameterParseTlvtriplet(appParamters.GetTlvtriplet(0x02), {0x01, 0x00, 0x00, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x02)->GetUnitLen(), 4);
    // GetTlvtriplet(item3)
    GetParameterParseTlvtriplet(appParamters.GetTlvtriplet(0x03), {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x03)->GetUnitLen(), 8);

    // Clone
    std::unique_ptr<ObexOptionalHeader> cloneAppParamters = header->GetItemAppParams()->Clone();
    EXPECT_NE(cloneAppParamters, nullptr);
    EXPECT_THAT(cloneAppParamters->GetHeaderId(), 0x4C);
    EXPECT_THAT(header->GetItemAppParams()->GetHeaderId(), 0x4C);
    EXPECT_THAT(cloneAppParamters->GetHeaderDataSize(), 20);
    std::unique_ptr<uint8_t[]> actualClonePtr = cloneAppParamters->GetBytes();
    std::vector<uint8_t> actualCloneVector(
        actualClonePtr.get(), actualClonePtr.get() + cloneAppParamters->GetHeaderDataSize());
    EXPECT_THAT(actualCloneVector,
        ElementsAreArray({0x01,
            0x02,
            0x00,
            0x01,
            0x02,
            0x04,
            0x00,
            0x00,
            0x00,
            0x01,
            0x03,
            0x08,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x01}));

    SetData2List(expectedVector, 1, (uint16_t)expectedLen);  // modify total size

    auto obexPacket = header->Build();

    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    auto headerParsed = ObexHeader::ParseResponse(obexPacket->GetBuffer(), obexPacket->GetSize(), false);
    auto obexPacketParsed = headerParsed->Build();
    std::vector<uint8_t> actualVectorParsed(
        obexPacketParsed->GetBuffer(), obexPacketParsed->GetBuffer() + obexPacketParsed->GetSize());
    EXPECT_THAT(actualVectorParsed, ElementsAreArray(expectedVector));
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x01)->GetUint16(), 1);
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x02)->GetUint32(), 1);
    EXPECT_THAT(header->GetItemAppParams()->GetTlvtriplet(0x03)->GetUint64(), 1);
}

TEST(ObexHeader_test, AppendItemAuthChallenges)
{
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        // HN:Auth.Challenge HI:0x4D
        uint16_t subStartPos = expectedVector.size();
        expectedVector.insert(std::end(expectedVector), {0x4D, 0x00, 0x00});
        uint16_t beforeLen = expectedLen;
        ObexDigestChallenge authChallenges;
        expectedLen += 3;  // header + len
        authChallenges.AppendNonce(NONCE, NONCE_LEN);
        expectedLen += (2 + NONCE_LEN);
        expectedVector.insert(std::end(expectedVector), {0x00, NONCE_LEN});
        expectedVector.insert(std::end(expectedVector), NONCE, NONCE + NONCE_LEN);

        authChallenges.AppendOptions(uint8_t(0x00));
        expectedLen += (2 + 1);
        expectedVector.insert(std::end(expectedVector), {0x01, 0x01, 0x00});

        authChallenges.AppendRealm(REALM, REALM_LEN);
        expectedLen += (2 + REALM_LEN);
        expectedVector.insert(std::end(expectedVector), {0x02, REALM_LEN});
        expectedVector.insert(std::end(expectedVector), REALM, REALM + REALM_LEN);

        header->AppendItemAuthChallenges(authChallenges);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

        // GetItemAuthChallengesParseNonce
        EXPECT_NE(header->GetItemAuthChallenges(), nullptr);
        std::vector<uint8_t> expectedNonceVector(NONCE, NONCE + NONCE_LEN);
        GetParameterParseTlvtriplet(header->GetItemAuthChallenges()->GetTlvtriplet(0x00), expectedNonceVector);
        // GetItemAuthChallengesParseOptions
        GetParameterParseTlvtriplet(header->GetItemAuthChallenges()->GetTlvtriplet(0x01), {0x00});
        // GetItemAuthChallengesParseRealm
        std::vector<uint8_t> expectedRealmVector(REALM, REALM + REALM_LEN);
        GetParameterParseTlvtriplet(header->GetItemAuthChallenges()->GetTlvtriplet(0x02), expectedRealmVector);

        // GetNonce
        GetParameterParseTlvtriplet(authChallenges.GetNonce(), expectedNonceVector);
        // GetOptions
        GetParameterParseTlvtriplet(authChallenges.GetOptions(), {0x00});
        // GetRealm
        GetParameterParseTlvtriplet(authChallenges.GetRealm(), expectedRealmVector);
    });
}

TEST(ObexHeader_test, AppendItemAuthResponse)
{
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        // HN:Auth.Response HI:0x4E
        uint16_t subStartPos = expectedVector.size();
        expectedVector.insert(std::end(expectedVector), {0x4E, 0x00, 0x00});
        uint16_t beforeLen = expectedLen;
        ObexDigestResponse digestResponse;
        expectedLen += 3;  // header + len
        digestResponse.AppendRequestDigest(REQUEST_DIGEST, REQUEST_DIGEST_LEN);
        expectedLen += (2 + REQUEST_DIGEST_LEN);
        expectedVector.insert(std::end(expectedVector), {0x00, REQUEST_DIGEST_LEN});
        expectedVector.insert(std::end(expectedVector), REQUEST_DIGEST, REQUEST_DIGEST + REQUEST_DIGEST_LEN);

        digestResponse.AppendUserId(USER_ID, USER_ID_LEN);
        expectedLen += (2 + USER_ID_LEN);
        expectedVector.insert(std::end(expectedVector), {0x01, USER_ID_LEN});
        expectedVector.insert(std::end(expectedVector), USER_ID, USER_ID + USER_ID_LEN);

        digestResponse.AppendNonce(NONCE, NONCE_LEN);
        expectedLen += (2 + NONCE_LEN);
        expectedVector.insert(std::end(expectedVector), {0x02, NONCE_LEN});
        expectedVector.insert(std::end(expectedVector), NONCE, NONCE + NONCE_LEN);

        header->AppendItemAuthResponse(digestResponse);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

        // GetItemAuthResponseParseRequestDigest
        EXPECT_NE(header->GetItemAuthResponse(), nullptr);
        std::vector<uint8_t> expectedRequestDigestVector(REQUEST_DIGEST, REQUEST_DIGEST + REQUEST_DIGEST_LEN);
        GetParameterParseTlvtriplet(header->GetItemAuthResponse()->GetTlvtriplet(0x00), expectedRequestDigestVector);
        // GetItemAuthResponseParseRequestDigestUserId
        std::vector<uint8_t> expectedUserIdVector(USER_ID, USER_ID + USER_ID_LEN);
        GetParameterParseTlvtriplet(header->GetItemAuthResponse()->GetTlvtriplet(0x01), expectedUserIdVector);
        // GetItemAuthResponseParseRequestDigestNonce
        std::vector<uint8_t> expectedNonceVector(NONCE, NONCE + NONCE_LEN);
        GetParameterParseTlvtriplet(header->GetItemAuthResponse()->GetTlvtriplet(0x02), expectedNonceVector);

        // GetRequestDigest
        GetParameterParseTlvtriplet(digestResponse.GetRequestDigest(), expectedRequestDigestVector);
        // GetUserId
        GetParameterParseTlvtriplet(digestResponse.GetUserId(), expectedUserIdVector);
        // GetNonce
        GetParameterParseTlvtriplet(digestResponse.GetNonce(), expectedNonceVector);
    });
}

TEST(ObexHeader_test, AppendItemSessionParams)
{
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        // HN:SessionParams HI:0x52
        uint16_t subStartPos = expectedVector.size();
        expectedVector.insert(std::end(expectedVector), {0x52, 0x00, 0x00});
        uint16_t beforeLen = expectedLen;
        expectedLen += 3;  // header + len
        ObexSessionParameters sessionParameters;

        uint8_t deviceAddress[6] = {0x00, 0x16, 0xA4, 0x5A, 0xFC, 0xBF};
        uint8_t deviceAddressLen = sizeof(deviceAddress);
        sessionParameters.AppendDeviceAddress((const uint8_t *)deviceAddress, deviceAddressLen);
        expectedLen += (2 + deviceAddressLen);
        expectedVector.insert(std::end(expectedVector), {0x00, deviceAddressLen});
        expectedVector.insert(std::end(expectedVector), deviceAddress, deviceAddress + deviceAddressLen);

        const char *nonce = "1234567890123456";
        uint8_t nonceLen = strlen(nonce);  // has not '\0'
        sessionParameters.AppendNonce((const uint8_t *)nonce, nonceLen);
        expectedLen += (2 + nonceLen);
        expectedVector.insert(std::end(expectedVector), {0x01, nonceLen});
        expectedVector.insert(std::end(expectedVector), nonce, nonce + nonceLen);

        const char *sessionId = "0123456789abcdef";
        uint8_t sessionIdLen = strlen(sessionId);  // has not '\0'
        sessionParameters.AppendSessionId((const uint8_t *)sessionId, sessionIdLen);
        expectedLen += (2 + nonceLen);
        expectedVector.insert(std::end(expectedVector), {0x02, sessionIdLen});
        expectedVector.insert(std::end(expectedVector), sessionId, sessionId + sessionIdLen);

        const uint8_t nextSeqNum = 0xFF;
        sessionParameters.AppendNextSeqNum(nextSeqNum);
        expectedLen += (2 + 1);
        expectedVector.insert(std::end(expectedVector), {0x03, 0x01, nextSeqNum});

        const uint32_t timeout = 0xFFFFFFFF;
        sessionParameters.AppendTimeout(timeout);
        expectedLen += (2 + 4);
        expectedVector.insert(std::end(expectedVector), {0x04, 0x04, 0xFF, 0xFF, 0xFF, 0xFF});

        sessionParameters.AppendSessionOpcode(ObexSessionParameters::SessionOpcode::CREATE);
        expectedLen += (2 + 1);
        expectedVector.insert(std::end(expectedVector), {0x05, 0x01, 0x00});

        header->AppendItemSessionParams(sessionParameters);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        SetData2List(expectedVector, subStartPos + 1, (uint16_t)(expectedLen - beforeLen));  // modify size

        // GetItemSessionParamsParseDeviceAddress
        EXPECT_NE(header->GetItemSessionParams(), nullptr);
        GetParameterParseTlvtriplet(
            header->GetItemSessionParams()->GetTlvtriplet(0x00), {0x00, 0x16, 0xA4, 0x5A, 0xFC, 0xBF});
        // GetItemSessionParamsParseNonce
        std::vector<uint8_t> expectedNonceVector(nonce, nonce + nonceLen);
        GetParameterParseTlvtriplet(header->GetItemSessionParams()->GetTlvtriplet(0x01), expectedNonceVector);
        // GetItemSessionParamsParseSessionId
        std::vector<uint8_t> expectedSessionIdVector(sessionId, sessionId + sessionIdLen);
        GetParameterParseTlvtriplet(header->GetItemSessionParams()->GetTlvtriplet(0x02), expectedSessionIdVector);
        // GetItemSessionParamsParseNextSeqNum
        GetParameterParseTlvtriplet(header->GetItemSessionParams()->GetTlvtriplet(0x03), {0xFF});
        // GetItemSessionParamsParseTimeout
        GetParameterParseTlvtriplet(header->GetItemSessionParams()->GetTlvtriplet(0x04), {0xFF, 0xFF, 0xFF, 0xFF});
        // GetItemSessionParamsParseSessionOpcode
        GetParameterParseTlvtriplet(header->GetItemSessionParams()->GetTlvtriplet(0x05), {0x00});

        // GetDeviceAddress
        GetParameterParseTlvtriplet(sessionParameters.GetDeviceAddress(), {0x00, 0x16, 0xA4, 0x5A, 0xFC, 0xBF});
        // GetNonce
        GetParameterParseTlvtriplet(sessionParameters.GetNonce(), expectedNonceVector);
        // GetSessionId
        GetParameterParseTlvtriplet(sessionParameters.GetSessionId(), expectedSessionIdVector);
        // GetNextSeqNum
        GetParameterParseTlvtriplet(sessionParameters.GetNextSeqNum(), {0xFF});
        // GetTimeout
        GetParameterParseTlvtriplet(sessionParameters.GetTimeout(), {0xFF, 0xFF, 0xFF, 0xFF});
        // GetSessionOpcode
        GetParameterParseTlvtriplet(sessionParameters.GetSessionOpcode(), {0x00});
    });
}

// String header
TEST(ObexHeader_test, AppendItemType)
{
    // HN:Type HI:0x42
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        std::string type = "x-bt/phonebook";
        header->AppendItemType(type);
        expectedLen += (3 + 14 + 1);
        expectedVector.insert(std::end(expectedVector), {0x42, 0x00, 0x12});
        expectedVector.insert(std::end(expectedVector), type.c_str(), type.c_str() + 15);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemType(), nullptr);
        EXPECT_THAT(header->GetItemType()->GetString(), type);
        EXPECT_THAT(header->GetItemType()->GetHeaderClassType(), ObexHeaderDataType::STRING);
        EXPECT_THAT(header->GetItemType()->GetHeaderClassTypeName(), "ObexOptionalStringHeader");

        // Clone
        std::unique_ptr<ObexOptionalHeader> cloneItemType = header->GetItemType()->Clone();
        EXPECT_NE(cloneItemType, nullptr);
        EXPECT_THAT(cloneItemType->GetHeaderId(), 0x42);
        EXPECT_THAT(header->GetItemType()->GetHeaderId(), 0x42);
        EXPECT_THAT(cloneItemType->GetHeaderDataSize(), 15);
        std::unique_ptr<uint8_t[]> actualClonePtr = cloneItemType->GetBytes();
        std::vector<uint8_t> actualCloneVector(
            actualClonePtr.get(), actualClonePtr.get() + cloneItemType->GetHeaderDataSize());
        std::vector<uint8_t> actualTypeVector(type.c_str(), type.c_str() + type.size() + 1);
        EXPECT_THAT(actualCloneVector, ElementsAreArray(actualTypeVector));
        ObexUtils::ObexHeaderDebug(*header);
    });
}

TEST(ObexHeader_test, AppendItemTypeIsEmpty)
{
    // HN:Type HI:0x42
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        std::string type = "";
        header->AppendItemType(type);
        expectedLen += (3);
        expectedVector.insert(std::end(expectedVector), {0x42, 0x00, 0x03});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemType(), nullptr);
        EXPECT_THAT(header->GetItemType()->GetString(), type);
        EXPECT_THAT(header->GetItemType()->GetHeaderClassType(), ObexHeaderDataType::STRING);
        EXPECT_THAT(header->GetItemType()->GetHeaderClassTypeName(), "ObexOptionalStringHeader");
    });
}

// Word header
TEST(ObexHeader_test, AppendItemConnectionId)
{
    // HN:ConnectionId HI:0xCB
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint32_t connectionId = 0x12345678;
        header->AppendItemConnectionId(connectionId);
        expectedLen += (1 + 4);
        expectedVector.insert(std::end(expectedVector), {0xCB});
        AddData2List(expectedVector, connectionId);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemConnectionId(), nullptr);
        EXPECT_THAT(header->GetItemConnectionId()->GetWord(), connectionId);
        EXPECT_THAT(header->GetItemConnectionId()->GetHeaderClassType(), ObexHeaderDataType::WORD);
        EXPECT_THAT(header->GetItemConnectionId()->GetHeaderClassTypeName(), "ObexOptionalWordHeader");

        // Clone
        std::unique_ptr<ObexOptionalHeader> cloneConnectionId = header->GetItemConnectionId()->Clone();
        EXPECT_NE(cloneConnectionId, nullptr);
        EXPECT_THAT(cloneConnectionId->GetHeaderId(), 0xCB);
        EXPECT_THAT(header->GetItemConnectionId()->GetHeaderId(), 0xCB);
        EXPECT_THAT(cloneConnectionId->GetHeaderDataSize(), 4);
        std::unique_ptr<uint8_t[]> actualClonePtr = cloneConnectionId->GetBytes();
        std::vector<uint8_t> actualCloneVector(
            actualClonePtr.get(), actualClonePtr.get() + cloneConnectionId->GetHeaderDataSize());
        std::unique_ptr<uint8_t[]> actualConnectionIdPtr = header->GetItemConnectionId()->GetBytes();
        std::vector<uint8_t> actualConnectionIdVector(actualConnectionIdPtr.get(), actualConnectionIdPtr.get() + 4);
        EXPECT_THAT(actualCloneVector, ElementsAreArray(actualConnectionIdVector));
    });
}

TEST(ObexHeader_test, AppendItemCount)
{
    // HN:Count HI:0xC0
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint32_t count = 0x12345678;
        header->AppendItemCount(count);
        expectedLen += (1 + 4);
        expectedVector.insert(std::end(expectedVector), {0xC0});
        AddData2List(expectedVector, count);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemCount(), nullptr);
        EXPECT_THAT(header->GetItemCount()->GetWord(), count);
    });
}

TEST(ObexHeader_test, AppendItemLength)
{
    // HN:Length HI:0xC3
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint32_t length = 0x12345678;
        header->AppendItemLength(length);
        expectedLen += (1 + 4);
        expectedVector.insert(std::end(expectedVector), {0xC3});
        AddData2List(expectedVector, length);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemLength(), nullptr);
        EXPECT_THAT(header->GetItemLength()->GetWord(), length);
    });
}

TEST(ObexHeader_test, AppendItemTime4byte)
{
    // HN:Time4byte HI:0xC4
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint32_t time4byte = 2012312359;  // YYMMDDHHMM
        header->AppendItemTime4byte(time4byte);
        expectedLen += (1 + 4);
        expectedVector.insert(std::end(expectedVector), {0xC4});
        AddData2List(expectedVector, time4byte);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemTime4byte(), nullptr);
        EXPECT_THAT(header->GetItemTime4byte()->GetWord(), time4byte);
    });
}

TEST(ObexHeader_test, AppendItemPermissions)
{
    // HN:Permissions HI:0xD6
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint32_t permissions = 0x12345678;
        header->AppendItemPermissions(permissions);
        expectedLen += (1 + 4);
        expectedVector.insert(std::end(expectedVector), {0xD6});
        AddData2List(expectedVector, permissions);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemPermissions(), nullptr);
        EXPECT_THAT(header->GetItemPermissions()->GetWord(), permissions);
    });
}

// Unicode header
TEST(ObexHeader_test, AppendItemName)
{
    // HN:Name HI:0x01
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        std::u16string name = u"telecom/pb.vcf";
        header->AppendItemName(name);
        expectedLen += (3 + 30);
        expectedVector.insert(std::end(expectedVector), {0x01, 0x00, 0x21});
        expectedVector.insert(std::end(expectedVector),
            {0x00,
                0x74,
                0x00,
                0x65,
                0x00,
                0x6C,
                0x00,
                0x65,
                0x00,
                0x63,
                0x00,
                0x6F,
                0x00,
                0x6D,
                0x00,
                0x2F,
                0x00,
                0x70,
                0x00,
                0x62,
                0x00,
                0x2E,
                0x00,
                0x76,
                0x00,
                0x63,
                0x00,
                0x66,
                0x00,
                0x00});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemName(), nullptr);
        EXPECT_THAT(header->GetItemName()->GetUnicodeText(), name);
        EXPECT_THAT(header->GetItemName()->GetHeaderClassType(), ObexHeaderDataType::UNICODE_TEXT);
        EXPECT_THAT(header->GetItemName()->GetHeaderClassTypeName(), "ObexOptionalUnicodeHeader");

        // Clone
        std::unique_ptr<ObexOptionalHeader> cloneName = header->GetItemName()->Clone();
        EXPECT_NE(cloneName, nullptr);
        EXPECT_THAT(cloneName->GetHeaderId(), 0x01);
        EXPECT_THAT(header->GetItemName()->GetHeaderId(), 0x01);
        EXPECT_THAT(cloneName->GetHeaderDataSize(), 30);
        std::unique_ptr<uint8_t[]> actualClonePtr = cloneName->GetBytes();
        std::vector<uint8_t> actualCloneVector(
            actualClonePtr.get(), actualClonePtr.get() + cloneName->GetHeaderDataSize());
        std::unique_ptr<uint8_t[]> actualNamePtr = header->GetItemName()->GetBytes();
        std::vector<uint8_t> actualNameVector(
            actualNamePtr.get(), actualNamePtr.get() + header->GetItemName()->GetHeaderDataSize());
        EXPECT_THAT(actualCloneVector, ElementsAreArray(actualNameVector));
        ObexUtils::ObexHeaderDebug(*header);
    });
}

TEST(ObexHeader_test, AppendItemDescription)
{
    // HN:Description HI:0x05
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        std::u16string description = u"text";
        header->AppendItemDescription(description);
        expectedLen += (3 + 10);
        expectedVector.insert(std::end(expectedVector), {0x05, 0x00, 0x0D});
        expectedVector.insert(std::end(expectedVector), {0x00, 0x74, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemDescription(), nullptr);
        EXPECT_THAT(header->GetItemDescription()->GetUnicodeText(), description);
    });
}

TEST(ObexHeader_test, AppendAndGetItemDestName)
{
    // HN:DestName HI:0x15
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        std::u16string destName = u"pb.vcf";
        header->AppendItemDestName(destName);
        expectedLen += (3 + 14);
        expectedVector.insert(std::end(expectedVector), {0x15, 0x00, 0x11});
        expectedVector.insert(std::end(expectedVector),
            {0x00, 0x70, 0x00, 0x62, 0x00, 0x2E, 0x00, 0x76, 0x00, 0x63, 0x00, 0x66, 0x00, 0x00});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemDestName(), nullptr);
        EXPECT_THAT(header->GetItemDestName()->GetUnicodeText(), destName);
    });
}

// Byte header
TEST(ObexHeader_test, AppendItemSessionSeqNum)
{
    // HN:SessionSeqNum HI:0x93
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        uint8_t num = 0x01;
        header->AppendItemSessionSeqNum(num);
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x93, 0x01});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemSessionSeqNum(), nullptr);
        EXPECT_THAT(header->GetItemSessionSeqNum()->GetByte(), 0x01);
        EXPECT_THAT(header->GetItemSessionSeqNum()->GetHeaderClassType(), ObexHeaderDataType::BYTE);
        EXPECT_THAT(header->GetItemSessionSeqNum()->GetHeaderClassTypeName(), "ObexOptionalByteHeader");

        // Clone
        std::unique_ptr<ObexOptionalHeader> cloneSessionSeqNum = header->GetItemSessionSeqNum()->Clone();
        EXPECT_NE(cloneSessionSeqNum, nullptr);
        EXPECT_THAT(cloneSessionSeqNum->GetHeaderId(), 0x93);
        EXPECT_THAT(header->GetItemSessionSeqNum()->GetHeaderId(), 0x93);
        EXPECT_THAT(cloneSessionSeqNum->GetHeaderDataSize(), 1);
        std::unique_ptr<uint8_t[]> actualClonePtr = cloneSessionSeqNum->GetBytes();
        std::vector<uint8_t> actualCloneVector(
            actualClonePtr.get(), actualClonePtr.get() + cloneSessionSeqNum->GetHeaderDataSize());
        EXPECT_THAT(actualCloneVector, ElementsAreArray({0x01}));

        // RemoveItem
        header->RemoveItem(0x94);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_THAT(header->GetItemSessionSeqNum()->GetHeaderId(), 0x93);
        header->RemoveItem(0x93);
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen - 2);
        EXPECT_THAT(header->GetItemSessionSeqNum(), Eq(nullptr));
        expectedLen -= (1 + 1);
        expectedVector.erase(expectedVector.begin() + expectedLen, expectedVector.begin() + expectedLen + 2);
    });
}

TEST(ObexHeader_test, AppendItemActionId)
{
    // HN:ActionId HI:0x94
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        ObexActionType actionId = ObexActionType::COPY;
        header->AppendItemActionId(actionId);
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x94, 0x00});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_NE(header->GetItemActionId(), nullptr);
        EXPECT_THAT(header->GetItemActionId()->GetByte(), 0x00);
    });
}

TEST(ObexHeader_test, AppendItemSrm)
{
    // HN:ActionId HI:0x97
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        bool srm = true;
        EXPECT_THAT(header->GetItemSrm(), false);
        header->AppendItemSrm(srm);
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x97, 0x01});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_THAT(header->GetItemSrm(), true);
    });
}

TEST(ObexHeader_test, AppendItemSrmp)
{
    // HN:ActionId HI:0x98
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        EXPECT_THAT(header->GetItemSrmp(), false);
        header->AppendItemSrmp();
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x98, 0x01});
        EXPECT_THAT(header->GetFieldPacketLength(), expectedLen);
        EXPECT_THAT(header->GetItemSrmp(), true);
    });
}

TEST(ObexHeader_test_ExceptionCatch, ObexHeaderParsePacketLengthException)
{
    // ParseRequest or ParseResponse return nullprt
    // ConnectObexHeader + SessionSeqNum
    std::vector<uint8_t> expectedVector;
    auto header = ObexHeader::CreateRequest(ObexOpeId::CONNECT);
    header->AppendItemSessionSeqNum(0x01);
    expectedVector.insert(std::end(expectedVector), {0x80, 0x00, 0x09, 0x10, 0x00, 0x04, 0x00, 0x93, 0x01});

    auto obexPacket = header->Build();
    std::vector<uint8_t> actualVector(obexPacket->GetBuffer(), obexPacket->GetBuffer() + obexPacket->GetSize());
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));
    // packetLength size is wrong
    auto requestParsed = ObexHeader::ParseRequest(obexPacket->GetBuffer(), 8);
    auto responseParsed = ObexHeader::ParseResponse(obexPacket->GetBuffer(), 8, true);
    EXPECT_THAT(requestParsed, Eq(nullptr));
    EXPECT_THAT(responseParsed, Eq(nullptr));
}

TEST(ObexHeader_test_ExceptionCatch, AppendItemTargetWhenLengthIsNot16)
{
    // AppendItemTarget length is wrong!
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        const char *data = "123456789012345";
        int dataLen = strlen(data);  // has not '\0'
        auto retBool = header->AppendItemTarget((const uint8_t *)data, dataLen);
        EXPECT_THAT(retBool, false);
    });
}

TEST(ObexHeader_test_ExceptionCatch, AppendItemTargetNotFirst)
{
    // Target must the first header in the request
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        header->AppendItemSrmp();
        expectedLen += 2;
        expectedVector.insert(std::end(expectedVector), {0x98, 0x01});

        const char *data = "1234567890123456";
        int dataLen = strlen(data);  // has not '\0'
        auto retBool = header->AppendItemTarget((const uint8_t *)data, dataLen);
        EXPECT_THAT(retBool, false);
    });
}

TEST(ObexHeader_test_ExceptionCatch, AppendItemSessionParams)
{
    // SessionParams must the first header in the request
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        header->AppendItemSrmp();
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x98, 0x01});

        ObexSessionParameters sessionParameters;
        uint8_t deviceAddress[6] = {0x00, 0x16, 0xA4, 0x5A, 0xFC, 0xBF};
        uint8_t deviceAddressLen = sizeof(deviceAddress);
        sessionParameters.AppendDeviceAddress((const uint8_t *)deviceAddress, deviceAddressLen);
        auto retBool = header->AppendItemSessionParams(sessionParameters);
        EXPECT_THAT(retBool, false);
    });
}

TEST(ObexHeader_test_ExceptionCatch, AppendItemConnectionId)
{
    // ConnectionId must the first header in the request
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        header->AppendItemSrmp();
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x98, 0x01});

        auto retBool = header->AppendItemConnectionId((uint32_t)1024);
        EXPECT_THAT(retBool, false);
    });
}

TEST(ObexHeader_test_ExceptionCatch, AppendItemSessionSeqNum)
{
    // SessionSeqNum must the first header in the request
    HeaderAppendItem([](ObexHeader *header, std::vector<uint8_t> &expectedVector, uint16_t &expectedLen) {
        header->AppendItemSrmp();
        expectedLen += (1 + 1);
        expectedVector.insert(std::end(expectedVector), {0x98, 0x01});

        auto retBool = header->AppendItemSessionSeqNum(0x01);
        EXPECT_THAT(retBool, false);
    });
}