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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines bluetooth bas host, including observer and common functions.
 *
 * @since 26
 */

/**
 * @file bluetooth_bas_host.h
 *
 * @brief Framework bluetooth bas host interface.
 *
 * @since 26
 */

#ifndef BLUETOOTH_BAS_HOST_H
#define BLUETOOTH_BAS_HOST_H

#include <map>
#include <string>
#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_no_destructor.h"
#include "refbase.h"

namespace OHOS { class IRemoteObject; }
namespace OHOS {
namespace Bluetooth {

/**
 * @brief Represents remote device battery observer.
 *
 * @since 26
 */
class BluetoothRemoteDeviceBatteryObserver {
public:
    /**
     * @brief A destructor used to delete the <b>BluetoothRemoteDeviceBatteryObserver</b> instance.
     *
     * @since 26
     */
    virtual ~BluetoothRemoteDeviceBatteryObserver() = default;

    /**
     * @brief Get batteryLevel event.
     *
     * @param device Remote device.
     * @param batteryLevel battery level. default value or no response value: -1.
     * @since 26
     */
    virtual void OnGetBatteryLevelEvent(const BluetoothRemoteDevice &device, int32_t batteryLevel) {};

    /**
     * @brief Notify batteryLevel changed.
     *
     * @param device Remote device.
     * @param batteryLevel battery level. default value or no response value: -1.
     * @since 26
     */
    virtual void OnBatteryLevelChanged(const BluetoothRemoteDevice &device, int32_t batteryLevel) {};
};

/**
 * @brief Represents framework bas host device.
 *
 * @since 26
 */
class BLUETOOTH_API BluetoothBasHost {
public:
    /**
     * @brief Get the instance of BluetoothBasHost object.
     *
     * @return Returns the pointer to the BluetoothBasHost instance.
     * @since 26
     */
    static BluetoothBasHost *GetProfile();

    /**
     * @brief Determine whether the local device can obtain the battery level of the remote device.
     *
     * @param[out] isSupported Returns <b>true</b> if the battery service is enabled;
     *             returns <b>false</b> if the battery service is disabled.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 26
     */
    int32_t IsBasSupported(bool &isSupported);

    /**
     * @brief Register remote device battery observer.
     *
     * @param observer Class BluetoothRemoteDeviceBatteryObserver pointer to register observer.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 26
     */
    int32_t RegisterBatteryObserver(std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer);

    /**
     * @brief Deregister remote device battery observer.
     *
     * @param observer Class BluetoothRemoteDeviceBatteryObserver pointer to deregister observer.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 26
     */
    int32_t DeregisterBatteryObserver(std::shared_ptr<BluetoothRemoteDeviceBatteryObserver> observer);

    /**
     * @brief Request the battery level of a remote device.
     *
     * @param address The address of the remote device.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 26
     */
    int32_t GetBatteryLevel(const std::string &address);

    /**
     * @brief Get the battery levels of all connected remote devices.
     *
     * @param[out] batteryInfos Returns a map where the key is the device address
     *             and the value is the battery level (0-100).
     *             If the battery level is not available, the value is -1.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 26
     */
    int32_t GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos);

private:
    /**
     * @brief A constructor used to create a <b>BluetoothBasHost</b> instance.
     *
     * @since 26
     */
    BluetoothBasHost();

    /**
     * @brief A destructor used to delete the <b>BluetoothBasHost</b> instance.
     *
     * @since 26
     */
    ~BluetoothBasHost();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBasHost);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<BluetoothBasHost>;
#endif
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_BAS_HOST_H