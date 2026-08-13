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

#ifndef PBAP_PCE_APP_PARAMS_H
#define PBAP_PCE_APP_PARAMS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
enum PbapPcePhoneBookStateType {
    PBAP_PCE_PHONEBOOK_STATE_DISCONNECTED = 0,
    PBAP_PCE_PHONEBOOK_STATE_DISCONNECTING,
    PBAP_PCE_PHONEBOOK_STATE_CONNECTING,
    PBAP_PCE_PHONEBOOK_STATE_CONNECTED,
    PBAP_PCE_PHONEBOOK_STATE_DOWNLOADING,
    PBAP_PCE_PHONEBOOK_STATE_DOWNLOADED,
    PBAP_PCE_PHONEBOOK_STATE_DOWNLOAD_ERROR,
};

class PbapPceAppParams {
public:
    PbapPceAppParams() = default;
    virtual ~PbapPceAppParams() = default;

    void AddToObexHeader(ObexHeader &hdr) const;
    static PbapPceAppParams FromObexHeader(const ObexHeader &hdr);

    static const uint8_t ORDER = 0x01;
    static const uint8_t SEARCH_VALUE = 0x02;
    static const uint8_t SEARCH_PROPERTY = 0x03;
    static const uint8_t MAX_LIST_COUNT = 0x04;
    static const uint8_t LIST_START_OFFSET = 0x05;
    static const uint8_t PROPERTY_SELECTOR = 0x06;
    static const uint8_t FORMAT = 0x07;
    static const uint8_t PHONEBOOK_SIZE = 0x08;
    static const uint8_t NEW_MISSED_CALLS = 0x09;
    static const uint8_t PRIMARY_FOLDER_VERSION = 0x0A;
    static const uint8_t SECONDARY_FOLDER_VERSION = 0x0B;
    static const uint8_t VCARD_SELECTOR = 0x0C;
    static const uint8_t DATABASE_IDENTIFIER = 0x0D;
    static const uint8_t VCARD_SELECTOR_OPERATOR = 0x0E;
    static const uint8_t RESET_NEW_MISSED_CALLS = 0x0F;
    static const uint8_t PBAP_SUPPORTED_FEATURES = 0x10;

    const std::string &GetName() const;
    uint8_t GetOrder() const;
    const std::vector<uint8_t> &GetSearchValueUtf8() const;
    uint8_t GetSearchProperty() const;
    uint16_t GetMaxListCount() const;
    uint16_t GetListStartOffset() const;
    uint64_t GetPropertySelector() const;
    uint8_t GetFormat() const;
    uint16_t GetPhonebookSize() const;
    uint8_t GetNewMissedCalls() const;
    const std::vector<uint8_t> &GetPrimaryFolderVersion() const;
    const std::vector<uint8_t> &GetSecondaryFolderVersion() const;
    uint64_t GetVcardSelector() const;
    const std::vector<uint8_t> &GetDatabaseIdentifier() const;
    uint8_t GetVcardSelectorOperator() const;
    uint8_t GetResetNewMissedCalls() const;
    uint32_t GetPbapSupportedFeatures() const;

    bool HasOrder() const;
    bool HasSearchProperty() const;
    bool HasMaxListCount() const;
    bool HasListStartOffset() const;
    bool HasPropertySelector() const;
    bool HasFormat() const;
    bool HasPhonebookSize() const;
    bool HasNewMissedCalls() const;
    bool HasVcardSelector() const;
    bool HasVcardSelectorOperator() const;
    bool HasResetNewMissedCalls() const;
    bool HasPbapSupportedFeatures() const;

    void SetName(const std::string &name);
    void SetOrder(uint8_t val);
    void SetSearchValueUtf8(const std::vector<uint8_t> &val);
    void SetSearchProperty(uint8_t val);
    void SetMaxListCount(uint16_t val);
    void SetListStartOffset(uint16_t val);
    void SetPropertySelector(uint64_t val);
    void SetFormat(uint8_t val);
    void SetPhonebookSize(uint16_t val);
    void SetNewMissedCalls(uint8_t val);
    void SetPrimaryFolderVersion(const std::vector<uint8_t> &val);
    void SetSecondaryFolderVersion(const std::vector<uint8_t> &val);
    void SetVcardSelector(uint64_t val);
    void SetDatabaseIdentifier(const std::vector<uint8_t> &val);
    void SetVcardSelectorOperator(uint8_t val);
    void SetResetNewMissedCalls(uint8_t val);
    void SetPbapSupportedFeatures(uint32_t val);

private:
    static const std::map<uint8_t, int> LENS_MAP;

    std::string name_ = "";
    uint8_t order_ = 0xFF;
    std::vector<uint8_t> searchValueUtf8_ {};
    uint8_t searchProperty_ = 0xFF;
    uint16_t maxListCount_ = 0xFFFF;
    uint16_t listStartOffset_ = 0;
    uint64_t propertySelector_ = 0;
    uint8_t format_ = 0xFF;
    uint16_t phonebookSize_ = 0;
    uint8_t newMissedCalls_ = 0xFF;
    std::vector<uint8_t> primaryFolderVer_ {};
    std::vector<uint8_t> secondaryFolderVer_ {};
    uint64_t vcardSelector_ = 0;
    std::vector<uint8_t> databaseIdentifier_ {};
    uint8_t vcardSelectorOp_ = 0xFF;
    uint8_t resetNewMissedCalls_ = 0xFF;
    uint32_t pbapSupportedFeatures_ = 0;

    bool hasOrder_ = false;
    bool hasSearchProperty_ = false;
    bool hasMaxListCount_ = false;
    bool hasListStartOffset_ = false;
    bool hasPropertySelector_ = false;
    bool hasFormat_ = false;
    bool hasPhonebookSize_ = false;
    bool hasNewMissedCalls_ = false;
    bool hasVcardSelector_ = false;
    bool hasVcardSelectorOp_ = false;
    bool hasResetNewMissedCalls_ = false;
    bool hasPbapSupportedFeatures_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_APP_PARAMS_H
