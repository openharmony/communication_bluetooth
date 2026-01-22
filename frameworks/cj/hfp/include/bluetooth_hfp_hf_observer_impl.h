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

#ifndef BLUETOOTH_HFP_HF_OBSERVER_IMPL_H
#define BLUETOOTH_HFP_HF_OBSERVER_IMPL_H

#include <shared_mutex>

#include "bluetooth_hfp_ag.h"
#include "bluetooth_hfp_utils.h"
#include "ffi_remote_data.h"
#include <mutex>

namespace OHOS {
namespace Bluetooth {
const char* const STR_BT_HANDS_FREE_UNIT_OBSERVER_CONNECTION_STATE_CHANGE = "connectionStateChange";

class HandsFreeUnitObserverImpl : public HandsFreeAudioGatewayObserver, public OHOS::FFI::FFIData {
    DECL_TYPE(HandsFreeUnitObserverImpl, OHOS::FFI::FFIData);

public:
    HandsFreeUnitObserverImpl() {};
    ~HandsFreeUnitObserverImpl() override = default;
    void RegisterStateChangeFunc(std::function<void(StateChangeParam)> cjCallback);
    void OnConnectionStateChanged(const BluetoothRemoteDevice& device, int state, int cause) override;
    void OnScoStateChanged(const BluetoothRemoteDevice& device, int state, int reason = 0) override {};

    void OnActiveDeviceChanged(const BluetoothRemoteDevice& device) override {}
    void OnHfEnhancedDriverSafetyChanged(const BluetoothRemoteDevice& device, int indValue) override {}
    void OnHfpStackChanged(const BluetoothRemoteDevice& device, int action) override {}

private:
    std::function<void(StateChangeParam)> stateChangeFunc { nullptr };
    std::mutex mtx_;
};

} // namespace Bluetooth
} // namespace OHOS
#endif /* BLUETOOTH_HFP_HF_OBSERVER_IMPL_H */