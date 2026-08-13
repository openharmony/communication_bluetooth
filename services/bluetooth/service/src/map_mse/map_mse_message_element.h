/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef MAP_MSE_MESSAGE_ELEMENT_H
#define MAP_MSE_MESSAGE_ELEMENT_H

#include <string>
#include <cstdint>
#include <optional>
#include <memory>
#include "map_mse_appparams.h"

namespace OHOS {
namespace bluetooth {
enum MessageType { TYPE_NONE = 0, TYPE_EMAIL = 1, TYPE_SMS_GSM = 2, TYPE_SMS_CDMA = 3, TYPE_MMS = 4, TYPE_IM = 5 };
const uint64_t HANDLE_TYPE_MASK = (static_cast<uint64_t>(0xFF) << 56);
const uint64_t HANDLE_TYPE_MMS_MASK = (static_cast<uint64_t>(0x01) << 56);
const uint64_t HANDLE_TYPE_EMAIL_MASK = (static_cast<uint64_t>(0x02) << 56);
const uint64_t HANDLE_TYPE_SMS_GSM_MASK = (static_cast<uint64_t>(0x04) << 56);
const uint64_t HANDLE_TYPE_SMS_CDMA_MASK = (static_cast<uint64_t>(0x08) << 56);
const uint64_t HANDLE_TYPE_IM_MASK = (static_cast<uint64_t>(0x10) << 56);
class MseMessageElement {
public:
    MseMessageElement(std::shared_ptr<MapMseAppParams> appParams, bool supportThreadId)
        : appParams_(appParams), supportThreadID_(supportThreadId){};
    ~MseMessageElement() = default;
    void GetEncodeData(std::string &xml);
    void SetHandle(int64_t value);
    void SetSubject(const std::string &value);
    void SetDateTime(int64_t value);
    void SetSenderName(const std::string &value);
    void SetSenderAddressing(const std::string &value);
    void SetReplytoAddressing(const std::string &value);
    void SetRecipientName(const std::string &value);
    void SetRecipientAddressing(const std::string &value);
    void SetType(MessageType value);
    void SetMsgTypeAppParamSet(bool value);
    void SetSize(int32_t value);
    void SetText(const std::string &value);
    void SetReceptionStatus(const std::string &value);
    void SetDeliveryStatus(const std::string &value);
    void SetAttachmentSize(int32_t value);
    void SetPriority(const std::string &value);
    void SetRead(bool value);
    void SetSent(const std::string &value);
    void SetProtect(const std::string &value);
    void SetFolderType(const std::string &value);
    void SetThreadId(const std::string &value);
    void SetThreadName(const std::string &value);
    void SetAttachmentMimeTypes(const std::string &value);
    void SetReportRead(bool value);

    static MessageType GetMessageTypeFromHandle(int64_t handle);
    static std::string GetMessageTypeStr(MessageType type);
    static uint64_t GetParsedMessageHandle(uint64_t handle);
    static std::string GenerateStringHandle(int64_t handle, MessageType type);

private:
    std::optional<std::string> GetFromatDataTime();
    template <class T>
    void EncodeBasicParam(std::optional<T> &value, const std::string &name, std::string &xml);
    void EncodeStringParam(std::optional<std::string> &value, const std::string &name, std::string &xml);

private:
    std::optional<int64_t> handle_;
    std::optional<std::string> subject_;
    std::optional<int64_t> dateTime_;
    std::optional<std::string> senderName_;
    std::optional<std::string> senderAddressing_;
    std::optional<std::string> replytoAddressing_;
    std::optional<std::string> recipientName_;
    std::optional<std::string> recipientAddressing_;
    std::optional<MessageType> type_;
    std::optional<bool> msgTypeAppParamSet_;
    std::optional<int32_t> size_;
    std::optional<std::string> text_;
    std::optional<std::string> receptionStatus_;
    std::optional<std::string> deliveryStatus_;
    std::optional<int32_t> attachmentSize_;
    std::optional<std::string> priority_;
    std::optional<bool> read_;
    std::optional<std::string> sent_;
    std::optional<std::string> protect_;
    std::optional<std::string> folderType_;
    std::optional<std::string> threadId_;
    std::optional<std::string> threadName_;
    std::optional<std::string> attachmentMimeTypes_;
    std::optional<bool> reportRead_;
    std::optional<int32_t> cursorIndex_;

    std::shared_ptr<MapMseAppParams> appParams_ = nullptr;
    bool supportThreadID_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif