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

#include <mutex>
#include "event_handler.h"  // libeventhandler
#include "event_runner.h"  // libeventhandler
#include "log.h"

using OHOS::AppExecFwk::EventRunner;
using OHOS::AppExecFwk::EventHandler;

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
    };

    auto it = threadNameMap.find(threadId);
    if (it == threadNameMap.end()) {
        HILOGE("Not find threadId: %{public}d", threadId);
        return "Unknown";
    }

    return it->second;
}

struct ThreadUtil::impl {
    impl();
    ~impl() = default;

    std::shared_ptr<OHOS::AppExecFwk::EventHandler> CreateThread(int threadId);

    SafeMap<int, std::shared_ptr<OHOS::AppExecFwk::EventHandler>> eventHandlerMap_ {};
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
 
    std::shared_ptr<EventHandler> eventHandler = nullptr;
    if (pimpl->eventHandlerMap_.Find(threadId, eventHandler) && eventHandler != nullptr) {
        eventHandler->PostTask(func, name, delayTime);
        return;
    }
    // If the thread not found, create it.
    eventHandler = pimpl->CreateThread(threadId);
    // Execute the first task.
    if (eventHandler) {
        eventHandler->PostTask(func, name, delayTime);
    }
}

void ThreadUtil::RemoveTask(int threadId, const std::string &name)
{
    std::shared_ptr<EventHandler> eventHandler = nullptr;
    if (pimpl->eventHandlerMap_.Find(threadId, eventHandler) && eventHandler != nullptr) {
        eventHandler->RemoveTask(name);
        return;
    }
}

std::shared_ptr<EventHandler> ThreadUtil::impl::CreateThread(int threadId)
{
    std::string threadName = GetThreadName(threadId);
    HILOGI("Create thread (%{public}s)", threadName.c_str());

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(threadName);
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>(eventRunner);
    CHECK_AND_RETURN_LOG_RET(eventHandler, nullptr, "Create eventHandler failed");
    eventHandlerMap_.EnsureInsert(threadId, eventHandler);
    return eventHandler;
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
