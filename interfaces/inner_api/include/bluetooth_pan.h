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
 * @file bluetooth_pan.h
 *
 * @brief Declares PAN role framework functions, including basic and observer functions.
 *
 */
#ifndef BLUETOOTH_PAN_H
#define BLUETOOTH_PAN_H

#include <string>
#include <vector>
#include <memory>

#include "bluetooth_def.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"
#include "bluetooth_no_destructor.h"
namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for pan observer functions.
 *
 */
class PanObserver {
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
     * @brief Destroy the PanObserver object.
     *
     */
    virtual ~PanObserver()
    {}
};

/**
 * @brief Class for Pan API.
 *
 */
class BLUETOOTH_API Pan {
public:
    /**
     * @brief Get the instance of Pan object.
     *
     * @return Returns the pointer to the Pan instance.
     */
    static Pan *GetProfile();

    /**
     * @brief Get remote Pan device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @return Returns the list of devices.
     */
    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result);

    /**
     * @brief Get the connection state of the specified remote Pan device.
     *
     * @param device Remote device object.
     * @return Returns the connection state of the remote device.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state);

    /**
     * @brief Release the connection from remote Pan device.
     *
     * @param device Remote device object.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

        /**
     * @brief Register Pan observer instance.
     *
     * @param observer Pan observer instance.
     */
    void RegisterObserver(std::shared_ptr<PanObserver> observer);

    /**
     * @brief Deregister Pan observer instance.
     *
     * @param observer Pan observer instance.
     */
    void DeregisterObserver(std::shared_ptr<PanObserver> observer);

    /**
     * @brief Set Tethering
     *
     * @param value Set Network Sharing.
     */
    int32_t SetTethering(bool value);

    /**
     * @brief Is Tethering On.
     *
     */
    int32_t IsTetheringOn(bool &value);

    /**
     * @brief The external process calls the Pan profile interface before the Bluetooth process starts. At this
     * time, it needs to monitor the start of the Bluetooth process, and then call this interface to initialize the
     * Pan proflie.
     */
    void Init();

    /**
     * @brief Static Pan observer instance.
     *
     */
    static PanObserver *instance_;

private:
    Pan();
    ~Pan();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(Pan);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<Pan>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PAN_H