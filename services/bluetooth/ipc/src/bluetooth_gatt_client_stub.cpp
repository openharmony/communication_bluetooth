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
#define LOG_TAG "bt_ipc_gatt_client_stub"
#endif

#include "bluetooth_gatt_client_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "common_util.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothGattClientInterfaceCode::code, {&BluetoothGattClientStub::func, perm}

namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothGattClientStub::GattClientStubFuncPerm> BluetoothGattClientStub::memberFuncMap_ = {
    {STUB_FUNC(BT_GATT_CLIENT_REGISTER_APP, RegisterApplicationInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_DEREGISTER_APP, DeregisterApplicationInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_CONNECT, ConnectInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_DIS_CONNECT, DisconnectInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_DISCOVERY_SERVICES, DiscoveryServicesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_READ_CHARACTERISTIC, ReadCharacteristicInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_READ_CHARACTERISTIC_BY_UUID, ReadCharacteristicByUuidInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_WRITE_CHARACTERISTIC, WriteCharacteristicInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_SIGNED_WRITE_CHARACTERISTIC, SignedWriteCharacteristicInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_READ_DESCRIPTOR, ReadDescriptorInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_WRITE_DESCRIPTOR, WriteDescriptorInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_REQUEST_EXCHANGE_MTU, RequestExchangeMtuInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_GET_ALL_DEVICE, GetAllDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GATT_CLIENT_REQUEST_CONNECTION_PRIORITY, RequestConnectionPriorityInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_GET_SERVICES, GetServicesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_REQUEST_FASTEST_CONNECTION, RequestFastestConnInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GATT_CLIENT_READ_REMOTE_RSSI_VALUE, ReadRemoteRssiValueInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_REQUEST_NOTIFICATION, RequestNotificationInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_GET_CONNECTED_STATE, GetConnectedStateInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_SET_PHY, SetPhyInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GATT_CLIENT_READ_PHY, ReadPhyInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
};

BluetoothGattClientStub::BluetoothGattClientStub()
{}

BluetoothGattClientStub::~BluetoothGattClientStub()
{}

int32_t BluetoothGattClientStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothGattClient ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothGattClientStub);
}

int32_t BluetoothGattClientStub::RegisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothGattClientCallback> callback = OHOS::iface_cast<IBluetoothGattClientCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(addr->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t transport = data.ReadInt32();
    int appId = 0;
    int result = RegisterApplication(callback, *addr, transport, appId);
    bool resultRet = reply.WriteInt32(result);
    bool appIdRet = reply.WriteInt32(appId);
    if (!(resultRet && appIdRet)) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::DeregisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int result = DeregisterApplication(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    bool autoConnect = data.ReadBool();
    int result = Connect(appId, autoConnect);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int result = Disconnect(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::DiscoveryServicesInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int result = DiscoveryServices(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::ReadCharacteristicInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    int result = ReadCharacteristic(appId, *characteristic);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::ReadCharacteristicByUuidInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientStub::ReadUsingCharacteristicUuidInner starts");
    int32_t appId = data.ReadInt32();
    std::string uuid = data.ReadString();
    int32_t startHandle = data.ReadInt32();
    int32_t endHandle = data.ReadInt32();
    if (!bluetooth::IsValidUuid(uuid) || startHandle < 0x0001 || endHandle > 0xFFFF || startHandle > endHandle) {
        HILOGE("invalid input params, uuid: %{public}s, startHandle: %{public}d, endHandle: %{public}d",
            uuid.c_str(), startHandle, endHandle);
        return BT_ERR_INVALID_PARAM;
    }
    int result = ReadCharacteristicByUuid(appId, uuid, startHandle, endHandle);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::WriteCharacteristicInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    bool withoutRespond = data.ReadBool();
    bool isWithContext = data.ReadBool();
    int result = WriteCharacteristic(appId, characteristic.get(), withoutRespond, isWithContext);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::SignedWriteCharacteristicInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    int result = SignedWriteCharacteristic(appId, characteristic.get());
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::ReadDescriptorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    int result = ReadDescriptor(appId, *descriptor);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::WriteDescriptorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    int result = WriteDescriptor(appId, descriptor.get());
    HILOGI("appId=%{public}d, result=%{public}d", appId, result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::RequestExchangeMtuInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int32_t mtu = data.ReadInt32();
    int result = RequestExchangeMtu(appId, mtu);
    HILOGI("appId=%{public}d, mtu=%{public}d, result=%{public}d", appId, mtu, result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::GetAllDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<BluetoothGattDevice> device;
    GetAllDevice(device);
    reply.WriteInt32(device.size());
    int num = static_cast<int>(device.size());
    HILOGI("device size:%{public}d", num);
    for (int i = 0; i < num; i++) {
        bool ret = reply.WriteParcelable(&device[i]);
        if (!ret) {
            HILOGE("WriteParcelable<GetAllDeviceInner> failed");
            return ERR_INVALID_VALUE;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::RequestConnectionPriorityInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int32_t connPriority = data.ReadInt32();
    int result = RequestConnectionPriority(appId, connPriority);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::GetServicesInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    std::vector<BluetoothGattService> service;
    int result = GetServices(appId, service);
    bool resultRet = reply.WriteInt32(result);
    bool sizeRet = reply.WriteInt32(service.size());
    if (!(resultRet && sizeRet)) {
        HILOGE("Write data failed");
        return ERR_INVALID_VALUE;
    }
    int num = static_cast<int>(service.size());
    for (int i = 0; i < num; i++) {
        bool ret = reply.WriteParcelable(&service[i]);
        if (!ret) {
            HILOGE("WriteParcelable<GetServicesInner> failed");
            return ERR_INVALID_VALUE;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::RequestFastestConnInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(addr->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = RequestFastestConn(*addr);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::ReadRemoteRssiValueInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int result = ReadRemoteRssiValue(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::RequestNotificationInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    uint16_t characterHandle = data.ReadUint16();
    bool enable = data.ReadBool();
    int result = RequestNotification(appId, characterHandle, enable);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int BluetoothGattClientStub::GetConnectedStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int state = -1;
    int res = GetConnectedState(deviceId, state);
    if (!reply.WriteInt32(res)) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (res != BT_NO_ERROR) {
        return res;
    }
    if (!reply.WriteInt32(state)) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothGattClientStub::SetPhyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int txPhy = data.ReadInt32();
    int rxPhy = data.ReadInt32();
    int phyOptions = data.ReadInt32();
    int result = SetPhy(appId, txPhy, rxPhy, phyOptions);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}
 
int32_t BluetoothGattClientStub::ReadPhyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appId = data.ReadInt32();
    int result = ReadPhy(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattClientStub: reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS