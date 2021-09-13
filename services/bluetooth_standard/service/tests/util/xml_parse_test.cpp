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

#include "xml_parse.h"
#include <stdlib.h>
#include <iostream>
#include "gtest/gtest.h"
#include <unistd.h>
#include "base_def.h"

class XmlparseTest : public testing::Test {
public:
    virtual void SetUp()
    {
        std::string copyCommand =
            "cp " + BT_CONFIG_PATH_BASE + "bt_config.xml" + " " + BT_CONFIG_PATH + "bt_config_ut.xml";
        system(copyCommand.c_str());
        xmlParse_ = new utility::XmlParse();
        xmlParse_->Load("bt_config_ut.xml");
    }
    virtual void TearDown()
    {
        std::string removeCommand = "rm " + BT_CONFIG_PATH + "bt_config_ut.xml";
        system(removeCommand.c_str());
        if (xmlParse_ != nullptr) {
            delete xmlParse_;
            xmlParse_ = nullptr;
        }
    }
    static utility::XmlParse *xmlParse_;
};
utility::XmlParse *XmlparseTest::xmlParse_ = nullptr;

// Test the normal cases
TEST_F(XmlparseTest, Xmlparse_Load)
{
    EXPECT_FALSE(xmlParse_->Load("non_exist.xml"));
    EXPECT_TRUE(xmlParse_->Load("bt_config_ut.xml"));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_bool1)
{
    bool setval = false;
    bool getval = true;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "IsValid", getval));
    EXPECT_FALSE(getval);

    setval = true;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "IsValid", getval));
    EXPECT_TRUE(getval);

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapterErr", "IsValid", getval));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_bool2)
{
    bool setval = false;
    bool getval = true;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", getval));
    EXPECT_FALSE(getval);

    setval = true;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", getval));
    EXPECT_TRUE(getval);

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_int1)
{
    int setval = 100;
    int getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_EQ(setval, getval);

    setval = 2147483647;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_EQ(setval, getval);

    setval = 2147483640;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_EQ(setval, getval);

    setval = -1;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_EQ(setval, getval);

    setval = -2147483648;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_EQ(setval, getval);

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapterErr", "IsValid", getval));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_int2)
{
    int setval = 100;
    int getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_EQ(setval, getval);

    setval = 2147483647;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_EQ(setval, getval);

    setval = 2147483640;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_EQ(setval, getval);

    setval = -1;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_EQ(setval, getval);

    setval = -2147483648;
    getval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_EQ(setval, getval);

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter3", "Value", getval));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_string1)
{
    std::string setval = "kob#dfaead???eteadagr4578ERDGA457";
    std::string getval = "";
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", getval));
    EXPECT_STREQ(setval.c_str(), getval.c_str());

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapterErr", "IsValid", getval));
}

TEST_F(XmlparseTest, Xmlparse_SetGetValue_string2)
{
    std::string setval = "kobe";
    std::string getval = "";
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "Value", setval));
    EXPECT_TRUE(xmlParse_->GetValue("ClassicAdapter", "ClassicAdapter2", "Value", getval));
    EXPECT_STREQ(setval.c_str(), getval.c_str());

    EXPECT_FALSE(xmlParse_->GetValue("ClassicAdapter", "SectionErr", "Value", getval));
}

TEST_F(XmlparseTest, Xmlparse_Save)
{
    int setval = -1;
    int getval = 0;
    xmlParse_->SetValue("1", "2", setval);
    EXPECT_TRUE(xmlParse_->Save());
    xmlParse_->GetValue("1", "2", getval);
    EXPECT_EQ(setval, getval);

    setval = 2147483647;
    getval = 0;
    xmlParse_->SetValue("1", "3", setval);
    EXPECT_TRUE(xmlParse_->Save());
    xmlParse_->GetValue("1", "3", getval);
    EXPECT_EQ(setval, getval);

    setval = 2147483640;
    getval = 0;
    xmlParse_->SetValue("1", "4", setval);
    EXPECT_TRUE(xmlParse_->Save());
    xmlParse_->GetValue("1", "4", getval);
    EXPECT_EQ(setval, getval);

    setval = -2147483648;
    getval = 0;
    xmlParse_->SetValue("1", "5", setval);
    EXPECT_TRUE(xmlParse_->Save());
    xmlParse_->GetValue("1", "5", getval);
    EXPECT_EQ(setval, getval);
}

