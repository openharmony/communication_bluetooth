/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_STATISTICS_MANAGER_H
#define BT_CHR_STATISTICS_MANAGER_H
#define UPLOAD_TIMEOUT_DEFAULT (6 * 60 * 60 * 1000)

#include "btcommon/timer_manager.h"
#include "bt_chr_base.h"
#include "safe_map.h"
#include <map>
#include <mutex>
#include <list>

namespace OHOS {
namespace bluetooth {
enum BtTransactionStatisticsResult {
    TRANSACTION_RESULT_NA = 0,
    TRANSACTION_RESULT_TOTAL = 1,
    TRANSACTION_RESULT_SUCCESS = 2,
    TRANSACTION_RESULT_FAIL = 3,
};

enum BtTransactionStatisticsSceneCode {
    TRANSACTION_SCENECODE_NA = 0,
    TRANSACTION_SCENECODE_1 = 1,
    TRANSACTION_SCENECODE_2 = 2,
    TRANSACTION_SCENECODE_3 = 3,
    TRANSACTION_SCENECODE_4 = 4,
    TRANSACTION_SCENECODE_5 = 5,
    TRANSACTION_SCENECODE_6 = 6,
    TRANSACTION_SCENECODE_7 = 7,
    TRANSACTION_SCENECODE_8 = 8,
    TRANSACTION_SCENECODE_9 = 9,
    TRANSACTION_SCENECODE_10 = 10,
    TRANSACTION_SCENECODE_11 = 11,
    TRANSACTION_SCENECODE_12 = 12,
    TRANSACTION_SCENECODE_13 = 13,
    TRANSACTION_SCENECODE_14 = 14,
    TRANSACTION_SCENECODE_15 = 15,
    TRANSACTION_SCENECODE_16 = 16,
};

enum BtTransactionStatisticsType {
    TRANSACTION_TYPE_NA = 0,
    TRANSACTION_TYPE_OPP_SEND = 1,
    TRANSACTION_TYPE_OPP_RECEIVE = 2,
    TRANSACTION_TYPE_HEARING_AID_CONNECT = 3,
    TRANSACTION_TYPE_WIRELESS_CHARGING_AUDIO = 4,
    TRANSACTION_TYPE_BTC_DATA_BUF_OVERFLOW = 5,
    TRANSACTION_TYPE_BLE_LOW_LATENCY_SCAN = 6,
};

constexpr uint8_t MIN_SCENE_CODE = 1;
constexpr uint8_t MAX_SCENE_CODE = 16;
constexpr uint32_t MAX_RESULT_CODE = 300;

class CounterGroup {
public:
    CounterGroup(const std::string &name) : name_(name) {}
    virtual ~CounterGroup() {}
    bool AddCount(const std::string &counterName, uint32_t step);
    bool AddStrParam(const std::string &param, const std::string &value);
    bool CleanCounterGroup();

    SafeMap<std::string, uint32_t> countersMap_;
    SafeMap<std::string, std::string> strParamMap_;
    std::string name_;
};

class BtChrTransactionManager {
public:
    static BtChrTransactionManager& GetInstance();
    // 数据上报:上报当前所有counterGroupList_
    bool ReportTransaction();
    // 设置数据上报任务
    bool SetTransactionReport();
    std::string GetCounterGroupJsonStr();
    void WriteTransactionStatictics(BtTransactionStatisticsType transactionType,
        const BtTransactionStatisticsResult result, const uint32_t resultCount = 1,
        const BtTransactionStatisticsSceneCode sceneCode = TRANSACTION_SCENECODE_NA, const uint32_t sceneCodeCount = 1);
private:
    std::mutex counterGroupTimerMutex_;
    std::shared_ptr<utility::Timer> reportTimer_ = nullptr;
    std::list<std::shared_ptr<CounterGroup>> counterGroupList_;
    void AddTransactionCounterGroup(const std::string &name);
    bool AddTransactionCounter(const std::string &name, const std::string &counterName, uint32_t step);
    bool AddTransactionStringParam(const std::string &name, const std::string &paramName, const std::string &value);
    bool HasTransactionCounterGroup(const std::string &name);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_STATISTICS_MANAGER_H */
