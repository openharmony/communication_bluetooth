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
 * Stub of the removed stack layer HID host profile interface (bt_hh.h),
 * mirroring bluedroid system/include/hardware/bt_hh.h.
 */

#ifndef BT_HH_H
#define BT_HH_H

#include <cstddef>
#include <cstdint>

#include "bt_types.h"

#define BTHH_MAX_DSC_LEN 884

typedef enum {
    BTHH_CONN_STATE_DISCONNECTED = 0,
    BTHH_CONN_STATE_CONNECTING,
    BTHH_CONN_STATE_CONNECTED,
    BTHH_CONN_STATE_DISCONNECTING,
} bthh_connection_state_t;

typedef enum {
    BTHH_ERR_NONE = 0,
    BTHH_ERR_DEV_NOT_CONNECTED,
} bthh_status_t;

typedef enum {
    BTHH_REPORT_MODE = 0x00,
    BTHH_BOOT_MODE = 0x01,
    BTHH_UNSUPPORTED_MODE = 0xff
} bthh_protocol_mode_t;

/* Report types */
typedef enum { BTHH_INPUT_REPORT = 1, BTHH_OUTPUT_REPORT, BTHH_FEATURE_REPORT } bthh_report_type_t;

typedef struct {
    int attr_mask;
    uint8_t sub_class;
    uint8_t app_id;
    int vendor_id;
    int product_id;
    int version;
    uint8_t ctry_code;
    int dl_len;
    uint8_t dsc_list[BTHH_MAX_DSC_LEN];
} bthh_hid_info_t;

typedef void (*bthh_connection_state_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                               tBT_TRANSPORT transport,
                                               bthh_connection_state_t state);

typedef void (*bthh_virtual_unplug_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                             tBT_TRANSPORT transport, bthh_status_t hh_status);

typedef void (*bthh_hid_info_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                       tBT_TRANSPORT transport, bthh_hid_info_t hid_info);

typedef void (*bthh_protocol_mode_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                            tBT_TRANSPORT transport, bthh_status_t hh_status,
                                            bthh_protocol_mode_t mode);

typedef void (*bthh_idle_time_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                        tBT_TRANSPORT transport, bthh_status_t hh_status,
                                        int idle_rate);

typedef void (*bthh_get_report_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                         tBT_TRANSPORT transport, bthh_status_t hh_status,
                                         uint8_t* rpt_data, int rpt_size);

typedef void (*bthh_handshake_callback)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                        tBT_TRANSPORT transport, bthh_status_t hh_status);

/* BT-HH callback structure. */
typedef struct {
    size_t size;
    bthh_connection_state_callback connection_state_cb;
    bthh_hid_info_callback hid_info_cb;
    bthh_protocol_mode_callback protocol_mode_cb;
    bthh_idle_time_callback idle_time_cb;
    bthh_get_report_callback get_report_cb;
    bthh_virtual_unplug_callback virtual_unplug_cb;
    bthh_handshake_callback handshake_cb;
} bthh_callbacks_t;

/* Represents the standard BT-HH interface. */
typedef struct {
    size_t size;
    bt_status_t (*init)(bthh_callbacks_t* callbacks);
    bt_status_t (*connect)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport);
    bt_status_t (*disconnect)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport,
                              bool reconnect_allowed);
    bt_status_t (*virtual_unplug)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                  tBT_TRANSPORT transport);
    bt_status_t (*set_info)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport,
                            bthh_hid_info_t hid_info);
    bt_status_t (*get_protocol)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                tBT_TRANSPORT transport, bthh_protocol_mode_t protocolMode);
    bt_status_t (*set_protocol)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                tBT_TRANSPORT transport, bthh_protocol_mode_t protocolMode);
    bt_status_t (*get_idle_time)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                 tBT_TRANSPORT transport);
    bt_status_t (*set_idle_time)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                 tBT_TRANSPORT transport, uint8_t idleTime);
    bt_status_t (*get_report)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport,
                              bthh_report_type_t reportType, uint8_t reportId, int bufferSize);
    bt_status_t (*get_report_reply)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type,
                                    tBT_TRANSPORT transport, bthh_status_t status, char* report,
                                    uint16_t size);
    bt_status_t (*set_report)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport,
                              bthh_report_type_t reportType, char* report);
    bt_status_t (*send_data)(RawAddress* bd_addr, tBLE_ADDR_TYPE addr_type, tBT_TRANSPORT transport,
                             char* data);
    void (*cleanup)(void);
    void (*configure_enabled_profiles)(bool enable_hidp, bool enable_hogp);
} bthh_interface_t;

#endif  // BT_HH_H
