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
#include "obex_body.h"
#include "securec.h"

using namespace testing;
using namespace bluetooth;

TEST(ObexArrayBodyObject_test, Read1)
{
    uint8_t body[100];
    for (uint8_t i = 0; i < 100; i++) {
        body[i] = i;
    }
    ObexArrayBodyObject bodyRead(body, 100);
    std::vector<uint8_t> expectedVector;
    uint8_t buf[50];
    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    int cnt = bodyRead.Read(buf, 50);
    EXPECT_THAT(cnt, 50);
    expectedVector.insert(std::end(expectedVector), buf, buf + cnt);

    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    cnt = bodyRead.Read(buf, 50);
    EXPECT_THAT(cnt, 50);
    expectedVector.insert(std::end(expectedVector), buf, buf + cnt);

    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    EXPECT_THAT(bodyRead.Read(buf, 50), 0);

    std::vector<uint8_t> actualVector(body, body + sizeof(body));
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    EXPECT_THAT(bodyRead.Close(), 0);
}

TEST(ObexArrayBodyObject_test, Read2)
{
    uint8_t body[100];
    for (uint8_t i = 0; i < 100; i++) {
        body[i] = i;
    }
    ObexArrayBodyObject bodyRead(body, 100);

    std::vector<uint8_t> expectedVector;
    uint8_t buf[49];
    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    int cnt = bodyRead.Read(buf, 49);
    EXPECT_THAT(cnt, 49);
    expectedVector.insert(std::end(expectedVector), buf, buf + cnt);

    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    cnt = bodyRead.Read(buf, 49);
    EXPECT_THAT(cnt, 49);
    expectedVector.insert(std::end(expectedVector), buf, buf + cnt);

    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    cnt = bodyRead.Read(buf, 49);
    EXPECT_THAT(cnt, 2);
    expectedVector.insert(std::end(expectedVector), buf, buf + cnt);

    memset_s(buf, sizeof(buf), 0x00, sizeof(buf));
    EXPECT_THAT(bodyRead.Read(buf, 49), 0);

    std::vector<uint8_t> actualVector(body, body + sizeof(body));
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    EXPECT_THAT(bodyRead.Close(), 0);
}

TEST(ObexArrayBodyObject_test, Write)
{
    uint8_t body[200];
    for (uint8_t i = 0; i < 200; i++) {
        body[i] = i;
    }
    ObexArrayBodyObject bodyWrite;
    int cnt = bodyWrite.Write(body, 100);
    EXPECT_THAT(cnt, 100);
    cnt = bodyWrite.Write(&body[100], 100);
    EXPECT_THAT(cnt, 100);
    std::vector<uint8_t> expectedVector;
    expectedVector.insert(std::end(expectedVector), body, body + sizeof(body));

    std::vector<uint8_t> actualVector;
    uint8_t tmpBuf[1];
    while (bodyWrite.Read(tmpBuf, sizeof(tmpBuf)) > 0) {
        actualVector.insert(actualVector.end(), tmpBuf, tmpBuf + sizeof(tmpBuf));
    }
    EXPECT_THAT(actualVector, ElementsAreArray(expectedVector));

    EXPECT_THAT(bodyWrite.Close(), 0);
}
