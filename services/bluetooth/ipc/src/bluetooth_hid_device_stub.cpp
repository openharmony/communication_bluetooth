/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_hid_device_stub"
#endif

#include "bluetooth_hid_device_stub.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "common_util.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) \
    static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::code), {&BluetoothHidDeviceStub::func, perm} \

namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothHidDeviceStub::HidDeviceStubFuncPerm> BluetoothHidDeviceStub::memberFuncMap_ = {
    {STUB_FUNC(COMMAND_CONNECT, ConnectInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_DISCONNECT, DisconnectInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    
    {STUB_FUNC(COMMAND_GET_CONNECTION_STATE, GetConnectionStateInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_CONNECTED_DEVICES, GetConnectedDevicesInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    
    {STUB_FUNC(COMMAND_REGISTER_APP, RegisterAppInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_UNREGISTER_APP, UnRegisterAppInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_REGISTER_OBSERVER, RegisterObserverInner, 
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_DEREGISTER_OBSERVER, DeregisterObserverInner, 
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_SEND_REPORT, SendReportInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_REPLY_REPORT, ReplyReportInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_REPORT_ERROR, ReportErrorInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_SET_CONNECT_STRATEGY, HidDeviceSetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_CONNECT_STRATEGY, HidDeviceGetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
 };

BluetoothHidDeviceStub::BluetoothHidDeviceStub()
{}

BluetoothHidDeviceStub::~BluetoothHidDeviceStub()
{}

int BluetoothHidDeviceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothHidDeviceStub ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothHidDeviceStub);
}

int32_t BluetoothHidDeviceStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (device == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t errCode = Connect(*device);
    // write error code
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t errCode = Disconnect();
    if (!reply.WriteInt32(errCode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::GetConnectionStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t state;
    int32_t errCode = GetConnectionState(*device, state);
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

int32_t BluetoothHidDeviceStub::GetConnectedDevicesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<BluetoothRawAddress> rawAddr;
    int32_t errCode = GetConnectedDevices(rawAddr);
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
    if (!reply.WriteInt32(rawAddr.size())) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    // write devices
    for (auto rawAddr : rawAddr) {
        if (!reply.WriteParcelable(&rawAddr)) {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::RegisterAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothHidDeviceSdp> sdpSettings(data.ReadParcelable<BluetoothHidDeviceSdp>());
    if (!sdpSettings) {
        return BT_ERR_INTERNAL_ERROR;
    }

    std::shared_ptr<BluetoothHidDeviceQos> inQos(data.ReadParcelable<BluetoothHidDeviceQos>());
    if (!inQos) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::shared_ptr<BluetoothHidDeviceQos> outQos(data.ReadParcelable<BluetoothHidDeviceQos>());
    if (!outQos) {
        return BT_ERR_INTERNAL_ERROR;
    }

    int errcode = RegisterHidDevice(*sdpSettings, *inQos, *outQos);
    if (!reply.WriteInt32(errcode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::UnRegisterAppInner(MessageParcel &data, MessageParcel &reply)
{
    int errcode = UnregisterHidDevice();
    if (!reply.WriteInt32(errcode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothHidDeviceObserver> observer = OHOS::iface_cast<IBluetoothHidDeviceObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothHidDeviceObserver> observer = OHOS::iface_cast<IBluetoothHidDeviceObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::SendReportInner(MessageParcel &data, MessageParcel &reply)
{
    int id = data.ReadInt32();
    std::vector<uint8_t> sendReportData;
    data.ReadUInt8Vector(&sendReportData);

    int errcode = SendReport(id, sendReportData);
    if (!reply.WriteInt32(errcode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::ReplyReportInner(MessageParcel &data, MessageParcel &reply)
{
    ReportType type = static_cast<ReportType>(data.ReadInt32());
    int id = data.ReadInt32();
    std::vector<uint8_t> replyReportData;
    data.ReadUInt8Vector(&replyReportData);

    int errcode = ReplyReport(type, id, replyReportData);
    if (!reply.WriteInt32(errcode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::ReportErrorInner(MessageParcel &data, MessageParcel &reply)
{
    ErrorReason type = static_cast<ErrorReason>(data.ReadInt32());

    int errcode = ReportError(type);
    if (!reply.WriteInt32(errcode)) {
        HILOGE("reply write failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceStub::HidDeviceSetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
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

int32_t BluetoothHidDeviceStub::HidDeviceGetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
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
} // Bluetooth
} // OHOS
