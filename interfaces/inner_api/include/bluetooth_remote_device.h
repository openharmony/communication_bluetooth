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

#ifndef BLUETOOTH_REMOTE_DEVICE_H
#define BLUETOOTH_REMOTE_DEVICE_H

#include <string>
#include <vector>

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_device_class.h"
#include "bluetooth_battery_info.h"

namespace OHOS {
namespace Bluetooth {
class BLUETOOTH_API BluetoothRemoteDevice {
public:
    /// common
    /**
     * @brief A structor used to create the <b>BluetoothRemoteDevice</b> instance.
     *
     * @since 6
     */
    BluetoothRemoteDevice(){};
    /**
     * @brief A structor used to create the <b>BluetoothRemoteDevice</b> instance.
     *
     * @since 6
     */
    BluetoothRemoteDevice(const std::string &addr, const int transport = BT_TRANSPORT_NONE);
    /**
     * @brief A destructor used to delete the <b>BluetoothRemoteDevice</b> instance.
     *
     * @since 6
     */
    virtual ~BluetoothRemoteDevice(){};

    /**
     * @brief Get phonebook permission for device.
     *
     * @return Returns permission grade.
     *         BTPermissionType::ACCESS_UNKNOWN;
     *         BTPermissionType::ACCESS_ALLOWED;
     *         BTPermissionType::ACCESS_FORBIDDEN.
     * @since 6
     */
    int GetPhonebookPermission() const;

    /**
     * @brief Set phonebook permission for device.
     *
     * @param permission permission grade.
     *        BTPermissionType::ACCESS_UNKNOWN;
     *        BTPermissionType::ACCESS_ALLOWED;
     *        BTPermissionType::ACCESS_FORBIDDEN.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetPhonebookPermission(int permission);

    /**
     * @brief Get message permission for device.
     *
     * @return Returns permission grade.
     *         BTPermissionType::ACCESS_UNKNOWN;
     *         BTPermissionType::ACCESS_ALLOWED;
     *         BTPermissionType::ACCESS_FORBIDDEN.
     * @since 6
     */
    int GetMessagePermission() const;

    /**
     * @brief Set message permission for device.
     *
     * @param permission permission grade.
     *        BTPermissionType::ACCESS_UNKNOWN;
     *        BTPermissionType::ACCESS_ALLOWED;
     *        BTPermissionType::ACCESS_FORBIDDEN.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetMessagePermission(int permission);

    /**
     * @brief Get power mode for device.
     *
     * @return Returns power mode grade.
     *         BTPowerMode::MODE_INVALID = 0x00,
     *         BTPowerMode::MODE_ACTIVE = 0x100,
     *         BTPowerMode::MODE_SNIFF_LEVEL_LOW = 0x201,
     *         BTPowerMode::MODE_SNIFF_LEVEL_MID = 0x202,
     *         BTPowerMode::MODE_SNIFF_LEVEL_HIG = 0x203,
     * @since 6
     */
    int GetPowerMode(void) const;

    // gap

    /**
     * @brief Get device address.
     *
     * @return Returns device adress.
     * @since 6
     */
    std::string GetDeviceAddr() const
    {
        return address_;
    };

    /**
     * @brief Get device name.
     *
     * @return Returns device name.
     * @since 6
     */
    std::string GetDeviceName() const;

    /**
     * @brief Get device name.
     *
     * @return Returns device name.
     * @since 6
     */
    int GetDeviceName(std::string &name) const;

    /**
     * @brief Get device alias.
     *
     * @return Returns device alias.
     * @since 6
     */
    std::string GetDeviceAlias() const;

    /**
     * @brief Set device alias.
     *
     * @param aliasName Device alias name.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>other error code</b> if the operation fails.
     * @since 6
     */
    int32_t SetDeviceAlias(const std::string &aliasName);

    /**
     * @brief Get device type.
     *
     * @return Returns device type.
     * @since 6
     */
    int GetDeviceType() const;

    /**
     * @brief Get device battery levele.
     *
     * @param[out] batteryInfo the device battery info.
     * @return Returns operation code
     * @since 12
     */
    int GetRemoteDeviceBatteryInfo(DeviceBatteryInfo &batteryInfo) const;

    /**
     * @brief Get device pair state.
     *
     * @return Returns device pair state.
     * @since 6
     */
    int GetPairState(int &pairState) const;

    /**
     * @brief Device start pair.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int StartPair();

    /**
     * @brief Credible Device start pair.
     *
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 10
     */
    int StartCrediblePair();

