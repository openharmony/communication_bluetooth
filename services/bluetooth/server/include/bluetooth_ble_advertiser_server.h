/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_SERVER_H

#include "bluetooth_ble_advertiser_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_ble_advertiser.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleAdvertiserServer : public BluetoothBleAdvertiserStub {
public:
    static constexpr int DEFAULT_MIN_ADV_INTERVAL = 160;  // it means 100ms
    BluetoothBleAdvertiserServer();
    ~BluetoothBleAdvertiserServer() override;

    void RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback) override;
    void DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback) override;
    int StartAdvertising(const BluetoothBleAdvertiserSettings &settings,
        const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse,
        int32_t advHandle, uint16_t duration, bool isRawData) override;
    int EnableAdvertising(uint8_t advHandle, uint16_t duration) override;
    int DisableAdvertising(uint8_t advHandle) override;
    int StopAdvertising(int32_t advHandle) override;
    void Close(int32_t advHandle) override;
    int32_t GetAdvertiserHandle(int32_t &advHandle, const sptr<IBluetoothBleAdvertiseCallback> &callback) override;
    void SetAdvertisingData(const BluetoothBleAdvertiserData &advData,
        const BluetoothBleAdvertiserData &scanResponse, int32_t advHandle,
        bluetooth::SetAdvDataType type = bluetooth::SET_ADV_DATA_BOTH) override;
    int ChangeAdvertisingParams(uint8_t advHandle, const BluetoothBleAdvertiserSettings &settings) override;
    int GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType, int8_t &advpower) override;
    int BleRestoreRangingAntSwitch(bluetooth::BleAppType appType) override;

private:
    void AdjustTxPowerForWalletService(const std::string& callingName, const uint32_t& tokenId,
        bluetooth::BleAdvertiserSettingsImpl &settings);

    static constexpr int propertyValueMax = 128;
    const int8_t minTxPower = -127;
    const int8_t maxTxPower = 1;
    const char *connectivityBtChipType = "ohos.boot.odm.conn.schiptype";
    const char *chipType07c = "mp17c";
    const char *chipType1112 = "mp12";
    const int8_t advTxPowerAdjust = 6;

    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleAdvertiserServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_SERVER_H