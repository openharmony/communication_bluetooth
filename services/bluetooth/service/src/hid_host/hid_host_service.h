/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef HID_HOST_SERVICE_H
#define HID_HOST_SERVICE_H

#include <list>
#include <mutex>
#include <vector>
#include <memory.h>
#include <cmath>
#include <cstring>
#include <mutex>

#include "adapter_config.h"
#include "base_def.h"
#include "base_observer_list.h"
#include "bluetooth.h"
#include "bt_hh.h"
#include "class_creator.h"
#include "context.h"
#include "hid_host_message.h"
#include "hid_host_statemachine.h"
#include "interface_profile_hid_host.h"
#include "log.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "securec.h"
#include "service_util.h"
#include "safe_map.h"
#include "util/safe_vector.h"

namespace OHOS {
namespace bluetooth {
class HidHostService : public IProfileHidHost, public utility::Context {
public:
    /**
     * @brief Get the instance of the HfpHfService object.
     *
     * @return Returns the instance of the HfpHfService object.
     */
    static HidHostService *GetService();
    /**
     * @brief Construct a new Hid Host Service object
     *
     */
    explicit HidHostService();
    /**
     * @brief Destroy the Hid Host Service object
     *
     */
    ~HidHostService() override;
    utility::Context *GetContext() override;
    void Enable(void) override;
    void Disable(void) override;
    int Connect(const RawAddress &device) override;
    int GetConnectState(void) override;
    int GetMaxConnectNum(void) override;
    int Disconnect(const RawAddress &device) override;
    int GetDeviceState(const RawAddress &device) override;
    void RegisterObserver(IHidHostObserver &hidHostObserver) override;
    void DeregisterObserver(IHidHostObserver &hidHostObserver) override;
    void NotifyStateChanged(const RawAddress &device, int state);
    void ShutDownDone(bool isAllDisconnected);
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) override;
    std::list<RawAddress> GetConnectDevices() override;
    void CleanUpAllStateMachine(void);
    void ReStartStackHidProfile(void) override;
    void ClearUpStackHidProfile();

    /**
     * @brief Send the event of the Hid Host role.
     *
     * @param event The event of the Hid Host role.
     */
    void RemoveStateMachine(const std::string &device);
    void ProcessEvent(const HidHostMessage &event);
    void PostEvent(const HidHostMessage &event);
    void ProcessRemoveStateMachine(const std::string &address);
    void ProcessConnectEvent(const HidHostMessage &event);
    void ProcessDefaultEvent(const HidHostMessage &event);
    std::string HidHostFindDeviceByLcid(uint16_t lcid, bool *isControlLcid);
    int HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type) override;
    int HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type) override;
    int HidHostSetReport(std::string device, uint8_t type, uint16_t size, const uint8_t* report) override;
    int HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type) override;
    int SetConnectStrategy(const RawAddress &device, int strategy) override;
    int GetConnectStrategy(const RawAddress &device) override;
    BthhInterface* getBluetoothHidInterface() const;

    static void ConnectionStateCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhConnectionState state);
    static void GetProtocolModeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhStatus hhStatus, BthhProtocolMode mode);
    static void GetIdleTimeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhStatus hhStatus, int idleTime);
    static void GetReportCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhStatus hhStatus, uint8_t* rptData, int rptSize);
    static void VirtualUnplugCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhStatus hhStatus);
    static void HandshakeCallback(STACK::RawAddress* bdAddr, BleAddrType addrType,
        BtTransport transport, BthhStatus hhStatus);

private:
    /**
     * @brief Service startup.
     *
     */
    void StartUp();
    void ShutDown();
    /**
     * @brief Get the max connection devices number.
     *
     * @return Returns the max connection devices number.
     */
    int GetMaxConnectedDeviceNum() const;
    int GetConnectedDeviceNum();
    void ConnectionStateCallbackInner(RawAddress rawAddr, int state);
    void NotifyConnStateChangedInner(const RawAddress &device, int state, int cause);
    std::string GetEventName(int what);
    static uint8_t CovertConnectStateFromStack(BthhConnectionState state);
    /**
     * @brief check is accept connection
     *
     * @return returns true if accept connection
     */
    bool IsAcceptConnection(RawAddress &rawAddr, int state);
    bool IsHidSupportedByRemoteDevice(const RawAddress &device);
    void HidProcessBtChrEvent(const std::string& addr, int toState);
    int IsLocalDeviceConnectAllowed(const RawAddress &device);

    //  service status
    bool isStarted_ {false};
    //  service status
    bool isShuttingDown_ {false};

    // The maximum default number of connection devices
    static const int HID_HOST_MAX_DEFAULT_CONNECTIONS_NUM = 8;
    static const int HIDHOSTTYPE = 0;
    // the maximum number of connection devices.
    int maxConnectionsNum_ {HID_HOST_MAX_DEFAULT_CONNECTIONS_NUM};
    BaseObserverList<IHidHostObserver> hidHostObservers_ {};
    // the map of the device and sate machine
    SafeMap<const std::string, std::shared_ptr<HidHostStateMachine>> stateMachines_ {};
    // const state map
    const std::map<const int, const int> stateMap_ = {
        {HID_HOST_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {HID_HOST_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {HID_HOST_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {HID_HOST_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
    BthhInterface* bluetoothHidInterface = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HID_HOST_SERVICE_H
