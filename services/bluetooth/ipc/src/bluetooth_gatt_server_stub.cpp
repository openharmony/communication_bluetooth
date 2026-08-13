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
#define LOG_TAG "bt_ipc_gatt_server_stub"
#endif

#include "bluetooth_gatt_server_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothGattServerInterfaceCode::code, {&BluetoothGattServerStub::func, perm}

namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothGattServerStub::GattServerStubFuncPerm> BluetoothGattServerStub::memberFuncMap_ = {
    {STUB_FUNC(GATT_SERVER_ADD_SERVICE, AddServiceInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_CLEAR_SERVICES, ClearServicesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_CONNECT, ConnectInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_CANCEL_CONNECTION, CancelConnectionInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_REGISTER, RegisterApplicationInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_DEREGISTER, DeregisterApplicationInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_NOTIFY_CLIENT, NotifyClientInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_REMOVE_SERVICE, RemoveServiceInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_RESPOND_CHARACTERISTIC_READ, RespondCharacteristicReadInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE, RespondCharacteristicWriteInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_RESPOND_DESCRIPTOR_READ, RespondDescriptorReadInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_RESPOND_DESCRIPTOR_WRITE, RespondDescriptorWriteInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_GET_CONNECTED_STATE, GetConnectedStateInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_SET_PHY, SetPhyInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GATT_SERVER_READ_PHY, ReadPhyInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
};

BluetoothGattServerStub::BluetoothGattServerStub()
{}

BluetoothGattServerStub::~BluetoothGattServerStub()
{}

int32_t BluetoothGattServerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothGattServer ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothGattServerStub);
}

int32_t BluetoothGattServerStub::AddServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appID = data.ReadInt32();
    std::shared_ptr<BluetoothGattService> service(data.ReadParcelable<BluetoothGattService>());
    if (!service) {
        return TRANSACTION_ERR;
    }
    int result = AddService(appID, service.get());
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::ClearServicesInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    int result = ClearServices(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read parcelable BluetoothGattDevice failed");
    bool isDirect = data.ReadBool();
    return Connect(appId, *device, isDirect);
}

int32_t BluetoothGattServerStub::CancelConnectionInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read parcelable BluetoothGattDevice failed");
    return CancelConnection(appId, *device);
}

int32_t BluetoothGattServerStub::RegisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothGattServerCallback> callback = OHOS::iface_cast<IBluetoothGattServerCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    int result = RegisterApplication(callback);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::DeregisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    int result = DeregisterApplication(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::NotifyClientInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    bool needConfirm = data.ReadBool();
    int result = NotifyClient(*device, characteristic.get(), needConfirm);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::RemoveServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattService> service(data.ReadParcelable<BluetoothGattService>());
    if (!service) {
        return TRANSACTION_ERR;
    }
    int result = RemoveService(appId, *service);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::RespondCharacteristicReadInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    int ret1 = data.ReadInt32();
    int result = RespondCharacteristicRead(*device, characteristic.get(), ret1);
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::RespondCharacteristicWriteInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    int ret1 = data.ReadInt32();
    int result = RespondCharacteristicWrite(*device, *characteristic, ret1);
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::RespondDescriptorReadInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    int ret1 = data.ReadInt32();
    int result = RespondDescriptorRead(*device, descriptor.get(), ret1);
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::RespondDescriptorWriteInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattDevice> device(data.ReadParcelable<BluetoothGattDevice>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    int ret1 = data.ReadInt32();
    int result = RespondDescriptorWrite(*device, *descriptor, ret1);
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::GetConnectedStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int state = -1;
    int res = GetConnectedState(deviceId, state);
    if (!reply.WriteInt32(res)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (res != BT_NO_ERROR) {
        return res;
    }
    if (!reply.WriteInt32(state)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattServerStub::SetPhyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::string deviceId = data.ReadString();
    int txPhy = data.ReadInt32();
    int rxPhy = data.ReadInt32();
    int phyOptions = data.ReadInt32();
    int result = SetPhy(appId, deviceId, txPhy, rxPhy, phyOptions);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}
 
int32_t BluetoothGattServerStub::ReadPhyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::string deviceId = data.ReadString();
    int result = ReadPhy(appId, deviceId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
