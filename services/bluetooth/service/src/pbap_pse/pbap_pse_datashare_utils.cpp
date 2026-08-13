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
#define LOG_TAG "bt_service_pbap_datashare_utils"
#endif

#include <chrono>
#include <ctime>
#include <sstream>
#include "log.h"
#include "pbap_pse_datashare_utils.h"
#include "vcard_constructor.h"
#include "vcard_manager.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {

namespace {
constexpr const char *CONTACT_DATASHARE_URI = "datashare:///com.ohos.contactsdataability";
constexpr const char *CONTACT_DATA_DATASHARE_URI_CONTENT =
    "datashare:///com.ohos.contactsdataability/contacts/contact_data";
constexpr const char *RAW_CONTACT_DATASHARE_URI_CONTENT =
    "datashare:///com.ohos.contactsdataability/contacts/raw_contact";
constexpr const char *CONTACT_DATASHARE_URI_CONTENT =
    "datashare:///com.ohos.contactsdataability/contacts/contact";

constexpr const char *CALL_LOG_DATASHARE_URI = "datashare:///com.ohos.calllogability";
constexpr const char *CALL_LOG_DATASHARE_URI_CONTENT = "datashare:///com.ohos.calllogability/calls/calllog";
constexpr const char *CALL_LOG_DATASHARE_PROXY_URI_CONTENT =
    "datashareproxy://com.ohos.contactsdataability/calls/calllog?Proxy=true&user=";

// field for call log datashare
constexpr const char *DISPLAY_NAME = "display_name";
constexpr const char *PHONE_NUMBER = "phone_number";
constexpr const char *CREATE_TIME = "create_time";
constexpr const char *CALL_DIRECTION = "call_direction";
constexpr const char *NUMBER_TYPE = "number_type";
constexpr const char *NUMBER_TYPE_NAME = "number_type_name";
constexpr const char *ANSWER_STATE = "answer_state";
constexpr const char *IS_READ = "is_read";
constexpr const char *CALLLOG_ID = "id";

//field for contact datashare
constexpr const char *ID = "id";
constexpr const char *CONTACT_ID = "contact_id";
constexpr const char *IS_DELETED = "is_deleted";
constexpr const char *DETAIL_INFO = "detail_info";
constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
constexpr const char *TYPE_ID = "type_id";
constexpr const char *RAW_CONTACT_ID = "raw_contact_id";

// Property for call log entry
constexpr const char *VCARD_PROPERTY_X_TIMESTAMP = "X-IRMC-CALL-DATETIME";
constexpr const char *VCARD_PROPERTY_CALLTYPE_INCOMING = "RECEIVED";
constexpr const char *VCARD_PROPERTY_CALLTYPE_OUTGOING = "DIALED";
constexpr const char *VCARD_PROPERTY_CALLTYPE_MISSED = "MISSED";

constexpr int TYPE_ID_NUMBER = 5;
constexpr int TYPE_ID_NAME = 6;
constexpr uint32_t CORE_EVENT_PRIORITY = 1;
constexpr uint32_t QUERY_NUM_LENGTH_LIMIT = 7;
constexpr uint32_t QUERY_PHONE_NUM_LENGTH_LIMIT = 11;

// direction call log
enum CallDirection {
    INCOMING_TYPE = 0,
    OUTGOING_TYPE = 1,
};

enum AnswerState {
    CALL_ANSWER_MISSED = 0,
    CALL_ANSWER_ACTIVED = 1,
    CALL_ANSWER_REJECT = 2,
};

constexpr const char *UNKNOWN_NAME = "Unknown name";
} // namespace

PbabPseDataShareUtils::PbabPseDataShareUtils()
{
    HILOGI("dataShare init.");
}

PbabPseDataShareUtils::~PbabPseDataShareUtils()
{
    HILOGI("~PbabPseDataShareUtils.");
    ReleaseCallLogDataShareHelper();
    ReleaseContactDataShareHelper();
}

void PbabPseDataShareUtils::ReleaseCallLogDataShareHelper()
{
    std::lock_guard<std::mutex> lock(mCallLogDataShareHelperMutex_);
    if (mCallLogDataShareHelper_ == nullptr) {
        return;
    }
    bool result = mCallLogDataShareHelper_->Release();
    mCallLogDataShareHelper_ = nullptr;
    HILOGI("ReleaseCallLogDataShareHelper result is %{public}d", result);
    return;
}

void PbabPseDataShareUtils::ReleaseContactDataShareHelper()
{
    std::lock_guard<std::mutex> lock(mCallDataShareHelperMutex_);
    if (mCallDataShareHelper_ == nullptr) {
        return;
    }
    bool result = mCallDataShareHelper_->Release();
    mCallDataShareHelper_ = nullptr;
    HILOGI("ReleaseContactDataShareHelper result is %{public}d", result);
    return;
}

std::shared_ptr<DataShare::DataShareHelper> PbabPseDataShareUtils::CreateDataShareHelper(const std::string &uriStr)
{
    HITRACE_METER(BT_TRACE_TAG);
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbilityManager failed.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, uriStr);
}

std::shared_ptr<DataShare::DataShareHelper> PbabPseDataShareUtils::GetCallLogDataShareHelper(std::string &url)
{
    std::lock_guard<std::mutex> lock(mCallLogDataShareHelperMutex_);
    if (mCallLogDataShareHelper_ == nullptr || lastUrl_ == "") {
        HILOGI("mCallLogDataShareHelper_ is nullptr. retry");
        AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId_);
        AccountSA::OsAccountManager::IsOsAccountVerified(userId_, isUserUnlocked_);
        if (!isUserUnlocked_) {
            url = CALL_LOG_DATASHARE_PROXY_URI_CONTENT + std::to_string(userId_);
        } else {
            url = CALL_LOG_DATASHARE_URI_CONTENT;
        }
        mCallLogDataShareHelper_ = CreateDataShareHelper(url);
        lastUrl_ = url;
        HILOGI("url %{public}s isUserUnlocked %{public}d, userId %{public}d", url.c_str(), isUserUnlocked_, userId_);
    }
    url = lastUrl_;
    return mCallLogDataShareHelper_;
}

std::shared_ptr<DataShare::DataShareHelper> PbabPseDataShareUtils::GetContactDataShareHelper()
{
    std::lock_guard<std::mutex> lock(mCallDataShareHelperMutex_);
    if (mCallDataShareHelper_ == nullptr) {
        HILOGW("mCallDataShareHelper_ is nullptr, retry.");
        mCallDataShareHelper_ = CreateDataShareHelper(CONTACT_DATASHARE_URI);
    }
    return mCallDataShareHelper_;
}

DataShare::DataSharePredicates PbabPseDataShareUtils::CreatContactPredicates(int32_t number, int32_t offset,
    std::string &orderByStr, int32_t shareType)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("CreatContactPredicates  number = %{public}d, offset = %{public}d, orderByStr = %{public}s, "
        "shareType %{public}d", number, offset, orderByStr.c_str(), shareType);

    DataShare::DataSharePredicates predicates;
    if (!orderByStr.empty()) {
        predicates.SetOrder(orderByStr);
    }
    predicates.EqualTo(IS_DELETED, 0);
    predicates.Limit(number, offset - 1); // limit 是从当前offset的下一个取number条数据

    return predicates;
}

std::string PbabPseDataShareUtils::ComposeContactVcard(int32_t vcardType, DataShare::DataSharePredicates &predicates)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("ComposeContactVcard !");
    std::string strVcard;
    auto callDataShareHelper = GetContactDataShareHelper();
    // 组装联系人 vcard
    Telephony::VCardManager::GetInstance().SetDataHelper(callDataShareHelper);
    {
        HITRACE_METER_NAME(BT_TRACE_TAG, "VCardManager::ExportToStr");
        Telephony::VCardManager::GetInstance().ExportToStr(strVcard, predicates, vcardType);
    }
    return strVcard;
}

int32_t PbabPseDataShareUtils::GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &fieldName, std::string& out)
{
    HITRACE_METER(BT_TRACE_TAG);
    int index = 0;
    auto ret = resultSet->GetColumnIndex(fieldName, index);
    if (ret != DataShare::E_OK) {
        HILOGW("GetValue [%{public}s] failed [%{public}d]", fieldName.c_str(), ret);
        return ret;
    }
    return resultSet->GetString(index, out);
}

int32_t PbabPseDataShareUtils::GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &fieldName, int64_t &out)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t index = 0;
    auto ret = resultSet->GetColumnIndex(fieldName, index);
    if (ret != DataShare::E_OK) {
        HILOGW("GetValue [%{public}s] failed [%{public}d]", fieldName.c_str(), ret);
        return ret;
    }
    return resultSet->GetLong(index, out);
}

