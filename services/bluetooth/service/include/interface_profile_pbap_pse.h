/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INTERFACE_PROFILE_PBAP_PSE_H
#define INTERFACE_PROFILE_PBAP_PSE_H

#include "interface_profile.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obsever for pbap server
 * when pbap server occur event, call these
 */
class IPbapPseObserver {
public:
    /**
     * @brief  ConnectionState Changed
     *
     * @param  device     bluetooth address
     * @param  state      changed status
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
    virtual ~IPbapPseObserver(){};
};

/**
 * @brief pbap pce service
 * pbap pce service
 */
class IProfilePbapPse : public IProfile {
public:
    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    virtual void RegisterObserver(IPbapPseObserver &observer) = 0;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    virtual void DeregisterObserver(IPbapPseObserver &observer) = 0;

    /**
     * @brief get the remote devices with the specified states
     *
     * @param states states
     * @return std::vector remote devices
     */
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;

    /**
     * @brief get the state of remote device
     *
     * @param device  remote device
     * @return int state
     */
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief Set the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param strategy Reference to the connection policy,
     * @return result.
     */
    virtual int32_t SetConnectionStrategy(const RawAddress &device, int32_t strategy) = 0;

    /**
     * @brief Get the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @return connection policy.
     */
    virtual int32_t GetConnectionStrategy(const RawAddress &device) const = 0;

    /**
     * @brief Set the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param shareType Reference to the share type
     * @return result.
     */
    virtual int32_t SetShareType(const RawAddress& device, int32_t shareType) = 0;

    /**
     * @brief Get the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] shareType Reference to the share type
     * @return result.
     */
    virtual int32_t GetShareType(const RawAddress& device, int32_t &shareType) = 0;

    /**
     * @brief Set the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param accessAuthorization Reference to the access authorization
     * @return result.
     */
    virtual int32_t SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization) = 0;

    /**
     * @brief Get the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] accessAuthorization Reference to the access authorization
     * @return result.
     */
    virtual int32_t GetPhoneBookAccessAuthorization(const RawAddress& device, int32_t &accessAuthorization) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // INTERFACE_PROFILE_PBAP_PSE_H