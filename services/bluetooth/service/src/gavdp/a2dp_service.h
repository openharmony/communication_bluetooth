/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
 * @addtogroup bluetooth
 * @file a2dp_service.h
 *
 * @brief Declare the interface and function implementation of A2DP service and profile.
 *
 * @since 6
 */

#ifndef A2DP_SERVICE_H
#define A2DP_SERVICE_H

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <atomic>

#include "a2dp_def.h"
#include "a2dp_service_connection.h"
#include "a2dp_service_device.h"
#include "base_observer_list.h"
#include "context.h"
#include "interface_profile.h"
#include "interface_profile_a2dp_src.h"
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "raw_address.h"
#include "bt_av.h"
#include "hw_bt_hwif.h"
#include "service_util.h"
#include "util/safe_vector.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief This class provides functions called by Framework API.
 *
 * @since 6.0
 */
class A2dpService : public IProfileA2dp, public utility::Context {
public:
    /**
     * @brief A constructor used to create an a2dpservice instance.
     *
     * @param name Service name.
     * @param version Profile version.
     * @param role Profile role.
     * @since 6.0
     */
    A2dpService(const std::string &name, const std::string version, const uint8_t role);

    /**
     * @brief A destructor used to delete the a2dpservice instance.
     *
     * @since 6.0
     */
    ~A2dpService() override;

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
     * @brief Get the instance of the A2DP source or sink object.
     *
     * @return Returns the instance of the A2DP source or sink object.
     * @since 6.0
     */
    utility::Context *GetContext() override;

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
     * @brief Get A2dpSourceConnectStatus
     *
     * @param device Remote device
     * @return A2dpSourceConnectStatus
     */
    int GetRemoteDeviceConnectStatus(const RawAddress &device);

