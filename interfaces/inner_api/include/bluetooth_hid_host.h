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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic bluetooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 */

/**
 * @file bluetooth_hid_host.h
 *
 * @brief Declares HID HOST role framework functions, including basic and observer functions.
 *
 */
#ifndef BLUETOOTH_HID_HOST_H
#define BLUETOOTH_HID_HOST_H

#include <string>
#include <vector>
#include <memory>

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_no_destructor.h"
namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for Hid Host observer functions.
 *
 */
class HidHostObserver {
public:
    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     * @param cause Connection cause.
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
    {}

    /**
     * @brief Destroy the HidHostObserver object.
     *
     */
    virtual ~HidHostObserver()
    {}
};

/**
 * @brief Class for Hid Host API.
 *
 */
class BLUETOOTH_API HidHost {
public:
    /**
     * @brief Get the instance of HidHost object.
     *
     * @return Returns the pointer to the HidHost instance.
     */
    static HidHost *GetProfile();

    /**
     * @brief Get remote Hid device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @return Returns the list of devices.
     */
    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result);

    /**
     * @brief Get the connection state of the specified remote Hid device.
     *
     * @param device Remote device object.
     * @return Returns the connection state of the remote device.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state);

    /**
     * @brief Initiate the establishment of a service level connection to remote Hid device.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    int32_t Connect(const BluetoothRemoteDevice &device);

    /**
     * @brief Release the connection from remote Hid device.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

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
     * @brief Register Hid Host observer instance.
     *
     * @param observer Hid Host observer instance.
     */
    void RegisterObserver(std::shared_ptr<HidHostObserver> observer);

    /**
     * @brief Deregister Hid Host observer instance.
     *
     * @param observer Hid Host observer instance.
     */
    void DeregisterObserver(std::shared_ptr<HidHostObserver> observer);

    /**
     * @brief Hid Host VCUnplug.
     *
     * @param observer Hid Host device id size type.
     */
    void HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type);

    /**
     * @brief Hid Host Send Data.
     *
     * @param observer Hid Host device id size type.
     */
    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type);

    /**
     * @brief Hid Host Set Report.
     *
     * @param observer Hid Host device type size report.
     */
    void HidHostSetReport(std::string device, uint8_t type, std::string &report);

    /**
     * @brief Hid Host Get Report.
     *
     * @param observer Hid Host device id size type.
     */
    void HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type);

    /**
     * @brief The external process calls the HidHost profile interface before the Bluetooth process starts. At this
     * time, it needs to monitor the start of the Bluetooth process, and then call this interface to initialize the
     * HidHost proflie.
     */
    void Init();

    /**
     * @brief Static Hid Host observer instance.
     *
     */
    static HidHostObserver *instance_;

private:
    HidHost();
    ~HidHost();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidHost);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<HidHost>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_HID_HOST_H