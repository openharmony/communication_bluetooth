/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "bluetooth_socket_proxy.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothSocketProxy::Connect(std::string &addr, bluetooth::Uuid &uuid, int32_t securityFlag, int32_t type)
{
    HILOGI("BluetoothSocketProxy::Connect starts");
    MessageParcel data;

    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("BluetoothSocketProxy::Connect WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(addr)) {
        HILOGE("BluetoothSocketProxy::Connect write addr error");
        return ERROR;
    }

    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("BluetoothSocketProxy::Connect write uuid error");
        return ERROR;
    }

    if (!data.WriteInt32(securityFlag)) {
        HILOGE("BluetoothSocketProxy::Connect write securityFlag error");
        return ERROR;
    }

    if (!data.WriteInt32(type)) {
        HILOGE("BluetoothSocketProxy::Connect write type error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothSocketProxy::Code::SOCKET_CONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothSocketProxy::Connect done fail, error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadFileDescriptor();
}

int BluetoothSocketProxy::Listen(std::string &name, bluetooth::Uuid &uuid, int32_t securityFlag, int32_t type)
{
    HILOGI("Listen starts");
    MessageParcel data;

    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_INVALID_SOCKET_FD;
    }
    if (!data.WriteString(name)) {
        HILOGE("write name error");
        return BT_INVALID_SOCKET_FD;
    }
    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("write uuid error");
        return BT_INVALID_SOCKET_FD;
    }

    if (!data.WriteInt32(securityFlag)) {
        HILOGE("write securityFlag error");
        return BT_INVALID_SOCKET_FD;
    }

    if (!data.WriteInt32(type)) {
        HILOGE("write type error");
        return BT_INVALID_SOCKET_FD;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothSocketProxy::Code::SOCKET_LISTEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("Listen done fail, error: %{public}d", error);
        return BT_INVALID_SOCKET_FD;
    }

    return reply.ReadFileDescriptor();
}
}  // namespace Bluetooth
}  // namespace OHOS