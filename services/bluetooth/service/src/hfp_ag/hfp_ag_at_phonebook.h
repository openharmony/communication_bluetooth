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

#ifndef HFP_AG_AT_PHONEBOOK_H
#define HFP_AG_AT_PHONEBOOK_H

#include <string>
#include "hfp_ag_defines.h"
#include "btcommon/timer_manager.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {

enum PhonebookType {
    PHONEBOOK = 1,
    INCOMING_CALL_LOG = 2,
    OUTGOING_CALL_LOG = 3,
    MISSED_CALL_LOG = 4,
    COMBINED_CALL_LOG = 5,
    FAVORITES = 6,
};

class HfpAgAtPhonebook {
public:
    explicit HfpAgAtPhonebook(const std::string &address);
    ~HfpAgAtPhonebook() {};
    void HandleCscsCommand(const std::string &atCommand);
    void HandleCpbsCommand(const std::string &atCommand);
    void HandleCpbrCommand(const std::string &atCommand);
    void HandlePbAccessResult(const std::string &addr, int32_t access, int32_t pbapLoadFlag);

    bool IsCheckingAccessPermission();
    void SetCheckingAccessPermission(bool checkingFlag);
    void ResetCpbrIndex();
    void NotifyNewAtCommandRecv();
    void StopRequestPermissionTimer();
private:
    int32_t ProcessCpbrCommand();
    AtCommandType GetAtCommandType(const std::string &atCommand) const;
    int32_t SetCharacterSet(const std::string &atCommand, int32_t &errorCode);
    bool CheckPhonebook(const std::string &pb) const;
    int32_t ReadCurrentPhonebookStorage(std::string &atCommandResponse);
    int32_t SetCurrentPhonebookStorage(const std::string &atCommand, int32_t &errorCode);
    int32_t ReadCurrentPhonebookInfo(std::string &atCommandResponse, int32_t &errorCode);
    int32_t ReadCurrentPhonebookEntries(const std::string &atCommand, int32_t &errorCode, bool &isRequestPermission);

    // some car kits ignore the current size and request max phone book size entries.
    // Thus, it takes a long time to transfer all the entries.
    // Use a heuristic to calculate the max phone book size considering future expansion.
    uint32_t GetMaxPhoneBookSize(uint32_t currSize);
    uint32_t GetPhonebookOrCalllogCount();
    bool ParseCpbrIndex(const std::string &atCommand);
    int32_t ProcessPbAccessPermission(bool &isRequestPermission);
    int32_t CheckAccessPermission(const std::string &addr);
    bool IsNeedSetPermissionToUnknown(const std::string &address, int32_t permission);
    int8_t GetPhonebookType();

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(const std::string &uriStr);
    void GenerateCallLogPredicates(int8_t pbType, DataShare::DataSharePredicates &predicates);
    int32_t GetPhonebookCount(int8_t pbType);

    int32_t ComposeContactInfo(int32_t offset, int32_t number);
    void ComposeAndSendOneContactInfo(std::shared_ptr<DataShare::DataShareHelper> datashare,
        int32_t contactId, int32_t index);
    std::shared_ptr<DataShare::DataShareResultSet> QueryContactId(const DataShare::DataSharePredicates &predicates);
    int32_t ComposeCallLogInfo(int8_t pbType, int32_t offset, int32_t number);
    void ComposeAndSendOneCallLogInfo(std::shared_ptr<DataShare::DataShareResultSet> resultSet, int8_t pbType,
        int32_t index);
    void GetContactInfoByTypeId(std::shared_ptr<DataShare::DataShareHelper> datashare, int32_t rawContactId,
        int32_t typeId, std::string &outInfo, std::string &labelId);
    std::string GetPhoneType(int32_t type);
    void GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet, const std::string &fieldName,
        std::string& out, std::string &defaultValue);
    void GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet, const std::string &fieldName,
        int32_t &out, int32_t defaultValue);

    std::string FormatNameInfo(const std::string& name, std::string &labelId, int8_t pbType);
    std::string FormatNumberInfo(const std::string& number);
    int32_t ToaFromNumber(const std::string& number);
    std::string FormatStr(const std::string& str);

    void ProcessAtResponseCodeEvent(int responseCode, int errorCode) const;
    void ProcessAtResponseStringEvent(const std::string &response) const;
    void RequestPermissionTimeout();
private:
    std::string characterSet_ = "UTF-8";
    std::string currentPhonebook_ = "ME"; // default to mobile phonebook
    int32_t cpbrIndex1_ = -1;
    int32_t cpbrIndex2_ = -1;
    bool checkingAccessPermission_ = false;
    // The address of the bluetooth device.
    std::string address_ = "";
    bool permissionAccessRequested_ = false;
    std::shared_ptr<utility::Timer> requestPermissionTimer_ = nullptr;
};
} // namespace bluetooth
} // OHOS
#endif // HFP_AG_AT_PHONEBOOK_H