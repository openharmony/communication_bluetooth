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
#include <sstream>
#include <algorithm>
#include "classic_mock.h"

ClassicMock *g_xmlmocker;
void registerXmlMock(ClassicMock *instance)
{
    g_xmlmocker = instance;
}
namespace utility {

struct XmlParse::impl {};

XmlParse::XmlParse() : pimpl()
{
    pimpl = std::make_unique<impl>();
}

XmlParse::~XmlParse()
{
    Save();
}

bool XmlParse::Load(const std::string &path)
{
    return g_xmlmocker->XmlParse_Load();
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
    if (property == "ClassOfDevice") {
        value = 1028;
    }
    return false;
}

bool XmlParse::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, std::string &value)
{
    value = g_xmlmocker->XmlGetValue();
    return true;
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
    if (property == "IOCapability") {
        value = 0x02;
    } else {
        value = 0x03;
    }
    return true;
}

bool XmlParse::GetValue(const std::string &section, const std::string &property, std::string &value)
{
    return false;
}

bool XmlParse::GetValue(const std::string &section, const std::string &property, bool &value)
{
    return false;
}

bool XmlParse::SetValue(const std::string &section, const std::string &property, const int &value)
{
    return false;
}

bool XmlParse::SetValue(const std::string &section, const std::string &property, const std::string &value)
{
    return g_xmlmocker->XmlSetValueString();
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
