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

#ifndef MAP_MSE_APPPARAMS_H
#define MAP_MSE_APPPARAMS_H

#include <vector>
#include <cstdint>
#include <functional>
#include <string>
#include <optional>

namespace OHOS {
namespace bluetooth {
const int32_t LENGTH_VARIABLE = 0;
const uint8_t PARAM_MAX_LIST_COUNT = 0x01;
const uint8_t PARAM_LIST_START_OFF_SET = 0x02;
const uint8_t PARAM_FILTER_MESSAGE_TYPE = 0x03;
const uint8_t PARAM_FILTER_PERIOD_BEGIN = 0x04;
const uint8_t PARAM_FILTER_PERIOD_END = 0x05;
const uint8_t PARAM_FILTER_READ_STATUS = 0x06;
const uint8_t PARAM_FILTER_RECIPIENT = 0x07;
const uint8_t PARAM_FILTER_ORIGINATOR = 0x08;
const uint8_t PARAM_FILTER_PRIORITY = 0x09;
const uint8_t PARAM_ATTACHMENT = 0x0A;
const uint8_t PARAM_TRANSPARENT = 0x0B;
const uint8_t PARAM_RETRY = 0x0C;
const uint8_t PARAM_NEW_MESSAGE = 0x0D;
const uint8_t PARAM_NOTIFICATION_STATUS = 0x0E;
const uint8_t PARAM_MAS_INSTANCEID = 0x0F;
const uint8_t PARAM_PARAMETER_MASK = 0x10;
const uint8_t PARAM_FOLDER_LISTING_SIZE = 0x11;
const uint8_t PARAM_LISTING_SIZE = 0x12;
const uint8_t PARAM_SUBJECT_LENGTH = 0x13;
const uint8_t PARAM_CHARSET = 0x14;
const uint8_t PARAM_FRACTION_REQUEST = 0x15;
const uint8_t PARAM_FRACTION_DELIVER = 0x16;
const uint8_t PARAM_STATUS_INDICATOR = 0x17;
const uint8_t PARAM_STATUS_VALUE = 0x18;
const uint8_t PARAM_MSE_TIME = 0x19;
const uint8_t PARAM_DATABASE_IDENTIFIER = 0x1A;
const uint8_t PARAM_CONVERSATION_LISTING_VERSION_COUNTER = 0x1B;
const uint8_t PARAM_PRESENCE_AVAILABILITY = 0x1C;
const uint8_t PARAM_PRESENCE_TEXT = 0x1D;
const uint8_t PARAM_LAST_ACTIVITY = 0x1E;
const uint8_t PARAM_FILTER_LAST_ACTIVITY_BEGIN = 0x1F;
const uint8_t PARAM_FILTER_LAST_ACTIVITY_END = 0x20;
const uint8_t PARAM_CHAT_STATE = 0x21;
const uint8_t PARAM_CONVERSATION_ID = 0x22;
const uint8_t PARAM_FOLDER_VERSION_COUNTER = 0x23;
const uint8_t PARAM_FILTER_MESSAGE_HANDLE = 0x24;
const uint8_t PARAM_NOTIFICATION_FILTER_MASK = 0x25;
const uint8_t PARAM_CONV_PARAMETER_MASK = 0x26;
const uint8_t PARAM_OWNER_UCI = 0x27;
const uint8_t PARAM_EXTENDED_DATA = 0x28;
const uint8_t PARAM_MAP_SUPPORTED_FEATURES = 0x29;
const uint8_t PARAM_CONVO_LISTING_SIZE = 0x36;

const uint8_t TAG_LEN_MAX_LIST_COUNT = 0x02;
const uint8_t TAG_LEN_LIST_START_OFF_SET = 0x02;
const uint8_t TAG_LEN_FILTER_MESSAGE_TYPE = 0x01;
const uint8_t TAG_LEN_FILTER_READ_STATUS = 0x01;
const uint8_t TAG_LEN_FILTER_PRIORITY = 0x01;
const uint8_t TAG_LEN_ATTACHMENT = 0x01;
const uint8_t TAG_LEN_TRANSPARENT = 0x01;
const uint8_t TAG_LEN_RETRY = 0x01;
const uint8_t TAG_LEN_NEW_MESSAGE = 0x01;
const uint8_t TAG_LEN_NOTIFICATION_STATUS = 0x01;
const uint8_t TAG_LEN_MAS_INSTANCEID = 0x01;
const uint8_t TAG_LEN_PARAMETER_MASK = 0x04;
const uint8_t TAG_LEN_FOLDER_LISTING_SIZE = 0x02;
const uint8_t TAG_LEN_LISTING_SIZE = 0x02;
const uint8_t TAG_LEN_SUBJECT_LENGTH = 0x01;
const uint8_t TAG_LEN_CHARSET = 0x01;
const uint8_t TAG_LEN_FRACTION_REQUEST = 0x01;
const uint8_t TAG_LEN_FRACTION_DELIVER = 0x01;
const uint8_t TAG_LEN_STATUS_INDICATOR = 0x01;
const uint8_t TAG_LEN_STATUS_VALUE = 0x01;
const uint8_t TAG_LEN_DATABASE_IDENTIFIER = 0x10;
const uint8_t TAG_LEN_CONVERSATION_LISTING_VERSION_COUNTER = 0x10;
const uint8_t TAG_LEN_PRESENCE_AVAILABILITY = 0x01;
const uint8_t TAG_LEN_CHAT_STATE = 0x01;
const uint8_t TAG_LEN_CONVERSATION_ID = 0x10;
const uint8_t TAG_LEN_FOLDER_VERSION_COUNTER = 0x10;
const uint8_t TAG_LEN_FILTER_MESSAGE_HANDLE = 0x08;
const uint8_t TAG_LEN_NOTIFICATION_FILTER_MASK = 0x04;
const uint8_t TAG_LEN_CONV_PARAMETER_MASK = 0x04;
const uint8_t TAG_LEN_MAP_SUPPORTED_FEATURES = 0x04;
const uint8_t TAG_LEN_CONVO_LISTING_SIZE = 0x02;

const uint8_t FILTER_NO_SMS_GSM = 0x01;
const uint8_t FILTER_NO_SMS_CDMA = 0x02;
const uint8_t FILTER_NO_EMAIL = 0x04;
const uint8_t FILTER_NO_MMS = 0x08;
const uint8_t FILTER_NO_IM = 0x10;
const uint8_t FILTER_MSG_TYPE_MASK = 0x1F;

// Parameter Mask for selection of parameters to return in listings
const uint32_t MASK_SUBJECT = 0x00000001;
const uint32_t MASK_DATETIME = 0x00000002;
const uint32_t MASK_SENDER_NAME = 0x00000004;
const uint32_t MASK_SENDER_ADDRESSING = 0x00000008;
const uint32_t MASK_RECIPIENT_NAME = 0x00000010;
const uint32_t MASK_RECIPIENT_ADDRESSING = 0x00000020;
const uint32_t MASK_TYPE = 0x00000040;
const uint32_t MASK_SIZE = 0x00000080;
const uint32_t MASK_RECEPTION_STATUS = 0x00000100;
const uint32_t MASK_TEXT = 0x00000200;
const uint32_t MASK_ATTACHMENT_SIZE = 0x00000400;
const uint32_t MASK_PRIORITY = 0x00000800;
const uint32_t MASK_READ = 0x00001000;
const uint32_t MASK_SENT = 0x00002000;
const uint32_t MASK_PROTECTED = 0x00004000;
const uint32_t MASK_REPLYTO_ADDRESSING = 0x00008000;
const uint32_t MASK_DELIVERY_STATUS = 0x00010000;
const uint32_t MASK_CONVERSATION_ID = 0x00020000;
const uint32_t MASK_CONVERSATION_NAME = 0x00040000;
const uint32_t MASK_FOLDER_TYPE = 0x00100000;
const uint32_t MASK_ATTACHMENT_MIME = 0x00100000;
const uint32_t CONVO_PARAM_MASK_CONVO_NAME = 0x00000001;
const uint32_t CONVO_PARAM_MASK_CONVO_LAST_ACTIVITY = 0x00000002;
const uint32_t CONVO_PARAM_MASK_CONVO_READ_STATUS = 0x00000004;
const uint32_t CONVO_PARAM_MASK_CONVO_VERSION_COUNTER = 0x00000008;
const uint32_t CONVO_PARAM_MASK_CONVO_SUMMARY = 0x00000010;
const uint32_t CONVO_PARAM_MASK_PARTTICIPANTS = 0x00000020;
const uint32_t CONVO_PARAM_MASK_PART_UCI = 0x00000040;
const uint32_t CONVO_PARAM_MASK_PART_DISP_NAME = 0x00000080;
const uint32_t CONVO_PARAM_MASK_PART_CHAT_STATE = 0x00000100;
const uint32_t CONVO_PARAM_MASK_PART_LAST_ACTIVITY = 0x00000200;
const uint32_t CONVO_PARAM_MASK_PART_X_BT_UID = 0x00000400;
const uint32_t CONVO_PARAM_MASK_PART_NAME = 0x00000800;
const uint32_t CONVO_PARAM_MASK_PART_PRESENCE = 0x00001000;
const uint32_t CONVO_PARAM_MASK_PART_PRESENCE_TEXT = 0x00002000;
const uint32_t CONVO_PARAM_MASK_PART_PRIORITY = 0x00004000;
const uint32_t MAP_FEATURE_DEFAULT_BITMASK = 0x0000001F;
const uint32_t PARAMETER_MASK_ALL_ENABLED = 0xFFFFFFFF;

class MapMseAppParams {
public:
    explicit MapMseAppParams();
    explicit MapMseAppParams(const std::vector<uint8_t> &appParams);
    void GetEncodeData(std::vector<uint8_t> &output);

