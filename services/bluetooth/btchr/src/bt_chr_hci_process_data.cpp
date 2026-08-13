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
#ifndef LOG_TAG
#define LOG_TAG "bt_chr_hci_process_data"
#endif

#include "bt_chr_hci_process_data.h"
#include <numeric>
#include "bluetooth_log.h"
#include "datetime_ex.h"
#include "thread_util.h"
#include "bt_chr_util.h"

constexpr uint32_t MAX_HCI_PROCESS_DATA_LEN = 1023;
constexpr uint32_t HCI_PROCESS_STRING_LEN = 3;
constexpr uint32_t TIMEOUT_TIME_MS = 15; // 单位s
constexpr uint32_t HCI_CODE_CLEAR_DELAY_TIME_MS = 90 * 1000; // 单位ms 90s

namespace OHOS {
namespace bluetooth {
BtChrHciProcessData::BtChrHciProcessData() {}

BtChrHciProcessData::~BtChrHciProcessData()
{
    processDataMap_.Clear();
    timestampMap_.Clear();
}

BtChrHciProcessData& BtChrHciProcessData::GetInstance()
{
    static BtChrHciProcessData instance;
    return instance;
}

void BtChrHciProcessData::SaveImportantHciProcessData(const std::string& peerAddr, const std::string& processCode)
{
    if (processCode.size() != HCI_PROCESS_STRING_LEN) {
        HILOGE("Invalid processCode len");
        return;
    }

    std::string addr = GetLowercaseAddr(peerAddr);
    std::string value {};
    processDataMap_.Find(addr, value);
    value += processCode;
    if (value.size() > MAX_HCI_PROCESS_DATA_LEN) {
        value = value.substr(value.size() - MAX_HCI_PROCESS_DATA_LEN);
    }
    processDataMap_.EnsureInsert(addr, value);
    return SubmitClearTask(addr);
}

const std::string BtChrHciProcessData::GetImportantHciProcessData(const std::string& peerAddr)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    std::string value {};
    processDataMap_.Find(addr, value);
    return value;
}

void BtChrHciProcessData::ClearHciDataByPeerAddr(const std::string& peerAddr)
{
    processDataMap_.Erase(peerAddr);
    timestampMap_.Erase(peerAddr);
}

void BtChrHciProcessData::SubmitClearTask(const std::string& peerAddr)
{
    int64_t lastTime = 0;
    int64_t curTime = 0;
    timestampMap_.Find(peerAddr, lastTime);
    curTime = GetSecondsSince1970ToNow();
    if ((curTime > lastTime) && ((curTime - lastTime) < TIMEOUT_TIME_MS)) {
        return;
    }

    timestampMap_.EnsureInsert(peerAddr, curTime);
    ThreadUtil::GetInstance().PostTask(THREAD_ID_ADAPTER_MANAGER,
        std::bind(&BtChrHciProcessData::ClearHciDataByPeerAddr, this, peerAddr),
        HCI_CODE_CLEAR_DELAY_TIME_MS, peerAddr + "hciCode");
}
}  // namespace bluetooth
}  // namespace OHOS