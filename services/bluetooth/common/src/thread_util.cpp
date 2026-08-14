/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_common_thread"
#endif

#include "thread_util.h"

#include "datetime_ex.h"
#include "log.h"
#include "ffrt_inner.h"

int GetFfrtQueueId(void)
{
    return ffrt::get_queue_id();
}

namespace OHOS {
namespace bluetooth {

static void PostTaskToThread(int threadId, const ThreadUtilFunc &func, uint64_t delayTime)
{
    ThreadUtil::GetInstance().PostTask(threadId, func, delayTime);
}

void DoInBleThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_BLE, func, delayTime);
}

void DoInGattThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_GATT, func, delayTime);
}

void DoInAvrcpTgThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_AVRCP_TG, func, delayTime);
}

void DoInClassicThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_CLASSIC, func, delayTime);
}

void DoInA2dpThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_A2DP, func, delayTime);
}

void DoInHfpThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_HFP, func, delayTime);
}

void DoInHidHostThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_HID_HOST, func, delayTime);
}

void DoInHidDeviceThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_HID_DEVICE, func, delayTime);
}

void DoInPanThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_PAN, func, delayTime);
}

void DoInSocketThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_SOCKET, func, delayTime);
}

void DoInAdapterManagerThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_ADAPTER_MANAGER, func, delayTime);
}

void DoInMapThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_MAP, func, delayTime);
}

void DoInPbapThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_PBAP, func, delayTime);
}

void DoInOppThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_OPP, func, delayTime);
}

void DoInHfThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_HF, func, delayTime);
}

void DoInVendorThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_VENDOR, func, delayTime);
}

void DoInLowPriorityThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_LOW_PRIORITY, func, delayTime);
}

void DoInHearingAidThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_HEARING_AID, func, delayTime);
}

void DoInBipThread(const ThreadUtilFunc &func, uint64_t delayTime)
{
    PostTaskToThread(THREAD_ID_BIP, func, delayTime);
}

static std::string GetThreadName(int threadId)
{
    const std::map<int, std::string> threadNameMap {
        { THREAD_ID_BLE,                "bt_ble" },
        { THREAD_ID_GATT,               "bt_gatt" },
        { THREAD_ID_AVRCP_TG,           "bt_avrcp_tg" },
        { THREAD_ID_CLASSIC,            "bt_classic" },
        { THREAD_ID_A2DP,               "bt_a2dp" },
        { THREAD_ID_HFP,                "bt_hfp" },
        { THREAD_ID_HID_HOST,           "bt_hid_host" },
        { THREAD_ID_HID_DEVICE,         "bt_hid_device" },
        { THREAD_ID_PAN,                "bt_pan" },
        { THREAD_ID_SOCKET,             "bt_socket" },
        { THREAD_ID_ADAPTER_MANAGER,    "bt_adapter_manager" },
        { THREAD_ID_MAP,                "bt_map" },
        { THREAD_ID_PBAP,               "bt_pbap" },
        { THREAD_ID_OPP,                "bt_opp" },
        { THREAD_ID_HF,                 "bt_hf" },
        { THREAD_ID_VENDOR,             "bt_vendor" },
        { THREAD_ID_LOW_PRIORITY,       "bt_low_priority" },
        { THREAD_ID_BIP,                "bt_bip" },
    };

    auto it = threadNameMap.find(threadId);
    if (it == threadNameMap.end()) {
        HILOGE("Not find threadId: %{public}d", threadId);
        return "Unknown";
    }

    return it->second;
}

struct ThreadUtil::impl {
    using DelayTaskKey = std::pair<int, std::string>;
    struct DelayTask {
        std::string name = "";
        std::atomic_bool trigger = false;
        ffrt::task_handle taskHandle = nullptr;
    };
    class TaskQueue {
    public:
        const uint64_t DELAY_TIME_MS_MAX = 0xFFFFFFFF;  // Max delay time is 8 years
        const uint64_t MILLISEC_TO_MICROSEC = 1000;

        TaskQueue(const char *name) : queue_(name) {}
        ~TaskQueue() = default;

        void PostTask(const ThreadUtilFunc &func, uint64_t delayTime, const std::string &name);
        void PostDelayTask(const ThreadUtilFunc &func, uint64_t delayTime, const std::string &name);
        void RemoveTask(const std::string &name);
        int GetQueueId(void);

    private:
        ffrt::queue queue_;
        ffrt::mutex delayTaskVecMutex_ {};
        std::vector<std::shared_ptr<DelayTask>> delayTaskVec_ {};
    };

    impl();
    ~impl() = default;
    std::shared_ptr<TaskQueue> CreateTaskQueue(int threadId);

    SafeMap<int, std::shared_ptr<TaskQueue>> taskQueueMap_ {};
};

ThreadUtil::impl::impl()
{}

ThreadUtil::ThreadUtil() : pimpl(std::make_unique<impl>())
{
    for (int i = 0; i < THREAD_ID_MAX; i++) {
        threadStateMap_.EnsureInsert(i, ThreadState::ENABLED);
    }
}

ThreadUtil::~ThreadUtil()
{
    threadStateMap_.Clear();
}

void ThreadUtil::impl::TaskQueue::PostDelayTask(const ThreadUtilFunc &func, uint64_t delayTime, const std::string &name)
{
    CHECK_AND_RETURN_LOG(
        delayTime < DELAY_TIME_MS_MAX, "Invalid delaytime(%{public}llu), taskName(%{public}s)", delayTime, name.c_str());

    {
        std::lock_guard<ffrt::mutex> lock(delayTaskVecMutex_);
        // Remove the delayed task if it's triggered, or it's a same task.
        for (auto it = delayTaskVec_.begin(); it != delayTaskVec_.end();) {
            if ((*it)->name == name || (*it)->trigger.load()) {
                queue_.cancel((*it)->taskHandle);
                it = delayTaskVec_.erase(it);
            } else {
                it++;
            }
        }
    }

    // Push a new delayed task
    std::shared_ptr<DelayTask> delayTask = std::make_shared<DelayTask>();
    CHECK_AND_RETURN_LOG(delayTask, "delayTask is nullptr");

    ffrt::task_attr taskAttr;
    taskAttr.name(name.c_str()).delay(delayTime * MILLISEC_TO_MICROSEC);
    auto taskFunc = [delayTask, func]() {
        delayTask->trigger = true;
        func();
    };

    auto taskHandle = queue_.submit_h(taskFunc, taskAttr);
    CHECK_AND_RETURN_LOG(taskHandle, "ffrt submit task failed");

    delayTask->name = name;
    delayTask->taskHandle = std::move(taskHandle);

    std::lock_guard<ffrt::mutex> lock(delayTaskVecMutex_);
    delayTaskVec_.push_back(delayTask);
}

void ThreadUtil::impl::TaskQueue::PostTask(const ThreadUtilFunc &func, uint64_t delayTime, const std::string &name)
{
    if (delayTime > 0) {
        PostDelayTask(func, delayTime, name);
        return;
    }

    queue_.submit(func);
}

void ThreadUtil::impl::TaskQueue::RemoveTask(const std::string &name)
{
    std::lock_guard<ffrt::mutex> lock(delayTaskVecMutex_);
    auto it = std::find_if(
        delayTaskVec_.begin(), delayTaskVec_.end(), [&name](auto &task) { return task->name == name; });
    if (it == delayTaskVec_.end()) {
        return;
    }

    queue_.cancel((*it)->taskHandle);
    delayTaskVec_.erase(it);
}

int ThreadUtil::impl::TaskQueue::GetQueueId(void)
{
    // Get the ffrt queue id, for debugging
    int id = -1;
    auto handle = queue_.submit_h([&id]() {
        id = ffrt::get_queue_id();
    });
    queue_.wait(handle);
    return id;
}

#define CHECK_THREAD_STATE_RETURN(threadId, func) \
do { \
    auto state = ThreadState::DISABLED; \
    threadStateMap_.Find((threadId), state); \
    if (state == ThreadState::NOT_SWITCH_THREAD) { \
        func(); \
    } \
    CHECK_AND_RETURN_LOG( \
        state == ThreadState::ENABLED, "threadId %{public}s is no enabled", GetThreadName(threadId).c_str()); \
} while (0)

void ThreadUtil::PostTask(int threadId, const ThreadUtilFunc &func, uint64_t delayTime, const std::string &name)
{
    // Check thread state for unit test.
    CHECK_THREAD_STATE_RETURN(threadId, func);

    std::shared_ptr<impl::TaskQueue> taskQueue = nullptr;
    {
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
        std::lock_guard<ffrt::mutex> lock(mutex_);
#endif
        if (pimpl->taskQueueMap_.Find(threadId, taskQueue) && taskQueue != nullptr) {
            taskQueue->PostTask(func, delayTime, name);
            return;
        }
        // If the thread not found, create it.
        taskQueue = pimpl->CreateTaskQueue(threadId);
    }
    // Execute the first task.
    if (taskQueue) {
        taskQueue->PostTask(func, delayTime, name);
    }
}

void ThreadUtil::RemoveTask(int threadId, const std::string &name)
{
    std::shared_ptr<impl::TaskQueue> taskQueue = nullptr;
    if (pimpl->taskQueueMap_.Find(threadId, taskQueue) && taskQueue != nullptr) {
        taskQueue->RemoveTask(name);
        return;
    }
}

std::shared_ptr<ThreadUtil::impl::TaskQueue> ThreadUtil::impl::CreateTaskQueue(int threadId)
{
    std::string threadName = GetThreadName(threadId);
    auto taskQueue = std::make_shared<TaskQueue>(threadName.c_str());
    CHECK_AND_RETURN_LOG_RET(taskQueue, nullptr, "Create %{public}s TaskQueue failed", threadName.c_str());

    int queueId = taskQueue->GetQueueId();
    LOG_INFO("bt_ffrt_queue: queueId(%{public}d),  name(%{public}s)", queueId, threadName.c_str());

    taskQueueMap_.EnsureInsert(threadId, taskQueue);
    return taskQueue;
}

// -------------------------- Unit Test Used ------------------------------------
// Only used for unit test to enable/disable thread.
void ThreadUtil::ChangeThreadState(int threadId, ThreadState state)
{
    threadStateMap_.EnsureInsert(threadId, state);
}

ThreadUtil &ThreadUtil::GetInstance()
{
    static ThreadUtil instance;
    return instance;
}

}  // namespace bluetooth
}  // namespace OHOS
