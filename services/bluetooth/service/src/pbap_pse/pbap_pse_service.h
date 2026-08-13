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

#ifndef PBAP_PSE_SERVICE_H
#define PBAP_PSE_SERVICE_H

#include <cstring>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include "safe_map.h"
#include "base_def.h"
#include "base_observer_list.h"
#include "btcommon/dynamic_library_loader.h"
#include "context.h"
#include "interface_profile_pbap_pse.h"
#include "btcommon/message.h"
#include "pbap_pse_def.h"
#include "pbap_pse_service_impl_interface.h"
#include "raw_address.h"
#include "../obex/obex_server_socket.h"
#include "../obex/obex_socket_observer.h"
#include "bt_recursive_mutex.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief phone book server service
 * process connect event
 */
class PbapPseService : public IProfilePbapPse, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtpbap_pse.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreatePbapServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyPbapServiceImplInterface";
    // Unload the dynamic library after 10min Calling the UnloadMediaInterfaceLib interface.
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;
    /**
     * @brief constructor
     *
     */
    PbapPseService();

    /**
     * @brief deconstructor
     *
     */
    ~PbapPseService() override;

    /**
     * @brief Load the dynamic library, may cost 20ms ~ 50ms。
     *
     * @return void
     */
    void LoadPbapPseServiceManagerInterfaceLib(void);

    /**
     * @brief Unload the dynamic library. The dynamic library is really unloaded after unloadTimerMs_ milliseconds.
     *
     * @return void
     */
    void UnloadPbapPseServiceManagerInterfaceLib(void);

    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    void RegisterObserver(IPbapPseObserver &observer) override;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    void DeregisterObserver(IPbapPseObserver &observer) override;

    /**
     * @brief get the remote devices with the specified states
     *
     * @param states states
     * @return std::vector remote devices
     */
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;

    /**
     * @brief get the state of remote device
     *
     * @param device  remote device
     * @return int state
     */
    int32_t GetDeviceState(const RawAddress &device) override;

    /**
     * @brief get context
     *
     * @return Context*
     */
    utility::Context *GetContext() override;

    /**
     * @brief start service
     *
     */
    void Enable() override;

    /**
     * @brief Disable service
     *
     */
    void Disable() override;

    /**
     * @brief connect to remote device
     * @details not support
     */
    int32_t Connect(const RawAddress &device) override;

    /**
     * @brief disconnect remote device
     *
     * @return result
     */
    int32_t Disconnect(const RawAddress &device) override;

    /**
     * @brief not used
     * @details not support
     */
    std::list<RawAddress> GetConnectDevices() override;

    /**
     * @brief Get Connect State for base service
     *
     */
    int32_t GetConnectState() override;

    /**
     * @brief Get Max Connect Num for base service
     * @details not support
     */
    int32_t GetMaxConnectNum() override;

    /**
     * @brief Set the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param strategy Reference to the connection policy,
     * @return result.
     */
    int32_t SetConnectionStrategy(const RawAddress &device, int strategy) override;

    /**
     * @brief Get the connection policy of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @return connection policy.
     */
    int32_t GetConnectionStrategy(const RawAddress &device) const override;

    /**
     * @brief Set the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param shareType Reference to the share type
     * @return result.
     */
    int32_t SetShareType(const RawAddress& device, int32_t shareType) override;

    /**
     * @brief Get the share type of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] shareType Reference to the share type
     * @return result.
     */
    int32_t GetShareType(const RawAddress& device, int32_t &shareType) override;

    /**
     * @brief Set the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param accessAuthorization Reference to the access authorization
     * @return result.
     */
    int32_t SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization) override;

    /**
     * @brief Get the phonebook access authorization of the specified device.
     *
     * @param device Reference to the remote bluetooth device.
     * @param[out] accessAuthorization Reference to the access authorization
     * @return result.
     */
    int32_t GetPhoneBookAccessAuthorization(const RawAddress& device, int32_t &accessAuthorization) override;

    static PbapPseService *GetService();

    void StartSocketListener();
    void StopSocketListener();
    void ReStartWhenSocketServerException();

    void LoadLibAndConnect(std::shared_ptr<ObexSocketDevice> &socketDevice);

    void NotifyStateChanged(const RawAddress &device, int state, int cause);
private:
    void EnableService();
    void DisableService();

    void CreateSdpRecord();
    void CleanSdpRecord();

    void NotifyPhonebookAuthResultToHfp(const RawAddress& device, int32_t accessAuthorization, int32_t pbapLoadFlag);

private:
    int32_t mSdpHandle = -1;
    // The mutex variable
    BtRecursiveMutex mutex_ {};

    std::shared_ptr<ObexServerSocket> obexServerSocket_;
    std::shared_ptr<IObexSocketObserver> socketObserver_;
    std::shared_ptr<IPbapPseObserver> pbapPseConnectionObserver_;

    CxxDynamicLibraryLoader<PbapPseServiceImplInterface> loader_;
    bool isLoaded_ = false;
    BaseObserverList<IPbapPseObserver> pbapPseObservers_ {};
    std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap_ {};
    DECLARE_IMPL();
    BT_DISALLOW_COPY_AND_ASSIGN(PbapPseService);
};

class PbapPseSocketObserver : public IObexSocketObserver {
public:
    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice) override;
};

class PbapPseConnectionObserver : public IPbapPseObserver {
public:
    ~PbapPseConnectionObserver() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause) override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PSE_SERVICE_H
