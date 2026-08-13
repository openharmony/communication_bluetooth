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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_device_battery"
#endif

#include "log.h"
#include "bluetooth_common_event_helper.h"
#include "common_util.h"
#include "classic_adapter.h"
#include "remote_device_properties.h"
#include "interface_adapter_manager.h"
#include "interface_profile_hfp_ag.h"
#include "interface_profile_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
const uint8_t IPHONEACCEV_BATTERY_LEVEL_KEY = 1;
const uint8_t KV_PAIR_NUMBER = 2;
const uint8_t KEY_INDEX = 1;
const uint8_t VALUE_INDEX = 2;

const uint8_t MAX_BATTERY_LEVEL = 100;
const uint8_t MAX_IPHONEACCEV_BATTERY_LEVEL = 9;
const int32_t INVALID_BATTERY_LEVEL = -1;

const uint8_t IRK_LEN = 1;
const uint8_t HBKP_LEN = 1;
const uint8_t ARGS_FOUR = 4;
const uint8_t MAX_REMOTE_NAME_LEN = 64;

const uint8_t DEVICE_DISCONNECTED = 0; // 0 means disconnect

bool CheckAddressValid(const std::string &addr)
{
    if (addr.empty() || addr == "00:00:00:00:00:00") {
        return false;
    }
    return true;
}

void BluetoothDeviceBatteryManager::ProcessFindDeviceCommond(const std::string &addr,
    const std::vector<uint8_t> &originalVec)
{
    CHECK_AND_RETURN_LOG(!originalVec.empty(), "originalVec is null");
    std::vector<uint8_t> descripValue = GetDescripValue(addr, originalVec);
    CHECK_AND_RETURN_LOG(!descripValue.empty(), "descripValue is null");
    HILOGI("addr: %{public}s, originalVec.size: %{public}d, descripValue.size: %{public}d",
        GET_ENCRYPT_STR_ADDR(addr), originalVec.size(), descripValue.size());

    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    classicAdapter->SendRemoteEchoInfo(RawAddress(addr), descripValue);
}

// 将设备名输出为固定长度64
static std::string NameTo64Bytes(const std::string &deviceNameStr)
{
    std::string deviceNameReal = deviceNameStr;
    if (deviceNameReal.length() > MAX_REMOTE_NAME_LEN) {
        deviceNameReal.resize(MAX_REMOTE_NAME_LEN);
    }
    uint8_t paddingLength = MAX_REMOTE_NAME_LEN - deviceNameReal.length();
    if (paddingLength > 0) {
        deviceNameReal.append(paddingLength, ' ');
    }
    return deviceNameReal;
}

static void convertArrayToString(const std::vector<uint8_t> &arr, std::string &dstStr)
{
    for (size_t i = 0; i < arr.size(); i++) {
        dstStr += DexToHexString((arr[i] & 0xF0) >> ARGS_FOUR);
        dstStr += DexToHexString(arr[i] & 0x0F);
    }
}

std::vector<uint8_t> BluetoothDeviceBatteryManager::GetDescripValue(const std::string &addr,
    const std::vector<uint8_t> &originalVec)
{
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(addr));
    std::vector<uint8_t> ret = {};
    CHECK_AND_RETURN_LOG_RET(remoteDevice != nullptr, ret, "remoteDevice is nullptr");

    // 转换成utf8可解析的dstStr
    std::string dstStr;
    std::string commandStr = "+FINDHWDEVICE:";
    dstStr += commandStr;
    dstStr += addr;
    convertArrayToString(originalVec, dstStr);
    std::string deviceNameStr = remoteDevice->GetRemoteName();
    std::string deviceNameReal = NameTo64Bytes(deviceNameStr);
    dstStr += deviceNameReal;
    std::vector<uint8_t> irkArr(IRK_LEN, 0);
    convertArrayToString(irkArr, dstStr);
    std::vector<uint8_t> hbkPArr(HBKP_LEN, 0);
    convertArrayToString(hbkPArr, dstStr);
    std::vector<uint8_t> dstVec(dstStr.begin(), dstStr.end());
    return dstVec;
}

void BluetoothDeviceBatteryManager::DeviceConnUpdate(const std::string &addr,
    uint8_t earphonePropVal, uint8_t connStateVal)
{
    if (connStateVal == 0xFF) { // 0xFF判断为未连接
        connStateVal = 0x00;
    }
    
    HILOGI("addr: %{public}s, earphonePropVal: %{public}u, connStateVal: %{public}u",
        GET_ENCRYPT_STR_ADDR(addr), earphonePropVal, connStateVal);
    std::vector<uint8_t> connUpdateValue;
    std::string commandStr = "+FINDHWDEVICECONNUPDATE:";
    std::vector<uint8_t> command(commandStr.begin(), commandStr.end());
    connUpdateValue.insert(connUpdateValue.end(), command.begin(), command.end());
    connUpdateValue.push_back(earphonePropVal);
    connUpdateValue.push_back(connStateVal);

    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    classicAdapter->SendRemoteEchoInfo(RawAddress(addr), connUpdateValue);
}

void BluetoothDeviceBatteryManager::DeviceControlStatus(const std::string &addr, uint8_t leftVal, uint8_t rightVal)
{
    HILOGI("addr: %{public}s, leftVal: %{public}u, rightVal: %{public}u",
        GET_ENCRYPT_STR_ADDR(addr), leftVal, rightVal);
    std::vector<uint8_t> controlStatusValue;
    std::string commandStr = "+FINDHWDEVICECTRLSTATUS:";
    std::vector<uint8_t> command(commandStr.begin(), commandStr.end());
    controlStatusValue.insert(controlStatusValue.end(), command.begin(), command.end());
    controlStatusValue.push_back(leftVal);
    controlStatusValue.push_back(rightVal);

    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    classicAdapter->SendRemoteEchoInfo(RawAddress(addr), controlStatusValue);
}

bool IsHfpConnected(const std::string &addr)
{
    CHECK_AND_RETURN_LOG_RET(CheckAddressValid(addr), false, "device not exist");
    IProfileHfpAg *agService =
        static_cast<IProfileHfpAg *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG_RET(agService != nullptr, false, "agService is null");
    if (agService->GetDeviceState(RawAddress(addr)) != static_cast<int>(BTConnectState::CONNECTED)) {
        return false;
    }
    return true;
}

BluetoothDeviceBatteryManager::BluetoothDeviceBatteryManager()
{}

BluetoothDeviceBatteryManager::~BluetoothDeviceBatteryManager()
{}

BluetoothDeviceBatteryManager *BluetoothDeviceBatteryManager::GetInstance()
{
    static BluetoothDeviceBatteryManager instance;
    return &instance;
}

bool BluetoothDeviceBatteryManager::ProcessBatteryCommond(const std::string &addr, const std::string &atCommand)
{
    std::string commandValue = GetCommandValue(atCommand);
    if (commandValue.empty()) {
        HILOGE("wrong atCommand: %{public}s", atCommand.c_str());
        return false;
    }

    bool result = false;
    if (StartWith(atCommand, "+IPHONEACCEV")) {
        SetDeviceSupportBattery(addr);
        result = ProcessIphoneAccevBatteryEvent(addr, commandValue);
    } else {
        HILOGE("wrong atCommand: %{public}s", atCommand.c_str());
    }
    return result;
}

bool BluetoothDeviceBatteryManager::ProcessHwBatteryCommond(const std::string &addr, const std::string &atCommand,
    uint8_t obtainType)
{
    return false;
}

bool BluetoothDeviceBatteryManager::ProcessIphoneAccevBatteryEvent(const std::string &addr,
    const std::string &commandValue)
{
    if (commandValue.empty()) {
        HILOGE("commandValue: %{public}s", commandValue.c_str());
        return false;
    }
    std::vector<std::string> values = SplitValueByDelim(commandValue, ",");
    if (values.size() == 0) {
        HILOGE("values size error");
        return false;
    }
    uint8_t kvPairNum = 0;
    if (!ConvertStrToDigit(values[0], kvPairNum)) {
        return false;
    }
    if (kvPairNum * KV_PAIR_NUMBER + 1 != values.size()) { // kvPairNum * (key + value) + kvPairNum(1)
        HILOGE("kvPairNum: %{public}u, size: %{public}u", kvPairNum, values.size());
        return false;
    }

    uint8_t indicatorValue = 0xFF;
    for (uint8_t i = 0; i < kvPairNum; i++) {
        uint8_t indicatorType = 0;
        if (!ConvertStrToDigit(values[i * KV_PAIR_NUMBER + KEY_INDEX], indicatorType)) {
            return false;
        }
        if (indicatorType != IPHONEACCEV_BATTERY_LEVEL_KEY) {
            continue;
        }
        if (!ConvertStrToDigit(values[i * KV_PAIR_NUMBER + VALUE_INDEX], indicatorValue)) {
            return false;
        }
        break;
    }
    if (indicatorValue > MAX_IPHONEACCEV_BATTERY_LEVEL) {
        HILOGE("indicatorValue = %{public}u", indicatorValue);
        return false;
    }
    uint8_t battery = (indicatorValue + 1) * 10; // 0-9 mean 0-90%, plus 10%
    HILOGI("battery: %{public}u", battery);
    BatteryInfo info;
    SetBatteryInfo(KEY_NORMAL_DEVICE_BATTERY_LEVEL, battery, info);
    UpdateBatteryInfoAndReport(addr, BATTERY_OBTAIN_TYPE_AT, info);
    return true;
}

void BluetoothDeviceBatteryManager::ProcessHwBatteryInfo(const std::string &addr, const std::string &commandValue,
    uint8_t obtainType)
{
    HILOGI("addr: %{public}s, value: %{public}s,", GET_ENCRYPT_STR_ADDR(addr), commandValue.c_str());
    BatteryInfo info;
    if (!ParseBatteryInfo(commandValue, info)) {
        HILOGE("parse fail, addr: %{public}s, value = %{public}s,", GET_ENCRYPT_STR_ADDR(addr), commandValue.c_str());
        return;
    }
    LogBatteryInfo(info);
    UpdateBatteryInfoAndReport(addr, obtainType, info);
}

void BluetoothDeviceBatteryManager::ReportOriginalEchoInfo(const std::string &addr, const std::string &commandValue)
{
    HILOGI("addr: %{public}s, commandValue: %{public}s,", GET_ENCRYPT_STR_ADDR(addr), commandValue.c_str());
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    std::vector<uint8_t> vec(commandValue.begin(), commandValue.end());
    classicAdapter->SendRemoteEchoInfo(RawAddress(addr), vec);
}

void BluetoothDeviceBatteryManager::SetRemoteDeviceBatteryInfo(const RawAddress &device, const BatteryInfo &batteryInfo)
{
    HILOGI("address(%{public}s)", GetEncryptAddr(device.GetAddress()).c_str());
    SetDeviceSupportBattery(device.GetAddress());
    UpdateBatteryInfoAndReport(device.GetAddress(), BATTERY_OBTAIN_TYPE_SYSTEM_HAP, batteryInfo);
}

