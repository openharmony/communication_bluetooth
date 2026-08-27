/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_gatt"
#endif

#include "gatt_service_base.h"

#include "gatt_api.h"
#include "log.h"
#include "securec.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
GattValue GattServiceBase::MoveToGattValue(std::unique_ptr<uint8_t[]> &value)
{
    std::shared_ptr<std::unique_ptr<uint8_t[]>> sharedPtr
        = std::make_shared<std::unique_ptr<uint8_t[]>>(std::move(value));
    return sharedPtr;
}

GattValue GattServiceBase::BuildGattValue(const uint8_t *value, size_t length)
{
    std::shared_ptr<std::unique_ptr<uint8_t[]>> sharedPtr =
        std::make_shared<std::unique_ptr<uint8_t[]>>(std::make_unique<uint8_t[]>(length));
    (void)memcpy_s(sharedPtr->get(), length, value, length);
    return sharedPtr;
}

const std::vector<std::pair<int, int>> GattServiceBase::g_gattStatusCode = {
    { GATT_SUCCESS,                      STACK::GATT_SUCCESS },
    { INVALID_HANDLE,                    STACK::GATT_INVALID_HANDLE },
    { READ_NOT_PERMITTED,                STACK::GATT_READ_NOT_PERMIT },
    { WRITE_NOT_PERMITTED,               STACK::GATT_WRITE_NOT_PERMIT },
    { INVALID_PDU,                       STACK::GATT_INVALID_PDU },
    { INSUFFICIENT_AUTHENTICATION,       STACK::GATT_INSUF_AUTHENTICATION },
    { REQUEST_NOT_SUPPORT,               STACK::GATT_REQ_NOT_SUPPORTED },
    { INVALID_OFFSET,                    STACK::GATT_INVALID_OFFSET },
    { INSUFFICIENT_AUTHORIZATION,        STACK::GATT_INSUF_AUTHORIZATION },
    { PREPARE_QUEUE_FULL,                STACK::GATT_PREPARE_Q_FULL },
    { HANDLE_NOT_FOUND,                  STACK::GATT_NOT_FOUND },
    { ATTRIBUTE_NOT_LONG,                STACK::GATT_NOT_LONG },
    { INSUFFICIENT_ENCRYPTION_KEY_SIZE,  STACK::GATT_INSUF_KEY_SIZE },
    { INVALID_ATTRIBUTE_VALUE_LENGTH,    STACK::GATT_INVALID_ATTR_LEN },
    { ERR_UNLIKELY,                      STACK::GATT_ERR_UNLIKELY },
    { INSUFFICIENT_ENCRYPTION,           STACK::GATT_INSUF_ENCRYPTION },
    { UNSUPPORT_GRP_TYPE,                STACK::GATT_UNSUPPORT_GRP_TYPE },
    { INSUF_RESOURCE,                    STACK::GATT_INSUF_RESOURCE },
    { DATABASE_OUT_OF_SYNC,              STACK::GATT_DATABASE_OUT_OF_SYNC },
    { VALUE_NOT_ALLOWED,                 STACK::GATT_VALUE_NOT_ALLOWED },
    { INVALID_PARAMETER,                 STACK::GATT_ILLEGAL_PARAMETER },
    { TOO_SHORT,                         STACK::GATT_TOO_SHORT },
    { NO_RESOURCES,                      STACK::GATT_NO_RESOURCES },
    { INTERNAL_ERROR,                    STACK::GATT_INTERNAL_ERROR },
    { WRONG_STATE,                       STACK::GATT_WRONG_STATE },
    { DB_FULL,                           STACK::GATT_DB_FULL },
    { REMOTE_DEVICE_BUSY,                STACK::GATT_BUSY },
    { REMOTE_DEVICE_ERROR,               STACK::GATT_ERROR },
    { CMD_STARTED,                       STACK::GATT_CMD_STARTED },
    { REMOTE_DEVICE_PENDING,             STACK::GATT_PENDING},
    { AUTHENTICATION_FAILED,             STACK::GATT_AUTH_FAIL},
    { GATT_MORE_REASON,                  STACK::GATT_MORE},
    { INVALID_CFG,                       STACK::GATT_INVALID_CFG},
    { SERVICE_STARTED,                   STACK::GATT_SERVICE_STARTED},
    { ENCRYPED_NO_MITM,                  STACK::GATT_ENCRYPED_NO_MITM},
    { NOT_ENCRYPTED,                     STACK::GATT_NOT_ENCRYPTED},
    { GATT_CONGESTION,                   STACK::GATT_CONGESTED},
    { DUP_REG,                           STACK::GATT_DUP_REG},
    { ALREADY_OPEN,                      STACK::GATT_ALREADY_OPEN},
    { GATT_DIS_CANCEL,                   STACK::GATT_CANCEL},
    { CCC_CFG_ERR,                       STACK::GATT_CCC_CFG_ERR},
    { PRC_IN_PROGRESS,                   STACK::GATT_PRC_IN_PROGRESS},
    { OUT_OF_RANGE,                      STACK::GATT_OUT_OF_RANGE},
};

