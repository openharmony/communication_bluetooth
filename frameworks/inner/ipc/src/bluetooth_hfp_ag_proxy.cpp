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
#define LOG_TAG "bt_ipc_hfp_ag_proxy"
#endif

#include "bluetooth_hfp_ag_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
#define MAX_HFP_VIRTUAL_DEVICE 10
int32_t BluetoothHfpAgProxy::GetConnectDevices(std::vector<BluetoothRawAddress> &devices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_CONNECT_DEVICES,
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

int BluetoothHfpAgProxy::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32Vector(states), BT_ERR_IPC_TRANS_FAILED, "write states error");
    
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_DEVICES_BY_STATES,
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
int32_t BluetoothHfpAgProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }
    // read state
    state = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothHfpAgProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHfpAgProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothHfpAgProxy::GetScoState(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_SCO_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHfpAgProxy::ConnectSco(uint8_t callType)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(callType), BT_ERR_IPC_TRANS_FAILED, "write callType error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CONNECT_SCO_EX,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHfpAgProxy::DisconnectSco(uint8_t callType)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(callType), BT_ERR_IPC_TRANS_FAILED, "write callType error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_DISCONNECT_SCO_EX,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

bool BluetoothHfpAgProxy::ConnectSco()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CONNECT_SCO, data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpAgProxy::DisconnectSco()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_DISCONNECT_SCO, data, reply, option, false);

    return reply.ReadBool();
}

void BluetoothHfpAgProxy::PhoneStateChanged(BluetoothPhoneState &phoneState)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteParcelable(&phoneState), "write phoneState error");
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_PHONE_STATE_CHANGED, data, reply, option);
}

void BluetoothHfpAgProxy::ClccResponse(
    int index, int direction, int status, int mode, bool mpty, const std::string &number, int type)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(index), "write index error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(direction), "write direction error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(status), "write status error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(mode), "write mode error");
    CHECK_AND_RETURN_LOG(data.WriteBool(mpty), "write mpty error");
    CHECK_AND_RETURN_LOG(data.WriteString(number), "write number error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(type), "write type error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CLCC_RESPONSE, data, reply, option);
}

bool BluetoothHfpAgProxy::OpenVoiceRecognition(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_OPEN_VOICE_RECOGNITION,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpAgProxy::CloseVoiceRecognition(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CLOSE_VOICE_RECOGNITION,
        data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpAgProxy::SetActiveDevice(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(
        BluetoothHfpAgInterfaceCode::BT_HFP_AG_SET_ACTIVE_DEVICE, data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpAgProxy::IntoMock(const BluetoothRawAddress &device, int state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(state), false, "write state error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_INTO_MOCK, data, reply, option, false);

    return reply.ReadBool();
}

bool BluetoothHfpAgProxy::SendNoCarrier(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        false, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), false, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_SEND_NO_CARRIER, data, reply, option, false);

    return reply.ReadBool();
}

std::string BluetoothHfpAgProxy::GetActiveDevice()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(
        data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "", "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_ACTIVE_DEVICE, data, reply, option, "");

    return reply.ReadString();
}

int BluetoothHfpAgProxy::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED, "write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_SET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothHfpAgProxy::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }

    return res;
}

int BluetoothHfpAgProxy::IsInbandRingingEnabled(bool &isEnabled)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_IS_IN_BAND_RINGING_ENABLE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int ret = reply.ReadInt32();
    isEnabled = reply.ReadBool();
    return ret;
}

void BluetoothHfpAgProxy::CallDetailsChanged(int callId, int callState)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(callId), "write callId error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(callState), "write callState error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CALL_DETAILS_CHANGED, data, reply, option);
}

int BluetoothHfpAgProxy::IsVgsSupported(const BluetoothRawAddress &device, bool &isSupported)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothHfpAgInterfaceCode::BT_HFP_AG_IS_VGS_SUPPORTED,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        isSupported = reply.ReadBool();
    }

    return res;
}

void BluetoothHfpAgProxy::RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothHfpAgProxy::DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_DEREGISTER_OBSERVER, data, reply, option);
}

void BluetoothHfpAgProxy::EnableBtCallLog(bool state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteBool(state), "Write state error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_CALL_LOG, data, reply, option);
}

void BluetoothHfpAgProxy::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_GET_VIRTUALDEVICE_LIST, data, reply, option);

    int32_t rawAddsSize = reply.ReadInt32();

    CHECK_AND_RETURN_LOG(rawAddsSize <= MAX_HFP_VIRTUAL_DEVICE, "virtual device size error.");

    for (int i = 0; i < rawAddsSize; i++) {
        devices.push_back(reply.ReadString());
    }

    return;
}

void BluetoothHfpAgProxy::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHfpAgProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(action), "write action error");
    CHECK_AND_RETURN_LOG(data.WriteString(address), "write address error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothHfpAgInterfaceCode::BT_HFP_AG_UPDATE_VIRTUALDEVICE, data, reply, option);
}
}  // namespace Bluetooth
}  // namespace OHOS
