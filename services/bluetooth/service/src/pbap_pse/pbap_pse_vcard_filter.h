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

#ifndef PBAP_PSE_VCARD_FILTER
#define PBAP_PSE_VCARD_FILTER

#include <vector>
#include <string>
#include <regex>
#include "pbap_pse_appparam.h"

namespace OHOS {
namespace bluetooth {

struct Filter {
    uint8_t pos;
    std::string prop;
    bool onlyCheckV21;
    bool excludeForV21;

    Filter(uint8_t pos, std::string prop, bool onlyCheckV21, bool excludeForV21)
        : pos(pos),
        prop(prop),
        onlyCheckV21(onlyCheckV21),
        excludeForV21(excludeForV21) {}
};

class PbabPseVcardFilter {
public:
    PbabPseVcardFilter() {};
    ~PbabPseVcardFilter() {};
    std::string Apply(std::string &vCard, int32_t vcardType, uint64_t propertySelector, int32_t shareType);
    std::vector<std::string> split(std::string &str, std::string delimiter);
    std::vector<std::string> splitByRegex(const std::string &str, const std::regex &pattern);

private:
    void SetFilteredIn(int32_t vcardType, uint64_t propertySelector, int32_t shareType,
        std::string &line, bool &filteredIn);
    bool IsFilteredIn(const Filter &filter, int32_t vcardType, uint64_t propertySelector);
};

} // namespace bluetooth
} // namespace OHOS

#endif // PBAP_PSE_VCARD_FILTER