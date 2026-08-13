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
#define LOG_TAG "bt_service_map_mse_appparams"
#endif

#include "map_mse_appparams.h"
#include "log.h"

namespace {
const int32_t CONVERSION_BASE = 8;
// 读取两个字节，转成uint16
uint16_t ReadUint16(const std::vector<uint8_t> &values, int32_t pos)
{
    uint16_t value = 0;
    if (pos + sizeof(uint16_t) > values.size()) {
        return value;
    }
    for (size_t i = pos; i < pos + sizeof(uint16_t); i++) {
        value <<= CONVERSION_BASE;
        value |= values[i];
    }
    return value;
}

// 读取四个字节，转成int32
int32_t ReadInt32(const std::vector<uint8_t> &values, int32_t pos)
{
    int32_t value = 0;
    if (pos + sizeof(int32_t) > values.size()) {
        return value;
    }
    for (size_t i = pos; i < pos + sizeof(int32_t); i++) {
        value <<= CONVERSION_BASE;
        value |= values[i];
    }
    return value;
}

// 读取八个字节，转成int64
int64_t ReadInt64(const std::vector<uint8_t> &values, int32_t pos)
{
    int64_t value = 0;
    if (pos + sizeof(int64_t) > values.size()) {
        return value;
    }
    for (size_t i = pos; i < pos + sizeof(int64_t); i++) {
        value <<= CONVERSION_BASE;
        value |= values[i];
    }
    return value;
}

template <typename T>
void EncodeBasicParam(std::optional<T> &param, uint8_t tag, uint8_t tagLength, std::vector<uint8_t> &output)
{
    if (param.has_value()) {
        output.push_back(tag);
        output.push_back(tagLength);
        for (int i = tagLength - 1; i >= 0; i--) {
            output.push_back((param.value() >> (i * CONVERSION_BASE)) & 0xFF);
        }
    }
}

template <typename T>
void EncodeLongBasicParam(
    std::optional<T> &param1, std::optional<T> &param2, uint8_t tag, uint8_t tagLength, std::vector<uint8_t> &output)
{
    if (param1.has_value() && param2.has_value()) {
        output.push_back(tag);
        output.push_back(tagLength);
        int tmplen = tagLength / 2; // high and low length is taglength / 2
        for (int i = tmplen - 1; i >= 0; i--) {
            output.push_back((param1.value() >> (i * CONVERSION_BASE)) & 0xFF);
        }
        for (int i = tmplen - 1; i >= 0; i--) {
            output.push_back((param2.value() >> (i * CONVERSION_BASE)) & 0xFF);
        }
    }
}

void EncodeStringParam(std::optional<std::string> &param, uint8_t tag, std::vector<uint8_t> &output)
{
    if (param.has_value()) {
        output.push_back(tag);
        output.push_back(param.value().length());
        output.insert(output.end(), param.value().begin(), param.value().end());
    }
}
}  // namespace

namespace OHOS {
namespace bluetooth {
MapMseAppParams::MapMseAppParams() {}

MapMseAppParams::MapMseAppParams(const std::vector<uint8_t> &appParams)
{
    ParseParams(appParams);
}

void MapMseAppParams::GetEncodeData(std::vector<uint8_t> &output)
{
    EncodeBasicParam(maxListCount_, PARAM_MAX_LIST_COUNT, TAG_LEN_MAX_LIST_COUNT, output);
    EncodeBasicParam(listStartOffSet_, PARAM_LIST_START_OFF_SET, TAG_LEN_LIST_START_OFF_SET, output);
    EncodeBasicParam(filterMessageType_, PARAM_FILTER_MESSAGE_TYPE, TAG_LEN_FILTER_MESSAGE_TYPE, output);
    EncodeBasicParam(filterReadStatus_, PARAM_FILTER_READ_STATUS, TAG_LEN_FILTER_READ_STATUS, output);
    EncodeBasicParam(filterPriority_, PARAM_FILTER_PRIORITY, TAG_LEN_FILTER_PRIORITY, output);
    EncodeBasicParam(attachment_, PARAM_ATTACHMENT, TAG_LEN_ATTACHMENT, output);
    EncodeBasicParam(transparent_, PARAM_TRANSPARENT, TAG_LEN_TRANSPARENT, output);
    EncodeBasicParam(retry_, PARAM_RETRY, TAG_LEN_RETRY, output);
    EncodeBasicParam(newMessage_, PARAM_NEW_MESSAGE, TAG_LEN_NEW_MESSAGE, output);
    EncodeBasicParam(notificationStatus_, PARAM_NOTIFICATION_STATUS, TAG_LEN_NOTIFICATION_STATUS, output);
    EncodeBasicParam(masInstanceId_, PARAM_MAS_INSTANCEID, TAG_LEN_MAS_INSTANCEID, output);
    EncodeBasicParam(parameterMask_, PARAM_PARAMETER_MASK, TAG_LEN_PARAMETER_MASK, output);
    EncodeBasicParam(folderListingSize_, PARAM_FOLDER_LISTING_SIZE, TAG_LEN_FOLDER_LISTING_SIZE, output);
    EncodeBasicParam(messagesListingSize_, PARAM_LISTING_SIZE, TAG_LEN_LISTING_SIZE, output);
    EncodeBasicParam(subjectLength_, PARAM_SUBJECT_LENGTH, TAG_LEN_SUBJECT_LENGTH, output);
    EncodeBasicParam(charSet_, PARAM_CHARSET, TAG_LEN_CHARSET, output);
    EncodeBasicParam(fractionRequest_, PARAM_FRACTION_REQUEST, TAG_LEN_FRACTION_REQUEST, output);
    EncodeBasicParam(fractionDeliver_, PARAM_FRACTION_DELIVER, TAG_LEN_FRACTION_DELIVER, output);
    EncodeBasicParam(statusIndicator_, PARAM_STATUS_INDICATOR, TAG_LEN_STATUS_INDICATOR, output);
    EncodeBasicParam(statusValue_, PARAM_STATUS_VALUE, TAG_LEN_STATUS_VALUE, output);
    EncodeBasicParam(presenceAvailability_, PARAM_PRESENCE_AVAILABILITY, TAG_LEN_PRESENCE_AVAILABILITY, output);
    EncodeBasicParam(chatState_, PARAM_CHAT_STATE, TAG_LEN_CHAT_STATE, output);
    EncodeBasicParam(folderVerCounterLow_, PARAM_CONVERSATION_ID, TAG_LEN_CONVERSATION_ID, output);
    EncodeBasicParam(filterMessageHandle_, PARAM_FILTER_MESSAGE_HANDLE, TAG_LEN_FILTER_MESSAGE_HANDLE, output);
    EncodeBasicParam(notificationFilterMask_, PARAM_NOTIFICATION_FILTER_MASK, TAG_LEN_NOTIFICATION_FILTER_MASK, output);
    EncodeBasicParam(convParameterMask_, PARAM_CONV_PARAMETER_MASK, TAG_LEN_CONV_PARAMETER_MASK, output);
    EncodeBasicParam(mapSupportedFeatures_, PARAM_MAP_SUPPORTED_FEATURES, TAG_LEN_MAP_SUPPORTED_FEATURES, output);
    EncodeBasicParam(convoListingSize_, PARAM_CONVO_LISTING_SIZE, TAG_LEN_CONVO_LISTING_SIZE, output);
    EncodeBasicParam(convoListingSize_, PARAM_CONVO_LISTING_SIZE, TAG_LEN_CONVO_LISTING_SIZE, output);
    EncodeLongBasicParam(conversationListingVersionCounterHigh_,
        conversationListingVersionCounterLow_,
        PARAM_CONVERSATION_LISTING_VERSION_COUNTER,
        TAG_LEN_CONVERSATION_LISTING_VERSION_COUNTER,
        output);

    EncodeStringParam(filterPeriodBegin_, PARAM_FILTER_PERIOD_BEGIN, output);
    EncodeStringParam(filterPeriodEnd_, PARAM_FILTER_PERIOD_END, output);
    EncodeStringParam(filterRecipient_, PARAM_FILTER_RECIPIENT, output);
    EncodeStringParam(filterOriginator_, PARAM_FILTER_ORIGINATOR, output);
    EncodeStringParam(mseTime_, PARAM_MSE_TIME, output);
    EncodeStringParam(presenceText_, PARAM_PRESENCE_TEXT, output);
    EncodeStringParam(lastActivity_, PARAM_LAST_ACTIVITY, output);
    EncodeStringParam(filterLastActivityBegin_, PARAM_FILTER_LAST_ACTIVITY_BEGIN, output);
    EncodeStringParam(filterLastActivityEnd_, PARAM_FILTER_LAST_ACTIVITY_END, output);
    EncodeStringParam(ownerUci_, PARAM_OWNER_UCI, output);
    EncodeStringParam(extendedData_, PARAM_EXTENDED_DATA, output);
}

void MapMseAppParams::SetConversationListingVersionCounter(int64_t highData, int64_t lowData)
{
    conversationListingVersionCounterHigh_ = highData;
    conversationListingVersionCounterLow_ = lowData;
}

void MapMseAppParams::SetDatabaseIdentifier(int64_t highData, int64_t lowData)
{
    databaseIdentifierHigh_ = highData;
    databaseIdentifierLow_ = lowData;
}

void MapMseAppParams::SetMasInsanceId(int32_t masInsanceId)
{
    masInstanceId_ = masInsanceId;
}

void MapMseAppParams::ParseParams(const std::vector<uint8_t> &appParams)
{
    uint32_t pos = 0;
    int32_t tagId = 0;
    int32_t tagLength = 0;
    while ((pos + 2) < appParams.size()) {  // 2 means next two bytes to read
        tagId = appParams[pos++] & 0xFF;
        tagLength = appParams[pos++] & 0xFF;
        if (tagLength < 0) {
            HILOGE("length error");
            break;
        }
        if (pos + tagLength > appParams.size()) {
            HILOGE("length error");
            continue;
        }
        if (parseFuncs.count(tagId) > 0) {
            auto func = parseFuncs[tagId];
            if (func != nullptr) {
                (this->*func)(appParams, pos, tagLength);
            }
        }
        pos += tagLength;
    }
}

void MapMseAppParams::ParseMaxListCount(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_MAX_LIST_COUNT) {
        HILOGE("length error");
    } else {
        maxListCount_ = ReadUint16(appParams, pos);
    }
}

void MapMseAppParams::ParseListStartOffSet(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_LIST_START_OFF_SET) {
        HILOGE("length error");
    } else {
        listStartOffSet_ = ReadUint16(appParams, pos);
    }
}

