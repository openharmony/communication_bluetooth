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

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "obex/obex_utils.h"
#include "pbap_pce_app_params.h"

using namespace testing;
using namespace bluetooth;

namespace pbap_pce_appparam_test {
std::vector<uint8_t> GetRandomVector()
{
    std::vector<uint8_t> data;
    for (uint8_t i = 0; i < 16; i++) {
        data.push_back((uint8_t)rand());
    }
    return data;
}
}  // namespace pbap_pce_appparam_test

TEST(pbap_pce_appparam_test, PbapPceAppParams)
{
    PbapPceAppParams appParams;
    appParams.SetOrder(rand());
    auto data = pbap_pce_appparam_test::GetRandomVector();
    appParams.SetSearchValueUtf8(data);
    appParams.SetSearchProperty(rand());
    appParams.SetMaxListCount(rand());
    appParams.SetListStartOffset(rand());
    appParams.SetPropertySelector(rand());
    appParams.SetFormat(rand());
    appParams.SetPhonebookSize(rand());
    appParams.SetNewMissedCalls(rand());
    auto data1 = pbap_pce_appparam_test::GetRandomVector();
    appParams.SetPrimaryFolderVersion(data1);
    auto data2 = pbap_pce_appparam_test::GetRandomVector();
    appParams.SetSecondaryFolderVersion(data2);
    appParams.SetVcardSelector(rand());
    auto data3 = pbap_pce_appparam_test::GetRandomVector();
    appParams.SetDatabaseIdentifier(data3);
    appParams.SetVcardSelectorOperator(rand());
    appParams.SetResetNewMissedCalls(rand());
    appParams.SetPbapSupportedFeatures(rand());

    auto obexHeader = ObexHeader::CreateRequest(ObexOpeId::GET);
    appParams.AddToObexHeader(*obexHeader);
    std::cout << "before\n" << ObexUtils::ToDebugString(obexHeader->Build()->GetPacket()) << std::endl;
    auto obexAppParams = obexHeader->GetItemAppParams();
    EXPECT_NE(obexAppParams, nullptr);
    auto &tlvParams = obexAppParams->GetTlvParamters();
    EXPECT_NE(tlvParams, nullptr);

    PbapPceAppParams newAppParams;
    EXPECT_THAT(newAppParams.Init(*tlvParams), true);

    EXPECT_THAT(*newAppParams.GetOrder(), *appParams.GetOrder());
    EXPECT_THAT(newAppParams.GetSearchValueUtf8(), appParams.GetSearchValueUtf8());
    EXPECT_THAT(*newAppParams.GetSearchProperty(), *appParams.GetSearchProperty());
    EXPECT_THAT(*newAppParams.GetMaxListCount(), *appParams.GetMaxListCount());
    EXPECT_THAT(*newAppParams.GetListStartOffset(), *appParams.GetListStartOffset());
    EXPECT_THAT(*newAppParams.GetPropertySelector(), *appParams.GetPropertySelector());
    EXPECT_THAT(*newAppParams.GetFormat(), *appParams.GetFormat());
    EXPECT_THAT(*newAppParams.GetPhonebookSize(), *appParams.GetPhonebookSize());
    EXPECT_THAT(*newAppParams.GetNewMissedCalls(), *appParams.GetNewMissedCalls());
    EXPECT_THAT(newAppParams.GetPrimaryFolderVersion(), appParams.GetPrimaryFolderVersion());
    EXPECT_THAT(newAppParams.GetSecondaryFolderVersion(), appParams.GetSecondaryFolderVersion());
    EXPECT_THAT(*newAppParams.GetVcardSelector(), *appParams.GetVcardSelector());
    EXPECT_THAT(newAppParams.GetDatabaseIdentifier(), appParams.GetDatabaseIdentifier());
    EXPECT_THAT(*newAppParams.GetVcardSelectorOperator(), *appParams.GetVcardSelectorOperator());
    EXPECT_THAT(*newAppParams.GetResetNewMissedCalls(), *appParams.GetResetNewMissedCalls());
    EXPECT_THAT(*newAppParams.GetPbapSupportedFeatures(), *appParams.GetPbapSupportedFeatures());
}
