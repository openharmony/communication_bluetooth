/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
 * @file bluetooth_hearingaid.h
 *
 * @brief Declares hearingAid framework functions, including basic and observer functions.
 *
 */
#ifndef BLUETOOTH_HEARINGAID_H
#define BLUETOOTH_HEARINGAID_H

#include <string>
#include <vector>
#include <memory>
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace Bluetooth {

constexpr const char* PROFILE_HEARINGAID_SERVER = "BluetoothHearingAidServer";

class HearingAidObserver {
public:
    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param address indicate which device's connection status has changed.
     * @param state indicate what the connection status has changed to.
     * @param cause the reason of state changed.
     */
    virtual void OnConnectionStateChanged(const std::string& address, int32_t state, int32_t cause)
    {}

    /**
     * @brief The observer function to notify active device changed.
     *
     * @param address indicate which device has changed to active device.
     */
    virtual void OnActiveDeviceChanged(const std::string& address, const std::string& name)
    {}

    /**
     * @brief Destroy the HearingAidObserver object.
     *
     */
    virtual ~HearingAidObserver()
    {}
};

class BluetoothHearingAid {
public:
    /**
     * @brief Get the instance of BluetoothHearingAid object.
     *
     * @return Returns the pointer to the BluetoothHearingAid instance.
     */
    static BluetoothHearingAid* GetProfile();

    /**
     * @brief Local device support hearing aid or not.
     *
     * @return Return operate result to confirm whether the capability is successfully got.
     */
    int32_t IsLocalDeviceSupportHearingAid(bool& support);

    /**
     * @brief Connect remote hearing aid device.
     *
     * @return Return operate result to confirm whether the Connect is successful.
     */
    int32_t Connect(const std::string& address);

    /**
     * @brief Disconnect remote hearing aid device.
     *
     * @return Return operate result to confirm whether the Disconnect is successful.
     */
    int32_t Disconnect(const std::string& address);

    /**
     * @brief Set the volume of the remote hearing aid device.
     *
     * @return Return operate result to confirm whether the volume is successfully set.
     */
    int32_t SetVolume(const std::string& address, int64_t volume);

    /**
     * @brief Get the information of the remote hearing aid devices.
     *
     * @return Return operate result to confirm whether the information is successfully got.
     */
    int32_t GetHearingAidDeviceInfo(const std::string& address, int32_t& side, int32_t& mode, int64_t& id);

    /**
     * @brief Get connected devices.
     *
     * @return Return operate result to confirm whether the devices is successfully got.
     */
    int32_t GetConnectedDevices(std::vector<std::string>& connectedDevices);

    /**
     * @brief Get active device.
     *
     * @return Return operate result to confirm whether the devices is successfully got.
     */
    int32_t GetActiveDevice(std::string& address, std::string& name);

    /**
     * @brief Get the connection state of the remote hearing aid device.
     *
     * @return Return operate result to confirm whether the state is successfully got.
     */
    int32_t GetConnectionState(const std::string& address, int32_t& state);

    /**
     * @brief Set the connection strategy of the remote hearing aid device.
     *
     * @return Return operate result to confirm whether the strategy is successfully set.
     */
    int32_t SetConnectionStrategy(const std::string& address, int32_t strategy);

    /**
     * @brief Get the connection strategy of the remote hearing aid device.
     *
     * @return Return operate result to confirm whether the strategy is successfully got.
     */
    int32_t GetConnectionStrategy(const std::string& address, int32_t& strategy);

    /**
     * @brief Register HearingAidObserver instance.
     *
     * @param observer HearingAidObserver instance.
     */
    void RegisterObserver(std::shared_ptr<HearingAidObserver> observer);

    /**
     * @brief Deregister HearingAidObserver instance.
     *
     * @param observer HearingAidObserver instance.
     */
    void DeregisterObserver(std::shared_ptr<HearingAidObserver> observer);
private:
    BluetoothHearingAid();
    BluetoothHearingAid(const BluetoothHearingAid &) = delete;
    BluetoothHearingAid& operator=(const BluetoothHearingAid &) = delete;
    struct impl;
    std::unique_ptr<impl> pimpl;
    const int64_t VOLUME_MIN = -128;
    const int64_t VOLUME_MAX = 0;
    const int32_t STRATEGY_MIN = 0;
    const int32_t STRATEGY_MAX = 2;

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<BluetoothHearingAid>;
#endif
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif