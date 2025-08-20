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

#ifndef TAIHE_BLUETOOTH_HID_HOST_OBSERVER_H_
#define TAIHE_BLUETOOTH_HID_HOST_OBSERVER_H_

#include <shared_mutex>
#include "bluetooth_hid_host.h"
#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {

class TaiheBluetoothHidHostObserver : public HidHostObserver {
public:
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause) override;
    
    TaiheBluetoothHidHostObserver();
    ~TaiheBluetoothHidHostObserver() override = default;

private:
    friend class HidHostProfileImpl;
    EventModule<void(::ohos::bluetooth::baseProfile::StateChangeParam const& data)> eventSubscribe_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_HID_HOST_OBSERVER_H_ */
