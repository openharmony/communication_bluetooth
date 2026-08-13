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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic bluetooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 * @since 6
 */

/**
 * @file interface_profile_hfp_ag.h
 *
 * @brief Declares HFP AG role interface profile functions, including basic and observer functions.
 *
 * @since 6
 */

#ifndef INTERFACE_PROFILE_HFP_AG_H
#define INTERFACE_PROFILE_HFP_AG_H

#include "interface_profile.h"
#include <vector>
#include <string>
#include "bluetooth_phone_state.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Class for HfpAgServiceObserver functions.
 *
 * @since 6
 */
class HfpAgServiceObserver {
public:
    /**
     * @brief Destroy the HfpAgServiceObserver Observer object.
     *
     * @since 6
     */
    virtual ~HfpAgServiceObserver() = default;

    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     * @param cause Disconnection cause.
     * @since 12
     */
    virtual void OnConnectionStateChanged(const RawAddress &device, int state, int cause)
    {}

    /**
     * @brief The observer function to notify audio connection state changed.
     *
     * @param device Remote device object.
     * @param state Audio connection state.
     * @param reason Reason for change status.
     * @since 6
     */
    virtual void OnScoStateChanged(const RawAddress &device, int state, int reason = 0)
    {}

    /**
     * @brief The observer function to notify active device changed.
     *
     * @param device Remote active device object.
     * @since 6
     */
    virtual void OnActiveDeviceChanged(const RawAddress &device)
    {}

    /**
     * @brief The observer function to notify enhanced driver safety changed.
     *
     * @param device Remote device object.
     * @param indValue Enhanced driver safety value.
     * @since 6
     */
    virtual void OnHfEnhancedDriverSafetyChanged(const RawAddress &device, int indValue)
    {}

    /**
     * @brief The observer function to notify audio framework the hfp stack changed.
     *
     * @param device Remote device object.
     * @param action Action on the device.
     * @since 10
     */
    virtual void OnHfpStackChanged(const RawAddress &device, int action)
    {}

    /**
     * @brief The observer function to notify virtual device changed.
     *
     * @param action add or remove.
     * @param address address of virtual device.
     * @since 10
     */
    virtual void OnVirtualDeviceChanged(int32_t action, std::string address)
    {}
};

/**
 * @brief Class for IProfileHfpAg API.
 *
 * @since 6
 */
class IProfileHfpAg : public IProfile {
public:
    /**
     * @brief Destroy the IProfileHfpAg object as default.
     *
     * @since 6
     */
    ~IProfileHfpAg() override = default;

