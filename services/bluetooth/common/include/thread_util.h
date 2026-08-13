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

#ifndef BT_THREAD_UTIL_H
#define BT_THREAD_UTIL_H

#include <functional>
#include "safe_map.h"  // SafeMap
#include "bt_recursive_mutex.h"

namespace OHOS {
namespace bluetooth {

enum ThreadId {
    THREAD_ID_BLE = 0,
    THREAD_ID_GATT,
    THREAD_ID_AVRCP_TG,
    THREAD_ID_CLASSIC,
    THREAD_ID_A2DP,
    THREAD_ID_HFP,
    THREAD_ID_HID_HOST,
    THREAD_ID_HID_DEVICE,
    THREAD_ID_PAN,
    THREAD_ID_SOCKET,
    THREAD_ID_ADAPTER_MANAGER,
    THREAD_ID_MAP,
    THREAD_ID_PBAP,
    THREAD_ID_OPP,
    THREAD_ID_HF,
    THREAD_ID_VENDOR,
    THREAD_ID_LOW_PRIORITY,
    THREAD_ID_HEARING_AID,
    THREAD_ID_BIP,
    THREAD_ID_MAX,
};

using ThreadUtilFunc = std::function<void(void)>;


/**
 * @brief Post the task to the ble thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInBleThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the gatt thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInGattThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the avrcp tg thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInAvrcpTgThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the classic thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInClassicThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the a2dp thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInA2dpThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the hfp thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInHfpThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the hid host thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInHidHostThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the hid device thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInHidDeviceThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the pan thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInPanThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the socket thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInSocketThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the adapter manager thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInAdapterManagerThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the map thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInMapThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the pbap thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInPbapThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the opp thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInOppThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the hf thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInHfThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the product vendor thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInVendorThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the low priority thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInLowPriorityThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the heraing aid thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInHearingAidThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

/**
 * @brief Post the task to the bip thread.
 * @param func The task to be executed.
 * @param delayTime Process the event after 'delayTime' milliseconds.
 */
void DoInBipThread(const ThreadUtilFunc &func, uint64_t delayTime = 0);

class ThreadUtil {
public:
    void PostTask(
        int threadId, const ThreadUtilFunc &func, uint64_t delayTime = 0, const std::string &name = std::string());
    /**
     * Remove a task.
     *
     * @param name Name of the task.
     */
    void RemoveTask(int threadId, const std::string &name);

    static ThreadUtil &GetInstance();

private:
    // -------------------------- Unit Test Used ------------------------------------
    enum ThreadState : int {
        ENABLED = 0,  // The task function is switched normally.
        DISABLED,  // The task function is not executed.
        NOT_SWITCH_THREAD,  // The task functions is executed in the same thread.
    };
    // Only used for unit test to enable/disable thread.
    void ChangeThreadState(int threadId, ThreadState state);
    // threadId <-> thread state
    SafeMap<int, ThreadState> threadStateMap_ {};
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    ffrt::mutex mutex_{};
#endif

    ThreadUtil();
    ~ThreadUtil();

    struct impl;
    std::unique_ptr<impl> pimpl;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BT_THREAD_UTIL_H
