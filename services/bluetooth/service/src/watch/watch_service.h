/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
 * @file watch_service.h
 *
 * @brief Service module of the watch product
 *
 * @since 12
 */

#ifndef WATCH_SERVICE_H
#define WATCH_SERVICE_H

#include "base_def.h"
#include "hw_bt_watch_interface.h"
#include "bt_hd.h"
#include "bt_hf_client.h"
#include "hfp_hf_defines.h"
#include "watch_define.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "classic_defs.h"
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "hands_free_unit_calls.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "thread_util.h"
#include "datashare_helper.h"
#include "data_ability_observer_stub.h"
#include "ipower_mgr.h"
#include "running_lock.h"

namespace OHOS {
namespace bluetooth {
class BluetoothWatchServiceSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit BluetoothWatchServiceSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~BluetoothWatchServiceSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class WatchService {
public:
    /**
     * @brief Get watch service singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 12
     */
    static WatchService *GetInstance();
    /**
     * @brief Initialize watch service.
     *
     * @param dispatch The dispatch used in watch service.
     * @since 12
     */
    static void Initialize();
    /**
     * @brief Uninitialize watch service.
     *
     * @since 12
     */
    static void Uninitialize();

    /**
     * @brief set link loss rssi.
     *
     * @param device disconnect device.
     * @param rssi rssi of link loss.
     * @since 12
     */
    void SetLinkLossRssi(const RawAddress &device, int rssi);

    /**
     * @brief call back of device disconnected.
     *
     * @param device device address.
     * @param reason disconnect reason.
     * @since 12
     */
    void DeviceDisconnectCallBack(const RawAddress &device, int reason);

    /**
     * @brief call back of device disconnected.
     *
     * @param device device address.
     * @param isConnected is sais connected.
     * @since 12
     */
    void SetSaisConnectionState(const RawAddress &device, bool isConnected);

    /**
     * @brief acl connection state change.
     *
     * @param device device address.
     * @param isConnected is device connected.
     * @since 12
     */
    void ProcessAclStateChangeEvt(const RawAddress &device, bool isConnected);

    /**
     * @brief pair state change.
     *
     * @param transport connection transport.
     * @param device device address.
     * @param status pair state.
     * @since 12
     */
    void ProcessPairStateChangeEvt(const BTTransport transport, const RawAddress &device, int status);

    /**
     * @brief set HID connection state.
     *
     * @param device device address.
     * @param state connection state.
     * @since 12
     */
    void SetHidConnectionState(const RawAddress &device, int state);

    /**
     * @brief process remove bond device.
     *
     * @since 12
     */
    void ProcessRemovePair();

    /**
     * @brief HF connection state change.
     *
     * @param device device address.
     * @param state connection state.
     * @since 12
     */
    void ProcessHfpStateChangeEvt(const RawAddress &device, int state);

    /**
     * @brief connect HF.
     *
     * @return is connect hfp
     * @since 12
     */
    bool ConnectHfp(const std::string& address);

    /**
     * @brief connect bonded phone.
     *
     * @since 12
     */
    void ConnectBondedPhone();

    /**
     * @brief set current call id.
     *
     * @param callId call id.
     * @since 12
     */
    void SetCallID(const std::string& address, int callId);

    /**
     * @brief set is allow sco connect.
     *
     * @param isAllow is allow soc connected.
     * @since 12
     */
    void SetAllowHfScoConnect(const std::string& address, bool isAllow);

    /**
     * @brief get is allow sco connect.
     *
     * @return is allow connect sco
     * @since 12
     */
    bool IsAllowHfScoConnect(const std::string& address);

    /**
     * @brief get is allow AG call.
     *
     * @return is allow AG call
     * @since 12
     */
    bool IsAllowAgCall(const std::string& address);

    /**
     * @brief set dail call.
     *
     * @param dailNumber dail call number.
     * @since 12
     */
    void SetDialedCallNumber(const std::string &dailNumber);

    /**
     * @brief get dail call number.
     *
     * @return dail call number
     * @since 12
     */
    std::string GetDialedCallNumber();

    /**
     * @brief dial call over, reset flag.
     *
     * @since 12
     */
    void DialCallFinished();

    /**
     * @brief whether need dial call.
     *
     * @return is need dial call
     * @since 12
     */
    bool IsNeedDialCall();

    /**
     * @brief set the flag of watch dialing call.
     *
     * @since 12
     */
    void SetDialingByWatch(const std::string& address);

    /**
     * @brief whether the call dialing by watch.
     *
     * @return is the call dialing by watch
     * @since 12
     */
    bool IsDialingByWatch(const std::string& address);

    /**
     * @brief set hf call state.
     *
     * @param isHfDiconnected is hf disconnected.
     * @since 12
     */
    void AllHfCallFinished(const std::string& address, bool isHfDisconnected);

    /**
     * @brief set whether hf on call.
     *
     * @param isOnCall is hf on call.
     * @since 12
     */
    void SetIsHfOnCall(const std::string& address, bool isOnCall);

    /**
     * @brief whether hf on call.
     *
     * @return is hfp on call
     * @since 12
     */
    bool IsHfOnCall(const std::string& address);

    /**
     * @brief call start changed.
     *
     * @param call current call.
     * @since 12
     */
    void ProcessCallStateChangeEvt(const std::string& address, const HandsFreeUnitCalls &call);

    /**
    * @brief handling virtual incoming calls.
    *
    * @param voip state.
    * @since 12
    */
    void HandleVoipState(int voipState);

    /**
    * @brief Write data to the database.
    *
    * @param uriStr Data table address.
    * @param keyStr key value.
    * @param inValue Data.
    * @since 12
    */
    bool BluetoothDataShareUpdate(const char *uriStr, const char *keyStr, std::string &inValue);

    /**
    * @brief handle chr data base changed.
    *
    * @since 12
    */
    void HandleConnectChrDataChanged();

    /**
    * @brief when bt on start connect phone.
    *
    * @since 12
    */
    void StartConnectPhone();

    /**
    * @brief set sais server is add
    *
    * @since 12
    */
    void BluetoothIsOn();

    /**
    * @brief bluetooth automatically turns on after exiting STR mode.
    *
    * @since 12
    */
    bool IsEnableBluetoothAfterExitStrMode();

    /**
    * @brief modifying system sttributes when entering STR mode
    *
    * @since 12
    */
    void ChangeStatusOfEnterStrMode();

    /**
    * @brief Handling when STR exits abnormally.
    *
    * @param callerName Name of the process that invokes enable.
    *
    * @return Returns whether the processing result is required.
    *         BT_STATUS_SUCCESS No exception handling is required.
    *         BT_STATUS_FAIL Execute the exception handling process.
    * @since 12
    */
    int StrModeAbnormalExitHandle(const BTTransport transport);

    /**
    * @brief Create a lock to prevent system sleep.
    *
    * @param timeOutMs Locking time.
    *
    * @since 12
    */
    void HoldRunningLock(int32_t timeOutMs);

    /**
    * @brief Release the lock that prevents the system from sleeping..
    *
    * @since 12
    */
    void ReleaseRunningLock();

    /**
    * @brief Unregister the str mode switch event.
    *
    * @since 12
    */
    void UnSubscribeWatchEnterStrEvent();
    void StartIphoneScoDisconnectTimer(const std::string& address);
    void StopIphoneScoDisconnectTimer(const std::string& address);
    void SetSaisAddStatus(bool isAdd);
    void HandleHealthStatusChanged();
    void SecondaryPhoneChange();
    void NotifyBluetoothTurningOff();
    bool IsAllowSystemPairDialog(const RawAddress &device);
    BtHwWatchInterface* GetBluetoothWatchInterface() const;
    bool IsIphone(const RawAddress &device);
    bool HidGetMapUpdateingState();
    void HidUpdateMap();
    void HidReconnectAfterUpdateMap(const RawAddress &device, bool isAclConnect);
    void HidVolumeIncrement();
    void HidVolumeDecrement();
    void HidDoubleClick();
    void HidSingleClick();
    void SetPhoneConnHfp(const std::string &address, bool isPhoneCon);
    void SendConnectionTimtout();
    void UpdateSaisDevice(const RawAddress &device, bool isSaisDevice);
    void UpdateHFStrategy(const RawAddress &device, int strategy);
    void AddNewCallToWatch(const std::string& address, int callId, bool isScoConnected);
    bool IsSaisDevice(const std::string& address);

