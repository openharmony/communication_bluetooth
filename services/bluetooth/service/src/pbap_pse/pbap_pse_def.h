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

#ifndef PBAP_PSE_DEF_H
#define PBAP_PSE_DEF_H

#include <cstdint>
#include <string>
#include "log.h"

namespace OHOS {
namespace bluetooth {
constexpr int32_t SDP_PBAP_SERVER_VERSION = 0x0101;
constexpr int32_t SDP_PBAP_SUPPORTED_REPOSITORIES = 0x0001;
constexpr int32_t SDP_PBAP_SUPPORTED_FEATURES = 0x0003;
// Currently not support SIM card

const std::vector<std::string> LEGAL_PATH_WITH_SIM = {
    "/telecom",
    "/telecom/pb",
    "/telecom/fav",
    "/telecom/ich",
    "/telecom/och",
    "/telecom/mch",
    "/telecom/cch",
    "/SIM1",
    "/SIM1/telecom",
    "/SIM1/telecom/ich",
    "/SIM1/telecom/och",
    "/SIM1/telecom/mch",
    "/SIM1/telecom/cch",
    "/SIM1/telecom/pb"
};

// SIM card
const std::string SIM1 = "SIM1";
// missed call history
const std::string MCH = "mch";
// incoming call history
const std::string ICH = "ich";
// outgoing call history
const std::string OCH = "och";
// combined call history
const std::string CCH = "cch";
// phone book
const std::string PB = "pb";
// favorites
const std::string FAV = "fav";

const std::string TELECOM_PATH = "/telecom";
const std::string ICH_PATH = "/telecom/ich";
const std::string OCH_PATH = "/telecom/och";
const std::string MCH_PATH = "/telecom/mch";
const std::string CCH_PATH = "/telecom/cch";
const std::string PB_PATH = "/telecom/pb";
const std::string FAV_PATH = "/telecom/fav";

// type for list vcard objects
const std::string TYPE_LISTING = "x-bt/vcard-listing";
// type for get single vcard object
const std::string TYPE_VCARD = "x-bt/vcard";
// type for download all vcard objects
const std::string TYPE_PB = "x-bt/phonebook";

constexpr int32_t VERSION_21 = 0;
constexpr int32_t VERSION_30 = 1;
constexpr int32_t VERSION_40 = 2;
constexpr int32_t VERSION_MASK = 3;

const std::string PBAP_PSE_SERVICE_NAME = "OBEX Phonebook Access Server";
constexpr int32_t PBAP_RFCOMM_CHANNEL = 19;
constexpr int32_t PBAP_L2CAP_PSM = 0x1025;
constexpr int32_t SDP_PBAP_OMIT_L2CAP = -1;

// ContentType
enum PbapContentType {
    PHONEBOOK = 1,
    INCOMING_CALL_HISTORY = 2,
    OUTGOING_CALL_HISTORY = 3,
    MISSED_CALL_HISTORY = 4,
    COMBINED_CALL_HISTORY = 5,
    FAVORITES = 6,
};

constexpr int32_t VCARD_NAME_SUFFIX_LENGTH = 5;

constexpr int32_t ORDER_BY_INDEXED = 0;
constexpr int32_t ORDER_BY_ALPHABETICAL = 1;
constexpr int32_t ORDER_TYPE_PHONETICAL = 2;

struct CallLogSelectParam {
    int32_t startPoint;
    int32_t endPoint;
    int32_t needSendBody;
    int32_t pbSize;
};

constexpr int32_t BASE_BIT_SIZE_8 = 8;

constexpr int32_t PBAP_REJECT_MAX_TIMES = 2;
constexpr int32_t PBAP_PERMISSION_REQUEST_MIN_TIME_DIFF = 10; // 10s

constexpr int32_t PBAP_CONTACT_MAX_NUM_PER_QUERY = 100;
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PSE_DEF_H