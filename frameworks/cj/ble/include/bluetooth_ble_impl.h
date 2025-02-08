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
#ifndef BLUETOOTH_BLE_IMPL_H
#define BLUETOOTH_BLE_IMPL_H

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_clientDevice.h"
#include "bluetooth_ble_ffi.h"
#include "bluetooth_ble_gattServer.h"
#include "bluetooth_host.h"
#include "napi_bluetooth_utils.h"

#include <cstdint>

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BleCentralManagerCallback;
using Bluetooth::BleScanResult;
using Bluetooth::ScanResult;
using Bluetooth::UUID;

using Bluetooth::BleAdvertiseCallback;
using Bluetooth::BleAdvertiserData;
using Bluetooth::BleAdvertiserSettings;

class FfiBluetoothBleCentralManagerCallback : public BleCentralManagerCallback {
public:
    FfiBluetoothBleCentralManagerCallback();
    ~FfiBluetoothBleCentralManagerCallback() override = default;

    static FfiBluetoothBleCentralManagerCallback &GetInstance(void);

    void OnScanCallback(const BleScanResult &result) override;
    void OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType) override{};
    void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) override{};
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override{};
    void OnNotifyMsgReportFromLpDevice(const UUID &uuid, int msgType, const std::vector<uint8_t> &value) override{};

    void RegisterBLEDeviceFindFunc(std::function<void(CArrScanResult)> cjCallback);

private:
    std::function<void(CArrScanResult)> bleDeviceFindFunc{nullptr};
};

class FfiBluetoothBleAdvertiseCallback : public BleAdvertiseCallback {
public:
    FfiBluetoothBleAdvertiseCallback();
    ~FfiBluetoothBleAdvertiseCallback() override = default;

    static std::shared_ptr<FfiBluetoothBleAdvertiseCallback> GetInstance(void);

    void OnStartResultEvent(int result, int advHandle) override;
    void OnEnableResultEvent(int result, int advHandle) override;
    void OnDisableResultEvent(int result, int advHandle) override;
    void OnStopResultEvent(int result, int advHandle) override;
    void OnSetAdvDataEvent(int result) override{};
    void OnGetAdvHandleEvent(int result, int advHandle) override;
    void OnChangeAdvResultEvent(int result, int advHandle) override;

    void RegisterAdvertisingStateChangeFunc(std::function<void(CAdvertisingStateChangeInfo)> cjCallback);
    int32_t GetAdvHandleEvent();

private:
    int32_t handleEvent{-1};
    std::function<void(CAdvertisingStateChangeInfo)> advertisingStateChangeFunc{nullptr};
};

class BleImpl {
public:
    BleImpl() = default;
    ~BleImpl() = default;

    static int32_t CreateGattServer(FfiGattServer *&ffiGattServer);
    static int32_t CreateGattClientDevice(std::string deviceId, FfiClientDevice *&ffiClientDevice);
    static int32_t GetConnectedBleDevices(CArrString &res);
    static int32_t StartBleScan(CArrNativeScanFilter filters, NativeScanOptions *options);
    static int32_t StopBleScan();
    static int32_t StartAdvertising(NativeAdvertiseSetting setting, NativeAdvertiseData advData,
                                    NativeAdvertiseData *advResponse);
    static int32_t StopAdvertising();
    static int32_t StartAdvertising(NativeAdvertisingParams advertisingParams, int32_t &id);
    static int32_t EnableAdvertising(NativeAdvertisingEnableParams advertisingEnableParams);
    static int32_t DisableAdvertising(NativeAdvertisingDisableParams advertisingDisableParams);
    static int32_t StopAdvertising(uint32_t advertisingId);
    static int32_t RegisterBleObserver(int32_t callbackType, void (*callback)());
};
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_BLE_IMPL_H