    /**
    * @brief Verifies the validity of the Bluetooth MAC address.
    *
    * @param address Bluetooth MAC address.
    *
    * @return true indicates that it conforms to the Bluetooth MAC format.
    *         false indicates that it does not conform to the Bluetooth MAC format.
    *         possibly due to invalid characters, incorrect length, or empty characters.
    * @since 12
    */
    bool IsValidAddress(const std::string &address);

    class ClassicRemoteDeviceObserver : public IClassicRemoteDeviceObserver {
    public:
        explicit ClassicRemoteDeviceObserver(WatchService *watchService)
            : watchService_(watchService){};
        ~ClassicRemoteDeviceObserver() = default;
        void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) override
        {
            watchService_->ProcessAclStateChangeEvt(device, (state == CONNECTION_STATE_CONNECTED));
        }
        void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
            const std::string &causeMessage) override
        {
            (void)causeMessage;
            watchService_->ProcessPairStateChangeEvt(transport, device, status);
        }
        void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override{};
        void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override{};
        void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override{};
        void OnRemoteCodChanged(const RawAddress &device, int cod) override{};
        void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override{};
        void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) override{};
    private:
        WatchService *watchService_{nullptr};
    };

    class ConnectChrObserver : public AAFwk::DataAbilityObserverStub {
    public:
        explicit ConnectChrObserver(WatchService &instance) : watchService_(instance) {}
        ~ConnectChrObserver() = default;
        void OnChange() override
        {
            DoInVendorThread([observer = &watchService_]() { observer->HandleConnectChrDataChanged(); });
        }
    private:
        WatchService &watchService_;
    };

    class HealthStatusObserver : public AAFwk::DataAbilityObserverStub {
    public:
        explicit HealthStatusObserver(WatchService &instance) : watchService_(instance) {}
        ~HealthStatusObserver() = default;
        void OnChange() override
        {
            DoInVendorThread([observer = &watchService_]() { observer->HandleHealthStatusChanged(); });
        }
    private:
        WatchService &watchService_;
    };

    class SecondaryPhoneObserver : public AAFwk::DataAbilityObserverStub {
    public:
        explicit SecondaryPhoneObserver(WatchService &instance) : watchService_(instance) {}
        ~SecondaryPhoneObserver() = default;
        void OnChange() override
        {
            DoInVendorThread([observer = &watchService_]() { observer->SecondaryPhoneChange(); });
        }
    private:
        WatchService &watchService_;
    };

private:
    /**
     * @brief Create watch services.
     *
     * @since 12
     */
    void Start();
    /**
     * @brief Delete watch services when Start() create.
     *
     * @since 12
     */
    void Stop();
    WatchService();
    ~WatchService();

    bool PublishEventWithIntParam(const std::string &eventAction, const std::string &device,
        const std::string &key, const int value, bool isSticky);
    bool IsPhoneOrComputer(const RawAddress &device);
    void RegisterHid();
    void GetBondedPhone();
    void ChangeScanMode();
    void ChrProcessRemovePair(const std::string& address, const bool isAclConnect);
    void StartBtDisconnectTimer(const std::string& address, int reason);
    void StopBtDisconnectTimer(const std::string& address);
    void ChrProcessDisconnect(const std::string& address, const int reason);
    void SubscribeWatchCommonEvent();
    void SubscribeWatchEnterStrEvent();
    void SendPhoneConnectStateChange(const RawAddress &device, bool connected);
    void ProcessAclStateChangeInner(const RawAddress &device, bool isConnected);
    void ProcessPairChangeInner(const BTTransport transport, const RawAddress &device, int status);
    void ProcessRemovePairInner(const RawAddress &device, bool isAclConnect);
    void WatchSetUnConnectable();
    void SubscribeDataShareEvent();
    void UnSubscribeDataShareEvent();
    void CheckDisconnectHid();
    void StartSaisAddedTimer();
    void StopSaisAddedTimer();
    void SaisAddedTimeout();
    bool IsWatchStrMode();
    void StartHidReconnectTimer(const RawAddress &device);
    void HidReconnectTimeout(const RawAddress &device);
    void StopHidReconnectTimer();
    void ChangeStatusEnterStrMode();
    void ChangeStatusExitStrMode();
    void PromptUnloadBluetoothSystemAbility();
    void UpdataHighPowerMode(bool enable);
    void StartDisconnectHfpOnDemandTimer(const std::string& address);
    void StopDisconnectHfpOnDemandTimer();
    void DisconnectHfpOnDemand(const std::string& address);
    void StartConnectHfpOnDemandTimer(const std::string& address);
    void StopConnectHfpOnDemandTimer(const std::string& address);
    void ConnectHfpOnDemand(const std::string& address);

    void StartConnectHfpTimer(const std::string& address, int delayTime);
    void StopConnectHfpTimer(const std::string& address);
    void StartControlScoConnectTimer(const std::string& address);
    void StopControlScoConnectTimer(const std::string& address);
    void ControlScoConnectTimeout(const std::string& address);
    void UpdateReconnectState(bool isBtOn);
    // rssi of br link loss.
    int rssiLinkLoss_ {MIN_NOT_TIMEOUT_RSSI};
    int hidConnectionState_{BTHD_CONN_STATE_DISCONNECTED};
    // current call id or the second call id if Three-Party Call
    uint8_t brHidReconnectCount_{0};

    std::string dailNumber_{""};
    std::string phoneMac_{""};

    bool isNeedDialAfterHFConnected_{false};
    bool isRemovePair_{false};
    bool isSaisAdded_{false};
    bool lastHighPowerEnable_{false};
    bool isNeedConnectHid_{false};
    bool isHidMapUpdateing_{false};
    bool isBluetoothDisable_{false};
    bool isSecondaryPair_{false};

    BtHwWatchInterface* bluetoothWatchInterface = nullptr;
    bthd_interface_t* bluetoothHidInterface = nullptr;
    bt_interface_t* bluetoothInterface = nullptr;
    std::unique_ptr<ClassicRemoteDeviceObserver> classicRemoteDeviceObserver_{nullptr};
    std::unique_ptr<utility::Timer> saisAddedTimer_ {nullptr};
    std::unique_ptr<utility::Timer> hidReconnectTimer_ {nullptr};
    std::unique_ptr<utility::Timer> disconnectHfpTimer_ {nullptr};
    std::unique_ptr<utility::Timer> disconnectScoTimer_ {nullptr};
    std::shared_ptr<BluetoothWatchServiceSubscriber> subscriber_ {nullptr};
    std::shared_ptr<BluetoothWatchServiceSubscriber> strSubscriber_ {nullptr};
    std::shared_ptr<DataShare::DataShareHelper> chrDataShareHelper_ = nullptr;
    std::shared_ptr<OHOS::PowerMgr::RunningLock> runningLock_ = nullptr;
    sptr<ConnectChrObserver> chrDataShareObserver_ = nullptr;
    sptr<HealthStatusObserver> healthStatusObserver_ = nullptr;
    sptr<SecondaryPhoneObserver> secondaryPhoneObserver_ = nullptr;
    mutable std::mutex bondPhoneAddressMutex_;
    mutable std::mutex disconnectScoTimerMutex_;
    BT_DISALLOW_COPY_AND_ASSIGN(WatchService);
};
}
}
#endif
