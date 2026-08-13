/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DEVICE_BATTERY_MANAGER
#define BLUETOOTH_DEVICE_BATTERY_MANAGER

#include "log.h"
#include "util/safe_vector.h"
#include "bluetooth_device.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
enum BatteryEventType {
    TYPE_BATTERY_NOTIFY = 0,
    TYPE_BATTERY_UPDATE = 1,
    TYPE_BATTERY_CANCEL = 2,
};

enum DeviceBatteryType {
    NORMAL_DEVICE_BATTERY = 0,
    SPECIAL_DEVICE_BATTERY = 1,
};

enum ChargeState {
    NORMAL_CHARGE_NOT_CHARGING = 0,
    NORMAL_CHARGE_CHARGING = 1,
    SUPER_CHARGE_NOT_CHARGING = 2,
    SUPER_CHARGE_CHARGING = 3,
};

const uint8_t KEY_NORMAL_DEVICE_BATTERY_LEVEL = 1;
const uint8_t KEY_LEFT_EAR_BATTERY_LEVEL = 2;
const uint8_t KEY_LEFT_EAR_CHARGE_STATE = 3;
const uint8_t KEY_RIGHT_EAR_BATTERY_LEVEL = 4;
const uint8_t KEY_RIGHT_EAR_CHARGE_STATE = 5;
const uint8_t KEY_BOX_BATTERY_LEVEL = 6;
const uint8_t KEY_BOX_CHARGE_STATE = 7;
const uint8_t KEY_EXCEPTION = 8;
const uint8_t KEY_BOX_ID = 9;
const uint8_t KEY_LEFT_EAR_MODEL_ID = 10;
const uint8_t KEY_RIGHT_EAR_MODEL_ID = 11;
const uint8_t KEY_HEADSET_POPUP_INFO = 12;
const uint8_t KEY_HEADSET_POSITION_INFO = 13;

const uint8_t BATTERY_OBTAIN_TYPE_AT = 0x01; // 从AT命令中获取到的电量信息
const uint8_t BATTERY_OBTAIN_TYPE_ECHO = 0x02; // 从ECHO命令中获取到的电量信息
const uint8_t BATTERY_OBTAIN_TYPE_SYSTEM_HAP = 0x03; // audio server等系统应用通过私有协议获取并设置的电量信息

class BluetoothDeviceBatteryManager {
public:
    BluetoothDeviceBatteryManager();
    ~BluetoothDeviceBatteryManager();
    static BluetoothDeviceBatteryManager* GetInstance();
    bool ProcessBatteryCommond(const std::string &addr, const std::string &atCommand);
    bool ProcessHwBatteryCommond(const std::string &addr, const std::string &atCommand, uint8_t obtainType);
    void ProcessActiveDeviceChanged(const std::string &profileName, const std::string &addr);
    void ProcessHfpDisconnected(const std::string &addr);
    void ProcessHfpConnected(const std::string &addr);
    void ProcessAclStateChanged(const std::string &addr, int32_t connectState);
    void ProcessFindDeviceCommond(const std::string &addr, const std::vector<uint8_t> &originalVec);
    void DeviceConnUpdate(const std::string &addr, uint8_t earphonePropVal, uint8_t connStateVal);
    void DeviceControlStatus(const std::string &addr, uint8_t leftVal, uint8_t rightVal);
    void SetRemoteDeviceBatteryInfo(const RawAddress &device, const BatteryInfo &batteryInfo);

private:
    bool ProcessIphoneAccevBatteryEvent(const std::string &addr, const std::string &commandValue);

    void ProcessHwBatteryInfo(const std::string &addr, const std::string &commandValue, uint8_t obtainType);
    bool ParseBatteryInfo(const std::string &commandValue, BatteryInfo &info);
    void SetBatteryInfo(uint8_t key, int32_t value, BatteryInfo &info);
    bool CheckBatteryInfoValid(uint8_t key, int32_t value);
    void LogBatteryInfo(const BatteryInfo &info);
    std::string GetCurrentBatteryDevice(bool isBatteryDevice);
    void ProcessAclConnected(const std::string &addr);
    void ProcessAclDisconnected(const std::string &addr);
    int32_t GetDeviceReportBatteryLevel(const std::string &addr);
    int32_t CalcuBatteryLevel(const BatteryDetailInfo &info);
    void UpdateBatteryInfoAndReport(const std::string &addr, uint8_t obtainType, const BatteryInfo &info);
    void UpdateDeviceBatteryInfo(const BatteryInfo &info, uint8_t obtainType, BatteryDetailInfo &batteryDetailInfo);
    bool IsInCharge(const BatteryInfo &info);

    void NotifyForBatteryInfoChanged(const std::string &addr, int32_t batteryLevel);
    void NotifyForDeviceChanged(const std::string &newBatteryDevice);
    void ReportDeviceBatteryInfo(const std::string &addr, int32_t batteryLevel);
    void ReportOriginalEchoInfo(const std::string &addr, const std::string &commandValue);
    std::vector<uint8_t> GetDescripValue(const std::string &addr, const std::vector<uint8_t> &originalVec);

    void SetDeviceSupportBattery(const std::string &addr);
    std::string GetLastConnectedDevice(bool isBatteryDevice);
    // 管理connectedDevices_
    std::pair<std::string, bool> FindConnectedDevice(const std::string &addr);
    void AddConnectedDevice(std::string addr);
    void UpdateBatteryStatus(std::string addr, bool batteryState);
private:
    std::string a2dpActiveDevice_ = "";
    std::string hfpActiveDevice_ = "";
    std::string currentBatteryDevice_ = "";
    int32_t reportBatteryLevel_ = -1;
    SafeMap<std::string, bool> batteryDeviceMap_ {};
    utility::SafeVector<std::pair<std::string, bool>> connectedDevices_ = {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_DEVICE_BATTERY_MANAGER