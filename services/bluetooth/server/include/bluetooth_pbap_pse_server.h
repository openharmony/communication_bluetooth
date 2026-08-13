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
#ifndef BLUETOOTH_PBAP_PSE_SERVER_H
#define BLUETOOTH_PBAP_PSE_SERVER_H

#include "bluetooth_pbap_pse_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_pbap_pse.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPbapPseServer : public BluetoothPbapPseStub {
public:
    /**
     * @brief constructor
     * @details constructor
     */
    explicit BluetoothPbapPseServer();

    /**
     * @brief deconstructor
     * @details deconstructor
     */
    ~BluetoothPbapPseServer() override;

    /**
     * @brief register observer
     * @details register observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothPbapPseObserver
     * @return Status
     */
    void RegisterObserver(const sptr<IBluetoothPbapPseObserver>& observer) override;

    /**
     * @brief deregister observer
     * @details deregister observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothPbapPseObserver
     * @return Status
     */
    void DeregisterObserver(const sptr<IBluetoothPbapPseObserver>& observer) override;

    /**
     * @brief get the remote devices
     * @details get the remote device with the specified states
     * @param states states
     * @return Status
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states,
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

    /**
     * @brief Set the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param strategy Reference to the connection policy,
     * @return result.
     */
    int32_t SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy) override;

    /**
     * @brief Get the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @return connection policy.
     */
    int32_t GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy) override;

    /**
     * @brief Set the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param shareType Reference to the share type
     * @return result.
     */
    int32_t SetShareType(const BluetoothRawAddress& device, int32_t shareType) override;

    /**
     * @brief Get the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] shareType Reference to the share type
     * @return result.
     */
    int32_t GetShareType(const BluetoothRawAddress& device, int32_t &shareType) override;

    /**
     * @brief Set the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param accessAuthorization Reference to the access authorization
     * @return result.
     */
    int32_t SetPhoneBookAccessAuthorization(const BluetoothRawAddress& device, int32_t accessAuthorization) override;

    /**
     * @brief Get the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] accessAuthorization Reference to the access authorization
     * @return result.
     */
    int32_t GetPhoneBookAccessAuthorization(const BluetoothRawAddress& device, int32_t &accessAuthorization) override;

private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothPbapPseServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PBAP_PSE_SERVER_H
