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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_map_mse_content"
#endif

#include "map_mse_content.h"
#include <type_traits>
#include <cctype>
#include <charconv>
#include "common_util.h"
#include "core_service_client.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "log.h"
#include "log_utils.h"
#include "hitrace_meter.h"
#include "bluetooth_code_convert_utils.h"

namespace OHOS {
namespace bluetooth {
using namespace Telephony;
const std::string SMS_MMS_DATASHARE_URI = "datashare:///com.ohos.smsmmsability";
const std::string SMS_MMS_INFO_URI_STR = "datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info";
const std::string SMS_MMS_SESSION_URI_STR = "datashare:///com.ohos.smsmmsability/sms_mms/session";
const std::string CONTACT_DATASHARE_URI = "datashare:///com.ohos.contactsdataability";
const std::string VIEW_CONTACT_DATA_URI_STR = "datashare:///com.ohos.contactsdataability/contacts/view_contact_data";
const int HEX_PREFIX_SIZE = 2;
const int SUB_LEN = 10;
static Uri SMS_MMS_INFO_URI = Uri(SMS_MMS_INFO_URI_STR);
static Uri SMS_MMS_SESSION_URI = Uri(SMS_MMS_SESSION_URI_STR);
static std::vector<std::string> SMS_MESSAGE_COLUMNS = {
    "msg_id",
    "is_sender",
    "receiver_number",
    "sender_number",
    "start_time",
    "end_time",
    "msg_title",
    "msg_content",
    "is_read",
    "session_id",
};
static std::string GetSelfPhoneNumber()
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t slotId = 0;
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId);
    std::u16string telephoneNumber;
    CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId, telephoneNumber);

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string result = convert.to_bytes(telephoneNumber);
    return result;
}

MapMseContent::MapMseContent()
{
    CreateSmsDataShareHelper();
}

MapMseContent::~MapMseContent()
{
}

std::string MapMseContent::GetMessageListing(const std::string &folderName,
    const std::shared_ptr<MapMseAppParams> &appParams, const std::string &version, int32_t &listSize, bool &hasUnRead)
{
    if (!smsDataShareHelper_) {
        HILOGE("smsDataShareHelper_ nullptr.");
        return "";
    }
    HILOGI("folderName %{public}s.!", folderName.c_str());
    if (SmsSelected(appParams)) {
        HILOGI("get sms message listing");
        auto messageList = GetSmsMessageListing(folderName, appParams, hasUnRead);
        listSize = messageList.size();
        auto result = EncodeSmsMessageListing(messageList, version);
        return result;
    }
    return "";
}
int32_t MapMseContent::GetMessageListingSize(
    const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead)
{
    if (!smsDataShareHelper_) {
        HILOGE("smsDataShareHelper_ nullptr.");
        return 0;
    }
    HILOGI("folderName %{public}s.!", folderName.c_str());
    int32_t count = 0;
    if (SmsSelected(appParams)) {
        count += GetSmsMessageListingSize(folderName, appParams, hasUnRead);
        HILOGI("SmsSelected count size %{public}d.", count);
    }
    return count;
}

bool IsHexDigit(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char c : str) {
        if (!isxdigit(c)) {
            return false;
        }
    }
    return true;
}

std::string MapMseContent::GetMessage(
    const std::string &handleStr, const std::shared_ptr<MapMseAppParams> &appParams, const std::string &version)
{
    HILOGI("MapMseContent::GetMessage handleStr is %{public}s.", handleStr.c_str());
    int64_t handle;
    CHECK_AND_RETURN_LOG_RET(ConvertStrToDigit(handleStr, handle, 16), "", "FromString failed"); // 16进制
    messageVersion_ = version;
    MessageType msgType = MseMessageElement::GetMessageTypeFromHandle(handle);
    if (msgType == TYPE_SMS_GSM || msgType == TYPE_SMS_CDMA) {
        return GetSmsMessage(handle);
    } else if (msgType == TYPE_MMS) {
    } else if (msgType == TYPE_EMAIL) {
    } else if (msgType == TYPE_IM) {
    }
    return "";
}

std::string MapMseContent::GetConversationListing()
{
    return "";
}

void MapMseContent::CreateSmsDataShareHelper()
{
    HITRACE_METER(BT_TRACE_TAG);
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbilityManager failed.");
        return;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbility Service Failed.");
        return;
    }
    smsDataShareHelper_ = DataShare::DataShareHelper::Creator(remoteObj, SMS_MMS_DATASHARE_URI);
    contactDataShareHelper_ = DataShare::DataShareHelper::Creator(remoteObj, CONTACT_DATASHARE_URI);
}

int32_t MapMseContent::GetSmsMessageListingSize(
    const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t count = 0;
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    Uri uri("");

    SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);
    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGE("GetMessageListingSize Query data failed.");
        return count;
    }

    ParseSmsMessageListingSizeResult(resultSet, folderName, count, hasUnRead);
    return count;
}

std::vector<std::unique_ptr<MseMessageElement>> MapMseContent::GetSmsMessageListing(
    const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("GetSmsMessageListing folder name %{public}s ", folderName.c_str());
    std::vector<std::unique_ptr<MseMessageElement>> messageList;
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    Uri uri("");

    SetSmsMessageListingQueryParam(folderName, appParams, predicates, columns, uri);
    if (appParams->maxListCount_.has_value() && appParams->listStartOffSet_.has_value()) {
        predicates.Limit(appParams->maxListCount_.value(), appParams->listStartOffSet_.value());
        HILOGI("appParams maxListCount_ is %{public}d listStartOffSet_ is %{public}d ",
            appParams->maxListCount_.value(), appParams->listStartOffSet_.value());
    }
    predicates.OrderByDesc("start_time");
    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGE("GetSmsMessageListing Query data failed.");
        return messageList;
    }

    ParseSmsMessageListingResult(resultSet, folderName, appParams, messageList, hasUnRead);
    return messageList;
}

std::string MapMseContent::GetSmsMessage(int64_t handle)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (!smsDataShareHelper_) {
        return "";
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    Uri uri("");
    SetSmsMessageQueryParam(handle, predicates, columns, uri);
    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGE("Query handle message data failed.");
        return "";
    }
    std::shared_ptr<MapMseBMessageSms> message = ParseSmsMessageResult(handle, resultSet);
    return message->Encode();
}

std::string MapMseContent::EncodeSmsMessageListing(
    const std::vector<std::unique_ptr<MseMessageElement>> &messageList, const std::string &version)
{
    std::string xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>";
    xml.append("<MAP-msg-listing version=\"");
    xml.append(version);
    xml.append("\">");
    for (const auto &ele : messageList) {
        ele->GetEncodeData(xml);
    }
    xml.append("</MAP-msg-listing>");
    std::string utf8xml = BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8(xml);
    return utf8xml;
}

static int64_t GetLongFromResultSet(std::shared_ptr<DataShare::DataShareResultSet> &resultSet, const std::string &name)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t index = 0;
    resultSet->GetColumnIndex(name, index);

    int64_t value = 0;
    resultSet->GetLong(index, value);
    return value;
}

static std::string GetStringFromResultSet(
    std::shared_ptr<DataShare::DataShareResultSet> &resultSet, const std::string &name)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t index = 0;
    resultSet->GetColumnIndex(name, index);

    std::string value = "";
    resultSet->GetString(index, value);
    return value;
}

void MapMseContent::ParseSmsMessageListingSizeResult(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &folderName, int32_t &count, bool &hasUnRead)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return;
    }
    while (opRet == DataShare::E_OK) {
        int32_t isReadValue = GetLongFromResultSet(resultSet, "is_read");
        if (isReadValue == 0) {
            hasUnRead = true;
        }
        count++;
        opRet = resultSet->GoToNextRow();
    }
}

void MapMseContent::ParseSmsMessageListingResult(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams,
    std::vector<std::unique_ptr<MseMessageElement>> &messageList, bool &hasUnRead)
{
    HILOGI("folder name %{public}s ", folderName.c_str());
    HITRACE_METER(BT_TRACE_TAG);
    int32_t opRet = resultSet->GoToFirstRow();
    CHECK_AND_RETURN_LOG(opRet == DataShare::E_OK, "GoToFirstRow error. error code is %{public}d.", opRet);
    while (opRet == DataShare::E_OK) {
        auto messageEle = std::make_unique<MseMessageElement>(appParams, threadIdSupport_);
        messageEle->SetHandle(GetLongFromResultSet(resultSet, "msg_id"));
        std::string sent = (GetLongFromResultSet(resultSet, "is_sender") == 0) ? "yes" : "no";
        messageEle->SetSent(sent);
        if (sent == "yes") {
            std::string number = GetStringFromResultSet(resultSet, "receiver_number");
            messageEle->SetRecipientAddressing(number);
            messageEle->SetRecipientName(QueryContactNameByNumber(number));
            messageEle->SetSenderAddressing(GetSelfPhoneNumber());
            messageEle->SetSenderName("");
        } else if (sent == "no") {
            messageEle->SetRecipientAddressing(GetSelfPhoneNumber());
            messageEle->SetRecipientName("");
            std::string number = GetStringFromResultSet(resultSet, "sender_number");
            messageEle->SetSenderAddressing(number);
            messageEle->SetSenderName(QueryContactNameByNumber(number));
        }
        int64_t dateTime;
        std::string startTime = GetStringFromResultSet(resultSet, "start_time");
        CHECK_AND_RETURN_LOG(ConvertStrToDigit(startTime, dateTime), "parse startTime error");
        messageEle->SetDateTime(dateTime);
        std::string subject = GetStringFromResultSet(resultSet, "msg_title");
        std::string con = GetStringFromResultSet(resultSet, "msg_content");
        messageEle->SetSubject(subject.empty() ? (con.length() > 0 ? con : "") : subject);
        messageEle->SetSize(subject.size());
        messageEle->SetText((GetStringFromResultSet(resultSet, "msg_content").length() > 0) ? "yes" : "no");
        bool read = (GetLongFromResultSet(resultSet, "is_read") == 1) ? true : false;
        messageEle->SetRead(read);
        messageEle->SetPriority("no");
        messageEle->SetType(TYPE_SMS_GSM);  // 2 GSM or 3 CDMA，phoneType 需要获取
        messageEle->SetReceptionStatus("complete");
        messageEle->SetAttachmentSize(0);
        messageEle->SetProtect("no");
        if (messageListingVersion_ > MAP_MESSAGE_LISTING_FORMAT_V10) {
            messageEle->SetDeliveryStatus("delivered");
            messageEle->SetThreadId("");
        }
        messageList.push_back(std::move(messageEle));
        opRet = resultSet->GoToNextRow();
        hasUnRead = (!read) ? true : hasUnRead;
    }
}

void MapMseContent::SetSmsMessageListingQueryParam(const std::string &folderName,
    const std::shared_ptr<MapMseAppParams> &appParams, DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, Uri &uri)
{
    HITRACE_METER(BT_TRACE_TAG);
    uri = SMS_MMS_INFO_URI;
    columns = SMS_MESSAGE_COLUMNS;
    if (folderName == MAP_MSE_FOLDER_NAME_DRAFT) {
        predicates.EqualTo("msg_state", 3); // 3 means draft message
    } else if (folderName == MAP_MSE_FOLDER_NAME_INBOX) {
        predicates.EqualTo("is_sender", 1)->And()->NotEqualTo("session_id", -1);
    } else if (folderName == MAP_MSE_FOLDER_NAME_OUTBOX) {
        predicates.EqualTo("is_sender", 2); // 2 means outbox type in mms database
    } else if (folderName == MAP_MSE_FOLDER_NAME_SENT) {
        predicates.EqualTo("is_sender", 0)->And()->NotEqualTo("session_id", -1);
    } else if (folderName == MAP_MSE_FOLDER_NAME_DELETED) {
        predicates.EqualTo("session_id", -1);
    } else if (folderName == MAP_MSE_FOLDER_NAME_OTHER) {
        predicates.EqualTo("session_id", 1);
    } else {
        uri = Uri("");
        columns = std::move(std::vector<std::string>{});
        HILOGE("folderName unknow");
        return;
    }
    if (folderName != MAP_MSE_FOLDER_NAME_DELETED) {
        // delete_time more than 0 means has been deleted
        predicates.And()->EqualTo("delete_time", 0);
    }
    // messge handle enabled, other filter will be ignored
    if (appParams->filterMessageHandle_.has_value()) {
        predicates.And()->EqualTo("msg_id", appParams->filterMessageHandle_.value());
    } else {
        SetOtherFiltersQueryParam(folderName, appParams, predicates);
    }
}

void MapMseContent::SetOtherFiltersQueryParam(const std::string &folderName,
    const std::shared_ptr<MapMseAppParams> &appParams, DataShare::DataSharePredicates &predicates)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (appParams->filterReadStatus_.has_value()) {
        if ((appParams->filterReadStatus_.value() & 0x01) != 0) {
            predicates.And()->EqualTo("is_read", 0);
        }
        if ((appParams->filterReadStatus_.value() & 0x02) != 0) {
            predicates.And()->EqualTo("is_read", 1);
        }
    }

    if (appParams->filterPeriodBegin_.has_value()) {
        predicates.And()->GreaterThanOrEqualTo("start_time", appParams->filterPeriodBegin_.value());
    }
    if (appParams->filterPeriodEnd_.has_value()) {
        predicates.And()->LessThan("start_time", appParams->filterPeriodEnd_.value());
    }

    if (appParams->conversationIdLow_.has_value()) {
        predicates.And()->EqualTo("session_id", appParams->conversationIdLow_.value());
    }
}

void MapMseContent::SetSmsMessageQueryParam(
    int64_t handle, DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns, Uri &uri)
{
    HITRACE_METER(BT_TRACE_TAG);
    int64_t parsedHandle = MseMessageElement::GetParsedMessageHandle(handle);
    HILOGI("handle is %{public}ld!", parsedHandle);
    uri = SMS_MMS_INFO_URI;
    columns = SMS_MESSAGE_COLUMNS;
    predicates.EqualTo("msg_id", parsedHandle);
}

