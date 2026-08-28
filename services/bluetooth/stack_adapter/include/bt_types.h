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
#include "bt_uuid.h"

/* STACK::Xxx references resolve to the global ::Xxx symbols. */
#ifndef STACK
#define STACK
#endif

/* RawAddress is provided by the ported stack layer, see types/raw_address.h. */

/* INVALID_MAC_ADDRESS is provided by frameworks/inner/ipc/common/bt_def.h
 * as a constexpr, do not redefine it as a macro here. */

#define BT_BD_NAME_LEN 248
#define BT_PIN_CODE_LEN 16

struct BtBdname {
    uint8_t name[BT_BD_NAME_LEN + 1];
};

struct BtPinCode {
    uint8_t pin[BT_PIN_CODE_LEN + 1];
};

enum BtStackStatus {
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
};

/* Human readable form of a BtStackStatus, used by the service layer logging. */
inline std::string BtStatusText(const BtStackStatus status)
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

enum BtPropertyType {
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
     * to them as BtPropertyType enumerators. */
    BT_PROPERTY_AUTO_CONNECT_SWITCH,
    BT_PROPERTY_CLOUD_CAP,
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
};

struct BtProperty {
    BtPropertyType type;
    int len;
    void *val;
};

enum BtTransport {
    BT_TRANSPORT_AUTO = 0,
    BT_TRANSPORT_BR_EDR = 1,
    BT_TRANSPORT_LE = 2,
    BT_TRANSPORT_UNKNOWN = 3,
    BT_TRANSPORT_INVALID = 255,
};

enum BleAddrType {
    BLE_ADDR_PUBLIC = 0x00,
    BLE_ADDR_RANDOM = 0x01,
    BLE_ADDR_PUBLIC_ID = 0x02,
    BLE_ADDR_RANDOM_ID = 0x03,
    BLE_ADDR_ANONYMOUS = 0xFF,
};

enum BtSspVariant {
    BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
    BT_SSP_VARIANT_PASSKEY_ENTRY,
    BT_SSP_VARIANT_CONSENT,
    BT_SSP_VARIANT_PASSKEY_NOTIFICATION,
};

enum BtHciErrorCode {
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
};

struct BtStackOobData {
    uint8_t c[16];
    uint8_t r[16];
    /* Local OOB data: 7 bytes address-with-type (little endian) followed by
     * the device name and LE role as reported by the stack. */
    uint8_t address[7];
    uint8_t deviceName[BT_BD_NAME_LEN + 1];
    uint8_t leDeviceRole;
    bool isValid;
};

enum BtCbThreadEvt {
    BT_CB_THREAD_EVT_APP_STARTED = 0,
    BT_CB_THREAD_EVT_APP_STOPPED,
    BT_CB_THREAD_EVT_USER_CONFIRM_REQUEST,
};

struct BtActivityEnergyInfo {
    uint64_t stackState;
    uint64_t controllerIdleTimeMs;
    uint64_t txTimeMs;
    uint64_t rxTimeMs;
    uint64_t idleTimeMs;
    uint64_t energyUsed;
};

struct BtUidTraffic {
    uint16_t uid;
    uint64_t rxBytes;
    uint64_t txBytes;
};

/* Remote chip version info carried by the BT_PROPERTY_REMOTE_VERSION_INFO
 * property (vendor extension of the stubbed stack). */
struct BtRemoteVersion {
    int32_t version;
    int32_t subVer;
    int32_t manufacturer;
};

/* Adapter scan modes as reported by the stack. */
enum BtScanMode {
    BT_SCAN_MODE_NONE = 0,
    BT_SCAN_MODE_CONNECTABLE = 1,
    BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE = 2,
};

/* Classic device type values of the stubbed stack. */
#define BT_DEVICE_TYPE_BREDR 1
#define BT_DEVICE_TYPE_BLE 2
#define BT_DEVICE_TYPE_DUMO 3


struct BtLocalLeFeatures {
    uint8_t leFeatures[8];
    uint16_t totalTrackableAdvertisers;
    uint16_t leMaximumAdvertisingDataLength;
    bool leExtendedAdvertisingSupported;
    bool le2mPhySupported;
};

enum BtState {
    BT_STATE_OFF = 0,
    BT_STATE_ON = 1,
};

enum BtDiscoveryState {
    BT_DISCOVERY_STOPPED = 0,
    BT_DISCOVERY_STARTED,
};

enum BtBondState {
    BT_BOND_STATE_NONE = 0,
    BT_BOND_STATE_BONDING,
    BT_BOND_STATE_BONDED,
};

enum BtAclState {
    BT_ACL_STATE_CONNECTED = 0,
    BT_ACL_STATE_DISCONNECTED,
};

enum BtPinState {
    BT_PIN_REQUEST = 0,
    BT_PIN_ENTERED,
};


/*
 * Sensing info reported by the stubbed stack. Defined after RawAddress and
 * OHOS::bluetooth::Uuid since it embeds them by value.
 */
struct BtSensingInfo {
    uint8_t sensingState;
    uint8_t eventType;
    uint32_t timestamp;
    RawAddress addr;
    OHOS::bluetooth::Uuid uuid;
    uint32_t resourceId;
    uint16_t interval;
};

/* Stream accessors of the removed stack layer packet.h, reading big-endian
 * fields from a byte stream and advancing the cursor. */
#define STREAM_TO_UINT8(p, s) \
    { (p) = static_cast<uint8_t>(*(s)); (s)++; }
#define STREAM_TO_INT8(p, s) \
    { (p) = static_cast<int8_t>(*(s)); (s)++; }
#define STREAM_TO_UINT16(p, s) \
    { (p) = static_cast<uint16_t>(static_cast<uint16_t>(*(s)) << 8 | static_cast<uint16_t>(*((s) + 1))); (s) += 2; }

#endif  // BT_TYPES_H
