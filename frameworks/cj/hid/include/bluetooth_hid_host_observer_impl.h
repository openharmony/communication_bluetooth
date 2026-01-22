/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_HID_HOST_OBSERVER_IMPL_H
#define BLUETOOTH_HID_HOST_OBSERVER_IMPL_H

#include "bluetooth_hid_host.h"
#include "bluetooth_hid_host_utils.h"
#include "ffi_remote_data.h"
#include <mutex>

namespace OHOS {
namespace Bluetooth {
const char* const STR_BT_HID_HOST_OBSERVER_CONNECTION_STATE_CHANGE = "connectionStateChange";

class BluetoothHidHostObserverImpl : public HidHostObserver, public OHOS::FFI::FFIData {
    DECL_TYPE(BluetoothHidHostObserverImpl, OHOS::FFI::FFIData);

public:
    BluetoothHidHostObserverImpl() {};
    ~BluetoothHidHostObserverImpl() override = default;
    void RegisterStateChangeFunc(std::function<void(StateChangeParam)> cjCallback);
    void OnConnectionStateChanged(const BluetoothRemoteDevice& device, int state, int cause) override;

private:
    std::function<void(StateChangeParam)> stateChangeFunc { nullptr };
    std::mutex mtx_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // BLUETOOTH_HID_HOST_OBSERVER_IMPL_H
