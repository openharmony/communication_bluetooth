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
#include "log.h"
#include "data_access.h"

using namespace testing;
using namespace bluetooth;

class PbapPseSqliteTest : public testing::Test {
protected:
    virtual void SetUp()
    {
        dataAccess_ = DataAccess::GetConnection("pbap_pse_test.s3db", false);
        EXPECT_NE(dataAccess_, nullptr);
    }
    virtual void TearDown()
    {}

    static int PrintResult(IDataResult *dataResult)
    {
        int col = dataResult->GetColumnCount();
        int rows = 0;
        std::cout << std::endl;
        for (int i = 0; i < col; i++) {
            std::string name = dataResult->GetColumnName(i);
            std::cout << name << "\t";
        }
        while (dataResult->Next()) {
            rows++;
            std::cout << std::endl;
            for (int i = 0; i < col; i++) {
                std::string val = dataResult->GetString(i);
                std::cout << val << "\t";
            }
        }
        std::cout << std::endl;
        return rows;
    }

    std::shared_ptr<DataAccess> dataAccess_ = nullptr;
};

TEST_F(PbapPseSqliteTest, sqlite)
{
    if (dataAccess_) {
        auto select = "select * from vcard_view where  folder_id = ? order by vcard_uid, profile_property_order";
        {
            auto stmt = dataAccess_->CreateStatement(select);
            std::u16string folderId = u"pb";
            stmt->SetParam16String(1, folderId);
            EXPECT_NE(stmt, nullptr);
            EXPECT_THAT(dataAccess_->BeginTransaction(), true);
            auto delSql = "delete from vcard";
            auto delStmt = dataAccess_->CreateStatement(delSql);
            delStmt->Delete();
            auto dataResult = stmt->Query();
            EXPECT_THAT(PrintResult(dataResult.get()) == 0, true);
            dataAccess_->Rollback();
        }
        {
            auto stmt = dataAccess_->CreateStatement(select);
            std::u16string folderId = u"pb";
            stmt->SetParam16String(1, folderId);
            EXPECT_NE(stmt, nullptr);

            auto dataResult = stmt->Query();
            EXPECT_THAT(PrintResult(dataResult.get()) > 0, true);
            dataAccess_->Rollback();
        }
    }
}