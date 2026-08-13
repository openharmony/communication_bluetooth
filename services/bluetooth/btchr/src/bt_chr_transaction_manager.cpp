/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_chr_statictics_manager"
#endif

#include "bt_chr_dft_statictics.h"
#include "bt_chr_transaction_manager.h"
#include "bt_chr_util.h"
#include "bluetooth_log.h"
#include "cJSON.h"

namespace OHOS {
namespace bluetooth {
static const std::map<BtTransactionStatisticsType, std::string> g_transactionTypeStatisticsMap = {
    { TRANSACTION_TYPE_OPP_SEND, "OPP_SEND" },
    { TRANSACTION_TYPE_OPP_RECEIVE, "OPP_RECEIVE" },
    { TRANSACTION_TYPE_HEARING_AID_CONNECT, "HEARING_AID_CONNECT" },
    { TRANSACTION_TYPE_WIRELESS_CHARGING_AUDIO, "WIRELESS_CHARGING_AUDIO" },
    { TRANSACTION_TYPE_BTC_DATA_BUF_OVERFLOW, "BTC_DATA_BUF_OVERFLOW" },
    { TRANSACTION_TYPE_BLE_LOW_LATENCY_SCAN, "BLE_LOW_LATENCY_SCAN" },
};

static const std::map<BtTransactionStatisticsResult, std::string> g_transactionResultStatisticsMap = {
    { TRANSACTION_RESULT_TOTAL, "TOTAL_CNT" },
    { TRANSACTION_RESULT_SUCCESS, "SUCCESS_CNT" },
    { TRANSACTION_RESULT_FAIL, "FAIL_CNT" },
};

BtChrTransactionManager& BtChrTransactionManager::GetInstance()
{
    static BtChrTransactionManager instance;
    return instance;
}

void BtChrTransactionManager::WriteTransactionStatictics(BtTransactionStatisticsType transactionType,
    const BtTransactionStatisticsResult result, const uint32_t resultCount,
    const BtTransactionStatisticsSceneCode sceneCode, const uint32_t sceneCodeCount)
{
    HILOGI("WriteTransactionStatictics");
    auto typeIter = g_transactionTypeStatisticsMap.find(transactionType);
    if (typeIter == g_transactionTypeStatisticsMap.end()) {
        HILOGE("invalid transaction type");
        return;
    }

    auto resultIter = g_transactionResultStatisticsMap.find(result);
    if (resultIter == g_transactionResultStatisticsMap.end()) {
        HILOGE("invalid result type");
        return;
    }

    if (resultCount > MAX_RESULT_CODE || sceneCodeCount > MAX_RESULT_CODE) {
        HILOGE("invalid result code, resultCode is %{public}u, sceneCount is %{public}u", resultCount, sceneCodeCount);
        return;
    }

    //业务首次调用该事件的时候起上报定时器
    SetTransactionReport();

    bool hasCounterGroup = HasTransactionCounterGroup(typeIter->second);
    if (!hasCounterGroup) {
        HILOGI("AddTransactionCounter");
        AddTransactionCounterGroup(typeIter->second);
    }
    AddTransactionStringParam(typeIter->second, "TRANSACTION_TYPE", typeIter->second);
    AddTransactionCounter(typeIter->second, resultIter->second, resultCount);
    HILOGI("AddCounter resultCount %{public}d", resultCount);
    uint8_t sceneCodeInt = static_cast<uint8_t>(sceneCode);
    if (sceneCodeInt < MIN_SCENE_CODE || sceneCodeInt > MAX_SCENE_CODE) {
        HILOGI("[invalid scene code]");
        return;
    }
    AddTransactionCounter(typeIter->second, std::to_string(sceneCodeInt), sceneCodeCount);
    HILOGI("AddCountersceneCode  %{public}d sceneCodeCount: %{public}d", sceneCodeInt, sceneCodeCount);
}

void BtChrTransactionManager::AddTransactionCounterGroup(const std::string &name)
{
    std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
    for (std::shared_ptr<CounterGroup> it : counterGroupList_) {
        if (it->name_ == name) {
            HILOGI("%{public}s, counterGroup is exist!", name.c_str());
            return;
        }
    }
    HILOGI("[AddCounterGroup]name:%{public}s", name.c_str());
    std::shared_ptr<CounterGroup> newCounterGroup = std::make_shared<CounterGroup>(name);
    counterGroupList_.push_back(newCounterGroup);
    return;
}

bool BtChrTransactionManager::HasTransactionCounterGroup(const std::string &name)
{
    std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
    for (auto it : counterGroupList_) {
        if (it->name_ == name) {
            return true;
        }
    }
    return false;
}

bool BtChrTransactionManager::AddTransactionCounter(const std::string &name,
    const std::string &counterName, uint32_t step)
{
    if (step == 0) {
        HILOGE("AddCounter step is 0, no need add");
        return false;
    }

    // 查看当前list中是否已经有eventname
    std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
    for (auto it : counterGroupList_) {
        if (it->name_ == name) {
            it->AddCount(counterName, step);
            return true;
        }
    }
    HILOGE("AddCounter %{public}s:%{public}s failed, counterGroup is not exist!",
                  name.c_str(), counterName.c_str());
    return false;
}

bool BtChrTransactionManager::AddTransactionStringParam(const std::string &name,
    const std::string &paramName, const std::string &value)
{
    std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
    for (auto it : counterGroupList_) {
        if (it->name_ == name) {
            it->AddStrParam(paramName, value);
            return true;
        }
    }

    return false;
}

bool BtChrTransactionManager::SetTransactionReport()
{
    HILOGI("SetTransactionReport begin");
    std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
    if (reportTimer_ != nullptr) {
        return false;
    }

    std::shared_ptr<utility::Timer> reportTimer = std::make_shared<utility::Timer>(
        [this]() { this->ReportTransaction(); });
    reportTimer_ = reportTimer;
    reportTimer_->Start(UPLOAD_TIMEOUT_DEFAULT);
    return true;
}

std::string BtChrTransactionManager::GetCounterGroupJsonStr()
{
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    {
        std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
        for (std::shared_ptr<CounterGroup> counterGroup : counterGroupList_) {
            cJSON *paramObj = cJSON_CreateObject();
            if (paramObj == nullptr) {
                HILOGE("cjson create paramObj is nullptr.");
                continue;
            }
            counterGroup->countersMap_.Iterate([&paramObj](std::string name, uint32_t num) {
                cJSON_AddItemToObject(paramObj, name.c_str(), cJSON_CreateNumber(num));
            });
            counterGroup->strParamMap_.Iterate([&paramObj](std::string name, std::string value) {
                cJSON_AddItemToObject(paramObj, name.c_str(), cJSON_CreateString(value.c_str()));
            });
            cJSON_AddItemToArray(array, paramObj);
        }
    }
    std::string listName = "TRANSACTION_LIST";
    cJSON_AddItemToObject(root, listName.c_str(), array);

    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(root);
        return "";
    }
    std::string result = std::string(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    HILOGI("The last string is %{public}s", result.c_str());
    return result;
}

bool BtChrTransactionManager::ReportTransaction()
{
    HILOGI("reportTransaction begin");
    {
        std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
        if (counterGroupList_.empty()) {
            HILOGI("counterGroupList_ is empty, no data to report.");
            if (reportTimer_ != nullptr){
                reportTimer_->Stop();
                reportTimer_ = nullptr;
            }
            return true;
        }
    }
    //上报数据
    BtChrStaticticsEventWrite(EVENT_TYPE_TRANSACTION_STATISTICS, GetCounterGroupJsonStr());

    {
        // 上报后清除当前记录
        std::lock_guard<std::mutex> lock(counterGroupTimerMutex_);
        for (std::shared_ptr<CounterGroup> group : counterGroupList_) {
            group->CleanCounterGroup();
        }
        counterGroupList_.clear();
        if (reportTimer_ != nullptr){
            reportTimer_->Stop();
            reportTimer_ = nullptr;
        }
    }
    SetTransactionReport();
    return true;
}

bool CounterGroup::CleanCounterGroup()
{
    countersMap_.Clear();
    strParamMap_.Clear();
    return true;
}

bool CounterGroup::AddStrParam(const std::string &param, const std::string &value)
{
    strParamMap_.EnsureInsert(param, value);
    return true;
}

bool CounterGroup::AddCount(const std::string &counterName, uint32_t step)
{
    uint32_t oldCount = 0;
    if (countersMap_.Find(counterName, oldCount)) {
        countersMap_.EnsureInsert(counterName, oldCount + step);
    } else {
        countersMap_.EnsureInsert(counterName, step);
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS