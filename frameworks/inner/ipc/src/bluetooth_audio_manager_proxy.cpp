/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_audio_manager_proxy"
#endif

#include "bluetooth_audio_manager_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_service_ipc_interface_code.h"
 
namespace OHOS {
namespace Bluetooth {
 
int BluetoothAudioManagerProxy::EnableWearDetection(const std::string &deviceId)
{
    return SetWearDetection(deviceId, true);
}

int BluetoothAudioManagerProxy::DisableWearDetection(const std::string &deviceId)
{
    return SetWearDetection(deviceId, false);
}

int BluetoothAudioManagerProxy::SetWearDetection(const std::string &deviceId, bool enable)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothAudioManagerProxy::GetDescriptor())) {
        HILOGE("SetWearDetection error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        HILOGE("SetWearDetectionwrite device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option{MessageOption::TF_SYNC};
    int opcode = enable ? BluetoothAudioManagerInterfaceCode::WEAR_DETECTION_ENABLE
                        : BluetoothAudioManagerInterfaceCode::WEAR_DETECTION_DISABLE;
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_IPC_TRANS_FAILED, "remote nullptr");
    int error = remote->SendRequest(opcode, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("SetWearDetection done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}
 
int BluetoothAudioManagerProxy::GetWearDetectionState(const std::string &deviceId, int32_t &ability)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothAudioManagerProxy::GetDescriptor())) {
        HILOGE("BluetoothWearDetectionProxy::Enable wear detection error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetection write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option{MessageOption::TF_SYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_IPC_TRANS_FAILED, "remote nullptr");
    int error = remote->SendRequest(
        static_cast<uint32_t>(BluetoothAudioManagerInterfaceCode::IS_WEAR_DETECTION_ENABLED), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetections done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    if (ret == BT_NO_ERROR) {
        ability = reply.ReadInt32();
    }

    return ret;
}

int32_t BluetoothAudioManagerProxy::IsDeviceWearing(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothAudioManagerProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "BluetoothAudioManagerProxy::IsDeviceWearing WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::IsDeviceWearing Write device error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_IPC_TRANS_FAILED, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothAudioManagerInterfaceCode::IS_DEVICE_WEARING, data, reply, option);

    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::IsDeviceWearing fail, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothAudioManagerProxy::IsWearDetectionSupported(const BluetoothRawAddress &device, bool &isSupported)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothAudioManagerProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "BluetoothAudioManagerProxy::IsWearDetectionSupported WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::IsWearDetectionSupported Write device error");
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_IPC_TRANS_FAILED, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothAudioManagerInterfaceCode::BT_IS_WEAR_DETECTION_SUPPORTED, data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::IsWearDetectionSupported fail, error: %{public}d", error);
    isSupported = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t BluetoothAudioManagerProxy::SendDeviceSelection(const BluetoothRawAddress &device,
    int useA2dp, int useHfp, int userSelection)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothAudioManagerProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "BluetoothAudioManagerProxy::IsWearDetectionSupported WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::SendDeviceSelection Write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(useA2dp), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::SendDeviceSelection Write useA2dp error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(useHfp), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::SendDeviceSelection Write useHfp error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(userSelection), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::SendDeviceSelection Write userSelection error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_IPC_TRANS_FAILED, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothAudioManagerInterfaceCode::BT_SEND_DEVICE_SELECTION, data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR,
        "BluetoothAudioManagerProxy::SendDeviceSelection fail, error: %{public}d", error);

    return reply.ReadInt32();
}

}  // namespace Bluetooth
}  // namespace OHOS
