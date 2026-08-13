/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_pan_stub"
#endif

#include "bluetooth_pan_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) \
    static_cast<uint32_t>(BluetoothPanInterfaceCode::code), {&BluetoothPanStub::func, perm} \

namespace OHOS {
namespace Bluetooth {
const int32_t PAN_DEVICES_STATES_MAX_NUMS = 0xFF;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothPanStub::BluetoothPanStubFuncPerm> BluetoothPanStub::memberFuncMap_ = {
    {STUB_FUNC(COMMAND_CONNECT, ConnectInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_DISCONNECT, DisconnectInner,
        CHECK_PERM(true, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(COMMAND_GET_DEVICE_STATE, GetDeviceStateInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(COMMAND_GET_DEVICES_BY_STATES, GetDevicesByStatesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(COMMAND_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(COMMAND_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(COMMAND_SET_TETHERING, SetTetheringInner,
        CHECK_PERM(true, {DISCOVER_BLUETOOTH}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_IS_TETHERING_ON, IsTetheringOnInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_SET_CONNECT_STRATEGY, SetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_CONNECT_STRATEGY, GetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothPanStub::BluetoothPanStub()
{}

BluetoothPanStub::~BluetoothPanStub()
{}

int BluetoothPanStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothPan ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothPanStub);
}

int32_t BluetoothPanStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (device == nullptr) {
        HILOGE("device is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t errCode = Connect(*device);
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t errCode = Disconnect(*device);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t state;
    int32_t errCode = GetDeviceState(*device, state);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (errCode != BT_NO_ERROR) {
        HILOGE("internal error.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // write state
    if (!reply.WriteInt32(state)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> states = {};
    int32_t stateSize = data.ReadInt32();
    if (static_cast<uint32_t>(stateSize) > PAN_DEVICES_STATES_MAX_NUMS) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < stateSize; i++) {
        int32_t state = data.ReadInt32();
        states.push_back(state);
    }
    std::vector<BluetoothRawAddress> rawAdds;
    int32_t errCode = GetDevicesByStates(states, rawAdds);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (errCode != BT_NO_ERROR) {
        HILOGE("internal error.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // write size
    if (!reply.WriteInt32(rawAdds.size())) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // write devices
    for (auto rawAdd : rawAdds) {
        if (!reply.WriteParcelable(&rawAdd)) {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothPanStub::RegisterObserverInner");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothPanObserver> observer = OHOS::iface_cast<IBluetoothPanObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, BT_ERR_INTERNAL_ERROR, "observer is nullptr");
    RegisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothPanStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothPanStub::DeregisterObserverInner");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothPanObserver> observer = OHOS::iface_cast<IBluetoothPanObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, BT_ERR_INTERNAL_ERROR, "observer is nullptr");
    DeregisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothPanStub::SetTetheringInner(MessageParcel &data, MessageParcel &reply)
{
    const bool value = data.ReadBool();
    int32_t errCode = SetTethering(value);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::IsTetheringOnInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = false;
    int32_t errCode = IsTetheringOn(result);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (errCode != BT_NO_ERROR) {
        HILOGE("internal error.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // write result
    if (!reply.WriteBool(result)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (device == nullptr) {
        HILOGE("Read device address failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int strategy = data.ReadInt32();
    int result = SetConnectStrategy(*device, strategy);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothPanStub::GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("Read device address failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int strategy = 0;
    int result = GetConnectStrategy(*device, strategy);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing res failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!reply.WriteInt32(strategy)) {
        HILOGE("reply writing strategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS