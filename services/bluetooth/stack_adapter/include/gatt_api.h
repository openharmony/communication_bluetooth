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
 * Stub of the removed stack layer GATT API header. Only the constants and
 * type names referenced by the service layer are provided.
 */

#ifndef GATT_API_H
#define GATT_API_H

#include <cstdint>
#include <set>

#include "bt_types.h"
#include "bt_gatt.h"

typedef uint16_t GattStatus;

/* GATT status codes of the removed stack layer (GattStatus in bluedroid
 * stack/include/gatt_api.h). A plain enum instead of macros: the service
 * layer references these through the STACK:: alias, and a #define would
 * expand STACK::GATT_XXX into an illegal "::0x0006" token. */
enum {
    GATT_SUCCESS = 0x0000,
    GATT_INVALID_HANDLE = 0x0001,
    GATT_READ_NOT_PERMIT = 0x0002,
    GATT_WRITE_NOT_PERMIT = 0x0003,
    GATT_INVALID_PDU = 0x0004,
    GATT_INSUF_AUTHENTICATION = 0x0005,
    GATT_REQ_NOT_SUPPORTED = 0x0006,
    GATT_INVALID_OFFSET = 0x0007,
    GATT_INSUF_AUTHORIZATION = 0x0008,
    GATT_PREPARE_Q_FULL = 0x0009,
    GATT_NOT_FOUND = 0x000a,
    GATT_NOT_LONG = 0x000b,
    GATT_INSUF_KEY_SIZE = 0x000c,
    GATT_INVALID_ATTR_LEN = 0x000d,
    GATT_ERR_UNLIKELY = 0x000e,
    GATT_INSUF_ENCRYPTION = 0x000f,
    GATT_UNSUPPORT_GRP_TYPE = 0x0010,
    GATT_INSUF_RESOURCE = 0x0011,
    GATT_DATABASE_OUT_OF_SYNC = 0x0012,
    GATT_VALUE_NOT_ALLOWED = 0x0013,
    GATT_NO_RESOURCES = 0x0080,
    GATT_INTERNAL_ERROR = 0x0081,
    GATT_WRONG_STATE = 0x0082,
    GATT_DB_FULL = 0x0083,
    GATT_BUSY = 0x0084,
    GATT_ERROR = 0x0085,
    GATT_CMD_STARTED = 0x0086,
    GATT_ILLEGAL_PARAMETER = 0x0087,
    GATT_PENDING = 0x0088,
    GATT_AUTH_FAIL = 0x0089,
    GATT_INVALID_CFG = 0x008b,
    GATT_SERVICE_STARTED = 0x008c,
    GATT_ENCRYPED_NO_MITM = 0x008d,
    GATT_NOT_ENCRYPTED = 0x008e,
    GATT_CONGESTED = 0x008f,
    GATT_DUP_REG = 0x0090,
    GATT_ALREADY_OPEN = 0x0091,
    GATT_CANCEL = 0x0092,
    /* Service-layer extensions of the status table (gatt_service_base.cpp),
     * not part of the bluedroid GattStatus set. */
    GATT_MORE = 0x0098,
    GATT_TOO_SHORT = 0x00a0,
    GATT_CCC_CFG_ERR = 0x00fd,
    GATT_PRC_IN_PROGRESS = 0x00fe,
    GATT_OUT_OF_RANGE = 0x00ff,
};

/* Invalid connection id, see gatt_client_application.h. */
#define GATT_INVALID_CONN_ID 0xFFFF

/* Authentication requirements of a remote operation, GattAuthReq in
 * bluedroid stack/include/gatt_api.h. */
enum GattAuthReq {
    GATT_AUTH_REQ_NONE = 0,
    GATT_AUTH_REQ_NO_MITM = 1,
    GATT_AUTH_REQ_MITM = 2,
    GATT_AUTH_REQ_SIGNED_NO_MITM = 3,
    GATT_AUTH_REQ_SIGNED_MITM = 4,
};

/* Write operation type, GattWriteType in bluedroid. */
enum GattWriteType {
    GATT_WRITE_NO_RSP = 1,
    GATT_WRITE = 2,
    GATT_WRITE_PREPARE = 3,
};

/* GATT server role of a remote device; MASTER/SLAVE/INVALID live in
 * frameworks bt_def.h. */
constexpr uint8_t GATT_ROLE_SECONDARY = 0x02;

/* Client characteristic configuration descriptor value bits */
#define GATT_CH_CLIENT_CONFIG_NONE 0x0000
#define GATT_CH_CLIENT_CONFIG_NOTIFY 0x0001
#define GATT_CH_CLIENT_CONFIG_INDICATE 0x0002

struct GattValue {
    GattStatus status;
    uint16_t handle;
};

enum GattAttrType {
    GATTS_INVALID_ATTR = 0,
    GATTS_INCL_SRVC_ATTR,
    GATTS_CHAR_ATTR,
    GATTS_CHAR_DESCR_ATTR,
};

/* Connection termination reason reported by the disconnect callback
 * (bluedroid stack/include/gatt_api.h); HCI error code values. */
enum GattDisconnReason : uint16_t {
    GATT_CONN_OK = 0,
    GATT_CONN_L2C_FAILURE = 1,        /* general L2cap failure */
    GATT_CONN_TIMEOUT = 0x08,         /* connection timeout */
    GATT_CONN_TERMINATE_PEER_USER = 0x13,      /* terminated by peer user */
    GATT_CONN_TERMINATE_LOCAL_HOST = 0x16,     /* terminated by local host */
    GATT_CONN_LMP_TIMEOUT = 0x22,     /* LMP response timeout */
    GATT_CONN_FAILED_ESTABLISHMENT = 0x3e,     /* connection failed to establish */
    GATT_CONN_TERMINATED_POWER_OFF = 0x05,     /* remote power off */
    BTA_GATT_CONN_NONE = 0x0101,      /* no connection to cancel */
};

enum GattPerm {
    GATT_PERM_NONE = 0x0000,
    GATT_PERM_READ = 0x0001,
    GATT_PERM_READ_ENCRYPTED = 0x0002,
    GATT_PERM_READ_ENC_MITM = 0x0004,
    GATT_PERM_WRITE = 0x0010,
    GATT_PERM_WRITE_ENCRYPTED = 0x0020,
    GATT_PERM_WRITE_ENC_MITM = 0x0040,
    GATT_PERM_WRITE_SIGNED = 0x0080,
    GATT_PERM_WRITE_SIGNED_MITM = 0x0100,
};

enum GattCharProp {
    GATT_CHAR_PROP_BIT_NONE = 0x00,
    GATT_CHAR_PROP_BIT_BROADCAST = 0x01,
    GATT_CHAR_PROP_BIT_READ = 0x02,
    GATT_CHAR_PROP_BIT_UNSIGNED_WRITE = 0x04,
    GATT_CHAR_PROP_BIT_WRITE = 0x08,
    GATT_CHAR_PROP_BIT_NOTIFY = 0x10,
    GATT_CHAR_PROP_BIT_INDICATE = 0x20,
    GATT_CHAR_PROP_BIT_AUTH_WRITE = 0x40,
    GATT_CHAR_PROP_BIT_EXT_PROP = 0x80,
};

/* GATT over BR/EDR and LE transports are both available (in the original
 * stack these were separate interface families; stubbed as one). */

#endif  // GATT_API_H
