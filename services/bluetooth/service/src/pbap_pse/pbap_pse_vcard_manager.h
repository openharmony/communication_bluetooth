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

#ifndef PBAP_PSE_VCARD_MANAGER
#define PBAP_PSE_VCARD_MANAGER
#define RELEASE_DATASHARE_TIMEOUT (5 * 60 * 1000)

#include <vector>

#include "btcommon/timer_manager.h"
#include "pbap_pse_def.h"
#include "pbap_pse_appparam.h"
#include "pbap_pse_datashare_utils.h"
#include "obex_def.h"
#include "obex_header.h"

namespace OHOS {
namespace bluetooth {

class PbabPseVcardManager {
public:
    explicit PbabPseVcardManager();
    ~PbabPseVcardManager() {}
    void Init();

    static PbabPseVcardManager *GetInstance();

    int32_t ComposePhonebookVcards(ObexHeader &reply, PbapPseAppParam &appParam, CallLogSelectParam &selectParam,
        int32_t shareType, std::vector<uint8_t> &output);
    int32_t ComposeOwnerPhoneNumberVcard(PbapPseAppParam &appParam, std::vector<uint8_t> &output);
    int32_t ComposePhonebookOneVcard(PbapPseAppParam &appParam, int32_t rawContactId, int32_t orderBy,
        int32_t shareType, std::vector<uint8_t> &output);
    int32_t ComposeCallLogVcards(PbapPseAppParam &appParam, CallLogSelectParam &selectParam,
        std::vector<uint8_t> &output);

    void GetContactNameList(std::vector<std::string> &nameList, uint8_t orderBy);
    void GetContactNamesByNumber(std::vector<std::string> &names, const std::string &number);
    void GetCallHistoryList(std::vector<std::string> &nameList, int32_t contentType);
    void GetCallHistoryPrimaryFolderVersion(int32_t contentType, std::vector<uint8_t> &retValue);

    int32_t GetPhonebookSize(int32_t contentType);
    int32_t GetContactsSize(int32_t contentType);
    int32_t GetCallHistorySize(int32_t contentType);
    int32_t GetNewMissedCallNum();

    void ReleaseDataShareHelper();
    std::shared_ptr<utility::Timer> GetReleaseDataShareHelperTimer();

private:
    std::string GetSubscriberNumber();

    std::shared_ptr<PbabPseDataShareUtils> mDatashareUtils_ = nullptr;

    std::mutex releaseDataShareHelperTimerMutex_;
    std::shared_ptr<utility::Timer> releaseDataShareHelperTimer_ = nullptr;
    int64_t sLastFetchedTimeStamp_ = 0;
};

} // namespace bluetooth
} // namespace OHOS

#endif // PBAP_PSE_VCARD_MANAGER