int GattServiceBase::GattStatusFromStack(int status)
{
    int ret = GATT_FAILURE;
    // statusCode bluedroid -> srv
    auto iter = g_gattStatusCode.begin();
    for (; iter != g_gattStatusCode.end(); iter++) {
        if (iter->second == status) {
            ret = iter->first;
            break;
        }
    }

    if (iter == g_gattStatusCode.end()) {
        HILOGE("Unsupported error code conversion, status: %{public}d", status);
    }
    return ret;
}

int GattServiceBase::GattStatusToStack(int status)
{
    int ret = STACK::GATT_REQ_NOT_SUPPORTED;
    auto iter = g_gattStatusCode.begin();
    for (; iter != g_gattStatusCode.end(); iter++) {
        if (iter->first == status) {
            ret = iter->second;
            break;
        }
    }

    if (iter == g_gattStatusCode.end()) {
        HILOGE("Unsupported error code conversion, status: %{public}d", status);
    }
    return ret;
}

const std::vector<std::pair<int, int>> GattServiceBase::g_gattTransport = {
    { GATT_TRANSPORT_TYPE_AUTO,     GATT_TRANSPORT_AUTO },
    { GATT_TRANSPORT_TYPE_LE,       GATT_TRANSPORT_LE },
    { GATT_TRANSPORT_TYPE_CLASSIC,  GATT_TRANSPORT_BR_EDR },
};

int GattServiceBase::GattTransportFromStack(int transport)
{
    int ret = GATT_TRANSPORT_TYPE_AUTO;
    auto iter = g_gattTransport.begin();
    for (; iter != g_gattTransport.end(); iter++) {
        if (iter->second == transport) {
            ret = iter->first;
            break;
        }
    }
    return ret;
}

int GattServiceBase::GattTransportToStack(int transport)
{
    int ret = GATT_TRANSPORT_INVALID;
    auto iter = g_gattTransport.begin();
    for (; iter != g_gattTransport.end(); iter++) {
        if (iter->first == transport) {
            ret = iter->second;
            break;
        }
    }
    return ret;
}

void GattServiceBase::ConvertDisconnectReason(int &convertReason, int reason, std::string &reasonMessage)
{
    switch (reason) {
        case tGATT_DISCONN_REASON::GATT_CONN_TIMEOUT:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_TIMEOUT);
            reasonMessage = GATT_DIS_MSG_0X08_CONN_TIMEOUT;
            break;
        case tGATT_DISCONN_REASON::GATT_CONN_TERMINATE_PEER_USER:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_TERMINATE_PEER_USER);
            reasonMessage = GATT_DIS_MSG_0X13_PEER_USER;
            break;
        // When local host call disconnect, the reason of disconnectCallback is GATT_CONN_OK.
        case tGATT_DISCONN_REASON::GATT_CONN_OK:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_TERMINATE_LOCAL_HOST);
            reasonMessage = GATT_DIS_MSG_0X16_LOCAL_HOST;
            break;
        case tGATT_DISCONN_REASON::GATT_CONN_TERMINATE_LOCAL_HOST:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_TERMINATE_LOCAL_HOST);
            reasonMessage = GATT_DIS_MSG_0X16_LOCAL_HOST;
            break;
        case tGATT_DISCONN_REASON::GATT_CONN_L2C_FAILURE:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
            reasonMessage = GATT_DIS_MSG_L2C_FAILURE;
            break;
        case tGATT_DISCONN_REASON::GATT_CONN_LMP_TIMEOUT:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
            reasonMessage = GATT_DIS_MSG_0X22_LMP_TIMEOUT;
            break;
        case tGATT_DISCONN_REASON::BTA_GATT_CONN_NONE:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
            reasonMessage = GATT_DIS_MSG_NO_CONN_TO_CANCEL;
            break;
        default:
            convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
            reasonMessage = GATT_DIS_MSG_CONN_UNKNOWN;
            break;
    }
}

}  // namespace bluetooth
}  // namespace OHOS
