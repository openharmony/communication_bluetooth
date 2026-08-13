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

#ifndef PBAP_PCE_VCARD_PARSER_H
#define PBAP_PCE_VCARD_PARSER_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// Parsed vCard contact entry.
/// (VERSION | FN | N | TEL) plus X-IRMC-CALL-DATETIME for call logs.
struct PbapPceVCardContact {
    /// Formatted display name (FN property, fallback to N property joined).
    std::string displayName {};
    /// Family name (N property, first element).
    std::string familyName {};
    /// Given name (N property, second element).
    std::string givenName {};
    /// Middle name (N property, third element).
    std::string middleName {};
    /// Phone numbers (TEL property, may be multiple).
    std::vector<std::string> phones {};
    /// Email addresses (EMAIL property, may be multiple).
    std::vector<std::string> emails {};
    /// Unknown X- properties, pair.first=propertyName, pair.second=propertyValue.
    /// Includes X-IRMC-CALL-DATETIME for call log timestamps.
    std::vector<std::pair<std::string, std::string>> unknownXData {};
    /// vCard version string (e.g. "3.0", "2.1").
    std::string version {};
};

/// Parsing scenario for vCard stream.
/// Call logs may have no name (only TEL), contacts typically require a name.
enum class PbapPceParseScenario {
    /// Contacts scenario: keep entry only when displayName is non-empty.
    CONTACTS,
    /// Call logs scenario: keep entry when phones is non-empty or displayName is non-empty.
    CALL_LOGS,
};

/// Lightweight vCard parser for PBAP PCE.
/// Parses vCard 2.1/3.0 byte stream in-memory (no temp files), aligned with
/// Only extracts fields needed by PBAP PCE: VERSION/FN/N/TEL/EMAIL/X-*.
class PbapPceVCardParser {
public:
    /// Parse a vCard byte stream into a list of contacts.
    /// Defaults to CONTACTS scenario (keeps entries with non-empty displayName).
    /// @param data vCard byte data from OBEX GET response body.
    /// @return List of parsed vCard contacts.
    static std::vector<PbapPceVCardContact> Parse(const std::vector<uint8_t> &data);

    /// Parse a vCard byte stream into a list of contacts with scenario.
    /// @param data vCard byte data from OBEX GET response body.
    /// @param scenario Parsing scenario (CONTACTS or CALL_LOGS).
    /// @return List of parsed vCard contacts.
    static std::vector<PbapPceVCardContact> Parse(
        const std::vector<uint8_t> &data, PbapPceParseScenario scenario);

private:
    /// Parse a single logical line into the current contact.
    /// @param line Logical vCard line (line-folding already unfolded).
    /// @param contact Current contact being built.
    /// @param inVCard Whether we are inside a BEGIN:VCARD..END:VCARD block.
    static void ParseLine(const std::string &line, PbapPceVCardContact &contact, bool &inVCard);

    /// Split a vCard content line "PROPERTY;PARAM=val:VALUE" into
    /// property name (uppercased), parameter map, and value.
    static void SplitContentLine(const std::string &line, std::string &propName,
        std::map<std::string, std::string> &params, std::string &value);

    /// Build display name from N property value "family;given;middle;prefix;suffix".
    static std::string BuildDisplayNameFromN(const std::string &nValue);

    /// Join phone numbers with comma separator.
    static std::string JoinPhones(const std::vector<std::string> &phones);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_VCARD_PARSER_H
