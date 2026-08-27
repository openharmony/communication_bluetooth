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
 * Stub of the removed stack layer btif gatt glue (btif_gatt.h).
 * The scanner/advertiser interfaces accept any argument list so that every
 * service layer call site compiles; the real stack is dlopened at runtime
 * and the instances stay null in the stub world.
 */

#ifndef BTIF_GATT_H
#define BTIF_GATT_H

#include "bt_types.h"
#include "bt_gatt.h"
#include "btif_common.h"

class BleScannerInterface {
public:
    virtual ~BleScannerInterface() = default;
    template <typename... Args>
    void Scan(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterEnable(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterParamSetup(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterAdd(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanEnable(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanDisable(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanConfigStorage(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanReadReports(Args &&...args)
    {
    }
    template <typename... Args>
    void SetScanParameters(Args &&...args)
    {
    }
};

/* Advertising parameters consumed by the ble advertiser service; the layout
 * mirrors the removed stack layer HAL (hardware/ble_advertiser.h) with the
 * own-address fields the service layer fills in. */
struct AdvertiseParameters {
    uint16_t advertising_event_properties = 0;
    uint32_t min_interval = 0;
    uint32_t max_interval = 0;
    uint8_t channel_map = 0;
    int8_t tx_power = 0;
    uint8_t primary_advertising_phy = 0;
    uint8_t secondary_advertising_phy = 0;
    uint8_t scan_request_notification_enable = 0;
    uint8_t own_addr[6] = { 0 };
    uint8_t own_addr_type = 0;
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
    template <typename... Args>
    void RegisterAdvertiser(Args &&...args)
    {
    }
    template <typename... Args>
    void Unregister(Args &&...args)
    {
    }
    template <typename... Args>
    void SetData(Args &&...args)
    {
    }
    template <typename... Args>
    void SetParameters(Args &&...args)
    {
    }
    template <typename... Args>
    void StartAdvertising(Args &&...args)
    {
    }
    template <typename... Args>
    void Enable(Args &&...args)
    {
    }
};

inline BleScannerInterface *get_ble_scanner_instance()
{
    return nullptr;
}

inline BleAdvertiserInterface *get_ble_advertiser_instance()
{
    return nullptr;
}

#endif  // BTIF_GATT_H
