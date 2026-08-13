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

#ifndef PBAP_PCE_PARAMETER_H
#define PBAP_PCE_PARAMETER_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace bluetooth {

struct IPbapPhoneBookData {
    std::string name_;
    uint32_t phoneBookSize_ = 0;
    std::vector<uint8_t> vcardData_;
    std::vector<uint8_t> result_;
    std::vector<uint8_t> primaryFolderVersion_;
    std::vector<uint8_t> secondaryFolderVersion_;
    std::vector<uint8_t> databaseIdentifier_;
    std::string deviceAddr_;
    std::string vcardPath_;
    std::string vcardFileName_;
    uint32_t resultLoaded_ = 0;
};

class IPbapPullPhoneBookParam {
public:
    static constexpr uint64_t PROPERTY_SELECTOR = 0x0000000000000001ULL;
    static constexpr uint64_t FORMAT = 0x0000000000000002ULL;
    static constexpr uint64_t MAX_LIST_COUNT = 0x0000000000000004ULL;
    static constexpr uint64_t LIST_START_OFFSET = 0x0000000000000008ULL;
    static constexpr uint64_t RESET_NEW_MISSED_CALLS = 0x0000000000000010ULL;
    static constexpr uint64_t VCARD_SELECTOR = 0x0000000000000020ULL;
    static constexpr uint64_t VCARD_SELECTOROP = 0x0000000000000040ULL;

    IPbapPullPhoneBookParam() = default;
    virtual ~IPbapPullPhoneBookParam() = default;

    virtual bool IsSpecified(uint64_t bit) const { return (specifiedBitset_ & bit) != 0; }
    virtual std::u16string GetName() const { return name_; }
    virtual uint64_t GetPropertySelector() const { return propertySelector_; }
    virtual uint8_t GetFormat() const { return format_; }
    virtual uint16_t GetMaxListCount() const { return maxListCount_; }
    virtual uint16_t GetListStartOffset() const { return listStartOffset_; }
    virtual uint8_t GetResetNewMissedCalls() const { return resetNewMissedCalls_; }
    virtual uint64_t GetvCardSelector() const { return vcardSelector_; }
    virtual uint8_t GetvCardSelectorOp() const { return vcardSelectorOp_; }
    virtual uint64_t GetSpecifiedBitset() const { return specifiedBitset_; }

    virtual void SetName(const std::u16string &name) { name_ = name; }
    virtual void SetPropertySelector(uint64_t selector) { propertySelector_ = selector; }
    virtual void SetFormat(uint8_t format) { format_ = format; }
    virtual void SetMaxListCount(uint16_t count) { maxListCount_ = count; }
    virtual void SetListStartOffset(uint16_t offset) { listStartOffset_ = offset; }
    virtual void SetResetNewMissedCalls(uint8_t reset) { resetNewMissedCalls_ = reset; }
    virtual void SetvCardSelector(uint64_t selector) { vcardSelector_ = selector; }
    virtual void SetvCardSelectorOp(uint8_t op) { vcardSelectorOp_ = op; }
    virtual void SetSpecifiedBitset(uint64_t bitset) { specifiedBitset_ = bitset; }

    virtual std::string ToDebugString() const { return ""; }

private:
    std::u16string name_ {};
    uint64_t propertySelector_ = 0;
    uint8_t format_ = 0;
    uint16_t maxListCount_ = 0;
    uint16_t listStartOffset_ = 0;
    uint8_t resetNewMissedCalls_ = 0;
    uint64_t vcardSelector_ = 0;
    uint8_t vcardSelectorOp_ = 0;
    uint64_t specifiedBitset_ = 0;
};

class IPbapPullvCardListingParam {
public:
    static constexpr uint64_t ORDER = 0x0000000000000001ULL;
    static constexpr uint64_t SEARCH_VALUE = 0x0000000000000002ULL;
    static constexpr uint64_t SEARCH_PROPERTY = 0x0000000000000004ULL;
    static constexpr uint64_t MAX_LIST_COUNT = 0x0000000000000008ULL;
    static constexpr uint64_t LIST_START_OFFSET = 0x0000000000000010ULL;
    static constexpr uint64_t RESET_NEW_MISSED_CALLS = 0x0000000000000020ULL;
    static constexpr uint64_t VCARD_SELECTOR = 0x0000000000000040ULL;
    static constexpr uint64_t VCARD_SELECTOROP = 0x0000000000000080ULL;

