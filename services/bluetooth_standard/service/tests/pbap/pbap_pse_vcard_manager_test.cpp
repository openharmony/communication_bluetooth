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

#include "log.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "pbap_pse_vcard_manager.h"
#include "obex_utils.h"

using namespace testing;
using namespace bluetooth;

TEST(pbap_pse_vcard_manager_test, VcardHandleId2Int)
{
    std::u16string hexStr = u"";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), -1);

    hexStr = u"gh12";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), -1);

    hexStr = u"0";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), 0);

    hexStr = u"F";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), 15);

    hexStr = u"FF";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), 255);

    hexStr = u"FFF";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), 4095);

    hexStr = u"FFfF";
    EXPECT_THAT(PbapPseVcardManager::VcardHandleId2Int(hexStr), 65535);
}

TEST(pbap_pse_vcard_manager_test, GetIncludeProperties)
{
    uint8_t bytes[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
    uint64_t propertySelector = ObexUtils::GetBufData64(bytes);
    std::vector<std::string> list = PbapPseVcardManager::GetIncludeProperties(propertySelector);
    EXPECT_THAT(list.size(), 0);

    uint8_t bytes1[8] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
    uint64_t propertySelector1 = ObexUtils::GetBufData64(bytes1);
    std::vector<std::string> list1 = PbapPseVcardManager::GetIncludeProperties(propertySelector1);
    EXPECT_THAT(list1.size(), 32);
}

TEST(pbap_pse_vcard_manager_test, PullvCardEntryWrongFolder)
{
    std::u16string currentPath = u"/telecom/pb2";
    std::u16string entryId = u"3";
    PbapPseAppParams pbapAppParams;
    uint32_t supportedFeatures = 3;
    PbapPseVcardManager::PhoneBookResult result;
    PbapPseVcardManager::PullvCardEntry(currentPath, entryId, pbapAppParams, supportedFeatures, result);
    EXPECT_THAT(result.rspCode_, ObexRspCode::NOT_FOUND);
}

TEST(pbap_pse_vcard_manager_test, PullPhoneBookMch)
{
    std::u16string nameWithFolder = u"/telecom/mch.vcf";
    PbapPseAppParams pbapAppParams;
    uint32_t supportedFeatures = 3;
    {
        PbapPseVcardManager::PhoneBookResult result;
        PbapPseVcardManager::PullPhoneBook(u"/telecom/mch2.vcf", pbapAppParams, supportedFeatures, result);
        EXPECT_THAT(result.rspCode_, ObexRspCode::NOT_FOUND);
    }
    {
        PbapPseVcardManager::PhoneBookResult result;
        PbapPseVcardManager::PullPhoneBook(nameWithFolder, pbapAppParams, supportedFeatures, result);
        EXPECT_THAT(result.rspCode_, ObexRspCode::SUCCESS);
        EXPECT_THAT(result.newMissedCalls_, true);
        EXPECT_THAT(result.newMissedCallsSize_ > 0, true);
    }
}

TEST(pbap_pse_vcard_manager_test, IsSupportedXBtUCI)
{
    EXPECT_THAT(PbapPseVcardManager::IsSupportedXBtUCI(1 << 6), true);
}

TEST(pbap_pse_vcard_manager_test, IsSupportedXBtUID)
{
    EXPECT_THAT(PbapPseVcardManager::IsSupportedXBtUID(1 << 7), true);
}

TEST(pbap_pse_vcard_manager_test, CheckVcardHandleId)
{
    EXPECT_THAT(PbapPseVcardManager::CheckVcardHandleId(u""), false);
}

TEST(pbap_pse_vcard_manager_test, CheckPhoneBookPath)
{
    EXPECT_THAT(PbapPseVcardManager::CheckPhoneBookPath(u"/telecom/mch"), true);
    EXPECT_THAT(PbapPseVcardManager::CheckPhoneBookPath(u"telecom/mch"), true);
}