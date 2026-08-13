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
#define LOG_TAG "bt_service_pbap_pse_appparam"
#endif

#include "pbap_pse_appparam.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {

uint64_t ReadUint64(const std::vector<uint8_t> &values, int32_t pos)
{
    uint64_t retValue = 0;
    if (pos + sizeof(uint64_t) > values.size()) {
        return retValue;
    }
    for (size_t i = pos; i < pos + sizeof(uint64_t); i++) {
        retValue <<= BASE_BIT_LEN_8;
        retValue |= values[i];
    }
    return retValue;
}

uint32_t ReadUint32(const std::vector<uint8_t> &values, int32_t pos)
{
    uint64_t retValue = 0;
    if (pos + sizeof(uint32_t) > values.size()) {
        return retValue;
    }
    for (size_t i = pos; i < pos + sizeof(uint32_t); i++) {
        retValue <<= BASE_BIT_LEN_8;
        retValue |= values[i];
    }
    return retValue;
}

std::string ReadString(const std::vector<uint8_t> &values, int32_t pos, int32_t len)
{
    std::string retStr = "";
    if (pos + len > static_cast<int32_t>(values.size())) {
        HILOGE("fail, pos = %{public}d, length = %{public}d", pos, len);
        return retStr;
    }
    for (int i = pos; i < pos + len; i++) {
        if (values[i] == 0) {
            continue;
        }
        retStr += static_cast<char>(values[i]);
    }
    return retStr;
}

bool PbapPseAppParam::ParseApplicationParameter(const std::vector<uint8_t> &appParam)
{
    int32_t pos = 0;
    bool parseOk = true;
    int32_t paramSize = appParam.size();
    while ((pos < paramSize) && (parseOk)) {
        switch (appParam[pos]) {
            case TRIPLET_TAGID_PROPERTY_SELECTOR:
                parseOk = ParseSelectorTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_SUPPORTEDFEATURE:
                parseOk = ParseSupportedFeatureTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_ORDER:
                parseOk = ParseOderTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_SEARCH_VALUE:
                parseOk = ParseSeachValueTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_SEARCH_ATTRIBUTE:
                parseOk = ParseSeachAttributeTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_MAXLISTCOUNT:
                parseOk = ParseMaxListCountTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_LISTSTARTOFFSET:
                parseOk = ParseListStartOffsetTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_FORMAT:
                parseOk = ParseFormatTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_VCARDSELECTOR:
                parseOk = ParseVcardSelectorTagId(appParam, paramSize, pos);
                break;
            case TRIPLET_TAGID_VCARDSELECTOROPERATOR:
                parseOk = ParseVcardSelectorOperaterTagId(appParam, paramSize, pos);
                break;
            default:
                parseOk = false;
                HILOGE("Parse Application Parameter error");
                break;
        }
    }
    return parseOk;
}

bool PbapPseAppParam::ParseSelectorTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_PROPERTY_SELECTOR) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    propertySelector_ = ReadUint64(appParam, pos);
    if (propertySelector_ != 0) {
        ignorefilter_ = false;
    }
    pos += TRIPLET_LENGTH_PROPERTY_SELECTOR;
    return true;
}

bool PbapPseAppParam::ParseSupportedFeatureTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_SUPPORTEDFEATURE) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    supportedFeature_ = ReadUint32(appParam, pos);
    pos += TRIPLET_LENGTH_SUPPORTEDFEATURE;
    return true;
}

bool PbapPseAppParam::ParseOderTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_ORDER) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    order_ = appParam[pos];
    pos += TRIPLET_LENGTH_ORDER;
    return true;
}

bool PbapPseAppParam::ParseSeachValueTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos++; // length field in triplet, length of search value is variable
    int length = appParam[pos++];
    if (length == 0) {
        return false;
    }
    searchValue_ = ReadString(appParam, pos, length);
    pos += length;
    return true;
}

bool PbapPseAppParam::ParseSeachAttributeTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_SEARCH_ATTRIBUTE) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    searchAttribute_ = appParam[pos];
    pos += TRIPLET_LENGTH_SEARCH_ATTRIBUTE;
    return true;
}

bool PbapPseAppParam::ParseMaxListCountTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_MAXLISTCOUNT) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    if (appParam[pos] == 0 && appParam[pos + 1] == 0) {
        isNeedPbSize_ = true;
    } else {
        maxListCount_ = appParam[pos];
        maxListCount_ = (maxListCount_ << BASE_BIT_LEN_8) + appParam[pos + 1];
    }
    pos += TRIPLET_LENGTH_MAXLISTCOUNT;
    return true;
}

bool PbapPseAppParam::ParseListStartOffsetTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_LISTSTARTOFFSET) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    listStartOffset_ = appParam[pos];
    listStartOffset_ = (listStartOffset_ << BASE_BIT_LEN_8) + appParam[pos + 1];
    pos += TRIPLET_LENGTH_LISTSTARTOFFSET;
    return true;
}

bool PbapPseAppParam::ParseFormatTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_FORMAT) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length field in triplet
    vcardFormat_ = appParam[pos];
    pos += TRIPLET_LENGTH_FORMAT;
    return true;
}

bool PbapPseAppParam::ParseVcardSelectorTagId(const std::vector<uint8_t> &appParam, int32_t paramSize, int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_VCARDSELECTOR) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN;
    vCardSelector_ = ReadUint64(appParam, pos);
    pos += TRIPLET_LENGTH_VCARDSELECTOR;
    return true;
}

bool PbapPseAppParam::ParseVcardSelectorOperaterTagId(const std::vector<uint8_t> &appParam, int32_t paramSize,
    int &pos)
{
    HILOGD("paramSize = %{public}d, pos = %{public}d", paramSize, pos);
    if (paramSize < pos + PBAP_APP_PARAM_TAG_LEN + TRIPLET_LENGTH_VCARDSELECTOROPERATOR) {
        HILOGE("fail, paramSize = %{public}d, pos = %{public}d", paramSize, pos);
        return false;
    }
    pos += PBAP_APP_PARAM_TAG_LEN; // length and tag field in triplet
    vCardSelectorOperator_ = appParam[pos];
    pos += TRIPLET_LENGTH_VCARDSELECTOROPERATOR;
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS