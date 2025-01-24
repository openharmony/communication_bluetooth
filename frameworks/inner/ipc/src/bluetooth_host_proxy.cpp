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
#define LOG_TAG "bt_ipc_host_proxy"
#endif

#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
void BluetoothHostProxy::RegisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    HILOGD("BluetoothHostProxy::RegisterObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RegisterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::RegisterObserver error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REGISTER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::RegisterObserver done fail, error: %{public}d", error);
        return;
    }
    HILOGD("BluetoothHostProxy::RegisterObserver success");
}

void BluetoothHostProxy::DeregisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DeregisterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::DeregisterObserver error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_DEREGISTER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::DeregisterObserver done fail, error: %{public}d", error);
        return;
    }
}

int32_t BluetoothHostProxy::EnableBt()
{
    MessageParcel data;
    HILOGI("BluetoothHostProxy::EnableBt starts");
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::EnableBt WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_ENABLE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::EnableBt done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::DisableBt()
{
    HILOGI("BluetoothHostProxy::DisableBt starts");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DisableBt WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_DISABLE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::DisableBt done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::SatelliteControl(int type, int state)
{
    HILOGI("BluetoothHostProxy::SatelliteControl starts");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SatelliteControl WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(type), BT_ERR_IPC_TRANS_FAILED, "Write type error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(state), BT_ERR_IPC_TRANS_FAILED, "Write state error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SATELLITE_CONTROL, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SatelliteControl done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

sptr<IRemoteObject> BluetoothHostProxy::GetProfile(const std::string &name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetProfile WriteInterfaceToken error");
        return nullptr;
    }
    if (!data.WriteString(name)) {
        HILOGE("BluetoothHostProxy::GetProfile name error");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GETPROFILE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGD("BluetoothHostProxy::GetProfile done fail, error: %{public}d", error);
        return nullptr;
    }
    return reply.ReadRemoteObject();
}

sptr<IRemoteObject> BluetoothHostProxy::GetBleRemote(const std::string &name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetProfile WriteInterfaceToken error");
        return nullptr;
    }
    if (!data.WriteString(name)) {
        HILOGE("BluetoothHostProxy::GetProfile name error");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BLE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetProfile done fail, error: %{public}d", error);
        return nullptr;
    }
    return reply.ReadRemoteObject();
}

int32_t BluetoothHostProxy::BluetoothFactoryReset()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t ret = InnerTransact(BluetoothHostInterfaceCode::BT_FACTORY_RESET, option, data, reply);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_IPC_TRANS_FAILED, "error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetBtState(int &state)
{
    HILOGD("BluetoothHostProxy::GetBtState starts");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBtState WriteInterfaceToken error");
        return -1;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GETSTATE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtState done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t exception = reply.ReadInt32();
    if (exception == NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::GetLocalAddress(std::string &addr)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "GetLocalAddress WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_LOCAL_ADDRESS, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_IPC_TRANS_FAILED, "error: %{public}d", error);

    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        addr = reply.ReadString();
    }
    return exception;
}

int32_t BluetoothHostProxy::DisableBle()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DisableBle WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_DISABLE_BLE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::DisableBle done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::EnableBle(bool noAutoConnect)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::EnableBle WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteBool(noAutoConnect)) {
        HILOGE("BluetoothHostProxy::EnableBle WriteBool error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_ENABLE_BLE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::EnableBle done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

std::vector<uint32_t> BluetoothHostProxy::GetProfileList()
{
    std::vector<uint32_t> vec;
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetProfileList WriteInterfaceToken error");
        return vec;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_PROFILE_LIST, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetProfileList done fail, error: %{public}d", error);
        return vec;
    }
    if (!reply.ReadUInt32Vector(&vec)) {
        HILOGE("BluetoothHostProxy::GetProfileList Read reply fail");
        return vec;
    }
    return vec;
}

int32_t BluetoothHostProxy::GetMaxNumConnectedAudioDevices()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetMaxNumConnectedAudioDevices WriteInterfaceToken error");
        return -1;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_GET_MAXNUM_CONNECTED_AUDIODEVICES, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetMaxNumConnectedAudioDevices done fail, error: %{public}d", error);
        return -1;
    }
    int32_t maxNum;
    if (!reply.ReadInt32(maxNum)) {
        HILOGE("BluetoothHostProxy::GetMaxNumConnectedAudioDevices Read reply fail");
        return -1;
    }
    return maxNum;
}

int32_t BluetoothHostProxy::GetBtConnectionState(int &state)
{
    HILOGD("BluetoothHostProxy::GetBtConnectionState starts");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBtConnectionState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BT_STATE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtConnectionState done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::GetBtProfileConnState(uint32_t profileId, int &state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBtProfileConnState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteUint32(profileId)) {
        HILOGE("BluetoothHostProxy::GetBtProfileConnState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BT_PROFILE_CONNSTATE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtProfileConnState done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::GetLocalDeviceClass()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetLocalDeviceClass WriteInterfaceToken error");
        return -1;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_LOCAL_DEVICE_CLASS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetLocalDeviceClass done fail, error: %{public}d", error);
        return -1;
    }

    int32_t result;
    if (!reply.ReadInt32(result)) {
        HILOGE("BluetoothHostProxy::GetLocalDeviceClass Read reply fail");
        return -1;
    }
    return result;
}

bool BluetoothHostProxy::SetLocalDeviceClass(const int32_t &deviceClass)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetLocalDeviceClass WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteUint32(deviceClass)) {
        HILOGE("BluetoothHostProxy::SetLocalDeviceClass WriteInterfaceToken error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_SET_LOCAL_DEVICE_CLASS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetLocalDeviceClass done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t BluetoothHostProxy::GetLocalName(std::string &name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetLocalName WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_LOCAL_NAME, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetLocalName done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        name = reply.ReadString();
    }
    return exception;
}

int32_t BluetoothHostProxy::SetLocalName(const std::string &name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetLocalName WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(name)) {
        HILOGE("BluetoothHostProxy::SetLocalName WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_SET_LOCAL_NAME, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetLocalName done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetBtScanMode(int32_t &scanMode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBtScanMode WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BT_SCAN_MODE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtScanMode done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        scanMode = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::SetBtScanMode(int32_t mode, int32_t duration)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetBtScanMode WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(mode)) {
        HILOGE("BluetoothHostProxy::SetBtScanMode WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(duration)) {
        HILOGE("BluetoothHostProxy::SetBtScanMode WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_SET_BT_SCAN_MODE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetBtScanMode done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetBondableMode(const int32_t transport)
{
    int32_t Mode;
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBondableMode WriteInterfaceToken error");
        return -1;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::GetBondableMode WriteInterfaceToken error");
        return -1;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BONDABLE_MODE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBondableMode done fail, error: %{public}d", error);
        return -1;
    }
    if (!reply.ReadInt32(Mode)) {
        HILOGE("BluetoothHostProxy::GetBondableMode Read reply fail");
        return -1;
    }
    return Mode;
}

bool BluetoothHostProxy::SetBondableMode(int32_t transport, int32_t mode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetBondableMode WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::SetBondableMode WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(mode)) {
        HILOGE("BluetoothHostProxy::SetBondableMode WriteInterfaceToken error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_SET_BONDABLE_MODE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetBondableMode done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t BluetoothHostProxy::StartBtDiscovery()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::StartBtDiscovery WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_START_BT_DISCOVERY, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::StartBtDiscovery done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::CancelBtDiscovery()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::CancelBtDiscovery WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_CANCEL_BT_DISCOVERY, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::CancelBtDiscovery done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::IsBtDiscovering(bool &isDisCovering, const int32_t transport)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::IsBtDiscovering WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::IsBtDiscovering WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_IS_BT_DISCOVERING, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::Start done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        isDisCovering = reply.ReadBool();
    }
    return exception;
}

long BluetoothHostProxy::GetBtDiscoveryEndMillis()
{
    long millis;
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBtDiscoveryEndMillis WriteInterfaceToken error");
        return -1;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BT_DISCOVERY_END_MILLIS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtDiscoveryEndMillis done fail, error: %{public}d", error);
        return -1;
    }
    millis = static_cast<long>(reply.ReadInt64());
    return millis;
}

int32_t BluetoothHostProxy::GetPairedDevices(std::vector<BluetoothRawAddress> &pairedAddr)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetPairedDevices WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_PAIRED_DEVICES, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetPairedDevices done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t size = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (size > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int32_t i = 0; i < size; i++) {
        std::shared_ptr<BluetoothRawAddress> rawAddress(reply.ReadParcelable<BluetoothRawAddress>());
        if (!rawAddress) {
            return BT_ERR_IPC_TRANS_FAILED;
        }
        pairedAddr.push_back(*rawAddress);
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::RemovePair(const int32_t transport, const sptr<BluetoothRawAddress> &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RemovePair WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::RemovePair WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(device.GetRefPtr())) {
        HILOGE("BluetoothHostProxy::RemovePair WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REMOVE_PAIR, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::RemovePair done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

bool BluetoothHostProxy::RemoveAllPairs()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RemoveAllPairs WriteInterfaceToken error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REMOVE_ALL_PAIRS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::RemoveAllPairs done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

void BluetoothHostProxy::RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RegisterRemoteDeviceObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::RegisterRemoteDeviceObserver WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REGISTER_REMOTE_DEVICE_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBtConnectionState done fail, error: %{public}d", error);
        return;
    }
    return;
}

void BluetoothHostProxy::DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DeregisterRemoteDeviceObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::DeregisterRemoteDeviceObserver WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_DEREGISTER_REMOTE_DEVICE_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::DeregisterRemoteDeviceObserver done fail, error: %{public}d", error);
        return;
    }
    return;
}

int32_t BluetoothHostProxy::GetBleMaxAdvertisingDataLength()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetBleMaxAdvertisingDataLength WriteInterfaceToken error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_GET_BLE_MAX_ADVERTISING_DATALENGTH, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetBleMaxAdvertisingDataLength done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetDeviceType(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetDeviceType WriteInterfaceToken error");
        return 0;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::GetDeviceType transport error");
        return 0;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetDeviceType address error");
        return 0;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_TYPE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetDeviceType done fail, error: %{public}d", error);
        return 0;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetPhonebookPermission(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetPhonebookPermission WriteInterfaceToken error");
        return 0;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetPhonebookPermission address error");
        return 0;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_PHONEBOOK_PERMISSION, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetPhonebookPermission done fail, error: %{public}d", error);
        return 0;
    }
    return reply.ReadInt32();
}

bool BluetoothHostProxy::SetPhonebookPermission(const std::string &address, int32_t permission)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetPhonebookPermission WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetPhonebookPermission address error");
        return false;
    }
    if (!data.WriteInt32(permission)) {
        HILOGE("BluetoothHostProxy::SetPhonebookPermission permission error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_PHONEBOOK_PERMISSION, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetPhonebookPermission done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t BluetoothHostProxy::GetMessagePermission(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetMessagePermission WriteInterfaceToken error");
        return 0;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetMessagePermission address error");
        return 0;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_MESSAGE_PERMISSION, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetMessagePermission done fail, error: %{public}d", error);
        return 0;
    }
    return reply.ReadInt32();
}

bool BluetoothHostProxy::SetMessagePermission(const std::string &address, int32_t permission)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetMessagePermission WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetMessagePermission address error");
        return false;
    }
    if (!data.WriteInt32(permission)) {
        HILOGE("BluetoothHostProxy::SetMessagePermission permission error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_MESSAGE_PERMISSION, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetMessagePermission done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t BluetoothHostProxy::GetPowerMode(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetPowerMode WriteInterfaceToken error");
        return 0;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetPowerMode address error");
        return 0;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_POWER_MODE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetPowerMode done fail, error: %{public}d", error);
        return 0;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetDeviceName(int32_t transport, const std::string &address, std::string &name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetDeviceName WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::GetDeviceName transport error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetDeviceName address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_NAME, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetDeviceName done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        name = reply.ReadString();
    }
    return exception;
}

std::string BluetoothHostProxy::GetDeviceAlias(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetDeviceAlias WriteInterfaceToken error");
        return std::string();
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetDeviceAlias address error");
        return std::string();
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_ALIAS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetDeviceAlias done fail, error: %{public}d", error);
        return std::string();
    }
    return reply.ReadString();
}

int32_t BluetoothHostProxy::SetDeviceAlias(const std::string &address, const std::string &aliasName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetDeviceAlias WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetDeviceAlias address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(aliasName)) {
        HILOGE("BluetoothHostProxy::SetDeviceAlias aliasName error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_DEVICE_ALIAS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetDeviceAlias done fail, error: %{public}d", error);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetRemoteDeviceBatteryInfo(const std::string &address, BluetoothBatteryInfo &batteryInfo)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "write InterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(address), BT_ERR_INTERNAL_ERROR, "write address error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t ret = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_BATTERY_INFO, option, data, reply);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "ret: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "ret: %{public}d", ret);
    std::shared_ptr<BluetoothBatteryInfo> info(reply.ReadParcelable<BluetoothBatteryInfo>());
    CHECK_AND_RETURN_LOG_RET(info != nullptr, BT_ERR_INTERNAL_ERROR, "read info fail");
    batteryInfo = *info;
    return ret;
}

int32_t BluetoothHostProxy::GetPairState(int32_t transport, const std::string &address, int32_t &pairState)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetPairState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::GetPairState transport error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetPairState address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_PAIR_STATE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetPairState done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        pairState = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::StartPair(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::StartPair WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::StartPair transport error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::StartPair address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::START_PAIR, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::StartPair done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::StartCrediblePair(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::StartCrediblePair WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::StartCrediblePair transport error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::StartCrediblePair address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::START_CREDIBLE_PAIR, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::StartCrediblePair done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

bool BluetoothHostProxy::CancelPairing(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::CancelPairing WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::CancelPairing transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::CancelPairing address error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::CANCEL_PAIRING, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::CancelPairing done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool BluetoothHostProxy::IsBondedFromLocal(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::IsBondedFromLocal WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::IsBondedFromLocal transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::IsBondedFromLocal address error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::IS_BONDED_FROM_LOCAL, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::IsBondedFromLocal done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool BluetoothHostProxy::IsAclConnected(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::IsAclConnected WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::IsAclConnected transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::IsAclConnected address error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::IS_ACL_CONNECTED, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::IsAclConnected done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool BluetoothHostProxy::IsAclEncrypted(int32_t transport, const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::IsAclEncrypted WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::IsAclEncrypted transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::IsAclEncrypted address error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::IS_ACL_ENCRYPTED, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::IsAclEncrypted done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t BluetoothHostProxy::GetDeviceClass(const std::string &address, int &cod)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetDeviceClass WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetDeviceClass address error");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_CLASS, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetDeviceClass done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        cod = reply.ReadInt32();
    }
    return exception;
}

int32_t BluetoothHostProxy::SetDevicePin(const std::string &address, const std::string &pin)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetDevicePin WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetDevicePin address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(pin)) {
        HILOGE("BluetoothHostProxy::SetDevicePin pin error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_DEVICE_PIN, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetDevicePin done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetDevicePairingConfirmation WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::SetDevicePairingConfirmation transport error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetDevicePairingConfirmation address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteBool(accept)) {
        HILOGE("BluetoothHostProxy::SetDevicePairingConfirmation accept error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_DEVICE_PAIRING_CONFIRMATION, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetDevicePairingConfirmation done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

bool BluetoothHostProxy::SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey address error");
        return false;
    }
    if (!data.WriteInt32(passkey)) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey passkey error");
        return false;
    }
    if (!data.WriteBool(accept)) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey accept error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_DEVICE_PASSKEY, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetDevicePasskey done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool BluetoothHostProxy::PairRequestReply(int32_t transport, const std::string &address, bool accept)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::PairRequestReply WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteInt32(transport)) {
        HILOGE("BluetoothHostProxy::PairRequestReply transport error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::PairRequestReply address error");
        return false;
    }
    if (!data.WriteBool(accept)) {
        HILOGE("BluetoothHostProxy::PairRequestReply accept error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::PAIR_REQUEST_PEPLY, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::PairRequestReply done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool BluetoothHostProxy::ReadRemoteRssiValue(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::ReadRemoteRssiValue WriteInterfaceToken error");
        return false;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::ReadRemoteRssiValue address error");
        return false;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::READ_REMOTE_RSSI_VALUE, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::ReadRemoteRssiValue done fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

void BluetoothHostProxy::GetLocalSupportedUuids(std::vector<std::string> &uuids)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetLocalSupportedUuids WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_LOCAL_SUPPORTED_UUIDS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetLocalSupportedUuids done fail, error: %{public}d", error);
        return;
    }
    int32_t size = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (size > maxSize) {
        return;
    }
    std::string uuid;
    for (int32_t i = 0; i < size; i++) {
        uuid = reply.ReadString();
        uuids.push_back(uuid);
    }
}

int32_t BluetoothHostProxy::GetDeviceUuids(const std::string &address, std::vector<std::string> &uuids)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetDeviceUuids WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetDeviceUuids Write address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_UUIDS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetDeviceUuids done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t size = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (size > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    std::string uuid;
    for (int32_t i = 0; i < size; i++) {
        uuid = reply.ReadString();
        uuids.push_back(uuid);
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetLocalProfileUuids(std::vector<std::string> &uuids)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetLocalProfileUuids WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_LOCAL_PROFILE_UUIDS, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetLocalProfileUuids done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t size = reply.ReadInt32();
    const int32_t maxSize = 1000;
    if (size > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    std::string uuid;
    for (int32_t i = 0; i < size; i++) {
        uuid = reply.ReadString();
        uuids.push_back(uuid);
    }
    return reply.ReadInt32();
}

void BluetoothHostProxy::RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    HILOGD("BluetoothHostProxy::RegisterBleAdapterObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RegisterBleAdapterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::RegisterBleAdapterObserver error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REGISTER_BLE_ADAPTER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::RegisterBleAdapterObserver done fail, error: %{public}d", error);
        return;
    }
    HILOGD("BluetoothHostProxy::RegisterBleAdapterObserver success");
}

void BluetoothHostProxy::DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    HILOGI("BluetoothHostProxy::DeregisterBleAdapterObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DeregisterBleAdapterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::DeregisterBleAdapterObserver error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_DEREGISTER_BLE_ADAPTER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::DeregisterBleAdapterObserver done fail, error: %{public}d", error);
        return;
    }
}

void BluetoothHostProxy::RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    HILOGD("BluetoothHostProxy::RegisterBlePeripheralCallback start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RegisterBlePeripheralCallback WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::RegisterBlePeripheralCallback WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_REGISTER_BLE_PERIPHERAL_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::RegisterBlePeripheralCallback done fail, error: %{public}d", error);
        return;
    }
    return;
}

void BluetoothHostProxy::DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    HILOGI("BluetoothHostProxy::DeregisterBlePeripheralCallback start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DeregisterBlePeripheralCallback WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::DeregisterBlePeripheralCallback WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_DEREGISTER_BLE_PERIPHERAL_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::DeregisterBlePeripheralCallback done fail, error: %{public}d", error);
        return;
    }
    return;
}

int32_t BluetoothHostProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int32_t err = remote->SendRequest(code, data, reply, flags);
    if (err != NO_ERROR) {
        HILOGD("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
    }
    return err;
}

int32_t BluetoothHostProxy::SetFastScan(bool isEnable)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SetBtScanMode WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteBool(isEnable)) {
        HILOGE("BluetoothHostProxy::SetFastScan WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_SET_FAST_SCAN, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SetFastScan done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetRandomAddress(const std::string &realAddr, std::string &randomAddr)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetRandomAddress WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(realAddr)) {
        HILOGE("BluetoothHostProxy::GetRandomAddress Write realAddr error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_RANDOM_ADDRESS, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetRandomAddress fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    randomAddr = reply.ReadString();
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::SyncRandomAddress(const std::string &realAddr, const std::string &randomAddr)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::SyncRandomAddress WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(realAddr)) {
        HILOGE("BluetoothHostProxy::SyncRandomAddress Write realAddr error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(randomAddr)) {
        HILOGE("BluetoothHostProxy::SyncRandomAddress Write randomAddr error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SYNC_RANDOM_ADDRESS, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::SyncRandomAddress fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::ConnectAllowedProfiles(const std::string &remoteAddr)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(remoteAddr), BT_ERR_IPC_TRANS_FAILED,
        "Write remoteAddr error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::CONNECT_ALLOWED_PROFILES, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::DisconnectAllowedProfiles(const std::string &remoteAddr)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(remoteAddr), BT_ERR_IPC_TRANS_FAILED,
        "Write remoteAddr error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::DISCONNECT_ALLOWED_PROFILES, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::SetDeviceCustomType(const std::string &address, int32_t deviceType)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(address), BT_ERR_IPC_TRANS_FAILED, "Write remoteAddr error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteInt32(deviceType), BT_ERR_IPC_TRANS_FAILED, "Write deviceType error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_CUSTOM_TYPE, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetRemoteDeviceInfo(const std::string &address,
    std::shared_ptr<BluetoothRemoteDeviceInfo> &deviceInfo, int type)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(address), BT_ERR_IPC_TRANS_FAILED, "Write remoteAddr error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(type), BT_ERR_IPC_TRANS_FAILED, "Write type error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_DEVICE_INFO_ID, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    BtErrCode exception = static_cast<BtErrCode>(reply.ReadInt32());
    if (exception == BT_NO_ERROR) {
        deviceInfo = std::shared_ptr<BluetoothRemoteDeviceInfo>(reply.ReadParcelable<BluetoothRemoteDeviceInfo>());
    }
    return exception;
}

void BluetoothHostProxy::RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::RegisterBtResourceManagerObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::RegisterBtResourceManagerObserver WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_REGISTER_RESOURCE_MANAGER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::RegisterBtResourceManagerObserver done fail, error: %{public}d", error);
        return;
    }
    return;
}

void BluetoothHostProxy::DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::DeregisterBtResourceManagerObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHostProxy::DeregisterBtResourceManagerObserver WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_DEREGISTER_RESOURCE_MANAGER_OBSERVER, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHostProxy::DeregisterBtResourceManagerObserver done fail, error: %{public}d", error);
        return;
    }
    return;
}

