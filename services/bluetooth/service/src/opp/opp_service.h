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

#ifndef OPP_SERVICE_H
#define OPP_SERVICE_H

#include <cstring>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include "base_def.h"
#include "base_observer_list.h"
#include "context.h"
#include "interface_profile_opp.h"
#include "interface_profile_socket.h"
#include "btcommon/message.h"
#include "raw_address.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "opp_message.h"
#include "opp_service_impl_interface.h"
#include "opp_statemachine.h"
#include "opp_transfer.h"
#include "opp_transfer_information.h"
#include "btcommon/dynamic_library_loader.h"
#include "../obex/obex_server_socket.h"
#include "../obex/obex_socket_observer.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief opp service
 * process connect event
 */
class OppService : public IProfileOpp, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtopp.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateOppServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyOppServiceImplInterface";
    // Unload the dynamic library after 10min Calling the UnloadMediaInterfaceLib interface.
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;
    /**
     * @brief constructor
     *
     */
    OppService();

    /**
     * @brief deconstructor
     *
     */
    ~OppService() override;

    /**
     * @brief Load the dynamic library, may cost 20ms ~ 50ms。
     *
     * @return void
     */
    void LoadOppServiceManagerInterfaceLib(void);

    /**
     * @brief Unload the dynamic library. The dynamic library is really unloaded after unloadTimerMs_ milliseconds.
     *
     * @return void
     */
    void UnloadOppServiceManagerInterfaceLib(void);

    /**
     * @brief GetDevicesByStates
     *
     * @param states the device states
     * @return devices
     */
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) override;

    /**
     * @brief GetDeviceState
     *
     * @param device the device
     * @return state
     */
    int GetDeviceState(const RawAddress &device) override;

    /**
     * @brief SetIncomingFileConfirmation
     *
     * @param accept accept or not
     * @param fd accept file fd
     * @return result
     */
    int SetIncomingFileConfirmation(const bool accept, int fd) override;

    /**
     * @brief SetLastReceivedFileUri
     *
     * @param uri uri
     * @return result
     */
    int SetLastReceivedFileUri(const std::string &uri) override;

    /**
     * @brief CancelTransfer
     *
     * @return result
     */
    int CancelTransfer() override;

    /**
     * @brief GetCurrentTransferInformation
     *
     * @return IOppTransferInformation
     */
    IOppTransferInformation GetCurrentTransferInformation() override;

    /**
     * @brief SendFile
     *
     * @param device the destinst device
     * @param fileHolders files to send
     * @return errorCode
     */
    int SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders) override;

    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a OppObserver
     * @return void
     */
    void RegisterObserver(IOppObserver &observer) override;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a OppObserver
     * @return void
     */
    void DeregisterObserver(IOppObserver &observer) override;

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
     * @brief get context
     *
     * @return Context*
     */
    utility::Context *GetContext() override;

    static OppService *GetService();

    /**
     * @brief start opp socket listen
     *
     * @return Context*
     */
    void StartSocketListener();
    void StopSocketListener();
    void ReStartWhenSocketServerException();

    void LoadLibAndConnect(std::shared_ptr<ObexSocketDevice> &socketDevice);

    /**
     * @brief sdp found result callback
     *
     */
    void SdpFoundCallback(const OppSdpFoundRecord &record);

    /**
     * @brief OnReceiveIncomingConnect
     *
     * @param connectId connectId
     * @return reponse code
     */
    int OnReceiveIncomingConnect(uint32_t connectId,
       std::string remoteAddr, std::shared_ptr<ObexServerSession> serverSession);

    /**
     * @brief Notify Transfer State Changed
     *
     * @param info info
     */
    void NotifyTransferStateChanged(IOppTransferInformation info);

    /**
     * @brief Notify Receive Incoming File
     *
     * @param info info
     */
    void NotifyReceiveIncomingFile(IOppTransferInformation info);

    /**
     * @brief  Opp Continuous Task.
     *
     * @param  action connect or close
     */
    void OppContinuousTask(const std::string &action);

    /**
     * @brief  AclDisConnEvent.
     *
     * @param  addr addr
     */
    void AclDisConnEvent(const std::string &addr);

private:
    void EnableService();
    void DisableService();
    void CreateSdpRecord();
    void CleanSdpRecord();
    void ProcessConnectEvent(const OppMessage &event);
    void ProcessDefaultEvent(const OppMessage &event);
    void ProcessRemoveStateMachine(const OppMessage &event);
    int ConnectSocket(const std::string &addr, const Uuid &uuid, int securityFlag, int type, int psm);
    int32_t oppSdpHandle = -1;
    // The mutex variable
    BtRecursiveMutex mutex_ {};
    bool isWakeUpApplication_ = false;
    std::shared_ptr<ObexServerSocket> obexServerSocket_ = nullptr;
    std::shared_ptr<IObexSocketObserver> socketObserver_ = nullptr;
    std::shared_ptr<IOppObserver> oppConnectionObserver_ = nullptr;
    // The map of the device and sate machine.
    std::map<const std::string, std::unique_ptr<OppStateMachine>> stateMachines_ {};

    BaseObserverList<IOppObserver> oppObservers_ {};
    // const state map
    const std::map<const int, const int> stateMap_ = {
        {OPP_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {OPP_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {OPP_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {OPP_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };

    CxxDynamicLibraryLoader<OppServiceImplInterface> loader_;
    std::atomic_bool isLoaded_ = false;

    DECLARE_IMPL();
    BT_DISALLOW_COPY_AND_ASSIGN(OppService);
};
class OppSocketObserver : public IObexSocketObserver {
public:
    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice) override;
};

class OppConnectionObserver : public IOppObserver {
public:
    ~OppConnectionObserver() override = default;
    void OnReceiveIncomingFile(const IOppTransferInformation &transferInformation) override;
    void OnTransferStateChange(const IOppTransferInformation &transferInformation) override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OPP_SERVICE_H