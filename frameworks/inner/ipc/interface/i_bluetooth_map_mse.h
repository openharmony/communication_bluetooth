/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_STANDARD_MAP_MSE_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_MAP_MSE_INTERFACE_H

#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_map_mse_observer.h"
#include "../parcel/bluetooth_raw_address.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothMapMse : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothMapMse");

    virtual int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) = 0;
    virtual int32_t GetDevicesByStates(const std::vector<int32_t> &states,
        std::vector<BluetoothRawAddress> &rawDevices) = 0;
    virtual int32_t Disconnect(const BluetoothRawAddress &device) = 0;
    virtual int32_t SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy) = 0;
    virtual int32_t GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy) = 0;
    virtual int32_t SetMessageAccessAuthorization(const BluetoothRawAddress &device, int32_t accessAuthorization) = 0;
    virtual int32_t GetMessageAccessAuthorization(const BluetoothRawAddress &device, int32_t &accessAuthorization) = 0;
    virtual void RegisterObserver(const sptr<IBluetoothMapMseObserver> &observer) = 0;
    virtual void DeregisterObserver(const sptr<IBluetoothMapMseObserver> &observer) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_MAP_MSE_INTERFACE_H