/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_bas_callback"
#endif

#include "bluetooth_battery_info.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_bas_callback.h"
#include "napi_native_object.h"

namespace OHOS {
namespace Bluetooth {

NapiBasCallback::NapiBasCallback()
    : eventSubscribe_(STR_BT_BAS_CALLBACK_BATTERY_LEVEL_CHANGE, BT_MODULE_NAME)
{}

void NapiBasCallback::OnGetBatteryLevelEvent(const BluetoothRemoteDevice &device, int32_t batteryLevel)
{
    BasBatteryInfo batteryInfo;
    batteryInfo.deviceId_ = device.GetDeviceAddr();
    batteryInfo.batteryLevel_ = batteryLevel;
    auto nativeObject = std::make_shared<NapiNativeBasBatteryInfo>(batteryInfo);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::BAS_GET_BATTERY_LEVEL, nativeObject,
        static_cast<int>(BT_NO_ERROR));
}

void NapiBasCallback::OnBatteryLevelChanged(const BluetoothRemoteDevice &device, int32_t batteryLevel)
{
    BasBatteryInfo batteryInfo;
    batteryInfo.deviceId_ = device.GetDeviceAddr();
    batteryInfo.batteryLevel_ = batteryLevel;
    auto nativeObject = std::make_shared<NapiNativeBasBatteryInfo>(batteryInfo);
    eventSubscribe_.PublishEvent(STR_BT_BAS_CALLBACK_BATTERY_LEVEL_CHANGE, nativeObject);
}
} // namespace Bluetooth
} // namespace OHOS
#endif