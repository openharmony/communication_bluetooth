/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BLUETOOTH_CONNECTION_IMPL_H
#define BLUETOOTH_CONNECTION_IMPL_H

#include "bluetooth_connection_ffi.h"
#include "cj_common_ffi.h"
#include "bluetooth_host.h"
#include "native/ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {
using Bluetooth::BluetoothHostObserver;
using Bluetooth::BluetoothRemoteDevice;
using Bluetooth::BluetoothRemoteDeviceObserver;
using Bluetooth::ParcelUuid;
using Bluetooth::BluetoothDeviceClass;
using Bluetooth::DeviceBatteryInfo;

class ConnectionImpl {
public:
    ConnectionImpl() = default;
    ~ConnectionImpl() = default;

    static void PairDevice(std::string deviceId, int32_t* errCode);
    static char* GetRemoteDeviceName(std::string deviceId, int32_t* errCode);
    static DeviceClass GetRemoteDeviceClass(std::string deviceId, int32_t* errCode);
    static CArrString GetRemoteProfileUuids(std::string deviceId, int32_t* errCode);
    static char* GetLocalName(int32_t* errCode);
    static CArrString GetPairedDevices(int32_t* errCode);
    static int32_t GetPairState(std::string deviceId, int32_t* errCode);
    static int32_t GetProfileConnectionState(int32_t profileId, int32_t* errCode);
    static void SetDevicePairingConfirmation(std::string deviceId, bool accept, int32_t* errCode);
    static void SetDevicePinCode(std::string deviceId, std::string code, int32_t* errCode);
    static void SetLocalName(std::string localName, int32_t* errCode);
    static void SetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode);
    static int32_t GetBluetoothScanMode(int32_t* errCode);
    static void StartBluetoothDiscovery(int32_t* errCode);
    static void StoptBluetoothDiscovery(int32_t* errCode);
    static bool IsBluetoothDiscovering(int32_t* errCode);
    static void SetRemoteDeviceName(std::string deviceId, std::string name, int32_t* errCode);
    static CBatteryInfo GetRemoteDeviceBatteryInfo(std::string deviceId, int32_t* errCode);
    static void RegisterConnectionObserver(int32_t callbackType, void (*callback)(), int32_t* errCode);
};

class CjBluetoothConnectionObserver : public BluetoothHostObserver {
public:
    CjBluetoothConnectionObserver();
    ~CjBluetoothConnectionObserver() override = default;

    void OnStateChanged(const int transport, const int status) override{};
    void OnDiscoveryStateChanged(int status) override{};
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override;
    void OnPairRequested(const BluetoothRemoteDevice &device) override{};
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override;
    void OnScanModeChanged(int mode) override{};
    void OnDeviceNameChanged(const std::string &deviceName) override{};
    void OnDeviceAddrChanged(const std::string &address) override{};

    void RegisterDeviceFindFunc(std::function<void(CArrString)> cjCallback);
    void RegisterPinRequestFunc(std::function<void(CPinRequiredParam)> cjCallback);

private:
    std::function<void(CArrString)> deviceFindFunc{nullptr};
    std::function<void(CPinRequiredParam)> pinRequestFunc{nullptr};
};

class CjBluetoothRemoteDeviceObserver : public BluetoothRemoteDeviceObserver {
public:
    CjBluetoothRemoteDeviceObserver();
    ~CjBluetoothRemoteDeviceObserver() override = default;

    void OnAclStateChanged(const BluetoothRemoteDevice &device, int state, unsigned int reason) override{};
    void OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause) override;
    void OnRemoteUuidChanged(const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids) override{};
    void OnRemoteNameChanged(const BluetoothRemoteDevice &device, const std::string &deviceName) override{};
    void OnRemoteAliasChanged(const BluetoothRemoteDevice &device, const std::string &alias) override{};
    void OnRemoteCodChanged(const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod) override{};
    void OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device, int batteryLevel) override{};
    void OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device, int rssi, int status) override{};
    void OnRemoteBatteryChanged(const BluetoothRemoteDevice &device, const DeviceBatteryInfo &batteryInfo) override;

    void RegisterBondStateFunc(std::function<void(CBondStateParam)> cjCallback);
    void RegisterBatteryChangeFunc(std::function<void(CBatteryInfo)> cjCallback);

private:
    std::function<void(CBondStateParam)> bondStateFunc{nullptr};
    std::function<void(CBatteryInfo)> batteryChangeFunc{nullptr};
};
} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_CONNECTION_IMPL_H