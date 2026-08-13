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
#define LOG_TAG "bt_service_map_mse_bmessage"
#endif

#include "common_util.h"
#include "map_mse_bmessage.h"
#include "string_ex.h"
#include "map_mse_message_element.h"
#include "log.h"

namespace {
std::string ParseNodeValue(const std::string &src, const std::string &node)
{
    if (src.find(node) != std::string::npos) {
        std::string::size_type start = src.find_first_of(':', 0);
        if (start != std::string::npos) {
            return src.substr(start + 1);
        }
    }
    return "";
}
}  // namespace

namespace OHOS {
namespace bluetooth {
const int32_t BBODY_HEADER_LENGTH = 22;
std::string MapMseBMessageSms::Encode()
{
    std::string bmessage;
    bmessage.append("BEGIN:BMSG").append("\r\n");
    bmessage.append("VERSION:" + version_).append("\r\n");
    bmessage.append("STATUS:" + status_).append("\r\n");
    bmessage.append("TYPE:" + type_).append("\r\n");
    bmessage.append("FOLDER:" + folder_).append("\r\n");
    if (version_ == "1.1") {
        bmessage.append("EXTENDEDDATA:");
        bmessage.append("\r\n");
    }
    for (auto &iter : originator_) {
        bmessage.append(iter.ToVcardString());
    }
    bmessage.append("BEGIN:BENV").append("\r\n");
    for (auto &iter : recipient_) {
        bmessage.append(iter.ToVcardString());
    }
    bmessage.append("BEGIN:BBODY").append("\r\n");
    bmessage.append("CHARSET:" + charset_).append("\r\n");
    bmessage.append("LENGTH:" + std::to_string(messageBody_.length() + BBODY_HEADER_LENGTH)).append("\r\n");
    bmessage.append("BEGIN:MSG").append("\r\n");
    bmessage.append(messageBody_).append("\r\n");
    bmessage.append("END:MSG").append("\r\n");
    bmessage.append("END:BBODY").append("\r\n");
    bmessage.append("END:BENV").append("\r\n");
    bmessage.append("END:BMSG").append("\r\n");
    return bmessage;
}

bool MapMseBMessageSms::ParseMessage(const std::vector<uint8_t> &rawData, uint8_t charset)
{
    std::string data(rawData.begin(), rawData.end());
    std::vector<std::string> splitStrs;
    SplitStr(data, "\r\n", splitStrs);

    uint32_t pos = 0;
    auto beginBmsg = pos < splitStrs.size() ? splitStrs[pos++] : "";
    if (beginBmsg.find("BEGIN:BMSG") == std::string::npos) {
        return false;
    }

    auto version = pos < splitStrs.size() ? splitStrs[pos++] : "";
    if (version.find("VERSION:") == std::string::npos) {
        return false;
    }

    while (pos < splitStrs.size()) {
        auto line = splitStrs[pos];
        if (line.find("BEGIN:VCARD") != std::string::npos || line.find("BEGIN:BENV") != std::string::npos) {
            break;
        }
        ParseMessageProperty(line);
        pos++;
    }
    if (charset == 0x00) {  // CHARSET_NATIVE
        if (GetType() != MseMessageElement::GetMessageTypeStr(MessageType::TYPE_SMS_GSM) &&
            GetType() != MseMessageElement::GetMessageTypeStr(MessageType::TYPE_SMS_CDMA)) {
            HILOGE("message type error");
            return false;
        }
    }
    if (GetStatus().empty()) {
        HILOGE("message status error");
        return false;
    }
    ParseOriginator(splitStrs, pos);
    ParseEnvelope(splitStrs, pos);

    return true;
}

bool MapMseBMessageSms::ParseMessageProperty(const std::string &line)
{
    bool ret = false;
    if (auto value = ParseNodeValue(line, "STATUS:"); !value.empty()) {
        SetStatus(value);
        ret = true;
    }
    if (auto value = ParseNodeValue(line, "TYPE:"); !value.empty()) {
        SetType(value);
        ret = true;
    }
    if (auto value = ParseNodeValue(line, "FOLDER:"); !value.empty()) {
        SetFolder(value);
        ret = true;
    }
    if (auto value = ParseNodeValue(line, "EXTENDEDDATA:"); !value.empty()) {
        ret = true;
    }
    return ret;
}

void MapMseBMessageSms::ParseOriginator(const std::vector<std::string> &src, uint32_t &pos)
{
    while (pos < src.size()) {
        auto line = src[pos];
        if (line.find("BEGIN:BENV") != std::string::npos) {
            break;
        }
        if (line.find("BEGIN:VCARD") != std::string::npos) {
            pos++;
            AddOriginator(ParseVcard(src, pos));
        }
        pos++;
    }
}

void MapMseBMessageSms::ParseRecipient(const std::vector<std::string> &src, uint32_t &pos)
{
    while (pos < src.size()) {
        auto line = src[pos];
        if (line.find("BEGIN:BBODY") != std::string::npos) {
            break;
        }
        if (line.find("BEGIN:VCARD") != std::string::npos) {
            pos++;
            AddRecipient(ParseVcard(src, pos));
        }
        pos++;
    }
}

void MapMseBMessageSms::ParseBody(const std::vector<std::string> &src, uint32_t &pos)
{
    while (pos < src.size()) {
        auto line = src[pos];
        if (line.find("END:BBODY") != std::string::npos) {
            break;
        }
        if (auto tempStr = ParseNodeValue(line, "ENCODING:"); !tempStr.empty()) {
            SetEncoding(tempStr);
        } else if (auto tempStr = ParseNodeValue(line, "CHARSET:"); !tempStr.empty()) {
            SetCharset(tempStr);
        } else if (auto tempStr = ParseNodeValue(line, "LANGUAGE:"); !tempStr.empty()) {
            SetLanuage(tempStr);
        } else if (auto tempStr = ParseNodeValue(line, "LENGTH:"); !tempStr.empty()) {
            CHECK_AND_RETURN_LOG(ConvertStrToDigit(tempStr, messageBodyLength_), "parse length error");
        }
        if (line.find("BEGIN:MSG") != std::string::npos) {
            pos++;
            ParseMsgContent(src, pos);
        }
        pos++;
    }
}

void MapMseBMessageSms::ParseMsgContent(const std::vector<std::string> &src, uint32_t &pos)
{
    while (pos < src.size()) {
        auto body = src[pos];
        if (body.find("END:MSG") != std::string::npos) {
            break;
        }
        messageBody_.append(body);
        pos++;
    }
}

void MapMseBMessageSms::ParseEnvelope(const std::vector<std::string> &src, uint32_t &pos)
{
    while (pos < src.size()) {
        auto line = src[pos];
        if (line.find("END:BENV") != std::string::npos) {
            break;
        }
        if (line.find("BEGIN:BENV") != std::string::npos) {
            pos++;
            ParseRecipient(src, pos);
            if (pos >= src.size()) {
                break;
            }
            line = src[pos];
            if (line.find("BEGIN:BBODY") != std::string::npos) {
                pos++;
                ParseBody(src, pos);
            }
        }
        pos++;
    }
}

MapMseBMessage::MapMseVcard MapMseBMessageSms::ParseVcard(const std::vector<std::string> &src, uint32_t &pos)
{
    std::string name = "";
    std::string version = "";
    std::string formattedName = "";
    std::vector<std::string> phoneNumbers{};
    std::vector<std::string> emailAddresses{};
    std::vector<std::string> btUids{};
    std::vector<std::string> btUcis{};
    while (pos < src.size()) {
        auto line = src[pos];
        if (line.find("END:VCARD") != std::string::npos) {
            break;
        }
        if (auto tempStr = ParseNodeValue(line, "VERSION:"); !tempStr.empty()) {
            version = tempStr == "3.0" ? "3.0" : "2.1";
        } else if (auto tempStr = ParseNodeValue(line, "FN:"); !tempStr.empty()) {
            formattedName = tempStr;
        } else if (auto tempStr = ParseNodeValue(line, "N:"); !tempStr.empty()) {
            name = tempStr;
        } else if (auto tempStr = ParseNodeValue(line, "TEL:"); !tempStr.empty()) {
            SplitStr(tempStr, ";", phoneNumbers);
        } else if (auto tempStr = ParseNodeValue(line, "EMAIL:"); !tempStr.empty()) {
            SplitStr(tempStr, ";", emailAddresses);
        } else if (auto tempStr = ParseNodeValue(line, "X-BT-UID:"); !tempStr.empty()) {
            SplitStr(tempStr, ";", btUids);
        } else if (auto tempStr = ParseNodeValue(line, "X-BT-UCI:"); !tempStr.empty()) {
            SplitStr(tempStr, ";", btUcis);
        }
        pos++;
    }
    MapMseBMessage::MapMseVcard::VcardVer3 vcard{
        .name = name,
        .version = "3.0",
        .formattedName = formattedName,
        .phoneNumbers = std::move(phoneNumbers),
        .emailAddresses = std::move(emailAddresses),
        .btUids = std::move(btUids),
        .btUcis = std::move(btUcis),
    };
    return MapMseBMessage::MapMseVcard(vcard);
}

MapMseBMessage::MapMseVcard::MapMseVcard(const VcardVer2 &vcard)
    : name_(vcard.name),
      version_(vcard.version),
      formattedName_(vcard.formattedName),
      phoneNumbers_(vcard.phoneNumbers),
      emailAddresses_(vcard.emailAddresses)
{
    SetFormattedPhoneNumber();
}

MapMseBMessage::MapMseVcard::MapMseVcard(const VcardVer3 &vcard)
    : name_(vcard.name),
      version_(vcard.version),
      formattedName_(vcard.formattedName),
      phoneNumbers_(vcard.phoneNumbers),
      emailAddresses_(vcard.emailAddresses),
      btUids_(vcard.btUids),
      btUcis_(vcard.btUcis)
{
    SetFormattedPhoneNumber();
}

std::string MapMseBMessage::MapMseVcard::ToVcardString()
{
    std::string vcd;
    vcd.append("BEGIN:VCARD").append("\r\n");
    vcd.append("VERSION:" + version_).append("\r\n");
    if (version_ == "3.0") {
        vcd.append("FN:" + formattedName_).append("\r\n");
    }
    vcd.append("N:" + name_).append("\r\n");
    for (const auto &number : phoneNumbers_) {
        vcd.append("TEL:" + number).append("\r\n");
    }
    for (const auto &email : emailAddresses_) {
        vcd.append("EMAIL:" + email).append("\r\n");
    }
    for (const auto &uid : btUids_) {
        vcd.append("X-BT-UID:" + uid).append("\r\n");
    }
    for (const auto &uci : btUcis_) {
        vcd.append("X-BT-UCI:" + uci).append("\r\n");
    }
    vcd.append("END:VCARD").append("\r\n");
    return vcd;
}

void MapMseBMessage::MapMseVcard::SetFormattedPhoneNumber()
{
    auto validSeparator = [](char c) -> bool { return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+'; };
    for (uint32_t i = 0; i < phoneNumbers_.size(); i++) {
        std::string phoneNumber = phoneNumbers_[i];
        std::string formattedPhoneNumber = "";
        for (auto c : phoneNumber) {
            if (validSeparator(c)) {
                formattedPhoneNumber += c;
            }
        }
        phoneNumbers_[i] = formattedPhoneNumber;
    }
}
}  // namespace bluetooth
}  // namespace OHOS