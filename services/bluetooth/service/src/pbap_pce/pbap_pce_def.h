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

#ifndef PBAP_PCE_DEF_H
#define PBAP_PCE_DEF_H

#include <cstdint>
#include <cstring>
#include "base_def.h"
#include "log.h"
#include "common_util.h"

#define PBAP_PCE_ENABLE_L2CAP_PSM
#define PBAP_PCE_LOG_DEBUG(fmt, ...) LOG_DEBUG("[PBAP_PCE]:" fmt, ##__VA_ARGS__)

#define PBAP_PCE_LOG_INFO(fmt, ...) LOG_INFO("[PBAP_PCE]:" fmt, ##__VA_ARGS__)

#define PBAP_PCE_LOG_WARN(fmt, ...) LOG_WARN("[PBAP_PCE]:" fmt, ##__VA_ARGS__)

#define PBAP_PCE_LOG_ERROR(fmt, ...) LOG_ERROR("[PBAP_PCE]:" fmt, ##__VA_ARGS__)

namespace OHOS {
namespace bluetooth {
/// phone book supported features
enum class PbapSupportedFeatures : uint8_t {
    PCE_ORDER = 0x01,
    PCE_SEARCH_VALUE = 0x02,
    PCE_SEARCH_PROPERTY = 0x03,
    PCE_MAX_LIST_COUNT = 0x04,
    PCE_LIST_START_OFFSET = 0x05,
    PCE_PROPERTY_SELECTOR = 0x06,
    PCE_FORMAT = 0x07,
    PCE_PHONE_BOOK_SIZE = 0x08,
    PCE_NEW_MISSED_CALLS = 0x09,
    PCE_PRIMARY_FOLDER_VERSION = 0x0A,
    PCE_SECONDARY_FOLDER_VERSION = 0x0B,
    PCE_VCARD_SELECTOR = 0x0C,
    PCE_DATABASE_IDENTIFIER = 0x0D,
    PCE_VCARD_SELECTOR_OPERATOR = 0x0E,
    PCE_RESET_NEW_MISSED_CALLS = 0x0F,
    PCE_PBAP_SUPPORTED_FEATURES = 0x10
};

// struct PbapObexClientConfig {
//     std::string addr;                        // Remote bluetooth address
//     uint16_t l2capLocalPsm = 0;              // l2cap's local psm while use l2cap
//     uint16_t l2capPsm = 0;                   // l2cap's psm
//     uint16_t rfCommChannel = 0;              // Rfcomm's channel num/ l2cap's psm
//     uint16_t mtu = 0;                       // The Maximum OBEX Packet Length.Default 1024byte
//     int32_t fileCount = 0;                  // The transfer file Count
//     bool isL2capPSM = false;                 // L2cap:true, rfcomm:false
//     bool isSupportSrm = false;               // Using Single Response Mode
//     bool isSupportReliableSession = false;   // Using reliable session
//     std::string serviceUUID;                 // Service's UUID128
// };

/// max device
static const int PCE_MAXIMUM_DEVICES = 10;

static const std::string PCE_SERVICE_NAME = "Phonebook Access PCE";
static const std::string PBAP_PCE_SERVICE_UUID_STR = "79a35f0f-c511-d809-6608-200c9a66";
static const uint16_t PBAP_PCE_LOCAL_GOEP_L2CAP_PSM = 0x1003;

static const std::string TELECOM_PATH = "/telecom";
static const std::string ICH_PATH = "/telecom/ich";
static const std::string OCH_PATH = "/telecom/och";
static const std::string MCH_PATH = "/telecom/mch";
static const std::string CCH_PATH = "/telecom/cch";
static const std::string PB_PATH = "/telecom/pb";

/// OBEX GET 接收超时（毫秒），对齐 ffrt 30s 拥塞阈值，防止下载线程永久阻塞
static const int PBAP_PCE_GET_TIMEOUT_MS = 30000;

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_DEF_H