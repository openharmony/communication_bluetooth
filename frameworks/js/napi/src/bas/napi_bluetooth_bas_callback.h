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

#ifndef NAPI_BLUETOOTH_BAS_CALLBACK_H_
#define NAPI_BLUETOOTH_BAS_CALLBACK_H_

#include "bluetooth_bas_host.h"
#include "napi_async_callback.h"
#include "napi_event_subscribe_module.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

const char * const STR_BT_BAS_CALLBACK_BATTERY_LEVEL_CHANGE = "BatteryLevelChange";

class NapiBasCallback : public BluetoothRemoteDeviceBatteryObserver {
public:
    NapiBasCallback();
    ~NapiBasCallback() override = default;

    void OnGetBatteryLevelEvent(const BluetoothRemoteDevice &device, int32_t batteryLevel) override;
    void OnBatteryLevelChanged(const BluetoothRemoteDevice &device, int32_t batteryLevel) override;

    NapiAsyncWorkMap asyncWorkMap_ {};
    NapiEventSubscribeModule eventSubscribe_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_BLUETOOTH_BAS_CALLBACK_H_ */