    /**
     * @brief Get devices by connection states.
     *
     * @param states The connection states of the bluetooth device.
     * @return Returns devices that match the connection states.
     * @since 6.0
     */
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> &states) const override;

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
    int GetDeviceState(const RawAddress &device) const override;

    /**
     * @brief Get device playing state by address when target device is on connected.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>1</b> if device is on playing;
     *         Returns <b>0</b> if device is not on playing;
     * @since 6.0
     */
    int GetPlayingState(const RawAddress &device, int &state) const override;

    /**
     * @brief Set target device as active device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or not in device list.
     * @since 6.0
     */
    int SetActiveSinkDevice(const RawAddress &device) override;

    /**
     * @brief Get active device.
     * @return Returns active device.
     * @since 6.0
     */
    const RawAddress &GetActiveSinkDevice() const override;

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
    int SetConnectStrategy(const RawAddress &device, int strategy) override;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 6.0
     */
    int GetConnectStrategy(const RawAddress &device) const override;

    /**
     * @brief Send delay reporting.
     *
     * @param device The address of the peer bluetooth device.
     * @param delayValue The delay value.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     */
    int SendDelay(const RawAddress &device, uint16_t delayValue) override;

    /**
     * @brief Get codec status information of connected device.
     *
     * @param device The address of the bluetooth device.
     * @return Returns codec status information of connected device.
     * @since 6.0
     */
    A2dpSrcCodecStatus GetCodecStatus(const RawAddress &device) const override;

    /**
     * @brief Get the codec encoding preferences of the specified device.
     *
     * @param device The address of the bluetooth device.
     * @param info The codec encoding information.
     * @return Return the result getted.
     * @since 6.0
     */
    int GetCodecPreference(const RawAddress &device, A2dpSrcCodecInfo &info) override;

    /**
     * @brief Set the codec encoding preferences of the specified device.
     *
     * @param device The address of the bluetooth device.
     * @param info The codec encoding information.
     * @return Return the result setted.
     * @since 6.0
     */
    int SetCodecPreference(const RawAddress &device, const A2dpSrcCodecInfo &info) override;

    /**
     * @brief Set whether the optional codec is valid.
     *
     * @param device The address of the bluetooth device.
     * @param isEnable Set true if the optional codec is valid.
     *                 Set false if the optional codec is invalid.
     * @since 6.0
     */
    void SwitchOptionalCodecs(const RawAddress &device, bool isEnable) override;

    /**
     * @brief Get whether the peer bluetooth device supports optional codec.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>A2DP_OPTIONAL_SUPPORT</b> The device supports optional codec.
     *         Returns <b>A2DP_OPTIONAL_NOT_SUPPORT</b> The device doesn't support optional codec.
     *         Returns <b>A2DP_OPTIONAL_SUPPORT_UNKNOWN</b> Don't know if the device support optional codec.
     * @since 6.0
     */
    int GetOptionalCodecsSupportState(const RawAddress &device) const override;

    /**
     * @brief Audio start streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int StartPlaying(const RawAddress &device) override;

    /**
     * @brief Audio suspend streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int SuspendPlaying(const RawAddress &device) override;

    /**
     * @brief Audio stop streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int StopPlaying(const RawAddress &device) override;

    /**
     * @brief Register observer function of framework.
     *
     * @param observer The observer function pointer of framework.
     * @since 6.0
     */
    void RegisterObserver(IA2dpObserver *observer) override;

    /**
     * @brief Deregister observer function of framework.
     *
     * @since 6.0
     */
    void DeregisterObserver(IA2dpObserver *observer) override;

    /**
     * @brief Write PCM data to A2dp Service
     * @param[in] data is the address of the input data
     * @param[in] size is the size of the input data
     * @since 6.0
     */
    int WriteFrame(const uint8_t *data, uint32_t size) override;

    /**
     * @brief Get the information of the current rendered position.
     *
     * @param device The address of the bluetooth device.
     * @param[out] dalayValue is the delayed time
     * @param[out] sendDataSize is the data size that has been sent
     * @param[out] timeStamp is the current time stamp
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                          uint32_t &timeStamp) override;

    /**
     * @brief Offload Audio start streaming control.
     *
     * @param device The address of the bluetooth device.
     * @param sessionsId audio framework set streams session id.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int OffloadStartPlaying(const RawAddress &device, const std::vector<int> &sessionsId) override;

    /**
     * @brief Audio offload stop streaming.
     *
     * @param device The address of the bluetooth device.
     * @param sessionsId audio framework set streams session id.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails, or device is not in device list.
     * @since 6.0
     */
    int OffloadStopPlaying(const RawAddress &device, const std::vector<int> &sessionsId) override;

    /**
     * @brief Get encode path information of connected sink device.
     *
     * @param device The address of the bluetooth device.
     * @param streamsInfo audio streams detail.
     * @return Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     *         Returns <b>UNKNOWN_ENCODING: 0, bt unable to judge encoding path.
     *         Returns <b>SOFTWARE_ENCODING: 1, audio should select a2dp hdi.
     *         Returns <b>HARDWARE_ENCODING: 2, audio should select offload hdi.
     * @since 6.0
     */
    int A2dpOffloadSessionPathRequest(const RawAddress &device,
        const std::vector<A2dpSrcStreamInfo> &streamsInfo) override;

    /**
     * @brief Get offload codec status information of connected device.
     *
     * @param device The address of the bluetooth device.
     * @return Returns offload codec status information of connected device.
     * @since 6.0
     */
    A2dpSrcOffloadCodecStatus GetA2dpOffloadCodecStatus(const RawAddress &device) override;

    /**
     * @brief Get boject pointer of A2dpConnectManager.
     *
     * @return Returns boject pointer of A2dpConnectManager.
     * @since 6.0
     */
    A2dpConnectManager& ConnectManager();

    /**
     * @brief Process connect callback function pointer of framework.
     * @param state The connection state of the bluetooth device.
     * @param device The address of the bluetooth device.
     * @since 6.0
     */
    void ProcessConnectFrameworkCallback(int state, const RawAddress &device);

    /**
     * @brief Process playing callback function pointer of framework.
     * @param playingState The new playing state of the bluetooth device.
     * @param device The address of the bluetooth device.
     * @param error The playing error state of the bluetooth device.
     * @since 6.0
     */
    void ProcessPlayingFrameworkCallback(int playingState, int error, const RawAddress &device);

    /**
     * @brief Process codec callback function pointer of framework.
     * @param info The new codec config information of the bluetooth device.
     * @param device The address of the bluetooth device.
     * @param error The codec error state of the bluetooth device.
     * @since 6.0
     */
    void ProcessCodecFrameworkCallback(const bluetooth::A2dpSrcCodecInfo &info, int error, const RawAddress &device);

    /**
     * @brief Get device information from device list by device address.
     *
     * @param device The address of the bluetooth device.
     * @return Returns device information from device list.
     * @since 6.0
     */
    std::shared_ptr<A2dpDeviceInfo> GetDeviceFromList(const RawAddress &device);

    /**
     * @brief Get hdap device information from device.
     *
     * @return Returns device information.
     * @since 6.0
     */
    const std::shared_ptr<A2dpDeviceInfo> &GetHdapDeviceInfo() const;

    /**
     * @brief Set hdap device information from device.
     *
     * @param hdapDevice The device info of the hdap device.
     * @since 6.0
     */
    void SetHdapDeviceInfo(const std::shared_ptr<A2dpDeviceInfo> &hdapDevice);

    /**
     * @brief Get device list that saves devices information.
     * @return Returns device list.
     * @since 6.0
     */
    std::map<std::string, std::shared_ptr<A2dpDeviceInfo>> GetDeviceList() const;

    /**
     * @brief Add device information to device list.
     *
     * @param address The address of the bluetooth device.
     * @param deviceInfo The device information.
     * @since 6.0
     */
    void AddDeviceToList(std::string address, std::shared_ptr<A2dpDeviceInfo> deviceInfo);

    /**
     * @brief Delete device information from device list.
     *
     * @param device The address of the bluetooth device.
     * @since 6.0
     */
    void DeleteDeviceFromList(const RawAddress &device);

    /**
     * @brief : Post the events.
     *
     * @param event : The event of the a2dp
     * @since 6.0
     */
    void PostEvent(utility::Message event, RawAddress &device);

    /**
     * @brief Process the events.
     *
     * @param event : The event of the a2dp
     * @since 6.0
     */
    void ProcessEvent(utility::Message event, RawAddress &device);

    /**
     * @brief Update active device
     *
     * @param device The address of the peer device.
     * @since 6.0
     */
    void UpdateActiveDevice(const RawAddress &device);

    /**
     * @brief Update optional codec status.
     *
     * @param device The address of the bluetooth device.
     * @since 6.0
     */
    void UpdateOptCodecStatus(const RawAddress &device);

    /**
     * @brief Check if do disable, and clear device list when doing disable.
     *
     * @since 6.0
     */
    void CheckDisable();

    /**
     * @brief  Activate remote device
     *
     * @since 6.0
     */
    void ActiveDevice();

    BtavSourceInterface* getBluetoothA2dpSrcInterface() const;
    BtavSinkInterface* getBluetoothA2dpSnkInterface() const;

    /**
     * @brief Notify media stack changed.
     * @param action Action on the device.
     * @param device The address of the peer device.
     * @since 10.0
     */
    void NotifyMediaStackChanged(int action, const RawAddress &device) override;

    /**
     * @brief Is active device in play to pause denyList
     * @return Returns <b>true</b> Is active device in play to pause denyList;
     *         returns <b>false</b> Is not active device in play to pause denyList.
     * @since 10.0
     */
    bool IsActiveDeviceInPlayToPauseBlackList() override;

    void NotifyWearDetectionActionAfterConnected(const RawAddress &device);

    void NotifyProfileState(const RawAddress &device);

    /**
     * @brief Force stop offload playing.
     * @param device The address of peer device.
     * @since 10.0
     */
    void ForceStopOffloadPlaying(const RawAddress &device) override;

    int EnableAutoPlay(const RawAddress &device) override;
    int DisableAutoPlay(const RawAddress &device, const int duration) override;
    int GetAutoPlayDisabledDuration(const RawAddress &device, int &duration) override;
    void OnRenderStateChange(bool isRenderActive) override;
    void OnRenderSceneChange(bool anyRenderRunning, bool gameRunning, bool musicRunning) override;
    void SetNavigationMode(bool navigationRunning) override;
    void ChangeCodecWithSceneSwitch();
    void ProcessA2dpHdfLoad(int state, const RawAddress &device);
    void ProcessAvrcpDynamicLoad(int state, const RawAddress &device, std::string profileName = "a2dp");
    void SendCommandPause();
    void SetCurrentDeviceMute();
    void UpdateVirtualDevice(int32_t action, const std::string &address) override;
    void GetVirtualDeviceList(std::vector<std::string> &devices) override;
    void HwSetActiveMode() override;
    bool IsA2dpPlaying(const RawAddress &device);
    void NotifyCaptureConnStateChanged(const RawAddress &device, int state, const bluetooth::A2dpSrcCodecInfo &info);
    std::list<RawAddress> GetConnectingOrConnectionDevices();
    void ProcessChrA2dpConnectionState(const RawAddress rawAddr, const int connectState);
    bool IsInCloudBondingState(const RawAddress &device) const;
    bool ProcCloudDeviceConnect(const RawAddress &device);
    void NotifyConnStateChanged(const RawAddress &device, int state, int cause);
    void NotifyBondStateChanged(BtStackStatus status, const RawAddress &device, BtBondState state);
    bool IsA2dpSceneChanged(bool isRenderStreamChanged, bool anyRenderRunning, CodecChangeAppScene lastScene);
    void SetA2dpPlayingStateCallback(std::function<void(int)> callbackFunction);
    void HandleA2dpPlayingStateChange(int state);
    CodecChangeAppScene FetchCurrentRenderScene(bool isRenderStreamChanged, bool anyRenderRunning,
        bool gameRunning, bool musicRunning);
    void SetWirelessChargingFlag(bool value);
    bool IsInWirelessCharging();
    int SendDeviceSelection(const RawAddress &device, int useA2dp, int useHfp, int userSelect);
    int GetMusicState() override;
    int GetGameState() override;