void MapMseAppParams::ParseFilterMessageType(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FILTER_MESSAGE_TYPE) {
        HILOGE("length error");
    } else {
        filterMessageType_ = appParams[pos];
    }
}

void MapMseAppParams::ParseFilterPeriodBegin(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength == 0) {
        HILOGE("length error");
    } else {
        filterPeriodBegin_ = std::string(appParams.begin() + pos, appParams.begin() + pos + tagLength);
    }
}

void MapMseAppParams::ParseFilterPeriodEnd(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength == 0) {
        HILOGE("length error");
    } else {
        filterPeriodEnd_ = std::string(appParams.begin() + pos, appParams.begin() + pos + tagLength);
    }
}

void MapMseAppParams::ParseFilterReadStatus(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FILTER_READ_STATUS) {
        HILOGE("length error");
    } else {
        filterReadStatus_ = appParams[pos];
    }
}

void MapMseAppParams::ParseFilterRecipient(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength == 0) {
        HILOGE("length error");
    } else {
        filterRecipient_ = std::string(appParams.begin() + pos, appParams.begin() + pos + tagLength);
    }
}

void MapMseAppParams::ParseFilterOriginator(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength == 0) {
        HILOGE("length error");
    } else {
        filterOriginator_ = std::string(appParams.begin() + pos, appParams.begin() + pos + tagLength);
    }
}

void MapMseAppParams::ParseFilterPriority(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FILTER_PRIORITY) {
        HILOGE("length error");
    } else {
        filterPriority_ = appParams[pos];
    }
}

void MapMseAppParams::ParseAttachment(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_ATTACHMENT) {
        HILOGE("length error");
    } else {
        attachment_ = appParams[pos];
    }
}

void MapMseAppParams::ParseTransparent(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_TRANSPARENT) {
        HILOGE("length error");
    } else {
        transparent_ = appParams[pos];
    }
}

void MapMseAppParams::ParseRetry(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_RETRY) {
        HILOGE("length error");
    } else {
        retry_ = appParams[pos];
    }
}

void MapMseAppParams::ParseNewMessage(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_NEW_MESSAGE) {
        HILOGE("length error");
    } else {
        newMessage_ = appParams[pos];
    }
}

void MapMseAppParams::ParseNotificationStatus(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_NOTIFICATION_STATUS) {
        HILOGE("length error");
    } else {
        notificationStatus_ = appParams[pos];
    }
}

void MapMseAppParams::ParseMasInstanceid(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_MAS_INSTANCEID) {
        HILOGE("length error");
    } else {
        masInstanceId_ = appParams[pos];
    }
}

void MapMseAppParams::ParseParameterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_PARAMETER_MASK) {
        HILOGE("length error");
    } else {
        parameterMask_ = ReadInt32(appParams, pos);
    }
}

void MapMseAppParams::ParseFolderListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FOLDER_LISTING_SIZE) {
        HILOGE("length error");
    } else {
        folderListingSize_ = ReadUint16(appParams, pos);
    }
}

void MapMseAppParams::ParseListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_LISTING_SIZE) {
        HILOGE("length error");
    } else {
        messagesListingSize_ = ReadUint16(appParams, pos);
    }
}

void MapMseAppParams::ParseSubjectLength(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_SUBJECT_LENGTH) {
        HILOGE("length error");
    } else {
        subjectLength_ = appParams[pos];
    }
}

void MapMseAppParams::ParseCharset(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_CHARSET) {
        HILOGE("length error");
    } else {
        charSet_ = appParams[pos];
    }
}

void MapMseAppParams::ParseFractionRequest(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FRACTION_REQUEST) {
        HILOGE("length error");
    } else {
        fractionRequest_ = appParams[pos];
    }
}

void MapMseAppParams::ParseFractionDeliver(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FRACTION_DELIVER) {
        HILOGE("length error");
    } else {
        fractionDeliver_ = appParams[pos];
    }
}

void MapMseAppParams::ParseStatusIndicator(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_STATUS_INDICATOR) {
        HILOGE("length error");
    } else {
        statusIndicator_ = appParams[pos];
    }
}

void MapMseAppParams::ParseStatusValue(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_STATUS_VALUE) {
        HILOGE("length error");
    } else {
        statusValue_ = appParams[pos];
    }
}

void MapMseAppParams::ParseMseTime(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    mseTime_ = std::string(appParams.begin() + pos, appParams.begin() + pos + tagLength);
}

void MapMseAppParams::ParseDatabaseIdentifier(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength) {}

void MapMseAppParams::ParseConversationListingVersionCounter(
    const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_CONVERSATION_LISTING_VERSION_COUNTER) {
        HILOGE("length error: %{public}d", tagLength);
        return;
    }
    conversationListingVersionCounterHigh_ = ReadInt64(appParams, pos);
    conversationListingVersionCounterLow_ = ReadInt64(appParams, pos + CONVERSION_BASE);
}

void MapMseAppParams::ParsePresenceAvailability(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{}

void MapMseAppParams::ParsePresenceText(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength) {}

void MapMseAppParams::ParseLastActivity(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength) {}

void MapMseAppParams::ParseFilterLastActivityBegin(
    const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{}

void MapMseAppParams::ParseFilterLastActivityEnd(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{}

void MapMseAppParams::ParseChatState(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength) {}

void MapMseAppParams::ParseConversationId(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_CONVERSATION_ID) {
        HILOGE("length error: %{public}d", tagLength);
        return;
    }
    conversationIdLow_ = ReadInt64(appParams, pos);
    conversationIdHigh_ = ReadInt64(appParams, pos + CONVERSION_BASE);
}

void MapMseAppParams::ParseFolderVersionCounter(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{}

void MapMseAppParams::ParseFilterMessageHandle(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_FILTER_MESSAGE_HANDLE) {
        HILOGE("length error: %{public}d", tagLength);
        return;
    }
    filterMessageHandle_ = ReadInt64(appParams, pos);
}

void MapMseAppParams::ParseNotificationFilterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{}

void MapMseAppParams::ParseConvParameterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength) {}

void MapMseAppParams::ParseConvoListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength)
{
    if (tagLength != TAG_LEN_CONVO_LISTING_SIZE) {
        HILOGE("length error: %{public}d", tagLength);
        return;
    }
    convoListingSize_ = ReadUint16(appParams, pos);
}
}  // namespace bluetooth
}  // namespace OHOS