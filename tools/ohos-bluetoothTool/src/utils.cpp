/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "utils.h"
#include <map>

namespace OHOS {
namespace Bluetooth {
namespace Utils {

static const std::map<int, std::string> STATE_STRING_MAP = {
    {0, "STATE_TURNING_ON"},
    {1, "STATE_TURN_ON"},
    {2, "STATE_TURNING_OFF"},
    {3, "STATE_TURN_OFF"},
};

static const std::map<int, std::string> STATE_DESC_MAP = {
    {0, "Enabling"},
    {1, "Bluetooth enabled"},
    {2, "Disabling"},
    {3, "Bluetooth Disabled"},
};

std::string BtStateToString(int state)
{
    auto it = STATE_STRING_MAP.find(state);
    if (it != STATE_STRING_MAP.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

std::string BtStateToDescription(int state)
{
    auto it = STATE_DESC_MAP.find(state);
    if (it != STATE_DESC_MAP.end()) {
        return it->second;
    }
    return "Unknown state";
}

int GetStateCode(int state)
{
    return state;
}

}
}
}