std::shared_ptr<MapMseBMessageSms> MapMseContent::ParseSmsMessageResult(
    int64_t handle, std::shared_ptr<DataShare::DataShareResultSet> &resultSet)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<MapMseBMessageSms> message = std::make_shared<MapMseBMessageSms>();
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return message;
    }
    while (opRet == DataShare::E_OK) {
        message->SetType(MseMessageElement::GetMessageTypeStr(TYPE_SMS_GSM));
        message->SetVersion(messageVersion_);
        message->SetStatus(GetLongFromResultSet(resultSet, "is_read") ? "READ" : "UNREAD");
        if (GetLongFromResultSet(resultSet, "is_sender") == 0) {
            message->SetFolder("telecom/msg/sent");
            std::string number = GetStringFromResultSet(resultSet, "receiver_number");
            SetSmsMessageVcardData(message, number, false);
        } else {
            message->SetFolder("telecom/msg/inbox");
            std::string number = GetStringFromResultSet(resultSet, "sender_number");
            SetSmsMessageVcardData(message, number, true);
        }
        message->SetCharset("UTF-8");
        message->SetMessageBody(GetStringFromResultSet(resultSet, "msg_content"));
        opRet = resultSet->GoToNextRow();
    }
    return message;
}

void MapMseContent::SetSmsMessageVcardData(
    std::shared_ptr<MapMseBMessageSms> &message, const std::string &number, bool incoming)
{
    if (number.empty()) {
        HILOGW("number is empty.");
        return;
    }
    std::string name = "";
    std::vector<std::string> emails;
    std::vector<std::string> phoneNumbers = {number};

    int64_t contactId = QueryContactIdByNumber(number);
    if (contactId != -1) {
        name = QueryContactNameByContactId(contactId);
        emails = QueryContactEmailsByContactId(contactId);
    }
    MapMseBMessage::MapMseVcard::VcardVer3 vcard{
        .name = name,
        .version = "3.0",
        .formattedName = name,
        .phoneNumbers = std::move(phoneNumbers),
        .emailAddresses = std::move(emails),
        .btUids = std::move(std::vector<std::string>{}),
        .btUcis = std::move(std::vector<std::string>{}),
    };
    if (incoming) {
        message->AddOriginator(MapMseBMessage::MapMseVcard(vcard));
    } else {
        message->AddRecipient(MapMseBMessage::MapMseVcard(vcard));
    }
}

std::string MapMseContent::QueryContactNameByNumber(const std::string &number)
{
    int64_t contactId = QueryContactIdByNumber(number);
    return QueryContactNameByContactId(contactId);
}

int64_t MapMseContent::QueryContactIdByNumber(const std::string &number)
{
    HITRACE_METER(BT_TRACE_TAG);
    int64_t contactId = -1;
    if (number.empty()) {
        HILOGW("number is empty.");
        return contactId;
    }
    if (!smsDataShareHelper_) {
        return contactId;
    }
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("type_id", 5)->And()->EqualTo("detail_info", number);  // 5 means contact database number type
    std::vector<std::string> columns = {"raw_contact_id"};
    Uri uri(VIEW_CONTACT_DATA_URI_STR);

    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGI_TIME_LIMIT(__func__, "GetSmsMessage Query data failed.");
        return contactId;
    }
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return contactId;
    }
    contactId = GetLongFromResultSet(resultSet, "raw_contact_id");
    return contactId;
}

std::string MapMseContent::QueryContactNameByContactId(int64_t contactId)
{
    HITRACE_METER(BT_TRACE_TAG);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("raw_contact_id", contactId)->And()->EqualTo("type_id", 1);
    std::vector<std::string> columns = {"detail_info"};
    Uri uri(VIEW_CONTACT_DATA_URI_STR);

    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGI_TIME_LIMIT(__func__, "GetSmsMessage Query data failed.");
        return "";
    }
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return "";
    }

    std::string name = GetStringFromResultSet(resultSet, "detail_info");
    return std::move(name);
}

std::vector<std::string> MapMseContent::QueryContactEmailsByContactId(int64_t contactId)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::vector<std::string> emails;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("raw_contact_id", contactId)->And()->EqualTo("type_id", 6);  // 6 means contact email type
    std::vector<std::string> columns = {"detail_info"};
    Uri uri(VIEW_CONTACT_DATA_URI_STR);

    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGE("GetSmsMessage Query data failed.");
        return std::move(emails);
    }
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return std::move(emails);
    }
    while (opRet == DataShare::E_OK) {
        emails.push_back(GetStringFromResultSet(resultSet, "detail_info"));
        opRet = resultSet->GoToNextRow();
    }
    return std::move(emails);
}

bool MapMseContent::SmsSelected(const std::shared_ptr<MapMseAppParams> &appParams)
{
    // need get phone type
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS