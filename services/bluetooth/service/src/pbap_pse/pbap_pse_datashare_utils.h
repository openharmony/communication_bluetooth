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

#ifndef PBAP_PSE_DATASHARE_UTILS
#define PBAP_PSE_DATASHARE_UTILS

#include <memory>
#include <utility>
#include <vector>
#include "bluetooth_common_event_subscriber.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datashare_helper.h"
#include "iservice_registry.h"
#include "pbap_pse_def.h"
#include "system_ability_definition.h"
#ifdef BLUETOOTH_OS_ACCOUNT
#include "os_account_manager.h"
#endif
#include "uri.h"

namespace OHOS {
namespace bluetooth {
const int32_t QUERY_TABLE_TYPE_CONTACT = 0;
const int32_t QUERY_TABLE_TYPE_CALLLOG = 1;

class PbabPseDataShareUtils : public std::enable_shared_from_this<PbabPseDataShareUtils> {
public:
    explicit PbabPseDataShareUtils();
    ~PbabPseDataShareUtils();

    DataShare::DataSharePredicates CreatContactPredicates(int32_t number, int32_t offset, std::string &orderByStr,
        int32_t shareType);
    std::string ComposeContactVcard(int32_t vcardType, DataShare::DataSharePredicates &predicates);
    void GetCallHistoryList(std::vector<std::string> &names, int32_t contentType);
    void GetContactNameList(std::vector<std::string> &names, uint8_t orderBy);
    int32_t GetContactsSize(int32_t contentType);
    int32_t GetCallHistorySize(int32_t contentType);
    int32_t GetNewMissedCallNum();
    int32_t GetCallHistorySizeByTime(int32_t contentType, int64_t timeStamp);
    std::string ComposeCallLogVcard(int32_t contentType, int32_t vcardType, int32_t number, int32_t offset);
    void GetContactNamesByNumber(const std::string &number, std::vector<std::string> &names);
    std::string ComposeOwnerVcard(int32_t vcardType, std::string &name, std::string &number);
    void ReleaseCallLogDataShareHelper();
    void ReleaseContactDataShareHelper();

private:
    void GenerateCallLogParamByType(int32_t contentType, DataShare::DataSharePredicates &predicates);
    int32_t GetQueryResultCount(int32_t queryTableType, std::string &url, DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns);
    std::string CreateOneCallLog(int32_t vcardType, std::shared_ptr<DataShare::DataShareResultSet> &result);
    int32_t GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
        const std::string &fieldName, std::string& out);
    int32_t GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
        const std::string &fieldName, int64_t &out);
    int32_t GetValue(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
        const std::string &fieldName, int32_t &out);
    std::string GetCallLogTimePara(std::shared_ptr<DataShare::DataShareResultSet> &result);
    std::string ToRfc2455Format(std::shared_ptr<DataShare::DataShareResultSet> &result);
    void GetNameListBySet(std::vector<std::string> &names, std::shared_ptr<DataShare::DataShareResultSet> &resultSet);
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(const std::string &uriStr);
    void QueryRawContactIdsByNumber(const std::string &number, std::vector<int32_t> &rawContactIds);
    void QueryNameByRawContactId(int32_t rawContactId, std::vector<std::string> &names);
    std::shared_ptr<DataShare::DataShareHelper> GetCallLogDataShareHelper(std::string &url);
    std::shared_ptr<DataShare::DataShareHelper> GetContactDataShareHelper();
    void SubscribeUserLockStateChangeCommonEvent();
    DataShare::DataSharePredicates GeneratePredicatesByNumberLimit(const std::string &number, uint32_t limitSize);

private:
    std::mutex mCallDataShareHelperMutex_ {};
    std::shared_ptr<DataShare::DataShareHelper> mCallDataShareHelper_ = nullptr;

    std::mutex mCallLogDataShareHelperMutex_ {};
    std::shared_ptr<DataShare::DataShareHelper> mCallLogDataShareHelper_ = nullptr;

    int32_t userId_ = 0;
    bool isUserUnlocked_ = false;
    std::string lastUrl_ = "";
};
} // namespace bluetooth
} // namespace OHOS

#endif // PBAP_PSE_DATASHARE_UTILS