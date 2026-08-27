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
 * Stub of the removed stack layer GATT profile glue header (bt_gatt.h).
 */

#ifndef BT_GATT_H
#define BT_GATT_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bt_types.h"
#include "btm_ble_api_types.h"
#include "gatt_api.h"
#include "base/callback.h"

#define BLE_GATT_MAX_ATTR_NUM 1000
#define BLE_GATT_ATTR_HANDLE_START 0x0001
#define BLE_GATT_ATTR_HANDLE_END 0xFFFF

/* Max length of a GATT attribute value (bluedroid GATT_MAX_ATTR_LEN). */
#define BTGATT_MAX_ATTR_LEN 512

/* Timeouts used by the gatt services; values match config/bt_config.h which
 * may be included first by the same translation unit. */
#ifndef BLUETOOTH_GATT_CLIENT_TIMEOUT
#define BLUETOOTH_GATT_CLIENT_TIMEOUT 1 // 1s
#endif  // BLUETOOTH_GATT_CLIENT_TIMEOUT
#ifndef BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT
#define BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT 2 // 2s
#endif  // BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT

typedef enum {
    BLE_GATT_CONNECT_OK = 0,
    BLE_GATT_CONNECT_FAIL,
    BLE_GATT_CONNECT_CANCEL,
} tBLE_GATT_CONNECT;

typedef enum {
    BLE_GATT_DISCONNECT_REASON_NORMAL = 0,
    BLE_GATT_DISCONNECT_REASON_REMOTE,
} tBLE_GATT_DISCONNECT_REASON;

typedef enum {
    BLE_GATT_SERVICE_PRIMARY = 0x01,
    BLE_GATT_SERVICE_SECONDARY = 0x02,
} tBLE_GATT_SERVICE_TYPE;

typedef enum {
    BLE_GATT_ATTR_PRIMARY_SERVICE = 0,
    BLE_GATT_ATTR_SECONDARY_SERVICE,
    BLE_GATT_ATTR_INCLUDE_SERVICE,
    BLE_GATT_ATTR_CHARACTERISTIC,
    BLE_GATT_ATTR_CHARACTERISTIC_DECLARATION,
    BLE_GATT_ATTR_DESCRIPTOR,
} tBLE_GATT_ATTR_TYPE;

typedef enum {
    BLE_GATT_PERM_READ = 0x0001,
    BLE_GATT_PERM_READ_ENCRYPTED = 0x0002,
    BLE_GATT_PERM_READ_ENCRYPTED_MITM = 0x0004,
    BLE_GATT_PERM_WRITE = 0x0010,
    BLE_GATT_PERM_WRITE_ENCRYPTED = 0x0020,
    BLE_GATT_PERM_WRITE_ENCRYPTED_MITM = 0x0040,
    BLE_GATT_PERM_WRITE_SIGNED = 0x0080,
    BLE_GATT_PERM_WRITE_SIGNED_MITM = 0x0100,
} tBLE_GATT_PERM;

typedef enum {
    BLE_GATT_CHAR_PROP_BROADCAST = 0x01,
    BLE_GATT_CHAR_PROP_READ = 0x02,
    BLE_GATT_CHAR_PROP_WRITE_NO_RSP = 0x04,
    BLE_GATT_CHAR_PROP_WRITE = 0x08,
    BLE_GATT_CHAR_PROP_NOTIFY = 0x10,
    BLE_GATT_CHAR_PROP_INDICATE = 0x20,
    BLE_GATT_CHAR_PROP_SIGNED_WRITE = 0x40,
} tBLE_GATT_CHAR_PROP;

typedef struct {
    uint8_t uuid[16];
} tBLE_GATT_UUID;

/* Service database element types reported by the stack. */
#define BTGATT_DB_PRIMARY_SERVICE 0
#define BTGATT_DB_SECONDARY_SERVICE 1
#define BTGATT_DB_INCLUDED_SERVICE 2
#define BTGATT_DB_CHARACTERISTIC 3
#define BTGATT_DB_DESCRIPTOR 4

/* Gatt attribute value, see btgatt_read_params_t. */
typedef struct {
    uint16_t len;
    uint8_t *value;
} bt_gatt_value_t;

/* GATT value filled in the response to a remote read request, and the
 * response union (bluedroid system/include/hardware/bt_gatt_server.h). */
typedef struct {
    uint8_t value[BTGATT_MAX_ATTR_LEN];
    uint16_t handle;
    uint16_t offset;
    uint16_t len;
    uint8_t auth_req;
} btgatt_value_t;

typedef union {
    btgatt_value_t attr_value;
    uint16_t handle;
} btgatt_response_t;

typedef struct {
    bluetooth::Uuid uuid;
    uint8_t type;
    uint16_t attribute_handle;
    uint16_t start_handle;
    uint16_t end_handle;
    uint8_t properties;
    uint32_t permissions;
} btgatt_db_element_t;

typedef struct {
    RawAddress bda;
    uint16_t handle;
    uint16_t len;
    uint8_t *value;
    bool is_connected;
} btgatt_notify_params_t;

typedef struct {
    uint16_t handle;
    uint8_t value_type;
    uint16_t status;
    bt_gatt_value_t value;
} btgatt_read_params_t;

typedef struct {
    uint64_t timeStamp;
} btgatt_rsp_params_t;

typedef struct {
    RawAddress bd_addr;
    int client_if;
    int advertiser_state;
    int addr_type;
    int rssi_value;
    uint8_t adv_pkt_len;
    uint8_t *p_adv_pkt_data;
    uint8_t scan_rsp_len;
    uint8_t *p_scan_rsp_data;
} btgatt_track_adv_info_t;

struct btgatt_client_callbacks_t;
struct btgatt_server_callbacks_t;
struct btgatt_scanner_callbacks_t;

/* Callbacks delivered to the stack through btgatt_interface_t::init. */
typedef struct {
    size_t size;
    const btgatt_client_callbacks_t *client_cb;
    const btgatt_server_callbacks_t *server_cb;
    const btgatt_scanner_callbacks_t *scanner_cb;
} btgatt_callbacks_t;

typedef struct btgatt_scanner_callbacks_t {
    void (*scan_result_cb)(uint16_t bleEvtType, uint8_t addrType, RawAddress *bda, uint8_t blePrimaryPhy,
        uint8_t bleSecondaryPhy, uint8_t bleAdvertisingSid, int8_t bleTxPower, int8_t rssi,
        uint16_t blePeriodicAdvInt, std::vector<uint8_t> advData);
    void (*batchscan_reports_cb)(int clientIf, int status, int reportFormat, int numRecords,
        std::vector<uint8_t> data);
    void (*batchscan_threshold_cb)(int clientIf);
    void (*track_adv_event_cb)(btgatt_track_adv_info_t *advTrackInfo);
} btgatt_scanner_callbacks_t;

typedef struct btgatt_server_callbacks_t {
    void (*register_server_cb)(int status, int serverIf, const bluetooth::Uuid &appUuid);
    void (*connection_cb)(int connId, int serverIf, int connected, const RawAddress &bda, int reason);
    void (*service_added_cb)(int status, int serverIf, std::vector<btgatt_db_element_t> service);
    void (*service_stopped_cb)(int status, int serverIf, int serviceHandle);
    void (*service_deleted_cb)(int status, int serverIf, int serviceHandle);
    void (*request_read_characteristic_cb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool isLong);
    void (*request_read_descriptor_cb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool isLong);
    void (*request_write_characteristic_cb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value);
    void (*request_write_descriptor_cb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value);
    void (*request_exec_write_cb)(int connId, int transId, const RawAddress &bda, int execWrite);
    void (*response_confirmation_cb)(int status, int handle);
    void (*indication_sent_cb)(int connId, int status);
    void (*congestion_cb)(int connId, bool congested);
    void (*mtu_changed_cb)(int connId, int mtu);
    void (*phy_updated_cb)(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status);
    void (*conn_updated_cb)(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status);
} btgatt_server_callbacks_t;

typedef struct btgatt_client_callbacks_t {
    void (*register_client_cb)(int status, int clientIf, const bluetooth::Uuid &appUuid);
    void (*connect_cb)(int connId, int status, int clientIf, const RawAddress &bda);
    void (*disconnect_cb)(int connId, int status, int clientIf, const RawAddress &bda, int reason);
    void (*cancel_open_cb)(int connId, int status, int clientIf, const RawAddress &bda);
    void (*search_complete_cb)(int connId, int status);
    void (*register_for_notification_cb)(int connId, int registered, int status, uint16_t handle);
    void (*notify_cb)(int connId, const btgatt_notify_params_t &data);
    void (*read_characteristic_cb)(int connId, int status, btgatt_read_params_t *pData);
    void (*write_characteristic_cb)(int connId, int status, uint16_t handle, const btgatt_rsp_params_t &rspContext);
    void (*read_descriptor_cb)(int connId, int status, const btgatt_read_params_t &data);
    void (*write_descriptor_cb)(int connId, int status, uint16_t handle);
    void (*execute_write_cb)(int connId, int status);
    void (*read_remote_rssi_cb)(int clientIf, const RawAddress &bda, int rssi, int status);
    void (*configure_mtu_cb)(int connId, int status, int mtu);
    void (*congestion_cb)(int connId, bool congested);
    void (*get_gatt_db_cb)(int connId, const btgatt_db_element_t *db, int count);
    void (*services_removed_cb)(int connId, uint16_t startHandle, uint16_t endHandle);
    void (*services_added_cb)(int connId, const btgatt_db_element_t &added, int addedCount);
    void (*phy_updated_cb)(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status);
    void (*conn_updated_cb)(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status);
    void (*service_changed_cb)(int connId);
} btgatt_client_callbacks_t;

/* Stack gatt interface, obtained via get_profile_interface(BT_PROFILE_GATT_ID). */
typedef struct {
    size_t size;
    bt_status_t (*init)(const btgatt_callbacks_t *callbacks);
} btgatt_interface_t;

