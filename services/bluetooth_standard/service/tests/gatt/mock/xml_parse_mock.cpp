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
#include "gatt_connection_manager.h"

using namespace utility;
bool XmlParse::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, int &value)
{
    return true;
};
bool XmlParse::GetValue(const std::string &section, const std::string &property, bool &value)
{
    const std::string PROPERTY_BLE_MIN_CONNECTION_INTERVAL;
    if (property == PROPERTY_BLE_MIN_CONNECTION_INTERVAL) {
        return true;
    } else {
        return false;
    }
};