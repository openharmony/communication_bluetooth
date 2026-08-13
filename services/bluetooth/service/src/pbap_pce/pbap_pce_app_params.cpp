/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pbap_pce_app_params.h"
#include <cstring>
#include <endian.h>

namespace OHOS {
namespace bluetooth {
const std::map<uint8_t, int> PbapPceAppParams::LENS_MAP = {
    {ORDER, 1},
    {SEARCH_VALUE, -1},
    {SEARCH_PROPERTY, 1},
    {MAX_LIST_COUNT, 2},
    {LIST_START_OFFSET, 2},
    {PROPERTY_SELECTOR, 8},
    {FORMAT, 1},
    {PHONEBOOK_SIZE, 2},
    {NEW_MISSED_CALLS, 1},
    {PRIMARY_FOLDER_VERSION, 16},
    {SECONDARY_FOLDER_VERSION, 16},
    {VCARD_SELECTOR, 8},
    {DATABASE_IDENTIFIER, 16},
    {VCARD_SELECTOR_OPERATOR, 1},
    {RESET_NEW_MISSED_CALLS, 1},
    {PBAP_SUPPORTED_FEATURES, 4}
};

const std::string &PbapPceAppParams::GetName() const
{
    return name_;
}

uint8_t PbapPceAppParams::GetOrder() const
{
    return order_;
}

const std::vector<uint8_t> &PbapPceAppParams::GetSearchValueUtf8() const
{
    return searchValueUtf8_;
}

uint8_t PbapPceAppParams::GetSearchProperty() const
{
    return searchProperty_;
}

uint16_t PbapPceAppParams::GetMaxListCount() const
{
    return maxListCount_;
}

uint16_t PbapPceAppParams::GetListStartOffset() const
{
    return listStartOffset_;
}

uint64_t PbapPceAppParams::GetPropertySelector() const
{
    return propertySelector_;
}

uint8_t PbapPceAppParams::GetFormat() const
{
    return format_;
}

uint16_t PbapPceAppParams::GetPhonebookSize() const
{
    return phonebookSize_;
}

uint8_t PbapPceAppParams::GetNewMissedCalls() const
{
    return newMissedCalls_;
}

const std::vector<uint8_t> &PbapPceAppParams::GetPrimaryFolderVersion() const
{
    return primaryFolderVer_;
}

const std::vector<uint8_t> &PbapPceAppParams::GetSecondaryFolderVersion() const
{
    return secondaryFolderVer_;
}

uint64_t PbapPceAppParams::GetVcardSelector() const
{
    return vcardSelector_;
}

const std::vector<uint8_t> &PbapPceAppParams::GetDatabaseIdentifier() const
{
    return databaseIdentifier_;
}

uint8_t PbapPceAppParams::GetVcardSelectorOperator() const
{
    return vcardSelectorOp_;
}

uint8_t PbapPceAppParams::GetResetNewMissedCalls() const
{
    return resetNewMissedCalls_;
}

uint32_t PbapPceAppParams::GetPbapSupportedFeatures() const
{
    return pbapSupportedFeatures_;
}

bool PbapPceAppParams::HasOrder() const { return hasOrder_; }
bool PbapPceAppParams::HasSearchProperty() const { return hasSearchProperty_; }
bool PbapPceAppParams::HasMaxListCount() const { return hasMaxListCount_; }
bool PbapPceAppParams::HasListStartOffset() const { return hasListStartOffset_; }
bool PbapPceAppParams::HasPropertySelector() const { return hasPropertySelector_; }
bool PbapPceAppParams::HasFormat() const { return hasFormat_; }
bool PbapPceAppParams::HasPhonebookSize() const { return hasPhonebookSize_; }
bool PbapPceAppParams::HasNewMissedCalls() const { return hasNewMissedCalls_; }
bool PbapPceAppParams::HasVcardSelector() const { return hasVcardSelector_; }
bool PbapPceAppParams::HasVcardSelectorOperator() const { return hasVcardSelectorOp_; }
bool PbapPceAppParams::HasResetNewMissedCalls() const { return hasResetNewMissedCalls_; }
bool PbapPceAppParams::HasPbapSupportedFeatures() const { return hasPbapSupportedFeatures_; }

void PbapPceAppParams::SetName(const std::string &val)
{
    name_ = val;
}

void PbapPceAppParams::SetOrder(uint8_t val)
{
    order_ = val;
    hasOrder_ = true;
}

void PbapPceAppParams::SetSearchValueUtf8(const std::vector<uint8_t> &val)
{
    searchValueUtf8_ = val;
}

void PbapPceAppParams::SetSearchProperty(uint8_t val)
{
    searchProperty_ = val;
    hasSearchProperty_ = true;
}

void PbapPceAppParams::SetMaxListCount(uint16_t val)
{
    maxListCount_ = val;
    hasMaxListCount_ = true;
}

void PbapPceAppParams::SetListStartOffset(uint16_t val)
{
    listStartOffset_ = val;
    hasListStartOffset_ = true;
}

void PbapPceAppParams::SetPropertySelector(uint64_t val)
{
    propertySelector_ = val;
    hasPropertySelector_ = true;
}

void PbapPceAppParams::SetFormat(uint8_t val)
{
    format_ = val;
    hasFormat_ = true;
}

void PbapPceAppParams::SetPhonebookSize(uint16_t val)
{
    phonebookSize_ = val;
    hasPhonebookSize_ = true;
}

void PbapPceAppParams::SetNewMissedCalls(uint8_t val)
{
    newMissedCalls_ = val;
    hasNewMissedCalls_ = true;
}

void PbapPceAppParams::SetPrimaryFolderVersion(const std::vector<uint8_t> &val)
{
    primaryFolderVer_ = val;
}

void PbapPceAppParams::SetSecondaryFolderVersion(const std::vector<uint8_t> &val)
{
    secondaryFolderVer_ = val;
}

void PbapPceAppParams::SetVcardSelector(uint64_t val)
{
    vcardSelector_ = val;
    hasVcardSelector_ = true;
}

void PbapPceAppParams::SetDatabaseIdentifier(const std::vector<uint8_t> &val)
{
    databaseIdentifier_ = val;
}

void PbapPceAppParams::SetVcardSelectorOperator(uint8_t val)
{
    vcardSelectorOp_ = val;
    hasVcardSelectorOp_ = true;
}

void PbapPceAppParams::SetResetNewMissedCalls(uint8_t val)
{
    resetNewMissedCalls_ = val;
    hasResetNewMissedCalls_ = true;
}

void PbapPceAppParams::SetPbapSupportedFeatures(uint32_t val)
{
    pbapSupportedFeatures_ = val;
    hasPbapSupportedFeatures_ = true;
}

void PbapPceAppParams::AddToObexHeader(ObexHeader &hdr) const
{
    ObexTlvParamters appParams;

    if (hasOrder_) {
        appParams.AppendTlvtriplet(TlvTriplet(ORDER, order_));
    }
    if (!searchValueUtf8_.empty()) {
        appParams.AppendTlvtriplet(TlvTriplet(SEARCH_VALUE, static_cast<uint8_t>(searchValueUtf8_.size()),
            searchValueUtf8_.data()));
    }
    if (hasSearchProperty_) {
        appParams.AppendTlvtriplet(TlvTriplet(SEARCH_PROPERTY, searchProperty_));
    }
    if (hasMaxListCount_) {
        appParams.AppendTlvtriplet(TlvTriplet(MAX_LIST_COUNT, htobe16(maxListCount_)));
    }
    if (hasListStartOffset_) {
        appParams.AppendTlvtriplet(TlvTriplet(LIST_START_OFFSET, htobe16(listStartOffset_)));
    }
    if (hasPropertySelector_) {
        appParams.AppendTlvtriplet(TlvTriplet(PROPERTY_SELECTOR, htobe64(propertySelector_)));
    }
    if (hasFormat_) {
        appParams.AppendTlvtriplet(TlvTriplet(FORMAT, format_));
    }
    if (hasResetNewMissedCalls_) {
        appParams.AppendTlvtriplet(TlvTriplet(RESET_NEW_MISSED_CALLS, resetNewMissedCalls_));
    }
    if (hasVcardSelector_) {
        appParams.AppendTlvtriplet(TlvTriplet(VCARD_SELECTOR, htobe64(vcardSelector_)));
    }
    if (hasVcardSelectorOp_) {
        appParams.AppendTlvtriplet(TlvTriplet(VCARD_SELECTOR_OPERATOR, vcardSelectorOp_));
    }
    if (hasPbapSupportedFeatures_) {
        appParams.AppendTlvtriplet(TlvTriplet(PBAP_SUPPORTED_FEATURES, htobe32(pbapSupportedFeatures_)));
    }

    hdr.AppendItemAppParams(appParams);
}

PbapPceAppParams PbapPceAppParams::FromObexHeader(const ObexHeader &hdr)
{
    PbapPceAppParams result;
    auto opt = hdr.GetAppParams();
    if (!opt) {
        return result;
    }
    for (auto &tlv : opt->GetTlvTriplets()) {
        if (tlv == nullptr) {
            continue;
        }
        switch (tlv->GetTagId()) {
            case ORDER:
                if (tlv->GetLen() >= 1) {
                    result.SetOrder(*tlv->GetVal());
                }
                break;
            case SEARCH_VALUE: {
                const uint8_t *val = tlv->GetVal();
                uint8_t len = tlv->GetLen();
                result.SetSearchValueUtf8(std::vector<uint8_t>(val, val + len));
                break;
            }
            case SEARCH_PROPERTY:
                if (tlv->GetLen() >= 1) {
                    result.SetSearchProperty(*tlv->GetVal());
                }
                break;
            case MAX_LIST_COUNT:
                result.SetMaxListCount(tlv->GetUint16());
                break;
            case LIST_START_OFFSET:
                result.SetListStartOffset(tlv->GetUint16());
                break;
            case PROPERTY_SELECTOR:
                result.SetPropertySelector(tlv->GetUint64());
                break;
            case FORMAT:
                if (tlv->GetLen() >= 1) {
                    result.SetFormat(*tlv->GetVal());
                }
                break;
            case PHONEBOOK_SIZE:
                result.SetPhonebookSize(tlv->GetUint16());
                break;
            case NEW_MISSED_CALLS:
                if (tlv->GetLen() >= 1) {
                    result.SetNewMissedCalls(*tlv->GetVal());
                }
                break;
            case PRIMARY_FOLDER_VERSION: {
                const uint8_t *val = tlv->GetVal();
                uint8_t len = tlv->GetLen();
                result.SetPrimaryFolderVersion(std::vector<uint8_t>(val, val + len));
                break;
            }
            case SECONDARY_FOLDER_VERSION: {
                const uint8_t *val = tlv->GetVal();
                uint8_t len = tlv->GetLen();
                result.SetSecondaryFolderVersion(std::vector<uint8_t>(val, val + len));
                break;
            }
            case VCARD_SELECTOR:
                result.SetVcardSelector(tlv->GetUint64());
                break;
            case DATABASE_IDENTIFIER: {
                const uint8_t *val = tlv->GetVal();
                uint8_t len = tlv->GetLen();
                result.SetDatabaseIdentifier(std::vector<uint8_t>(val, val + len));
                break;
            }
            case VCARD_SELECTOR_OPERATOR:
                if (tlv->GetLen() >= 1) {
                    result.SetVcardSelectorOperator(*tlv->GetVal());
                }
                break;
            case RESET_NEW_MISSED_CALLS:
                if (tlv->GetLen() >= 1) {
                    result.SetResetNewMissedCalls(*tlv->GetVal());
                }
                break;
            case PBAP_SUPPORTED_FEATURES:
                result.SetPbapSupportedFeatures(tlv->GetUint32());
                break;
            default:
                break;
        }
    }
    return result;
}

const uint8_t PbapPceAppParams::ORDER;
const uint8_t PbapPceAppParams::SEARCH_VALUE;
const uint8_t PbapPceAppParams::SEARCH_PROPERTY;
const uint8_t PbapPceAppParams::MAX_LIST_COUNT;
const uint8_t PbapPceAppParams::LIST_START_OFFSET;
const uint8_t PbapPceAppParams::PROPERTY_SELECTOR;
const uint8_t PbapPceAppParams::FORMAT;
const uint8_t PbapPceAppParams::PHONEBOOK_SIZE;
const uint8_t PbapPceAppParams::NEW_MISSED_CALLS;
const uint8_t PbapPceAppParams::PRIMARY_FOLDER_VERSION;
const uint8_t PbapPceAppParams::SECONDARY_FOLDER_VERSION;
const uint8_t PbapPceAppParams::VCARD_SELECTOR;
const uint8_t PbapPceAppParams::DATABASE_IDENTIFIER;
const uint8_t PbapPceAppParams::VCARD_SELECTOR_OPERATOR;
const uint8_t PbapPceAppParams::RESET_NEW_MISSED_CALLS;
const uint8_t PbapPceAppParams::PBAP_SUPPORTED_FEATURES;
}  // namespace bluetooth
}  // namespace OHOS
