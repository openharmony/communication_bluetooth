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

#ifndef BLUETOOTH_BATTERY_INFO_H
#define BLUETOOTH_BATTERY_INFO_H

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_raw_address.h"
#include "../parcel/bluetooth_device_battery_info.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief bluetooth battery information define.
 *
 * @since 12
 */

/**
 * @brief Enum for the charge state.
 *
 * @since 12
 */

enum class DeviceChargeState : int32_t {
    // not support super charge, and not charged
    DEVICE_NORMAL_CHARGE_NOT_CHARGED = 0,

    // not support super charge, and in charged
    DEVICE_NORMAL_CHARGE_IN_CHARGING = 1,

    // support super charge, and not charged
    DEVICE_SUPER_CHARGE_NOT_CHARGED = 2,

    // support super charge, and in charged
    DEVICE_SUPER_CHARGE_IN_CHARGING = 3,
};

struct DeviceBatteryInfo {
    // The device
    std::string deviceId_;

    // Electricity value of the general device. -1 means no power infomation.
    int32_t batteryLevel_;

    // Electricity value of the left ear. -1 means no power infomation.
    int32_t leftEarBatteryLevel_;

    // The charge state of the left ear.
    DeviceChargeState leftEarChargeState_;

    // Electricity value of the right ear. -1 means no power infomation.
    int32_t rightEarBatteryLevel_;

    // The charge state of the right ear.
    DeviceChargeState rightEarChargeState_;

    // Electricity value of the box. -1 means no power infomation.
    int32_t boxBatteryLevel_;

    // The charge state of the box
    DeviceChargeState boxChargeState_;

    DeviceBatteryInfo()
    {
        deviceId_ = "";
        batteryLevel_ = -1;
        leftEarBatteryLevel_ = -1;
        leftEarChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_NOT_CHARGED;
        rightEarBatteryLevel_ = -1;
        rightEarChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_NOT_CHARGED;
        boxBatteryLevel_ = -1;
        boxChargeState_ = DeviceChargeState::DEVICE_NORMAL_CHARGE_NOT_CHARGED;
    };

    DeviceBatteryInfo(const std::string &address, const BluetoothBatteryInfo &batteryInfo)
    {
        deviceId_ = address;
        batteryLevel_ = batteryInfo.batteryLevel_;
        leftEarBatteryLevel_ = batteryInfo.leftEarBatteryLevel_;
        leftEarChargeState_ = static_cast<DeviceChargeState>(batteryInfo.leftEarChargeState_);
        rightEarBatteryLevel_ = batteryInfo.rightEarBatteryLevel_;
        rightEarChargeState_ = static_cast<DeviceChargeState>(batteryInfo.rightEarChargeState_);
        boxBatteryLevel_ = batteryInfo.boxBatteryLevel_;
        boxChargeState_ = static_cast<DeviceChargeState>(batteryInfo.boxChargeState_);
    };
};
} // namespace Bluetooth
} // namespace OHOS
#endif