int32_t PbabPseDataShareUtils::GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &fieldName, int32_t &out)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t index = 0;
    auto ret = resultSet->GetColumnIndex(fieldName, index);
    if (ret != DataShare::E_OK) {
        HILOGW("GetValue [%{public}s] failed [%{public}d]", fieldName.c_str(), ret);
        return ret;
    }
    return resultSet->GetInt(index, out);
}

std::string PbabPseDataShareUtils::ComposeCallLogVcard(int32_t contentType, int32_t vcardType,
    int32_t number, int32_t offset)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::string callLogVard;
    DataShare::DataSharePredicates predicates;
    GenerateCallLogParamByType(contentType, predicates);
    std::vector<std::string> columns;
    predicates.Limit(number, offset - 1); // limit 是从当前offset的下一个取number条数据
    predicates.OrderByDesc(CALLLOG_ID);

    std::string url = "";
    auto callLogDataShareHelper = GetCallLogDataShareHelper(url);
    if (callLogDataShareHelper == nullptr) {
        HILOGE("callLogDataShareHelper is nullptr.");
        return "";
    }
    HILOGI("number =  %{public}d, offset = %{public}d", number, offset);
    Uri uri(url);

    auto result = callLogDataShareHelper->Query(uri, predicates, columns);
    if (result == nullptr) {
        HILOGE("dataShare query error, result is null");
        return callLogVard;
    }
    int32_t opRet = result->GoToFirstRow();
    while (opRet == DataShare::E_OK) {
        callLogVard += CreateOneCallLog(vcardType, result);
        opRet = result->GoToNextRow();
    }

    return callLogVard;
}

std::string PbabPseDataShareUtils::CreateOneCallLog(int32_t vcardType,
    std::shared_ptr<DataShare::DataShareResultSet> &result)
{
    HITRACE_METER(BT_TRACE_TAG);
    // 获取姓名
    std::string name;
    if (GetValue(result, DISPLAY_NAME, name) != DataShare::E_OK) {
        name = "";
    }
    // 获取电话号码
    std::string number;
    if (GetValue(result, PHONE_NUMBER, number) != DataShare::E_OK) {
        number = "";
    }
    // labelId
    int type = -1;
    if (GetValue(result, NUMBER_TYPE, type) != DataShare::E_OK) {
        HILOGW("get NUMBER_TYPE fail");
    }
    type = static_cast<int32_t>(Telephony::PhoneVcType::NUM_HOME); // 通话管理暂未实现，先写死
    std::string labelId = std::to_string(type);
    // labelName
    std::string labelName;
    if (GetValue(result, NUMBER_TYPE_NAME, labelName) != DataShare::E_OK) {
        HILOGW("get NUMBER_TYPE_NAME fail");
    }
    if (labelName.empty()) {
        labelName = labelId;
    }

    // 获取呼叫时间 e.g. "X-IRMC-CALL-DATETIME;MISSED:20050320T100000"
    std::string callLogTimePara = GetCallLogTimePara(result);
    std::string timestr = ToRfc2455Format(result);

    std::vector<std::string> timeParamList;
    timeParamList.push_back(callLogTimePara);
    // 组装通话记录 vcard
    Telephony::VCardConstructor vcardConstructor(vcardType);
    vcardConstructor.ContactBegin();
    vcardConstructor.AddLine(Telephony::VCARD_TYPE_FN, name);
    vcardConstructor.AddLine(Telephony::VCARD_TYPE_N, name);
    vcardConstructor.AddTelLine(labelId, labelName, number);
    vcardConstructor.AddLine(VCARD_PROPERTY_X_TIMESTAMP, timeParamList, timestr);
    vcardConstructor.ContactEnd();
    return vcardConstructor.ToString();
}

std::string PbabPseDataShareUtils::ComposeOwnerVcard(int32_t vcardType, std::string &name, std::string &number)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t numType = static_cast<int32_t>(Telephony::PhoneVcType::NUM_MOBILE);
    std::string labelName = std::to_string(numType);
    Telephony::VCardConstructor vcardConstructor(vcardType);
    vcardConstructor.ContactBegin();
    if (!name.empty()) {
        vcardConstructor.AddLine(Telephony::VCARD_TYPE_FN, name);
    }
    if (!number.empty()) {
        vcardConstructor.AddTelLine(labelName, labelName, number);
    }
    vcardConstructor.ContactEnd();
    return vcardConstructor.ToString();
}