void BluetoothDeviceBatteryManager::UpdateBatteryInfoAndReport(const std::string &addr, uint8_t obtainType,
    const BatteryInfo &info)
{
    if (obtainType != BATTERY_OBTAIN_TYPE_AT && obtainType != BATTERY_OBTAIN_TYPE_ECHO &&
        obtainType != BATTERY_OBTAIN_TYPE_SYSTEM_HAP) {
        HILOGE("unknow obtainType, type = %{public}u", obtainType);
        return;
    }

    if (!CheckAddressValid(addr)) {
        HILOGE("addr is invalid");
        return;
    }

    BatteryDetailInfo batteryDetailInfo;
    if (!RemoteDeviceProperties::GetInstance()->GetDeviceBatteryInfo(addr, batteryDetailInfo)) {
        HILOGE("addr: %{public}s not exist", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }
    UpdateDeviceBatteryInfo(info, obtainType, batteryDetailInfo);
    RemoteDeviceProperties::GetInstance()->SetDeviceBatteryInfo(addr, batteryDetailInfo);

    NotifyForBatteryInfoChanged(addr, CalcuBatteryLevel(batteryDetailInfo));

    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    classicAdapter->SendRemoteBatteryChanged(RawAddress(addr), batteryDetailInfo.batteryInfo_);
}

bool BluetoothDeviceBatteryManager::IsInCharge(const BatteryInfo &info)
{
    return (info.leftEarChargeState_ == NORMAL_CHARGE_CHARGING || info.leftEarChargeState_ == SUPER_CHARGE_CHARGING) &&
        (info.rightEarChargeState_ == NORMAL_CHARGE_CHARGING || info.rightEarChargeState_ == SUPER_CHARGE_CHARGING);
}

void BluetoothDeviceBatteryManager::UpdateDeviceBatteryInfo(const BatteryInfo &info, uint8_t obtainType,
    BatteryDetailInfo &batteryDetailInfo)
{
    // 当存储的信息只有echo信息(可能是HFP断连后清除了AT的标志位)需直接更新所有信息
    // 信息显示左右耳均充电时，初始化AT电量，避免AT电量更新不及时导致状态栏电量不增加
    // 如果是系统应用下设的电量，直接更新
    if ((obtainType == BATTERY_OBTAIN_TYPE_SYSTEM_HAP) ||
        (obtainType == BATTERY_OBTAIN_TYPE_ECHO &&
        (batteryDetailInfo.batteryObtainType_ == BATTERY_OBTAIN_TYPE_ECHO || IsInCharge(info)))) {
        batteryDetailInfo.batteryInfo_ = info;
        return;
    }

    batteryDetailInfo.batteryObtainType_ |= obtainType;
    // 上报的电量是包含左右耳电量信息
    if (info.batteryLevel_ == INVALID_BATTERY_LEVEL) {
        batteryDetailInfo.batteryInfo_.leftEarBatteryLevel_ = info.leftEarBatteryLevel_;
        batteryDetailInfo.batteryInfo_.leftEarChargeState_ = info.leftEarChargeState_;
        batteryDetailInfo.batteryInfo_.rightEarBatteryLevel_ = info.rightEarBatteryLevel_;
        batteryDetailInfo.batteryInfo_.rightEarChargeState_ = info.rightEarChargeState_;
        batteryDetailInfo.batteryInfo_.boxBatteryLevel_ = info.boxBatteryLevel_;
        batteryDetailInfo.batteryInfo_.boxChargeState_ = info.boxChargeState_;
        return;
    }

    // DTS2025022724268 开盒充电场景，AT电量上报时初始化左右耳电量值，避免左右耳电量不及时更新导致状态栏电量不会增加
    if (batteryDetailInfo.batteryInfo_.batteryLevel_ != INVALID_BATTERY_LEVEL &&
        info.batteryLevel_ > batteryDetailInfo.batteryInfo_.batteryLevel_) {
        batteryDetailInfo.batteryInfo_.leftEarBatteryLevel_ = info.leftEarBatteryLevel_;
        batteryDetailInfo.batteryInfo_.rightEarBatteryLevel_ = info.rightEarBatteryLevel_;
    }

    // 上报的电量是普通电量
    batteryDetailInfo.batteryInfo_.batteryLevel_ = info.batteryLevel_;
}

bool BluetoothDeviceBatteryManager::ParseBatteryInfo(const std::string &commandValue, BatteryInfo &info)
{
    std::vector<std::string> values = SplitValueByDelim(commandValue, ",");
    if (values.empty()) {
        return false;
    }
    uint8_t kvPairNum = 0;
    if (!ConvertStrToDigit(values[0], kvPairNum)) {
        return false;
    }
    HILOGE("kvPairNum: %{public}u, size: %{public}u", kvPairNum, values.size());
    if (kvPairNum * KV_PAIR_NUMBER + 1 != values.size()) {
        return false;
    }

    uint8_t key = 0;
    int32_t value = 0;
    for (uint32_t i = 1; i < values.size() - 1; i += KV_PAIR_NUMBER) {
        if (!ConvertStrToDigit(values[i], key)) {
            return false;
        }
        if (!ConvertStrToDigit(values[i + 1], value)) {
            return false;
        }
        HILOGD("key: %{public}u, value: %{public}d", key, value);
        if (!CheckBatteryInfoValid(key, value)) {
            return false;
        }

        SetBatteryInfo(key, value, info);
    }
    return true;
}

bool BluetoothDeviceBatteryManager::CheckBatteryInfoValid(uint8_t key, int32_t value)
{
    if (key == KEY_NORMAL_DEVICE_BATTERY_LEVEL || key == KEY_LEFT_EAR_BATTERY_LEVEL ||
        key == KEY_RIGHT_EAR_BATTERY_LEVEL || key == KEY_BOX_BATTERY_LEVEL) {
        if (value < 0 || value > MAX_BATTERY_LEVEL) {
            HILOGE("invalid param, key = %{public}u, value = %{public}u,", key, value);
            return false;
        }
    }
    if (key == KEY_LEFT_EAR_CHARGE_STATE || key == KEY_RIGHT_EAR_CHARGE_STATE || key == KEY_BOX_CHARGE_STATE) {
        if (value < ChargeState::NORMAL_CHARGE_NOT_CHARGING || value > ChargeState::SUPER_CHARGE_CHARGING) {
            HILOGE("invalid param, key = %{public}u, value = %{public}u,", key, value);
            return false;
        }
    }
    return true;
}

void BluetoothDeviceBatteryManager::SetBatteryInfo(uint8_t key, int32_t value, BatteryInfo &info)
{
    switch (key) {
        case KEY_NORMAL_DEVICE_BATTERY_LEVEL:
            info.batteryLevel_ = value;
            break;
        case KEY_LEFT_EAR_BATTERY_LEVEL:
            info.leftEarBatteryLevel_ = value;
            break;
        case KEY_LEFT_EAR_CHARGE_STATE:
            info.leftEarChargeState_ = value;
            break;
        case KEY_RIGHT_EAR_BATTERY_LEVEL:
            info.rightEarBatteryLevel_ = value;
            break;
        case KEY_RIGHT_EAR_CHARGE_STATE:
            info.rightEarChargeState_ = value;
            break;
        case KEY_BOX_BATTERY_LEVEL:
            info.boxBatteryLevel_ = value;
            break;
        case KEY_BOX_CHARGE_STATE:
            info.boxChargeState_ = value;
            break;
        default:
            HILOGE("unknow param, key = %{public}u, value = %{public}u", key, value);
            break;
    }
}

void BluetoothDeviceBatteryManager::ProcessActiveDeviceChanged(const std::string &profileName, const std::string &addr)
{
    if (profileName == "HfpAgService") {
        hfpActiveDevice_ = addr;
    } else if (profileName == "A2dpSrcService") {
        a2dpActiveDevice_ = addr;
    } else {
        HILOGE("unknow profileName, profileName = %{public}s", profileName.c_str());
        return;
    }
    HILOGI("profileName: %{public}s, addr: %{public}s", profileName.c_str(), GET_ENCRYPT_STR_ADDR(addr));
    std::string newDeviceAddr = "";
    if (CheckAddressValid(addr)) {
        newDeviceAddr = addr;
        HILOGI("currentBatteryDevice = %{public}s", GET_ENCRYPT_STR_ADDR(currentBatteryDevice_));
    } else {
        newDeviceAddr = GetCurrentBatteryDevice(true);
    }

    NotifyForDeviceChanged(newDeviceAddr);
}

void BluetoothDeviceBatteryManager::ProcessHfpConnected(const std::string &addr)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    int32_t batteryLevel = GetDeviceReportBatteryLevel(addr);
    if (batteryLevel == INVALID_BATTERY_LEVEL) {
        HILOGE("batteryLevel is INVALID");
        return;
    }

    UpdateBatteryStatus(addr, true);
    if (addr != GetCurrentBatteryDevice(true)) {
        HILOGE("addr: %{public}s not CurrentBatteryDevice", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }
    NotifyForDeviceChanged(addr);
}

void BluetoothDeviceBatteryManager::ProcessHfpDisconnected(const std::string &addr)
{
    HILOGI("addr = %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    if (!CheckAddressValid(addr)) {
        HILOGE("addr is invalid");
        return;
    }
    if (addr == hfpActiveDevice_) {
        hfpActiveDevice_ = "";
    }
    BatteryDetailInfo info;
    if (!RemoteDeviceProperties::GetInstance()->GetDeviceBatteryInfo(addr, info)) {
        HILOGE("addr: %{public}s not exist", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }

    // 存在ECHO上报的信息
    if (info.batteryObtainType_ & BATTERY_OBTAIN_TYPE_ECHO) {
        info.batteryObtainType_ = BATTERY_OBTAIN_TYPE_ECHO;
        RemoteDeviceProperties::GetInstance()->SetDeviceBatteryInfo(addr, info);
    } else {
        // 不存在ECHO上报的信息，即只有AT上报
        RemoteDeviceProperties::GetInstance()->ClearDeviceBatteryInfo(addr);
    }
    UpdateBatteryStatus(addr, false);
    std::string newDeviceAddr = GetCurrentBatteryDevice(true);
    NotifyForDeviceChanged(newDeviceAddr);
}

void BluetoothDeviceBatteryManager::ProcessAclStateChanged(const std::string &addr, int32_t connectState)
{
    if (connectState == CONNECTION_STATE_CONNECTED) {
        ProcessAclConnected(addr);
        return;
    } else if (connectState == CONNECTION_STATE_DISCONNECTED) {
        ProcessAclDisconnected(addr);
    } else {
        HILOGI("no need process");
        return;
    }

    std::string newDeviceAddr = GetCurrentBatteryDevice(true);
    NotifyForDeviceChanged(newDeviceAddr);
}

void BluetoothDeviceBatteryManager::ProcessAclConnected(const std::string &addr)
{
    AddConnectedDevice(addr);
    HILOGI("device: %{public}s, map size: %{public}d",
        GET_ENCRYPT_STR_ADDR(addr), connectedDevices_.Size());
}

void BluetoothDeviceBatteryManager::ProcessAclDisconnected(const std::string &addr)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    if (!CheckAddressValid(addr)) {
        HILOGE("addr is invalid");
        return;
    }
    RemoteDeviceProperties::GetInstance()->ClearDeviceBatteryInfo(addr);
    std::pair<std::string, bool> connectedDevice = FindConnectedDevice(addr);
    connectedDevices_.Erase(connectedDevice);

    if (addr == hfpActiveDevice_) {
        hfpActiveDevice_ = "";
    }
    if (addr == a2dpActiveDevice_) {
        a2dpActiveDevice_ = "";
    }
}

std::string BluetoothDeviceBatteryManager::GetCurrentBatteryDevice(bool isBatteryDevice)
{
    std::string newCurrentBatteryDevice = "";
    bool hfpActiveDeviceSupportBattery = FindConnectedDevice(hfpActiveDevice_).second;
    bool a2dpActiveDeviceSupportBattery = FindConnectedDevice(a2dpActiveDevice_).second;

    if (CheckAddressValid(hfpActiveDevice_) && hfpActiveDeviceSupportBattery) {
        newCurrentBatteryDevice = hfpActiveDevice_;
    } else if (CheckAddressValid(a2dpActiveDevice_) && a2dpActiveDeviceSupportBattery) {
        newCurrentBatteryDevice = a2dpActiveDevice_;
    } else {
        newCurrentBatteryDevice = GetLastConnectedDevice(isBatteryDevice);
    }
    HILOGI("newCurrentBatteryDevice: %{public}s", GET_ENCRYPT_STR_ADDR(newCurrentBatteryDevice));
    return newCurrentBatteryDevice;
}

void BluetoothDeviceBatteryManager::LogBatteryInfo(const BatteryInfo &info)
{
    HILOGI("batteryLevel = %{public}d, leftEarBatteryLevel: %{public}d,\
        leftEarChargeState = %{public}d, rightEarBatteryLevel: %{public}d, rightEarChargeState = %{public}d, \
        boxBatteryLevel_ = %{public}d, boxChargeState_: %{public}d",
        info.batteryLevel_, info.leftEarBatteryLevel_, info.leftEarChargeState_,
        info.rightEarBatteryLevel_, info.rightEarChargeState_, info.boxBatteryLevel_,
        info.boxChargeState_);
}

void BluetoothDeviceBatteryManager::NotifyForBatteryInfoChanged(const std::string &addr, int32_t batteryLevel)
{
    if (addr != currentBatteryDevice_) {
        HILOGI("addr: %{public}s, currentBatteryDevice_: %{public}s",
            GET_ENCRYPT_STR_ADDR(addr), GET_ENCRYPT_STR_ADDR(currentBatteryDevice_));
        return;
    }
    if (reportBatteryLevel_ == batteryLevel) {
        HILOGE("addr = %{public}s, batteryLevel = %{public}d not change", GET_ENCRYPT_STR_ADDR(addr), batteryLevel);
        return;
    }
    HILOGI("addr: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_STR_ADDR(addr), batteryLevel);
    ReportDeviceBatteryInfo(addr, batteryLevel);
}

void BluetoothDeviceBatteryManager::NotifyForDeviceChanged(const std::string &newBatteryDevice)
{
    if (!newBatteryDevice.empty() && !FindConnectedDevice(newBatteryDevice).second) {
        HILOGE("device not battery device");
        return;
    }
    int32_t batteryLevel = GetDeviceReportBatteryLevel(newBatteryDevice);
    if (newBatteryDevice == currentBatteryDevice_ && batteryLevel != INVALID_BATTERY_LEVEL) {
        HILOGI("currentBatteryDevice no change, currentBatteryDevice: %{public}s",
            GET_ENCRYPT_STR_ADDR(currentBatteryDevice_));
        return;
    }
    HILOGI("newBatteryDevice: %{public}s, currentBatteryDevice_: %{public}s",
        GET_ENCRYPT_STR_ADDR(newBatteryDevice), GET_ENCRYPT_STR_ADDR(currentBatteryDevice_));
    currentBatteryDevice_ = newBatteryDevice;
    if (reportBatteryLevel_ == INVALID_BATTERY_LEVEL && batteryLevel == INVALID_BATTERY_LEVEL) {
        HILOGI("newBatteryDevice: %{public}s batteryLevel is -1", GET_ENCRYPT_STR_ADDR(newBatteryDevice));
        return;
    }
    ReportDeviceBatteryInfo(newBatteryDevice, batteryLevel);
}

void BluetoothDeviceBatteryManager::ReportDeviceBatteryInfo(const std::string &addr, int32_t batteryLevel)
{
    HILOGI("addr: %{public}s, batteryLevel: %{public}d", GET_ENCRYPT_STR_ADDR(addr), batteryLevel);
    reportBatteryLevel_ = batteryLevel;
    DoInAdapterManagerThread([addr, batteryLevel]() {
        BluetoothHelper::BluetoothCommonEventHelper::PublishDeviceBatteryLevelEvent(addr, batteryLevel);
    });
}

int32_t BluetoothDeviceBatteryManager::GetDeviceReportBatteryLevel(const std::string &addr)
{
    if (!CheckAddressValid(addr)) {
        HILOGI("currentBatteryDevice is invalid");
        return INVALID_BATTERY_LEVEL;
    }
    BatteryDetailInfo info;
    if (!RemoteDeviceProperties::GetInstance()->GetDeviceBatteryInfo(addr, info)) {
        HILOGE("addr: %{public}s not exist", GET_ENCRYPT_STR_ADDR(addr));
        return INVALID_BATTERY_LEVEL;
    }

    return CalcuBatteryLevel(info);
}

int32_t BluetoothDeviceBatteryManager::CalcuBatteryLevel(const BatteryDetailInfo &info)
{
    int32_t batteryLevel = MAX_BATTERY_LEVEL + 1;
    if (info.batteryInfo_.leftEarBatteryLevel_ != INVALID_BATTERY_LEVEL &&
        info.batteryInfo_.leftEarBatteryLevel_ < batteryLevel) {
        batteryLevel = info.batteryInfo_.leftEarBatteryLevel_;
    }

    if (info.batteryInfo_.rightEarBatteryLevel_ != INVALID_BATTERY_LEVEL &&
        info.batteryInfo_.rightEarBatteryLevel_ < batteryLevel) {
        batteryLevel = info.batteryInfo_.rightEarBatteryLevel_;
    }

    if (info.batteryInfo_.batteryLevel_ != INVALID_BATTERY_LEVEL &&
        info.batteryInfo_.batteryLevel_ < batteryLevel) {
        batteryLevel = info.batteryInfo_.batteryLevel_;
    }

    return (batteryLevel == MAX_BATTERY_LEVEL + 1) ? INVALID_BATTERY_LEVEL : batteryLevel;
}

void BluetoothDeviceBatteryManager::SetDeviceSupportBattery(const std::string &addr)
{
    if (!CheckAddressValid(addr)) {
        HILOGE("device not set");
        return;
    }
    if (!IsHfpConnected(addr) || FindConnectedDevice(addr).second) {
        HILOGE("failed!");
        return;
    }
    HILOGI("device: %{public}s supports battery ", GET_ENCRYPT_STR_ADDR(addr));
    UpdateBatteryStatus(addr, true);
    currentBatteryDevice_ = GetCurrentBatteryDevice(true);
}

std::string BluetoothDeviceBatteryManager::GetLastConnectedDevice(bool isBatteryDevice)
{
    std::string addr = "";
    if (connectedDevices_.Empty()) {
        HILOGI("No connected device exists.");
        return addr;
    }
    std::vector<std::pair<std::string, bool>> devices = connectedDevices_.GetVector();
    int vectorSize = static_cast<int>(devices.size());
    for (int idx = vectorSize - 1; idx >= 0; idx--) {
        if (devices[idx].second == isBatteryDevice) {
            addr = devices[idx].first;
            break;
        }
    }
    return addr;
}

void BluetoothDeviceBatteryManager::AddConnectedDevice(std::string addr)
{
    // 该函数只适用于第一次添加设备，如果该设备已经存在于设备列表里，将会删除后重新添加，这样会改变原有的连接顺序
    // 请谨慎使用，如果只是需要更新设备电量状态，可以使用UpdateBatteryStatus
    std::pair<std::string, bool> connectDevice = FindConnectedDevice(addr);
    if (CheckAddressValid(connectDevice.first)) {
        HILOGI("device: %{public}s is exist, add device again", GET_ENCRYPT_STR_ADDR(connectDevice.first));
        connectedDevices_.Erase(connectDevice);
        connectedDevices_.PushNoDuplicate(std::make_pair(addr, false));
        return;
    }
    HILOGI("first add device! %{public}s", GET_ENCRYPT_STR_ADDR(connectDevice.first));
    connectedDevices_.PushNoDuplicate(std::make_pair(addr, false));
}

void BluetoothDeviceBatteryManager::UpdateBatteryStatus(std::string addr, bool batteryState)
{
    std::pair<std::string, bool> connectDevice = FindConnectedDevice(addr);
    if (!CheckAddressValid(connectDevice.first)) {
        HILOGE("device not exist!");
        return;
    }
    HILOGI("device supports battery status change");
    connectedDevices_.ForEach([batteryState, addr](std::pair<std::string, bool> &device) {
        if (device.first == addr) {
            device.second = batteryState;
            return;
        }
    });
}

std::pair<std::string, bool> BluetoothDeviceBatteryManager::FindConnectedDevice(const std::string &addr)
{
    std::pair<std::string, bool> connectedDevice = {};
    if (connectedDevices_.Empty()) {
        HILOGI("No connected battery device exists.");
        return connectedDevice;
    }
    HILOGI("size: %{public}d", connectedDevices_.Size());
    connectedDevices_.ForEach([&connectedDevice, addr](std::pair<std::string, bool> &device) {
        if (device.first == addr) {
            connectedDevice = device;
            return;
        }
    });
    return connectedDevice;
}
} // namespace bluetooth
} // namespace OHOS