/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#include "bluetooth_errorcode.h"
#include "bluetooth_audio_manager_proxy.h"
#include "bluetooth_log.h"
#include "refbase.h"
 
namespace OHOS {
namespace Bluetooth {
 
int BluetoothAudioManagerProxy::EnableBtAudioManager(const std::string &deviceId, int32_t supportVal)
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
 
    if (!data.WriteInt32(supportVal)) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetection write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
 
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothAudioManagerInterfaceCode::AUDIO_MANAGER_ENABLE), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetections done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return 0;
}
 
int BluetoothAudioManagerProxy::DisableBtAudioManager(const std::string &deviceId, int32_t supportVal)
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
 
    if (!data.WriteInt32(supportVal)) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetection write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
 
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothAudioManagerInterfaceCode::AUDIO_MANAGER_DISABLE), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetections done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return 0;
}
 
int BluetoothAudioManagerProxy::IsBtAudioManagerEnabled(const std::string &deviceId)
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
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothAudioManagerInterfaceCode::IS_AUDIO_MANAGER_ENABLED), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothWearDetectionProxy::EnableWearDetections done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return 0;
}
}  // namespace Bluetooth
}  // namespace OHOS