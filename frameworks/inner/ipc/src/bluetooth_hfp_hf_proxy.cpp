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
#define LOG_TAG "bt_ipc_hfp_hf_proxy"
#endif

#include "bluetooth_hfp_hf_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothHfpHfProxy::ConnectSco(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_CONNECT_SCO,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::DisconnectSco(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_DISCONNECT_SCO,
        data, reply, option, false);

    return reply.ReadBool();
}

int BluetoothHfpHfProxy::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32Vector(states), false, "write states error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_GET_DEVICES_BY_STATES,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    uint32_t devNum = reply.ReadUint32();
    const uint32_t maxSize = 100;
    if (devNum > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < devNum; i++) {
        std::shared_ptr<BluetoothRawAddress> dev(reply.ReadParcelable<BluetoothRawAddress>());
        if (!dev) {
            return BT_ERR_IPC_TRANS_FAILED;
        }
        devices.push_back(*dev);
    }
    return BT_NO_ERROR;
}

int BluetoothHfpHfProxy::GetDeviceState(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothHfpHfProxy::GetScoState(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_GET_SCO_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

bool BluetoothHfpHfProxy::SendDTMFTone(const BluetoothRawAddress &device, uint8_t code)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(
        data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()), false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(code), false, "write code error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_SEND_DTMF_TONE, data, reply, option, false);

    return reply.ReadBool();
}

int BluetoothHfpHfProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothHfpHfProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

bool BluetoothHfpHfProxy::OpenVoiceRecognition(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_OPEN_VOICE_RECOGNITION,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::CloseVoiceRecognition(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_CLOSE_VOICE_RECOGNITION,
        data, reply, option, false);

    return reply.ReadBool();
}

int BluetoothHfpHfProxy::GetCurrentCallList(const BluetoothRawAddress &device, std::vector<BluetoothHfpHfCall> &calls)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_GET_CURRENT_CALL_LIST,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);
    uint32_t callNum = reply.ReadUint32();
    const uint32_t maxSize = 100;
    if (callNum > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < callNum; i++) {
        std::shared_ptr<BluetoothHfpHfCall> call(reply.ReadParcelable<BluetoothHfpHfCall>());
        if (!call) {
            return TRANSACTION_ERR;
        }
        calls.push_back(*call);
    }
    return BT_NO_ERROR;
}

bool BluetoothHfpHfProxy::AcceptIncomingCall(const BluetoothRawAddress &device, int flag)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(flag), false, "write flag error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_ACCEPT_INCOMING_CALL,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::HoldActiveCall(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_HOLD_ACTIVE_CALL, data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::RejectIncomingCall(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_REJECT_INCOMING_CALL,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::HandleIncomingCall(const BluetoothRawAddress &device, int flag)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(flag), false, "write flag error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_HANDLE_INCOMING_CALL,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::HandleMultiCall(const BluetoothRawAddress &device, int flag, int index)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(flag), false, "write flag error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(index), false, "write index error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_HANDLE_MULLTI_CALL,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::DialLastNumber(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_DIAL_LAST_NUMBER,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::DialMemory(const BluetoothRawAddress &device, int index)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(index), false, "write index error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_DIAL_MEMORY,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::SendVoiceTag(const BluetoothRawAddress &device, int index)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(index), false, "write index error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_SEND_VOICE_TAG,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::SendKeyPressed(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_SEND_KEY_PRESSED,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpHfProxy::FinishActiveCall(const BluetoothRawAddress &device, const BluetoothHfpHfCall &call)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&call), false, "write call error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_FINISH_ATIVE_CALL,
        data, reply, option, false);

    return reply.ReadBool();
}

int BluetoothHfpHfProxy::StartDial(
    const BluetoothRawAddress &device, const std::string &number, BluetoothHfpHfCall &call)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(number), BT_ERR_IPC_TRANS_FAILED, "write number error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&call), BT_ERR_IPC_TRANS_FAILED, "write call error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpHfInterfaceCode::BT_HFP_HF_START_DIAL,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

void BluetoothHfpHfProxy::RegisterObserver(const sptr<IBluetoothHfpHfObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpHfInterfaceCode::BT_HFP_HF_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothHfpHfProxy::DeregisterObserver(const sptr<IBluetoothHfpHfObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpHfProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpHfInterfaceCode::BT_HFP_HF_DEREGISTER_OBSERVER, data, reply, option);
}

}  // namespace Bluetooth
}  // namespace OHOS
