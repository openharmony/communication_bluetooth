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
 * @file bluetooth_hid_device.h
 *
 * @brief Declares HID DEVICE role framework functions, including basic and observer functions.
 *
 */
#ifndef BLUETOOTH_HID_DEVICE_H
#define BLUETOOTH_HID_DEVICE_H
#include <string>
#include <vector>
#include <memory>
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_no_destructor.h"
#include "bluetooth_hid_device_info.h"
#include "bluetooth_address_info.h"
#include "bluetooth_hid_device_sdp_parcel.h"
#include "bluetooth_hid_device_qos_parcel.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for Hid Device observer functions.
 *
 */
class HidDeviceObserver {
public:
    /**
     * @brief The observer function to notify app registration status changed.
     *
     * @param state registration status.
     */
    virtual void OnAppStatusChanged(int state) {}

    /**
     * @brief The observer function to notify connection state changed.
     *
     * @param device Remote device object.
     * @param state Connection state.
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state) {}

    /**
     * @brief The observer function to notify message when GET_REMOTE message is received from remote.
     *
     * @param type reportType of GET_RREPORT data.
     * @param id id of GET_RREPORT data.
     * @param bufferSize  bufferSize of GET_RREPORT data, maximum number of octets to transfer during data phase.
     */
    virtual void OnGetReport(int type, int id, uint16_t bufferSize) {}

    /**
     * @brief The observer function to notify message when InterruptData message is received from remote.
     *
     * @param protocol protocol of protocol data.
     */
    virtual void OnSetProtocol(int protocol) {}

    /**
     * @brief The observer function to notify message when InterruptData message is received from remote.
     *
     * @param id id of Interrupt data.
     * @param data data of Interrupt data.
     */
    virtual void OnInterruptDataReceived(int id, std::vector<uint8_t> data) {}

    /**
     * @brief The observer function to notify message when SET_REMOTE message is received from remote.
     *
     * @param type report type of SET_REPORT data.
     * @param id id of SET_REPORT data.
     * @param data data of SET_REPORT data.
     */
    virtual void OnSetReport(int type, int id, std::vector<uint8_t> data) {}

    /**
     * @brief The observer function to notify when virtual cable is removed.
     *
     */
    virtual void OnVirtualCableUnplug() {}

    /**
     * @brief Destroy the HidDeviceObserver object.
     *
     */
    virtual ~HidDeviceObserver() {}
};

/**
 * @brief Class for Hid Host API.
 *
 */
class BLUETOOTH_API HidDevice {
public:
    /**
     * @brief Get the instance of Hid Device object.
     *
     * @return Returns the pointer to the Hid Device instance.
     */
    static HidDevice *GetProfile();

    /**
     * @brief Static Hid Device observer instance.
     *
     */
    static HidDeviceObserver *instance_;

        /**
     * @brief Register Hid Device observer instance.
     *
     * @param observer Hid Device observer instance.
     */
    void RegisterObserver(std::shared_ptr<HidDeviceObserver> observer);

    /**
     * @brief Deregister Hid Device observer instance.
     *
     * @param observer Hid Device observer instance.
     */
    void DeregisterObserver(std::shared_ptr<HidDeviceObserver> observer);

    /**
     * @brief Get remote Hid host list which are in connection state.
     *
     * @param states List of remote device states.
     * @return Returns the list of devices.
     */
    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice> &result);

    /**
     * @brief Get the connection state of the specified remote Hid host.
     *
     * @param device Remote device object.
     * @return Returns the connection state of the remote device.
     */
    int32_t GetConnectionState(const BluetoothRemoteDevice &device, int32_t &state);

    /**
     * @brief Application registers the HID Device capability.
     *        Application will only successfully call this API when it's in the foreground.
     *
     * @param sdpSetting Describe the hid device capability fields of this endpoint being quired.
     * @param inQos Describe the In Quality of Service(Qos).
     * @param outQos Describe the Out Quality of Service(Qos).
     */
    int32_t RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
        const BluetoothHidDeviceQos &inQos,
        const BluetoothHidDeviceQos &outQos);

    /**
     * @brief Application unregisters the HID Device capability.
     *
     */
    int32_t UnregisterHidDevice();

    /**
     * @brief Initiate the establishment of a service level connection to remote Hid host.
     *
     * @param device Remote device object.
     */
    int32_t Connect(const AddressInfo &device);

    /**
     * @brief Release the connection from remote Hid host.
     *
     * @param device Remote device object.
     */
    int32_t Disconnect();

    /**
     * @brief Send report to a remote hid host device.
     *
     * @param id Report Id, as is defined in descriptor.
     * @param data Report data send to host.
     */
    int32_t SendReport(int id, const std::vector<uint8_t> &data);

    /**
     * @brief Reply report to a remote hid host device.
     *
     * @param type Report type fpr reply.
     * @param id Report Id, as is defined in descriptor.
     * @param data Report data send to host.
     */
    int32_t ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data);

    /**
     * @brief Report error to a remote hid host device.
     *
     * @param type Error reason to send.
     */
    int32_t ReportError(ErrorReason type);

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
    int32_t SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy);

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
    int32_t GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const;
private:
    HidDevice();
    ~HidDevice();
    int32_t CheckProfileState() const;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidDevice);
    BLUETOOTH_DECLARE_IMPL();
#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<HidDevice>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_HID_DEVICE_H
