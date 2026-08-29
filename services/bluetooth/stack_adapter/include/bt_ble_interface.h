/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer BLE scanner and advertiser glue (bt_ble_interface.h).
 * The scanner/advertiser interfaces mirror bluedroid
 * system/include/hardware/ble_scanner.h and ble_advertiser.h with
 * std::function callbacks; SetScanParameters is the 5-arg vendor variant
 * consumed by the service layer. The real stack is dlopened at runtime and
 * the instances stay null in the stub world.
 */

#ifndef BT_BLE_INTERFACE_H
#define BT_BLE_INTERFACE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "bt_types.h"
#include "bt_gatt.h"
#include "bt_common.h"

class BleScannerInterface {
public:
    virtual ~BleScannerInterface() = default;
    virtual void Scan(bool start) = 0;
    virtual void ScanFilterEnable(bool enable,
        std::function<void(uint8_t action, uint8_t btmStatus)> cb) = 0;
    virtual void ScanFilterParamSetup(int clientIf, uint8_t action, int filtIndex,
        std::unique_ptr<BtgattFiltParamSetup> filtParam,
        std::function<void(uint8_t avblSpace, uint8_t actionType, uint8_t btmStatus)> cb) = 0;
    virtual void ScanFilterAdd(int filterIndex, std::vector<ApcfCommand> filters,
        std::function<void(uint8_t filtType, uint8_t avblSpace, uint8_t action,
            uint8_t btmStatus)> cb) = 0;
    virtual void BatchscanEnable(int scanMode, int scanInterval, int scanWindow, int addrType,
        int discardRule, std::function<void(uint8_t btmStatus)> cb) = 0;
    virtual void BatchscanDisable(std::function<void(uint8_t btmStatus)> cb) = 0;
    virtual void BatchscanConfigStorage(int clientIf, int batchScanFullMax,
        int batchScanTruncMax, int batchScanNotifyThreshold,
        std::function<void(uint8_t btmStatus)> cb) = 0;
    virtual void BatchscanReadReports(int clientIf, int scanMode) = 0;
    virtual void SetScanParameters(int scanInterval, int scanWindow, bool legacy, int scanPhy,
        std::function<void(uint8_t btmStatus)> cb) = 0;
};

/* Advertising parameters consumed by the ble advertiser service; the layout
 * mirrors the removed stack layer HAL (hardware/ble_advertiser.h) with the
 * own-address fields the service layer fills in. */
struct AdvertiseParameters {
    uint16_t advertisingEventProperties = 0;
    uint32_t minInterval = 0;
    uint32_t maxInterval = 0;
    uint8_t channelMap = 0;
    int8_t txPower = 0;
    uint8_t primaryAdvertisingPhy = 0;
    uint8_t secondaryAdvertisingPhy = 0;
    uint8_t scanRequestNotificationEnable = 0;
    uint8_t ownAddr[6] = { 0 };
    uint8_t ownAddrType = 0;
};

/* Timeout (seconds) of the create-advertising future used by the service.
 * Guarded to stay consistent with config/bt_config.h, which may be included
 * first by the same translation unit. */
#ifndef BLUETOOTH_BLE_CREATE_ADV_TIMEOUT
#define BLUETOOTH_BLE_CREATE_ADV_TIMEOUT 2 // 2s
#endif  // BLUETOOTH_BLE_CREATE_ADV_TIMEOUT

class BleAdvertiserInterface {
public:
    virtual ~BleAdvertiserInterface() = default;
    virtual void RegisterAdvertiser(
        std::function<void(uint8_t advertiserId, uint8_t status)> cb) = 0;
    virtual void Unregister(uint8_t advertiserId) = 0;
    virtual void SetData(int advertiserId, bool setScanRsp, std::vector<uint8_t> data,
        std::function<void(uint8_t status)> cb) = 0;
    virtual void SetParameters(uint8_t advertiserId, AdvertiseParameters params,
        std::function<void(uint8_t status, int8_t txPower)> cb) = 0;
    virtual void StartAdvertising(uint8_t advertiserId,
        std::function<void(uint8_t status)> cb, AdvertiseParameters params,
        std::vector<uint8_t> advertiseData, std::vector<uint8_t> scanResponseData,
        int timeoutS, std::function<void(uint8_t status)> timeoutCb) = 0;
    virtual void Enable(uint8_t advertiserId, bool enable,
        std::function<void(uint8_t status)> cb, uint16_t duration, uint8_t maxExtAdvEvents,
        std::function<void(uint8_t status)> timeoutCb) = 0;
};

inline BleScannerInterface *GetBleScannerInstance()
{
    return nullptr;
}

inline BleAdvertiserInterface *GetBleAdvertiserInstance()
{
    return nullptr;
}

#endif  // BT_BLE_INTERFACE_H