std::string PbabPseDataShareUtils::GetCallLogTimePara(std::shared_ptr<DataShare::DataShareResultSet> &result)
{
    std::string callLogTypeStr = "";
    int32_t direction;
    int32_t answerState;
    if (GetValue(result, CALL_DIRECTION, direction) != DataShare::E_OK) {
        HILOGW("unknown calllog typel");
        return callLogTypeStr;
    }
    if (GetValue(result, ANSWER_STATE, answerState) != DataShare::E_OK) {
        HILOGW("unknown answer state");
        return callLogTypeStr;
    }

    switch (direction) {
        case INCOMING_TYPE: {
            if (answerState == CALL_ANSWER_MISSED) {
                callLogTypeStr = VCARD_PROPERTY_CALLTYPE_MISSED;
            } else {
                callLogTypeStr = VCARD_PROPERTY_CALLTYPE_INCOMING;
            }
            break;
        }
        case OUTGOING_TYPE: {
            callLogTypeStr = VCARD_PROPERTY_CALLTYPE_OUTGOING;
            break;
        }
        default: {
            HILOGW("Call log type not correct.");
            return callLogTypeStr;
        }
    }
    return callLogTypeStr;
}

std::string PbabPseDataShareUtils::ToRfc2455Format(std::shared_ptr<DataShare::DataShareResultSet> &result)
{
    std::string timeStr = "";
    int64_t time64;
    if (GetValue(result, CREATE_TIME, time64) != DataShare::E_OK) {
        return timeStr;
    }

    // format  "yyyyMMdd'T'HHmmss"
    std::tm *localNow = std::localtime(&time64);
    std::ostringstream ss;
    ss << std::put_time(localNow, "%Y%m%dT%H%M%S");

    timeStr = ss.str();
    return timeStr;
}

int32_t PbabPseDataShareUtils::GetContactsSize(int32_t contentType)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::string url = "";
    std::vector<std::string> columns = {};
    DataShare::DataSharePredicates predicates;
    url = CONTACT_DATASHARE_URI_CONTENT;
    if (contentType == PbapContentType::FAVORITES) {
        predicates.EqualTo("favorite", 1);
    }
    columns.push_back(ID);
    predicates.EqualTo(IS_DELETED, 0);
    return GetQueryResultCount(QUERY_TABLE_TYPE_CONTACT, url, predicates, columns);
}

int32_t PbabPseDataShareUtils::GetCallHistorySize(int32_t contentType)
{
    HITRACE_METER(BT_TRACE_TAG);
    DataShare::DataSharePredicates predicates;
    GenerateCallLogParamByType(contentType, predicates);
    std::vector<std::string> columns = {"id"};
    std::string url = CALL_LOG_DATASHARE_URI_CONTENT;
    return GetQueryResultCount(QUERY_TABLE_TYPE_CALLLOG, url, predicates, columns);
}

int32_t PbabPseDataShareUtils::GetNewMissedCallNum()
{
    HITRACE_METER(BT_TRACE_TAG);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(CALL_DIRECTION, INCOMING_TYPE)->And()->EqualTo(ANSWER_STATE, CALL_ANSWER_MISSED)
        ->And()->EqualTo(IS_READ, 0);
    std::vector<std::string> columns = {"id"};
    std::string url = CALL_LOG_DATASHARE_URI_CONTENT;
    return GetQueryResultCount(QUERY_TABLE_TYPE_CALLLOG, url, predicates, columns);
}

int32_t PbabPseDataShareUtils::GetCallHistorySizeByTime(int32_t contentType, int64_t timeStamp)
{
    HITRACE_METER(BT_TRACE_TAG);
    DataShare::DataSharePredicates predicates;
    GenerateCallLogParamByType(contentType, predicates);
    predicates.GreaterThan("create_time", timeStamp);

    std::vector<std::string> columns = {"id"};
    std::string url = CALL_LOG_DATASHARE_URI_CONTENT;
    return GetQueryResultCount(QUERY_TABLE_TYPE_CALLLOG, url, predicates, columns);
}

