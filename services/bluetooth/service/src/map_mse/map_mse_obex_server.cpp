/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_map_mse_obex_server"
#endif

#include "map_mse_obex_server.h"

#include <sstream>
#include <chrono>
#include <iomanip>

#include "log.h"
#include "map_mse_masinstance.h"
#include "map_mse_service_impl.h"
#include "securec.h"

namespace {
std::string GetCurrentTimeToString()
{
    struct tm time;
    if (memset_s(&time, sizeof(struct tm), 0x00, sizeof(tm)) != EOK) {
        HILOGE("memset fail.");
        return "";
    }

    auto now = std::chrono::system_clock::now();
    std::time_t timetNow = std::chrono::system_clock::to_time_t(now);
    localtime_r(&timetNow, &time);

    std::stringstream ss;
    ss << std::put_time(&time, "%Y%m%dT%H%M%S");
    return ss.str();
}
}  // namespace

namespace OHOS {
namespace bluetooth {
const std::vector<uint8_t> MAP_TARGET = {
    0xBB, 0x58, 0x2B, 0x40, 0x42, 0x0C, 0x11, 0xDB, 0xB0, 0xDE, 0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66};
const std::string TYPE_GET_FOLDER_LISTING = "x-obex/folder-listing";
const std::string TYPE_GET_MESSAGE_LISTING = "x-bt/MAP-msg-listing";
const std::string TYPE_GET_CONVO_LISTING = "x-bt/MAP-convo-listing";
const std::string TYPE_MESSAGE = "x-bt/message";
const std::string TYPE_SET_MESSAGE_STATUS = "x-bt/messageStatus";
const std::string TYPE_SET_NOTIFICATION_REGISTRATION = "x-bt/MAP-NotificationRegistration";
const std::string TYPE_MESSAGE_UPDATE = "x-bt/MAP-messageUpdate";
const std::string TYPE_GET_MAS_INSTANCE_INFORMATION = "x-bt/MASInstanceInformation";
const std::string TYPE_SET_OWNER_STATUS = "x-bt/participant";
const std::string TYPE_SET_NOTIFICATION_FILTER = "x-bt/MAP-notification-filter";
MapMseObexServer::MapMseObexServer(
    std::weak_ptr<MapMseMasInstance> masInstance, std::weak_ptr<MapMseMnsClient> mnsClient,
    MapMseServiceImpl *mapMseServiceImpl) : masInstance_(masInstance), mapMseServiceImpl_(mapMseServiceImpl)
{
    content_ = std::make_shared<MapMseContent>();
    contentObserver_ = std::make_shared<MapMseContentObserver>(mnsClient);
    CreateFolder();
}

void MapMseObexServer::UpdateMnsClient(std::weak_ptr<MapMseMnsClient> mnsClient)
{
    if (mnsClient.lock() == nullptr) {
        HILOGE("mnsClient is nullptr");
        return;
    }
    contentObserver_->UpdateMnsClient(mnsClient);
}

int MapMseObexServer::OnConnect(ObexHeader &request, ObexHeader &reply)
{
    auto uuid = request.GetHeader(TARGET);
    if (uuid.empty()) {
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }

    if (uuid.size() != UUID_LENGTH) {
        HILOGE("Wrong UUID length.");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    for (int i = 0; i < UUID_LENGTH; i++) {
        if (uuid[i] != MAP_TARGET[i]) {
            HILOGE("Wrong UUID.");
            return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
        }
    }
    reply.SetHeader(WHO, uuid);

    int32_t threadedMailKey = 0;
    auto threadedMailKeyVec = request.GetHeader(THREADED_MAIL_HEADER_ID);
    if (!threadedMailKeyVec.empty() && threadedMailKeyVec.size() >= sizeof(threadedMailKey)) {
        if (memcpy_s(&threadedMailKey, sizeof(threadedMailKey), threadedMailKeyVec.data(), threadedMailKeyVec.size()) !=
            EOK) {
            threadedMailKey = 0;
        }
    }

    auto remote = request.GetHeader(WHO);
    if (!remote.empty()) {
        reply.SetHeader(TARGET, remote);
    }

    if (threadedMailKey == THREAD_MAIL_KEY) {
        threadIdSupport_ = true;
        content_->SetThreadIdSupport(threadIdSupport_);
        std::vector<uint8_t> data(sizeof(threadedMailKey), 0);
        if (memcpy_s(data.data(), data.size(), &threadedMailKey, sizeof(threadedMailKey)) == EOK) {
            reply.SetHeader(THREADED_MAIL_HEADER_ID, data);
        }
    }

    if ((remoteFeatureMask_ & MAP_FEATURE_MESSAGE_FORMAT_V11_BIT) == MAP_FEATURE_MESSAGE_FORMAT_V11_BIT) {
        messageVersion_ = MAP_V11_STR;
    }
    if ((remoteFeatureMask_ & MAP_FEATURE_MESSAGE_LISTING_FORMAT_V11_BIT) ==
        MAP_FEATURE_MESSAGE_LISTING_FORMAT_V11_BIT) {
        threadIdSupport_ = true;
        content_->SetThreadIdSupport(threadIdSupport_);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::OnDisconnect(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    std::string type = request.GetType();
    HILOGI("type %{public}s", type.c_str());
    if (type.empty()) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }

    int code = ResponseCodes::OBEX_HTTP_OK;
    appParams_ = std::make_shared<MapMseAppParams>(request.GetHeader(APPLICATION_PARAMETER));
    if (type == TYPE_GET_FOLDER_LISTING) {
        code = SendFolderListingRsp(reply, output, sendBodyHeader);
    } else if (type == TYPE_GET_MESSAGE_LISTING) {
        code = SendMessageListingRsp(request, reply, output, sendBodyHeader);
    } else if (type == TYPE_GET_CONVO_LISTING) {
        code = SendConvoListingRsp(reply, sendBodyHeader);
    } else if (type == TYPE_GET_MAS_INSTANCE_INFORMATION) {
        code = SendMasInstanceInfoRsp(output);
    } else if (type == TYPE_MESSAGE) {
        code = SendMessageRsp(request, output);
    } else {
        code = ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    appParams_ = nullptr;
    return code;
}

int MapMseObexServer::OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished)
{
    std::string type = request.GetType();
    std::string name = request.GetName();
    HILOGI("type %{public}s", type.c_str());
    if (type.empty()) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }

    int code = ResponseCodes::OBEX_HTTP_OK;
    appParams_ = std::make_shared<MapMseAppParams>(request.GetHeader(APPLICATION_PARAMETER));
    if (type == TYPE_SET_NOTIFICATION_REGISTRATION) {
        auto sharedMasInstance = masInstance_.lock();
        if (sharedMasInstance != nullptr) {
            code = contentObserver_->SetNotificationRegistration(sharedMasInstance->GetMasId(), appParams_);
        }
    } else if (type == TYPE_SET_MESSAGE_STATUS) {
        code = contentObserver_->SetMessageStatus(name, appParams_);
    } else if (type == TYPE_MESSAGE) {
        code = PushMessage(name, reply, input, requestFinished);
    } else {
        code = ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }

    appParams_ = nullptr;
    return code;
}

int MapMseObexServer::OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create)
{
    auto folderName = request.GetName();
    HILOGI("requestName(%{public}s), currentName(%{public}s), backup(%{public}d), create(%{public}d)",
        folderName.c_str(), currentFolder_->GetName().c_str(), backup, create);
    if (backup) {
        if (currentFolder_->GetParent() != nullptr) {
            currentFolder_ = currentFolder_->GetParent();
        } else {
            HILOGE("currentFolder parent is null");
            return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
        }
    }

    if (folderName.empty()) {
        if (!backup) {
            currentFolder_ = currentFolder_->GetRoot();
            if (currentFolder_ != nullptr) {
                HILOGI("currentFolder_ name %{public}s.!", currentFolder_->GetName().c_str());
            }
        }
    } else {
        auto folder = currentFolder_->GetSubFolder(folderName);
        if (folder != nullptr) {
            HILOGI("folder name %{public}s.!", folder->GetName().c_str());
            currentFolder_ = folder;
        } else {
            return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
        }
    }

    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::OnDelete(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::OnAbort(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

void MapMseObexServer::OnClose(bool isThreadStart)
{
    HILOGI("isThreadStart is %{public}d", isThreadStart);
    auto sharedMasInstance = masInstance_.lock();
    if (sharedMasInstance != nullptr) {
        sharedMasInstance->OnClose();
    }
}

void MapMseObexServer::SetConnectionId(int connectionId)
{
    connectionId_ = connectionId;
}

int MapMseObexServer::GetConnectionId()
{
    return connectionId_;
}

void MapMseObexServer::CreateFolder()
{
    currentFolder_ = std::make_shared<MapMseFolder>("root", nullptr);
    auto telecom = currentFolder_->AddFolder("telecom");
    auto msg = telecom->AddFolder("msg");
    InitDefaultFolders(msg);
    // smsMms check enable
    InitSmsMmsFolders(msg);
}

void MapMseObexServer::InitDefaultFolders(std::shared_ptr<MapMseFolder> &root)
{
    root->AddFolder(MAP_MSE_FOLDER_NAME_INBOX);
    root->AddFolder(MAP_MSE_FOLDER_NAME_SENT);
    root->AddFolder(MAP_MSE_FOLDER_NAME_OUTBOX);
    root->AddFolder(MAP_MSE_FOLDER_NAME_DELETED);
}

void MapMseObexServer::InitSmsMmsFolders(std::shared_ptr<MapMseFolder> &root)
{
    root->AddSmsMmsFolder(MAP_MSE_FOLDER_NAME_INBOX);
    root->AddSmsMmsFolder(MAP_MSE_FOLDER_NAME_SENT);
    root->AddSmsMmsFolder(MAP_MSE_FOLDER_NAME_OUTBOX);
    root->AddSmsMmsFolder(MAP_MSE_FOLDER_NAME_DELETED);
    root->AddSmsMmsFolder(MAP_MSE_FOLDER_NAME_DRAFT);
}

int MapMseObexServer::SendFolderListingRsp(ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    int32_t maxListCount = appParams_->maxListCount_.has_value() ? appParams_->maxListCount_.value() : MAX_LIST_COUNT;
    HILOGI("maxListCount is %{public}d", maxListCount);
    int32_t listStartOffset = appParams_->listStartOffSet_.has_value() ? appParams_->listStartOffSet_.value() : 0;

    std::unique_ptr<MapMseAppParams> outAppParams = std::make_unique<MapMseAppParams>();
    if (maxListCount == 0) {
        outAppParams->folderListingSize_ = currentFolder_->GetSubFolderSize();
        sendBodyHeader = false;
    } else {
        std::string encodeData = currentFolder_->GetEncodeData(listStartOffset, maxListCount);
        output.insert(output.end(), encodeData.begin(), encodeData.end());
    }
    std::vector<uint8_t> outAppParamsRaw;
    outAppParams->GetEncodeData(outAppParamsRaw);
    reply.SetHeader(APPLICATION_PARAMETER, outAppParamsRaw);
    return ResponseCodes::OBEX_HTTP_OK;
}

std::shared_ptr<MapMseFolder> MapMseObexServer::ParseMessageListingFolder(ObexHeader &request)
{
    std::shared_ptr<MapMseFolder> folder = nullptr;
    if (appParams_->filterMessageHandle_.has_value()) {
        if (currentFolder_ != nullptr) {
            folder = currentFolder_->GetRoot();
            if (folder != nullptr) {
                folder->SetIgnore(true);
            }
        }
    } else {
        std::string name = request.GetName();
        folder = GetFolderElementFromName(name);
    }
    if (folder) {
        HILOGI("foldername %{public}s ", folder->GetName().c_str());
    }
    return folder;
}

std::shared_ptr<MapMseFolder> MapMseObexServer::GetFolderElementFromName(std::string folderName)
{
    if (currentFolder_ == nullptr) {
        return nullptr;
    }
    if (folderName.empty()) {
        return currentFolder_;
    }

    return currentFolder_->GetSubFolder(folderName);
}

int MapMseObexServer::SendMessageListingRsp(
    ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    std::shared_ptr<MapMseFolder> folder = ParseMessageListingFolder(request);
    CHECK_AND_RETURN_LOG_RET(folder, ResponseCodes::OBEX_HTTP_BAD_REQUEST, "folder is null");
    if (!appParams_->maxListCount_.has_value()) {
        appParams_->maxListCount_ = MAX_LIST_COUNT;
    }
    if (!appParams_->listStartOffSet_.has_value()) {
        appParams_->listStartOffSet_ = 0;
    }

    std::shared_ptr<MapMseAppParams> outAppParams = std::make_shared<MapMseAppParams>();
    bool hasUnread = false;
    int32_t listSize = 0;
    if (appParams_->maxListCount_.value() != 0) {
        if (!appParams_->parameterMask_.has_value() || appParams_->parameterMask_.value() == 0) {
            appParams_->parameterMask_ = PARAMETER_MASK_ALL_ENABLED;
        }
        std::string version = MAP_V10_STR;
        if ((remoteFeatureMask_ & MAP_FEATURE_MESSAGE_FORMAT_V11_BIT) > 0) {
            version = MAP_V11_STR;
        }
        std::string messageListingBody =
            content_->GetMessageListing(folder->GetName(), appParams_, version, listSize, hasUnread);
        output.insert(output.end(), messageListingBody.begin(), messageListingBody.end());
    } else {
        listSize = content_->GetMessageListingSize(folder->GetName(), appParams_, hasUnread);
        sendBodyHeader = false;
    }
    folder->SetIgnore(false);
    outAppParams->messagesListingSize_ = listSize;
    outAppParams->newMessage_ = hasUnread ? 1 : 0;

    if ((remoteFeatureMask_ & MAP_FEATURE_DATABASE_INDENTIFIER_BIT) == MAP_FEATURE_DATABASE_INDENTIFIER_BIT) {
        outAppParams->databaseIdentifierHigh_ = 0;
        if (mapMseServiceImpl_ != nullptr && mapMseServiceImpl_->getDbIdentifierFunc_ != nullptr) {
            outAppParams->databaseIdentifierLow_ = mapMseServiceImpl_->getDbIdentifierFunc_();
        }
    }
    if ((remoteFeatureMask_ & MAP_FEATURE_FOLDER_VERSION_COUNTER_BIT) == MAP_FEATURE_FOLDER_VERSION_COUNTER_BIT) {
        outAppParams->folderVerCounterHigh_ = 0;
        auto sharedMasInstance = masInstance_.lock();
        if (sharedMasInstance != nullptr) {
            outAppParams->folderVerCounterLow_ = sharedMasInstance->GetFolderVersionCounter();
        }
    }
    outAppParams->mseTime_ = GetCurrentTimeToString();
    std::vector<uint8_t> outAppParamsRaw{};
    outAppParams->GetEncodeData(outAppParamsRaw);
    reply.SetHeader(APPLICATION_PARAMETER, outAppParamsRaw);

    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::SendConvoListingRsp(
    ObexHeader &reply, bool &sendBodyHeader)
{
    if (!appParams_->maxListCount_.has_value()) {
        appParams_->maxListCount_ = MAX_LIST_COUNT;
    }
    if (!appParams_->listStartOffSet_.has_value()) {
        appParams_->listStartOffSet_ = 0;
    }
    std::shared_ptr<MapMseAppParams> outAppParams = std::make_shared<MapMseAppParams>();
    if (appParams_->maxListCount_.value() != 0) {
    } else {
        sendBodyHeader = false;
    }

    if (mapMseServiceImpl_ != nullptr && mapMseServiceImpl_->getDbIdentifierFunc_ != nullptr) {
        outAppParams->SetDatabaseIdentifier(0, mapMseServiceImpl_->getDbIdentifierFunc_());
    }
    outAppParams->mseTime_ = GetCurrentTimeToString();
    std::vector<uint8_t> outData{};
    outAppParams->GetEncodeData(outData);
    reply.SetHeader(APPLICATION_PARAMETER, outData);

    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::SendMasInstanceInfoRsp(std::vector<uint8_t> &output)
{
    auto sharedMasInstance = masInstance_.lock();
    if (!appParams_->masInstanceId_.has_value() ||
        sharedMasInstance == nullptr || sharedMasInstance->GetMasId() != appParams_->masInstanceId_.value()) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    // current only sms_mms
    std::string outStr = "SMS/MMS";
    output.insert(output.end(), outStr.begin(), outStr.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::SendMessageRsp(ObexHeader &request, std::vector<uint8_t> &output)
{
    if (appParams_->fractionRequest_.has_value() && appParams_->fractionRequest_.value() == 1) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    std::string handle = request.GetName();
    if (handle.empty()) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    std::string result = content_->GetMessage(handle, appParams_, messageVersion_);
    output.insert(output.end(), result.begin(), result.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

int MapMseObexServer::PushMessage(const std::string &name,
    ObexHeader &reply, std::vector<uint8_t> &input, bool requestFinished)
{
    if (!appParams_->charSet_.has_value()) {
        HILOGE("cannot parse content");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }
    auto folderElement = GetFolderElementFromName(name);
    if (!folderElement) {
        HILOGE("folder element is null");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }
    std::string folderName = folderElement->GetName();
    if (folderName != MAP_MSE_FOLDER_NAME_OUTBOX && folderName != MAP_MSE_FOLDER_NAME_DRAFT) {
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }

    inputMessage_.insert(inputMessage_.end(), input.begin(), input.end());
    HILOGI("pushMessage size is %{public}d requestFinished %{public}d", inputMessage_.size(), requestFinished);
    if (!requestFinished) {
        return ResponseCodes::OBEX_HTTP_CONTINUE;
    }
    int32_t ret = contentObserver_->SendNewMessage(messageVersion_, folderElement, appParams_, reply, inputMessage_);
    inputMessage_.clear();
    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS