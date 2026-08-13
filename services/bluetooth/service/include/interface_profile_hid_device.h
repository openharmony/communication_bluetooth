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
 * @brief Defines hid device service interface, including observer and api functions.
 *
 */

/**
 * @file interface_profile_hid_device.h
 *
 * @brief hid device interface.
 *
 */

 #ifndef INTERFACE_PROFILE_HID_DEVICE_H
#define INTERFACE_PROFILE_HID_DEVICE_H

#include "interface_profile.h"
#include "bluetooth_hid_device_sdp_parcel.h"
#include "bluetooth_hid_device_qos_parcel.h"


namespace OHOS {
namespace Bluetooth {
/**
 * @brief hid device observer for framework api
 *
 */
class IHidDeviceObserver {
public:
    /**
     * @brief Destroy the IHidDeviceObserver object
     *
     */
    virtual ~IHidDeviceObserver() = default;
    virtual void OnAppStatusChanged(int state) = 0;
    virtual void OnConnectionStateChanged(bluetooth::RawAddress device, int state) = 0;
    virtual void OnGetReport(int type, int id, uint16_t bufferSize) = 0;
    virtual void OnInterruptDataReceived(int report_id, std::vector<uint8_t> data) = 0;
    virtual void OnSetProtocol(int protocol) = 0;
    virtual void OnSetReport(int type, int id, std::vector<uint8_t> data) = 0;
    virtual void OnVirtualCableUnplug() = 0;  

};

class IProfileHidDevice : public bluetooth::IProfile {
public:
    /**
     * @brief  register observer
     *
     * @param  observer         function pointer
     */
    virtual void RegisterObserver(IHidDeviceObserver &observer) = 0;
    /**
     * @brief  deregister observer
     *
     * @param  observer         function pointer
     */
    virtual void DeregisterObserver(IHidDeviceObserver &observer) = 0;
    
    virtual int RegisterHidDevice(BluetoothHidDeviceSdp sdp, 
       BluetoothHidDeviceQos inQos, BluetoothHidDeviceQos outQos) = 0;
    
    virtual int AutoUnregisterHidDevice() = 0;
    virtual int UnregisterHidDevice() = 0;

    virtual int32_t SendReport(int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReportError(uint8_t type) = 0;

    virtual int SetConnectStrategy(const bluetooth::RawAddress &device, int strategy) = 0;
    virtual int GetConnectStrategy(const bluetooth::RawAddress &device) = 0;

    virtual int DisconnectHidDevice() = 0;
    virtual int GetConnectionState(const bluetooth::RawAddress &device) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // INTERFACE_PROFILE_HID_DEVICE_H
