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

#include "bluetooth_device_battery_info.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothBatteryInfo::Marshalling(Parcel &parcel) const
{
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(batteryLevel_), false, "write batteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(leftEarBatteryLevel_), false, "write leftEarBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(leftEarChargeState_), false, "write leftEarChargeState error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(rightEarBatteryLevel_), false, "write rightEarBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(rightEarChargeState_), false, "write rightEarChargeState error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(boxBatteryLevel_), false, "write boxBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteInt32(boxChargeState_), false, "write boxChargeState error");
    return true;
}

bool BluetoothBatteryInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothBatteryInfo *BluetoothBatteryInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothBatteryInfo *batteryInfo = new BluetoothBatteryInfo();
    if (batteryInfo != nullptr && !batteryInfo->ReadFromParcel(parcel)) {
        delete batteryInfo;
        batteryInfo = nullptr;
    }
    return batteryInfo;
}

bool BluetoothBatteryInfo::ReadFromParcel(Parcel &parcel)
{
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(batteryLevel_), false, "read batteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(leftEarBatteryLevel_), false, "read leftEarBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(leftEarChargeState_), false, "read leftEarChargeState error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(rightEarBatteryLevel_), false, "read rightEarBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(rightEarChargeState_), false, "read rightEarChargeState error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(boxBatteryLevel_), false, "read boxBatteryLevel error");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadInt32(boxChargeState_), false, "read boxChargeState error");
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS