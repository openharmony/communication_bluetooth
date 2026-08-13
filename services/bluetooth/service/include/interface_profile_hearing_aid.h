/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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

/**
 * @file interface_profile_hearing_aid.h
 *
 * @brief  Defines hearing aid profile functions
 *
 * @since 20
 */

#ifndef INTERFACE_PROFILE_HEARING_AID_H
#define INTERFACE_PROFILE_HEARING_AID_H

#include "interface_profile.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief Callback function api of Hearing Aid service, including connection, disconnection.
 *
 * @since 20
 */
class IHearingAidObserver {
public:
    /**
     * @brief A destructor used to delete the Hearing Aid Service Observer instance.
     *
     * @since 20
     */
    virtual ~IHearingAidObserver() = default;

    /**
     * @brief ConnectionState Changed observer.
     * @param device bluetooth device address.
     * @param state Connection state.
     * @param cause Disconnection cause.
     * @since 20
     */
    virtual void OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause) {};

    /**
     * @brief Active Changed observer.
     * @param remoteAddr bluetooth device address,00:00:00:00:00:00 means no active device.
     * @param name bluetooth device name.
     * @since 20
     */
    virtual void OnActiveDeviceChanged(const RawAddress &remoteAddr, const std::string& name) {};
};

/**
 * @brief This class provides functions called by Framework API.
 *
 * @since 20
 */
class IProfileHearingAid : public IProfile {
public:
    /**
     * @brief Get a2dp source service instance.
     *
     * @return Returns an instance of a2dp source service.
     * @since 20
     */
    static IProfileHearingAid *GetProfile();

    /**
     * @brief Get devices by connection states.
     *
     * @param states The connection states of the bluetooth device.
     * @return Returns devices that match the connection states.
     * @since 20
     */
    virtual std::vector<RawAddress> GetDevicesByStates(std::vector<int> &states) = 0;

    /**
     * @brief Get device connection state by address.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>A2DP_DISCONNECTED</b> if device connect state is disconnected;
     *         Returns <b>A2DP_DISCONNECTING</b> if device connect state is disconnecting;
     *         Returns <b>A2DP_CONNECTED</b> if device connect state is connected;
     *         Returns <b>A2DP_CONNECTING</b> if device connect state is connecting;
     *         Returns <b>A2DP_INVALID_STATUS</b> if target device is not in device list;
     * @since 20
     */
    virtual int GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief Set target device as active device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or set fails.
     * @since 20
     */
    virtual int SetActiveDevice(const RawAddress &device) = 0;

    /**
     * @brief Set target device volume.
     *
     * @param device The address of the peer bluetooth device.
     * @param volume volume want to set.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or set fails.
     * @since 20
     */
    virtual int SetVolumeByAddr(const RawAddress &device, int volume) = 0;

    /**
     * @brief Get active device.
     * @param device The address of the peer bluetooth device.
     * @param name The name of the peer bluetooth device.
     * @return Returns active device.
     * @since 20
     */
    virtual void GetActiveDevice(RawAddress &addr, std::string &name) = 0;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 20
     */
    virtual int SetConnectStrategy(const RawAddress &device, int strategy) = 0;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 20
     */
    virtual int GetConnectStrategy(const RawAddress &device) = 0;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param side The device side, left is 0, right is 1.
     * @param mode 0 is monaural, 1 is binaural.
     * @param hiSyncId The ID of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 20
     */
    virtual int GetHearingAidDeviceInfo(const RawAddress &device, int& side, int mode, int64_t& hiSyncId) = 0;

    /**
     * @brief Register observer function of framework.
     *
     * @param observer The observer function pointer of framework.
     * @since 20
     */
    virtual void RegisterObserver(IHearingAidObserver &observer) = 0;

    /**
     * @brief Deregister observer function of framework.
     *
     * @since 20
     */
    virtual void DeregisterObserver(IHearingAidObserver &observer) = 0;

    /**
     * @brief get hearing aid support.
     *
     * @return return le2m supported
     * @since 20
     */
    virtual bool IsLe2MPhySupported() = 0;

    /**
     * @brief Start Stack Profile.
     */
    virtual void StartStackProfile() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_PROFILE_HEARING_AID_H