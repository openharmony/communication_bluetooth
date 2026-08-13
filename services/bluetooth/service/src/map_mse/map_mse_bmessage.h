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

#ifndef MAP_MSE_BMESSAGE_H
#define MAP_MSE_BMESSAGE_H

#include <string>

namespace OHOS {
namespace bluetooth {
class MapMseBMessage {
public:
    class MapMseVcard {
    public:
        struct VcardVer2 {
            std::string name = "";
            std::string version = "";
            std::string formattedName = "";
            std::vector<std::string> phoneNumbers{};
            std::vector<std::string> emailAddresses{};
        };
        struct VcardVer3 {
            std::string name = "";
            std::string version = "";
            std::string formattedName = "";
            std::vector<std::string> phoneNumbers{};
            std::vector<std::string> emailAddresses{};
            std::vector<std::string> btUids{};
            std::vector<std::string> btUcis{};
        };
        explicit MapMseVcard(const VcardVer2 &vcard);
        explicit MapMseVcard(const VcardVer3 &vcard);
        ~MapMseVcard() = default;
        std::string ToVcardString();
        std::string GetFirstPhoneNumber() { return phoneNumbers_.empty() ? "" : phoneNumbers_[0]; };
        std::string GetFirstEmail() { return emailAddresses_.empty() ? "" : emailAddresses_[0]; };

    private:
        void SetFormattedPhoneNumber();
    private:
        std::string name_ = "";
        std::string version_ = "";
        std::string formattedName_ = "";
        std::vector<std::string> phoneNumbers_{};
        std::vector<std::string> emailAddresses_{};
        std::vector<std::string> btUids_{};
        std::vector<std::string> btUcis_{};
    };

public:
    MapMseBMessage() = default;
    virtual ~MapMseBMessage() = default;

    virtual void SetMessageBody(const std::string &body) = 0;
    virtual std::string GetMessageBody() = 0;

    inline void SetVersion(const std::string &value) { version_ = value; }
    inline std::string GetVersion() { return version_; }

    inline void SetStatus(const std::string &value) { status_ = value; }
    inline std::string GetStatus() { return status_; }

    inline void SetType(const std::string &value) { type_ = value; }
    inline std::string GetType() { return type_; }

    inline void SetFolder(const std::string &value) { folder_ = value; }
    inline std::string GetFolder() { return folder_; }

    inline void SetEncoding(const std::string &value) { encoding_ = value; }
    inline std::string GetEncoding() { return encoding_; }

    inline void SetCharset(const std::string &value) { charset_ = value; }
    inline std::string GetCharset() { return charset_; }

    inline void SetLanuage(const std::string &value) { lanuage_ = value; }
    inline std::string GetLanuage() { return lanuage_; }

    inline void AddOriginator(const MapMseVcard &value) { originator_.push_back(value); }
    inline std::vector<MapMseVcard> GetOriginator() { return originator_; }

    inline void AddRecipient(const MapMseVcard &value) { recipient_.push_back(value); }
    inline std::vector<MapMseVcard> GetRecipient() { return recipient_; }

protected:
    std::string version_;
    std::string status_;
    std::string type_;
    std::string folder_;
    std::string encoding_;
    std::string charset_;
    std::string lanuage_;
    std::vector<MapMseVcard> originator_;
    std::vector<MapMseVcard> recipient_;
};

class MapMseBMessageSms : public MapMseBMessage {
public:
    MapMseBMessageSms() = default;
    ~MapMseBMessageSms() override = default;

    inline void SetMessageBody(const std::string &body) override { messageBody_ = body; };
    inline std::string GetMessageBody() override { return messageBody_; };
    std::string Encode();
    bool ParseMessage(const std::vector<uint8_t> &rawData, uint8_t charset);

private:
    bool ParseMessageProperty(const std::string &line);
    void ParseOriginator(const std::vector<std::string> &src, uint32_t &pos);
    void ParseEnvelope(const std::vector<std::string> &src, uint32_t &pos);
    void ParseRecipient(const std::vector<std::string> &src, uint32_t &pos);
    void ParseBody(const std::vector<std::string> &src, uint32_t &pos);
    void ParseMsgContent(const std::vector<std::string> &src, uint32_t &pos);
    MapMseBMessage::MapMseVcard ParseVcard(const std::vector<std::string> &src, uint32_t &pos);

private:
    uint32_t messageBodyLength_ = 0;
    std::string messageBody_ = "";
};
}  // namespace bluetooth
}  // namespace OHOS
#endif