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
 * Stub of the removed stack layer btif sdp glue (btif_sdp.h).
 */

#ifndef BTIF_SDP_H
#define BTIF_SDP_H

#include "bt_types.h"

/* SDP record types of the removed stack layer HAL (hardware/bt_sdp.h); only
 * the entries referenced by the service layer are kept. */
typedef enum {
    SDP_TYPE_RAW = 0,
    SDP_TYPE_MAP_MAS,
    SDP_TYPE_MAP_MNS,
    SDP_TYPE_PBAP_PSE,
    SDP_TYPE_PBAP_PCE,
    SDP_TYPE_OPP_SERVER,
    SDP_TYPE_SAP_SERVER,
    SDP_TYPE_DIP,
    SDP_TYPE_MPS,
} bluetooth_sdp_types;

typedef struct _bluetooth_sdp_hdr_overlay {
    bluetooth_sdp_types type;
    bluetooth::Uuid uuid;
    uint32_t service_name_length;
    const char *service_name;
    int32_t rfcomm_channel_number;
    int32_t l2cap_psm;
    int32_t profile_version;

    /* User pointers, only used for some signals (bluedroid bt_sdp.h). */
    int user1_ptr_len;
    const uint8_t *user1_ptr;
    int user2_ptr_len;
    const uint8_t *user2_ptr;
} bluetooth_sdp_hdr_overlay;

typedef struct _bluetooth_sdp_mas_record {
    bluetooth_sdp_hdr_overlay hdr;
    uint32_t mas_instance_id;
    uint32_t supported_features;
    uint32_t supported_message_types;
} bluetooth_sdp_mas_record;

typedef struct _bluetooth_sdp_mns_record {
    bluetooth_sdp_hdr_overlay hdr;
    uint32_t supported_features;
} bluetooth_sdp_mns_record;

typedef struct _bluetooth_sdp_pse_record {
    bluetooth_sdp_hdr_overlay hdr;
    uint32_t supported_features;
    uint32_t supported_repositories;
} bluetooth_sdp_pse_record;

#define SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH 15

typedef struct _bluetooth_sdp_ops_record {
    bluetooth_sdp_hdr_overlay hdr;
    int supported_formats_list_len;
    uint8_t supported_formats_list[SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH];
} bluetooth_sdp_ops_record;

typedef struct _bluetooth_sdp_sap_record {
    bluetooth_sdp_hdr_overlay hdr;
} bluetooth_sdp_sap_record;

typedef struct _bluetooth_sdp_dip_record {
    bluetooth_sdp_hdr_overlay hdr;
    uint16_t spec_id;
    uint16_t vendor;
    uint16_t vendor_id_source;
    uint16_t product;
    uint16_t version;
    bool primary_record;
} bluetooth_sdp_dip_record;

typedef struct _bluetooth_sdp_mps_record {
    bluetooth_sdp_hdr_overlay hdr;
    uint8_t supported_scenarios_mpsd[8];
    uint8_t supported_scenarios_mpmd[8];
    uint8_t supported_dependencies[2];
} bluetooth_sdp_mps_record;

typedef struct {
    bluetooth_sdp_hdr_overlay hdr;
    union {
        bluetooth_sdp_mas_record mas;
        bluetooth_sdp_mns_record mns;
        bluetooth_sdp_pse_record pse;
        bluetooth_sdp_ops_record ops;
        bluetooth_sdp_sap_record sap;
        bluetooth_sdp_dip_record dip;
        bluetooth_sdp_mps_record mps;
    };
} bluetooth_sdp_record;

/* Callback invoked when an SDP search completes. */
typedef void (*btsdp_search_callback)(bt_status_t status, const RawAddress &bd_addr,
    const bluetooth::Uuid &uuid, int num_records, bluetooth_sdp_record *records);

typedef struct {
    size_t size;
    btsdp_search_callback sdp_search_cb;
} btsdp_callbacks_t;

/* SDP interface of the removed stack layer HAL (hardware/bt_sdp.h); only the
 * members referenced by the service layer are kept. */
typedef struct {
    size_t size;
    bt_status_t (*init)(btsdp_callbacks_t *callbacks);
    bt_status_t (*deinit)();
    bt_status_t (*sdp_search)(RawAddress *bd_addr, const bluetooth::Uuid &uuid);
    bt_status_t (*create_sdp_record)(bluetooth_sdp_record *record, int *record_handle);
    bt_status_t (*remove_sdp_record)(int sdp_handle);
} btsdp_interface_t;

#endif  // BTIF_SDP_H
