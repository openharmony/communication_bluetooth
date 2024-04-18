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

#ifndef BLUETOOTH_PBAP_PSE_H
#define BLUETOOTH_PBAP_PSE_H

#include <memory>
#include <string>
#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for Pbap PSE observer functions.
 *
 */
class PbapPseObserver {
public:
    /**
     * @brief Destroy the PbapPseObserver object.
     *
     */
    virtual ~PbapPseObserver() = default;

    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     * @param cause Connection cause.
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int32_t state, int32_t cause)
    {}
};

/**
 * @brief Class for Pbap Server API.
 *
 */
class BLUETOOTH_API PbapPse {
public:
    /**
     * @brief Get the instance of PbapPse object.
     *
     * @return Returns the pointer to the PbapPse instance.
     */
    static PbapPse *GetProfile();

    /**
     * @brief Register PbapPse observer instance.
     *
     * @param observer PbapPse observer instance.
     */
    void RegisterObserver(std::shared_ptr<PbapPseObserver> observer);

    /**
     * @brief Deregister PbapPse observer instance.
     *
     * @param observer PbapPse observer instance.
     */
    void DeregisterObserver(std::shared_ptr<PbapPseObserver> observer);

    /**
     * @brief Get remote pbap pse device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @param result the list of devices
     * @return Returns operate result.
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRemoteDevice> &result) const;

    /**
     * @brief Get the connection state of the specified remote pbap device.
     *
     * @param device Remote device object.
     * @param state the connection state of the remote device
     * @return Returns operate result.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const;

    /**
     * @brief Release the connection from remote pbap device.
     *
     * @param device Remote device object.
     * @return Returns operate result.
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

    /**
     * @brief Set connection strategy for remote bluetooth device.
     *
     * @param device Remote device object.
     * @param strategy The device connect strategy.
     * @return Returns operate result
     */
    int32_t SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy);

    /**
     * @brief Get connection strategy of remote bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns operate result.
     */
    int32_t GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const;

    /**
     * @brief Set share type for remote bluetooth device.
     *
     * @param device Remote device object.
     * @param shareType The share type.
     * @return Returns operate result
     */
    int32_t SetShareType(const BluetoothRemoteDevice &device, int32_t shareType);

    /**
     * @brief Get share type of remote bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param shareType The share type.
     * @return Returns operate result.
     */
    int32_t GetShareType(const BluetoothRemoteDevice &device, int32_t &shareType) const;

    /**
     * @brief Set accessAuthorization for remote bluetooth device.
     *
     * @param device Remote device object.
     * @param accessAuthorization The accessAuthorization.
     * @return Returns operate result
     */
    int32_t SetPhoneBookAccessAuthorization(const BluetoothRemoteDevice &device, int32_t accessAuthorization);

    /**
     * @brief Get accessAuthorization of remote bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param accessAuthorization The accessAuthorization.
     * @return Returns operate result.
     */
    int32_t GetPhoneBookAccessAuthorization(const BluetoothRemoteDevice &device, int32_t &accessAuthorization) const;

    void Init();
    void Uinit();

private:
    /**
     * @brief constructor.
     *
     */
    PbapPse();

    /**
     * @brief deconstructor.
     *
     */
    ~PbapPse();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPse);
    BLUETOOTH_DECLARE_IMPL();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PBAP_PSE_H