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
#ifndef BLUETOOTH_HID_HOST_IMPL_H
#define BLUETOOTH_HID_HOST_IMPL_H

#include "bluetooth_hid_host.h"
#include "bluetooth_hid_host_observer_impl.h"
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHidHostImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(BluetoothHidHostImpl, OHOS::FFI::FFIData)
public:
    explicit BluetoothHidHostImpl() {};
    ~BluetoothHidHostImpl() {};
    static CArrString GetConnectionDevices(int32_t* errCode);
    static int32_t GetConnectionState(char* device, int32_t* errCode);
    static void On(int32_t type, int64_t id, int32_t* errCode);
    static std::shared_ptr<BluetoothHidHostObserverImpl> observer_;
    static bool isRegistered_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // BLUETOOTH_HID_HOST_IMPL_H