    void SetConversationListingVersionCounter(int64_t highData, int64_t lowData);
    void SetDatabaseIdentifier(int64_t highData, int64_t lowData);
    void SetMasInsanceId(int32_t masInsanceId);

private:
    void ParseParams(const std::vector<uint8_t> &appParams);
    void ParseMaxListCount(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseListStartOffSet(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterMessageType(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterPeriodBegin(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterPeriodEnd(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterReadStatus(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterRecipient(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterOriginator(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterPriority(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseAttachment(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseTransparent(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseRetry(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseNewMessage(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseNotificationStatus(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseMasInstanceid(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseParameterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFolderListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseSubjectLength(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseCharset(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFractionRequest(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFractionDeliver(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseStatusIndicator(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseStatusValue(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseMseTime(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseDatabaseIdentifier(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseConversationListingVersionCounter(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParsePresenceAvailability(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParsePresenceText(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseLastActivity(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterLastActivityBegin(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterLastActivityEnd(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseChatState(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseConversationId(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFolderVersionCounter(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseFilterMessageHandle(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseNotificationFilterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseConvParameterMask(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    void ParseConvoListingSize(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);

public:
    std::optional<uint16_t> maxListCount_;
    std::optional<uint16_t> listStartOffSet_;
    std::optional<uint8_t> filterMessageType_;
    std::optional<std::string> filterPeriodBegin_;
    std::optional<std::string> filterPeriodEnd_;
    std::optional<uint8_t> filterReadStatus_;
    std::optional<std::string> filterRecipient_;
    std::optional<std::string> filterOriginator_;
    std::optional<uint8_t> filterPriority_;
    std::optional<uint8_t> attachment_;
    std::optional<uint8_t> transparent_;
    std::optional<uint8_t> retry_;
    std::optional<uint8_t> newMessage_;
    std::optional<uint8_t> notificationStatus_;
    std::optional<uint8_t> masInstanceId_;
    std::optional<int32_t> parameterMask_;
    std::optional<uint16_t> folderListingSize_;
    std::optional<uint16_t> messagesListingSize_;
    std::optional<uint8_t> subjectLength_;
    std::optional<uint8_t> charSet_;
    std::optional<uint8_t> fractionRequest_;
    std::optional<uint8_t> fractionDeliver_;
    std::optional<uint8_t> statusIndicator_;
    std::optional<uint8_t> statusValue_;
    std::optional<std::string> mseTime_;

    std::optional<int64_t> databaseIdentifierLow_;
    std::optional<int64_t> databaseIdentifierHigh_;
    std::optional<int64_t> conversationListingVersionCounterLow_;
    std::optional<int64_t> conversationListingVersionCounterHigh_;
    std::optional<uint8_t> presenceAvailability_;
    std::optional<std::string> presenceText_;
    std::optional<std::string> lastActivity_;
    std::optional<std::string> filterLastActivityBegin_;
    std::optional<std::string> filterLastActivityEnd_;
    std::optional<uint8_t> chatState_;
    std::optional<int64_t> conversationIdLow_;
    std::optional<int64_t> conversationIdHigh_;
    std::optional<int64_t> folderVerCounterLow_;
    std::optional<int64_t> folderVerCounterHigh_;
    std::optional<int64_t> filterMessageHandle_;
    std::optional<uint32_t> notificationFilterMask_;
    std::optional<uint32_t> convParameterMask_;
    std::optional<std::string> ownerUci_;
    std::optional<std::string> extendedData_;
    std::optional<uint32_t> mapSupportedFeatures_;
    std::optional<uint16_t> convoListingSize_;

    using ParseFunc = void (MapMseAppParams::*)(const std::vector<uint8_t> &appParams, int32_t pos, int32_t tagLength);
    std::unordered_map<int32_t, ParseFunc> parseFuncs = {{PARAM_MAX_LIST_COUNT, &MapMseAppParams::ParseMaxListCount},
        {PARAM_LIST_START_OFF_SET, &MapMseAppParams::ParseListStartOffSet},
        {PARAM_FILTER_MESSAGE_TYPE, &MapMseAppParams::ParseFilterMessageType},
        {PARAM_FILTER_PERIOD_BEGIN, &MapMseAppParams::ParseFilterPeriodBegin},
        {PARAM_FILTER_PERIOD_END, &MapMseAppParams::ParseFilterPeriodEnd},
        {PARAM_FILTER_READ_STATUS, &MapMseAppParams::ParseFilterReadStatus},
        {PARAM_FILTER_RECIPIENT, &MapMseAppParams::ParseFilterRecipient},
        {PARAM_FILTER_ORIGINATOR, &MapMseAppParams::ParseFilterOriginator},
        {PARAM_FILTER_PRIORITY, &MapMseAppParams::ParseFilterPriority},
        {PARAM_ATTACHMENT, &MapMseAppParams::ParseAttachment},
        {PARAM_TRANSPARENT, &MapMseAppParams::ParseTransparent},
        {PARAM_RETRY, &MapMseAppParams::ParseRetry},
        {PARAM_NEW_MESSAGE, &MapMseAppParams::ParseNewMessage},
        {PARAM_NOTIFICATION_STATUS, &MapMseAppParams::ParseNotificationStatus},
        {PARAM_MAS_INSTANCEID, &MapMseAppParams::ParseMasInstanceid},
        {PARAM_PARAMETER_MASK, &MapMseAppParams::ParseParameterMask},
        {PARAM_FOLDER_LISTING_SIZE, &MapMseAppParams::ParseFolderListingSize},
        {PARAM_LISTING_SIZE, &MapMseAppParams::ParseListingSize},
        {PARAM_SUBJECT_LENGTH, &MapMseAppParams::ParseSubjectLength},
        {PARAM_CHARSET, &MapMseAppParams::ParseCharset},
        {PARAM_FRACTION_REQUEST, &MapMseAppParams::ParseFractionRequest},
        {PARAM_FRACTION_DELIVER, &MapMseAppParams::ParseFractionDeliver},
        {PARAM_STATUS_INDICATOR, &MapMseAppParams::ParseStatusIndicator},
        {PARAM_STATUS_VALUE, &MapMseAppParams::ParseStatusValue},
        {PARAM_MSE_TIME, &MapMseAppParams::ParseMseTime},
        {PARAM_DATABASE_IDENTIFIER, &MapMseAppParams::ParseDatabaseIdentifier},
        {PARAM_CONVERSATION_LISTING_VERSION_COUNTER, &MapMseAppParams::ParseConversationListingVersionCounter},
        {PARAM_PRESENCE_AVAILABILITY, &MapMseAppParams::ParsePresenceAvailability},
        {PARAM_PRESENCE_TEXT, &MapMseAppParams::ParsePresenceText},
        {PARAM_LAST_ACTIVITY, &MapMseAppParams::ParseLastActivity},
        {PARAM_FILTER_LAST_ACTIVITY_BEGIN, &MapMseAppParams::ParseFilterLastActivityBegin},
        {PARAM_FILTER_LAST_ACTIVITY_END, &MapMseAppParams::ParseFilterLastActivityEnd},
        {PARAM_CHAT_STATE, &MapMseAppParams::ParseChatState},
        {PARAM_CONVERSATION_ID, &MapMseAppParams::ParseConversationId},
        {PARAM_FOLDER_VERSION_COUNTER, &MapMseAppParams::ParseFolderVersionCounter},
        {PARAM_FILTER_MESSAGE_HANDLE, &MapMseAppParams::ParseFilterMessageHandle},
        {PARAM_NOTIFICATION_FILTER_MASK, &MapMseAppParams::ParseNotificationFilterMask},
        {PARAM_CONV_PARAMETER_MASK, &MapMseAppParams::ParseConvParameterMask},
        {PARAM_CONVO_LISTING_SIZE, &MapMseAppParams::ParseConvoListingSize}};
};
}  // namespace bluetooth
}  // namespace OHOS

#endif