TEST_F(XmlparseTest, Xmlparse_GetSubSections_RemoveSection)
{
    std::vector<std::string> list;

    EXPECT_FALSE(xmlParse_->GetSubSections("SectionErr", list));

    EXPECT_TRUE(xmlParse_->RemoveSection("ClassicAdapter"));
    EXPECT_FALSE(xmlParse_->HasSection("ClassicAdapter"));

    std::string setval = "aaaaa";
    xmlParse_->SetValue("ClassicAdapter", "0", "Value", setval);
    setval = "bbbbb";
    xmlParse_->SetValue("ClassicAdapter", "1", "Value", setval);
    xmlParse_->SetValue("ClassicAdapter", "2", "Value", 0x6849);
    xmlParse_->SetValue("ClassicAdapter", "3", "Value", true);
    EXPECT_TRUE(xmlParse_->Save());

    std::string getval = "";
    xmlParse_->GetValue("ClassicAdapter", "0", "Value", getval);
    EXPECT_STREQ("aaaaa", getval.c_str());
    xmlParse_->GetValue("ClassicAdapter", "1", "Value", getval);
    EXPECT_STREQ("bbbbb", getval.c_str());
    int getValInt = 0;
    xmlParse_->GetValue("ClassicAdapter", "2", "Value", getValInt);
    EXPECT_EQ(0x6849, getValInt);
    bool getValBool = false;
    xmlParse_->GetValue("ClassicAdapter", "3", "Value", getValBool);
    EXPECT_TRUE(getValBool);

    EXPECT_TRUE(xmlParse_->GetSubSections("ClassicAdapter", list));
    EXPECT_STREQ("0", list[0].c_str());
    EXPECT_STREQ("1", list[1].c_str());
    EXPECT_STREQ("2", list[2].c_str());
    EXPECT_STREQ("3", list[3].c_str());

    xmlParse_->SetValue("ClassicAdapter", "1", "Value0", 11);
    xmlParse_->SetValue("ClassicAdapter", "1", "Value1", 22);

    EXPECT_TRUE(xmlParse_->RemoveSection("ClassicAdapter", "1"));
    EXPECT_FALSE(xmlParse_->HasSection("ClassicAdapter", "1"));

    std::vector<std::string> list2;
    EXPECT_TRUE(xmlParse_->GetSubSections("ClassicAdapter", list2));
    EXPECT_STREQ("0", list2[0].c_str());
    EXPECT_STREQ("2", list2[1].c_str());
    EXPECT_STREQ("3", list2[2].c_str());

    EXPECT_FALSE(xmlParse_->RemoveSection("SectionErr", "2"));
    EXPECT_FALSE(xmlParse_->RemoveSection("ClassicAdapter2"));
    EXPECT_FALSE(xmlParse_->RemoveSection("ClassicAdapter", "4"));

    xmlParse_->Save();
}

TEST_F(XmlparseTest, Xmlparse_RemoveProperty)
{
    xmlParse_->SetValue("ClassicAdapter", "Value0", "aa");
    xmlParse_->SetValue("ClassicAdapter", "Value1", "bb");
    xmlParse_->SetValue("ClassicAdapter", "Value2", 15674);
    xmlParse_->SetValue("ClassicAdapter", "Value3", true);
    xmlParse_->Save();

    EXPECT_TRUE(xmlParse_->RemoveProperty("ClassicAdapter", "Value0"));
    EXPECT_FALSE(xmlParse_->HasProperty("ClassicAdapter", "Value0"));

    xmlParse_->SetValue("ClassicAdapter", "0", "Value", true);
    EXPECT_TRUE(xmlParse_->RemoveProperty("ClassicAdapter", "0", "Value"));
    EXPECT_FALSE(xmlParse_->HasProperty("ClassicAdapter", "0", "Value"));
    xmlParse_->Save();

    EXPECT_FALSE(xmlParse_->RemoveProperty("ClassicAdapter", "ClassicAdapter2", "Value"));
    EXPECT_FALSE(xmlParse_->RemoveProperty("ClassicAdapter", "Value"));
    EXPECT_FALSE(xmlParse_->RemoveProperty("ClassicAdapterErr", "Value"));
}

TEST_F(XmlparseTest, Xmlparse_HasSection)
{
    EXPECT_TRUE(xmlParse_->HasSection("ClassicAdapter"));

    EXPECT_FALSE(xmlParse_->HasSection("ClassicAdapterErr"));

    bool setval = true;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->HasSection("ClassicAdapter", "ClassicAdapter2"));

    EXPECT_FALSE(xmlParse_->HasSection("ClassicAdapter", "ClassicAdapter3"));
}

TEST_F(XmlparseTest, Xmlparse_HasProperty)
{
    EXPECT_TRUE(xmlParse_->HasProperty("ClassicAdapter", "IsValid"));
    EXPECT_FALSE(xmlParse_->HasProperty("ClassicAdapterErr", "IsValid"));
    EXPECT_FALSE(xmlParse_->HasProperty("ClassicAdapter", "IsValidErr"));

    int setval = 0;
    EXPECT_TRUE(xmlParse_->SetValue("ClassicAdapter", "ClassicAdapter2", "IsValid", setval));
    EXPECT_TRUE(xmlParse_->HasProperty("ClassicAdapter", "ClassicAdapter2", "IsValid"));

    EXPECT_FALSE(xmlParse_->HasProperty("ClassicAdapter", "ClassicAdapter3", "IsValid"));
}