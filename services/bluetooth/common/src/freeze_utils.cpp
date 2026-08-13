/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_freeze_util"
#endif
#include "freeze_utils.h"
#include "log.h"
#ifdef RES_SCHED_SUPPORT
#include "iservice_registry.h"
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace bluetooth {
namespace  {
    constexpr int32_t DELAY_INTERVAL = 3 * 60 * 1000 * 1000;
    constexpr int32_t FIRST_APPLICATION_UID = 10000;
    constexpr int32_t RES_SCHED_SYS_ABILITY_ID = 1901;
    constexpr int32_t SUSPEND_MANAGER_SYSTEM_ABILITY_ID = 1910;
    const std::string FROZEN_WITHOUT_TRANSFER = "FROZEN_WITHOUT_TRANSFER";
    const std::string FROZEN_TASK_QUEUE = "FROZEN_TASK_QUEUE";
    const std::string FROZEN_TASK = "FROZEN_TASK";
}

#ifdef RES_SCHED_SUPPORT
FreezeUtil::~FreezeUtil()
{
    if (ffrtQueue_) {
        ffrtQueue_.reset();
    }
}

FreezeUtil *FreezeUtil::GetInstance(void)
{
    static FreezeUtil instance;
    return &instance;
}

void FreezeUtil::InitQueueTask()
{
    ffrtQueue_ = std::make_shared<ffrt::queue>(FROZEN_TASK_QUEUE.c_str(),
        ffrt::queue_attr().qos(ffrt::qos_user_interactive));
    if (!ffrtQueue_) {
        HILOGE("init frozen queue fail.");
        return;
    }
    SubscribeRssSystemAbility();
    ffrtQueue_->submit([this]() {
            CheckPeriodical();
            }, ffrt::task_attr().name(FROZEN_TASK.c_str()).delay(DELAY_INTERVAL));
}

void FreezeUtil::CheckPeriodical()
{
    SetFreezeState();
    if (!ffrtQueue_) {
        return;
    }
    ffrt_queue_t* queue = reinterpret_cast<ffrt_queue_t*>(ffrtQueue_.get());
    if (queue == nullptr) {
        return;
    }
    if (!ffrt_queue_has_task(*queue, FROZEN_TASK.c_str())) {
        ffrtQueue_->submit([this]() {
            CheckPeriodical();
            }, ffrt::task_attr().name(FROZEN_TASK.c_str()).delay(DELAY_INTERVAL));
    }
}

void FreezeUtil::SetFreezeState()
{
    std::vector<std::pair<int, int>> tmpReport;
    hasDataReport_.Iterate([&tmpReport, this](std::pair<int, int> appInfo, bool &isReported) {
        if (!isReported) {  //再过3分钟，如果期间没有数据传输，通知RSS数据传输结束
            ReportBtDataToRss(appInfo.first, appInfo.second, false, FROZEN_WITHOUT_TRANSFER);
            tmpReport.emplace_back(appInfo);
        } else {
            isReported = false;   //3分钟后,将上报过的标记位置为false
        }
    });
    for (auto info : tmpReport) {
        hasDataReport_.Erase(info);
    }
}

void FreezeUtil::RequestActive(const int32_t pid, const int32_t uid, const std::string & reason)
{
    HILOGD("bt data transfer uid: %{public}d, reason: %{public}s", uid, reason.c_str());
    if (uid < FIRST_APPLICATION_UID) {
        return;
    }
    std::pair<int, int> appInfo;
    appInfo.first = pid;
    appInfo.second = uid;
    if (isReport(appInfo)) {
        ReportBtDataToRss(pid, uid, true, reason);
    }
    hasDataReport_.EnsureInsert(appInfo, true);
}

bool FreezeUtil::isReport(std::pair<int, int> appInfo)
{
    if (!ffrtQueue_) {
        InitQueueTask();
    }
    bool isReport = true;
    if (!hasDataReport_.Find(appInfo, isReport) || !isReport) { //没find到代表没上报给RSS，report为false代表需要重新上报给RSS
        HILOGI("gatt request active pid: %{public}d, uid: %{public}d", appInfo.first, appInfo.second);
        return true;
    }
    return false;
}

void FreezeUtil::ReportBtDataToRss(
    const int32_t pid, const int32_t uid, const bool &isTransfer, const std::string &reason)
{
    std::unordered_map<std::string, std::string> payload;
    payload["PID"] = std::to_string(pid);
    payload["UID"] = std::to_string(uid);
    payload["ISTRANSFER"] = std::to_string(isTransfer);
    payload["REASON"] = reason;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
        ResourceSchedule::ResType::BtServiceEvent::GATT_DATA_TRANSFER,
        payload);
}

void FreezeUtil::ReportSppConnectState(
    int32_t pid, int32_t uid, const std::string &address, int id, const std::string &action)
{
    std::unordered_map<std::string, std::string> payload;
    payload["PID"] = std::to_string(pid);
    payload["UID"] = std::to_string(uid);
    payload["ID"] = std::to_string(id);
    payload["ACTION"] = action;
    payload["ADDRESS"] = address;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
        ResourceSchedule::ResType::BtServiceEvent::SPP_CONNECT_STATE,
        payload);
}

void FreezeUtil::SubscribeRssSystemAbility()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgrProxy) {
        HILOGE("failed to get samgrProxy");
        return;
    }
    if (statusChangeListener_) {
        HILOGE("has been subscribed");
        return;
    }
    const std::set<int> saIdSet = {RES_SCHED_SYS_ABILITY_ID, SUSPEND_MANAGER_SYSTEM_ABILITY_ID};
    auto addFunc = [this](int32_t systemAbilityId) {
        hasDataReport_.Iterate([](std::pair<int, int> appInfo, bool &isReported) {
            isReported = false;
        });
    };
    auto removeFunc = [](int32_t systemAbilityId) {
        return;
    };
    statusChangeListener_ = new SystemAbilitySubscriber(saIdSet, addFunc, removeFunc);
    int32_t ret = samgrProxy->SubscribeSystemAbility(RES_SCHED_SYS_ABILITY_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: call manager service failed!");
        statusChangeListener_ = nullptr;
        return;
    }
    ret = samgrProxy->SubscribeSystemAbility(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: core service failed!");
        statusChangeListener_ = nullptr;
        return;
    }
}
#endif
}  // namespace OHOS
}  // namespace bluetooth
