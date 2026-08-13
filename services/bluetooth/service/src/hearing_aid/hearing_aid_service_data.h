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

#ifndef HEARING_AID_SERVICE_DATA_H
#define HEARING_AID_SERVICE_DATA_H

#include <cmath>
#include <cstring>
#include <list>
#include <memory>
#include <shared_mutex>
#include <vector>

#include "base_def.h"
#include "bt_def.h"
#include "safe_map.h"
#include "base_observer_list.h"
#include "bt_hearing_aid.h"
#include "hearing_aid_statemachine.h"
#include "raw_address.h"
#include "securec.h"
#include "bt_recursive_mutex.h"
#include "interface_profile_hearing_aid.h"
#include "hearing_aid_defines.h"
#include "gavdp/hdf_dynamic_library_loader.h"
namespace OHOS {
namespace bluetooth {

class DeviceStateCache {
public:
    void UpdateDeviceState(const std::string &addr, const int &state)
    {
        std::lock_guard<BtRecursiveMutex> lck(mutex_);
        if (state < HEARING_AID_STATE_DISCONNECTED || state > HEARING_AID_STATE_DISCONNECTING) {
            HILOGE("updateDeviceState invalid data");
            return;
        }
        stateCache_.insert_or_assign(addr, state);
    };
    int GetDeviceStateCache(const std::string &addr)
    {
        std::lock_guard<BtRecursiveMutex> lck(mutex_);
        auto it = stateCache_.find(addr);
        if (it != stateCache_.end()) {
            return it->second;
        }
        return HEARING_AID_STATE_DISCONNECTED;
    };

    void Iterator(std::function<void(const std::string, const int)> func)
    {
        std::lock_guard<BtRecursiveMutex> lck(mutex_);
        for (auto it = stateCache_.begin(); it != stateCache_.end(); it++) {
            func(it->first, it->second);
        }
    };

    void RemoveDevice(const std::string &addr)
    {
        std::lock_guard<BtRecursiveMutex> lck(mutex_);
        auto it = stateCache_.find(addr);
        if (it != stateCache_.end()) {
            stateCache_.erase(it);
        }
    };

private:
    BtRecursiveMutex mutex_;
    std::map<std::string, int> stateCache_;
};

class HearingAidServiceData {
public:
    HearingAidServiceData()
    {
        HILOGI("HearingAidServiceData construct");
    };
    ~HearingAidServiceData()
    {
        HILOGI("HearingAidServiceData destroy");
    };
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface_ = nullptr;

    // 10 from double
    static constexpr int HEARING_AID_MAX_DEFAULT_CONNECTIONS_NUMR = 10;
    // the maximum number of connection devices.
    int maxConnectionsNum_ {HEARING_AID_MAX_DEFAULT_CONNECTIONS_NUMR};

    HdfDynamicLibraryLoader& heariAidHalLoader_ = HdfDynamicLibraryLoader::GetInstance();

    BtRecursiveMutex mutex_;

    bool isInit_ {false};

    //  service status
    bool isStarted_ {false};

    bool isShuttingDown_ {false};

    std::set<std::string> hdfLoadedDevice_ = {};

    BaseObserverList<IHearingAidObserver> hearingAidObservers_ {};

    // the map of the device and sate machine
    std::map<const std::string, std::unique_ptr<HearingAidStateMachine>> stateMachines_ {};
    DeviceStateCache stateCache_{};

    SafeMap<std::string, uint8_t> mDeviceCapabilitiesMap_;
    SafeMap<std::string, uint64_t> mDeviceHiSyncIdMap_;
    std::atomic<uint64_t> mActiveDeviceHiSyncId_ = HI_SYNC_ID_INVALID;
    RawAddress lastNotifyDevice_ = RawAddress(INVALID_MAC_ADDRESS);

    // const state map
    const std::map<const int, const int> stateMap_ = {
        {HEARING_AID_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {HEARING_AID_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {HEARING_AID_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {HEARING_AID_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_SERVICE_DATA_H