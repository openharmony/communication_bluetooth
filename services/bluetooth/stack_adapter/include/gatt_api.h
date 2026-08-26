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

#include "bt_types.h"

typedef uint16_t tGATT_STATUS;

/* constexpr instead of a macro: the service layer declares an enumerator
 * named GATT_SUCCESS (bluetooth_def.h) and a macro would clobber it. */
static constexpr tGATT_STATUS GATT_SUCCESS = 0x0000;
#define GATT_INVALID_HANDLE 0x0001
#define GATT_READ_NOT_PERMIT 0x0002
#define GATT_WRITE_NOT_PERMIT 0x0003
#define GATT_INVALID_PDU 0x0004
#define GATT_INSUF_AUTHENTICATION 0x0005
#define GATT_REQ_NOT_SUPPORTED 0x0006
#define GATT_INVALID_OFFSET 0x0007
#define GATT_INSUF_AUTHORIZATION 0x0008
#define GATT_PREPARE_Q_FULL 0x0009
#define GATT_NOT_FOUND 0x000a
#define GATT_NOT_LONG 0x000b
#define GATT_INSUF_KEY_SIZE 0x000c
#define GATT_INVALID_ATTR_LEN 0x000d
#define GATT_ERR_UNLIKELY 0x000e
#define GATT_INSUF_ENCRYPTION 0x000f
#define GATT_UNSUPPORT_GRP_TYPE 0x0010
#define GATT_INSUF_RESOURCE 0x0011
#define GATT_ILLEGAL_PARAMETER 0x0087
#define GATT_NO_RESOURCES 0x0088
#define GATT_INTERNAL_ERROR 0x0089
#define GATT_WRONG_STATE 0x008a
#define GATT_DB_FULL 0x008b
#define GATT_BUSY 0x008c
#define GATT_ERROR 0x008d
#define GATT_CMD_STARTED 0x008e
#define GATT_ILLEGAL_PARAMETER_2 0x008f
#define GATT_PENDING 0x0090
#define GATT_AUTH_FAIL 0x0091
#define GATT_MORE 0x0092
#define GATT_INVALID_CFG 0x0093
#define GATT_SERVICE_STARTED 0x0094
#define GATT_ENCRYPED_NO_MITM 0x0095
#define GATT_NOT_ENCRYPTED 0x0096
#define GATT_CONGESTED 0x0097

/* Client characteristic configuration descriptor value bits */
#define GATT_CH_CLIENT_CONFIG_NONE 0x0000
#define GATT_CH_CLIENT_CONFIG_NOTIFY 0x0001
#define GATT_CH_CLIENT_CONFIG_INDICATE 0x0002

typedef struct {
    tGATT_STATUS status;
    uint16_t handle;
} tGATT_VALUE;

typedef enum {
    GATTS_INVALID_ATTR = 0,
    GATTS_INCL_SRVC_ATTR,
    GATTS_CHAR_ATTR,
    GATTS_CHAR_DESCR_ATTR,
} tGATT_ATTR_TYPE;

typedef enum {
    GATT_CLOSE_GRP_ATTR = 0,
    GATT_REMOVE_ATTR,
    GATT_CHAR_ATTR_READ,
    GATT_CHAR_ATTR_WRITE,
    GATT_CHAR_ATTR_UPDATE,
    GATT_DELETE_ATTR,
} tGATT_DISCONN_REASON;

typedef enum {
    GATT_PERM_NONE = 0x0000,
    GATT_PERM_READ = 0x0001,
    GATT_PERM_READ_ENCRYPTED = 0x0002,
    GATT_PERM_READ_ENC_MITM = 0x0004,
    GATT_PERM_WRITE = 0x0010,
    GATT_PERM_WRITE_ENCRYPTED = 0x0020,
    GATT_PERM_WRITE_ENC_MITM = 0x0040,
    GATT_PERM_WRITE_SIGNED = 0x0080,
    GATT_PERM_WRITE_SIGNED_MITM = 0x0100,
} tGATT_PERM;

typedef enum {
    GATT_CHAR_PROP_BIT_NONE = 0x00,
    GATT_CHAR_PROP_BIT_BROADCAST = 0x01,
    GATT_CHAR_PROP_BIT_READ = 0x02,
    GATT_CHAR_PROP_BIT_UNSIGNED_WRITE = 0x04,
    GATT_CHAR_PROP_BIT_WRITE = 0x08,
    GATT_CHAR_PROP_BIT_NOTIFY = 0x10,
    GATT_CHAR_PROP_BIT_INDICATE = 0x20,
    GATT_CHAR_PROP_BIT_AUTH_WRITE = 0x40,
    GATT_CHAR_PROP_BIT_EXT_PROP = 0x80,
} tGATT_CHAR_PROP;

/* GATT over BR/EDR and LE transports are both available (in the original
 * stack these were separate interface families; stubbed as one). */

#endif  // GATT_API_H
