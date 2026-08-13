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

#ifndef MAP_MSE_SERVICE_IMPL_INTERFACE_H
#define MAP_MSE_SERVICE_IMPL_INTERFACE_H

#include "raw_address.h"
#include "obex_socket_device.h"
#include "interface_profile_map_mse.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "bt_uuid.h"

namespace OHOS {
namespace bluetooth {
class MapMseServiceImplInterface {
public:
    MapMseServiceImplInterface() = default;
    virtual ~MapMseServiceImplInterface() = default;

    virtual void StartUp(const MapMnsSdpFoundRecord &record,
        std::shared_ptr<ObexSocketDevice> &socketDevice, const bool isReconnClient) = 0;

    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a IMapMseObserver
     * @return void
     */
    virtual void RegisterObserver(std::shared_ptr<IMapMseObserver> &observer) = 0;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a IMapMseObserver
     * @return void
     */
    virtual void DeregisterObserver(std::shared_ptr<IMapMseObserver> &observer) = 0;

    /**
     * @brief Disconnect with the peer bluetooth service.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>BT_NO_ERROR</b> if perform normal disconnection processing.
     *         Returns <b>RET_BAD_PARAM</b> if target device is not in the device list.
     *         Returns <b>BT_OPERATION_FAILED</b> if target device is on disconnected,or disconnecting.
     *         Returns <b>RET_NO_SUPPORT</b> if disconnection fails.
     * @since 6.0
     */
    virtual int Disconnect(const RawAddress &device) = 0;

    virtual void OnClose(int32_t masInstanceId) = 0;
    
    /**
     * @brief Get connected device.
     *
     * @return Returns Connected devices.
     * @since 6.0
     */
    virtual std::list<RawAddress> GetConnectDevices() = 0;

    /**
     * @brief Get connect state of local device.
     *
     * @return Returns <b>PROFILE_STATE_DISCONNECTED</b> if device connection state is disconnected;
     *         Returns <b>PROFILE_STATE_DISCONNECTING</b> if device connection state is disconnecting;
     *         Returns <b>PROFILE_STATE_CONNECTED</b> if device connection state is connected;
     *         Returns <b>PROFILE_STATE_CONNECTING</b> if device connection state is connecting;
     * @since 6.0
     */
    virtual int32_t GetConnectState() = 0;

    /**
     * @brief Get device connection state by address.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>DISCONNECTED</b> if device connect state is disconnected;
     *         Returns <b>DISCONNECTING</b> if device connect state is disconnecting;
     *         Returns <b>CONNECTED</b> if device connect state is connected;
     *         Returns <b>CONNECTING</b> if device connect state is connecting;
     *         Returns <b>INVALID_STATUS</b> if target device is not in device list;
     * @since 6.0
     */
    virtual int GetDeviceState(const RawAddress &device) = 0;

    virtual void DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc) = 0;

    virtual void DllRegisterGetDbIndetifierFunc(const std::function<int()> &func) = 0;

    virtual void DllSetIsNeedReconnClientFunc(const std::function<void(const bool isNeedReconnClient)> &func) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // MAP_MSE_SERVICE_IMPL_INTERFACE_H