int32_t PbabPseDataShareUtils::GetQueryResultCount(int32_t queryTableType, std::string &url,
    DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t retCount = 0;
    std::shared_ptr<DataShare::DataShareHelper> dataShare = nullptr;
    if (queryTableType == QUERY_TABLE_TYPE_CONTACT) {
        dataShare = GetContactDataShareHelper();
    } else if (queryTableType == QUERY_TABLE_TYPE_CALLLOG) {
        dataShare = GetCallLogDataShareHelper(url);
    } else {
        HILOGE("queryTableType is error");
        return retCount;
    }
    if (dataShare == nullptr) {
        HILOGE("dataShare is nullptr");
        return retCount;
    }
    Uri uri(url);
    auto result = dataShare->Query(uri, predicates, columns);
    if (result == nullptr) {
        HILOGE("dataShare query error, result is null");
        return retCount;
    }
    int32_t opRet = result->GoToFirstRow();
    while (opRet == DataShare::E_OK) {
        retCount++;
        opRet = result->GoToNextRow();
    }
    return retCount;
}

void PbabPseDataShareUtils::GenerateCallLogParamByType(int32_t contentType, DataShare::DataSharePredicates &predicates)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (contentType == INCOMING_CALL_HISTORY) {
        predicates.EqualTo(CALL_DIRECTION, INCOMING_TYPE);
    } else if (contentType == OUTGOING_CALL_HISTORY) {
        predicates.EqualTo(CALL_DIRECTION, OUTGOING_TYPE);
    } else if (contentType == MISSED_CALL_HISTORY) {
        predicates.EqualTo(CALL_DIRECTION, INCOMING_TYPE)->And()->EqualTo(ANSWER_STATE, CALL_ANSWER_MISSED);
    } else if (contentType == COMBINED_CALL_HISTORY) {
        HILOGD("COMBINED_CALL_HISTORY, no need filter");
    } else {
        HILOGE("contentType is error, contentType: %{public}d", contentType);
    }
}

void PbabPseDataShareUtils::GetCallHistoryList(std::vector<std::string> &names, int32_t contentType)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::string url = "";
    auto callLogDataShareHelper = GetCallLogDataShareHelper(url);
    if (callLogDataShareHelper == nullptr) {
        HILOGE("callLogDataShareHelper is nullptr.");
        return;
    }
    DataShare::DataSharePredicates predicates;
    GenerateCallLogParamByType(contentType, predicates);
    Uri uri(url);
    std::vector<std::string> columns = {PHONE_NUMBER, DISPLAY_NAME};
    auto resultSet = callLogDataShareHelper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOGE("callLogDataShareHelper query error, result is null");
        return;
    }

    int32_t operationResult = resultSet->GoToFirstRow();
    while (operationResult == DataShare::E_OK) {
        GetNameListBySet(names, resultSet);
        operationResult = resultSet->GoToNextRow();
    }
}

void PbabPseDataShareUtils::GetNameListBySet(std::vector<std::string> &names,
    std::shared_ptr<DataShare::DataShareResultSet> &resultSet)
{
    HITRACE_METER(BT_TRACE_TAG);
    // 先看有没有名字，没有名字就展示号码（如果号码配置不展示、隐藏，则显示unknown）
    std::string name = UNKNOWN_NAME;
    std::string phoneNumber = UNKNOWN_NAME;
    int32_t columnIndex = 0;
    resultSet->GetColumnIndex(DISPLAY_NAME, columnIndex);
    int32_t operationResult = resultSet->GetString(columnIndex, name);
    if (operationResult == DataShare::E_OK) {
        if (name.empty()) {
            resultSet->GetColumnIndex(PHONE_NUMBER, columnIndex);
            operationResult = resultSet->GetString(columnIndex, phoneNumber);
            names.push_back(phoneNumber);
        } else {
            names.push_back(name);
        }
    }
}

