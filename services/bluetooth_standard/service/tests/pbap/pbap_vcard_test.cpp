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

#include "stub/vcard_util.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace stub;
using namespace testing;

TEST(pbap_vcard_test, vcard30)
{
    std::string str = "BEGIN:VCARD\n"
                      "VERSION:3.0\n"
                      "N:Gump;Forrest;;Mr.;\n"
                      "FN;CHARSET=UTF-8:Forrest Gump\n"
                      "ADR;TYPE=HOME,text;LABEL=\"42 Plantation St.\\nBaytown\\, LA 30314\\nUnited States of "
                      "America\":;;Plantation St.;Baytown;LA;30314;United States of America\n"
                      "END:VCARD\n"
                      "BEGIN:VCARD\n"
                      "VERSION:3.0\n"
                      "N:Gump;Forrest;;Mr.;\n"
                      "FN;CHARSET=UTF-8:Forrest Gump\n"
                      "TEL;TYPE=CELL,HOME:+8613112341234\n"
                      "ADR;TYPE=HOME,text;LABEL=\"42 Plantation St.\\nBaytown\\, LA 30314\\nUnited States of "
                      "America\":;;Plantation St.;Baytown;LA;30314;United States of America\n"
                      "END:VCARD";

    std::stringstream s;
    s << str;
    std::vector<VCard> cards = VCardUtil::Parse(s);

    EXPECT_THAT(cards.size(), 2);
    EXPECT_THAT(cards[0].GetVersion(), VCardVersion::VER_3_0);
    auto vProps = cards[0].Properties();
    EXPECT_THAT(vProps.size(), 4);

    EXPECT_THAT(cards[1].GetVersion(), VCardVersion::VER_3_0);
    vProps = cards[1].Properties();
    EXPECT_THAT(vProps.size(), 5);

    std::cout << "------Output check start------" << std::endl;
    std::vector<uint8_t> listBuf;
    for (auto card : cards) {
        auto lists = VCardUtil::Build(card, VCardVersion::VER_2_1);
        for (auto list : lists) {
            std::cout << list;
        }
        listBuf.insert(listBuf.end(), lists.begin(), lists.end());
    }

    std::vector<VCard> cards2 = VCardUtil::Parse(listBuf);

    for (auto card : cards2) {
        auto lists = VCardUtil::Build(card, VCardVersion::VER_3_0);
        for (auto list : lists) {
            std::cout << list;
        }
    }
    std::cout << "------Output check end------" << std::endl;
}

TEST(pbap_vcard_test, vcard21)
{
    std::string str =
        "BEGIN:VCARD\r\n"
        "VERSION:2.1\r\n"
        "FN:Jean Dupont\r\n"
        "N:Dupont;Jean;;;\r\n"
        "ADR;WORK;PREF:;;100 Waters Edge;Baytown;LA;30314;United States of America\r\n"
        "TEL;CELL;PREF;LABEL=\"42 Plantation St.\\nBaytown\\, LA 30314\\nUnited States of !\":+1234 56789\r\n"
        "EMAIL;INTERNET:jean.dupont@example.com\r\n"
        "X-BT-UCI;WORK:lync:jean.dupont@example.com\r\n"
        "X-BT-UCI:skype:jean18061977\r\n"
        "X-BT-UCI:skype:jeand_private1815\r\n"
        "END:VCARD\r\n"
        "BEGIN:VCARD\r\n"
        "VERSION:2.1\r\n"
        "FN:Jean Dupont\r\n"
        "N:Dupont;Jean;;;\r\n"
        "ADR;WORK;PREF:;;100 Waters Edge;Baytown;LA;30314;United States of America\r\n"
        "TEL;CELL;PREF;LABEL=\"42 Plantation St.\\nBaytown\\, LA 30314\\nUnited States of !\":+1234 56789\r\n"
        "EMAIL;INTERNET:jean.dupont@example.com\r\n"
        "X-BT-UCI;WORK:lync:jean.dupont@example.com\r\n"
        "X-BT-UCI:skype:jean18061977\r\n"
        "X-BT-UCI:skype:jeand_private1815\r\n"
        "END:VCARD";

    std::stringstream s;
    s << str;
    std::vector<VCard> cards = VCardUtil::Parse(s);

    EXPECT_THAT(cards.size(), 2);
    EXPECT_THAT(cards[0].GetVersion(), VCardVersion::VER_2_1);
    auto vProps = cards[0].Properties();
    EXPECT_THAT(vProps.size(), 9);

    EXPECT_THAT(cards[1].GetVersion(), VCardVersion::VER_2_1);
    vProps = cards[1].Properties();
    EXPECT_THAT(vProps.size(), 9);

    std::cout << "------Output check start------" << std::endl;
    std::vector<uint8_t> listBuf;
    for (auto card : cards) {
        auto lists = VCardUtil::Build(card, VCardVersion::VER_2_1);
        for (auto list : lists) {
            std::cout << list;
        }
        listBuf.insert(listBuf.end(), lists.begin(), lists.end());
    }

    std::vector<VCard> cards2 = VCardUtil::Parse(listBuf);

    for (auto card : cards2) {
        auto lists = VCardUtil::Build(card, VCardVersion::VER_3_0);
        for (auto list : lists) {
            std::cout << list;
        }
    }
    std::cout << "------Output check end------" << std::endl;
}

TEST(pbap_vcard_test, VCardPropertyVCard21)
{
    // TEL;WORK;VOICE:(111) 555-1212
    VCardVersion ver = VCardVersion::VER_2_1;
    std::string group = "";
    std::string name = "TEL";
    std::string value = "(111) 555-1212";
    std::string paramsStr = "WORK;VOICE";
    VCardProperty p(ver, group, name, value, paramsStr);
    EXPECT_THAT(p.GetGroup(), "");
    EXPECT_THAT(p.GetName(), "TEL");
    EXPECT_THAT(p.GetValues().size(), 1);
    EXPECT_THAT(p.GetParams().size(), 2);

    std::cout << "------Output check start------" << std::endl;
    VCard vcard;
    vcard.SetVersion(ver);
    vcard.AddProperty(p);
    auto lists = VCardUtil::Build(vcard, ver);
    for (auto list : lists) {
        std::cout << list;
    }
    std::cout << "------Output check end------" << std::endl;
}

TEST(pbap_vcard_test, VCardPropertyVCard30)
{
    // TEL;TYPE=HOME,VOICE:(111) 555-1212
    VCardVersion ver = VCardVersion::VER_3_0;
    std::string group = "work";
    std::string name = "TEL";
    std::string value = "(111) 555-1212";
    std::string paramsStr = "TYPE=HOME,VOICE";
    VCardProperty p(ver, group, name, value, paramsStr);
    EXPECT_THAT(p.GetGroup(), "work");
    EXPECT_THAT(p.GetName(), "TEL");
    EXPECT_THAT(p.GetValues().size(), 1);
    EXPECT_THAT(p.GetParams().size(), 2);

    std::cout << "------Output check start------" << std::endl;
    VCard vcard;
    vcard.SetVersion(ver);
    vcard.AddProperty(p);
    auto lists = VCardUtil::Build(vcard, ver);
    for (auto list : lists) {
        std::cout << list;
    }
    std::cout << "------Output check end------" << std::endl;
}
