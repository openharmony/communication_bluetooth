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

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_vcard_parser"
#endif

#include "pbap_pce_vcard_parser.h"

#include <algorithm>
#include <cctype>
#include "log.h"

namespace OHOS {
namespace bluetooth {

/// Trim leading/trailing whitespace from a string.
static std::string Trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

/// Convert ASCII string to uppercase in-place.
static std::string ToUpper(const std::string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

/// Split a string by delimiter, returning all parts (including empty ones).
static std::vector<std::string> Split(const std::string &s, char delimiter)
{
    std::vector<std::string> parts;
    std::string current;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += s[i];
        }
    }
    parts.push_back(current);
    return parts;
}

std::vector<PbapPceVCardContact> PbapPceVCardParser::Parse(const std::vector<uint8_t> &data)
{
    return Parse(data, PbapPceParseScenario::CONTACTS);
}

std::vector<PbapPceVCardContact> PbapPceVCardParser::Parse(
    const std::vector<uint8_t> &data, PbapPceParseScenario scenario)
{
    std::vector<PbapPceVCardContact> contacts;
    if (data.empty()) {
        HILOGI("[PbapPceVCardParser] Parse: data is empty");
        return contacts;
    }

    // Convert byte vector to string for line-by-line processing.
    std::string text(data.begin(), data.end());

    // Split into lines, handling both \r\n and \n line endings.
    // Also handle vCard line folding: a line beginning with a space continues the previous line.
    std::vector<std::string> rawLines;
    std::string currentLine;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            // Strip trailing \r if present
            if (!currentLine.empty() && currentLine.back() == '\r') {
                currentLine.pop_back();
            }
            rawLines.push_back(currentLine);
            currentLine.clear();
        } else {
            currentLine += text[i];
        }
    }
    if (!currentLine.empty()) {
        if (currentLine.back() == '\r') {
            currentLine.pop_back();
        }
        rawLines.push_back(currentLine);
    }

    // Unfold lines: vCard 3.0 spec says a line beginning with a single space
    // is a continuation of the previous line (the space is removed).
    std::vector<std::string> lines;
    for (const std::string &raw : rawLines) {
        if (!raw.empty() && raw[0] == ' ' && !lines.empty()) {
            // Continuation line: append to previous (skip the leading space)
            lines.back() += raw.substr(1);
        } else {
            lines.push_back(raw);
        }
    }

    HILOGI("[PbapPceVCardParser] Parse: total raw lines=%{public}zu, unfolded lines=%{public}zu",
        rawLines.size(), lines.size());

    // Parse each line
    bool inVCard = false;
    PbapPceVCardContact currentContact {};
    for (const std::string &line : lines) {
        std::string trimmed = Trim(line);
        if (trimmed.empty()) {
            continue;
        }
        ParseLine(trimmed, currentContact, inVCard);
        // Scenario-aware keep policy: call logs may have only TEL (no name),
        // contacts typically require a name.
        bool shouldKeep = false;
        if (scenario == PbapPceParseScenario::CALL_LOGS) {
            shouldKeep = !currentContact.phones.empty() || !currentContact.displayName.empty();
        } else {
            shouldKeep = !currentContact.displayName.empty();
        }
        if (!inVCard && shouldKeep) {
            // END:VCARD was processed, push the completed contact
            contacts.push_back(currentContact);
            currentContact = PbapPceVCardContact {};
        }
    }

    HILOGI("[PbapPceVCardParser] Parse: scenario=%{public}d, parsed %{public}zu vCard contacts",
        static_cast<int32_t>(scenario), contacts.size());
    return contacts;
}

