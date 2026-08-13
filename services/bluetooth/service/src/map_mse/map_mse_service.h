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

#ifndef MAP_MSE_SERVICE_H
#define MAP_MSE_SERVICE_H

#include <map>

#include "btcommon/timer_manager.h"
#include "interface_profile_map_mse.h"
#include "context.h"
#include "base_observer_list.h"
#include "btcommon/dynamic_library_loader.h"
#include "map_mse_service_impl_interface.h"
#include "../obex/obex_server_socket.h"
#include "../obex/obex_socket_observer.h"
#include "obex_reject_server.h"
#include "obex_server_session.h"
#include "../sdp_adapter/sdp_adapter.h"

namespace OHOS {
namespace bluetooth {
class MapMseService : public IProfileMapMse, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtmap_mse.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateMapMseServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyMapMseServiceImplInterface";
    // Unload the dynamic library after 10min Calling the UnloadMediaInterfaceLib interface.
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;
    /**
     * @brief A constructor used to create an MapMseService instance.
     *
     * @since 6.0
     */
    MapMseService();

    /**
     * @brief A destructor used to delete the MapMseService instance.
     *
     * @since 6.0
     */
    ~MapMseService() override;

    /**
     * @brief Load the dynamic library, may cost 20ms ~ 50ms。
     *
     * @return void
     */
    void LoadMapMseServiceImplInterfaceLib(void);

    /**
     * @brief Unload the dynamic library. The dynamic library is really unloaded after unloadTimerMs_ milliseconds.
     *
     * @return void
     */
    void UnloadMapMseServiceImplInterfaceLib(void);

    /**
     * @brief Enable the target service.
     *
     * @since 6.0
     */
    void Enable() override;

    /**
     * @brief Disable the target service.
     *
     * @since 6.0
     */
    void Disable() override;

    /**
     * @brief Get the instance of the MapMseService.
     *
     * @return Returns the instance of the MapMseService.
     * @since 6.0
     */
    utility::Context *GetContext() override;

    /**
     * @brief Get the instance of the MapMseService.
     *
     * @return Returns the instance of the MapMseService.
     * @since 6.0
     */
    static MapMseService *GetService();

    /**
     * @brief Connect to the peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>BT_NO_ERROR</b> Perform normal connection processing.
     *         Returns <b>RET_BAD_STATUS</b> Target device is on connected,or connecting.
     *         Returns <b>RET_NO_SUPPORT</b> Target device is not allowed to connect,or the connection fails.
     * @since 6.0
     */
    int Connect(const RawAddress &device) override;

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
    int Disconnect(const RawAddress &device) override;

    /**
     * @brief Get connected device.
     *
     * @return Returns Connected devices.
     * @since 6.0
     */
    std::list<RawAddress> GetConnectDevices() override;

    /**
     * @brief Get connect state of local device.
     *
     * @return Returns <b>PROFILE_STATE_DISCONNECTED</b> if device connection state is disconnected;
     *         Returns <b>PROFILE_STATE_DISCONNECTING</b> if device connection state is disconnecting;
     *         Returns <b>PROFILE_STATE_CONNECTED</b> if device connection state is connected;
     *         Returns <b>PROFILE_STATE_CONNECTING</b> if device connection state is connecting;
     * @since 6.0
     */
    int GetConnectState() override;

    /**
     * @brief Get max number of connected device.
     *
     * @return Returns max connect number.
     * @since 6.0
     */
    int GetMaxConnectNum() override;

    /**
     * @brief Get devices by connection states.
     *
     * @param states The connection states of the bluetooth device.
     * @return Returns devices that match the connection states.
     * @since 6.0
     */
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;

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
    int GetDeviceState(const RawAddress &device) override;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int SetConnectionStrategy(const RawAddress &device, int strategy) override;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 6.0
     */
    int GetConnectionStrategy(const RawAddress &device) override;

    int32_t SetMessageAccessAuthorization(const RawAddress &device, int32_t accessAuthorization) override;

    int32_t GetMessageAccessAuthorization(const RawAddress &device, int32_t &accessAuthorization) override;

    /**
     * @brief register observer
     * @details register observer for the service
     * @param observer the pointer that point to a MapMseObserver
     * @return void
     */
    void RegisterObserver(IMapMseObserver &observer) override;

    /**
     * @brief deregister observer
     * @details deregister observer for the service
     * @param observer the pointer that point to a MapMseObserver
     * @return void
     */
    void DeregisterObserver(IMapMseObserver &observer) override;

    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice, int32_t masInstanceId);
    void ReStartWhenSocketServerException(int32_t masInstanceId);
    void SetIsNeedReconnClient(const bool isNeedReconnClient);
    void SdpFoundCallback(const MapMnsSdpFoundRecord &record);
    void Clear();

private:
    void EnableNative();
    void DisableNative();
    void StartSocketListener(int masId);
    void StopSocketListener();
    void UpdateDbIdentifier();
    int GetDbIdentifier();
    void DealOnConnect(const std::shared_ptr<ObexSocketDevice> &socketDevice, int32_t masInstanceId);
    void CheckOrGetPermission(const std::string &address);
    void RequestPermissionTimeout();
    void StopRequestPermissionTimer();
    int ConnectSocket(const std::string &addr, const Uuid &uuid, int securityFlag, int type, int psm);
    void RejectConnection();

private:
    std::atomic_long dbIndetifier_ = 0;
    bool sdpSearchDone_ = false;
    bool requestPermissionFlag = false;
    bool isNeedReconnClient_ = false;
    std::map<int, int> masIdToSdpHandle_{};
    CxxDynamicLibraryLoader<MapMseServiceImplInterface> loader_;

    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    std::shared_ptr<ObexServerSocket> obexServerSocket_;
    std::shared_ptr<IObexSocketObserver> socketObserver_;
    std::shared_ptr<IMapMseObserver> mapMseConnectionObserver_;
    std::shared_ptr<utility::Timer> requestPermissionTimer_ = nullptr;
    std::mutex obexRejectServerSessionMutex_;
    std::shared_ptr<ObexServerSession> obexRejectServerSession_ = nullptr;
};

class MapMseSocketObserver : public IObexSocketObserver {
public:
    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice) override;
};

class MapMseConnectionObserver : public IMapMseObserver {
public:
    ~MapMseConnectionObserver() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause) override;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif