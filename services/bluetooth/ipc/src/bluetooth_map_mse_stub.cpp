/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_map_mse_stub"
#endif

#include "bluetooth_map_mse_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothMapMseInterfaceCode::code, {&BluetoothMapMseStub::func, perm}

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothMapMseStub::MapMseStubFuncPerm> BluetoothMapMseStub::memberFuncMap_ = {
    {STUB_FUNC(MSE_GET_DEVICE_STATE, GetDeviceStateInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(MSE_GET_DEVICES_BY_STATES, GetDevicesByStatesInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(MSE_DISCONNECT, DisconnectInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(MSE_SET_CONNECTION_STRATEGY, SetConnectionStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(MSE_GET_CONNECTION_STRATEGY, GetConnectionStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(MSE_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(MSE_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(MSE_SET_ACCESS_AUTHORIZATION, SetMessageAccessAuthorizationInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(MSE_GET_ACCESS_AUTHORIZATION, GetMessageAccessAuthorizationInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothMapMseStub::BluetoothMapMseStub()
{}

BluetoothMapMseStub::~BluetoothMapMseStub()
{}

int32_t BluetoothMapMseStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothMapMse ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothMapMseStub);
};

int32_t BluetoothMapMseStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");

    int32_t state;
    int32_t ret = GetDeviceState(*device, state);

    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(state), BT_ERR_INTERNAL_ERROR, "reply write state failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> state;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32Vector(&state), BT_ERR_INTERNAL_ERROR, "Read state failed");

    std::vector<BluetoothRawAddress> rawAdds;
    int32_t ret = GetDevicesByStates(state, rawAdds);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(rawAdds.size()), BT_ERR_INTERNAL_ERROR, "reply write devices failed");

    // write devices
    for (auto rawAdd : rawAdds) {
        if (!reply.WriteParcelable(&rawAdd)) {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");

    int32_t ret = Disconnect(*device);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::SetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");
    int32_t strategy = data.ReadInt32();

    int32_t ret = SetConnectionStrategy(*device, strategy);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::GetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");

    int32_t strategy = 0;
    int32_t ret = GetConnectionStrategy(*device, strategy);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(strategy), BT_ERR_INTERNAL_ERROR, "reply write strategy failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::SetMessageAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");
    int32_t accessAuthorization = data.ReadInt32();

    int32_t ret = SetMessageAccessAuthorization(*device, accessAuthorization);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::GetMessageAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");

    int32_t accessAuthorization = 0;
    int32_t ret = GetMessageAccessAuthorization(*device, accessAuthorization);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(accessAuthorization),
        BT_ERR_INTERNAL_ERROR, "reply write accessAuthorization failed");
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothMapMseObserver> observer = OHOS::iface_cast<IBluetoothMapMseObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothMapMseObserver> observer = OHOS::iface_cast<IBluetoothMapMseObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);
    return BT_NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS