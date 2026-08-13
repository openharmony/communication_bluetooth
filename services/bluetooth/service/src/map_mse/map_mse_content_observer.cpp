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
#define LOG_TAG "bt_service_map_mse_content"
#endif

#include "common_event_manager.h"
#include "common_util.h"
#include "map_mse_content_observer.h"
#include "../obex/obex_def.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "sms_service_manager_client.h"
#include "string_ex.h"
#include "log.h"
#include "hitrace_meter.h"
#include "want.h"
#include "bt_chr_dft_exception.h"

namespace {
std::vector<uint8_t> Utf8ToUnicode(const std::string &utf8Str)
{
    const uint8_t lengthMultiple = 2;
    std::vector<uint8_t> result(utf8Str.length() * lengthMultiple + lengthMultiple);
    for (uint32_t i = 0; i < utf8Str.length(); i++) {
        result[(i * lengthMultiple)] = (utf8Str[i] >> 8);  // Convert Multiply 8
        result[((i * lengthMultiple) + 1)] = utf8Str[i];
    }

    // Add the UNICODE null character
    result[result.size() - lengthMultiple] = 0;
    result[result.size() - 1] = 0;

    return result;
}

int64_t GetLongFromResultSet(std::shared_ptr<OHOS::DataShare::DataShareResultSet> &resultSet, const std::string &name)
{
    int32_t index = 0;
    resultSet->GetColumnIndex(name, index);

    int64_t value = 0;
    resultSet->GetLong(index, value);
    return value;
}
}  // namespace

namespace OHOS {
namespace bluetooth {
const std::string SMS_MMS_DATASHARE_URI = "datashare:///com.ohos.smsmmsability";
const std::string SMS_MMS_INFO_URI_STR = "datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info";
const std::string SMS_MMS_SESSION_URI_STR = "datashare:///com.ohos.smsmmsability/sms_mms/session";
const std::string SMS_MMS_READSTATUS_URI_STR = "datashare:///com.ohos.smsmmsability/sms_mms/single_mark_unread";
const std::string EVENT_TYPE_NEW = "NewMessage";
const std::string EVENT_TYPE_DELETE = "MessageDeleted";
const std::string EVENT_TYPE_REMOVED = "MessageRemoved";
const std::string EVENT_TYPE_SHIFT = "MessageShift";
const std::string EVENT_TYPE_DELEVERY_SUCCESS = "DeliverySuccess";
const std::string EVENT_TYPE_SENDING_SUCCESS = "SendingSuccess";
const std::string EVENT_TYPE_SENDING_FAILURE = "SendingFailure";
const std::string EVENT_TYPE_DELIVERY_FAILURE = "DeliveryFailure";
const std::string EVENT_TYPE_READ_STATUS = "ReadStatusChanged";
const std::string EVENT_TYPE_CONVERSATION = "ConversationChanged";
const std::string EVENT_TYPE_PRESENCE = "ParticipantPresenceChanged";
const std::string EVENT_TYPE_CHAT_STATE = "ParticipantChatStateChanged";
const std::string MAP_EVENT_FOLDER_INBOX = "telecom/msg/inbox";
const std::string MAP_EVENT_FOLDER_OUTBOX = "telecom/msg/outbox";
const std::string MAP_EVENT_FOLDER_SENT = "telecom/msg/sent";
const std::string MAP_EVENT_FOLDER_DRAFT = "telecom/msg/draft";
const std::vector<std::string> READ_MESSAGES_PERMISSIONS {"ohos.permission.READ_MESSAGES"};

const uint8_t STATUS_INDICATOR_READ = 0;
const uint8_t STATUS_INDICATOR_DELETED = 1;
const uint8_t MESSAGE_STATE_IS_DRAFT = 3;

int32_t MapMseContentObserver::SetNotificationRegistration(int32_t masId, std::shared_ptr<MapMseAppParams> &params)
{
    if (!params->notificationStatus_.has_value()) {
        HILOGE("notificationStatus is empty");
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    masInstanceId_ = masId;
    std::thread([client = mnsClient_, masId, params] {
        auto sharedClient = client.lock();
        if (sharedClient != nullptr) {
            sharedClient->HandleRegistration(masId, params->notificationStatus_.value());
        }
    }).detach();
    return ResponseCodes::OBEX_HTTP_OK;
}

int32_t MapMseContentObserver::SetMessageStatus(const std::string &handle, std::shared_ptr<MapMseAppParams> &params)
{
    if (!params->statusIndicator_.has_value() || !params->statusValue_.has_value()) {
        HILOGE("statusIndicator_ is empty");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }
    uint8_t indicator = params->statusIndicator_.value();
    uint8_t statusValue = params->statusValue_.value();
    uint64_t messageHandle;
    CHECK_AND_RETURN_LOG_RET(ConvertStrToDigit(handle, messageHandle, HEX_STRING_TO_INT),
        ResponseCodes::OBEX_HTTP_PRECON_FAILED, "parse length error");
    int64_t newHandle = MseMessageElement::GetParsedMessageHandle(messageHandle);
    HILOGI("indicator = %{public}u, statusValue = %{public}u, handle = %{public}u", indicator, statusValue, newHandle);

    bool ret = false;
    if (indicator == STATUS_INDICATOR_READ) {
        ret = SetMessageStatusRead(newHandle, statusValue);
    } else if (indicator == STATUS_INDICATOR_DELETED) {
        ret = SetMessageStatusDeleted(newHandle);
    } else {
        HILOGE("error indicator: %{public}u", indicator);
    }

    if (!ret) {
        HILOGE("SetMessageStatus error");
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

int32_t MapMseContentObserver::SendNewMessage(const std::string &messageVersion, std::shared_ptr<MapMseFolder> &folder,
    std::shared_ptr<MapMseAppParams> &params, ObexHeader &reply, std::vector<uint8_t> &input)
{
    std::unique_ptr<MapMseBMessageSms> message = std::make_unique<MapMseBMessageSms>();
    message->ParseMessage(input, params->charSet_.value());

    auto messageBody = message->GetMessageBody();
    if (messageBody.empty()) {
        HILOGE("message body is empty");
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    if (message->GetType() != MseMessageElement::GetMessageTypeStr(MessageType::TYPE_SMS_GSM) &&
        message->GetType() != MseMessageElement::GetMessageTypeStr(MessageType::TYPE_SMS_CDMA)) {
        HILOGE("message type error");
        return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }
    if (folder->GetName() != MAP_MSE_FOLDER_NAME_OUTBOX) {
        HILOGI("don't need send message");
        return ResponseCodes::OBEX_HTTP_OK;
    }

    for (auto vcard : message->GetRecipient()) {
        std::string phone = vcard.GetFirstPhoneNumber();
        std::string email = vcard.GetFirstEmail();

        // 调用sms的接口，发送
        std::u16string destAddr = Str8ToStr16(phone);
        std::u16string msgBody = Str8ToStr16(messageBody);
        int32_t slotId = Singleton<Telephony::SmsServiceManagerClient>::GetInstance().GetDefaultSmsSlotId();
        mapSendMessageCallback_ = new MapSendMessageCallback(*this);
        int64_t sentHandle = -1;
        {
            std::lock_guard<std::mutex> lock(smsMessageCollectionLock_);
            int32_t result = Singleton<Telephony::SmsServiceManagerClient>::GetInstance()
                .SendMessage(slotId, destAddr, Str8ToStr16(""), msgBody, mapSendMessageCallback_, nullptr);
            HILOGI("SendMessage result %{public}d.", result);
            sentHandle = GetSendMessageHandle();
        }
        if (sentHandle < 0) {
            HILOGE("send message error, maybe no sim crad");
            return ResponseCodes::OBEX_HTTP_UNAVAILABLE;
        }
        auto handleStr = MseMessageElement::GenerateStringHandle(sentHandle, MessageType::TYPE_SMS_GSM);
        std::vector<uint8_t> handleRaw = Utf8ToUnicode(handleStr);
        reply.SetHeader(NAME, handleRaw);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

void MapMseContentObserver::UpdateMnsClient(std::weak_ptr<MapMseMnsClient> mnsClient)
{
    if (mnsClient.lock() == nullptr) {
        HILOGE("mnsClient is nullptr");
        return;
    }
    mnsClient_ = mnsClient;
}

void MapMseContentObserver::HandleSmsMessageListChanged()
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("recv new message or deliverd success");
    std::lock_guard<std::mutex> lock(smsMessageCollectionLock_);
    auto resultSet = QueryMessages();
    if (!resultSet) {
        HILOGE("QueryMessages failed.");
        BtChrBtExcpEvent("", BTOPT_QUERY_MAP_DATASHARE_FAIL, CHR_SUB_ERRCODE_CASE1);
        return;
    }
    std::map<int64_t, Message> currMessageList;
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error may no message. error code %{public}d", opRet);
        return;
    }
    while (opRet == DataShare::E_OK) {
        int64_t handle = GetLongFromResultSet(resultSet, "msg_id");
        int64_t msgState = GetLongFromResultSet(resultSet, "msg_state");
        bool sent = (GetLongFromResultSet(resultSet, "is_sender") == 0) ? true : false;
        bool read = (GetLongFromResultSet(resultSet, "is_read") == 1) ? true : false;
        Message msg{handle, msgState, sent, read};
        if (!smsMessageCollection_.count(handle)) {
            // new message
            HandleListChangeForNewMessage(currMessageList, msg);
        } else {
            // exist message will update state, such as update read status
            HandleListChangeForExistMessage(currMessageList, msg);
        }
        opRet = resultSet->GoToNextRow();
    }
    // handle if message has been deleted
    HandleListChangeForDeleteMessage(currMessageList);

    smsMessageCollection_ = std::move(currMessageList);
}

void MapMseContentObserver::HandleSmsSendResult(const ISendShortMessageCallback::SmsSendResult result)
{
    std::lock_guard<std::mutex> lock(smsMessageCollectionLock_);
    if (smsMessageCollection_.count(smsMessageSentHandle_) == 0) {
        HILOGE("send success but database not update");
        return;
    }
    std::string eventType = (result == ISendShortMessageCallback::SEND_SMS_SUCCESS) ? EVENT_TYPE_SENDING_SUCCESS
                                                                                    : EVENT_TYPE_SENDING_FAILURE;
    MapEvent event(smsMessageSentHandle_, eventType, MAP_EVENT_FOLDER_SENT);
    HILOGI("SendMessage result %{public}s.", eventType.c_str());
    event.SetReportVersion(MAP_EVENT_REPORT_V10);
    SendEvent(event);
}

bool MapMseContentObserver::SetMessageStatusRead(int64_t handle, int32_t statusValue)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (smsDataShareHelper_ == nullptr) {
        HILOGE("smsDataShareHelper_ is null");
        return false;
    }
    HILOGI("SetMessageStatusRead msg_id %{public}d.", handle);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("msg_id", static_cast<double>(handle));
    predicates.EqualTo("msg_type", "sms_mms_info");
 
    Uri uri(SMS_MMS_READSTATUS_URI_STR);
    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject valueObj(statusValue);
    valuesBucket.Put("is_read", valueObj);

    int32_t ret = smsDataShareHelper_->Update(uri, predicates, valuesBucket);
    if (ret == -1) {
        HILOGE("Update fail");
        return false;
    }
    PublishRefreshSmsListEvent();
    return true;
}

bool MapMseContentObserver::SetMessageStatusDeleted(int64_t handle)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (smsDataShareHelper_ == nullptr) {
        HILOGE("smsDataShareHelper_ is null");
        return false;
    }
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("msg_id", handle);
    Uri uri(SMS_MMS_INFO_URI_STR);
    int32_t ret = smsDataShareHelper_->Delete(uri, predicates);
    if (ret == -1) {
        HILOGE("Delete fail");
        return false;
    }
    PublishRefreshSmsListEvent();
    return true;
}

bool MapMseContentObserver::PublishRefreshSmsListEvent()
{
    AAFwk::Want eventWant;
    eventWant.SetAction("refreshConversionList");
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(eventWant);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSticky(false);
    publishInfo.SetSubscriberPermissions(READ_MESSAGES_PERMISSIONS);
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    HILOGI("eventAction: %{public}s publishResult = %{public}d", eventWant.GetAction().c_str(), publishResult);
    return publishResult;
}

void MapMseContentObserver::SubscribeSmsEvent()
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("SubscribeSmsEvent");
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
    if (smsDataShareHelper_ && mapDataShareObserver_ == nullptr) {
        mapDataShareObserver_ = std::make_shared<MapDataShareObserver>(*this);
        smsDataShareHelper_->RegisterObserverExt(Uri(SMS_MMS_INFO_URI_STR), mapDataShareObserver_, true);
    }
}

void MapMseContentObserver::UnSubscribeSmsEvent()
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("UnSubscribeSmsEvent");
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
    if (smsDataShareHelper_) {
        smsDataShareHelper_->UnregisterObserverExt(Uri(SMS_MMS_INFO_URI_STR), mapDataShareObserver_);
        mapDataShareObserver_ = nullptr;
    }
}

void MapMseContentObserver::InitMessageList()
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = nullptr;
    while (!resultSet && retryQueryTimes_ < 3) { // 3代表最多重试3次，保证不因重试导致耗时
        resultSet = QueryMessages();
        retryQueryTimes_++;
    }
    if (!resultSet) {
        HILOGE("InitMessageList failed.");
        BtChrBtExcpEvent("", BTOPT_QUERY_MAP_DATASHARE_FAIL, CHR_SUB_ERRCODE_CASE1);
        return;
    }
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return;
    }
    while (opRet == DataShare::E_OK) {
        int64_t handle = GetLongFromResultSet(resultSet, "msg_id");
        int64_t msgState = GetLongFromResultSet(resultSet, "msg_state");
        bool sent = GetLongFromResultSet(resultSet, "is_sender") == 0 ? true : false;
        bool read = (GetLongFromResultSet(resultSet, "is_read") == 1) ? true : false;
        Message msg{handle, msgState, sent, read};
        smsMessageCollection_[handle] = msg;
        opRet = resultSet->GoToNextRow();
    }
}

std::shared_ptr<DataShare::DataShareResultSet> MapMseContentObserver::QueryMessages()
{
    HITRACE_METER(BT_TRACE_TAG);
    if (!smsDataShareHelper_) {
        return nullptr;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns = {"msg_id",
        "is_sender",
        "receiver_number",
        "sender_number",
        "start_time",
        "end_time",
        "msg_title",
        "msg_content",
        "is_read",
        "session_id",
        "msg_state"};
    Uri uri(SMS_MMS_INFO_URI_STR);
    auto resultSet = smsDataShareHelper_->Query(uri, predicates, columns);
    if (!resultSet) {
        HILOGE("Query data failed.");
        return nullptr;
    }
    return resultSet;
}

void MapMseContentObserver::SendEvent(const MapEvent &event)
{
    auto sharedMnsClient = mnsClient_.lock();
    if (sharedMnsClient != nullptr && sharedMnsClient->IsConnect()) {
        sharedMnsClient->SendEvent(masInstanceId_, event.Encode());
    } else {
        HILOGE("masInstanceId_ is %{public}d, mnsClient_ is not connect", masInstanceId_);
    }
}

void MapMseContentObserver::HandleListChangeForNewMessage(std::map<int64_t, Message> &newMsgList, const Message &msg)
{
    HITRACE_METER(BT_TRACE_TAG);
    newMsgList[msg.handle] = msg;
    if (!msg.sent) {  // recieve new messages
        SendEvent(MapEvent(msg.handle, EVENT_TYPE_NEW, MAP_EVENT_FOLDER_INBOX));
        return;
    }
    // send messages
    if (msg.handle == smsMessageSentHandle_) {  // bluetooth send message success, send MessageShift event
        if (msg.msgState == 0) {                // success
            SendEvent(MapEvent(msg.handle, EVENT_TYPE_SHIFT, MAP_EVENT_FOLDER_SENT, MAP_EVENT_FOLDER_OUTBOX));
        }
    } else if (msg.msgState == MESSAGE_STATE_IS_DRAFT) {
        SendEvent(MapEvent(msg.handle, EVENT_TYPE_NEW, MAP_EVENT_FOLDER_DRAFT));
    } else {  // other domain send message, first send NewMessage event, if success send MessageShift event
        SendEvent(MapEvent(msg.handle, EVENT_TYPE_NEW, MAP_EVENT_FOLDER_OUTBOX));
        if (msg.msgState == 0) {
            SendEvent(MapEvent(msg.handle, EVENT_TYPE_SHIFT, MAP_EVENT_FOLDER_SENT, MAP_EVENT_FOLDER_OUTBOX));
        }
    }
}

void MapMseContentObserver::HandleListChangeForExistMessage(std::map<int64_t, Message> &newMsgList, const Message &msg)
{
    HITRACE_METER(BT_TRACE_TAG);
    newMsgList[msg.handle] = msg;
    auto existMsg = smsMessageCollection_[msg.handle];

    if ((msg.msgState != existMsg.msgState) && (msg.msgState == 0)) {  // mse send message success
        SendEvent(MapEvent(msg.handle, EVENT_TYPE_SHIFT, MAP_EVENT_FOLDER_SENT, MAP_EVENT_FOLDER_OUTBOX));
    }
    if (msg.readFlag != existMsg.readFlag) {  // message has been read
        SendEvent(MapEvent(msg.handle, EVENT_TYPE_READ_STATUS, MAP_EVENT_FOLDER_INBOX));
    }
}

void MapMseContentObserver::HandleListChangeForDeleteMessage(std::map<int64_t, Message> &newMsgList)
{
    HILOGI("newMsgList.size()=%{public}d smsMessageCollection_.size()=%{public}d",
        newMsgList.size(), smsMessageCollection_.size());
    if (newMsgList.size() == smsMessageCollection_.size()) {
        return;
    }
    // some message has been deleted when smsMessageCollection_ message not in currMessageList
    for (auto &msg : smsMessageCollection_) {
        if (!newMsgList.count(msg.first)) {
            HILOGI("msgId=%{public}ld has been delete", msg.first);
            SendEvent(MapEvent(msg.first, EVENT_TYPE_DELETE, GetMessageEventFolder(msg.second)));
            break;
        }
    }
}

// 发送成功或者失败，都会存储到数据库中，根据数据库查询结果，可以判断是发送成功还是失败
int64_t MapMseContentObserver::GetSendMessageHandle()
{
    auto resultSet = QueryMessages();
    if (!resultSet) {
        return -1;
    }
    int32_t opRet = resultSet->GoToFirstRow();
    if (opRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow error. error code is %{public}d", opRet);
        return -1;
    }
    std::map<int64_t, Message> currMessageList;
    while (opRet == DataShare::E_OK) {
        int64_t handle = GetLongFromResultSet(resultSet, "msg_id");
        bool sent = GetLongFromResultSet(resultSet, "is_sender") == 0 ? true : false;
        if (!smsMessageCollection_.count(handle)) {
            if (sent) {
                smsMessageSentHandle_ = handle;
                return handle;
            }
        }
        opRet = resultSet->GoToNextRow();
    }
    return -1;
}

std::string MapMseContentObserver::GetMessageEventFolder(const Message &msg)
{
    if (!msg.sent) {
        return MAP_EVENT_FOLDER_INBOX;
    }
    if (msg.msgState == MESSAGE_STATE_IS_DRAFT) {
        return MAP_EVENT_FOLDER_DRAFT;
    }
    return MAP_EVENT_FOLDER_SENT;
}

std::string MapMseContentObserver::MapEvent::Encode() const
{
    std::string xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>";
    xml.append("<MAP-event-report version=\"");
    std::string version = "1.0";
    if (reportVersion_ == MAP_EVENT_REPORT_V12) {
        version = "1.2";
    } else if (reportVersion_ == MAP_EVENT_REPORT_V11) {
        version = "1.1";
    }
    xml.append(version).append("\">");
    xml.append("<event type=\"").append(type_).append("\"");
    auto handleStr = MseMessageElement::GenerateStringHandle(handle_, TYPE_SMS_GSM);
    xml.append(" handle=\"").append(handleStr).append("\"");
    xml.append(" folder=\"").append(folder_).append("\"");
    if (!oldFolder_.empty()) {
        xml.append(" old_folder=\"").append(oldFolder_).append("\"");
    }
    xml.append(" msg_type=\"").append(MseMessageElement::GetMessageTypeStr(TYPE_SMS_GSM)).append("\"");
    xml.append(" />");
    xml.append("</MAP-event-report>");
    return std::move(xml);
}
}  // namespace bluetooth
}  // namespace OHOS