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

#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"

using namespace taihe;
using namespace ohos::bluetooth::ble;

namespace {
// To be implemented.

class GattClientDeviceImpl {
public:
    void OnBLECharacteristicChange(callback_view<void(BLECharacteristic const&)> callback)
    {
    }

    void OffBLECharacteristicChange(optional_view<callback<void(BLECharacteristic const&)>> callback)
    {
    }

    void SetCharacteristicChangeNotificationSync(BLECharacteristic const& characteristic, bool enable)
    {
    }

    double GetRssiValueSync()
    {
        return 1;
    }

    void WriteCharacteristicValueSync(BLECharacteristic const& characteristic, GattWriteType writeType)
    {
    }

    void GetServicesSync()
    {
    }

    void WriteDescriptorValueSync(BLEDescriptor const& descriptor)
    {
    }

    void OnBLEMtuChange(callback_view<void(double)> callback)
    {
    }

    void OffBLEMtuChange(optional_view<callback<void(double)>> callback)
    {
    }

    void OnBLEConnectionStateChange(callback_view<void(BLEConnectionChangeState const&)> callback)
    {
    }

    void OffBLEConnectionStateChange(optional_view<callback<void(BLEConnectionChangeState const&)>> callback)
    {
    }

    void SetBLEMtuSize(double mtu)
    {
    }

    void Connect()
    {
    }

    void Disconnect()
    {
    }

    void Close()
    {
    }
};

class GattServerImpl {
public:
    GattServerImpl()
    {
        // Don't forget to implement the constructor.
    }

    void OnCharacteristicRead(callback_view<void(CharacteristicReadRequest const&)> callback)
    {
    }

    void OffCharacteristicRead(optional_view<callback<void(CharacteristicReadRequest const&)>> callback)
    {
    }

    void OnBLEMtuChange(callback_view<void(double)> callback)
    {
    }

    void OffBLEMtuChange(optional_view<callback<void(double)>> callback)
    {
    }

    void OnDescriptorRead(callback_view<void(DescriptorReadRequest const&)> callback)
    {
    }

    void OffDescriptorRead(optional_view<callback<void(DescriptorReadRequest const&)>> callback)
    {
    }

    void OnDescriptorWrite(callback_view<void(DescriptorWriteRequest const&)> callback)
    {
    }

    void OffDescriptorWrite(optional_view<callback<void(DescriptorWriteRequest const&)>> callback)
    {
    }

    void OnConnectionStateChange(callback_view<void(BLEConnectionChangeState const&)> callback)
    {
    }

    void OffConnectionStateChange(optional_view<callback<void(BLEConnectionChangeState const&)>> callback)
    {
    }

    void Close()
    {
    }
};

class BleScannerImpl {
public:
    BleScannerImpl()
    {
        // Don't forget to implement the constructor.
    }

    void OnBLEDeviceFind(callback_view<void(ScanReport const&)> callback)
    {
    }

    void OffBLEDeviceFind(optional_view<callback<void(ScanReport const&)>> callback)
    {
    }
};

void OnBLEDeviceFind(callback_view<void(ScanReport const&)> callback)
{
}

void OffBLEDeviceFind(optional_view<callback<void(ScanReport const&)>> callback)
{
}

void OnAdvertisingStateChange(callback_view<void(AdvertisingStateChangeInfo const&)> callback)
{
}

void OffAdvertisingStateChange(optional_view<callback<void(AdvertisingStateChangeInfo const&)>> callback)
{
}

void StopAdvertising()
{
}

void StopAdvertisingSync(double advertisingId)
{
}

void StopBLEScan()
{
}

GattClientDevice CreateGattClientDevice(string_view deviceId)
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<GattClientDeviceImpl, GattClientDevice>();
}

GattServer CreateGattServer()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<GattServerImpl, GattServer>();
}

BleScanner CreateBleScanner()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<BleScannerImpl, BleScanner>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnBLEDeviceFind(OnBLEDeviceFind);
TH_EXPORT_CPP_API_OffBLEDeviceFind(OffBLEDeviceFind);
TH_EXPORT_CPP_API_OnAdvertisingStateChange(OnAdvertisingStateChange);
TH_EXPORT_CPP_API_OffAdvertisingStateChange(OffAdvertisingStateChange);
TH_EXPORT_CPP_API_CreateGattClientDevice(CreateGattClientDevice);
TH_EXPORT_CPP_API_CreateGattServer(CreateGattServer);
TH_EXPORT_CPP_API_CreateBleScanner(CreateBleScanner);
TH_EXPORT_CPP_API_StopAdvertising(StopAdvertising);
TH_EXPORT_CPP_API_StopAdvertisingSync(StopAdvertisingSync);
TH_EXPORT_CPP_API_StopBLEScan(StopBLEScan);
// NOLINTEND

