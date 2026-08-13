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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_socket_stub"
#endif

#include "bluetooth_socket_stub.h"
#include "bluetooth_socket_observer_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothSocketInterfaceCode::code, {&BluetoothSocketStub::func, perm}

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothSocketStub::SocketStubFuncPerm> BluetoothSocketStub::memberFuncMap_ = {
    {STUB_FUNC(SOCKET_CONNECT, ConnectInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SOCKET_LISTEN, ListenInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SOCKET_UPDATE_COC_PARAMS, UpdateCocConnectionParamsInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(DEREGISTER_SERVER_OBSERVER, DeregisterServerObserverInner, nullptr)},
    {STUB_FUNC(REGISTER_CLIENT_OBSERVER, RegisterClientObserverInner, nullptr)},
    {STUB_FUNC(DEREGISTER_CLIENT_OBSERVER, DeregisterClientObserverInner, nullptr)},
    {STUB_FUNC(SOCKET_IS_ALLOW_CONNECT, IsAllowSocketConnectInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothSocketStub::BluetoothSocketStub()
{}

BluetoothSocketStub::~BluetoothSocketStub()
{}

int32_t BluetoothSocketStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothSocket ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothSocketStub);
}

int32_t BluetoothSocketStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("reply writing failed.");
        return BT_ERR_INVALID_PARAM;
    }
    ConnectSocketParam param {
        .addr = addr,
        .uuid = *uuid,
        .securityFlag = data.ReadInt32(),
        .type = data.ReadInt32(),
        .psm = data.ReadInt32()
    };

    int fd = -1;
    int ret = Connect(param, fd);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply WriteInt32 failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (ret == BT_NO_ERROR) {
        if (!reply.WriteFileDescriptor(fd)) {
            HILOGE("reply write file descriptor failed.");
            close(fd);
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }
    close(fd);
    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::ListenInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name = data.ReadString();
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("reply writing failed.");
        return BT_ERR_INVALID_PARAM;
    }

    ListenSocketParam param {
        .name = name,
        .uuid = *uuid,
        .securityFlag = data.ReadInt32(),
        .type = data.ReadInt32(),
        .psm = data.ReadInt32()
    };

    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothServerSocketObserver> observer = OHOS::iface_cast<IBluetoothServerSocketObserver>(remote);
    param.observer = observer;
    if (observer == nullptr) {
        HILOGE("observer nullptr");
    }

    int fd = -1;
    int ret = Listen(param, fd);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply WriteInt32 failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (ret == BT_NO_ERROR) {
        if (!reply.WriteFileDescriptor(fd)) {
            close(fd);
            HILOGD("reply write file descriptor failed.");
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }
    close(fd);
    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::DeregisterServerObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IBluetoothServerSocketObserver> observer =
        OHOS::iface_cast<IBluetoothServerSocketObserver>(data.ReadRemoteObject());
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterServerObserver(observer);

    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::UpdateCocConnectionParamsInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothSocketCocInfo> info(data.ReadParcelable<BluetoothSocketCocInfo>());
    if (!info) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    UpdateCocConnectionParams(*info);

    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::RegisterClientObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("reply writing failed.");
        return BT_ERR_INVALID_PARAM;
    }
    sptr<IBluetoothClientSocketObserver> observer =
        OHOS::iface_cast<IBluetoothClientSocketObserver>(data.ReadRemoteObject());
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    int result = RegisterClientObserver(*addr, *uuid, observer);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::DeregisterClientObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        HILOGE(" addr read fail");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("reply writing failed.");
        return BT_ERR_INVALID_PARAM;
    }
    sptr<IBluetoothClientSocketObserver> observer =
        OHOS::iface_cast<IBluetoothClientSocketObserver>(data.ReadRemoteObject());
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    int result = DeregisterClientObserver(*addr, *uuid, observer);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothSocketStub::IsAllowSocketConnectInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t socketType = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(socketType), BT_ERR_IPC_TRANS_FAILED, "Read socketType failed");
    std::string addr;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(addr), BT_ERR_IPC_TRANS_FAILED, "Read addr failed");
    bool isAllowed = true;
    int result = IsAllowSocketConnect(socketType, addr, isAllowed);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteBool(isAllowed), BT_ERR_IPC_TRANS_FAILED, "reply writing failed");
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS