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

#ifndef XML_PARSE_H
#define XML_PARSE_H

#include <map>
#include <string>
#include <vector>
#include "base_def.h"

namespace utility {

#define SIZEOF_0X 2
#define BASE_16 16
class XmlParse {
public:
    XmlParse();
    ~XmlParse();
    bool Load(const std::string &path);
    bool Parse();
    bool Save();
    bool GetValue(const std::string &section, const std::string &subSection, const std::string &property, int &value);
    bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, std::string &value);
    bool GetValue(const std::string &section, const std::string &subSection, const std::string &property, bool &value);
    bool SetValue(
        const std::string &section, const std::string &subSection, const std::string &property, const int &value);
    bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const std::string &value);
    bool SetValue(
        const std::string &section, const std::string &subSection, const std::string &property, const bool &value);
    bool HasSection(const std::string &section, const std::string &subSection);
    bool GetSubSections(const std::string &section, std::vector<std::string> &subSections);
    bool HasProperty(const std::string &section, const std::string &subSection, const std::string &property);
    bool RemoveSection(const std::string &section, const std::string &subSection);
    bool RemoveProperty(const std::string &section, const std::string &subSection, const std::string &property);
    bool GetValue(const std::string &section, const std::string &property, int &value);
    bool GetValue(const std::string &section, const std::string &property, std::string &value);
    bool GetValue(const std::string &section, const std::string &property, bool &value);
    bool SetValue(const std::string &section, const std::string &property, const int &value);
    bool SetValue(const std::string &section, const std::string &property, const std::string &value);
    bool SetValue(const std::string &section, const std::string &property, const bool &value);
    bool HasSection(const std::string &section);
    bool HasProperty(const std::string &section, const std::string &property);
    bool RemoveSection(const std::string &section);
    bool RemoveProperty(const std::string &section, const std::string &property);

private:
    std::string filePath_ = "";
};

};  // namespace utility

#endif