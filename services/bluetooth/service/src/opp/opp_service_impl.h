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

#ifndef OPP_SERVICE_IMPL_H
#define OPP_SERVICE_IMPL_H

#include <cstring>
#include <list>
#include <map>
#include <vector>
#include "base_def.h"
#include "base_observer_list.h"
#include "context.h"
#include "btcommon/message.h"
#include "bt_chr_dft_statictics.h"
#include "opp_def.h"
#include "opp_message.h"
#include "opp_transfer.h"
#include "opp_statemachine.h"
#include "raw_address.h"
#include "opp_service_impl_interface.h"
#include "util/safe_vector.h"
#include "common_util.h"
#include "thread_util.h"
#include "adapter_device_config.h"


namespace OHOS {
namespace bluetooth {
class OppServiceImpl : public OppServiceImplInterface {
public:
    OppServiceImpl() = default;
    ~OppServiceImpl() override;

    std::function<int(const std::string &addr, const Uuid &uuid, int securityFlag, int type,
        int psm)> connectSocketFunc_ = nullptr;

    std::function<bool()> unloadOppFunc_ = nullptr;

    std::function<void(const std::string &addr)> createOppSdpFunc_ = nullptr;

    std::function<bool(const std::string &addr)> isAllowedOppReadFunc_ = nullptr;

    /**
     * @brief post opp message
     *
     * @param event OppMessage
     * @return void
     */
    void PostEvent(const OppMessage &event);

    /**
     * @brief Process opp Event
     *
     * @param event OppMessage
     * @return void
     */
    void ProcessEvent(const OppMessage &event);

    /**
     * @brief GetDevicesByStates
     *
     * @param states the device states
     * @return devices
     */
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;

    /**
     * @brief GetDeviceState
     *
     * @param device the device
     * @return state
     */
    int GetDeviceState(const RawAddress &device) override;

    /**
     * @brief GetCurrentTransferInformation
     *
     * @return IOppTransferInformation
     */
    IOppTransferInformation GetCurrentTransferInformation() override;

    /**
     * @brief sdp found result callback
     *
     */
    void SdpFoundCallback(const OppSdpFoundRecord &record) override;

    /**
     * @brief SendFile
     *
     * @param device the destinst device
     * @param fileHolders files to send
     * @return errorCode
     */
    int SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders) override;

    /**
     * @brief CancelTransfer
     *
     * @return result
     */
    int CancelTransfer() override;

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
     * @param uri last received fileuri
     * @return result
     */
    int SetLastReceivedFileUri(const std::string &uri) override;


    /**
     * @brief RemoveAllDeviceTransfer
     *
     * @param device remove device
     */
    void RemoveAllDeviceTransfer(const std::string &device) override;

    /**
     * @brief RemoveTransfer
     *
     * @param device remove device
     */
    void RemoveTransfer(const std::string &device);

    /**
     * @brief ConnectObex
     *
     * @param obexConfig cpnnect obexConfig
     * @return void
     */
    void ConnectObex(std::shared_ptr<ObexClientConfig> obexConfig);

    /**
     * @brief disconnect device transfer obex connect
     *
     * @param device device
     */
    void OnObexDisconnected(const std::string &device);

    /**
     * @brief OnObexConnected
     *
     * @param device device
     */
    void OnObexConnected(const std::string &device);

    /**
     * @brief DisconnectObex
     *
     * @param device device
     */
    void DisconnectObex(const std::string &device);

    /**
     * @brief RemoveStateMachine
     *
     * @param device device
     */
    void RemoveStateMachine(const std::string &device);

    /**
     * @brief OnReceiveIncomingConnect
     *
     * @param connectId connectId
     * @return reponse code
     */
    int OnReceiveIncomingConnect(uint32_t connectId, int32_t totalFileCount,
       std::string remoteAddr, std::shared_ptr<ObexServerSession> serverSession);

    /**
     * @brief OnTransferPositionChange
     *
     * @param device device
     * @param position position
     * @return reponse code
     */
    void OnTransferPositionChange(const std::string &device, size_t position);

    /**
     * @brief OnTransferPositionChange
     *
     * @param device device
     * @param state state
     * @param reason reason
     * @return reponse code
     */
    void OnTransferStateChange(const std::string &device, int state, int reason);

    /**
     * @brief OnOpreationStateChange
     *
     * @param device device
     * @param state state
     */
    void OnOpreationStateChange(const std::string &device, int state);

    /**
     * @brief OnReceiveIncomingFile
     *
     * @param info info
     * @return reponse code
     */
    int OnReceiveIncomingFile(IOppTransferInformation info);

    /**
     * @brief CancelTransfer
     *
     * @param device device
     * @return result
     */
    int CancelTransfer(const std::string &device);

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
     * @brief Notify Receive auto Confirm Incoming File
     *
     * @param info oppTransferInformation info
     */
    void NotifyReceiveIncomingNoConfirmFile(IOppTransferInformation info);

    /**
     * @brief Notify statenachine state change
     *
     * @param device device
     * @param state state
     */
    void NotifyStateChanged(const RawAddress &device, int state);

    /**
     * @brief GetFirstTransferFileName
     *
     * @param device device
     * @return first transfer fileName
     */
    std::string GetFirstTransferFileName(const std::string &device);

    /**
     * @brief DllRegisterConnectSocketFunc
     *
     * @param info info
     */
    void DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc) override;

    void DllRegisterUnloadOppFunc(const std::function<bool()> &unloadOppFunc) override;

    void DllRegisterCreateOppSdpFunc(const std::function<void(const std::string &addr)> &createOppSdpFunc) override;

    void DllRegisterIsAllowedOppReadFunc(const std::function<bool(const std::string &addr)>
        &isAllowedOppReadFunc) override;

    void DllRegisterOppContinuousTaskFunc(const std::function<void(const std::string &action)>
        &oppContinuousTaskFunc) override;
    
    void DllRegisterAclDisConnEventFunc(const std::function<void(const std::string &addr)>
        &aclDisConnEventFunc) override;

    void RegisterObserver(std::shared_ptr<IOppObserver> &observer) override;

    void DeregisterObserver() override;

    void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice) override;

private:
    void ProcessConnectEvent(const OppMessage &event);
    void ProcessDefaultEvent(const OppMessage &event);
    void ProcessRemoveStateMachine(const OppMessage &event);
    void StartNextTransfer();

    // The map of the device and sate machine.
    std::map<const std::string, std::unique_ptr<OppStateMachine>> stateMachines_ {};
    std::shared_ptr<IOppObserver> oppConnectionObserver_ = nullptr;
    int64_t lastNotifyRunningStateTime_ = 0;

    utility::SafeVector<std::shared_ptr<OppTransfer>> oppTransferList_ {};

    // const state map
    const std::map<const int, const int> stateMap_ = {
        {OPP_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {OPP_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {OPP_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {OPP_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
    BtRecursiveMutex mutex_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OPP_SERVICE_IMPL_H