/* GATT client interface consumed by the service layer (gatt_client_application.cpp);
 * signatures follow the call sites, bluedroid reference is
 * system/include/hardware/bt_gatt_client.h. */
typedef struct {
    size_t size;
    bt_status_t (*register_client)(const bluetooth::Uuid &app_uuid, bool eatt_support);
    bt_status_t (*unregister_client)(int client_if);
    bt_status_t (*connect)(int client_if, const RawAddress &bd_addr, bool is_direct, int transport,
        bool opportunistic, int phy);
    bt_status_t (*disconnect)(int client_if, const RawAddress &bd_addr, int conn_id);
    bt_status_t (*search_service)(int conn_id, const bluetooth::Uuid *filter_uuid);
    bt_status_t (*get_gatt_db)(int conn_id);
    bt_status_t (*read_characteristic)(int conn_id, int handle, int auth_req);
    bt_status_t (*read_using_characteristic_uuid)(int conn_id, const bluetooth::Uuid &uuid,
        int start_handle, int end_handle, int auth_req);
    bt_status_t (*write_characteristic)(int conn_id, int handle, int write_type, int auth_req,
        std::vector<uint8_t> value);
    bt_status_t (*read_descriptor)(int conn_id, int handle, int auth_req);
    bt_status_t (*write_descriptor)(int conn_id, int handle, int auth_req,
        std::vector<uint8_t> value);
    bt_status_t (*register_for_notification)(int client_if, const RawAddress &bd_addr, int handle);
    bt_status_t (*deregister_for_notification)(int client_if, const RawAddress &bd_addr, int handle);
    bt_status_t (*configure_mtu)(int conn_id, int mtu);
    bt_status_t (*conn_parameter_update)(const RawAddress &bd_addr, int min_interval,
        int max_interval, int latency, int timeout, int min_ce_len, int max_ce_len);
    bt_status_t (*read_remote_rssi)(int client_if, const RawAddress &bd_addr);
    bt_status_t (*set_preferred_phy)(const RawAddress &bd_addr, uint8_t tx_phy, uint8_t rx_phy,
        uint16_t phy_options);
    bt_status_t (*read_phy)(const RawAddress &bd_addr,
        base::Callback<void(uint8_t tx_phy, uint8_t rx_phy, uint8_t status)> cb);
} btgatt_client_interface_t;

/* Null-initialized instance: the service layer detects the empty table and
 * takes its "stack unavailable" path (register_client == nullptr checks). */
inline btgatt_client_interface_t btgattClientInterface {};

/* GATT server interface consumed by gatt_server_application.cpp; bluedroid
 * reference is system/include/hardware/bt_gatt_server.h. */
typedef struct {
    size_t size;
    bt_status_t (*register_server)(const bluetooth::Uuid &uuid, bool eatt_support);
    bt_status_t (*unregister_server)(int server_if);
    bt_status_t (*connect)(int server_if, const RawAddress &bd_addr, bool is_direct, int transport);
    bt_status_t (*disconnect)(int server_if, const RawAddress &bd_addr, int conn_id);
    bt_status_t (*add_service)(int server_if, std::vector<btgatt_db_element_t> service);
    bt_status_t (*delete_service)(int server_if, int service_handle);
    bt_status_t (*send_indication)(int server_if, int attribute_handle, int conn_id, int confirm,
        std::vector<uint8_t> value);
    bt_status_t (*send_response)(int conn_id, int trans_id, int status,
        const btgatt_response_t &response);
    bt_status_t (*set_preferred_phy)(const RawAddress &bd_addr, uint8_t tx_phy, uint8_t rx_phy,
        uint16_t phy_options);
    bt_status_t (*read_phy)(const RawAddress &bd_addr,
        base::Callback<void(uint8_t tx_phy, uint8_t rx_phy, uint8_t status)> cb);
} btgatt_server_interface_t;

/* Null-initialized instance; gatt_server_service.cpp checks register_server
 * before calling into the table. */
inline btgatt_server_interface_t btgattServerInterface {};

/* Scan filter parameter setup, see BleScanFilter::SetScanFilterParameters. */
typedef struct {
    uint16_t feat_seln;
    uint16_t list_logic_type;
    uint8_t filt_logic_type;
    int8_t rssi_high_thres;
    int8_t rssi_low_thres;
    uint8_t dely_mode;
    uint16_t found_timeout;
    uint16_t lost_timeout;
    uint16_t found_timeout_cnt;
    uint16_t num_of_tracking_entries;
} btgatt_filt_param_setup_t;

/* One scan filter command in APCF format, see BleScanFilter::GetCommand. */
typedef struct {
    uint8_t type;
    RawAddress address;
    uint8_t addr_type;
    std::array<uint8_t, 16> irk;
    bluetooth::Uuid uuid;
    bluetooth::Uuid uuid_mask;
    std::vector<uint8_t> name;
    uint16_t company;
    uint16_t company_mask;
    std::vector<uint8_t> data;
    std::vector<uint8_t> data_mask;
} ApcfCommand;

#endif  // BT_GATT_H
