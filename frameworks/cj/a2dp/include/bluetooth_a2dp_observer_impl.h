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

#ifndef BLUETOOTH_A2DP_OBSERVER_IMPL_H
#define BLUETOOTH_A2DP_OBSERVER_IMPL_H

#include <shared_mutex>

#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_utils.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace Bluetooth {

const char* const STR_BT_A2DP_SOURCE_CONNECTION_STATE_CHANGE = "connectionStateChange";

class A2dpSourceObserverImpl : public A2dpSourceObserver, public OHOS::FFI::FFIData {
    DECL_TYPE(A2dpSourceObserverImpl, OHOS::FFI::FFIData);

public:
    A2dpSourceObserverImpl() {};
    ~A2dpSourceObserverImpl() override = default;
    void OnConnectionStateChanged(const BluetoothRemoteDevice& device, int state, int cause) override;
    void RegisterStateChangeFunc(std::function<void(StateChangeParam)> cjCallback);

private:
    friend class A2dpSourceProfileImpl;
    std::function<void(StateChangeParam)> stateChangeFunc { nullptr };
};

} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_BLUETOOTH_A2DP_SRC_OBSERVER_H_ */