    /**
     * @brief Get remote HF device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @return Returns the list of devices.
     * @since 6
     */
    virtual std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) = 0;

    /**
     * @brief Get the connection state of the specified remote HF device.
     *
     * @param device Remote device object.
     * @return Returns the connection state of the remote device.
     * @since 6
     */
    virtual int GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief Get the Audio connection state of the specified remote HF device.
     *
     * @param device Remote device object.
     * @return Returns the Audio connection state.
     * @since 6
     */
    virtual int GetScoState(const RawAddress &device) = 0;

    /**
     * @brief Initiate the establishment of an audio connection to remote active HF device.
     *
     * @param callType the type of the call, Refer to enum BTCallType.
     * @return Returns <b>0</b> if the operation is successful; returns <b>other</b> if the operation fails.
     * @since 10
     */
    virtual int32_t ConnectSco(uint8_t callType) = 0;

    /**
     * @brief Release the audio connection from remote active HF device.
     *
     * @return Returns <b>0</b> if the operation is successful; returns <b>other</b> if the operation fails.
     * @since 10
     */
    virtual int32_t DisconnectSco(uint8_t callType) = 0;

    /**
     * @brief Initiate the establishment of an audio connection to remote active HF device.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool ConnectSco() = 0;

    /**
     * @brief Release the audio connection from remote active HF device.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DisconnectSco() = 0;

    /**
     * @brief This function used to Update changed phone call information.
     *
     * @param phoneState Bluetooth phone state.
     * @since 6
     */
    virtual void PhoneStateChanged(Bluetooth::BluetoothPhoneState &phoneState, bool isVirtualCall = false) = 0;

    /**
     * @brief Send response for querying standard list current calls by remote Hf device.
     *
     * @param index Index of the current call.
     * @param direction Direction of the current call.
     * @param status Status of the current call.
     * @param mode Source Mode of the current call.
     * @param mpty Is this call a member of a conference call.
     * @param number Phone call number.
     * @param type Type of phone call number.
     * @since 6
     */
    virtual void ClccResponse(
        int index, int direction, int status, int mode, bool mpty, const std::string &number, int type) = 0;

    /**
     * @brief Open the voice recognition.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool OpenVoiceRecognition(const RawAddress &device) = 0;

    /**
     * @brief Close the voice recognition.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool CloseVoiceRecognition(const RawAddress &device) = 0;

    /**
     * @brief Set the active device for audio connection.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetActiveDevice(const RawAddress &device) = 0;

    /**
     * @brief Set mock state.
     *
     * @param state mock state.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 9
     */
    virtual bool IntoMock(int state) = 0;

    /**
     * @brief Set no carrier.
     *
     * @param device Remote device object..
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 9
     */
    virtual bool SendNoCarrier(const RawAddress &device) = 0;

    /**
     * @brief Get the active device object.
     *
     * @param device Remote active device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual std::string GetActiveDevice() = 0;

    virtual bool IsAudioConnected() = 0;

    /**
     * @brief Register HandsFree AudioGateway observer instance.
     *
     * @param observer HandsFree AudioGateway observer instance.
     * @since 6
     */
    virtual void RegisterObserver(HfpAgServiceObserver &observer) = 0;

    /**
     * @brief Deregister HandsFree AudioGateway observer instance.
     *
     * @param observer HandsFree AudioGateway observer instance.
     * @since 6
     */
    virtual void DeregisterObserver(HfpAgServiceObserver &observer) = 0;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 10.0
     */
    virtual int SetConnectStrategy(const RawAddress &device, int strategy) = 0;

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>CONNECTION_ALLOWED</b> if the peer device is allowed to connect.
     *         Returns <b>CONNECTION_FORBIDDEN</b> if the peer device is not allowed to connect.
     *         Returns <b>CONNECTION_UNKNOWN</b> if the connection policy is unknown.
     * @since 10.0
     */
    virtual int GetConnectStrategy(const RawAddress &device) = 0;

    /**
     * @brief Notify audio volume info.
     * @param[in] streamType The stream type.
     * @param[in] volume The device volume.
     */
    virtual void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume) = 0;

    /**
     * @brief Check whether in-band-ringing is enabled
     *
     * @param isEnabled whether in-band-ringing is enabled
     * @return Returns {@link BT_NO_ERROR} if the operation fails.
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 11
     */
    virtual bool IsInbandRingingEnabled() = 0;

    /**
     * @brief Notify phonebook accessAuthorization Result.
     * @param device The address of the peer bluetooth device.
     * @param accessAuthorization Reference to the access authorization.
     * @param isPbapLoad Reference to whether pbap is load.
     */
    virtual void NotifyPhonebookAuthResult(const RawAddress& device, int32_t accessAuthorization,
        int32_t pbapLoadFlag) = 0;

    /**
     * @brief This function used to update changed phone call state.
     *
     * @param callId Current call id.
     * @param callState Current call state.
     * @since 11
     */
    virtual void CallDetailsChanged(int callId, int callState) = 0;

    /**
     * Enable or disable the bluetooth log.
     *
     * @param state true: Enable the log. false: Disable the log.
     * @since 11
     */
    virtual void EnableBtCallLog(bool state) = 0;

    /**
     * @brief Check whether vgs is supported.
     *
     * @param device The address of the peer bluetooth device.
     * @param isSupported Whether vgs is supported.
     * @return Returns {@link BT_NO_ERROR} if the operation fails.
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 12
     */
    virtual int IsVgsSupported(const RawAddress &device, bool &isSupported) = 0;

    /**
    * @brief update hfp virtual device.
    *
    * @param action 0: add, 1: remove.
    * @param address address of virtual device.
    * @since 12
    */
    virtual void UpdateVirtualDevice(int32_t action, const std::string &address) = 0;

    /**
    * @brief get virtual device list.
    *
    * @param devices address of virtual device list.
    * @since 12
    */
    virtual void GetVirtualDeviceList(std::vector<std::string> &devices) = 0;

    /**
     * @brief Get current call type.
     *
     * @param callType current call type.
     * @return Returns {@link BT_NO_ERROR} if the operation successful.
     * @since 20
     */
    virtual int GetCurrentCallType(int &callType) = 0;
    /**
    * @brief Check whether voice recognition is enabled.
    *
    * @param device The address of the peer bluetooth device.
    * @param isSupported Whether voice recognition is supported.
    * @return Returns {@link BT_NO_ERROR} if the operation fails.
    *         returns an error code defined in {@link BtErrCode} otherwise.
    * @since 20
    */
    virtual int IsVoiceRecognitionSupported(const RawAddress &device, bool &isSupported) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // INTERFACE_PROFILE_HFP_AG_H