    /**
     * @brief Cancel pair operation.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int CancelPairing();

    /**
     * @brief Check if device was bonded from local.
     *
     * @return Returns <b>true</b> if device was bonded from local;
     *         returns <b>false</b> if device was not bonded from local.
     * @since 6
     */
    bool IsBondedFromLocal() const;

    /**
     * @brief Check if device acl connected.
     *
     * @return Returns <b>true</b> if device acl connected;
     *         returns <b>false</b> if device does not acl connect.
     * @since 6
     */
    bool IsAclConnected() const;

    /**
     * @brief Check if device acl Encrypted.
     *
     * @return Returns <b>true</b> if device acl Encrypted;
     *         returns <b>false</b> if device does not acl Encrypt.
     * @since 6
     */
    bool IsAclEncrypted() const;

    /**
     * @brief Get device class.
     *
     * @return Returns device class;
     * @since 6
     */
    int GetDeviceClass(int &cod) const;

    /**
     * @brief Get the product id of a remote device.
     *
     * @return Returns device product id;
     * @since 11
     */
    int GetDeviceProductId(std::string &prodcutId) const;

    /**
     * @brief Get device uuids.
     *
     * @return Returns device uuids;
     * @since 6
     */
    int GetDeviceUuids(std::vector<std::string> &uuids) const;

    /**
     * @brief Set device pair pin.
     *
     * @param pin Pin code.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int SetDevicePin(const std::string &pin);

    /**
     * @brief Set device pairing confirmation.
     *
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int SetDevicePairingConfirmation(bool accept);

    /**
     * @brief Set device pair passkey.
     *
     * @param passkey Device passkey.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetDevicePasskey(int passkey, bool accept);

    /**
     * @brief Check device pair request reply.
     *
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool PairRequestReply(bool accept);

    /**
     * @brief Get device transport type.
     *
     * @return Returns device transport type;
     * @since 6
     */
    int GetTransportType() const;

    // ble
    /**
     * @brief Read remote device rssi value.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool ReadRemoteRssiValue();

    /**
     * @brief Check if bluetooth remote device is valid.
     *
     * @return Returns <b>true</b> if bluetooth remote device is valid;
     *         returns <b>false</b> if bluetooth remote device is not valid.
     * @since 6
     */
    bool IsValidBluetoothRemoteDevice() const;

    /**
     * @brief Get the product type of the device, such as headsets, watchs and car.
     *
     * @param cod Class of device.
     * @param majorClass The major class.
     * @param majorMinorClass The minor class.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 11
     */
    int GetDeviceProductType(int &cod, int &majorClass, int &majorMinorClass) const;

    /**
     * @brief Set the product type of the device, such as headsets, watchs and car.
     *
     * @param deviceType The type of device.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t SetDeviceCustomType(int32_t deviceType) const;

    /**
     * @brief Get the product type of the device, such as headsets, watchs and car.
     *
     * @param deviceType The type of device.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t GetDeviceCustomType(int32_t &deviceType) const;

    /**
     * @brief Get the vendor id of the device.
     *
     * @param[out] vendorId The vendor id of device.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t GetDeviceVendorId(uint16_t &vendorId) const;

    /**
     * @brief Get the product id of the device.
     *
     * @param[out] productId The product id of device.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t GetDeviceProductId(uint16_t &productId) const;

    /**
     * @brief Get remote device is supported virtual auto connect.
     *
     * @param[out] outSupport The remote device configured auto connect value.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t IsSupportVirtualAutoConnect(bool &outSupport) const;

    /**
     * @brief Set remote device auto connect type and business connect type.
     *
     * @param connType Auto connect type value.
     * @param businessType Business connect type value.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    int32_t SetVirtualAutoConnectType(int connType, int businessType) const;

    /**
     * @brief Control the actions of Bluetooth peripherals.
     *
     * @param controlType The control type.
     * @param controlTypeVal The value of control type.
     * @param controlObject The control object.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 16
     */
    int32_t ControlDeviceAction(uint32_t controlType, uint32_t controlTypeVal, uint32_t controlObject) const;

    /**
     * @brief Get latest connection time of device.
     *
     * @return latest connection time, or 0 if there was a problem.
     * @since 16
     */
    int32_t GetLastConnectionTime(int64_t &connectionTime) const;

    int32_t GetCloudBondState(int32_t &cloudBondState) const;
private:
    std::string address_ = "00:00:00:00:00:00";
    int transport_ = BT_TRANSPORT_NONE;
};
}  // namespace Bluetooth
} // namespace OHOS

#endif  // BLUETOOTH_REMOTE_DEVICE_H
