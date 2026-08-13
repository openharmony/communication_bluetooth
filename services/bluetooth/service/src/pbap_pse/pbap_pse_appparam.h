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

#ifndef PBAP_PSE_APPPARAM_H
#define PBAP_PSE_APPPARAM_H

#include <cstdint>
#include <string>
#include "log.h"

namespace OHOS {
namespace bluetooth {

// TRIPLET_TAGID
constexpr uint8_t TRIPLET_TAGID_ORDER = 0x01;
constexpr uint8_t TRIPLET_TAGID_SEARCH_VALUE = 0x02;
constexpr uint8_t TRIPLET_TAGID_SEARCH_ATTRIBUTE = 0x03;
constexpr uint8_t TRIPLET_TAGID_MAXLISTCOUNT = 0x04;
constexpr uint8_t TRIPLET_TAGID_LISTSTARTOFFSET = 0x05;
constexpr uint8_t TRIPLET_TAGID_PROPERTY_SELECTOR = 0x06;
constexpr uint8_t TRIPLET_TAGID_FORMAT = 0x07;
constexpr uint8_t TRIPLET_TAGID_PHONEBOOKSIZE = 0x08;
constexpr uint8_t TRIPLET_TAGID_NEWMISSEDCALLS = 0x09; // only used in "mch" in response
constexpr uint8_t TRIPLET_TAGID_SUPPORTEDFEATURE = 0x10;
constexpr uint8_t TRIPLET_TAGID_PRIMARYVERSIONCOUNTER = 0x0A;
constexpr uint8_t TRIPLET_TAGID_SECONDARYVERSIONCOUNTER = 0x0B;
constexpr uint8_t TRIPLET_TAGID_VCARDSELECTOR = 0x0C;
constexpr uint8_t TRIPLET_TAGID_DATABASEIDENTIFIER = 0x0D;
constexpr uint8_t TRIPLET_TAGID_VCARDSELECTOROPERATOR = 0x0E;
constexpr uint8_t TRIPLET_TAGID_RESET_NEW_MISSED_CALLS = 0x0F;

// TRIPLET_LENGTH
constexpr uint8_t TRIPLET_LENGTH_ORDER = 1;
constexpr uint8_t TRIPLET_LENGTH_SEARCH_ATTRIBUTE = 1;
constexpr uint8_t TRIPLET_LENGTH_MAXLISTCOUNT = 2;
constexpr uint8_t TRIPLET_LENGTH_LISTSTARTOFFSET = 2;
constexpr uint8_t TRIPLET_LENGTH_PROPERTY_SELECTOR = 8;
constexpr uint8_t TRIPLET_LENGTH_FORMAT = 1;
constexpr uint8_t TRIPLET_LENGTH_PHONEBOOKSIZE = 2;
constexpr uint8_t TRIPLET_LENGTH_NEWMISSEDCALLS = 1;
constexpr uint8_t TRIPLET_LENGTH_SUPPORTEDFEATURE = 4;
constexpr uint8_t TRIPLET_LENGTH_PRIMARYVERSIONCOUNTER = 16;
constexpr uint8_t TRIPLET_LENGTH_SECONDARYVERSIONCOUNTER = 16;
constexpr uint8_t TRIPLET_LENGTH_VCARDSELECTOR = 8;
constexpr uint8_t TRIPLET_LENGTH_DATABASEIDENTIFIER = 16;
constexpr uint8_t TRIPLET_LENGTH_VCARDSELECTOROPERATOR = 1;
constexpr uint8_t TRIPLET_LENGTH_RESETNEWMISSEDCALLS = 1;

constexpr int32_t PBAP_APP_PARAM_TAG_LEN = 2;
constexpr int32_t PBAP_CALLLOG_NUM_LIMIT = 50;

constexpr int32_t BASE_BIT_LEN_8 = 8;

enum SearchAttribute {
    SEARCH_ATTR_NAME = 0,
    SEARCH_ATTR_NUMBER = 1,
    SEARCH_ATTR_SOUND = 2,
};

enum OrderType {
    ORDER_BY_INDEX = 0,
    ORDER_BY_ALPHANUMERIC = 1,
    ORDER_BY_PHONETIC = 2,
};

enum FormatVcardVersion {
    VCARD_FORMAT_VERSION_21 = 0,
    VCARD_FORMAT_VERSION_30 = 1,
};

enum VcardSelectorOperator {
    VCARD_SELECTOR_OPERATOR_OR = 0,
    VCARD_SELECTOR_OPERATOR_AND = 1,
};

class PbapPseAppParam {
public:
    PbapPseAppParam() {}
    bool ParseApplicationParameter(const std::vector<uint8_t> &appParam);

private:
    bool ParseSelectorTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseSupportedFeatureTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseOderTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseSeachValueTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseSeachAttributeTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseMaxListCountTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseListStartOffsetTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseFormatTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseVcardSelectorTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);
    bool ParseVcardSelectorOperaterTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos);

public:
    uint8_t searchAttribute_ = SEARCH_ATTR_NAME;
    uint8_t order_ = ORDER_BY_INDEX;
    uint8_t vcardFormat_ = VCARD_FORMAT_VERSION_21;
    uint8_t vCardSelectorOperator_ = VCARD_SELECTOR_OPERATOR_OR;
    uint16_t maxListCount_ = 0xFFFF;
    uint16_t listStartOffset_ = 0;
    uint32_t supportedFeature_ = 0;
    uint64_t propertySelector_ = 0;
    uint64_t vCardSelector_ = 0;
    std::string searchValue_ = "";
    std::vector<uint8_t> callHistoryVersionCounter_ {};

    bool ignorefilter_ = true;
    bool isNeedPbSize_ = false;
    int32_t contentType_ = 0;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PSE_APPPARAM_H