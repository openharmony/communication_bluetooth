/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_map_mse_meassage"
#endif

#include "map_mse_message_element.h"
#include <sstream>
#include <iomanip>
#include "log.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {
void MseMessageElement::GetEncodeData(std::string &xml)
{
    xml.append("<msg");
    if (type_.has_value()) {
        xml.append(" handle=\"").append(GenerateStringHandle(handle_.value(), type_.value())).append("\"");
    } else {
        xml.append(" handle=\"").append(GenerateStringHandle(handle_.value(), TYPE_NONE)).append("\"");
    }
    EncodeStringParam(subject_, "subject", xml);

    std::optional<std::string> dateTime = GetFromatDataTime();
    EncodeStringParam(dateTime, "datetime", xml);

    EncodeStringParam(senderName_, "sender_name", xml);
    EncodeStringParam(senderAddressing_, "sender_addressing", xml);
    EncodeStringParam(replytoAddressing_, "replyto_addressing", xml);
    EncodeStringParam(recipientName_, "recipient_name", xml);
    EncodeStringParam(recipientAddressing_, "recipient_addressing", xml);

    if (msgTypeAppParamSet_.has_value() && msgTypeAppParamSet_.value()) {
        std::optional<std::string> type = MseMessageElement::GetMessageTypeStr(type_.value());
        EncodeStringParam(type, "type", xml);
    }

    EncodeBasicParam(size_, "size", xml);
    EncodeStringParam(text_, "text", xml);
    EncodeStringParam(receptionStatus_, "reception_status", xml);
    EncodeStringParam(deliveryStatus_, "delivery_status", xml);
    EncodeBasicParam(attachmentSize_, "attachment_size", xml);
    EncodeStringParam(priority_, "priority", xml);

    if (reportRead_.has_value() && reportRead_.value()) {
        std::optional<std::string> read = (read_.has_value() && read_.value()) ? "yes" : "no";
        EncodeStringParam(read, "read", xml);
    }

    EncodeStringParam(sent_, "sent", xml);
    EncodeStringParam(protect_, "protected", xml);
    EncodeStringParam(folderType_, "folder_type", xml);

    if (supportThreadID_) {
        EncodeStringParam(threadId_, "conversation_id", xml);
        EncodeStringParam(threadName_, "conversation_name", xml);
    }

    EncodeStringParam(attachmentMimeTypes_, "attachment_mime_types", xml);
    xml.append("/>");
}

void MseMessageElement::SetHandle(int64_t value)
{
    handle_ = value;
}

void MseMessageElement::SetSubject(const std::string &value)
{
    uint32_t subjectLength = 255;
    uint32_t maxLength = 16 * 3;
    if (appParams_->subjectLength_.has_value()) {
        if (appParams_->subjectLength_.value() < subjectLength) {
            subjectLength = appParams_->subjectLength_.value();
        }
    }
    subjectLength = (subjectLength <= value.length()) ? subjectLength : value.length();
    if (!appParams_->parameterMask_.has_value() || (appParams_->parameterMask_.value() & MASK_SUBJECT) != 0) {
        if (value.length() > subjectLength) {
            if (value.length() > maxLength) {
                subject_ = value.substr(0, maxLength);
                subject_.value() += "...";
            }
        } else {
            subject_ = value;
        }
    }
}

void MseMessageElement::SetDateTime(int64_t value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_DATETIME) != 0)) {
        dateTime_ = value;
    }
}

void MseMessageElement::SetSenderName(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_SENDER_NAME) != 0)) {
        senderName_ = value;
    }
}

void MseMessageElement::SetSenderAddressing(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() ||
        ((appParams_->parameterMask_.value() & MASK_SENDER_ADDRESSING) != 0)) {
        senderAddressing_ = value;
    }
}

void MseMessageElement::SetReplytoAddressing(const std::string &value) {}

void MseMessageElement::SetRecipientName(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_RECIPIENT_NAME) != 0)) {
        recipientName_ = value;
    }
}

void MseMessageElement::SetRecipientAddressing(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() ||
        ((appParams_->parameterMask_.value() & MASK_RECIPIENT_ADDRESSING) != 0)) {
        recipientAddressing_ = value;
    }
}

void MseMessageElement::SetType(MessageType value)
{
    type_ = value;
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_TYPE) != 0)) {
        SetMsgTypeAppParamSet(true);
    }
}

void MseMessageElement::SetMsgTypeAppParamSet(bool value)
{
    msgTypeAppParamSet_ = value;
}

void MseMessageElement::SetSize(int32_t value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_SIZE) != 0)) {
        size_ = value > 0 ? value : 1;
    }
}

void MseMessageElement::SetText(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_TEXT) != 0)) {
        text_ = value;
    }
}

void MseMessageElement::SetReceptionStatus(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() ||
        ((appParams_->parameterMask_.value() & MASK_RECEPTION_STATUS) != 0)) {
        receptionStatus_ = value;
    }
}

void MseMessageElement::SetDeliveryStatus(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_DELIVERY_STATUS) != 0)) {
        deliveryStatus_ = value;
    }
}

