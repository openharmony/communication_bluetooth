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

#ifndef LOG_TAG
#define LOG_TAG "bt_common_dynamic_loader"
#endif

#include "btcommon/dynamic_library_loader.h"

#include <dlfcn.h>
#include "bluetooth_log.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
using utility::StateMachine;
using std::make_unique;

DynamicLibraryLoader::DynamicLibraryLoader(std::string libPath)
    : DynamicLibraryLoader(libPath, DEFAULT_UNLOAD_DELAY_TIME)
{}

DynamicLibraryLoader::DynamicLibraryLoader(std::string libPath, uint32_t unloadDelayTime,
    std::function<void(const ThreadUtilFunc &func)> threadFunc)
    : libPath_(libPath), unloadDelayTime_(unloadDelayTime), threadFunc_(threadFunc)
{
    /*  UNLOAD State -> LOADED State (LOAD_EVENT)
        LOADED State -> WAIT_FOR_UNLOAD state (UNLOAD_EVENT: if loadTimes == 0)
        LOADED State -> LOADED State (LOAD_EVENT or UNLOAD_EVENT: if loadTimes > 0)
        WAIT_FOR_UNLOAD State -> LOADED State (LOAD_EVENT)
        WAIT_FOR_UNLOAD State -> UNLOAD State (UNLOAD_TIMER_EVENT)
     */
    std::unique_ptr<StateMachine::State> unload = make_unique<DynamicLibraryLoaderUnloadState>(*this);
    StateMachine::Move(unload);
    std::unique_ptr<StateMachine::State> loaded = make_unique<DynamicLibraryLoaderLoadedState>(*this);
    StateMachine::Move(loaded);
    std::unique_ptr<StateMachine::State> waitForUnload = make_unique<DynamicLibraryLoaderWaitForUnloadState>(*this);
    StateMachine::Move(waitForUnload);

    StateMachine::InitState(DLL_UNLOAD_STATE);
    HILOGI("DLL_SM(%{public}s): UNLOAD_STATE, reason: state machine created", libPath_.c_str());
}

DynamicLibraryLoader::~DynamicLibraryLoader()
{
}

static const char *EventToString(int event)
{
    switch (event) {
        case DynamicLibraryLoader::LOAD_EVENT: return "LOAD_EVENT";
        case DynamicLibraryLoader::UNLOAD_EVENT: return "UNLOAD_EVENT";
        case DynamicLibraryLoader::UNLOAD_TIMER_EVENT: return "UNLOAD_TIMER_EVENT";
        default: break;
    }
    return "Unknown";
}

#define DLL_LOG_DEBUG_EVENT(stateStr, eventId) \
    HILOGD("DLL_SM(%{public}s): recv event(%{public}s) in " stateStr, sm_.libPath_.c_str(), EventToString(eventId))


void DynamicLibraryLoaderUnloadState::Entry()
{}

void DynamicLibraryLoaderUnloadState::Exit()
{}

__attribute__((no_sanitize("cfi")))
bool DynamicLibraryLoaderUnloadState::Dispatch(const utility::Message &utilMsg)
{
    HITRACE_METER(BT_TRACE_TAG);
    DLL_LOG_DEBUG_EVENT("UNLOAD_STATE", utilMsg.what_);

    switch (utilMsg.what_) {
        case DynamicLibraryLoader::LOAD_EVENT: {
            if (sm_.LoadTask()) {
                Transition(DLL_LOADED_STATE);
                break;
            }
            return false;
        }
        default:
            HILOGE("DLL_SM(%{public}s): recv unsupported event(%{public}s) in UNLOAD_STATE",
                sm_.libPath_.c_str(), EventToString(utilMsg.what_));
            break;
    }

    return true;
}

void DynamicLibraryLoaderLoadedState::Entry()
{
    sm_.loadTimes_ = 1;  // first loaded
}

void DynamicLibraryLoaderLoadedState::Exit()
{
    sm_.loadTimes_ = 0;  // Clear load times
}

bool DynamicLibraryLoaderLoadedState::Dispatch(const utility::Message &utilMsg)
{
    HITRACE_METER(BT_TRACE_TAG);
    DLL_LOG_DEBUG_EVENT("LOADED_STATE", utilMsg.what_);

    switch (utilMsg.what_) {
        case DynamicLibraryLoader::LOAD_EVENT: {
            sm_.loadTimes_++;
            break;
        }
        case DynamicLibraryLoader::UNLOAD_EVENT: {
            sm_.loadTimes_--;
            if (sm_.loadTimes_.load() <= 0) {
                HILOGI("DLL_SM(%{public}s): LOADED_STATE->WAIT_FOR_UNLOAD_STATE, "
                    "reason: recv UNLOAD_EVENT and loadTimes is less than or equal to 0", sm_.libPath_.c_str());
                Transition(DLL_WAIT_FOR_UNLOAD_STATE);
            }
            break;
        }
        default:
            HILOGE("DLL_SM(%{public}s): recv unsupported event(%{public}s) in LOADED_STATE",
                sm_.libPath_.c_str(), EventToString(utilMsg.what_));
            break;
    }

    return true;
}

// Call in timer thread.
void DynamicLibraryLoaderWaitForUnloadState::DelayUnloadTimerCallback(void)
{
    HITRACE_METER(BT_TRACE_TAG);
    utility::Message msg(DynamicLibraryLoader::UNLOAD_TIMER_EVENT);
    if (sm_.threadFunc_) {
        sm_.threadFunc_([this, msg]() {
             std::lock_guard<std::mutex> lock(sm_.stateMachineMutex_);
             sm_.ProcessMessage(msg);
         });
    } else {
        std::lock_guard<std::mutex> lock(sm_.stateMachineMutex_);
        sm_.ProcessMessage(msg);
    }
}

__attribute__((no_sanitize("cfi")))
void DynamicLibraryLoaderWaitForUnloadState::Entry()
{
    // Start timer
    delayUnloadTimer_.Start(sm_.unloadDelayTime_);
    // Notify the dynamic library is closed.
    sm_.OnDynamicLibraryClosed();
}

void DynamicLibraryLoaderWaitForUnloadState::Exit()
{}

bool DynamicLibraryLoaderWaitForUnloadState::Dispatch(const utility::Message &utilMsg)
{
    DLL_LOG_DEBUG_EVENT("WAIT_FOR_UNLOAD_STATE", utilMsg.what_);

    switch (utilMsg.what_) {
        case DynamicLibraryLoader::LOAD_EVENT: {
            HILOGI("DLL_SM(%{public}s): WAIT_FOR_UNLOAD_STATE->LOADED_STATE, "
                "reason: recv LOAD_EVENT", sm_.libPath_.c_str());
            // Stop timer
            delayUnloadTimer_.Stop();
            Transition(DLL_LOADED_STATE);
            break;
        }
        case DynamicLibraryLoader::UNLOAD_TIMER_EVENT: {
            if (sm_.UnloadTask()) {
                Transition(DLL_UNLOAD_STATE);
                break;
            };
            return false;
        }
        default:
            HILOGE("DLL_SM(%{public}s): recv unsupported event(%{public}s) in WAIT_FOR_UNLOAD_STATE",
                sm_.libPath_.c_str(), EventToString(utilMsg.what_));
            break;
    }
    return true;
}

void DynamicLibraryLoader::OpenLib(void)
{
    std::lock_guard<std::mutex> lock(stateMachineMutex_);

    utility::Message msg(LOAD_EVENT);
    ProcessMessage(msg);
}

__attribute__((no_sanitize("cfi")))
void *DynamicLibraryLoader::GetSymbol(std::string name)
{
    std::lock_guard<std::mutex> lock(stateMachineMutex_);

    CHECK_AND_RETURN_LOG_RET(GetState() && GetState()->Name() == DLL_LOADED_STATE, nullptr,
        "DLL_SM %{public}s is not loaded", libPath_.c_str());

    void *symbol = nullptr;
    if (symbolMap_.Find(name, symbol)) {
        return symbol;
    }

    symbol = dlsym(handle_, name.c_str());
    CHECK_AND_RETURN_LOG_RET(symbol, nullptr, "DLL_SM dlsym %{public}s failed: %{public}s", name.c_str(), dlerror());

    symbolMap_.EnsureInsert(name, symbol);
    return symbol;
}

__attribute__((no_sanitize("cfi")))
void DynamicLibraryLoader::CloseLib()
{
    std::lock_guard<std::mutex> lock(stateMachineMutex_);

    utility::Message msg(UNLOAD_EVENT);
    ProcessMessage(msg);
}

bool DynamicLibraryLoader::LoadTask()
{
    // handle 判空
    handle_ = dlopen(libPath_.c_str(), RTLD_LAZY);
    CHECK_AND_RETURN_LOG_RET(handle_, false, "DLL_SM(%{public}s): open failed", libPath_.c_str());
    HILOGI("DLL_SM(%{public}s): UNLOAD_STATE->LOADED_STATE, reason: recv LOAD_EVENT", libPath_.c_str());
    return true;
}
 
bool DynamicLibraryLoader::UnloadTask()
{
    int ret = dlclose(handle_);
    if (ret != 0) {
        HILOGE("DLL_SM dlclose %{public}s failed: %{public}s", libPath_.c_str(), dlerror());
        return false;
    }
    handle_ = nullptr;
    symbolMap_.Clear();
    HILOGI("DLL_SM(%{public}s): WAIT_FOR_UNLOAD_STATE->UNLOAD_STATE, "
        "reason: recv UNLOAD_TIMER_EVENT", libPath_.c_str());
    return true;
}

bool DynamicLibraryLoader::IsLibraryLoaded()
{
    if (loadTimes_.load() <= 0) {
        return false;
    }
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
