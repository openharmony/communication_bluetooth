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
 * @file bluetooth_hfp_ag.h
 *
 * @brief Declares HFP AG role framework functions, including basic and observer functions.
 *
 * @since 6
 */

#ifndef BLUETOOTH_HFP_AG_H
#define BLUETOOTH_HFP_AG_H

#include <string>
#include <vector>
#include <memory>
#include <list>

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for HandsFree AudioGateway observer functions.
 *
 * @since 6
 */
class HandsFreeAudioGatewayObserver {
public:
    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     * @param cause Connecton cause.
     * @since 12
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int32_t state, int32_t cause)
    {}

    /**
     * @brief The observer function to notify audio connection state changed.
     *
     * @param device Remote device object.
     * @param state Audio connection state.
     * @param reason Reason for Audio status change.
     * @since 6
     */
    virtual void OnScoStateChanged(const BluetoothRemoteDevice &device, int32_t state, int32_t reason)
    {}

    /**
     * @brief The observer function to notify active device changed.
     *
     * @param device Remote active device object.
     * @since 6
     */
    virtual void OnActiveDeviceChanged(const BluetoothRemoteDevice &device)
    {}

    /**
     * @brief The observer function to notify enhanced driver safety changed.
     *
     * @param device Remote device object.
     * @param indValue Enhanced driver safety value.
     * @since 6
     */
    virtual void OnHfEnhancedDriverSafetyChanged(const BluetoothRemoteDevice &device, int32_t indValue)
    {}

    /**
     * @brief The observer function to notify audio framework the hfp stack changed.
     *
     * @param device Remote device object.
     * @param action Action on the device.
     * @since 11
     */
    virtual void OnHfpStackChanged(const BluetoothRemoteDevice &device, int32_t action)
    {}

    /**
     * @brief Destroy the HandsFreeAudioGateway Observer object.
     *
     * @since 6
     */
    virtual ~HandsFreeAudioGatewayObserver()
    {}

    /**
     * @brief The observer function to notify virtual device changed.
     *
     * @param Action on the device.
     * @param device bluetooth device address.
     * @since 12
     */
    virtual void OnVirtualDeviceChanged(int32_t action, std::string address)
    {}
};

/**
 * @brief Class for HandsFree AudioGateway API.
 *
 * @since 6
 */
class BLUETOOTH_API HandsFreeAudioGateway {
public:
    /**
     * @brief Get the instance of HandsFreeAudioGateway object.
     *
     * @return Returns the pointer to the HandsFreeAudioGateway instance.
     * @since 6
     */
    static HandsFreeAudioGateway *GetProfile();

    /**
     * @brief Get remote HF device list which are in the connected state.
     *
     * @return Returns the list of devices.
     * @since 6
     */
    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice>& devices);

    /**
     * @brief Get remote HF device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @return Returns the list of devices.
     * @since 6
     */
    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states);

    /**
     * @brief Get the connection state of the specified remote HF device.
     *
     * @param device Remote device object.
     * @return Returns the connection state of the remote device.
     * @since 6
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state);

    /**
     * @brief Initiate the establishment of a service level connection to remote HF device.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    int32_t Connect(const BluetoothRemoteDevice &device);

    /**
     * @brief Release the audio connection from remote HF device.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

    /**
     * @brief Get the Audio connection state of the specified remote HF device.
     *
     * @param device Remote device object.
     * @return Returns the Audio connection state.
     * @since 6
     */
    int GetScoState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Initiate the establishment of an audio connection to remote active HF device.
     *
     * @param callType the type of the call, Refer to enum BtCallType
     * @return Returns <b>0</b> if the operation is successful; returns <b>Other</b> if the operation fails.
     * @since 10
     */
    int32_t ConnectSco(uint8_t callType);

    /**
     * @brief Release the audio connection from remote active HF device.
     *
     * @param callType the type of the call, Refer to enum BtCallType
     * @return Returns <b>0</b> if the operation is successful; returns <b>Other</b> if the operation fails.
     * @since 10
     */
    int32_t DisconnectSco(uint8_t callType);

    /**
     * @brief Initiate the establishment of an audio connection to remote active HF device.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool ConnectSco();

    /**
     * @brief Release the audio connection from remote active HF device.
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool DisconnectSco();

    /**
     * @brief This function used to Update changed phone call informations.
     *
     * @param numActive Active call number.
     * @param numHeld Held call number.
     * @param callState Current call state.
     * @param number Phone call number.
     * @param type Type of phone call number.
     * @param name Name of phone call number.
     * @since 6
     */
    void PhoneStateChanged(
        int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name);

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
    void ClccResponse(int index, int direction, int status, int mode, bool mpty, const std::string &number, int type);

    /**
     * @brief Open the voice recognition.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device);

    /**
     * @brief Close the voice recognition.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device);

    /**
     * @brief Set the active device for audio connection.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetActiveDevice(const BluetoothRemoteDevice &device);

    bool IntoMock(const BluetoothRemoteDevice &device, int state);
    bool SendNoCarrier(const BluetoothRemoteDevice &device);

    /**
     * @brief Get the active device object.
     *
     * @param device Remote active device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    BluetoothRemoteDevice GetActiveDevice() const;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>BT_ERR_PERMISSION_FAILED</b> Permission denied.
     *         Returns <b>BT_ERR_INVALID_PARAM</b> Input error.
     *         Returns <b>BT_ERR_INVALID_STATE</b> BT_ERR_INVALID_STATE.
     *         Returns <b>BT_ERR_INTERNAL_ERROR</b> Operation failed.
     * @since 10.0
     */
    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy);

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>BT_ERR_PERMISSION_FAILED</b> Permission denied.
     *         Returns <b>BT_ERR_INVALID_PARAM</b> Input error.
     *         Returns <b>BT_ERR_INVALID_STATE</b> BT_ERR_INVALID_STATE.
     *         Returns <b>BT_ERR_INTERNAL_ERROR</b> Operation failed.
     * @since 10.0
     */
    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const;

    /**
     * @brief Register HandsFree AudioGateway observer instance.
     *
     * @param observer HandsFree AudioGateway observer instance.
     * @since 6
     */
    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer);

    /**
     * @brief Deregister HandsFree AudioGateway observer instance.
     *
     * @param observer HandsFree AudioGateway observer instance.
     * @since 6
     */
    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer);

    /**
     * @brief Get remote HF device list which are in the connected state.
     *
     * @return Returns the list of devices.
     * @since 6
     */
    std::vector<BluetoothRemoteDevice> GetConnectedDevices() const;

    /**
     * @brief Check whether in-band-ringing is enabled
     *
     * @return Returns <b>true</b> if the in-band-ringing is enbale;
     * returns <b>false</b> if the in-band-ringing is disable.
     * @since 11
     */
    int IsInbandRingingEnabled(bool &isEnabled) const;

    /**
     * @brief This function used to update changed phone call state.
     *
     * @param callId Current call id.
     * @param callState Current call state.
     * @since 11
     */
    void CallDetailsChanged(int callId, int callState);

    /**
     * @brief Enable or disable the bluetooth log.
     *
     * @param state True: enable log, false: diaable log.
     * @since 11
     */
    void EnableBtCallLog(bool state);

    /**
     * @brief Check whether vgs is supported.
     *
     * @param device The address of the peer bluetooth device.
     * @param isSupported Whether vgs is supported.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>BT_ERR_PERMISSION_FAILED</b> Permission denied.
     *         Returns <b>BT_ERR_INVALID_PARAM</b> Input error.
     *         Returns <b>BT_ERR_INVALID_STATE</b> BT_ERR_INVALID_STATE.
     *         Returns <b>BT_ERR_INTERNAL_ERROR</b> Operation failed.
     * @since 12
     */
    int IsVgsSupported(const BluetoothRemoteDevice &device, bool &isSupported) const;

    /**
     * get virtual device list.
     *
     * @param devices virtual device list.
     * @since 12
     */
    void GetVirtualDeviceList(std::vector<std::string> &devices) const;

    /**
     * @brief Static HandsFree AudioGateway observer instance.
     *
     * @since 6
     */
    static HandsFreeAudioGateway *instance_;

    /**
     * @brief The external process calls the HfpAg profile interface before the Bluetooth process starts. At this
     * time, it needs to monitor the start of the Bluetooth process, and then call this interface to initialize the
     * HfpAg proflie.
     */
    void Init();

private:
    HandsFreeAudioGateway();
    ~HandsFreeAudioGateway();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HandsFreeAudioGateway);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<HandsFreeAudioGateway>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_HFP_AG_H
