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

#include "mock/xml_parse.h"
#include <sstream>
#include <algorithm>
#include "di_mock.h"

extern XmlMock *g_xmlmock;
namespace utility {

XmlParse::XmlParse()
{}

XmlParse::~XmlParse()
{
    Save();
}

bool XmlParse::Load(const std::string &path)
{
    return g_xmlmock->Load();
}

bool XmlParse::Parse()
{
    return true;
}

bool XmlParse::Save()
{
    return true;
}

bool XmlParse::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, int &value)
{
    return false;
}

bool XmlParse::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, std::string &value)
{
    value = "000000000,00001234,";
    return false;
}

bool XmlParse::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, bool &value)
{
    return false;
}

bool XmlParse::SetValue(
    const std::string &section, const std::string &subSection, const std::string &property, const int &value)
{
    return false;
}

bool XmlParse::SetValue(
    const std::string &section, const std::string &subSection, const std::string &property, const std::string &value)
{
    return false;
}

bool XmlParse::SetValue(
    const std::string &section, const std::string &subSection, const std::string &property, const bool &value)
{
    return false;
}

bool XmlParse::HasProperty(const std::string &section, const std::string &subSection, const std::string &property)
{
    return false;
}

bool XmlParse::HasSection(const std::string &section, const std::string &subSection)
{
    return false;
}

bool XmlParse::GetSubSections(const std::string &section, std::vector<std::string> &subSections)
{
    subSections.push_back("00:00:00:00:00:00");
    subSections.push_back("07:00:00:00:00:00");
    return false;
}

bool XmlParse::RemoveSection(const std::string &section, const std::string &subSection)
{
    return false;
}

bool XmlParse::RemoveProperty(const std::string &section, const std::string &subSection, const std::string &property)
{
    return false;
}

bool XmlParse::GetValue(const std::string &section, const std::string &property, int &value)
{
    value = 3;
    return g_xmlmock->GetValue();
}

bool XmlParse::GetValue(const std::string &section, const std::string &property, std::string &value)
{
    return false;
}

bool XmlParse::GetValue(const std::string &section, const std::string &property, bool &value)
{
    return g_xmlmock->GetValue();
}

bool XmlParse::SetValue(const std::string &section, const std::string &property, const int &value)
{
    return false;
}

bool XmlParse::SetValue(const std::string &section, const std::string &property, const std::string &value)
{
    return false;
}

bool XmlParse::SetValue(const std::string &section, const std::string &property, const bool &value)
{
    return false;
}

bool XmlParse::HasProperty(const std::string &section, const std::string &property)
{
    return false;
}

bool XmlParse::HasSection(const std::string &section)
{
    return false;
}

bool XmlParse::RemoveSection(const std::string &section)
{
    return false;
}

bool XmlParse::RemoveProperty(const std::string &section, const std::string &property)
{
    return false;
}
}  // namespace utility
