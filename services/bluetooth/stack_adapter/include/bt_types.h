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
 * Common type definitions of the ported stack layer (originally from
 * Bluedroid). Types are kept in the global namespace (AOSP style);
 * STACK::Xxx references resolve to the global ::Xxx symbols.
 */

#ifndef BT_TYPES_H
#define BT_TYPES_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include "types/raw_address.h"

/* STACK::Xxx references resolve to the global ::Xxx symbols. */
#ifndef STACK
#define STACK
#endif

/* RawAddress is provided by the ported stack layer, see types/raw_address.h. */

/* INVALID_MAC_ADDRESS is provided by frameworks/inner/ipc/common/bt_def.h
 * as a constexpr, do not redefine it as a macro here. */

#define BT_BD_NAME_LEN 248
#define BT_PIN_CODE_LEN 16

typedef struct {
    uint8_t name[BT_BD_NAME_LEN + 1];
} bt_bdname_t;

typedef struct {
    uint8_t pin[BT_PIN_CODE_LEN + 1];
} bt_pin_code_t;

typedef enum {
    BT_STATUS_SUCCESS = 0,
    BT_STATUS_FAIL,
    BT_STATUS_NOT_READY,
    BT_STATUS_NOMEM,
    BT_STATUS_BUSY,
    BT_STATUS_DONE,
    BT_STATUS_UNSUPPORTED = 0xFE,
    BT_STATUS_PARM_INVALID,
    BT_STATUS_UNHANDLED,
    BT_STATUS_AUTH_FAILURE,
    BT_STATUS_RMT_DEV_DOWN,
    BT_STATUS_AUTH_REJECTED,
    BT_STATUS_JNI_ENVIRONMENT_ERROR,
    BT_STATUS_JNI_THREAD_ATTACH_ERROR,
    BT_STATUS_INVALID_STATIC_RAND_ADDR,
    BT_STATUS_PENDING,
    BT_STATUS_UNACCEPT_CONN_INTERVAL,
    BT_STATUS_PARAM_OUT_OF_RANGE,
    BT_STATUS_ALREADY_OPENED,
    BT_STATUS_ALREADY_CLOSED,
    BT_STATUS_AGAIN,
    BT_STATUS_WAKELOCK_ERROR,
} bt_status_t;

/* Human readable form of a bt_status_t, used by the service layer logging. */
inline std::string bt_status_text(const bt_status_t status)
{
    switch (status) {
        case BT_STATUS_SUCCESS:
            return "success";
        case BT_STATUS_FAIL:
            return "failed";
        case BT_STATUS_NOT_READY:
            return "not ready";
        case BT_STATUS_NOMEM:
            return "no memory";
        case BT_STATUS_BUSY:
            return "busy";
        case BT_STATUS_DONE:
            return "done";
        default:
            return "unknown";
    }
}

typedef enum {
    BT_PROPERTY_BDNAME = 0x1,
    BT_PROPERTY_BDADDR,
    BT_PROPERTY_UUIDS,
    BT_PROPERTY_CLASS_OF_DEVICE,
    BT_PROPERTY_TYPE_OF_DEVICE,
    BT_PROPERTY_REMOTE_RSSI,
    BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
    BT_PROPERTY_ADAPTER_BONDED_DEVICES,
    BT_PROPERTY_ADAPTER_DISCOVERABLE,
    BT_PROPERTY_ADAPTER_SCAN_MODE,
    BT_PROPERTY_REMOTE_FRIENDLY_NAME,
    BT_PROPERTY_REMOTE_VERSION_INFO,
    BT_PROPERTY_LOCAL_LE_FEATURES,
    BT_PROPERTY_LOCAL_IO_CAPS,
    BT_PROPERTY_LOCAL_IO_CAPS_BLE,
    BT_PROPERTY_DYNAMIC_AUDIO_BUFFER_SIZE,
    BT_PROPERTY_REMOTE_IS_BEING_PAIRED,
    BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP,
    /* Vendor property types of the stubbed stack; the service layer refers
     * to them as bt_property_type_t enumerators. */
    BT_PROPERTY_AUTO_CONNECT_SWITCH,
    BT_PROPERTY_ClOUD_CAP,
    HW_BT_PROPERTY_ABS_VOLUM_KEY,
    HW_BT_PROPERTY_CUSTOM_TYPE,
    HW_BT_PROPERTY_RMT_IO_CAP_KEY,
    BT_PROPERTY_VENDOR_ID,
    BT_PROPERTY_PRODUCT_ID,
    BT_PROPERTY_REMOTE_MANUFACTURE_HW,
    BT_PROPERTY_CONNECTION_TIME,
    BT_PROPERTY_NAME_CHANGE_VERSION,
    BT_PROPERTY_REMOTE_NEW_MODEL_ID,
    BT_PROPERTY_REMOTE_MODEL_ID,
    BT_PROPERTY_DEVICE_INFO,
    BT_PROPERTY_TIMESTAMP,
    BT_PROPERTY_DEVICE_TYPE_ID,
    BT_PROPERTY_REMOTE_SUB_MODEL_ID,
} bt_property_type_t;

typedef struct {
    bt_property_type_t type;
    int len;
    void *val;
} bt_property_t;

typedef enum {
    BT_TRANSPORT_AUTO = 0,
    BT_TRANSPORT_BR_EDR = 1,
    BT_TRANSPORT_LE = 2,
    BT_TRANSPORT_UNKNOWN = 3,
    BT_TRANSPORT_INVALID = 255,
} tBT_TRANSPORT;

typedef enum {
    BLE_ADDR_PUBLIC = 0x00,
    BLE_ADDR_RANDOM = 0x01,
    BLE_ADDR_PUBLIC_ID = 0x02,
    BLE_ADDR_RANDOM_ID = 0x03,
    BLE_ADDR_ANONYMOUS = 0xFF,
} tBLE_ADDR_TYPE;

typedef enum {
    BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
    BT_SSP_VARIANT_PASSKEY_ENTRY,
    BT_SSP_VARIANT_CONSENT,
    BT_SSP_VARIANT_PASSKEY_NOTIFICATION,
} bt_ssp_variant_t;

typedef enum {
    BT_HCI_ERROR_SUCCESS = 0x00,
    BT_HCI_ERROR_ILLEGAL_COMMAND = 0x01,
    BT_HCI_ERROR_NO_CONNECTION = 0x02,
    BT_HCI_ERROR_HW_FAILURE = 0x03,
    BT_HCI_ERROR_PAGE_TIMEOUT = 0x04,
    BT_HCI_ERROR_AUTH_FAILURE = 0x05,
    BT_HCI_ERROR_KEY_MISSING = 0x06,
    BT_HCI_ERROR_MEMORY_FULL = 0x07,
    BT_HCI_ERROR_CONNECTION_TOUT = 0x08,
    BT_HCI_ERROR_MAX_NUM_OF_CONNECTIONS = 0x09,
    BT_HCI_ERROR_MAX_NUM_OF_SCO = 0x0A,
    BT_HCI_ERROR_ACL_CONNECTION_EXISTS = 0x0B,
    BT_HCI_ERROR_COMMAND_DISALLOWED = 0x0C,
} bt_hci_error_code_t;

typedef struct {
    uint8_t c[16];
    uint8_t r[16];
    /* Local OOB data: 7 bytes address-with-type (little endian) followed by
     * the device name and LE role as reported by the stack. */
    uint8_t address[7];
    uint8_t device_name[BT_BD_NAME_LEN + 1];
    uint8_t le_device_role;
    bool is_valid;
} bt_oob_data_t;

typedef enum {
    BT_CB_THREAD_EVT_APP_STARTED = 0,
    BT_CB_THREAD_EVT_APP_STOPPED,
    BT_CB_THREAD_EVT_USER_CONFIRM_REQUEST,
} bt_cb_thread_evt;

typedef struct {
    uint64_t stack_state;
    uint64_t controller_idle_time_ms;
    uint64_t tx_time_ms;
    uint64_t rx_time_ms;
    uint64_t idle_time_ms;
    uint64_t energy_used;
} bt_activity_energy_info;

typedef struct {
    uint16_t uid;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} bt_uid_traffic_t;

/* Remote chip version info carried by the BT_PROPERTY_REMOTE_VERSION_INFO
 * property (vendor extension of the stubbed stack). */
typedef struct {
    int32_t version;
    int32_t sub_ver;
    int32_t manufacturer;
} bt_remote_version_t;

/* Adapter scan modes as reported by the stack. */
typedef enum {
    BT_SCAN_MODE_NONE = 0,
    BT_SCAN_MODE_CONNECTABLE = 1,
    BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE = 2,
} bt_scan_mode_t;

/* Classic device type values of the stubbed stack. */
#define BT_DEVICE_TYPE_BREDR 1
#define BT_DEVICE_TYPE_BLE 2
#define BT_DEVICE_TYPE_DUMO 3


typedef struct {
    uint8_t le_features[8];
    uint16_t total_trackable_advertisers;
    uint16_t le_maximum_advertising_data_length;
    bool le_extended_advertising_supported;
    bool le_2m_phy_supported;
} bt_local_le_features_t;

typedef enum {
    BT_STATE_OFF = 0,
    BT_STATE_ON = 1,
} bt_state_t;

typedef enum {
    BT_DISCOVERY_STOPPED = 0,
    BT_DISCOVERY_STARTED,
} bt_discovery_state_t;

typedef enum {
    BT_BOND_STATE_NONE = 0,
    BT_BOND_STATE_BONDING,
    BT_BOND_STATE_BONDED,
} bt_bond_state_t;

typedef enum {
    BT_ACL_STATE_CONNECTED = 0,
    BT_ACL_STATE_DISCONNECTED,
} bt_acl_state_t;

typedef enum {
    BT_PIN_REQUEST = 0,
    BT_PIN_ENTERED,
} bt_pin_state_t;


/*
 * Stub Uuid of the removed stack layer. STACK::bluetooth::Uuid expands to
 * bluetooth::Uuid after the macro, so it lives in the global bluetooth
 * namespace, distinct from OHOS::bluetooth::Uuid.
 */
namespace bluetooth {
class Uuid {
public:
    static constexpr size_t kNumBytes128 = 16;

    Uuid() { uu.fill(0); }
    explicit Uuid(const std::array<uint8_t, kNumBytes128> &uuid) : uu(uuid) {}

    static Uuid From128BitBE(const uint8_t *uuid)
    {
        std::array<uint8_t, kNumBytes128> tmp;
        std::copy(uuid, uuid + kNumBytes128, tmp.begin());
        return Uuid(tmp);
    }

    static Uuid From128BitBE(const std::array<uint8_t, kNumBytes128> &uuid)
    {
        return Uuid(uuid);
    }

    /* RFC 4122 base UUID, also used as the prefix of short (16/32 bit) uuids. */
    static constexpr std::array<uint8_t, kNumBytes128> kBase = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

    static Uuid From16Bit(uint16_t uuid16bit)
    {
        Uuid tmp(kBase);
        tmp.uu[2] = static_cast<uint8_t>((uuid16bit >> 8) & 0xFF);
        tmp.uu[3] = static_cast<uint8_t>(uuid16bit & 0xFF);
        return tmp;
    }

    static Uuid From32Bit(uint32_t uuid32bit)
    {
        Uuid tmp(kBase);
        tmp.uu[0] = static_cast<uint8_t>((uuid32bit >> 24) & 0xFF);
        tmp.uu[1] = static_cast<uint8_t>((uuid32bit >> 16) & 0xFF);
        tmp.uu[2] = static_cast<uint8_t>((uuid32bit >> 8) & 0xFF);
        tmp.uu[3] = static_cast<uint8_t>(uuid32bit & 0xFF);
        return tmp;
    }

    static Uuid FromString(const std::string &uuid)
    {
        Uuid tmp;
        if (uuid.length() < 36) {
            return tmp;
        }
        for (size_t i = 0; i < 8; ++i) {
            tmp.uu[i] = static_cast<uint8_t>(strtoul(uuid.substr(i * 2, 2).c_str(), nullptr, 16));
        }
        for (size_t i = 0; i < 4; ++i) {
            tmp.uu[8 + i] = static_cast<uint8_t>(
                strtoul(uuid.substr(9 + i * 2, 2).c_str(), nullptr, 16));
        }
        for (size_t i = 0; i < 4; ++i) {
            tmp.uu[12 + i] = static_cast<uint8_t>(
                strtoul(uuid.substr(14 + i * 2, 2).c_str(), nullptr, 16));
        }
        return tmp;
    }

    /* Random 128-bit uuid, used by the service layer to register a gatt
     * client application with the (stubbed) stack. */
    static Uuid GetRandom()
    {
        Uuid tmp;
        for (auto &b : tmp.uu) {
            b = static_cast<uint8_t>(std::rand());
        }
        return tmp;
    }

    std::string ToString() const
    {
        char buf[37] = { 0 };
        snprintf(buf, sizeof(buf),
            "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            uu[0], uu[1], uu[2], uu[3], uu[4], uu[5], uu[6], uu[7], uu[8], uu[9],
            uu[10], uu[11], uu[12], uu[13], uu[14], uu[15]);
        return std::string(buf);
    }

    std::array<uint8_t, kNumBytes128> uu;
};
}  // namespace bluetooth

/*
 * Sensing info reported by the stubbed stack. Defined after RawAddress and
 * bluetooth::Uuid since it embeds them by value.
 */
typedef struct {
    uint8_t sensing_state;
    uint8_t event_type;
    uint32_t timestamp;
    RawAddress addr;
    bluetooth::Uuid uuid;
    uint32_t resourceId;
    uint16_t interval;
} bt_sensing_info_t;

/* Stream accessors of the removed stack layer packet.h, reading big-endian
 * fields from a byte stream and advancing the cursor. */
#define STREAM_TO_UINT8(p, s) \
    { (p) = static_cast<uint8_t>(*(s)); (s)++; }
#define STREAM_TO_INT8(p, s) \
    { (p) = static_cast<int8_t>(*(s)); (s)++; }
#define STREAM_TO_UINT16(p, s) \
    { (p) = static_cast<uint16_t>(static_cast<uint16_t>(*(s)) << 8 | static_cast<uint16_t>(*((s) + 1))); (s) += 2; }

#endif  // BT_TYPES_H
