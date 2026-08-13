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

#ifndef INTERFACE_MAP_MSE_SERVICE_H
#define INTERFACE_MAP_MSE_SERVICE_H

#include "interface_profile.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obsever for pbap server
 * when pbap server occur event, call these
 */
class IMapMseObserver {
public:
    /**
     * @brief  ConnectionState Changed
     *
     * @param device bluetooth address
     * @param state changed status
     * @param cause Disconnection cause.
     * @since 12
     */
    virtual void OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause) = 0;

    /**
     * @brief deconstructor
     * @details deconstructor
     * @return
     * @since 6
     */
    virtual ~IMapMseObserver(){};
};

/**
 * @brief map mse service interface
 */
class IProfileMapMse : public IProfile {
public:
    /**
     * @brief register observer
     * @details register observer for the service of phone book serve
     * @param observer the ref that point to a MapMseObserver
     * @return void
     * @since 6
     */
    virtual void RegisterObserver(IMapMseObserver &observer) = 0;

    /**
     * @brief deregister observer
     * @details deregister observer for the service of phone book serve
     * @param observer the ref that point to a MapMseObserver
     * @return void
     * @since 6
     */
    virtual void DeregisterObserver(IMapMseObserver &observer) = 0;

    /**
     * @brief get the remote devices
     * @details get the remote device with the specified states
     * @param states states
     * @return std::vector remote devices
     * @since 6
     */
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;

    /**
     * @brief get the state of device
     * @details get the state with the specified remote device
     * @param device  remote device
     * @return device connection state @link{BTConnectState}
     * @since 6
     */
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief Set the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param strategy Reference to the connection policy,
     *     @c UNKNOWN : the connection policy for unkown state.
     *     @c ALLOWED : the connection policy for allowed state.
     *     @c FORBIDDEN : the connection policy for forbidden state.
     * @return Returns true if the operation is successful;returns false if the operation fails.
     * @since 6
     */
    virtual int32_t SetConnectionStrategy(const RawAddress &device, int32_t strategy) = 0;

    /**
     * @brief Get the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @return Returns the connection police of the specified bluetooth address.
     * @since 6
     */
    virtual int32_t GetConnectionStrategy(const RawAddress &device) = 0;

    virtual int32_t SetMessageAccessAuthorization(const RawAddress& device, int32_t accessAuthorization) = 0;

    virtual int32_t GetMessageAccessAuthorization(const RawAddress& device, int32_t &accessAuthorization) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif