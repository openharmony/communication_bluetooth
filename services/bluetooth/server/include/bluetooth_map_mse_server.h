/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_MAP_MSE_SERVER_H
#define BLUETOOTH_MAP_MSE_SERVER_H

#include "i_bluetooth_map_mse.h"
#include "bluetooth_map_mse_stub.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"
#include "bluetooth_types.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothMapMseServer : public BluetoothMapMseStub {
public:
    /**
     * @brief constructor
     * @details constructor
     */
    explicit BluetoothMapMseServer();

    /**
     * @brief deconstructor
     * @details deconstructor
     */
    ~BluetoothMapMseServer() override;

    /**
     * @brief register observer
     * @details register observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothMapMseObserver
     * @return Status
     */
    void RegisterObserver(const sptr<IBluetoothMapMseObserver>& observer) override;

    /**
     * @brief deregister observer
     * @details deregister observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothMapMseObserver
     * @return Status
     */
    void DeregisterObserver(const sptr<IBluetoothMapMseObserver>& observer) override;

    /**
     * @brief get the remote devices
     * @details get the remote device with the specified states
     * @param states states
     * @return Status
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &tmpStates,
        std::vector<BluetoothRawAddress> &rawDevices) override;

    /**
     * @brief get the state of device
     * @details get the state with the specified remote device
     * @param device  remote device
     * @return Status
     */
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override;

    /**
     * @brief disconnect device
     * @details disconnect from remote device
     * @param device  remote device
     * @return Status
     */
    int32_t Disconnect(const BluetoothRawAddress &device) override;

    int32_t SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy) override;
    int32_t GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy) override;

    int32_t SetMessageAccessAuthorization(const BluetoothRawAddress& device, int32_t accessAuthorization) override;
    int32_t GetMessageAccessAuthorization(const BluetoothRawAddress& device, int32_t &accessAuthorization) override;

private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothMapMseServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_MAP_MSE_SERVER_H
