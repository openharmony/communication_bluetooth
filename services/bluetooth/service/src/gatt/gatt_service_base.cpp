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
    { GATT_SUCCESS,                      BLUEDROID::GATT_SUCCESS },
    { INVALID_HANDLE,                    BLUEDROID::GATT_INVALID_HANDLE },
    { READ_NOT_PERMITTED,                BLUEDROID::GATT_READ_NOT_PERMIT },
    { WRITE_NOT_PERMITTED,               BLUEDROID::GATT_WRITE_NOT_PERMIT },
    { INVALID_PDU,                       BLUEDROID::GATT_INVALID_PDU },
    { INSUFFICIENT_AUTHENTICATION,       BLUEDROID::GATT_INSUF_AUTHENTICATION },
    { REQUEST_NOT_SUPPORT,               BLUEDROID::GATT_REQ_NOT_SUPPORTED },
    { INVALID_OFFSET,                    BLUEDROID::GATT_INVALID_OFFSET },
    { INSUFFICIENT_AUTHORIZATION,        BLUEDROID::GATT_INSUF_AUTHORIZATION },
    { PREPARE_QUEUE_FULL,                BLUEDROID::GATT_PREPARE_Q_FULL },
    { HANDLE_NOT_FOUND,                  BLUEDROID::GATT_NOT_FOUND },
    { ATTRIBUTE_NOT_LONG,                BLUEDROID::GATT_NOT_LONG },
    { INSUFFICIENT_ENCRYPTION_KEY_SIZE,  BLUEDROID::GATT_INSUF_KEY_SIZE },
    { INVALID_ATTRIBUTE_VALUE_LENGTH,    BLUEDROID::GATT_INVALID_ATTR_LEN },
    { ERR_UNLIKELY,                      BLUEDROID::GATT_ERR_UNLIKELY },
    { INSUFFICIENT_ENCRYPTION,           BLUEDROID::GATT_INSUF_ENCRYPTION },
    { UNSUPPORT_GRP_TYPE,                BLUEDROID::GATT_UNSUPPORT_GRP_TYPE },
    { INSUF_RESOURCE,                    BLUEDROID::GATT_INSUF_RESOURCE },
    { DATABASE_OUT_OF_SYNC,              BLUEDROID::GATT_DATABASE_OUT_OF_SYNC },
    { VALUE_NOT_ALLOWED,                 BLUEDROID::GATT_VALUE_NOT_ALLOWED },
    { INVALID_PARAMETER,                 BLUEDROID::GATT_ILLEGAL_PARAMETER },
    { TOO_SHORT,                         BLUEDROID::GATT_TOO_SHORT },
    { NO_RESOURCES,                      BLUEDROID::GATT_NO_RESOURCES },
    { INTERNAL_ERROR,                    BLUEDROID::GATT_INTERNAL_ERROR },
    { WRONG_STATE,                       BLUEDROID::GATT_WRONG_STATE },
    { DB_FULL,                           BLUEDROID::GATT_DB_FULL },
    { REMOTE_DEVICE_BUSY,                BLUEDROID::GATT_BUSY },
    { REMOTE_DEVICE_ERROR,               BLUEDROID::GATT_ERROR },
    { CMD_STARTED,                       BLUEDROID::GATT_CMD_STARTED },
    { REMOTE_DEVICE_PENDING,             BLUEDROID::GATT_PENDING},
    { AUTHENTICATION_FAILED,             BLUEDROID::GATT_AUTH_FAIL},
    { GATT_MORE_REASON,                  BLUEDROID::GATT_MORE},
    { INVALID_CFG,                       BLUEDROID::GATT_INVALID_CFG},
    { SERVICE_STARTED,                   BLUEDROID::GATT_SERVICE_STARTED},
    { ENCRYPED_NO_MITM,                  BLUEDROID::GATT_ENCRYPED_NO_MITM},
    { NOT_ENCRYPTED,                     BLUEDROID::GATT_NOT_ENCRYPTED},
    { GATT_CONGESTION,                   BLUEDROID::GATT_CONGESTED},
    { DUP_REG,                           BLUEDROID::GATT_DUP_REG},
    { ALREADY_OPEN,                      BLUEDROID::GATT_ALREADY_OPEN},
    { GATT_DIS_CANCEL,                   BLUEDROID::GATT_CANCEL},
    { CCC_CFG_ERR,                       BLUEDROID::GATT_CCC_CFG_ERR},
    { PRC_IN_PROGRESS,                   BLUEDROID::GATT_PRC_IN_PROGRESS},
    { OUT_OF_RANGE,                      BLUEDROID::GATT_OUT_OF_RANGE},
};

int GattServiceBase::GattStatusFromBluedroid(int status)
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

int GattServiceBase::GattStatusToBluedroid(int status)
{
    int ret = BLUEDROID::GATT_REQ_NOT_SUPPORTED;
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

int GattServiceBase::GattTransportFromBluedroid(int transport)
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

int GattServiceBase::GattTransportToBluedroid(int transport)
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
