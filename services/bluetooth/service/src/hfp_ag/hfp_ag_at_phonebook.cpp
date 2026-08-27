/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hfp_ag"
#endif

#include "log.h"
#include "bt_def.h"
#include "common_util.h"
#include "service_util.h"
#include "hfp_ag_at_phonebook.h"
#include "hfp_ag_service.h"
#include "adapter_device_config.h"
#include "../dialog/bluetooth_dialog.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
const uint32_t MIN_CURRENT_PB_SIZE = 100;

constexpr const char *RAW_CONTACT_DATASHARE_URI = "datashare:///com.ohos.contactsdataability/contacts/raw_contact";
constexpr const char *CONTACT_DATA_DATASHARE_URI = "datashare:///com.ohos.contactsdataability/contacts/contact_data";
constexpr const char *CALL_LOG_DATASHARE_URI = "datashare:///com.ohos.calllogability/calls/calllog";
constexpr const char *CALL_DIRECTION = "call_direction";
constexpr const char *ANSWER_STATE = "answer_state";
constexpr const char *CONTACT_ID = "contact_id";
constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
constexpr const char *CALL_LOG_ID = "id";
constexpr const char *DISPLAY_NAME = "display_name";
constexpr const char *PHONE_NUMBER = "phone_number";
constexpr const char *DETAIL_INFO = "detail_info";
constexpr const char *TYPE_ID = "type_id";
constexpr const char *LABEL_ID = "extend7";
constexpr const char *IS_DELETED = "is_deleted";
constexpr int REQUEST_PERMISSION_TIMEOUT_MS = 30000;  // 30s
const int32_t TYPE_ID_PHONE  = 5;
const int32_t TYPE_ID_NAME = 6;

const int32_t REJECT_MAX_TIMES = 2;

const int32_t MAX_NAME_LENGTH = 28;
const int32_t MAX_NUMBER_LENGTH = 30;

const int32_t TOA_INTERNATIONAL = 0x91;
const int32_t TOA_UNKNOWN = 0x81;

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

enum PhoneType {
    PHONE_TYPE_CUSTOM = 0,   // Indicates a custom label.
    PHONE_TYPE_HOME = 1,     // Indicates a home number.
    PHONE_TYPE_MOBILE = 2,   // Indicates a mobile phone number.
    PHONE_TYPE_WORK = 3,     // Indicates a work number.
    PHONE_TYPE_FAX_WORK = 4, // Indicates a work fax number.
    PHONE_TYPE_FAX_HOME = 5, // Indicates a home fax number.
    PHONE_TYPE_PAGER = 6,    // Indicates a pager number.
    PHONE_TYPE_OTHER = 7,    // Indicates a number of the OTHER type.
};

std::string Split(const std::string &inputStr, const std::string &delimiter)
{
    std::string result = "";
    size_t pos = inputStr.find(delimiter);
    if (pos == std::string::npos) {
        return result;
    }
    result = inputStr.substr(pos + 1);
    return result;
}

HfpAgAtPhonebook::HfpAgAtPhonebook(const std::string &address) :address_(address)
{
    requestPermissionTimer_ = std::make_shared<utility::Timer>(
        std::bind(&bluetooth::HfpAgAtPhonebook::RequestPermissionTimeout, this));
}

AtCommandType HfpAgAtPhonebook::GetAtCommandType(const std::string &atCommand) const
{
    AtCommandType ret = AtCommandType::TYPE_UNKNOWN;
    if (atCommand.size() <= AT_COMMAND_LENGTH_FIVE) {
        return ret;
    }
    std::string subStr = atCommand.substr(AT_COMMAND_LENGTH_FIVE);
    if (StartWith(subStr, "?")) {
        ret = AtCommandType::TYPE_READ;
    } else if (StartWith(subStr, "=?")) {
        ret = AtCommandType::TYPE_TEST;
    } else if (StartWith(subStr, "=")) {
        ret = AtCommandType::TYPE_SET;
    } else {
        HILOGE("unknowType, atCommand = %{public}s", atCommand.c_str());
    }
    return ret;
}

void HfpAgAtPhonebook::HandleCscsCommand(const std::string &atCommand)
{
    std::string atCommandResponse = "";
    int32_t errorCode = 0;
    int32_t atCommandResult = HFP_AG_RESULT_ERROR;
    AtCommandType commandType = GetAtCommandType(atCommand);
    switch (commandType) {
        case AtCommandType::TYPE_READ: {
            atCommandResponse = "+CSCS: \"" + characterSet_ + "\"";
            atCommandResult = HFP_AG_RESULT_OK;
            break;
        }
        case AtCommandType::TYPE_TEST: {
            atCommandResponse = "+CSCS: (\"UTF-8\",\"IRA\",\"GSM\")";
            atCommandResult = HFP_AG_RESULT_OK;
            break;
        }
        case AtCommandType::TYPE_SET: {
            atCommandResult = SetCharacterSet(atCommand, errorCode);
            break;
        }
        default:
            errorCode = HFP_AG_ERROR_INVALID_CHARS_IN_TEXT_STRING;
    }
    if (!atCommandResponse.empty()) {
        ProcessAtResponseStringEvent(atCommandResponse);
    }
    ProcessAtResponseCodeEvent(atCommandResult, errorCode);
}

void HfpAgAtPhonebook::HandleCpbsCommand(const std::string &atCommand)
{
    std::string atCommandResponse = "";
    int32_t errorCode = 0;
    int32_t atCommandResult = HFP_AG_RESULT_ERROR;
    AtCommandType commandType = GetAtCommandType(atCommand);
    switch (commandType) {
        case AtCommandType::TYPE_READ: {
            atCommandResult = ReadCurrentPhonebookStorage(atCommandResponse);
            break;
        }
        case AtCommandType::TYPE_TEST: {
            atCommandResponse = "+CPBS: (\"ME\",\"SM\",\"DC\",\"RC\",\"MC\")";
            atCommandResult = HFP_AG_RESULT_OK;
            break;
        }
        case AtCommandType::TYPE_SET: {
            atCommandResult = SetCurrentPhonebookStorage(atCommand, errorCode);
            break;
        }
        default:
            errorCode = HFP_AG_ERROR_INVALID_CHARS_IN_TEXT_STRING;
    }
    if (!atCommandResponse.empty()) {
        ProcessAtResponseStringEvent(atCommandResponse);
    }
    ProcessAtResponseCodeEvent(atCommandResult, errorCode);
}

void HfpAgAtPhonebook::HandleCpbrCommand(const std::string &atCommand)
{
    int32_t atCommandResult = HFP_AG_RESULT_ERROR;
    std::string atCommandResponse = "";
    int32_t errorCode = 0;
    bool isRequestPermission = false;
    AtCommandType commandType = GetAtCommandType(atCommand);
    switch (commandType) {
        case AtCommandType::TYPE_TEST: {
            atCommandResult = ReadCurrentPhonebookInfo(atCommandResponse, errorCode);
            break;
        }
        case AtCommandType::TYPE_READ:
        case AtCommandType::TYPE_SET: {
            atCommandResult = ReadCurrentPhonebookEntries(atCommand, errorCode, isRequestPermission);
            break;
        }
        default:
            errorCode = HFP_AG_ERROR_INVALID_CHARS_IN_TEXT_STRING;
    }
    if (isRequestPermission) {
        HILOGI("address: %{public}s is request permission", GET_ENCRYPT_STR_ADDR(address_));
        return;
    }
    if (!atCommandResponse.empty()) {
        ProcessAtResponseStringEvent(atCommandResponse);
    }
    ProcessAtResponseCodeEvent(atCommandResult, errorCode);
}

void HfpAgAtPhonebook::HandlePbAccessResult(const std::string &addr, int32_t access, int32_t pbapLoadFlag)
{
    int32_t errorCode = 0;
    int32_t atCommandResult = HFP_AG_RESULT_ERROR;
    if (access == static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN) && !pbapLoadFlag) {
        int32_t rejectCount = 0;
        bool getResult = GetPbapRejectCount(addr, rejectCount);
        if (getResult) {
            rejectCount++;
        } else {
            rejectCount = 1;
        }
        HILOGI("addr: %{public}s, rejectCount: %{public}d", GET_ENCRYPT_STR_ADDR(addr), rejectCount);
        bool setResult = SetPbapRejectCount(addr, rejectCount);
        if (!setResult) {
            HILOGE("addr: %{public}s SetPbapRejectCount fail", GET_ENCRYPT_STR_ADDR(addr));
        }
    }
    if (access == static_cast<int>(BTPermissionType::ACCESS_ALLOWED)) {
        atCommandResult = ProcessCpbrCommand();
    }
    ResetCpbrIndex();
    SetCheckingAccessPermission(false);
    ProcessAtResponseCodeEvent(atCommandResult, errorCode);
}

void HfpAgAtPhonebook::NotifyNewAtCommandRecv()
{
    if (IsCheckingAccessPermission()) {
        HILOGI("addr: %{public}s device recv new AT command, reset flag", GET_ENCRYPT_STR_ADDR(address_));
        SetCheckingAccessPermission(false);
        ResetCpbrIndex();
    }
}

int32_t HfpAgAtPhonebook::SetCharacterSet(const std::string &atCommand, int32_t &errorCode)
{
    std::string tmpStr = Split(atCommand, "=");
    std::string characterSet = FormatStr(tmpStr);
    HILOGD("characterSet = %{public}s", characterSet.c_str());
    if (characterSet.empty()) {
        errorCode = HFP_AG_ERROR_OPERATION_NOT_SUPPORTED;
        return HFP_AG_RESULT_ERROR;
    }

    if (characterSet != "GSM" && characterSet != "IRA" && characterSet != "UTF-8" && characterSet != "UTF8") {
        errorCode = HFP_AG_ERROR_OPERATION_NOT_SUPPORTED;
        HILOGE("characterSet = %{public}s", characterSet.c_str());
        return HFP_AG_RESULT_ERROR;
    }
    characterSet_ = characterSet;
    return HFP_AG_RESULT_OK;
}

int32_t HfpAgAtPhonebook::ReadCurrentPhonebookStorage(std::string &atCommandResponse)
{
    if (currentPhonebook_ == "SM") {
        atCommandResponse = "+CPBS: \"SM\",0," + std::to_string(GetMaxPhoneBookSize(0));
        return HFP_AG_RESULT_OK;
    }
    uint32_t count = GetPhonebookOrCalllogCount();
    atCommandResponse = "+CPBS: \"" + currentPhonebook_ + "\","
        + std::to_string(count) + "," + std::to_string(GetMaxPhoneBookSize(0));
    return HFP_AG_RESULT_OK;
}

int32_t HfpAgAtPhonebook::SetCurrentPhonebookStorage(const std::string &atCommand, int32_t &errorCode)
{
    std::string tmpStr = Split(atCommand, "=");
    std::string currentPhonebook = FormatStr(tmpStr);
    HILOGD("currentPhonebook = %{public}s", currentPhonebook.c_str());
    if (currentPhonebook.empty()) {
        errorCode = HFP_AG_ERROR_OPERATION_NOT_SUPPORTED;
        return HFP_AG_RESULT_ERROR;
    }
    if (!CheckPhonebook(currentPhonebook) && (currentPhonebook != "SM")) {
        errorCode = HFP_AG_ERROR_OPERATION_NOT_ALLOWED;
        return HFP_AG_RESULT_ERROR;
    }
    currentPhonebook_ = currentPhonebook;
    return HFP_AG_RESULT_OK;
}

int32_t HfpAgAtPhonebook::ReadCurrentPhonebookInfo(std::string &atCommandResponse, int32_t &errorCode)
{
    uint32_t size = 0;
    if (currentPhonebook_ == "SM") {
        size = 0;
    } else {
        if (!CheckPhonebook(currentPhonebook_)) {
            errorCode = HFP_AG_ERROR_OPERATION_NOT_ALLOWED;
            return HFP_AG_RESULT_ERROR;
        }
        size = GetPhonebookOrCalllogCount();
    }
    if (size == 0) {
        /* Sending "+CPBR: (1-0)" can confused some carkits, send "1-1" * instead */
        size = 1;
    }
    atCommandResponse = "+CPBR: (1-" + std::to_string(size) + "),30,30";
    return HFP_AG_RESULT_OK;
}

int32_t HfpAgAtPhonebook::ReadCurrentPhonebookEntries(const std::string &atCommand, int32_t &errorCode,
    bool &isRequestPermission)
{
    if (cpbrIndex1_ != -1) {
        errorCode = HFP_AG_ERROR_OPERATION_NOT_ALLOWED;
        return HFP_AG_RESULT_ERROR;
    }
    if (!ParseCpbrIndex(atCommand)) {
        errorCode = HFP_AG_ERROR_INVALID_CHARS_IN_TEXT_STRING;
        return HFP_AG_RESULT_ERROR;
    }
    return ProcessPbAccessPermission(isRequestPermission);
}

bool HfpAgAtPhonebook::CheckPhonebook(const std::string &pb) const
{
    if (pb != "ME" && pb != "DC" && pb != "RC" && pb != "MC") {
        HILOGE("unknown pb = %{public}s", pb.c_str());
        return false;
    }
    return true;
}

uint32_t HfpAgAtPhonebook::GetMaxPhoneBookSize(uint32_t currSize)
{
    uint32_t maxSize = (currSize < MIN_CURRENT_PB_SIZE) ? MIN_CURRENT_PB_SIZE : currSize;
    maxSize += maxSize / 2; // maxSize = currSize + currSize / 2 rounded up to nearest power of 2

    uint32_t maxPbSize = 1;
    while (maxPbSize < maxSize) {
        maxPbSize = maxPbSize << 1;
    }
    return maxPbSize;
}

uint32_t HfpAgAtPhonebook::GetPhonebookOrCalllogCount()
{
    int8_t pbType = GetPhonebookType();
    CHECK_AND_RETURN_LOG_RET(pbType != 0, 0, "pbType is wrong.");

    return GetPhonebookCount(pbType);
}

int8_t HfpAgAtPhonebook::GetPhonebookType()
{
    int8_t pbType = 0;
    if (currentPhonebook_ == "ME") {
        pbType = PhonebookType::PHONEBOOK;
    } else if (currentPhonebook_ == "RC") {
        pbType = PhonebookType::INCOMING_CALL_LOG;
    } else if (currentPhonebook_ == "DC") {
        pbType = PhonebookType::OUTGOING_CALL_LOG;
    } else if (currentPhonebook_ == "MC") {
        pbType = PhonebookType::MISSED_CALL_LOG;
    } else {
        HILOGE("error currentPhonebook = %{public}s", currentPhonebook_.c_str());
    }
    return pbType;
}

bool HfpAgAtPhonebook::ParseCpbrIndex(const std::string &atCommand)
{
    std::string command = Split(atCommand, "=");
    if (command.empty()) {
        return false;
    }

    std::vector<std::string> result {};
    size_t pos = command.find(",");
    while (pos != std::string::npos) {
        std::string tmpstr = command.substr(0, pos);
        result.push_back(tmpstr);
        command.erase(0, pos + 1);
        pos = command.find(",");
    }
    result.push_back(command);

    int32_t index1;
    int32_t index2;
    if (result.size() == 0) {
        return false;
    }
    if (!ConvertStrToDigit(result[0], index1)) {
        return false;
    }

    if (result.size() == 1) {
        cpbrIndex1_ = index1;
        cpbrIndex2_ = index1;
        return true;
    }
    if (!ConvertStrToDigit(result[1], index2)) {
        return false;
    }
    cpbrIndex1_ = index1;
    cpbrIndex2_ = index2;
    HILOGI("Parse Index, cpbrIndex1_ = %{public}d, cpbrIndex2_ = %{public}d", cpbrIndex1_, cpbrIndex2_);
    return true;
}

std::string HfpAgAtPhonebook::FormatStr(const std::string& str)
{
    std::string retStr = "";
    for (char ch : str) {
        if (ch == '\"') {
            continue;
        }
        retStr += ch;
    }
    return retStr;
}

void HfpAgAtPhonebook::ResetCpbrIndex()
{
    cpbrIndex1_ = -1;
    cpbrIndex2_ = -1;
}

bool HfpAgAtPhonebook::IsCheckingAccessPermission()
{
    return checkingAccessPermission_;
}

void HfpAgAtPhonebook::SetCheckingAccessPermission(bool checkingFlag)
{
    checkingAccessPermission_ = checkingFlag;
}

int32_t HfpAgAtPhonebook::ProcessPbAccessPermission(bool &isRequestPermission)
{
    SetCheckingAccessPermission(true);
    int32_t permission = CheckAccessPermission(address_);
    if (permission == static_cast<int32_t>(BTPermissionType::ACCESS_ALLOWED)) {
        SetCheckingAccessPermission(false);
        int32_t atCommandResult = ProcessCpbrCommand();
        ResetCpbrIndex();
        return atCommandResult;
    }
    if (permission == static_cast<int32_t>(BTPermissionType::ACCESS_FORBIDDEN)) {
        SetCheckingAccessPermission(false);
        ResetCpbrIndex();
        return HFP_AG_RESULT_ERROR;
    }
    isRequestPermission = true;
    return HFP_AG_RESULT_OK;
}

int32_t HfpAgAtPhonebook::ProcessCpbrCommand()
{
    if (currentPhonebook_ == "SM") {
        return HFP_AG_RESULT_OK;
    }
    if (!CheckPhonebook(currentPhonebook_)) {
        return HFP_AG_RESULT_ERROR;
    }

    int8_t pbType = GetPhonebookType();
    if (pbType == 0) {
        return HFP_AG_RESULT_OK;
    }

    int32_t count = GetPhonebookCount(pbType);
    if (count == 0) {
        HILOGE("count is 0.");
        return HFP_AG_RESULT_OK;
    }
    HILOGI("cpbrIndex1_ = %{public}d, cpbrIndex2_ = %{public}d, count = %{public}d", cpbrIndex1_, cpbrIndex2_, count);
    if (cpbrIndex1_ <= 0 || cpbrIndex2_ < cpbrIndex1_ || cpbrIndex1_ > count) {
        return HFP_AG_RESULT_OK;
    }
    if (cpbrIndex2_ > count) {
        cpbrIndex2_ = count;
    }
    int32_t offset = cpbrIndex1_ > 0 ? cpbrIndex1_ - 1 : cpbrIndex1_;
    int32_t number = cpbrIndex2_ - offset;

    if (pbType == PhonebookType::PHONEBOOK) {
        return ComposeContactInfo(offset, number);
    }
    return ComposeCallLogInfo(pbType, offset, number);
}

int32_t HfpAgAtPhonebook::ComposeContactInfo(int32_t offset, int32_t number)
{
    DataShare::DataSharePredicates predicates;
    predicates.Limit(number, offset);
    predicates.EqualTo(IS_DELETED, 0);
    auto resultSet = QueryContactId(predicates);
    CHECK_AND_RETURN_LOG_RET(resultSet != nullptr, HFP_AG_RESULT_ERROR, "dataShare query error, result is null.");
    int32_t opRet = resultSet->GoToFirstRow();

    std::shared_ptr<DataShare::DataShareHelper> contactDataShare = CreateDataShareHelper(CONTACT_DATA_DATASHARE_URI);
    CHECK_AND_RETURN_LOG_RET(contactDataShare != nullptr, HFP_AG_RESULT_ERROR, "dataShare is nullptr.");

    int32_t index = cpbrIndex1_;
    while (opRet == DataShare::E_OK) {
        int32_t contactId;
        GetValue(resultSet, CONTACT_ID, contactId, -1);
        if (contactId != -1) {
            ComposeAndSendOneContactInfo(contactDataShare, contactId, index);
            index++;
        }
        opRet = resultSet->GoToNextRow();
    }
    contactDataShare->Release();
    return HFP_AG_RESULT_OK;
}

void HfpAgAtPhonebook::ComposeAndSendOneContactInfo(std::shared_ptr<DataShare::DataShareHelper> datashare,
    int32_t contactId, int32_t index)
{
    std::string name = "";
    std::string number = "";
    std::string labelId = "";
    GetContactInfoByTypeId(datashare, contactId, TYPE_ID_NAME, name, labelId);
    GetContactInfoByTypeId(datashare, contactId, TYPE_ID_PHONE, number, labelId);

    std::string regionType = std::to_string(ToaFromNumber(number));
    std::string record = "+CPBR: " + std::to_string(index) + ",\"" + FormatNumberInfo(number) + "\"," +
        regionType + ",\"" + FormatNameInfo(name, labelId, PhonebookType::PHONEBOOK) + "\"";
    ProcessAtResponseStringEvent(record);
}

void HfpAgAtPhonebook::GetContactInfoByTypeId(std::shared_ptr<DataShare::DataShareHelper> datashare,
    int32_t rawContactId, int32_t typeId, std::string &outInfo, std::string &labelId)
{
    CHECK_AND_RETURN_LOG(datashare != nullptr, "dataShare is nullptr.");
    std::string defaultValue = "";
    std::vector<std::string> columns = { DETAIL_INFO };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(RAW_CONTACT_ID, rawContactId)->And()->EqualTo(TYPE_ID, typeId);
    Uri uri(CONTACT_DATA_DATASHARE_URI);

    auto result = datashare->Query(uri, predicates, columns);
    CHECK_AND_RETURN_LOG(result != nullptr, "dataShare query error, result is null.");
    int32_t opRet = result->GoToFirstRow();
    CHECK_AND_RETURN_LOG(opRet == DataShare::E_OK, "GoToFirstRow fail.");

    GetValue(result, DETAIL_INFO, outInfo, defaultValue);
    if (typeId == TYPE_ID_PHONE) {
        GetValue(result, LABEL_ID, labelId, defaultValue);
    }
}

std::shared_ptr<DataShare::DataShareResultSet> HfpAgAtPhonebook::QueryContactId(
    const DataShare::DataSharePredicates &predicates)
{
    std::vector<std::string> columns = {CONTACT_ID};
    std::shared_ptr<DataShare::DataShareHelper> dataShare = CreateDataShareHelper(RAW_CONTACT_DATASHARE_URI);
    CHECK_AND_RETURN_LOG_RET(dataShare != nullptr, nullptr, "dataShare is nullptr.");
    Uri uri(RAW_CONTACT_DATASHARE_URI);
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = dataShare->Query(uri, predicates, columns);
    dataShare->Release();
    return resultSet;
}

int32_t HfpAgAtPhonebook::ComposeCallLogInfo(int8_t pbType, int32_t offset, int32_t number)
{
    DataShare::DataSharePredicates predicates;
    GenerateCallLogPredicates(pbType, predicates);
    predicates.Limit(number, offset);
    predicates.OrderByDesc(CALL_LOG_ID);

    std::shared_ptr<DataShare::DataShareHelper> dataShare = CreateDataShareHelper(CALL_LOG_DATASHARE_URI);
    CHECK_AND_RETURN_LOG_RET(dataShare != nullptr, HFP_AG_RESULT_ERROR, "dataShare is nullptr.");

    std::vector<std::string> columns;
    Uri uri(CALL_LOG_DATASHARE_URI);
    auto resultSet = dataShare->Query(uri, predicates, columns);
    CHECK_AND_RETURN_LOG_RET(resultSet != nullptr, HFP_AG_RESULT_ERROR, "dataShare query error, result is null.");
    int32_t opRet = resultSet->GoToFirstRow();

    int32_t index = cpbrIndex1_;
    while (opRet == DataShare::E_OK) {
        ComposeAndSendOneCallLogInfo(resultSet, pbType, index);
        index++;
        opRet = resultSet->GoToNextRow();
    }
    dataShare->Release();
    return HFP_AG_RESULT_OK;
}

void HfpAgAtPhonebook::ComposeAndSendOneCallLogInfo(std::shared_ptr<DataShare::DataShareResultSet> resultSet,
    int8_t pbType, int32_t index)
{
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "resultSet is null.");
    std::string name = "";
    std::string number = "";
    std::string defaultValue = "";
    std::string labelId = "";
    GetValue(resultSet, DISPLAY_NAME, name, defaultValue);
    GetValue(resultSet, PHONE_NUMBER, number, defaultValue);

    std::string regionType = std::to_string(ToaFromNumber(number));
    std::string record = "+CPBR: " + std::to_string(index) + ",\"" + FormatNumberInfo(number) + "\"," +
        regionType + ",\"" + FormatNameInfo(name, labelId, pbType) + "\"";
    ProcessAtResponseStringEvent(record);
}

std::string HfpAgAtPhonebook::FormatNameInfo(const std::string& name, std::string &labelId, int8_t pbType)
{
    std::string retStr = name;
    if (name.length() > MAX_NAME_LENGTH) {
        retStr = name.substr(0, MAX_NAME_LENGTH);
    }
    if (pbType == PhonebookType::PHONEBOOK) {
        int32_t type;
        HILOGI("labelId: %{public}s", labelId.c_str());
        retStr += "/";
        if (ConvertStrToDigit(labelId, type)) {
            retStr += GetPhoneType(type);
        } else {
            retStr += "M";
        }
    }
    return retStr;
}

std::string HfpAgAtPhonebook::FormatNumberInfo(const std::string& number)
{
    std::string retStr = number;
    if (number.length() > MAX_NUMBER_LENGTH) {
        retStr = number.substr(0, MAX_NUMBER_LENGTH);
    }
    return retStr;
}

int32_t HfpAgAtPhonebook::ToaFromNumber(const std::string& number)
{
    if (number.length() > 0 && number.at(0) == '+') {
        return TOA_INTERNATIONAL;
    }
    return TOA_UNKNOWN;
}

std::string HfpAgAtPhonebook::GetPhoneType(int32_t type)
{
    switch (type) {
        case PHONE_TYPE_HOME:
            return "H";
        case PHONE_TYPE_MOBILE:
            return "M";
        case PHONE_TYPE_WORK:
            return "W";
        case PHONE_TYPE_FAX_WORK:
        case PHONE_TYPE_FAX_HOME:
            return "F";
        case PHONE_TYPE_OTHER:
        case PHONE_TYPE_CUSTOM:
        default:
            return "O";
    }
}

int32_t HfpAgAtPhonebook::CheckAccessPermission(const std::string &addr)
{
    int32_t permission = static_cast<int32_t>(BTPermissionType::ACCESS_FORBIDDEN);
    GetPbapPermission(addr, permission);

    if (IsNeedSetPermissionToUnknown(addr, permission)) {
        permission = static_cast<int32_t>(BTPermissionType::ACCESS_UNKNOWN);
        permissionAccessRequested_ = true;
    }

    if (permission == static_cast<int32_t>(BTPermissionType::ACCESS_UNKNOWN)) {
        DialogInfo dialog{addr, PBAP_AUTH_DIALOG, REQUEST_PERMISSION_TIMEOUT_MS, requestPermissionTimer_};
        BluetoothDialog::RequestAuthDialog(dialog);
    }
    return permission;
}

bool HfpAgAtPhonebook::IsNeedSetPermissionToUnknown(const std::string &address, int32_t permission)
{
    if (permission != static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)) {
        return false;
    }
    if (permissionAccessRequested_) {
        return false;
    }

    int32_t rejectCount = 0;
    bool getResult = GetPbapRejectCount(address, rejectCount);
    if (!getResult) {
        return true;
    }
    HILOGD("rejectCount %{public}d", rejectCount);
    return rejectCount < REJECT_MAX_TIMES;
}

std::shared_ptr<DataShare::DataShareHelper> HfpAgAtPhonebook::CreateDataShareHelper(const std::string &uriStr)
{
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET(saManager != nullptr, nullptr, "GetSystemAbilityManager failed.");

    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG_RET(remoteObj != nullptr, nullptr, "GetSystemAbility Service Failed.");
    return DataShare::DataShareHelper::Creator(remoteObj, uriStr);
}

void HfpAgAtPhonebook::GenerateCallLogPredicates(int8_t pbType, DataShare::DataSharePredicates &predicates)
{
    if (pbType == PhonebookType::INCOMING_CALL_LOG) {
        predicates.EqualTo(CALL_DIRECTION, CallDirection::INCOMING_TYPE)->And()
            ->EqualTo(ANSWER_STATE, AnswerState::CALL_ANSWER_ACTIVED);
    } else if (pbType == OUTGOING_CALL_LOG) {
        predicates.EqualTo(CALL_DIRECTION, CallDirection::OUTGOING_TYPE);
    } else if (pbType == MISSED_CALL_LOG) {
        predicates.EqualTo(CALL_DIRECTION, CallDirection::INCOMING_TYPE)->And()
            ->NotEqualTo(ANSWER_STATE, AnswerState::CALL_ANSWER_ACTIVED);
    } else if (pbType == COMBINED_CALL_LOG) {
        HILOGD("COMBINED_CALL, no need filter");
    } else {
        HILOGE("pbType is error, pbType: %{public}d", pbType);
    }
}

int32_t HfpAgAtPhonebook::GetPhonebookCount(int8_t pbType)
{
    DataShare::DataSharePredicates predicates;
    std::string uriStr;
    std::vector<std::string> columns {};
    if (pbType == PhonebookType::PHONEBOOK) {
        uriStr = RAW_CONTACT_DATASHARE_URI;
        columns = {CONTACT_ID};
        predicates.EqualTo(IS_DELETED, 0);
    } else {
        uriStr = CALL_LOG_DATASHARE_URI;
        GenerateCallLogPredicates(pbType, predicates);
        columns = {CALL_LOG_ID};
    }

    std::shared_ptr<DataShare::DataShareHelper> dataShare = CreateDataShareHelper(uriStr);
    CHECK_AND_RETURN_LOG_RET(dataShare != nullptr, 0, "dataShare is nullptr.");

    Uri uri(uriStr);
    auto result = dataShare->Query(uri, predicates, columns);
    CHECK_AND_RETURN_LOG_RET(result != nullptr, 0, "dataShare query error, result is null.");

    int32_t retCount = 0;
    result->GetRowCount(retCount);
    dataShare->Release();
    HILOGD("pbType: %{public}d, retCount: %{public}d", pbType, retCount);
    return retCount;
}

void HfpAgAtPhonebook::GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    const std::string &fieldName, std::string& out, std::string &defaultValue)
{
    int index = 0;
    if (resultSet->GetColumnIndex(fieldName, index) != DataShare::E_OK) {
        HILOGE("GetColumnIndex [%{public}s] failed", fieldName.c_str());
        out = defaultValue;
    }
    if (resultSet->GetString(index, out) != DataShare::E_OK) {
        HILOGE("GetString [%{public}s] failed", fieldName.c_str());
        out = defaultValue;
    }
}

void HfpAgAtPhonebook::GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet, const std::string &fieldName,
    int32_t &out, int32_t defaultValue)
{
    int32_t index = 0;
    if (resultSet->GetColumnIndex(fieldName, index) != DataShare::E_OK) {
        HILOGE("GetColumnIndex [%{public}s] failed", fieldName.c_str());
        out = defaultValue;
    }
    if (resultSet->GetInt(index, out) != DataShare::E_OK) {
        HILOGE("GetString [%{public}s] failed", fieldName.c_str());
        out = defaultValue;
    }
}

void HfpAgAtPhonebook::ProcessAtResponseCodeEvent(int responseCode, int errorCode) const
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("BluetoothHfpInterface is null");
        return;
    }
    HILOGI("Hfp device address[%{public}s]", GET_ENCRYPT_STR_ADDR(address_));
    bt_status_t result = bluetoothHfpInterface->AtResponse(
        static_cast<::bluetooth::headset::bthf_at_response_t>(responseCode), errorCode, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("Failed AtResponseCode, status: %{public}d", result);
        return;
    }
}

void HfpAgAtPhonebook::ProcessAtResponseStringEvent(const std::string &response) const
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("BluetoothHfpInterface is null");
        return;
    }
    HILOGI("Hfp device address[%{public}s]", GET_ENCRYPT_STR_ADDR(address_));
    bt_status_t result = bluetoothHfpInterface->FormattedAtResponse(response.c_str(), &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("Failed AtResponseString, status: %{public}d", result);
        return;
    }
}

void HfpAgAtPhonebook::RequestPermissionTimeout()
{
    HITRACE_METER(BT_TRACE_TAG);
    SetCheckingAccessPermission(false);
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, HFP_AG_ERROR_AG_FAILURE);
    BluetoothDialog::DismissCurAndShowNext();
}

void HfpAgAtPhonebook::StopRequestPermissionTimer()
{
    HILOGI("Stop RequestPermission timer!");
    CHECK_AND_RETURN_LOG((requestPermissionTimer_ != nullptr), "requestPermissionTimer is null");
    requestPermissionTimer_->Stop();
}
}  // namespace bluetooth
}  // namespace OHOS