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
 * The scanner/advertiser interfaces mirror bluedroid
 * system/include/hardware/ble_scanner.h and ble_advertiser.h with
 * std::function callbacks; SetScanParameters is the 5-arg vendor variant
 * consumed by the service layer. The real stack is dlopened at runtime and
 * the instances stay null in the stub world.
 */

#ifndef BTIF_GATT_H
#define BTIF_GATT_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "bt_types.h"
#include "bt_gatt.h"
#include "btif_common.h"

class BleScannerInterface {
public:
    virtual ~BleScannerInterface() = default;
    virtual void Scan(bool start) = 0;
    virtual void ScanFilterEnable(bool enable,
        std::function<void(uint8_t action, uint8_t btm_status)> cb) = 0;
    virtual void ScanFilterParamSetup(int client_if, uint8_t action, int filt_index,
        std::unique_ptr<btgatt_filt_param_setup_t> filt_param,
        std::function<void(uint8_t avbl_space, uint8_t action_type, uint8_t btm_status)> cb) = 0;
    virtual void ScanFilterAdd(int filter_index, std::vector<ApcfCommand> filters,
        std::function<void(uint8_t filt_type, uint8_t avbl_space, uint8_t action,
            uint8_t btm_status)> cb) = 0;
    virtual void BatchscanEnable(int scan_mode, int scan_interval, int scan_window, int addr_type,
        int discard_rule, std::function<void(uint8_t btm_status)> cb) = 0;
    virtual void BatchscanDisable(std::function<void(uint8_t btm_status)> cb) = 0;
    virtual void BatchscanConfigStorage(int client_if, int batch_scan_full_max,
        int batch_scan_trunc_max, int batch_scan_notify_threshold,
        std::function<void(uint8_t btm_status)> cb) = 0;
    virtual void BatchscanReadReports(int client_if, int scan_mode) = 0;
    virtual void SetScanParameters(int scan_interval, int scan_window, bool legacy, int scan_phy,
        std::function<void(uint8_t btm_status)> cb) = 0;
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
    virtual void RegisterAdvertiser(
        std::function<void(uint8_t advertiser_id, uint8_t status)> cb) = 0;
    virtual void Unregister(uint8_t advertiser_id) = 0;
    virtual void SetData(int advertiser_id, bool set_scan_rsp, std::vector<uint8_t> data,
        std::function<void(uint8_t status)> cb) = 0;
    virtual void SetParameters(uint8_t advertiser_id, AdvertiseParameters params,
        std::function<void(uint8_t status, int8_t tx_power)> cb) = 0;
    virtual void StartAdvertising(uint8_t advertiser_id,
        std::function<void(uint8_t status)> cb, AdvertiseParameters params,
        std::vector<uint8_t> advertise_data, std::vector<uint8_t> scan_response_data,
        int timeout_s, std::function<void(uint8_t status)> timeout_cb) = 0;
    virtual void Enable(uint8_t advertiser_id, bool enable,
        std::function<void(uint8_t status)> cb, uint16_t duration, uint8_t max_ext_adv_events,
        std::function<void(uint8_t status)> timeout_cb) = 0;
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
