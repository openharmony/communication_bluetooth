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

#ifndef DYNAMIC_LIBRARY_LOADER
#define DYNAMIC_LIBRARY_LOADER

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "../../../service/src/base/base_def.h"
#include "safe_map.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
constexpr const char *DLL_UNLOAD_STATE = "Unload";
constexpr const char *DLL_LOADED_STATE = "Loaded";
constexpr const char *DLL_WAIT_FOR_UNLOAD_STATE = "WaitForUnload";

class DynamicLibraryLoader : public utility::StateMachine {
public:
    static constexpr uint32_t DEFAULT_UNLOAD_DELAY_TIME = 600000;  // 10min

    enum STATE_MACHINE_EVENT {
        INVALID_EVENT = 0,
        LOAD_EVENT = 1,
        UNLOAD_EVENT,
        UNLOAD_TIMER_EVENT,  // Delay unload timer trigger
    };

    explicit DynamicLibraryLoader(std::string libPath);
    DynamicLibraryLoader(std::string libPath, uint32_t unloadDelayTime,
        std::function<void(const ThreadUtilFunc &func)> threadFunc = nullptr);
    virtual ~DynamicLibraryLoader() override;

    void OpenLib(void);
    void *GetSymbol(std::string name);
    void CloseLib(void);
    virtual bool LoadTask();
    virtual bool UnloadTask();
    bool IsLibraryLoaded(void);

    // Notify the dynamic library is closed
    virtual void OnDynamicLibraryClosed(void) {}

private:
    void *handle_ = nullptr;  // dlxxx() handle.
    std::string libPath_ {};  // dynamic library path
    std::atomic_int64_t loadTimes_ = 0;
    uint32_t unloadDelayTime_ = DEFAULT_UNLOAD_DELAY_TIME;
    SafeMap<std::string, void *> symbolMap_ {};  // Used to cache symbol
    std::mutex stateMachineMutex_ {};  // StateMachine is not thread safe, need to lock.
    std::function<void(const ThreadUtilFunc &func)> threadFunc_ = nullptr;

    friend class DynamicLibraryLoaderUnloadState;
    friend class DynamicLibraryLoaderLoadedState;
    friend class DynamicLibraryLoaderWaitForUnloadState;

    BT_DISALLOW_COPY_AND_ASSIGN(DynamicLibraryLoader);
};

class DynamicLibraryLoaderState : public utility::StateMachine::State {
public:
    DynamicLibraryLoaderState(const std::string &name, DynamicLibraryLoader &stateMachine)
        : State(name, stateMachine), sm_(stateMachine) {}
    virtual ~DynamicLibraryLoaderState() = default;

protected:
    DynamicLibraryLoader &sm_;
};

class DynamicLibraryLoaderUnloadState : public DynamicLibraryLoaderState {
public:
    explicit DynamicLibraryLoaderUnloadState(DynamicLibraryLoader &stateMachine)
        : DynamicLibraryLoaderState(DLL_UNLOAD_STATE, stateMachine) {}

    ~DynamicLibraryLoaderUnloadState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

class DynamicLibraryLoaderLoadedState : public DynamicLibraryLoaderState {
public:
    explicit DynamicLibraryLoaderLoadedState(DynamicLibraryLoader &stateMachine)
        : DynamicLibraryLoaderState(DLL_LOADED_STATE, stateMachine) {}

    ~DynamicLibraryLoaderLoadedState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

class DynamicLibraryLoaderWaitForUnloadState : public DynamicLibraryLoaderState {
public:
    explicit DynamicLibraryLoaderWaitForUnloadState(DynamicLibraryLoader &stateMachine)
        : DynamicLibraryLoaderState(DLL_WAIT_FOR_UNLOAD_STATE, stateMachine),
        delayUnloadTimer_([this]() { DelayUnloadTimerCallback(); })
    {}

    ~DynamicLibraryLoaderWaitForUnloadState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;

private:
    void DelayUnloadTimerCallback(void);

    utility::Timer delayUnloadTimer_;
};

/************************************ Interface ***********************************************************/

class CDynamicLibraryLoader : public DynamicLibraryLoader {
public:
    explicit CDynamicLibraryLoader(std::string libPath) : DynamicLibraryLoader(libPath) {}
    CDynamicLibraryLoader(std::string libPath, uint32_t unloadDelayTime)
        : DynamicLibraryLoader(libPath, unloadDelayTime) {}
    ~CDynamicLibraryLoader() override = default;

    void OnDynamicLibraryClosed(void) override {}

private:
    BT_DISALLOW_COPY_AND_ASSIGN(CDynamicLibraryLoader);
};

template <typename T>
class CxxDynamicLibraryLoader : public DynamicLibraryLoader {
public:
    using AllocatorFunc = T *(*)(void);
    using DeleterFunc = void (*)(T *);

    CxxDynamicLibraryLoader(
        std::string libPath,
        std::string allocClassSymbol = "BtDynamicLibraryAllocator",
        std::string deleteClassSymbol = "BtDynamicLibraryDeleter")
        : DynamicLibraryLoader(libPath), allocClassSymbol_(allocClassSymbol), deleteClassSymbol_(deleteClassSymbol) {}

    CxxDynamicLibraryLoader(
        std::string libPath,
        uint32_t unloadDelayTime,
        std::string allocClassSymbol = "BtDynamicLibraryAllocator",
        std::string deleteClassSymbol = "BtDynamicLibraryDeleter")
        : DynamicLibraryLoader(libPath, unloadDelayTime),
          allocClassSymbol_(allocClassSymbol),
          deleteClassSymbol_(deleteClassSymbol) {}

    ~CxxDynamicLibraryLoader() override = default;

    /**
     * @brief: 函数返回的智能指针必须在 CloseLib() 接口前销毁析构，尽量即用即取
     * @return
     */
    std::shared_ptr<T> GetLibInstance(void)
    {
        std::lock_guard<std::mutex> lock(libClassPtrMutex_);
        if (libClassPtr_) {
            return libClassPtr_;
        }

        AllocatorFunc allocator = reinterpret_cast<AllocatorFunc>(GetSymbol(allocClassSymbol_));
        if (!allocator) {
            return nullptr;
        }
        DeleterFunc deleter = reinterpret_cast<DeleterFunc>(GetSymbol(deleteClassSymbol_));
        if (!deleter) {
            return nullptr;
        }

        libClassPtr_ = std::shared_ptr<T>(allocator(), [deleter](T *p) { deleter(p); });
        return libClassPtr_;
    }

    void OnDynamicLibraryClosed(void) override
    {
        std::lock_guard<std::mutex> lock(libClassPtrMutex_);
        libClassPtr_ = nullptr;
    }

private:
    // DynamicLibraryLoader loader_;
    std::string allocClassSymbol_ {};  // The symbol of new a c++ class.
    std::string deleteClassSymbol_ {};  // The symbol of delete a c++ class.

    std::mutex libClassPtrMutex_ {};
    std::shared_ptr<T> libClassPtr_ {nullptr};

    BT_DISALLOW_COPY_AND_ASSIGN(CxxDynamicLibraryLoader);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // DYNAMIC_LIBRARY_LOADER
