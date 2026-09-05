/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_PBAP_PCE_H
#define BLUETOOTH_PBAP_PCE_H

#include <memory>
#include <string>
#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for Pbap PCE observer functions.
 *
 */
class PbapPceObserver {
public:
    /**
     * @brief Destroy the PbapPceObserver object.
     *
     */
    virtual ~PbapPceObserver() = default;

    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     * @param cause Connection cause.
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int32_t state, int32_t cause)
    {}

    /**
     * @brief Observes the phonebook sync sub-state changes.
     *
     * @param device Remote device object.
     * @param syncState Phonebook sync sub-state, one of PhoneBookSyncStateType.
     */
    virtual void OnSyncStateChange(const BluetoothRemoteDevice &device, int32_t syncState)
    {}
};

/**
 * @brief Class for Pbap Client API.
 *
 */
class BLUETOOTH_API PbapPce {
public:
    /**
     * @brief Get the instance of PbapPce object.
     *
     * @return Returns the pointer to the PbapPce instance.
     */
    static PbapPce *GetProfile();

    /**
     * @brief Register PbapPce observer instance.
     *
     * @param observer PbapPce observer instance.
     */
    void RegisterObserver(std::shared_ptr<PbapPceObserver> observer);

    /**
     * @brief Deregister PbapPce observer instance.
     *
     * @param observer PbapPce observer instance.
     */
    void DeregisterObserver(std::shared_ptr<PbapPceObserver> observer);

    /**
     * @brief Get remote pbap pce device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @param result the list of devices
     * @return Returns operate result.
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRemoteDevice> &result) const;

    /**
     * @brief Get the connection state of the specified remote pbap device.
     *
     * @param device Remote device object.
     * @param state the connection state of the remote device
     * @return Returns operate result.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const;

    /**
     * @brief establish the connection from remote pbap device.
     *
     * @param device Remote device object.
     * @return Returns operate result.
     */
    int32_t Connect(const BluetoothRemoteDevice &device);

    /**
     * @brief Release the connection from remote pbap device.
     *
     * @param device Remote device object.
     * @return Returns operate result.
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

    /**
     * @brief Set connection strategy for remote bluetooth device.
     *
     * @param device Remote device object.
     * @param strategy The device connect strategy.
     * @return Returns operate result
     */
    int32_t SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy);

    /**
     * @brief Get connection strategy of remote bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns operate result.
     */
    int32_t GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const;

    /**
     * @brief Get the phonebook connection synchronization status of the specified remote pbap device.
     *
     * @param device Remote device object.
     * @param state the connection synchronization status of the remote device.
     * @return Returns operate result.
     */
    int32_t GetPhoneBookSyncState(const BluetoothRemoteDevice &device) const;

    void Init();
    void Uinit();

private:
    /**
     * @brief constructor.
     *
     */
    PbapPce();

    /**
     * @brief deconstructor.
     *
     */
    ~PbapPce();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPce);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<PbapPce>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PBAP_PCE_H