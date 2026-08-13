/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PBAP_PCE_DATASHARE_HELPER_H
#define PBAP_PCE_DATASHARE_HELPER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "datashare_helper.h"
#include "pbap_pce_vcard_parser.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {

/// Call log type, determined by PBAP path (MCH/ICH/OCH).
enum PbapPceCallType {
    PBAP_PCE_CALL_TYPE_MISSED = 0,   // MCH path
    PBAP_PCE_CALL_TYPE_INCOMING = 1, // ICH path
    PBAP_PCE_CALL_TYPE_OUTGOING = 2, // OCH path
};

/// DataShare helper for writing parsed vCard contacts and call logs to OHOS
/// contacts/calllog databases.
/// Contacts: datashare:///com.ohos.contactsdataability/contacts/import_batch
/// Call logs: datashare:///com.ohos.calllogability/calls/calllog
class PbapPceDataShareHelper {
public:
    PbapPceDataShareHelper();
    ~PbapPceDataShareHelper();

    /// Start a new contact sync session. Generates a new traceId and resets batchId.
    /// Should be called at the beginning of each contact download round.
    void StartContactSync();

    /// Batch insert parsed contacts into contacts database.
    /// Uses high-level semantic fields: "name" + "phoneNumbers" (comma-separated).
    /// The URL is appended with traceId/isFinish/batchId query parameters per
    /// contactdatashare.md spec.
    /// @param contacts Parsed vCard contacts.
    /// @param isFinish Whether this is the last batch of the current sync round.
    /// @return 0 on success, negative on failure.
    int32_t BatchInsertContacts(const std::vector<PbapPceVCardContact> &contacts, bool isFinish);

    /// Batch insert parsed call logs into calllog database.
    /// Extracts phone number, display name, and timestamp from X-IRMC-CALL-DATETIME.
    /// @param contacts Parsed vCard call log entries.
    /// @param callType Call type derived from PBAP path (MCH/ICH/OCH).
    /// @return 0 on success, negative on failure.
    int32_t BatchInsertCallLogs(const std::vector<PbapPceVCardContact> &contacts, PbapPceCallType callType);

    /// Release DataShareHelper instances.
    void Release();

private:
    /// Create a DataShareHelper for the given URI.
    std::shared_ptr<DataShare::DataShareHelper> CreateHelper(const std::string &uri);

    /// Get or create the contacts DataShareHelper (lazy init).
    std::shared_ptr<DataShare::DataShareHelper> GetContactHelper();

    /// Get or create the calllog DataShareHelper (lazy init).
    std::shared_ptr<DataShare::DataShareHelper> GetCallLogHelper();

    /// Build the contact batch URI with traceId/isFinish/batchId query parameters.
    /// @param traceId Current sync round traceId.
    /// @param isFinish Whether this batch is the last one of the round.
    /// @param batchId Current batch index (starting from 0).
    /// @return Full URI string with query parameters.
    static std::string BuildContactBatchUri(const std::string &traceId, bool isFinish, uint32_t batchId);

    /// Generate a unique traceId for a new sync round.
    /// @return A unique traceId string.
    static std::string GenerateTraceId();

    /// Build a comma-separated phone numbers string from a contact.
    static std::string BuildPhoneNumbersString(const PbapPceVCardContact &contact);

    /// Parse X-IRMC-CALL-DATETIME timestamp from contact's unknownXData.
    /// @param contact Parsed vCard entry with X-IRMC-CALL-DATETIME property.
    /// @return Epoch milliseconds, or 0 if not found/parse error.
    static int64_t ParseCallLogTimestamp(const PbapPceVCardContact &contact);

    /// Parse vCard timestamp format "yyyyMMdd'T'HHmmss" to epoch milliseconds.
    /// @param timestampStr e.g. "20050320T100000"
    /// @return Epoch milliseconds, or 0 on parse error.
    static int64_t ParseTimestampString(const std::string &timestampStr);

    /// Map call type to calllog database call_direction field.
    /// INCOMING_TYPE=0, OUTGOING_TYPE=1 
    static int32_t GetCallDirection(PbapPceCallType callType);

    /// Map call type to calllog database answer_state field.
    /// CALL_ANSWER_MISSED=0, CALL_ANSWER_ACTIVED=1 (aligned with PSE constants).
    static int32_t GetAnswerState(PbapPceCallType callType);

    std::mutex mutex_;
    std::shared_ptr<DataShare::DataShareHelper> contactHelper_ = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> callLogHelper_ = nullptr;

    /// Current contact sync session state (traceId + batchId).
    std::string traceId_;
    uint32_t batchId_ = 0;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_DATASHARE_HELPER_H
