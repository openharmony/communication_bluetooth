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
#define LOG_TAG "bt_service_pbap_obex_server"
#endif

#include "common_util.h"
#include "pbap_pse_obex_server.h"
#include "pbap_pse_service_impl.h"
#include "log.h"
#include "string_ex.h"
#include "adapter_device_config.h"

namespace OHOS {
namespace bluetooth {
constexpr int32_t APPEND_CHAR_NUM = 1;
constexpr int32_t UUID_LENGTH = 16;
// 128 bit UUID for PBAP
const std::vector<uint8_t> PBAP_TARGET_HEADER_UUID = {
    0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8,
    0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

const std::vector<std::string> LEGAL_PATH = {
    "/telecom",
    "/telecom/pb",
    "/telecom/fav",
    "/telecom/ich",
    "/telecom/och",
    "/telecom/mch",
    "/telecom/cch"
};

PbapPseObexServer::PbapPseObexServer(std::string &device, PbapPseServiceImpl *pbapPseServiceImpl)
    : deviceAddr_(device), pbapPseServiceImpl_(pbapPseServiceImpl)
{
    mVcardManager_ = PbabPseVcardManager::GetInstance();
}

int PbapPseObexServer::OnConnect(ObexHeader &request, ObexHeader &reply)
{
    HILOGI("OnConnect.");
    std::vector<uint8_t> targetHeaderUuid = request.GetHeader(TARGET);
    if (targetHeaderUuid.empty()) {
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }

    if (targetHeaderUuid.size() != UUID_LENGTH) {
        HILOGE("Wrong UUID length.");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    for (int i = 0; i < UUID_LENGTH; i++) {
        if (targetHeaderUuid[i] != PBAP_TARGET_HEADER_UUID[i]) {
            HILOGE("Wrong UUID.");
            return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
        }
    }
    reply.SetHeader(WHO, targetHeaderUuid);

    std::vector<uint8_t> remote = request.GetHeader(WHO);
    if (!remote.empty()) {
        reply.SetHeader(TARGET, remote);
    }

    std::vector<uint8_t> paramData = request.GetHeader(APPLICATION_PARAMETER);
    if (!connectAppParam_.ParseApplicationParameter(paramData)) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    needPhonebookSize_ = connectAppParam_.isNeedPbSize_;

    return ResponseCodes::OBEX_HTTP_OK;
}

int PbapPseObexServer::OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create)
{
    std::string currentPathTmp = currentPath_;
    std::string tmpPath = request.GetName();
    HILOGI("currentPath = %{public}s, backup = %{public}d, create = %{public}d, name = %{public}s",
        currentPath_.c_str(), backup, create, tmpPath.c_str());

    if (backup) {
        if (currentPathTmp.length() != 0) {
            currentPathTmp = currentPathTmp.substr(0, currentPathTmp.find_last_of('/'));
        }
    } else {
        if (tmpPath.empty()) {
            currentPathTmp = "";
        } else {
            currentPathTmp = currentPathTmp + "/" + tmpPath;
        }
    }
    HILOGD("currentPathTmp = %{public}s", currentPathTmp.c_str());

    if ((currentPathTmp.length() != 0) && (!IsLegalPath(currentPathTmp))) {
        if (create) {
            HILOGE("path create is forbidden!");
            return ResponseCodes::OBEX_HTTP_FORBIDDEN;
        }
        HILOGE("path is not legal");
        return ResponseCodes::OBEX_HTTP_NOT_FOUND;
    }
    currentPath_ = currentPathTmp;
    HILOGD("OnSetPath, currentPath_ == %{public}s", currentPath_.c_str());
    return ResponseCodes::OBEX_HTTP_OK;
}

int PbapPseObexServer::OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    int ret;
    std::string type = request.GetType();
    std::string name = request.GetName();
    HILOGI("type = %{public}s, name = %{public}s", type.c_str(), name.c_str());

    if (type.empty()) {
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    if (!CheckAccessAuthorization()) {
        HILOGE("Access reject");
        return ResponseCodes::OBEX_HTTP_NOT_FOUND;
    }

    // Accroding to specification,the name header could be omitted such as sony erriccsonHBH-DS980
    // For "x-bt/phonebook" and "x-bt/vcard-listing": if name == null,guess what carkit actually want from current path
    // For "x-bt/vcard": We decide which kind of content client would like per current path
    bool validName = true;
    if (name.empty()) {
        validName = false;
    }
    PbapPseAppParam appParam;
    if (!validName || type == TYPE_VCARD) {
        ret = GetContentTypeForNoNameOrVcard(validName, type, appParam.contentType_);
        if (ret != ResponseCodes::OBEX_HTTP_OK) {
            return ret;
        }
    } else {
        if (name.find(SIM1.c_str(), 0, SIM1.length()) != std::string::npos) {
            HILOGE("Not support access SIM card info!");
            return ResponseCodes::OBEX_HTTP_OK;
        }
        ret = GetContentTypeAndCounter(name, appParam);
        if (ret != ResponseCodes::OBEX_HTTP_OK) {
            return ret;
        }
    }

    std::vector<uint8_t> dataValue = request.GetHeader(APPLICATION_PARAMETER);
    if (!appParam.ParseApplicationParameter(dataValue)) {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
    needPhonebookSize_ = appParam.isNeedPbSize_;

    if (type == TYPE_LISTING) {
        ret = PullVcardListing(appParam, reply, output, sendBodyHeader);
    } else if (type == TYPE_VCARD) {
        ret = PullVcardEntry(appParam, reply, name, output, sendBodyHeader);
    } else if (type == TYPE_PB) {
        ret = PullPhonebook(appParam, reply, name, output, sendBodyHeader);
    } else {
        HILOGE("unknown type request, type = %{public}s", type.c_str());
        ret = ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    mVcardManager_->ReleaseDataShareHelper();
    return ret;
}

int PbapPseObexServer::OnDisconnect(ObexHeader &request, ObexHeader &reply)
{
    mVcardManager_->ReleaseDataShareHelper();
    return ResponseCodes::OBEX_HTTP_OK;
}

int PbapPseObexServer::OnAbort(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

int PbapPseObexServer::OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished)
{
    HILOGI("onPut(): not support PUT request.");
    return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
}
int PbapPseObexServer::OnDelete(ObexHeader &request, ObexHeader &reply)
{
    HILOGI("onDelete(): not support PUT request.!");
    return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
}

void PbapPseObexServer::OnClose(bool isThreadStart)
{
    HILOGI("isThreadStart: %{public}d", isThreadStart);
    CHECK_AND_RETURN_LOG(pbapPseServiceImpl_, "pbapPseServiceImpl_ is null");
    PbapPseMessage event(PBAP_PSE_DISCONNECT_EVT);
    event.dev_ = deviceAddr_;
    pbapPseServiceImpl_->PostEvent(event);
    mVcardManager_->ReleaseDataShareHelper();
}

void PbapPseObexServer::SetConnectionId(int connectionId)
{
    connectionId_ = connectionId;
}

int PbapPseObexServer::GetConnectionId()
{
    return connectionId_;
}

bool PbapPseObexServer::IsLegalPath(const std::string &path)
{
    if (path.length() == 0) {
        return true;
    }
    HILOGI("size = %{public}d, path = %{public}s", path.size(), path.c_str());
    for (size_t i = 0; i < LEGAL_PATH.size(); i++) {
        if (path == LEGAL_PATH[i]) {
            return true;
        }
    }
    return false;
}

bool PbapPseObexServer::IsNameMatchTarget(const std::string &name, const std::string &target)
{
    HILOGI("name = %{public}s, target = %{public}s", name.c_str(), target.c_str());
    if (name.empty()) {
        return false;
    }

    // end = .vcf, contentTypeName = telecom/pb
    uint32_t vcfLen = 4;
    std::string contentTypeName = name;
    if (contentTypeName.length() > vcfLen) {
        std::string end = contentTypeName.substr(contentTypeName.length() - vcfLen);
        if (end == ".vcf") {
            contentTypeName = contentTypeName.substr(0, contentTypeName.length() - vcfLen);
        }
    }

    size_t pos = contentTypeName.find("/");
    while (pos != std::string::npos) {
        std::string subName = contentTypeName.substr(0, pos);
        if (subName == target) {
            return true;
        }

        contentTypeName = contentTypeName.substr(pos + 1);
        pos = contentTypeName.find("/");
    }
    if (contentTypeName == target) {
        return true;
    }
    return false;
}

int PbapPseObexServer::PullVcardListing(PbapPseAppParam &appParam, ObexHeader &reply,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    if (appParam.searchAttribute_ == SEARCH_ATTR_SOUND) {
        HILOGE("not support search by sound");
        return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }
    if (appParam.searchAttribute_ > SEARCH_ATTR_SOUND) {
        HILOGE("searchAttribute invalid");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }

    if (appParam.order_ == ORDER_BY_PHONETIC) {
        HILOGE("not support order by sound");
        return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
    }
    if (appParam.order_ > ORDER_BY_PHONETIC) {
        HILOGE("Order parameter invalid");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }

    int32_t pbSize = mVcardManager_->GetPhonebookSize(appParam.contentType_);
    int32_t ret = HandleAppParaForResponse(pbSize, reply);
    if (ret != -1) {
        sendBodyHeader = false;
        return ret;
    }
    if (pbSize == 0) {
        HILOGI("pbSize is 0, return");
        return ResponseCodes::OBEX_HTTP_OK;
    }
    return SendVcardListingXml(reply, appParam, output);
}

int PbapPseObexServer::SendVcardListingXml(ObexHeader &reply, PbapPseAppParam &appParam, std::vector<uint8_t> &output)
{
    std::string xml;
    xml.append("<?xml version=\"1.0\"?>");
    xml.append("<!DOCTYPE vcard-listing SYSTEM \"vcard-listing.dtd\">");
    xml.append("<vCard-listing version=\"1.0\">");
    if (appParam.contentType_ == PbapContentType::PHONEBOOK || appParam.contentType_ == PbapContentType::FAVORITES) {
        CreatePhoneBookList(appParam, xml);
    } else {
        CreateCallHistoryList(appParam, xml);
    }
    xml.append("</vCard-listing>");
    output.insert(output.end(), xml.begin(), xml.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

void WriteVcardXmlData(int32_t index, const std::string &name, std::string &xml)
{
    xml.append("<card handle=\"");
    xml.append(std::to_string(index));
    xml.append(".vcf\" name=\"");
    for (auto ch : name) {
        if (ch == '<') {
            xml.append("&lt;");
        } else if (ch == '>') {
            xml.append("&gt;");
        } else if (ch == '\"') {
            xml.append("&quot;");
        } else if (ch == '\'') {
            xml.append("&#039;");
        } else if (ch == '&') {
            xml.append("&amp;");
        } else {
            xml.append(APPEND_CHAR_NUM, ch);
        }
    }
    xml.append("\"/>");
}

void PbapPseObexServer::CreatePhoneBookList(PbapPseAppParam &appParam, std::string &xml)
{
    std::vector<std::string> selectNameList;
    std::vector<uint32_t> selectNameListPos;
    std::vector<std::string> nameList;
    mVcardManager_->GetContactNameList(nameList, appParam.order_);

    if (appParam.searchAttribute_ == SEARCH_ATTR_NUMBER) {
        std::vector<std::string> names;
        mVcardManager_->GetContactNamesByNumber(names, appParam.searchValue_);
        CreateNumSearchType(nameList, names, selectNameList, selectNameListPos);
    } else {
        CreateOtherSearchType(appParam, nameList, selectNameList, selectNameListPos);
    }
    uint32_t requestSize = nameList.size() >= appParam.maxListCount_ ? appParam.maxListCount_ : nameList.size();
    uint32_t start = appParam.listStartOffset_;
    uint32_t end = (start + requestSize) > selectNameList.size() ? selectNameList.size() : (start + requestSize);
    HILOGI("requestSize %{public}u, start %{public}u, end %{public}u", requestSize, start, end);
    for (uint32_t i = start; i < end; i++) {
        WriteVcardXmlData(selectNameListPos[i], selectNameList[i], xml);
    }
}

void PbapPseObexServer::CreateNumSearchType(std::vector<std::string> &nameList, std::vector<std::string> &names,
    std::vector<std::string> &selectNameList, std::vector<uint32_t> &selectNameListPos)
{
    std::string compareValue;
    std::string currentValue;
    for (uint32_t i = 0; i < names.size(); i++) {
        compareValue = TrimStr(names[i]);
        for (uint32_t j = 0; j < nameList.size(); j++) {
            currentValue = nameList[j];
            if (currentValue != compareValue) {
                continue;
            }
            auto index = currentValue.find_last_of(',');
            if (index != std::string::npos) {
                currentValue = currentValue.substr(0, index);
            }
            selectNameList.push_back(currentValue);
            selectNameListPos.push_back(j);
        }
    }
}

void PbapPseObexServer::CreateOtherSearchType(PbapPseAppParam &appParam, std::vector<std::string> &nameList,
    std::vector<std::string> &selectNameList, std::vector<uint32_t> &selectNameListPos)
{
    std::string compareValue;
    std::string currentValue;
    if (!appParam.searchValue_.empty()) {
        compareValue = LowerStr(TrimStr(appParam.searchValue_));
    }
    for (uint32_t j = 0; j < nameList.size(); j++) {
        currentValue = nameList[j];
        if (currentValue != compareValue) {
            continue;
        }
        auto index = currentValue.find_last_of(',');
        if (index != std::string::npos) {
            currentValue = currentValue.substr(0, index);
        }
        if (compareValue.empty() || (currentValue.find(compareValue) == 0)) {
            selectNameList.push_back(currentValue);
            selectNameListPos.push_back(j);
        }
    }
}

void PbapPseObexServer::CreateCallHistoryList(PbapPseAppParam &appParam, std::string &xml)
{
    std::vector<std::string> nameList;
    mVcardManager_->GetCallHistoryList(nameList, appParam.contentType_);
    uint32_t requestSize = nameList.size() >= appParam.maxListCount_ ? appParam.maxListCount_ : nameList.size();
    uint32_t start = appParam.listStartOffset_;
    uint32_t end = (start + requestSize) > nameList.size() ? nameList.size() : (start + requestSize);
    for (uint32_t i = start; i < end; i++) {
        WriteVcardXmlData(i + 1, nameList[i], xml);
    }
}

int32_t PbapPseObexServer::GetRawContactIdByNameIndex(int32_t index, uint8_t orderBy)
{
    int32_t rawContactId = 0;
    // 查询raw contact 表的nameList&rawContactid
    std::vector<std::string> nameList;
    mVcardManager_->GetContactNameList(nameList, orderBy);
    if (nameList.size() > UINT16_MAX) {
        HILOGE("nameList size over uint16 max");
        return rawContactId;
    }
    // 找到index对应的name 和rawContactid
    if (index < 0 || index >= static_cast<int32_t>(nameList.size())) {
        HILOGE("The vcard is not acceptable! index: %{public}d, name size: %{public}d", index, nameList.size());
        return rawContactId;
    }
    std::string& nameWithRawContactId = nameList[index];

    auto idIndex = nameWithRawContactId.find_last_of(',');
    if (idIndex != std::string::npos && idIndex >= 1) {
        std::string rawContactIdStr = nameWithRawContactId.substr(idIndex + 1);
        CHECK_AND_RETURN_LOG_RET(ConvertStrToDigit(rawContactIdStr, rawContactId),
            rawContactId, "convertStrToDigit error");
    }
    HILOGI("rawContactId is : %{public}d", rawContactId);
    return rawContactId;
}

int PbapPseObexServer::PullVcardEntry(PbapPseAppParam &appParam, ObexHeader &reply, std::string &name,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    int32_t index = 0;
    if (!GetVcardEntryIndex(name, index)) {
        HILOGE("index return false, return");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    int32_t pbSize = mVcardManager_->GetPhonebookSize(appParam.contentType_);
    HILOGI("index: %{public}d, pbSize: %{public}d", index, pbSize);
    int32_t needSendBody = HandleAppParaForResponse(pbSize, reply);
    if (needSendBody != -1) {
        sendBodyHeader = false;
        return needSendBody;
    }
    if (pbSize == 0) {
        HILOGE("pbSize is 0, return");
        return ResponseCodes::OBEX_HTTP_OK;
    }
    if (appParam.contentType_ == 0) {
        HILOGE("wrong path");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    if (index < 0 || index >= pbSize) {
        HILOGE("The requested vcard is not acceptable! index: %{public}d, pbSize: %{public}d", index, pbSize);
        return ResponseCodes::OBEX_HTTP_NOT_FOUND;
    }

    int shareType = 0;
    GetPbapShareType(deviceAddr_, shareType);
    HILOGI("index %{public}d order %{public}d, contentType %{public}d",
        index, appParam.order_, appParam.contentType_);
    if (appParam.contentType_ == PbapContentType::PHONEBOOK || appParam.contentType_ == PbapContentType::FAVORITES) {
        if (index == 0 && appParam.contentType_ == PbapContentType::PHONEBOOK) {
            // For PB_PATH, 0.vcf is the phone number of this phone.
            return mVcardManager_->ComposeOwnerPhoneNumberVcard(appParam, output);
        }
        // 查询index对应的rawContactId
        int32_t rawContactId = GetRawContactIdByNameIndex(index, appParam.order_);
        if (rawContactId == 0) {
            HILOGE("rawContactId is invaild");
            return ResponseCodes::OBEX_HTTP_NOT_FOUND;
        }
        // 组装contactId对应的单个vcard
        return mVcardManager_->ComposePhonebookOneVcard(appParam, rawContactId, ORDER_BY_INDEXED, shareType, output);
    }
    // For others (ich/och/cch/mch), 0.vcf is meaningless, and must begin from 1.vcf
    if (index >= 1) {
        CallLogSelectParam param{ index, index, pbSize, needSendBody };
        return mVcardManager_->ComposeCallLogVcards(appParam, param, output);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

bool PbapPseObexServer::GetVcardEntryIndex(std::string &name, int32_t &index)
{
    if (name.empty() || name.length() < VCARD_NAME_SUFFIX_LENGTH) {
        HILOGE("Name is Null, or the length of name < 5 !");
        return false;
    }
    std::string strIndex = name.substr(0, name.length() - VCARD_NAME_SUFFIX_LENGTH + 1);
    if (strIndex.empty()) {
        return true;
    }
    CHECK_AND_RETURN_LOG_RET(ConvertStrToDigit(strIndex, index), false, "parse length error");
    return true;
}

int PbapPseObexServer::PullPhonebook(PbapPseAppParam &appParam, ObexHeader &reply, std::string &name,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    HILOGI("PullPhonebook. contentType: %{public}d", appParam.contentType_);
    if (!CheckNameIncludeVcf(name)) {
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }

    int32_t pbSize = mVcardManager_->GetPhonebookSize(appParam.contentType_);
    int32_t needSendBody = HandleAppParaForResponse(pbSize, reply);
    if (needSendBody != -1) {
        sendBodyHeader = false;
        return needSendBody;
    }
    if (pbSize == 0) {
        HILOGI("PhonebookSize is 0.");
        return ResponseCodes::OBEX_HTTP_OK;
    }
    CallLogSelectParam selectParam{ 0, 0, needSendBody, pbSize };
    if (!CalcCallLogParam(appParam, selectParam)) {
        return ResponseCodes::OBEX_HTTP_OK;
    }

    int shareType = 0;
    GetPbapShareType(deviceAddr_, shareType);

    if ((appParam.contentType_ == PbapContentType::PHONEBOOK) || appParam.contentType_ == PbapContentType::FAVORITES) {
        if (selectParam.startPoint == 0) {
            int32_t ret = mVcardManager_->ComposeOwnerPhoneNumberVcard(appParam, output);
            if (selectParam.endPoint == 0) {
                return ret;
            }
            selectParam.startPoint = 1;
        }
        return mVcardManager_->ComposePhonebookVcards(reply, appParam, selectParam, shareType, output);
    }
    return mVcardManager_->ComposeCallLogVcards(appParam, selectParam, output);
}

bool PbapPseObexServer::CalcCallLogParam(PbapPseAppParam &appParam, CallLogSelectParam &param)
{
    int32_t requestSize = param.pbSize >= static_cast<int32_t>(
        appParam.maxListCount_) ? static_cast<int32_t>(appParam.maxListCount_) : param.pbSize;
    int32_t startIndex = 1;
    int32_t lastIndex = param.pbSize;
    if (appParam.contentType_ == PbapContentType::PHONEBOOK) {
        startIndex = 0;
        lastIndex = param.pbSize - 1;
    }
    param.startPoint = startIndex + appParam.listStartOffset_;
    if (param.startPoint > lastIndex) {
        HILOGI("listStartOffset is not correct. startPoint: %{public}d", param.startPoint);
        return false;
    }
    param.endPoint = param.startPoint + requestSize - 1;
    if (param.endPoint > lastIndex) {
        param.endPoint = lastIndex;
    }
    HILOGI("startPoint: %{public}d, endPoint: %{public}d, startIndex: %{public}d, lastIndex: %{public}d, requestSize: "
        "%{public}d", param.startPoint, param.endPoint, startIndex, lastIndex, requestSize);
    return true;
}

bool PbapPseObexServer::CheckNameIncludeVcf(std::string &name)
{
    if (name.empty()) {
        return true;
    }

    size_t pos = name.find(".");
    if (pos == std::string::npos) {
        return true;
    }
    std::string vcf = "vcf";
    std::string subName = name.substr(pos + 1, name.length() - 1);
    if (subName == vcf) {
        return true;
    }
    HILOGI("name is not .vcf.");
    return false;
}

int PbapPseObexServer::GetContentTypeForNoNameOrVcard(bool validName, std::string &type, int &contentType)
{
    if (currentPath_ == PB_PATH) {
        contentType = PbapContentType::PHONEBOOK;
    } else if (currentPath_ == FAV_PATH) {
        contentType = PbapContentType::FAVORITES;
    } else if (currentPath_ == ICH_PATH) {
        contentType = PbapContentType::INCOMING_CALL_HISTORY;
    } else if (currentPath_ == OCH_PATH) {
        contentType = PbapContentType::OUTGOING_CALL_HISTORY;
    } else if (currentPath_ == MCH_PATH) {
        contentType = PbapContentType::MISSED_CALL_HISTORY;
        needNewMissedCallsNum_ = true;
    } else if (currentPath_ == CCH_PATH) {
        contentType = PbapContentType::COMBINED_CALL_HISTORY;
    } else if (currentPath_ == TELECOM_PATH) {
        if (!validName && type == TYPE_LISTING) { // PBAP 1.1.1 change
            HILOGE("invalid vcard listing request in default folder");
            return ResponseCodes::OBEX_HTTP_NOT_FOUND;
        }
    } else {
        HILOGE("currentPath_ is not valid path!!!");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    HILOGI("contentType = %{public}d", contentType);
    return ResponseCodes::OBEX_HTTP_OK;
}

int PbapPseObexServer::GetContentTypeAndCounter(std::string &name, PbapPseAppParam &appParam)
{
    // we have weak name checking here to provide better compatibility with other devices,
    // although unique name such as "pb.vcf" is required by SIG spec.
    HILOGI("currentPath_ = %{public}s", currentPath_.c_str());
    int32_t contentType;
    bool isHaveCallHistoryVersionCounter = false;
    if (IsNameMatchTarget(name, PB)) {
        contentType = PbapContentType::PHONEBOOK;
    } else if (IsNameMatchTarget(name, FAV)) {
        contentType = PbapContentType::FAVORITES;
    } else if (IsNameMatchTarget(name, ICH)) {
        contentType = PbapContentType::INCOMING_CALL_HISTORY;
        isHaveCallHistoryVersionCounter = true;
    } else if (IsNameMatchTarget(name, OCH)) {
        contentType = PbapContentType::OUTGOING_CALL_HISTORY;
        isHaveCallHistoryVersionCounter = true;
    } else if (IsNameMatchTarget(name, MCH)) {
        contentType = PbapContentType::MISSED_CALL_HISTORY;
        isHaveCallHistoryVersionCounter = true;
    } else if (IsNameMatchTarget(name, CCH)) {
        contentType = PbapContentType::COMBINED_CALL_HISTORY;
        isHaveCallHistoryVersionCounter = true;
    } else {
        HILOGW("Input name doesn't contain valid info!!!");
        return ResponseCodes::OBEX_HTTP_NOT_FOUND;
    }
    appParam.contentType_ = contentType;
    if (isHaveCallHistoryVersionCounter) {
        mVcardManager_->GetCallHistoryPrimaryFolderVersion(appParam.contentType_, appParam.callHistoryVersionCounter_);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

int32_t PbapPseObexServer::HandleAppParaForResponse(int32_t size, ObexHeader &reply)
{
    std::vector<uint8_t> appParameter{};
    // In such case, PCE only want the number of index. So response not contain any Body header.
    if (needPhonebookSize_) {
        HILOGI("Need Phonebook size in response header");
        needPhonebookSize_ = false;
        std::vector<uint8_t> pbData{};
        pbData.push_back((size >> BASE_BIT_SIZE_8) & 0xFF);
        pbData.push_back(size & 0xFF);
        AppendAppHeaderData(TRIPLET_TAGID_PHONEBOOKSIZE, TRIPLET_LENGTH_PHONEBOOKSIZE, pbData, appParameter);
        if (needNewMissedCallsNum_) {
            needNewMissedCallsNum_ = false;
            AppendNewMissdCallData(appParameter);
        }
        reply.SetHeader(APPLICATION_PARAMETER, appParameter);
        return ResponseCodes::OBEX_HTTP_OK;
    }
    if (needNewMissedCallsNum_) {
        needNewMissedCallsNum_ = false;
        AppendNewMissdCallData(appParameter);
        reply.SetHeader(APPLICATION_PARAMETER, appParameter);
    }
    return -1;
}

void PbapPseObexServer::AppendNewMissdCallData(std::vector<uint8_t> &retValue)
{
    uint8_t missedCallsNum = mVcardManager_->GetNewMissedCallNum();
    std::vector<uint8_t> mcData{};
    mcData.push_back(missedCallsNum);
    AppendAppHeaderData(TRIPLET_TAGID_NEWMISSEDCALLS, TRIPLET_LENGTH_NEWMISSEDCALLS, mcData, retValue);
}

void PbapPseObexServer::AppendAppHeaderData(uint8_t tag, uint8_t length, std::vector<uint8_t> &data,
    std::vector<uint8_t> &retValue)
{
    retValue.push_back(tag);
    retValue.push_back(length);
    retValue.insert(retValue.end(), data.begin(), data.end());
}

bool PbapPseObexServer::CheckAccessAuthorization()
{
    int32_t permission = 0;
    if (!GetPbapPermission(deviceAddr_, permission)) {
        return false;
    }
    return (permission == static_cast<int32_t>(BTPermissionType::ACCESS_ALLOWED));
}
} // namespace bluetooth
} // namespace OHOS