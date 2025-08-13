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

#ifndef TAIHE_BLUETOOTH_CONNECTION_CALLBACK_H
#define TAIHE_BLUETOOTH_CONNECTION_CALLBACK_H

#include <shared_mutex>
#include "bluetooth_host.h"
#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {
struct PairConfirmedCallBackInfo {
    int number;
    int pinType;
    std::string deviceAddr;

    PairConfirmedCallBackInfo(int number, int pinType, std::string deviceAddr)
    {
        this->number = number;
        this->pinType = pinType;
        this->deviceAddr = deviceAddr;
    }
};

enum class BondState { BOND_STATE_INVALID = 0, BOND_STATE_BONDING = 1, BOND_STATE_BONDED = 2 };

class TaiheBluetoothConnectionInit {
public:
    static void RegisterConnectionObserverToHost();
};

class TaiheBluetoothRemoteDeviceObserver : public BluetoothRemoteDeviceObserver {
public:
    TaiheBluetoothRemoteDeviceObserver();
    ~TaiheBluetoothRemoteDeviceObserver() override = default;

    static std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> GetInstance(void);

    void OnAclStateChanged(const BluetoothRemoteDevice &device, int state, unsigned int reason) override;
    void OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause) override;
    void OnRemoteUuidChanged(const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids) override;
    void OnRemoteNameChanged(const BluetoothRemoteDevice &device, const std::string &deviceName) override;
    void OnRemoteAliasChanged(const BluetoothRemoteDevice &device, const std::string &alias) override;
    void OnRemoteCodChanged(const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod) override;
    void OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device, int batteryLevel) override;
    void OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device, int rssi, int status) override;
    void OnRemoteBatteryChanged(const BluetoothRemoteDevice &device, const DeviceBatteryInfo &batteryInfo) override;

    std::vector<::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BondStateParam const& data)>>> bondStateChangeVec = {};
    std::shared_mutex bondStateChangeLock;

    std::vector<::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BatteryInfo const& data)>>> remoteBatteryChangeVec = {};
    std::shared_mutex remoteBatteryChangeLock;
};

class TaiheBluetoothConnectionObserver : public BluetoothHostObserver {
public:
    TaiheBluetoothConnectionObserver();
    ~TaiheBluetoothConnectionObserver() override = default;

    static std::shared_ptr<TaiheBluetoothConnectionObserver> GetInstance(void);

    void OnStateChanged(const int transport, const int status) override{};
    void OnDiscoveryStateChanged(int status) override;
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override;
    void OnPairRequested(const BluetoothRemoteDevice &device) override;
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override;
    void OnScanModeChanged(int mode) override;
    void OnDeviceNameChanged(const std::string &deviceName) override;
    void OnDeviceAddrChanged(const std::string &address) override;

    std::vector<::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>>> discoveryResultVec = {};
    std::shared_mutex discoveryResultLock;

    std::vector<::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::taihe::string> data)>>> bluetoothDeviceFindVec = {};
    std::shared_mutex bluetoothDeviceFindLock;

    std::vector<::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::PinRequiredParam const& data)>>> pinRequiredVec = {};
    std::shared_mutex pinRequiredLock;

private:
    void OnPairConfirmedCallBack(const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo);
    void OnDiscoveryResultCallBack(const BluetoothRemoteDevice &device);
    void OnDiscoveryResultCallBack(
        const BluetoothRemoteDevice &device, int rssi, const std::string &deviceName, int deviceClass);
    static std::string GetFormatPinCode(uint32_t pinType, uint32_t pinCode);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_CONNECTION_CALLBACK_H