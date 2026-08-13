/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_opp_file_utils"
#endif

#include "log.h"

#include "cJSON.h"
#include "extension_manager_client.h"
#include <filesystem>
#include "opp_file_utils.h"
#include <map>
#include "system_ability_definition.h"
#include "bluetooth_common_event_helper.h"

namespace OHOS {
namespace bluetooth {
const std::map<std::string, std::string> suffixToMimeTypeMap_ = {
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".arc", "application/x-freearc"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/x-ms-bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".csh", "application/x-csh"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot", "application/vnd.ms-fontobject"},
    {".epub", "application/epub+zip"},
    {".gz", "application/gzip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".ico", "image/vnd.microsoft.icon"},
    {".ics", "text/calendar"},
    {".jar", "application/java-archive"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".jsonld", "application/ld+json"},
    {".mid", "audio/midi"},
    {".midi", "audio/midi"},
    {".mjs", "text/javascript"},
    {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},
    {".mpeg", "video/mpeg"},
    {".mpkg", "application/vnd.apple.installer+xml"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".oga", "audio/ogg"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".opus", "audio/opus"},
    {".otf", "font/otf"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".php", "application/php"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar", "application/vnd.rar"},
    {".rtf", "application/rtf"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".swf", "application/x-shockwave-flash"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ts", "video/mp2t"},
    {".ttf", "font/ttf"},
    {".txt", "text/plain"},
    {".vcf", "text/x-vcard"},
    {".vsd", "application/vnd.visio"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".xul", "application/vnd.mozilla.xul+xml"},
    {".zip", "application/zip"},
    {".7z", "application/x-7z-compressed"}
};

std::string GetFileNameFromFilePath(std::string filePath, bool isNeedTimeStamp)
{
    std::filesystem::path fs_path(filePath);
    std::string fileName = fs_path.filename().string();
    std::string extension = GetFileExtension(fileName);
    if (extension.empty()) {
        HILOGI("error file path without type");
        return fileName;
    }

    std::string timeStr = GetCurrentTime(isNeedTimeStamp);
    std::string fileNewName = fs_path.stem().string() + timeStr + "." + extension;
    if (extension == FILE_ENCRYPTED_SUFFFIX) {
        extension = GetFileExtension(fs_path.stem().string());
        extension = extension.empty() ? extension : ("." + extension);
        std::string encryptedName = GetEncryptedFileName(fs_path.stem().string());
        fileNewName = encryptedName + timeStr + extension + "." + FILE_ENCRYPTED_SUFFFIX;
    }
    return GetLimitedLengthFileName(fileNewName, MAX_FILE_NAME_LENGTH);
}

std::string GetCurrentTime(bool isNeedTimeStamp)
{
    if (!isNeedTimeStamp) {
        return "";
    }
    time_t now = time(nullptr);
    char currentTime[TIME_STMP_STR_LEN];
    struct tm *localTime = localtime(&now);
    if (localTime == nullptr) {
        return "";
    }
    strftime(currentTime, sizeof(currentTime), "%Y%m%d_%H%M%S", localTime);
    std::string result = "";
    result = "_" + result.append(currentTime);
    return result;
}

std::string GetEncryptedFileName(const std::string filePath)
{
    std::filesystem::path fs_path(filePath);
    std::string fileName = fs_path.filename().string();
    std::string newExtension = GetFileExtension(fileName);
    if (newExtension.empty()) {
        HILOGI("error file path without type");
        return fileName;
    }
    return fs_path.stem().string();
}

std::string GetLimitedLengthFileName(std::string fileName, size_t maxNameLen)
{
    if (fileName.length() <= maxNameLen) {
        return fileName;
    }
    std::string extension = GetFileExtension(fileName);
    const size_t dotLen = 1;
    if (extension.length() + dotLen >= maxNameLen) { // 如果后缀长度大于最大长度，则只保留后缀
        return "." + extension;
    }
    // 总长度大于最大长度，后缀小于最大长度，截断前缀的后面部分使总长度等于最大长度，保留完整后缀
    return fileName.substr(0, maxNameLen - dotLen - extension.length()) + "." + extension;
}

std::string GetFileExtension(std::string filePath)
{
    std::filesystem::path fs_path(filePath);
    std::string fileName = fs_path.filename().string();

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == fileName.length() - 1) {
        HILOGI("error file path without type");
        return "";
    }
    return fileName.substr(dotPos + 1);
}

BtTransactionStatisticsType GetBtTransactionStatisticType(const int transactionType)
{
    switch(transactionType) {
        case 1: return TRANSACTION_TYPE_OPP_SEND;
        case 2: return TRANSACTION_TYPE_OPP_RECEIVE;
        default: return TRANSACTION_TYPE_NA;
    }
}

BtTransactionStatisticsResult GetBtTransactionStatisticResult(const int result)
{
    switch(result) {
        case 1: return TRANSACTION_RESULT_TOTAL;
        case 2: return TRANSACTION_RESULT_SUCCESS;
        case 3: return TRANSACTION_RESULT_FAIL;
        default: return TRANSACTION_RESULT_NA;
    }
}

BtTransactionStatisticsSceneCode GetBtTransactionStatisticSceneCode(const int sceneCode)
{
    switch(sceneCode) {
        case 1: return TRANSACTION_SCENECODE_1;
        case 2: return TRANSACTION_SCENECODE_2;
        case 3: return TRANSACTION_SCENECODE_3;
        case 4: return TRANSACTION_SCENECODE_4;
        case 5: return TRANSACTION_SCENECODE_5;
        case 6: return TRANSACTION_SCENECODE_6;
        case 7: return TRANSACTION_SCENECODE_7;
        case 8: return TRANSACTION_SCENECODE_8;
        case 9: return TRANSACTION_SCENECODE_9;
        case 10: return TRANSACTION_SCENECODE_10;
        case 11: return TRANSACTION_SCENECODE_11;
        case 12: return TRANSACTION_SCENECODE_12;
        case 13: return TRANSACTION_SCENECODE_13;
        case 14: return TRANSACTION_SCENECODE_14;
        case 15: return TRANSACTION_SCENECODE_15;
        case 16: return TRANSACTION_SCENECODE_16;
        default: return TRANSACTION_SCENECODE_NA;
    }
}

std::string GetMimeTypeFromFilePath(std::string filePath)
{
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == filePath.length() - 1) {
        HILOGI("error file path without type");
        return "application/octet-stream";
    }
    std::string extension = filePath.substr(dotPos);
    HILOGI("extension is %{public}s", extension.c_str());
    auto it = suffixToMimeTypeMap_.find(extension);
    if (it != suffixToMimeTypeMap_.end()) {
        HILOGI("end type is %{public}s", it->second.c_str());
        return it->second;
    }
    return "application/octet-stream";
}

void OppAbilityManagerUtils::StartUIAbilityByCall(AAFwk::Want &abilityWant, AAFwk::Want &eventWant)
{
    sptr<UIAbilityConnection> connection = new (std::nothrow) UIAbilityConnection(eventWant);
    if (connection == nullptr) {
        HILOGE("connect StartUIAbilityByCall fail");
        return;
    }
    if (AAFwk::AbilityManagerClient::GetInstance() != nullptr) {
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(abilityWant, connection);
        HILOGI("StartUIAbilityByCall result = %{public}d", ret);
        return;
    }
}

void OppAbilityManagerUtils::WakeUpOppSendUIAbility(const std::string &fileName)
{
    HILOGI("WakeUpOpp BluetoothSendUIAbility");
    AAFwk::Want abilityWant;
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothSendUIAbility";
    abilityWant.SetElementName(bundleName, abilityName);
    abilityWant.SetParam("fileName", fileName);

    AAFwk::Want eventWant;
    OppAbilityManagerUtils::StartUIAbilityByCall(abilityWant, eventWant);
}

std::function<void(const std::string &action)> oppContinuousTaskFunc_ = nullptr;
std::function<void(const std::string &addr)> aclDisConnEventFunc_ = nullptr;


void OppAbilityManagerUtils::SetOppContinuousTaskFunc(std::function<void(const std::string &action)> oppTaskFunc)
{
    oppContinuousTaskFunc_ = oppTaskFunc;
}

void OppAbilityManagerUtils::SetAclDisConnEventFunc(std::function<void(const std::string &addr)> aclDisConnEventFunc)
{
    aclDisConnEventFunc_ = aclDisConnEventFunc;
}

void UIAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOGI("UIAbilityConnection OnAbilityConnectDone");
    if (eventWant_.GetAction().empty()) {
        HILOGI("eventWant_.GetAction() is empty");
        return;
    }
    if (oppContinuousTaskFunc_ != nullptr) {
        HILOGI("oppContinuousTaskFunc_ begin");
        oppContinuousTaskFunc_("connect");
    }
    AAFwk::Want eventWant = eventWant_;
    DoInLowPriorityThread([eventWant]() {
        HILOGI("eventAction: %{public}s", eventWant.GetAction().c_str());
        OHOS::EventFwk::CommonEventData data;
        data.SetWant(eventWant);
        OHOS::EventFwk::CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(false);
        publishInfo.SetSticky(false);
        publishInfo.SetSubscriberPermissions(MAMAGE_PERMISSIONS);
        bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
        HILOGI("publishResult = %{public}d", publishResult);
        OppAbilityManagerUtils::SubscribOppReceiveEvent();
        }, DELAY_TIME_MS);
}
 
void UIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    HILOGI("UIAbilityConnection OnAbilityDisconnectDone");
}

std::shared_ptr<OHOS::bluetooth::OppEventSubscriber> oppEventSubscriber_ = nullptr;
void OppAbilityManagerUtils::SubscribOppReceiveEvent()
{
    if (oppEventSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EVENT_NOTIFICATION_BT_TAP_ACCEPT);
        matchingSkills.AddEvent(EVENT_NOTIFICATION_BT_TAP_REJECT);
        matchingSkills.AddEvent(EVENT_NOTIFICATION_BT_TAP_REMOVE);
        matchingSkills.AddEvent(EVENT_NOTIFICATION_BT_TAP_FINISH_NOTIFICATION);
        matchingSkills.AddEvent(EVENT_NOTIFICATION_BT_TAP_FINISH_REMOVE);
        matchingSkills.AddEvent(EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED_REALMAC);

        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        uint32_t coreEventPriority = 1;
        subscribeInfo.SetPriority(coreEventPriority);
        subscribeInfo.SetPermission(MANAGE_BLUETOOTH);
        oppEventSubscriber_ = std::make_shared<OppEventSubscriber>(subscribeInfo);
        bool result =
            EventFwk::CommonEventManager::SubscribeCommonEvent(oppEventSubscriber_);
        HILOGI("Subscrib OppReceiveEvents, ret: %{public}d", result);
    }
}

