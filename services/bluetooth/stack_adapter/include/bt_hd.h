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
 * Stub of the removed stack layer HID device profile interface (bt_hd.h).
 */

#ifndef BT_HD_H
#define BT_HD_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTHD_CONN_STATE_DISCONNECTED = 0,
    BTHD_CONN_STATE_CONNECTING,
    BTHD_CONN_STATE_CONNECTED,
    BTHD_CONN_STATE_DISCONNECTING,
} bthd_connection_state_t;

/* Report types accepted by bthd_interface_t::send_report. */
typedef enum {
    BTHD_REPORT_TYPE_OTHER = 0,
    BTHD_REPORT_TYPE_INPUT,
    BTHD_REPORT_TYPE_OUTPUT,
    BTHD_REPORT_TYPE_FEATURE,
    /* Special value for reports sent on the interrupt channel (INPUT assumed). */
    BTHD_REPORT_TYPE_INTRDATA
} bthd_report_type_t;

/* Application registration state reported by the application state callback. */
typedef enum {
    BTHD_APP_STATE_NOT_REGISTERED = 0,
    BTHD_APP_STATE_REGISTERED,
} bthd_application_state_t;

/* HID device application parameters of bthd_interface_t::register_app. */
typedef struct {
    const char *name;
    const char *description;
    const char *provider;
    uint8_t subclass;
    uint8_t *desc_list;
    int desc_list_len;
} bthd_app_param_t;

/* HID device QoS parameters of bthd_interface_t::register_app. */
typedef struct {
    uint8_t service_type;
    uint32_t token_rate;
    uint32_t token_bucket_size;
    uint32_t peak_bandwidth;
    uint32_t access_latency;
    uint32_t delay_variation;
} bthd_qos_param_t;

/* HID device callbacks (bluedroid system/include/hardware/bt_hd.h). */
typedef void (*bthd_application_state_callback)(RawAddress *bd_addr,
                                                bthd_application_state_t state);
typedef void (*bthd_connection_state_callback)(RawAddress *bd_addr,
                                               bthd_connection_state_t state);
typedef void (*bthd_get_report_callback)(uint8_t type, uint8_t id, uint16_t buffer_size);
typedef void (*bthd_set_report_callback)(uint8_t type, uint8_t id, uint16_t len,
                                         uint8_t *p_data);
typedef void (*bthd_set_protocol_callback)(uint8_t protocol);
typedef void (*bthd_intr_data_callback)(uint8_t report_id, uint16_t len, uint8_t *p_data);
typedef void (*bthd_vc_unplug_callback)(void);

typedef struct {
    size_t size;
    bthd_application_state_callback application_state_cb;
    bthd_connection_state_callback connection_state_cb;
    bthd_get_report_callback get_report_cb;
    bthd_set_report_callback set_report_cb;
    bthd_set_protocol_callback set_protocol_cb;
    bthd_intr_data_callback intr_data_cb;
    bthd_vc_unplug_callback vc_unplug_cb;
} bthd_callbacks_t;

/* HID device interface consumed by hid_device and watch services; layout
 * mirrors bluedroid system/include/hardware/bt_hd.h. */
typedef struct {
    size_t size;
    bt_status_t (*init)(bthd_callbacks_t *callbacks);
    void (*cleanup)(void);
    bt_status_t (*register_app)(bthd_app_param_t *app_param, bthd_qos_param_t *in_qos,
                                bthd_qos_param_t *out_qos);
    bt_status_t (*unregister_app)(void);
    bt_status_t (*connect)(RawAddress *bd_addr);
    bt_status_t (*disconnect)(void);
    bt_status_t (*send_report)(bthd_report_type_t type, uint8_t id, uint16_t len,
                               uint8_t *p_data);
    bt_status_t (*report_error)(uint8_t error);
    bt_status_t (*virtual_cable_unplug)(void);
} bthd_interface_t;

#endif  // BT_HD_H
