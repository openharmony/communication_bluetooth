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

#ifndef BLUETOOTH_MAP_MSE_H
#define BLUETOOTH_MAP_MSE_H

#include <memory>
#include <string>
#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for MAP MSE observer functions.
 *
 */
class MapMseObserver {
public:
    /**
     * @brief Destroy the MapMseObserver object.
     *
     */
    virtual ~MapMseObserver() = default;

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
 * @brief Class for MapMse Server API.
 *
 */
class BLUETOOTH_API MapMse {
public:
    /**
     * @brief Get the instance of MapMse object.
     *
     * @return Returns the pointer to the MapMse instance.
     */
    static MapMse *GetProfile();

    /**
     * @brief Register MapMse observer instance.
     *
     * @param observer MapMse observer instance.
     */
    void RegisterObserver(std::shared_ptr<MapMseObserver> observer);

    /**
     * @brief Deregister MapMse observer instance.
     *
     * @param observer MapMse observer instance.
     */
    void DeregisterObserver(std::shared_ptr<MapMseObserver> observer);

    /**
     * @brief Get remote map mse device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @param result the list of devices
     * @return Returns operate result.
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRemoteDevice> &result) const;

    /**
     * @brief Get the connection state of the specified remote map device.
     *
     * @param device Remote device object.
     * @param state the connection state of the remote device
     * @return Returns operate result.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const;

    /**
     * @brief Release the connection from remote map device.
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
     * @brief Set meta data for remote bluetooth device.
     *
     * @param device Remote device object.
     * @param accessAuthorization The accessAuthorization.
     * @return Returns operate result
     */
    int32_t SetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t accessAuthorization);

    /**
     * @brief Get meta data strategy of remote bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param accessAuthorization out accessAuthorization.
     * @return Returns operate result.
     */
    int32_t GetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t &accessAuthorization) const;

    void Init();
    void Uinit();

private:
    MapMse();
    ~MapMse();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(MapMse);
    BLUETOOTH_DECLARE_IMPL();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_MAP_MSE_H