int32_t BluetoothHostProxy::IsSupportVirtualAutoConnect(const std::string &address, bool &outSupport)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(address), BT_ERR_IPC_TRANS_FAILED, "Write remoteAddr error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::GET_VIRTUAL_AUTO_CONN_SWITCH, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    outSupport = reply.ReadBool();
    return BT_NO_ERROR;
}

int32_t BluetoothHostProxy::SetVirtualAutoConnectType(const std::string &address, int connType, int businessType)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(address), BT_ERR_IPC_TRANS_FAILED, "Write remoteAddr error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(connType), BT_ERR_IPC_TRANS_FAILED, "Write connType error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(businessType), BT_ERR_IPC_TRANS_FAILED, "Write businessType error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_VIRTUAL_AUTO_CONN_TYPE, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return BT_NO_ERROR;
}

void BluetoothHostProxy::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(action), "Write action error");
    CHECK_AND_RETURN_LOG(data.WriteString(address), "Write address error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::UPDATE_VIRTUAL_DEVICE, option, data, reply);
    CHECK_AND_RETURN_LOG((error == BT_NO_ERROR), "error: %{public}d", error);
    return;
}
int32_t BluetoothHostProxy::SetFastScanLevel(int level)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(level), BT_ERR_IPC_TRANS_FAILED, "Write level error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::SET_FAST_SCAN_LEVEL, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::EnableBluetoothToRestrictMode(void)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_ENABLE_BLUETOOTH_TO_RESTRICT_MODE, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
    uint32_t controlTypeVal, uint32_t controlObject)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(deviceId), BT_ERR_IPC_TRANS_FAILED, "Write deviceId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint32(controlType), BT_ERR_IPC_TRANS_FAILED, "Write controlType error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint32(controlTypeVal), BT_ERR_IPC_TRANS_FAILED, "Write controlTypeVal error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint32(controlObject), BT_ERR_IPC_TRANS_FAILED, "Write controlObject error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::CTRL_DEVICE_ACTION, option, data, reply);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetLastConnectionTime(const std::string &address, int64_t &connectionTime)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetLastConnectionTime WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetLastConnectionTime WriteAddress error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::GET_CONNECTION_TIME, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetLastConnectionTime done fail error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    BtErrCode exception = static_cast<BtErrCode>(reply.ReadInt32());
    if (exception == BT_NO_ERROR) {
        connectionTime = reply.ReadInt64();
    }
    return exception;
}

int32_t BluetoothHostProxy::UpdateCloudBluetoothDevice(std::vector<BluetoothTrustPairDevice> &cloudDevices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    int32_t cloudDevSize = static_cast<int32_t>(cloudDevices.size());
    CHECK_AND_RETURN_LOG_RET(cloudDevSize < TRUST_PAIR_DEVICE_SIZE_MAX,
        BT_ERR_IPC_TRANS_FAILED, "error size:%{public}d", cloudDevSize);
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(cloudDevSize),
        BT_ERR_IPC_TRANS_FAILED, "Write cloudDevices size error");
    for (auto &dev : cloudDevices) {
        CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&dev),
            BT_ERR_IPC_TRANS_FAILED, "Write cloudDevices error");
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::BT_UPDATE_CLOUD_DEVICE, option, data, reply);
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDev %{public}d.", cloudDevSize);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothHostProxy::GetCloudBondState(const std::string &address, int32_t &cloudBondState)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHostProxy::GetCloudBondState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("BluetoothHostProxy::GetCloudBondState WriteAddress error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothHostInterfaceCode::GET_CLOUD_BOND_STATE, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHostProxy::GetCloudBondState done fail error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    BtErrCode exception = static_cast<BtErrCode>(reply.ReadInt32());
    if (exception == BT_NO_ERROR) {
        cloudBondState = reply.ReadInt32();
    }
    return exception;
}
}  // namespace Bluetooth
}  // namespace OHOS
