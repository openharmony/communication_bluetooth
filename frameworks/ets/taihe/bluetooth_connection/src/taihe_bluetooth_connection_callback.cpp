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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_connection_observer"
#endif

#include <string>
#include <memory>
#include <map>
#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "taihe_bluetooth_connection_callback.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

void TaiheBluetoothConnectionInit::RegisterConnectionObserverToHost()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    host->RegisterObserver(TaiheBluetoothConnectionObserver::GetInstance());
    host->RegisterRemoteDeviceObserver(TaiheBluetoothRemoteDeviceObserver::GetInstance());
}

TaiheBluetoothRemoteDeviceObserver::TaiheBluetoothRemoteDeviceObserver()
{}

std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> TaiheBluetoothRemoteDeviceObserver::GetInstance(void)
{
    static std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> instance =
        std::make_shared<TaiheBluetoothRemoteDeviceObserver>();
    return instance;
}

void TaiheBluetoothRemoteDeviceObserver::OnAclStateChanged(
    const BluetoothRemoteDevice &device, int state, unsigned int reason)
{}

void TaiheBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause)
{
    std::shared_lock<std::shared_mutex> guard(bondStateChangeLock);
    int bondStatus = 0;
    HILOGD("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = static_cast<int>(::OHOS::Bluetooth::BondState::BOND_STATE_INVALID);
            break;
        case PAIR_PAIRING:
            bondStatus = static_cast<int>(::OHOS::Bluetooth::BondState::BOND_STATE_BONDING);
            break;
        case PAIR_PAIRED:
            bondStatus = static_cast<int>(::OHOS::Bluetooth::BondState::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
    HILOGI("addr:%{public}s, bondStatus:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), bondStatus, cause);

    auto deviceId_ = static_cast<::taihe::string>(device.GetDeviceAddr());
    auto state_ = ::ohos::bluetooth::connection::BondState::from_value(bondStatus);
    auto cause_ = ::ohos::bluetooth::connection::UnbondCause::from_value(cause);
    auto taiheResult = ::ohos::bluetooth::connection::BondStateParam{
        .deviceId = deviceId_,
        .state = state_,
        .cause = cause_,
    };
    for (auto callback : bondStateChangeVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteUuidChanged(
    const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids)
{
    HILOGD("called");
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteNameChanged(
    const BluetoothRemoteDevice &device, const std::string &deviceName)
{
    HILOGD("addr:%{public}s, deviceName:%{private}s", GET_ENCRYPT_ADDR(device), deviceName.c_str());
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteAliasChanged(
    const BluetoothRemoteDevice &device, const std::string &alias)
{
    HILOGD("addr:%{public}s, alias:%{private}s", GET_ENCRYPT_ADDR(device), alias.c_str());
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteCodChanged(
    const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod)
{
    HILOGD("addr:%{public}s, cod:%{public}d", GET_ENCRYPT_ADDR(device), cod.GetClassOfDevice());
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteBatteryLevelChanged(
    const BluetoothRemoteDevice &device, int batteryLevel)
{
    HILOGD("addr:%{public}s, batteryLevel:%{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
}

void TaiheBluetoothRemoteDeviceObserver ::OnRemoteBatteryChanged(
    const BluetoothRemoteDevice &device, const DeviceBatteryInfo &batteryInfo)
{
    std::shared_lock<std::shared_mutex> guard(remoteBatteryChangeLock);
    auto deviceId_ = static_cast<::taihe::string_view>(batteryInfo.deviceId_);
    auto batteryLevel_ = batteryInfo.batteryLevel_;
    auto leftEarBatteryLevel_ = batteryInfo.leftEarBatteryLevel_;
    auto leftEarChargeState_ = static_cast<::ohos::bluetooth::connection::DeviceChargeState::key_t>(
        batteryInfo.leftEarChargeState_);
    auto rightEarBatteryLevel_ = batteryInfo.rightEarBatteryLevel_;
    auto rightEarChargeState_ = static_cast<::ohos::bluetooth::connection::DeviceChargeState::key_t>(
        batteryInfo.rightEarChargeState_);
    auto boxBatteryLevel_ = batteryInfo.boxBatteryLevel_;
    auto boxChargeState_ = static_cast<::ohos::bluetooth::connection::DeviceChargeState::key_t>(
        batteryInfo.boxChargeState_);
    auto taiheResult = ::ohos::bluetooth::connection::BatteryInfo{
        .deviceId = deviceId_,
        .batteryLevel = batteryLevel_,
        .leftEarBatteryLevel = leftEarBatteryLevel_,
        .leftEarChargeState = leftEarChargeState_,
        .rightEarBatteryLevel = rightEarBatteryLevel_,
        .rightEarChargeState = rightEarChargeState_,
        .boxBatteryLevel = boxBatteryLevel_,
        .boxChargeState = boxChargeState_,
    };
    for (auto callback : remoteBatteryChangeVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

void TaiheBluetoothRemoteDeviceObserver ::OnReadRemoteRssiEvent(
    const BluetoothRemoteDevice &device, int rssi, int status)
{
    HILOGD("addr:%{public}s, rssi:%{public}d, status is %{public}d", GET_ENCRYPT_ADDR(device), rssi, status);
}

TaiheBluetoothConnectionObserver::TaiheBluetoothConnectionObserver()
{}

std::shared_ptr<TaiheBluetoothConnectionObserver> TaiheBluetoothConnectionObserver::GetInstance(void)
{
    static std::shared_ptr<TaiheBluetoothConnectionObserver> instance =
        std::make_shared<TaiheBluetoothConnectionObserver>();
    return instance;
}

void TaiheBluetoothConnectionObserver::OnDiscoveryStateChanged(int status)
{
    switch (status) {
        case DISCOVERY_STARTED:
            HILOGD("DISCOVERY_STARTED(1)");
            break;
        case DISCOVERYING:
            HILOGD("DISCOVERYING(2)");
            break;
        case DISCOVERY_STOPED:
            HILOGD("DISCOVERY_STOPED(3)");
            break;
        default:
            HILOGE("invaild status is %{public}d", status);
            break;
    }
}

void TaiheBluetoothConnectionObserver::OnDiscoveryResult(
    const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass)
{
    OnDiscoveryResultCallBack(device);
    OnDiscoveryResultCallBack(device, rssi, deviceName, deviceClass);
}

void TaiheBluetoothConnectionObserver::OnPairRequested(const BluetoothRemoteDevice &device)
{
    HILOGD("start");
    BluetoothRemoteDevice remoteDevice;
    if (device.GetTransportType() == BT_TRANSPORT_BREDR) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BREDR);
    } else if (device.GetTransportType() == BT_TRANSPORT_BLE) {
        remoteDevice = BluetoothHost::GetDefaultHost().GetRemoteDevice(device.GetDeviceAddr(), BT_TRANSPORT_BLE);
    }
    remoteDevice.PairRequestReply(true);
}

void TaiheBluetoothConnectionObserver::OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number)
{
    HILOGD("OnPairConfirmed");
    std::shared_ptr<PairConfirmedCallBackInfo> pairConfirmInfo =
        std::make_shared<PairConfirmedCallBackInfo>(number, reqType, device.GetDeviceAddr());
    OnPairConfirmedCallBack(pairConfirmInfo);
}

void TaiheBluetoothConnectionObserver::OnScanModeChanged(int mode)
{
    HILOGI("mode is %{public}d", mode);
}

void TaiheBluetoothConnectionObserver::OnDeviceNameChanged(const std::string &deviceName)
{
    HILOGI("name is %{private}s", deviceName.c_str());
}

void TaiheBluetoothConnectionObserver::OnDeviceAddrChanged(const std::string &address)
{
    HILOGI("address is %{public}s", GetEncryptAddr(address).c_str());
}

void TaiheBluetoothConnectionObserver::OnPairConfirmedCallBack(
    const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo)
{
    std::shared_lock<std::shared_mutex> guard(pinRequiredLock);
    CHECK_AND_RETURN_LOG(pairConfirmInfo, "pairConfirmInfo is nullptr");
    HILOGI("Addr: %{public}s", GetEncryptAddr(pairConfirmInfo->deviceAddr).c_str());

    auto deviceId_ = static_cast<::taihe::string>(pairConfirmInfo->deviceAddr);
    auto pinCode_ = static_cast<::taihe::string>(GetFormatPinCode(pairConfirmInfo->pinType, pairConfirmInfo->number));
    auto pinType_ = static_cast<::ohos::bluetooth::connection::PinType::key_t>(pairConfirmInfo->pinType);
    auto taiheResult = ::ohos::bluetooth::connection::PinRequiredParam{
        .deviceId = deviceId_,
        .pinCode = pinCode_,
        .pinType = pinType_,
    };
    for (auto callback : pinRequiredVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

std::string TaiheBluetoothConnectionObserver::GetFormatPinCode(uint32_t pinType, uint32_t pinCode)
{
    std::string pinCodeStr = std::to_string(pinCode);
    if (pinType != PIN_TYPE_CONFIRM_PASSKEY && pinType != PIN_TYPE_NOTIFY_PASSKEY) {
        return pinCodeStr;
    }
    const uint32_t FORMAT_PINCODE_LENGTH = 6;
    while (pinCodeStr.length() < FORMAT_PINCODE_LENGTH) {
        pinCodeStr = "0" + pinCodeStr;
    }
    return pinCodeStr;
}

void TaiheBluetoothConnectionObserver::OnDiscoveryResultCallBack(const BluetoothRemoteDevice &device)
{
    std::shared_lock<std::shared_mutex> guard(bluetoothDeviceFindLock);
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    ::taihe::string oneResult(remoteDevice->GetDeviceAddr());
    ::taihe::array<::taihe::string> taiheResult = {oneResult};
    for (auto callback : bluetoothDeviceFindVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

void TaiheBluetoothConnectionObserver::OnDiscoveryResultCallBack(
    const BluetoothRemoteDevice &device, int rssi, const std::string &deviceName, int deviceClass)
{
    std::shared_lock<std::shared_mutex> guard(discoveryResultLock);
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    CHECK_AND_RETURN_LOG(remoteDevice, "remoteDevice is nullptr");
    ::taihe::string deviceId_(remoteDevice->GetDeviceAddr());
    int rssi_ = rssi;
    ::taihe::string deviceName_(deviceName);
    BluetoothDeviceClass classOfDevice = BluetoothDeviceClass(deviceClass);
    auto majorClass_ = static_cast<::ohos::bluetooth::constant::MajorClass::key_t>(
        classOfDevice.GetMajorClass());
    auto majorMinorClass_ = static_cast<::ohos::bluetooth::constant::MajorMinorClass::key_t>(
        classOfDevice.GetMajorMinorClass());
    int classOfDevice_ = deviceClass;
    auto deviceClass_ = ::ohos::bluetooth::connection::DeviceClass{
        .majorClass = majorClass_,
        .majorMinorClass = majorMinorClass_,
        .classOfDevice = classOfDevice_,
    };
    auto oneDiscoveryResult = ::ohos::bluetooth::connection::DiscoveryResult{
        .deviceId = deviceId_,
        .rssi = rssi_,
        .deviceName = deviceName_,
        .deviceClass = deviceClass_,
    };
    ::taihe::array<::ohos::bluetooth::connection::DiscoveryResult> taiheResult = {oneDiscoveryResult};
    for (auto callback : discoveryResultVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}
} // namespace Bluetooth
} // namespace OHOS