void PbabPseDataShareUtils::GetContactNameList(std::vector<std::string> &names, uint8_t orderBy)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto callDataShareHelper = GetContactDataShareHelper();
    if (callDataShareHelper == nullptr) {
        HILOGE("callDataShareHelper is nullptr.");
        return;
    }
    DataShare::DataSharePredicates predicates;
    std::string orderByWhat = CALLLOG_ID;
    if (orderBy == ORDER_BY_ALPHABETICAL) {
        orderByWhat = DISPLAY_NAME;
    }
    predicates.OrderByDesc(orderByWhat);
    Uri uri(RAW_CONTACT_DATASHARE_URI_CONTENT);
    std::vector<std::string> columns = {CALLLOG_ID, DISPLAY_NAME};
    auto resultSet = callDataShareHelper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOGE("callDataShareHelper query error, result is null");
        return;
    }

    int32_t operationResult = resultSet->GoToFirstRow();
    while (operationResult == DataShare::E_OK) {
        std::string name = "";
        int32_t rawContactId = 0;
        int32_t columnIdIndex = 0;
        int32_t columeNameIndex = 0;
        resultSet->GetColumnIndex(DISPLAY_NAME, columeNameIndex);
        resultSet->GetColumnIndex(CALLLOG_ID, columnIdIndex);
        operationResult = resultSet->GetInt(columnIdIndex, rawContactId);
        if (operationResult != DataShare::E_OK) {
            HILOGE("raw_contact_id error retry");
            resultSet->GetInt(columnIdIndex, rawContactId);
        }
        operationResult = resultSet->GetString(columeNameIndex, name);
        if (name.empty()) {
            name = UNKNOWN_NAME;
        }
        name += "," + std::to_string(rawContactId);
        names.push_back(name);
        operationResult = resultSet->GoToNextRow();
    }
}

void PbabPseDataShareUtils::GetContactNamesByNumber(const std::string &number, std::vector<std::string> &names)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::vector<int32_t> rawContactIds;
    QueryRawContactIdsByNumber(number, rawContactIds);
    for (auto rawContactId : rawContactIds) {
        QueryNameByRawContactId(rawContactId, names);
    }
}

DataShare::DataSharePredicates PbabPseDataShareUtils::GeneratePredicatesByNumberLimit(const std::string &number,
    uint32_t limitSize)
{
    HILOGI("limitSize is %{public}u. number size is %{public}u", limitSize, number.size());
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(TYPE_ID, TYPE_ID_NUMBER);
    // 长度小于limit，number全词匹配
    if (number.size() < limitSize) {
        predicates.EqualTo(DETAIL_INFO, number);
    } else {
        // 长度大于limit，endsWith查询
        std::string endNumber = number.substr(number.size() - limitSize);
        predicates.EndsWith(DETAIL_INFO, endNumber)->Or()->EndsWith(FORMAT_PHONE_NUMBER, endNumber);
    }
    return predicates;
}

void PbabPseDataShareUtils::QueryRawContactIdsByNumber(const std::string &number, std::vector<int32_t> &rawContactIds)
{
    auto callDataShareHelper = GetContactDataShareHelper();
    if (callDataShareHelper == nullptr) {
        HILOGE("callDataShareHelper is nullptr.");
        return;
    }
    Uri uri(CONTACT_DATA_DATASHARE_URI_CONTENT);
    DataShare::DataSharePredicates predicates = GeneratePredicatesByNumberLimit(number, QUERY_PHONE_NUM_LENGTH_LIMIT);
    std::vector<std::string> columns = {RAW_CONTACT_ID};
    auto resultSet = callDataShareHelper->Query(uri, predicates, columns);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "query rawContactId error, result is null");
    int32_t goToRet = resultSet->GoToFirstRow();
    if (goToRet != DataShare::E_OK) {
        HILOGE("GoToFirstRow failed, ret: %{public}d", goToRet);
        return;
    }
    int32_t rawContactId;
    if (GetValue(resultSet, RAW_CONTACT_ID, rawContactId) == DataShare::E_OK) {
        rawContactIds.push_back(rawContactId);
    }
}

void PbabPseDataShareUtils::QueryNameByRawContactId(int32_t rawContactId, std::vector<std::string> &names)
{
    auto callDataShareHelper = GetContactDataShareHelper();
    if (callDataShareHelper == nullptr) {
        HILOGE("callDataShareHelper is nullptr.");
        return;
    }
    Uri uri(CONTACT_DATA_DATASHARE_URI_CONTENT);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(TYPE_ID, TYPE_ID_NAME);
    predicates.EqualTo(RAW_CONTACT_ID, rawContactId);

    std::vector<std::string> columns = {DETAIL_INFO};
    auto resultSet = callDataShareHelper->Query(uri, predicates, columns);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "query rawContactId error, result is null");

    int32_t operationResult = resultSet->GoToFirstRow();
    std::string name;
    if (operationResult == DataShare::E_OK && GetValue(resultSet, DETAIL_INFO, name) == DataShare::E_OK) {
        name += "," + std::to_string(rawContactId);
        names.push_back(name);
    }
}
}  // namespace Bluetooth
}  // namespace OHOS