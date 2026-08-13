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
#define LOG_TAG "bt_service_pbap_vcard_manager"
#endif

#include <chrono>
#include "log.h"
#include "core_service_client.h"
#include "pbap_pse_def.h"
#include "pbap_pse_vcard_manager.h"
#include "pbap_pse_vcard_filter.h"
#include "../hfp_ag/hfp_ag_system_interface.h"
#include "hitrace_meter.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {

// view_contact表中raw_contact外键
constexpr const char *NAME_RAW_CONTACT_ID = "name_raw_contact_id";

PbabPseVcardManager::PbabPseVcardManager()
{
    HILOGI("PbabPseVcardManager Create");
    Init();
}

void PbabPseVcardManager::Init()
{
    mDatashareUtils_ = std::make_shared<PbabPseDataShareUtils>();
    sLastFetchedTimeStamp_ = GetTimeStamp();
}

PbabPseVcardManager *PbabPseVcardManager::GetInstance()
{
    static PbabPseVcardManager instance;
    return &instance;
}

int32_t PbabPseVcardManager::ComposePhonebookVcards(ObexHeader &reply, PbapPseAppParam &appParam,
    CallLogSelectParam &selectParam, int32_t shareType, std::vector<uint8_t> &output)
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t number = PBAP_CONTACT_MAX_NUM_PER_QUERY;
    HILOGI("appParam.vcardFormat_: %{public}d", appParam.vcardFormat_);
    std::string orderByStr;
    std::shared_ptr<PbabPseVcardFilter> pbabPseVcardFilter = std::make_shared<PbabPseVcardFilter>();
    while (selectParam.startPoint <= selectParam.endPoint) {
        int32_t leftNumber = selectParam.endPoint - (selectParam.startPoint - 1);
        if (leftNumber < number) {
            number = leftNumber;
        }
        auto predicates = mDatashareUtils_->CreatContactPredicates(number, selectParam.startPoint, orderByStr,
            shareType);
        std::string vcard = mDatashareUtils_->ComposeContactVcard(appParam.vcardFormat_, predicates);
        uint64_t propertySelector = 0;
        if (!appParam.ignorefilter_) {
            propertySelector = appParam.propertySelector_;
        }
        std::string filterVcard = pbabPseVcardFilter->Apply(vcard, appParam.vcardFormat_, propertySelector, shareType);
        output.insert(output.end(), filterVcard.begin(), filterVcard.end());
        selectParam.startPoint = selectParam.startPoint + number;
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

static std::string GetSelfPhoneNumber()
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t slotId = 0;
    Telephony::CoreServiceClient::GetInstance().GetPrimarySlotId(slotId);
    std::u16string telephoneNumber;
    Telephony::CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId, telephoneNumber);

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string result = convert.to_bytes(telephoneNumber);
    return result;
}

int32_t PbabPseVcardManager::ComposeOwnerPhoneNumberVcard(PbapPseAppParam &appParam, std::vector<uint8_t> &output)
{
    std::string retStr;
    std::string number = GetSelfPhoneNumber();
    std::string name = number;
    // 组装vcard;
    retStr = mDatashareUtils_->ComposeOwnerVcard(appParam.vcardFormat_, name, number);
    output.insert(output.end(), retStr.begin(), retStr.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

int32_t PbabPseVcardManager::ComposePhonebookOneVcard(PbapPseAppParam &appParam, int32_t rawContactId, int32_t orderBy,
    int32_t shareType, std::vector<uint8_t> &output)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("rawContactId %{public}d, orderBy : %{public}d", rawContactId, orderBy);
    if (rawContactId < 1) {
        HILOGE("rawContactId is not correct, rawContactId: %{public}d", rawContactId);
        return ResponseCodes::OBEX_HTTP_INTERNAL_ERROR;
    }

    std::string orderByStr = "id";
    if (orderBy == ORDER_BY_ALPHABETICAL) {
        orderByStr = "display_name";
    }

    int32_t number = 1;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(NAME_RAW_CONTACT_ID, rawContactId);
    std::string vcard = mDatashareUtils_->ComposeContactVcard(appParam.vcardFormat_, predicates);
    output.insert(output.end(), vcard.begin(), vcard.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

int32_t PbabPseVcardManager::ComposeCallLogVcards(PbapPseAppParam &appParam,
    CallLogSelectParam &selectParam, std::vector<uint8_t> &output)
{
    std::string calllogVcard;
    int32_t number = selectParam.endPoint - (selectParam.startPoint - 1);
    HILOG_COMM_INFO("number: %{public}d, vcardType: %{public}u", number, appParam.vcardFormat_);
    calllogVcard = mDatashareUtils_->ComposeCallLogVcard(appParam.contentType_, appParam.vcardFormat_,
        number, selectParam.startPoint);
    output.insert(output.end(), calllogVcard.begin(), calllogVcard.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

void PbabPseVcardManager::GetContactNameList(std::vector<std::string> &nameList, uint8_t orderBy)
{
    mDatashareUtils_->GetContactNameList(nameList, orderBy);
}

void PbabPseVcardManager::GetContactNamesByNumber(std::vector<std::string> &names, const std::string &number)
{
    if (number.empty()) {
        HILOGE("number is empty");
        return;
    }
    mDatashareUtils_->GetContactNamesByNumber(number, names);
}

void PbabPseVcardManager::GetCallHistoryList(std::vector<std::string> &nameList, int32_t contentType)
{
    mDatashareUtils_->GetCallHistoryList(nameList, contentType);
}

int32_t PbabPseVcardManager::GetPhonebookSize(int32_t contentType)
{
    int32_t size;
    switch (contentType) {
        case PbapContentType::PHONEBOOK:
        case PbapContentType::FAVORITES:
            size = GetContactsSize(contentType);
            break;
        default:
            size = GetCallHistorySize(contentType);
            break;
    }

    HILOG_COMM_INFO("phonebookSize: %{public}d, contentType: %{public}d", size, contentType);
    return size;
}

int32_t PbabPseVcardManager::GetContactsSize(int32_t contentType)
{
    // add 1 for always has the 0.vcf
    return mDatashareUtils_->GetContactsSize(contentType) + 1;
}

int32_t PbabPseVcardManager::GetCallHistorySize(int32_t contentType)
{
    return mDatashareUtils_->GetCallHistorySize(contentType);
}

int32_t PbabPseVcardManager::GetNewMissedCallNum()
{
    return mDatashareUtils_->GetNewMissedCallNum();
}

void PbabPseVcardManager::GetCallHistoryPrimaryFolderVersion(int32_t contentType, std::vector<uint8_t> &retValue)
{
    int64_t count = mDatashareUtils_->GetCallHistorySizeByTime(contentType, sLastFetchedTimeStamp_);
    sLastFetchedTimeStamp_ = GetTimeStamp();
    int64_t primaryVcMsb = 0;
    for (int i = BASE_BIT_SIZE_8 - 1; i >= 0; i--) {
        retValue.push_back((count >> (i * BASE_BIT_SIZE_8)) & 0xFF);
    }
    for (int j = BASE_BIT_SIZE_8 - 1; j >= 0; j--) {
        retValue.push_back((primaryVcMsb >> (j * BASE_BIT_SIZE_8)) & 0xFF);
    }
    return;
}

void PbabPseVcardManager::ReleaseDataShareHelper()
{
    std::lock_guard<std::mutex> lock(releaseDataShareHelperTimerMutex_);
    if (releaseDataShareHelperTimer_ != nullptr) {
        releaseDataShareHelperTimer_->Stop();
    }
    std::shared_ptr<utility::Timer> releaseDataShareHelperTimer = std::make_shared<utility::Timer>([this]() {
        HILOGI("releaseDataShareHelper begin running.");
        this->mDatashareUtils_->ReleaseContactDataShareHelper();
        this->mDatashareUtils_->ReleaseCallLogDataShareHelper();
    });
    HILOGI("releaseDataShareHelper start new 5 min timer.");
    releaseDataShareHelperTimer_ = releaseDataShareHelperTimer;
    releaseDataShareHelperTimer_->Start(RELEASE_DATASHARE_TIMEOUT);
}

std::shared_ptr<utility::Timer> PbabPseVcardManager::GetReleaseDataShareHelperTimer()
{
    std::lock_guard<std::mutex> lock(releaseDataShareHelperTimerMutex_);
    return releaseDataShareHelperTimer_;
}
}  // namespace Bluetooth
}  // namespace OHOS