std::shared_ptr<OHOS::bluetooth::OppEventSubscriber> transactionChrSubscriber_ = nullptr;
void OppAbilityManagerUtils::SubscribTransactionChrEvent()
{
    if (transactionChrSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EVENT_BLUETOOTH_TRANSACTION_CHR_REPORT);
 
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(MANAGE_BLUETOOTH);
        uint32_t coreEventPriority = 1;
        subscribeInfo.SetPriority(coreEventPriority);
        transactionChrSubscriber_ = std::make_shared<OppEventSubscriber>(subscribeInfo);
        bool result =
            EventFwk::CommonEventManager::SubscribeCommonEvent(transactionChrSubscriber_);
        HILOGI("Subscrib OppReceiveEvents, ret: %{public}d", result);
    }
}

void OppEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    
    auto want = data.GetWant();
    std::string action = want.GetAction();
    HILOGI("OnReceiveEvent. action is %{public}s", action.c_str());

    AAFwk::Want abilityWant;
    std::string bundleName = "com.ohos.bluetooth";
    std::string abilityName = "BluetoothReceiveUIAbility";
    abilityWant.SetElementName(bundleName, abilityName);

    AAFwk::Want eventWant;
    std::string eventAction = "";
    if (action == EVENT_NOTIFICATION_BT_TAP_ACCEPT) {
        eventAction = EVENT_BLUETOOTH_OPP_TAP_ACCEPT;
        eventWant.SetParam("acceptType", OPP_TRANSFER_USER_ACCEPT);
    } else if (action == EVENT_NOTIFICATION_BT_TAP_REJECT) {
        eventAction = EVENT_BLUETOOTH_OPP_TAP_REJECT;
    } else if (action == EVENT_NOTIFICATION_BT_TAP_REMOVE) {
        eventAction = EVENT_BLUETOOTH_OPP_TAP_REMOVE;
    } else if (action == EVENT_NOTIFICATION_BT_TAP_FINISH_NOTIFICATION) {
        eventAction = EVENT_BLUETOOTH_OPP_TAP_FINISH_NOTIFICATION;
    } else if (action == EVENT_NOTIFICATION_BT_TAP_FINISH_REMOVE) {
        eventAction = EVENT_BLUETOOTH_OPP_TAP_FINISH_REMOVE;
    } else if (action == EVENT_BLUETOOTH_TRANSACTION_CHR_REPORT) {
        int resultCount = want.GetIntParam("resultCount", DEFAULT_VAL);
        int sceneCodeCount = want.GetIntParam("sceneCodeCount", DEFAULT_VAL);
        BtChrTransactionManager::GetInstance().
            WriteTransactionStatictics(GetBtTransactionStatisticType(want.GetIntParam("transactionType", DEFAULT_VAL)),
            GetBtTransactionStatisticResult(want.GetIntParam("result", DEFAULT_VAL)), resultCount,
            GetBtTransactionStatisticSceneCode(want.GetIntParam("sceneCode", DEFAULT_VAL)), sceneCodeCount);
        return;
    } else if (action == EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED_REALMAC && aclDisConnEventFunc_ != nullptr) {
        aclDisConnEventFunc_(want.GetStringParam("deviceAddr"));
        return;
    } 
    eventWant.SetAction(eventAction);
    OppAbilityManagerUtils::StartUIAbilityByCall(abilityWant, eventWant);
}

void OppAbilityManagerUtils::StartUIExtentionAbility(std::map<std::string, std::string> &params)
{
    AAFwk::Want want;
    std::string bundleName = "com.ohos.sceneboard";
    std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    params.emplace("ability.want.params.uiExtensionType", "sysDialog/common");
    cJSON* root = cJSON_CreateObject();
    for (auto it = params.begin(); it != params.end(); ++it) {
        cJSON_AddStringToObject(root, it->first.c_str(), it->second.c_str());
    }
    // 拉起BluetoothReceiveServiceUIAbility时，不抢占页面焦点
    cJSON_AddNumberToObject(root, "focusState", 0);
    char* cmdData = cJSON_PrintUnformatted(root);
    if (cmdData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(root);
        return;
    }
    sptr<UIExtensionAbilityConnection> connection(
        new (std::nothrow) UIExtensionAbilityConnection(cmdData, "com.ohos.bluetooth",
            "BluetoothReceiveServiceUIAbility"));
    cJSON_Delete(root);
    cJSON_free(cmdData);
    if (connection == nullptr) {
        HILOGE("connect UIExtensionAbilityConnection fail");
        return;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret =
        AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want, connection, nullptr, -1);
    HILOGI("connect service extension ability result = %{public}d", ret);
    IPCSkeleton::SetCallingIdentity(identity);
    if (ret != ERR_OK) {
        HILOGE("ret isn't ERR_OK");
        return;
    }
}

void UIExtensionAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOGI("on ability connected");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(SIGNAL_NUM);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(bundleName_));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(abilityName_));
    data.WriteString16(u"parameters");
    data.WriteString16(Str8ToStr16(commandStr_));

    int32_t errCode = remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    HILOGI("AbilityConnectionWrapperProxy::OnAbilityConnectDone result %{public}d", errCode);
}

void UIExtensionAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    HILOGI("on ability disconnected");
}
}  // namespace bluetooth
}  // namespace OHOS