void PbapPceVCardParser::ParseLine(const std::string &line, PbapPceVCardContact &contact, bool &inVCard)
{
    std::string propName;
    std::map<std::string, std::string> params;
    std::string value;

    // Handle BEGIN:VCARD and END:VCARD specially (no params/colon-in-value issues)
    std::string upperLine = ToUpper(line);
    if (upperLine == "BEGIN:VCARD") {
        inVCard = true;
        contact = PbapPceVCardContact {};
        return;
    }
    if (upperLine == "END:VCARD") {
        inVCard = false;
        return;
    }
    if (!inVCard) {
        return;
    }

    SplitContentLine(line, propName, params, value);

    if (propName == "VERSION") {
        contact.version = value;
    } else if (propName == "FN") {
        contact.displayName = value;
    } else if (propName == "N") {
        // N: family;given;middle;prefix;suffix
        if (contact.displayName.empty()) {
            contact.displayName = BuildDisplayNameFromN(value);
        }
        std::vector<std::string> parts = Split(value, ';');
        if (parts.size() > 0) {
            contact.familyName = Trim(parts[0]);
        }
        if (parts.size() > 1) {
            contact.givenName = Trim(parts[1]);
        }
        if (parts.size() > 2) {
            contact.middleName = Trim(parts[2]);
        }
    } else if (propName == "TEL") {
        std::string phone = Trim(value);
        if (!phone.empty()) {
            contact.phones.push_back(phone);
        }
    } else if (propName == "EMAIL") {
        std::string email = Trim(value);
        if (!email.empty()) {
            contact.emails.push_back(email);
        }
    } else if (propName.size() > 2 && propName[0] == 'X' && propName[1] == '-') {
        // Catch all for X- properties (includes X-IRMC-CALL-DATETIME)
        // Preserve original property name with params for call log timestamp extraction.
        // Reconstruct full property name with params: "X-IRMC-CALL-DATETIME;MISSED"
        std::string fullPropName = propName;
        for (const auto &param : params) {
            fullPropName += ";" + param.first;
            if (!param.second.empty()) {
                fullPropName += "=" + param.second;
            }
        }
        contact.unknownXData.push_back(std::make_pair(fullPropName, value));
    }
}

void PbapPceVCardParser::SplitContentLine(const std::string &line, std::string &propName,
    std::map<std::string, std::string> &params, std::string &value)
{
    // A vCard content line: PROPERTY;PARAM1=val1;PARAM2=val2:VALUE
    // The first colon (not inside quotes) separates "property+params" from "value".
    // Property name and params are separated by semicolons.

    // Find the first unquoted colon to split property-params from value.
    size_t colonPos = std::string::npos;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '"') {
            inQuotes = !inQuotes;
        } else if (line[i] == ':' && !inQuotes) {
            colonPos = i;
            break;
        }
    }
    if (colonPos == std::string::npos) {
        propName = ToUpper(Trim(line));
        value = "";
        return;
    }

    std::string propAndParams = line.substr(0, colonPos);
    value = line.substr(colonPos + 1);

    // Split property and params by semicolon (unquoted)
    std::vector<std::string> segments;
    std::string current;
    inQuotes = false;
    for (size_t i = 0; i < propAndParams.size(); i++) {
        if (propAndParams[i] == '"') {
            inQuotes = !inQuotes;
            current += propAndParams[i];
        } else if (propAndParams[i] == ';' && !inQuotes) {
            segments.push_back(current);
            current.clear();
        } else {
            current += propAndParams[i];
        }
    }
    segments.push_back(current);

    if (segments.empty()) {
        propName = "";
        return;
    }

    // First segment is the property name
    propName = ToUpper(Trim(segments[0]));

    // Remaining segments are parameters: PARAM=value or just PARAM (e.g. TYPE param)
    for (size_t i = 1; i < segments.size(); i++) {
        std::string seg = segments[i];
        size_t eqPos = seg.find('=');
        if (eqPos != std::string::npos) {
            std::string paramKey = ToUpper(Trim(seg.substr(0, eqPos)));
            std::string paramVal = Trim(seg.substr(eqPos + 1));
            // Strip surrounding quotes from value
            if (paramVal.size() >= 2 && paramVal.front() == '"' && paramVal.back() == '"') {
                paramVal = paramVal.substr(1, paramVal.size() - 2);
            }
            params[paramKey] = paramVal;
        } else {
            // Parameter without value (e.g. "MISSED" in X-IRMC-CALL-DATETIME;MISSED)
            // Store with empty value, key is the segment itself.
            std::string paramKey = Trim(seg);
            if (!paramKey.empty()) {
                params[paramKey] = "";
            }
        }
    }
}

std::string PbapPceVCardParser::BuildDisplayNameFromN(const std::string &nValue)
{
    // N: family;given;middle;prefix;suffix
    // Build display name as "given family" (common format)
    std::vector<std::string> parts = Split(nValue, ';');
    std::string family = (parts.size() > 0) ? Trim(parts[0]) : "";
    std::string given = (parts.size() > 1) ? Trim(parts[1]) : "";

    std::string displayName;
    if (!given.empty() && !family.empty()) {
        displayName = given + " " + family;
    } else if (!given.empty()) {
        displayName = given;
    } else if (!family.empty()) {
        displayName = family;
    }
    return displayName;
}

std::string PbapPceVCardParser::JoinPhones(const std::vector<std::string> &phones)
{
    std::string result;
    for (size_t i = 0; i < phones.size(); i++) {
        if (i > 0) {
            result += ",";
        }
        result += phones[i];
    }
    return result;
}

}  // namespace bluetooth
}  // namespace OHOS