    IPbapPullvCardListingParam() = default;
    virtual ~IPbapPullvCardListingParam() = default;

    virtual bool IsSpecified(uint64_t bit) const { return (specifiedBitset_ & bit) != 0; }
    virtual std::u16string GetName() const { return name_; }
    virtual uint8_t GetOrder() const { return order_; }
    virtual std::string GetSearchValue() const { return searchValue_; }
    virtual uint8_t GetSearchProperty() const { return searchProperty_; }
    virtual uint16_t GetMaxListCount() const { return maxListCount_; }
    virtual uint16_t GetListStartOffset() const { return listStartOffset_; }
    virtual uint8_t GetResetNewMissedCalls() const { return resetNewMissedCalls_; }
    virtual uint64_t GetvCardSelector() const { return vcardSelector_; }
    virtual uint8_t GetvCardSelectorOp() const { return vcardSelectorOp_; }
    virtual uint64_t GetSpecifiedBitset() const { return specifiedBitset_; }

    virtual void SetName(const std::u16string &name) { name_ = name; }
    virtual void SetOrder(uint8_t order) { order_ = order; }
    virtual void SetSearchValue(const std::string &value) { searchValue_ = value; }
    virtual void SetSearchProperty(uint8_t property) { searchProperty_ = property; }
    virtual void SetMaxListCount(uint16_t count) { maxListCount_ = count; }
    virtual void SetListStartOffset(uint16_t offset) { listStartOffset_ = offset; }
    virtual void SetResetNewMissedCalls(uint8_t reset) { resetNewMissedCalls_ = reset; }
    virtual void SetvCardSelector(uint64_t selector) { vcardSelector_ = selector; }
    virtual void SetvCardSelectorOp(uint8_t op) { vcardSelectorOp_ = op; }
    virtual void SetSpecifiedBitset(uint64_t bitset) { specifiedBitset_ = bitset; }

    virtual std::string ToDebugString() const { return ""; }

private:
    std::u16string name_ {};
    uint8_t order_ = 0;
    std::string searchValue_ {};
    uint8_t searchProperty_ = 0;
    uint16_t maxListCount_ = 0;
    uint16_t listStartOffset_ = 0;
    uint8_t resetNewMissedCalls_ = 0;
    uint64_t vcardSelector_ = 0;
    uint8_t vcardSelectorOp_ = 0;
    uint64_t specifiedBitset_ = 0;
};

class IPbapPullvCardEntryParam {
public:
    static constexpr uint64_t PROPERTY_SELECTOR = 0x0000000000000001ULL;
    static constexpr uint64_t FORMAT = 0x0000000000000002ULL;

    IPbapPullvCardEntryParam() = default;
    virtual ~IPbapPullvCardEntryParam() = default;

    virtual bool IsSpecified(uint64_t bit) const { return (specifiedBitset_ & bit) != 0; }
    virtual std::u16string GetName() const { return name_; }
    virtual uint64_t GetPropertySelector() const { return propertySelector_; }
    virtual uint8_t GetFormat() const { return format_; }
    virtual uint64_t GetSpecifiedBitset() const { return specifiedBitset_; }

    virtual void SetName(const std::u16string &name) { name_ = name; }
    virtual void SetPropertySelector(uint64_t selector) { propertySelector_ = selector; }
    virtual void SetFormat(uint8_t format) { format_ = format; }
    virtual void SetSpecifiedBitset(uint64_t bitset) { specifiedBitset_ = bitset; }

    virtual std::string ToDebugString() const { return ""; }

private:
    std::u16string name_ {};
    uint64_t propertySelector_ = 0;
    uint8_t format_ = 0;
    uint64_t specifiedBitset_ = 0;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_PARAMETER_H
