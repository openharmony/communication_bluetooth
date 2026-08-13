/*
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

#ifndef A2DP_SERVICE_DEVICE_H
#define A2DP_SERVICE_DEVICE_H

#include <cstdint>
#include <map>

#include "a2dp_service_state_machine.h"
#include "bt_def.h"
#include "interface_profile_a2dp_src.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Single device information management,including device address,playing state,
 *        connection state,state machine,etc.
 *
 * @since 6.0
 */
class A2dpDeviceInfo {
public:
    /**
     * @brief A constructor used to create an <b>A2dpDeviceInfo</b> instance.
     *
     * @since 6.0
     */
    explicit A2dpDeviceInfo(const RawAddress &device);

    /**
     * @brief A destructor used to delete the <b>A2dpDeviceInfo</b> instance.
     *
     * @since 6.0
     */
    ~A2dpDeviceInfo();

    /**
     * @brief Save codec status in Device information.
     *
     * @param codecStatusInfo The codec information.
     * @since 6.0
     */
    void SetCodecStatus(A2dpSrcCodecStatus codecStatusInfo);

    /**
     * @brief Save playing state in Device information.
     *
     * @param state The playing state of device.
     * @since 6.0
     */
    void SetPlayingState(bool state);

    /**
     * @brief Save connection state in Device information..
     *
     * @param state The connection state of device.
     * @since 6.0
     */
    void SetConnectState(int state);

    /**
     * @brief Get device from Device information.
     *
     * @return Return the device address
     * @since 6.0
     */
    RawAddress GetDevice() const;

    /**
     * @brief Get state machine from Device information.
     *
     * @return Returns the device statemachine
     * @since 6.0
     */
    A2dpStateManager *GetStateMachine();

    /**
     * @brief Get codec status from Device information.
     *
     * @return Returns the device code information
     * @since 6.0
     */
    A2dpSrcCodecStatus GetCodecStatus() const;

    /**
     * @brief Get playing state from Device information.
     *
     * @return Returns <b>true</b> if device is on playing;
     *         Returns <b>false</b> if device is not on playing.
     * @since 6.0
     */
    bool GetPlayingState() const;

    /**
     * @brief Get connect state from Device information.
     *
     * @return Returns <b>DISCONNECTED</b> if device connect state is disconnected;
     *         Returns <b>DISCONNECTING</b> if device connect state is disconnecting;
     *         Returns <b>CONNECTED</b> if device connect state is connected;
     *         Returns <b>CONNECTING</b> if device connect state is connecting;
     * @since 6.0
     */
    int GetConnectState() const;

    /**
     * @brief Save offload codec status in Device information.
     *
     * @param offloadCodecStatus The offload codec information.
     * @since 6.0
     */
    void SetOffloadCodecStatus(A2dpSrcOffloadCodecStatus offloadCodecStatus);

    /**
     * @brief Get offload codec status from Device information.
     *
     * @return Returns the device offload codec information
     * @since 6.0
     */
    A2dpSrcOffloadCodecStatus GetOffloadCodecStatus() const;

    /**
     * @brief set try reconnectA2dp.
     *
     * @param tryToReconnect tryToReconnect
     * @since 12.0
     */
    void SetTryReconnectA2dp(bool tryToReconnect);

    /**
     * @brief set try reconnectA2dp.
     *
     * @return returns tryToReconnect
     * @since 12.0
     */
    bool GetTryReconnectA2dp() const;

    /**
     * @brief get pre connectstate.
     *
     * @return returns PreConnectState
     * @since 12.0
     */
    int GetPreConnectState() const;

    /**
     * @brief get hdap connectstate.
     *
     * @return returns connectstate
     * @since 12.0
     */
    bool IsHdapConnected() const;

    /**
     * @brief set hdap connectstate.
     *
     * @param isHdapConnected isHdapConnected
     * @since 12.0
     */
    void SetIsHdapConnected(bool isHdapConnected);

    /**
     * @brief get codec info.
     *
     * @return returns codec info
     * @since 12.0
     */
    const A2dpSrcCodecInfo &GetCodecInfo() const;

    /**
     * @brief set codec info.
     *
     * @param codecInfo hdap Device codecInfo
     * @since 12.0
     */
    void SetCodecInfo(const A2dpSrcCodecInfo &codecInfo);

private:
    A2dpDeviceInfo() = delete;
    RawAddress peerAddress_;
    // The codec status information.
    A2dpSrcCodecStatus codecStatus_ {};
    // The offload codec status information.
    A2dpSrcOffloadCodecStatus offloadCodecStatus_ {};
    // The playing state of device.
    bool isPlaying_ = false;
    // The connection state of device.
    int currentConnectState_ = static_cast<int>(BTConnectState::DISCONNECTED);
    // The pointer of device's state machine.
    A2dpStateManager state_ {};
    int preConnectState_ = static_cast<int>(BTConnectState::DISCONNECTED);
    bool tryToReconnect_ = false;
    bool isHdapConnected_ = false;
    A2dpSrcCodecInfo codecInfo_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SERVICE_DEVICE_H