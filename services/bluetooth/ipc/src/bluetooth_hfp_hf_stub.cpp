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
#define LOG_TAG "bt_ipc_hfp_hf_stub"
#endif

#include "bluetooth_hfp_hf_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "common_util.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothHfpHfInterfaceCode::code, {&BluetoothHfpHfStub::func, perm}

namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothHfpHfStub::HfpHfStubFuncPerm> BluetoothHfpHfStub::memberFuncMap_ = {
    {STUB_FUNC(BT_HFP_HF_CONNECT_SCO, ConnectScoInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_DISCONNECT_SCO, DisconnectScoInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_GET_DEVICES_BY_STATES, GetDevicesByStatesInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_GET_DEVICE_STATE, GetDeviceStateInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_GET_SCO_STATE, GetScoStateInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_SEND_DTMF_TONE, SendDTMFToneInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_CONNECT, ConnectInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_DISCONNECT, DisconnectInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_OPEN_VOICE_RECOGNITION, OpenVoiceRecognitionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_CLOSE_VOICE_RECOGNITION, CloseVoiceRecognitionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_GET_CURRENT_CALL_LIST, GetCurrentCallListInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_ACCEPT_INCOMING_CALL, AcceptIncomingCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_HOLD_ACTIVE_CALL, HoldActiveCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_REJECT_INCOMING_CALL, RejectIncomingCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_SEND_KEY_PRESSED, SendKeyPressedInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_HANDLE_INCOMING_CALL, HandleIncomingCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_HANDLE_MULLTI_CALL, HandleMultiCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_DIAL_LAST_NUMBER, DialLastNumberInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_DIAL_MEMORY, DialMemoryInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_SEND_VOICE_TAG, SendVoiceTagInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_FINISH_ATIVE_CALL, FinishActiveCallInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_START_DIAL, StartDialInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(BT_HFP_HF_SET_CONNECT_STRATEGY, SetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_HFP_HF_GET_CONNECT_STRATEGY, GetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothHfpHfStub::BluetoothHfpHfStub()
{}

BluetoothHfpHfStub::~BluetoothHfpHfStub()
{}

int32_t BluetoothHfpHfStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothHfpHf ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothHfpHfStub);
}

int32_t BluetoothHfpHfStub::ConnectScoInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = ConnectSco(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::DisconnectScoInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = DisconnectSco(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<BluetoothRawAddress> devices;
    std::vector<int> states;
    data.ReadInt32Vector(&states);
    GetDevicesByStates(states, devices);
    uint32_t devNum = devices.size();
    if (!reply.WriteUint32(devNum)) {
        HILOGE("BluetoothHfpHfStub:WriteInt32 failed.");
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < devNum; i++) {
        if (!reply.WriteParcelable(&devices[i])) {
            HILOGE("BluetoothHfpHfStub:WriteParcelable failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = GetDeviceState(*device);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::GetScoStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = GetScoState(*device);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::SendDTMFToneInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    uint8_t code = data.ReadUint8();
    int result = SendDTMFTone(*device, code);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_IPC_TRANSACTION_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = Connect(*device);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return BT_ERR_IPC_TRANSACTION_FAILED;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_IPC_TRANSACTION_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = Disconnect(*device);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return BT_ERR_IPC_TRANSACTION_FAILED;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::OpenVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = OpenVoiceRecognition(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::CloseVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = CloseVoiceRecognition(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::GetCurrentCallListInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<BluetoothHfpHfCall> calls;
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    GetCurrentCallList(*device, calls);
    uint32_t callNum = calls.size();
    if (!reply.WriteUint32(callNum)) {
        HILOGE("BluetoothHfpHfStub:WriteInt32 failed.");
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < callNum; i++) {
        if (!reply.WriteParcelable(&calls[i])) {
            HILOGE("BluetoothHfpHfStub:WriteParcelable failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::AcceptIncomingCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int flag = data.ReadInt32();
    int result = AcceptIncomingCall(*device, flag);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::HoldActiveCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = HoldActiveCall(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::SendKeyPressedInner(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::RejectIncomingCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = RejectIncomingCall(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::HandleIncomingCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("BluetoothHfpHfStub: device is not exist.");
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int flag = data.ReadInt32();
    int result = HandleIncomingCall(*device, flag);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::HandleMultiCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("BluetoothHfpHfStub: device is not exist.");
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int flag = data.ReadInt32();
    int index = data.ReadInt32();
    int result = HandleMultiCall(*device, flag, index);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::DialLastNumberInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("BluetoothHfpHfStub: device is not exist.");
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = DialLastNumber(*device);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::DialMemoryInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("BluetoothHfpHfStub: device is not exist.");
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int index = data.ReadInt32();
    HILOGE("BluetoothHfpHfStub: reply writing failed. index = %{public}d", index);
    int result = DialMemory(*device, index);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::SendVoiceTagInner(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::FinishActiveCallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    std::shared_ptr<BluetoothHfpHfCall> call(data.ReadParcelable<BluetoothHfpHfCall>());
    if (!call) {
        return TRANSACTION_ERR;
    }
    int result = FinishActiveCall(*device, *call);
    if (!reply.WriteBool(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::StartDialInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    std::string number = data.ReadString();
    std::shared_ptr<BluetoothHfpHfCall> call(data.ReadParcelable<BluetoothHfpHfCall>());
    if (!call) {
        return TRANSACTION_ERR;
    }
    int result = StartDial(*device, number, *call);
    if (!reply.WriteInt32(result)) {
        HILOGE("BluetoothHfpHfStub: reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> tempObject = data.ReadRemoteObject();
    sptr<IBluetoothHfpHfObserver> observer = iface_cast<IBluetoothHfpHfObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> tempObject = data.ReadRemoteObject();
    sptr<IBluetoothHfpHfObserver> observer = iface_cast<IBluetoothHfpHfObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("Read device address failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int strategy = data.ReadInt32();

    int result = SetConnectStrategy(*device, strategy);

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return NO_ERROR;
}

int32_t BluetoothHfpHfStub::GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("Read device address failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int strategy = 0;
    int result = GetConnectStrategy(*device, strategy);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing res failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!reply.WriteInt32(strategy)) {
        HILOGE("reply writing strategy failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