void MseMessageElement::SetAttachmentSize(int32_t value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_ATTACHMENT_SIZE) != 0)) {
        attachmentSize_ = value;
    }
}

void MseMessageElement::SetPriority(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_PRIORITY) != 0)) {
        priority_ = value;
    }
}

void MseMessageElement::SetRead(bool value)
{
    read_ = value;
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_READ) != 0)) {
        SetReportRead(true);
    }
}

void MseMessageElement::SetSent(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_SENT) != 0)) {
        sent_ = value;
    }
}

void MseMessageElement::SetProtect(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_PROTECTED) != 0)) {
        protect_ = value;
    }
}

void MseMessageElement::SetFolderType(const std::string &value)
{
    folderType_ = value;
}

void MseMessageElement::SetThreadId(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() || ((appParams_->parameterMask_.value() & MASK_CONVERSATION_ID) != 0)) {
        threadId_ = value;
    }
}

void MseMessageElement::SetThreadName(const std::string &value)
{
    if (!appParams_->parameterMask_.has_value() ||
        ((appParams_->parameterMask_.value() & MASK_CONVERSATION_NAME) != 0)) {
        threadName_ = value;
    }
}

void MseMessageElement::SetAttachmentMimeTypes(const std::string &value)
{
    attachmentMimeTypes_ = value;
}

void MseMessageElement::SetReportRead(bool value)
{
    reportRead_ = value;
}

MessageType MseMessageElement::GetMessageTypeFromHandle(int64_t handle)
{
    if ((handle & HANDLE_TYPE_MMS_MASK) != 0) {
        return TYPE_MMS;
    }
    if ((handle & HANDLE_TYPE_EMAIL_MASK) != 0) {
        return TYPE_EMAIL;
    }
    if ((handle & HANDLE_TYPE_SMS_GSM_MASK) != 0) {
        return TYPE_SMS_GSM;
    }
    if ((handle & HANDLE_TYPE_SMS_CDMA_MASK) != 0) {
        return TYPE_SMS_CDMA;
    }
    if ((handle & HANDLE_TYPE_IM_MASK) != 0) {
        return TYPE_IM;
    }

    return TYPE_NONE;
}

std::string MseMessageElement::GetMessageTypeStr(MessageType type)
{
    switch (type) {
        case TYPE_NONE:
            return "NONE";
        case TYPE_EMAIL:
            return "EMAIL";
        case TYPE_SMS_GSM:
            return "SMS_GSM";
        case TYPE_SMS_CDMA:
            return "SMS_CDMA";
        case TYPE_MMS:
            return "MMS";
        case TYPE_IM:
            return "IM";
        default:
            return "NONE";
    }
}

uint64_t MseMessageElement::GetParsedMessageHandle(uint64_t handle)
{
    uint64_t parsedHandle = handle;
    parsedHandle &= (~HANDLE_TYPE_MASK);
    return parsedHandle;
}

std::string MseMessageElement::GenerateStringHandle(int64_t handle, MessageType type)
{
    std::string messageHandle = "-1";
    auto integerToHexString = [](int64_t value) -> std::string {
        std::stringstream ss;
        ss << std::hex << value;
        return ss.str();
    };
    switch (type) {
        case TYPE_NONE:
            break;
        case TYPE_EMAIL:
            messageHandle = integerToHexString(handle | HANDLE_TYPE_EMAIL_MASK);
            break;
        case TYPE_SMS_GSM:
            messageHandle = integerToHexString(handle | HANDLE_TYPE_SMS_GSM_MASK);
            break;
        case TYPE_SMS_CDMA:
            messageHandle = integerToHexString(handle | HANDLE_TYPE_SMS_CDMA_MASK);
            break;
        case TYPE_MMS:
            messageHandle = integerToHexString(handle | HANDLE_TYPE_MMS_MASK);
            break;
        case TYPE_IM:
            messageHandle = integerToHexString(handle | HANDLE_TYPE_IM_MASK);
            break;
        default:
            break;
    }

    return messageHandle;
}

std::optional<std::string> MseMessageElement::GetFromatDataTime()
{
    std::optional<std::string> result;
    if (!dateTime_.has_value()) {
        return result;
    }
    struct tm time;
    if (memset_s(&time, sizeof(struct tm), 0x00, sizeof(tm)) != EOK) {
        HILOGE("memset fail.");
        return result;
    }
    time_t second = dateTime_.value() / 1000;  // convert milliseconds to seconds, divided by 1000
    localtime_r(&second, &time);

    std::stringstream ss;
    ss << std::put_time(&time, "%Y%m%dT%H%M%S");
    result = ss.str();
    return result;
}

template <class T>
void MseMessageElement::EncodeBasicParam(std::optional<T> &value, const std::string &name, std::string &xml)
{
    if (value.has_value()) {
        xml.append(" ").append(name).append("=\"").append(std::to_string(value.value())).append("\"");
    }
}

void MseMessageElement::EncodeStringParam(std::optional<std::string> &value, const std::string &name, std::string &xml)
{
    if (value.has_value()) {
        xml.append(" ").append(name).append("=\"").append(value.value()).append("\"");
    }
}
}  // namespace bluetooth
}  // namespace OHOS