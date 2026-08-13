/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_pbap_vcard_filter"
#endif

#include "pbap_pse_vcard_filter.h"
#include "log.h"
#include <sstream>
#include <cctype>
#include "hitrace_meter.h"
#include "string_ex.h"

namespace OHOS {
namespace bluetooth {
constexpr int32_t PBAP_FILTER_X_LEN = 2;

const std::vector<Filter> FILTER_VEC = {
    // pos, prop, onlyCheckV21, excludeForV21
    Filter(1, "FN", true, false),
    Filter(3, "PHOTO", false, false),
    Filter(4, "BDAY", false, false),
    Filter(5, "ADR", false, false),
    Filter(8, "EMAIL", false, false),
    Filter(12, "TITLE", false, false),
    Filter(16, "ORG", false, false),
    Filter(17, "NOTE", false, false),
    Filter(19, "SOUND", false, false),
    Filter(20, "URL", false, false),
    Filter(23, "NICKNAME", false, true),
    Filter(28, "X-IRMC-CALL-DATETIME", false, false)
};

const std::vector<std::string> BASIC_PROPS = {
    "FN", "N", "TEL", "PHOTO", "BEGIN", "END", "VERSION", "X-IRMC-CALL-DATETIME"};

std::string PbabPseVcardFilter::Apply(std::string &vCard, int32_t vcardType,
    uint64_t propertySelector, int32_t shareType)
{
    HITRACE_METER_FMT(BT_TRACE_TAG, "vCard size:%zu", vCard.size());
    if (propertySelector == 0) {
        HILOGI("propertySelector: %{public}d", propertySelector);
        return vCard;
    }
    std::string delimiter = "\n";
    std::vector<std::string> lines = split(vCard, delimiter);
    std::string filteredVCard;
    bool filteredIn = false;

    for (std::string &line : lines) {
        if (line.empty()) {
            continue;
        }
        // Check whether the current property is changing (ignoring multi-line properties)
        // and determine if the current property is filtered in.
        if (!std::isspace(line[0]) && (line.substr(0, 1) != "=") && (line.substr(0, 1) != ";")) {
            SetFilteredIn(vcardType, propertySelector, shareType, line, filteredIn);
        }

        // Build filtered vCard
        if (filteredIn) {
            filteredVCard.append(line).append(delimiter);
        }
    }
    return filteredVCard;
}

void PbabPseVcardFilter::SetFilteredIn(int32_t vcardType, uint64_t propertySelector, int32_t shareType,
    std::string &line, bool &filteredIn)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::regex pattern("[;:]");
    std::string currentProp = splitByRegex(line, pattern)[0];
    filteredIn = true;
    for (const Filter &filter : FILTER_VEC) {
        if (filter.prop == currentProp) {
            filteredIn = IsFilteredIn(filter, vcardType, propertySelector);
            break;
        }
    }

    if (currentProp.length() > PBAP_FILTER_X_LEN && currentProp.substr(0, PBAP_FILTER_X_LEN).compare("X-") == 0) {
        filteredIn = false;
        if (currentProp == "X-IRMC-CALL-DATETIME") {
            filteredIn = true;
        }
    }
    if (shareType == 0 && std::find(BASIC_PROPS.begin(), BASIC_PROPS.end(), currentProp) == BASIC_PROPS.end()) {
        filteredIn = false;
    }
}

bool PbabPseVcardFilter::IsFilteredIn(const Filter &filter, int32_t vcardType, uint64_t propertySelector)
{
    if (vcardType != VCARD_FORMAT_VERSION_21 && filter.onlyCheckV21) {
        return true;
    }
    if (vcardType == VCARD_FORMAT_VERSION_21 && filter.excludeForV21) {
        return false;
    }

    return ((propertySelector >> filter.pos) & 0x01) != 0;
}

std::vector<std::string> PbabPseVcardFilter::split(std::string &str, std::string delimiter)
{
    HITRACE_METER(BT_TRACE_TAG);

    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    // 添加最后一个子字符串
    result.push_back(str.substr(start));

    return result;
}

std::vector<std::string> PbabPseVcardFilter::splitByRegex(const std::string &str, const std::regex &pattern)
{
    std::vector<std::string> result(
        std::sregex_token_iterator(str.begin(), str.end(), pattern, -1),
        std::sregex_token_iterator()
    );
    return result;
}
}  // namespace bluetooth
}  // namespace OHOS