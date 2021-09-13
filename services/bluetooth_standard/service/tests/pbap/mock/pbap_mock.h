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
#ifndef PBAP_MOCK_H
#define PBAP_MOCK_H
#include <map>
#include <string>

class PbapProfileConfigMock {
public:
    static bool GetValue(const std::string &addr, const std::string &section, const std::string &property, int &value);
    static bool SetValue(const std::string &addr, const std::string &section, const std::string &property, int &value);

private:
    PbapProfileConfigMock() = default;
    virtual ~PbapProfileConfigMock() = default;
    static std::map<std::string, int> phoneBookConfigMap_;
    static std::string MakeKey(const std::string &addr, const std::string &section, const std::string &property);
};

class PbapAdapterConfigMock {
public:
    static bool GetValueInt(const std::string &section, const std::string &property, int &value);
    static bool GetValueBool(const std::string &section, const std::string &property, bool &value);

private:
    PbapAdapterConfigMock() = default;
    virtual ~PbapAdapterConfigMock() = default;
};
#endif