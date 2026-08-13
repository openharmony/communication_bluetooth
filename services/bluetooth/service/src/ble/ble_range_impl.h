/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SERVICE_BLE_RANGE_IMPL_H
#define BLUETOOTH_SERVICE_BLE_RANGE_IMPL_H

#include <memory>
#include <mutex>
#include <future>
#include <string>
#include <vector>
#include <dlfcn.h>
#include "cJSON.h"
#include "bt_types.h"
#include "bt_def.h"
#include "satellite_manager.h"

namespace OHOS {
namespace bluetooth {

using RangingAntSwitchHandler = void (*)(uint8_t macID, uint8_t antID);

enum class SATELLITE_CONTROL_MODE : int32_t {
    ANTENNA = 0,
    BLUETOOTH_SWITCH = 1,
    SIGNALHUB_MAC_SWITCH = 2,
};

class BleRangeImpl : public std::enable_shared_from_this<BleRangeImpl> {
public:
    BleRangeImpl();
    ~BleRangeImpl();
    void Init();

    int GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType);
    int BleRestoreRangingAntSwitch(bluetooth::BleAppType appType);

    int GetCurrentAdvPower() const;
    void SetCurrentAdvPower(int advPower);
    int32_t GetAdvPower() const;
    int32_t GetAdvPowerByAntInfo();

    struct AntSwitchInfo {
        uint8_t macID;
        uint8_t antID;
    };
    
    AntSwitchInfo GetRangingAntSwitchInfo() const;

private:
    uint32_t configMac_;
    uint32_t configAnt_;
    uint32_t defaultMac_;

    void HandleRangingSwitching(bluetooth::BleAppType appType, int advPower);

    static void OnRangingAntSwitchCallbackStatic(uint8_t macID, uint8_t antID);
    void OnRangingAntSwitchCallbackInternal(uint8_t macID, uint8_t antID);

    bool SwitchMacIfNeeded(uint8_t currentMacId, uint8_t targetMacId);
    int HandleNonRangingAppType(bluetooth::BleAppType appType, int advPower);
    void RegisterAntSwitchCallback();
    bool SendBleAdvStartMsg();
    void SendAntennaInfoQueryMsg();
    bool PrepareAndSendBleAdv(int advPower);
    void WaitForAntSwitchCallbackAndHandleResult(uint8_t defaultMacId, uint8_t targetMacId, int advPower);

    int32_t InitGetBleRangeParam();
    void StoreRangingAntSwitchInfo(uint8_t macID, uint8_t antID);
    void ParseAdvPowerAntFromConfig(cJSON* configItem);

    const bt_interface_t* btInterface_;
    std::shared_ptr<SatelliteManager> satelliteManager_;

    std::mutex rangingMutex_;
    std::shared_ptr<std::promise<bool>> rangingPromise_;
    int32_t advPower_;
    std::vector<int32_t> advPowerMac0Ant_;
    std::vector<int32_t> advPowerMac1Ant_;
    std::vector<int32_t> advPowerFoldedMac0Ant_;
    std::vector<int32_t> advPowerFoldedMac1Ant_;

    std::atomic<int8_t> currentAdvPower_;
    static constexpr int RANGING_ANT_SWITCH_TIMEOUT_MS = 50;

    static std::weak_ptr<BleRangeImpl> s_currentInstance;
    static std::mutex s_instanceMutex;

    mutable std::mutex antSwitchInfoMutex_;
    uint8_t cachedMacID_;
    uint8_t cachedAntID_;

    bool hasSwitchedMac_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_SERVICE_BLE_RANGE_IMPL_H