private:
    /**
     * @brief Process the events.
     *
     * @param event : The event of the a2dp
     * @since 6.0
     */
    void ProcessMessage(const utility::Message &msg) const;

    /**
     * @brief Enable the service.
     *
     * @since 6.0
     */
    void EnableService();

    /**
     * @brief Disable the service
     *
     * @since 6.0
     */
    void DisableService();

    /**
     * @brief Clear map a2dpDevices_
     *
     * @since 6.0
     */
    void ClearDeviceMap();

    /**
     * @brief Set whether the peer bluetooth device supports optional codec.
     *
     * @param device The address of the bluetooth device.
     * @param Returns <b>A2DP_OPTIONAL_SUPPORT</b> The device supports optional codec.
     *         Returns <b>A2DP_OPTIONAL_NOT_SUPPORT</b> The device doesn't support optional codec.
     *         Returns <b>A2DP_OPTIONAL_SUPPORT_UNKNOWN</b> Don't know if the device support optional codec.
     * @since 6.0
     */
    void SetOptionalCodecsSupportState(const RawAddress &device, int state);

    /**
     * @brief Find the state matched
     *
     * @param states The states searched
     * @param connectState The state of device connected
     * @return Returns <b>true</b> if the codec configuration is valid.
     *         Returns <b>false</b> if the codec configuration is invalid.
     * @since 6.0
     */
    bool FindStateMatched(std::vector<int> states, int connectState) const;

    /**
     * @brief Check if other codec configuration is similar with current configuration.
     *
     * @param codecInfo  codec configuration saved.
     * @param newInfo  Another codec configuration.
     * @return Returns <b>true</b> if other codec configuration is similar with current configuration.
     *         Returns <b>false</b> if other codec configuration is not similar with current configuration.
     * @since 6.0
     */
    bool IsSimilarCodecConfig(A2dpSrcCodecInfo codecInfo, A2dpSrcCodecInfo newInfo) const;

    /**
     * @brief Check if the codec information matches local information.
     *
     * @param codecInfo The codec information.
     * @param codecStatus The codec status information of local device.
     * @return Returns <b>true</b> codec information matches confirmed information.
     *         Returns <b>false</b> codec information doesn't match local information.
     * @since 6.0
     */
    bool IsLocalCodecInfo(A2dpSrcCodecStatus codecStatus, A2dpSrcCodecInfo codecInformation) const;

        /**
     * @brief Check if the codec information matches confirm information.
     *
     * @param codecInfo The codec information.
     * @param codecStatus The codec status information of local device.
     * @return Returns <b>true</b> codec information matches confirmed information.
     *         Returns <b>false</b> codec information doesn't match confirmed information.
     * @since 6.0
     */
    bool IsConfirmCodecInfo(A2dpSrcCodecStatus codecStatus, A2dpSrcCodecInfo codecInfo) const;

    bool IsRemoteA2dpSinkSupported(const RawAddress &device);

    void postA2dpConnectEvent(bool isReachMaxConnect, const RawAddress &device);

    /**
     * @brief add offload request streams details to a2dpStreamSessions_.
     *
     * @param streamsInfo audio framework request stream infos vector.
     *
     * @since 6.0
     */
    void A2dpOffloadAddRequestSessions(const std::vector<A2dpSrcStreamInfo> &streamsInfo);

    /**
     * @brief clear a2dpStreamSessions_ not active streams frome offload request streams details.
     *
     * @param streamsInfo audio framework request streams info vector.
     *
     * @since 6.0
     */
    void A2dpOffloadClearNotActiveSessions(const std::vector<A2dpSrcStreamInfo> &streamsInfo);

    /**
     * @brief update a2dp playing and reserved sessions vector.
     *
     * @param sessionsId The a2dp audio stream session id.
     * @param isPlaying The new playing state, play: true or stop : false.
     *
     * @since 6.0
     */
    void A2dpOffloadUpdateSessionsState(std::vector<int> sessionsId, bool isPlaying);

    /**
     * @brief judge a2dp offload have audio session on playing state.
     *
     * @return Returns <b>true</b> have audio session on playing state.
     *         Returns <b>false</b> no audio seesion on playing state.
     * @since 10.0
     */
    bool A2dpOffloadIsSessionOnPlaying(void);

    /**
     * @brief a2dp offload delayed to handle stop when audio framework call OffloadStopPlaying.
     *
     * @param device The raw address of the bluetooth device.
     * @since 10.0
     */
    void A2dpOffloadHandleDelayStop(const RawAddress &device);

    /**
     * @brief Report connected state change.
     *
     * @since 12.0
     */
    void NotifyConnStateChangedInner(const RawAddress &device, int state, int cause);

    /**
     * @brief Report hdap connected state change.
     *
     * @param device The raw address of the bluetooth device.
     * @since 12.0
     */
    void NotifyCaptureConnStateChangedInner(const RawAddress &device);

    /**
     * @brief Notify media stack changed.
     * @param action Action on the device.
     * @param device The address of the peer device.
     * @since 10.0
     */
    void BluetoothNotifyMediaStackChanged(int action, const RawAddress &device);

    /**
     * @brief send device selection messages to remote device.
     *
     * @param device The remote device.
     * @param useA2dp use A2dp.
     * @param useHfp use Hfp.
     * @param userSelect user select.
     * @return code of send device selection messages to remote address.
     * @since 21
     */
    int ConnectDeviceMax(const RawAddress &device, const std::string &callingName);
    void ConvertCodecConfig(BtavA2dpCodecConfig &codecConfig, const A2dpSrcCodecInfo &codecInfo);
    void ClearAvrcpLoadedMap();
    bool SubscribeCommonEvent();
    int IsLocalDeviceConnectAllowed(const RawAddress &device);

    std::vector<BtavA2dpCodecConfig> prepareCodecPreferences();

    class ReceiveMessage : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        ReceiveMessage(const EventFwk::CommonEventSubscribeInfo &subscriberInfo, A2dpService &a2dpService)
            : EventFwk::CommonEventSubscriber(subscriberInfo),
              a2dpService_(a2dpService) {};
 
        void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
 
    private:
        A2dpService &a2dpService_;
    };

    int maxConnectNumSnk_ = A2DP_CONNECT_NUM_MAX;             // max number of connected peer device
    uint8_t role_ = A2DP_ROLE_SOURCE;                         // A2DP role
    uint32_t profileId_ = PROFILE_ID_A2DP_SRC;                // profile ID
    RawAddress activeDevice_ {};                               // Address to active device
    A2dpConnectManager connectManager_ {};                     // A2dpConnectManager's object pointer.
    std::string name_ = "";                                   // service name
    std::string version_ = "";                                // profile's version
    std::map<std::string, std::shared_ptr<A2dpDeviceInfo>> a2dpDevices_ {};   // devicelist
    BaseObserverList<IA2dpObserver> a2dpFramworkCallback_ {};  // callback of framework.
    bool isDoDisable = false;                                 // if device will disable
    std::vector<HwBtA2dpSteamSessionInfo> a2dpStreamSessions_ {};
    std::shared_ptr<utility::Timer> offloadDelayStopTimer_ {nullptr};
    std::shared_ptr<utility::Timer> timerForRestoreVolumeIfPaused_ = nullptr;
    std::atomic<bool> isRenderActive_ = false;
    bool isNavigationRunning_ = false;
    utility::SafeVector<std::string> virtualDevices_ = {};

    BtavSourceInterface* sBluetoothA2dpSrcInterface = nullptr;
    BtavSinkInterface* sBluetoothA2dpSnkInterface = nullptr;
    const std::string NULL_ADDRESS {""};
    const std::string EMPTY_ADDRESS {"00:00:00:00:00:00"};
    utility::SafeVector<std::string> hdfLoadedDevice_ = {};
    utility::SafeVector<std::string> avrcpLoadedDevice_ = {};
    utility::SafeVector<std::string> a2dpLoadedDevice_ = {};

    std::mutex renderSenceChangeMutex_;
    bool anyRenderRunning_ = false;
    bool gameRunning_ = false;
    bool musicRunning_ = false;
    bool isLocalConnect_ = false; // Check whether the connection is initiated by the local for chr
    bool isInWirelessCharging_ = false;
    CodecChangeAppScene codecChangeScene_ = CodecChangeAppScene::APP_TYPE_OTHER;
    int lastIwareAppType_ = -1; // default is unknown type
    std::shared_ptr<A2dpDeviceInfo> hdapDevice_ = nullptr;
    std::shared_ptr<ReceiveMessage> subscriber_ = nullptr;
    std::function<void(int)> a2dpStatecallback_;
    bool isLastOnLowLatency_ = false;
};

/**
 * @brief A function to get profile instance
 *
 * @param[in] role The role of profile
 * @since 6.0
 */
A2dpService *GetServiceInstance(uint8_t role);

void PreferenceL2hcStCodec(std::vector<BtavA2dpCodecConfig> &codecPreferences);
}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SERVICE_H
