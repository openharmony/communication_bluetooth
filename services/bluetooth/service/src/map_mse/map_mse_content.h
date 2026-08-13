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

#ifndef MAP_MSE_CONTENT_H
#define MAP_MSE_CONTENT_H

#include <string>
#include <memory>
#include "map_mse_appparams.h"
#include "map_mse_bmessage.h"
#include "map_mse_folder.h"
#include "map_mse_message_element.h"
#include "datashare_helper.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {
const std::string MAP_V10_STR = "1.0";
const std::string MAP_V11_STR = "1.1";
const std::string MAP_V12_STR = "1.2";

const uint32_t MAP_FEATURE_MESSAGE_FORMAT_V11_BIT = 1 << 8;
const uint32_t MAP_FEATURE_MESSAGE_LISTING_FORMAT_V11_BIT = 1 << 9;
const uint32_t MAP_FEATURE_DATABASE_INDENTIFIER_BIT = 1 << 11;
const uint32_t MAP_FEATURE_FOLDER_VERSION_COUNTER_BIT = 1 << 12;
const uint32_t MAP_FEATURE_CONVERSATION_VERSION_COUNTER_BIT = 1 << 13;

const uint32_t MAP_MESSAGE_LISTING_FORMAT_V10 = 10;  // MAP spec below 1.3
const uint32_t MAP_MESSAGE_LISTING_FORMAT_V11 = 11;  // MAP spec 1.3
class MapMseContent {
public:
    MapMseContent();
    ~MapMseContent();
    void SetThreadIdSupport(bool support) { threadIdSupport_ = support; }
    int32_t GetMessageListingSize(
        const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead);
    std::string GetMessageListing(const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams,
        const std::string &version, int32_t &listSize, bool &hasUnRead);
    std::string GetMessage(
        const std::string &handleStr, const std::shared_ptr<MapMseAppParams> &appParams, const std::string &version);
    std::string GetConversationListing();

private:
    void CreateSmsDataShareHelper();

    void SetSmsMessageListingQueryParam(const std::string &folderName,
        const std::shared_ptr<MapMseAppParams> &appParams, DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, Uri &uri);
    void SetOtherFiltersQueryParam(const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams,
        DataShare::DataSharePredicates &predicates);

    void ParseSmsMessageListingSizeResult(std::shared_ptr<DataShare::DataShareResultSet> &result,
        const std::string &folderName, int32_t &count, bool &hasUnRead);
    int32_t GetSmsMessageListingSize(
        const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead);
    void ParseSmsMessageListingResult(std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
        const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams,
        std::vector<std::unique_ptr<MseMessageElement>> &messageList, bool &hasUnRead);
    std::vector<std::unique_ptr<MseMessageElement>> GetSmsMessageListing(
        const std::string &folderName, const std::shared_ptr<MapMseAppParams> &appParams, bool &hasUnRead);
    std::string EncodeSmsMessageListing(
        const std::vector<std::unique_ptr<MseMessageElement>> &messageList, const std::string &version);

    void SetSmsMessageQueryParam(
        int64_t handle, DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns, Uri &uri);
    std::shared_ptr<MapMseBMessageSms> ParseSmsMessageResult(
        int64_t handle, std::shared_ptr<DataShare::DataShareResultSet> &resultSet);
    void SetSmsMessageVcardData(std::shared_ptr<MapMseBMessageSms> &message, const std::string &number, bool incoming);
    std::string QueryContactNameByContactId(int64_t contactId);
    std::vector<std::string> QueryContactEmailsByContactId(int64_t contactId);
    std::string QueryContactNameByNumber(const std::string &number);
    int64_t QueryContactIdByNumber(const std::string &number);
    std::string GetSmsMessage(int64_t handle);

    bool SmsSelected(const std::shared_ptr<MapMseAppParams> &appParams);

private:
    std::shared_ptr<DataShare::DataShareHelper> smsDataShareHelper_ = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> contactDataShareHelper_ = nullptr;
    uint32_t messageListingVersion_ = MAP_MESSAGE_LISTING_FORMAT_V10;
    std::string messageVersion_ = MAP_V10_STR;
    bool threadIdSupport_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif