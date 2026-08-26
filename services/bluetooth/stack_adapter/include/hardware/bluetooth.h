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
 * Stub of the removed stack layer HAL interface (hardware/bluetooth.h).
 * The stack library is dlopened at runtime via hal_util; a null interface is
 * tolerated by the service layer, only the shape of the entry point matters.
 */

#ifndef HARDWARE_BLUETOOTH_H
#define HARDWARE_BLUETOOTH_H

#include <cstddef>
#include <cstdint>

#include "bt_types.h"

/* Callbacks registered by the service layer via bt_interface_t::init; the
 * signatures mirror the AdapterManager static callbacks. */
typedef struct {
    size_t size;
    void (*adapter_state_changed_cb)(bt_state_t state);
    void (*adapter_properties_cb)(bt_status_t status, int num_properties, bt_property_t* properties);
    void (*remote_device_properties_cb)(bt_status_t status, RawAddress* bd_addr, int num_properties,
        bt_property_t* properties);
    void (*device_found_cb)(int num_properties, bt_property_t* properties);
    void (*discovery_state_changed_cb)(bt_discovery_state_t state);
    void (*pin_request_cb)(RawAddress* remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod, bool min_16_digit);
    void (*ssp_request_cb)(RawAddress* remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod,
        bt_ssp_variant_t pairing_variant, uint32_t pass_key);
    void (*bond_state_changed_cb)(bt_status_t status, RawAddress* remote_bd_addr, bt_bond_state_t state);
    void (*acl_state_changed_cb)(bt_status_t status, RawAddress* remote_bd_addr, bt_acl_state_t state,
        bt_hci_error_code_t hci_reason, tBT_TRANSPORT link_type);
    void (*thread_evt_cb)(bt_cb_thread_evt evt);
    void (*dut_mode_recv_cb)(uint16_t opcode, uint8_t* buf, uint8_t len);
    void (*energy_info_cb)(bt_activity_energy_info* energy_info, bt_uid_traffic_t* uid_data);
    void (*generate_local_oob_data_cb)(tBT_TRANSPORT transport, bt_oob_data_t oob_data);
    void (*sensing_state_changed_cb)(uint8_t event_id, bt_sensing_info_t* info);
} bt_callbacks_t;

/* OS callouts optionally registered via set_os_callouts. */
typedef struct {
    size_t size;
    int (*set_wake_alarm)(uint64_t alarm_time_millis, bool should_wake);
    int (*acquire_wake_lock)(const char* lock_name);
    int (*release_wake_lock)(const char* lock_name);
} bt_os_callouts_t;

/* Forward declaration of the stubbed AVRCP service interface so that the
 * get_avrcp_service entry point can return it directly (service code assigns
 * the result without a cast). */
namespace bluetooth {
namespace avrcp {
class ServiceInterface;
}  // namespace avrcp
}  // namespace bluetooth

typedef struct {
    size_t size;
    int (*init)(...);
    int (*enable)(...);
    int (*disable)(...);
    void (*cleanup)(void);
    int (*set_os_callouts)(void *callouts);
    const void *(*get_profile_interface)(const char *profileId);
    bluetooth::avrcp::ServiceInterface *(*get_avrcp_service)(void);
    int (*get_remote_services)(...);
    int (*enable_bluetooth_highpower)(...);
    int (*enable_fast_scan)(...);
    int (*setFastScan)(...);
    int (*send_antenna_status_msg)(...);
    int (*send_ble_scan_msg)(...);
    int (*config_clear)(...);
    int (*create_bond)(...);
    int (*create_bond_out_of_band)(...);
    int (*cancel_bond)(...);
    int (*remove_bond)(...);
    int (*pin_reply)(...);
    int (*ssp_reply)(...);
    int (*get_remote_device_properties)(...);
    int (*get_remote_device_property)(...);
    int (*set_remote_device_property)(...);
    int (*set_adapter_property)(...);
    int (*start_discovery)(...);
    int (*cancel_discovery)(...);
    int (*generate_local_oob_data)(...);
} bt_interface_t;

#endif  // HARDWARE_BLUETOOTH_H
