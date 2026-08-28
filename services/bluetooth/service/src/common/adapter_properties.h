/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ADAPTER_PROPERTIES_H
#define ADAPTER_PROPERTIES_H

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>

#include "base_def.h"
#include "bluetooth.h"
#include "bt_def.h"
#include "bt_types.h"
#include "bt_uuid.h"
#include "classic_config.h"
#include "classic_defs.h"
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {

constexpr int MAX_ALIAS_LENGTH = 64;
constexpr int32_t DM_OK = 0;
// Thread safety object
class AdapterProperties {
/// IO capabilities
#define GAP_IO_DISPLAYONLY 0x00
#define GAP_IO_DISPLAYYESNO 0x01
#define GAP_IO_KEYBOARDONLY 0x02
#define GAP_IO_NOINPUTNOOUTPUT 0x03
#define GAP_IO_KEYBOARDDISPLAY 0x04

public:
    AdapterProperties();
    ~AdapterProperties();
    static AdapterProperties *GetInstance(void);

    void LoadHostInfo();
    bool LoadConfigInfo();
    bool LoadLocalName();
    bool RegisterNameChangeObserver();
    void UnregisterNameChangeObserver();
    void HandleNameChange();

    void ParseAdapterProps(BtStackStatus status, int numProperties, BtProperty *properties);
    std::string ParseDeviceName(BtProperty* property);
    RawAddress ParseDeviceAddr(BtProperty* property);
    std::vector<Uuid> ParseDeviceUuid(BtProperty* property);
    uint32_t ParseDeviceType(BtProperty* property);
    uint32_t ParseDeviceCod(BtProperty* property);
    int8_t ParseDeviceRssi(BtProperty* property);
    int ParseDeviceIoCapability(BtProperty* property);
    bool ParseScanMode(BtProperty* property, int &scanMode);
    std::vector<std::string> ParseBondedDevices(BtProperty* property);
    void ParseLocalLeFeatures(BtProperty* property);
    int32_t ParseAbsVolumeAbility(BtProperty* property);
    std::string ParseAlias(BtProperty* property);
    int32_t ParseDeviceCustomType(BtProperty* property);
    int32_t ParseRemoteDeviceIoCapability(BtProperty* property);
    int32_t ParseDeviceVendorId(BtProperty* property);
    int32_t ParseDeviceProductId(BtProperty* property);
    int32_t ParseDeviceAutoConnSwitch(BtProperty* property);
    bool GetSettingDeviceName(std::string &value);
    bool SetDeviceName(std::string name);
    bool SetBroadcastName(std::string deviceName);
    std::string GetDeviceName(void) const;

    void SetDeviceAddress(std::string deviceAddress);
    std::string GetDeviceAddress(void) const;

    void SetServiceUuids(std::vector<Uuid> uuids);
    std::vector<Uuid> GetServiceUuids(void) const;

    std::vector<std::string> GetPairedAddrList(void) const;
    void RemovePairedDeviceList(std::string addr);
    std::vector<std::string> AddPairedAddrList(RawAddress &device);

    bool SetDeviceClass(int deviceClass);
    int GetDeviceClass(void) const;

    void SetDeviceType(uint32_t deviceType);
    uint32_t GetDeviceType(void) const;

    bool SetIoCapability(int ioCapability);
    uint8_t GetIoCapability(void) const;

    void SetDeviceRssi(uint8_t deviceRssi);
    uint8_t GetDeviceRssi(void) const;

    uint16_t GetTotalNumOfTrackableAdvertisements(void) const;
    uint16_t GetBleMaxAdvertisingDataLength(void) const;
    bool GetLeExtendedAdvertisingSupported(void) const;
    bool GetLe2mPhySupported(void) const;
    int64_t ParseDeviceConnectionTime(BtProperty* property);
private:

    uint32_t ConvertDeviceTypeFromBluetdroid(uint32_t deviceType);
    void HandlePropertyLocalAddress(BtProperty* property);
    void HandlePropertyScanMode(BtProperty* property);
    std::string GetTruncationName(const std::string &deviceName);
    int GetUTF8StringLength(const char firstByte);
    int GetValidUTF8StringLength(const std::string &name);
    void StartQueryDeviceNameTimer();
    // properties mutex
    mutable std::mutex mutex_ {};

    // Bluetooth device name
    std::string deviceName_ {DEFAULT_DEVICE_NAME};
    // Bluetooth device address
    std::string macAddr_ {INVALID_MAC_ADDRESS};
    // Bluetooth service 128-bit UUIDs
    std::vector<Uuid> uuids_ {};
    // Bluetooth Class of Device as found in Assigned Numbers
    std::atomic<int> cod_ {DEFAULT_CLASS_OF_DEVICE};
    // Device Type - BREDR, BLE or DUAL Mode
    uint32_t deviceType_ = DEVICE_TYPE_UNKNOWN;
    // List of bonded devices
    std::vector<std::string> pairedAddrList_ {};
    // Local Input/Output Capabilities for Bluetooth, type: BLE_IO_CAP in bt_def.h
    std::atomic<int> ioCapability_ {GAP_IO_DISPLAYYESNO};
    // The current set of supported LE features as obtained from the stack. The
    // values here are all initially set to 0 and updated when the corresponding
    // adapter property has been received from the stack.
    BtLocalLeFeatures localLeFeatures_ {};
    ClassicConfig &config_;
    std::shared_ptr<utility::Timer> queryDeviceNameTimeout_ = nullptr;
    int32_t delayQueryTime = 5000; // delay 5s

    enum {
        UTF8_INVALID_BYTE_LENGTH,
        UTF8_SINGLE_BYTE_LENGTH,
        UTF8_DOUBLE_BYTE_LENGTH,
        UTF8_TRIPLE_BYTE_LENGTH,
        UTF8_QUADRUPLE_BYTE_LENGTH
    };

    BT_DISALLOW_COPY_AND_ASSIGN(AdapterProperties);
    BT_DISALLOW_MOVE_AND_ASSIGN(AdapterProperties);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
