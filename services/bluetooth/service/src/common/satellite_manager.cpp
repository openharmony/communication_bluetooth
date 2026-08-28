/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_satellite"
#endif

#include "satellite_manager.h"

#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "datetime_ex.h"
#include "preferences_manager.h"

using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

SatelliteManager::SatelliteManager()
{}

SatelliteManager::~SatelliteManager()
{}

void SatelliteManager::Init(void)
{
    timer_ = std::make_shared<utility::Timer>([satelliteWeakPtr = weak_from_this()]() {
        HILOGI("SatelliteManager timer triggered");
        auto satelliteSharedPtr = satelliteWeakPtr.lock();
        CHECK_AND_RETURN_LOG(satelliteSharedPtr, "satelliteWeakPtr is nullptr");
        satelliteSharedPtr->DeleteSatelliteControlConfig();
    });

    isSatelliteControlBtSwitch_ =
        PreferencesManager::Get(IS_BT_SWITCH_CONTROL_ENABLED_KEY, false, PreferencesManagerType::SATELLITE_CONTROL);
    if (IsSatelliteControlBtSwitch()) {
        int64_t enableTimes =
            PreferencesManager::Get<int64_t>(BT_SWITCH_TIME_KEY, 0, PreferencesManagerType::SATELLITE_CONTROL);
        int64_t curTimes = GetSecondsSince1970ToNow();
        HILOGI("curTime: %{public}ld, enableTime: %{public}ld", curTimes, enableTimes);
        if (curTimes <= enableTimes) {
            HILOGW("BtSwitchTime value is invalid: %{public}d", enableTimes);
            DeleteSatelliteControlConfig();
            return;
        }
        if (curTimes - enableTimes >= TWO_MIN_TIMEOUT) {  // 120 is 2min
            HILOGI("BtSwitchTime is timeout, clear satellite config");
            DeleteSatelliteControlConfig();
            return;
        }
        // 0 < curTimes - enableTimes < 120
        timer_->Start((curTimes - enableTimes) * SEC_TO_MILLISEC);
    }
}

int32_t SatelliteManager::ProcessControlAntennaEvent(int state)
{
    HILOGI("antenna sate is %{public}d", state);
    const BthwifInterface *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, BT_ERR_INTERNAL_ERROR, "Failed to get bthwif interface handle.");
    bthwif->hwSatelliteModeSwitch(state);
    return BT_NO_ERROR;
}

int32_t SatelliteManager::ProcessControlBtSwitchEvent(int state)
{
    HILOGI("satellite control bt switch is %{public}d", state);
    // 1 means satellite control bluetooth switch, ohter's can't not enable bluetooth.
    isSatelliteControlBtSwitch_ = (state == 1);
    if (IsSatelliteControlBtSwitch()) {
        // save config file
        int64_t timeSeconds = GetSecondsSince1970ToNow();
        PreferencesManager::Save(BT_SWITCH_TIME_KEY, timeSeconds, PreferencesManagerType::SATELLITE_CONTROL);
        PreferencesManager::Save(IS_BT_SWITCH_CONTROL_ENABLED_KEY, isSatelliteControlBtSwitch_.load(),
            PreferencesManagerType::SATELLITE_CONTROL);
        // start 2 min timer to auto recovery
        if (timer_) {
            timer_->Start(timeoutMs_);
        }
    } else {
        DeleteSatelliteControlConfig();
        if (timer_) {
            timer_->Stop();
        }
    }
    return BT_NO_ERROR;
}

int32_t SatelliteManager::SatelliteControl(int type, int state)
{
    if (type == static_cast<int>(SATELLITE_CONTROL_MODE::ANTENNA)) {
        return ProcessControlAntennaEvent(state);
    }
    if (type == static_cast<int>(SATELLITE_CONTROL_MODE::BLUETOOTH_SWITCH)) {
        return ProcessControlBtSwitchEvent(state);
    }
    if (type == static_cast<int>(SATELLITE_CONTROL_MODE::SIGNALHUB_MAC_SWITCH)) {
        return ProcessSetMacIdEvent(state);
    }
    return BT_ERR_INTERNAL_ERROR;
}

void SatelliteManager::DeleteSatelliteControlConfig(void)
{
    isSatelliteControlBtSwitch_ = false;
    PreferencesManager::Delete(BT_SWITCH_TIME_KEY, PreferencesManagerType::SATELLITE_CONTROL);
    PreferencesManager::Delete(IS_BT_SWITCH_CONTROL_ENABLED_KEY, PreferencesManagerType::SATELLITE_CONTROL);
}

int32_t SatelliteManager::ProcessSetMacIdEvent(int state)
{
    HILOGI("Process SetMacId Event, state:%{public}d", state);
    const BthwifInterface *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, BT_ERR_INTERNAL_ERROR, "Failed to get bthwif interface handle.");
    bthwif->hwSetMacId(state);
    return BT_NO_ERROR;
}

}  // namespace bluetooth
}  // namespace OHOS
