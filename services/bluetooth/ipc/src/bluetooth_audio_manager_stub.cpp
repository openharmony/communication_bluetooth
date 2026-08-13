/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_audio_manager_stub"
#endif

#include "bluetooth_audio_manager_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_pan_stub.h"
#include "bluetooth_log.h"
#include "common_util.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothAudioManagerInterfaceCode::code, {&BluetoothAudioManagerStub::func, perm}

namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothAudioManagerStub::AudioManagerStubFuncPerm>
    BluetoothAudioManagerStub::memberFuncMap_ = {
    {STUB_FUNC(WEAR_DETECTION_ENABLE, EnableWearDetectionInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(WEAR_DETECTION_DISABLE, DisableWearDetectionInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(IS_WEAR_DETECTION_ENABLED, GetWearDetectionStateInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(IS_DEVICE_WEARING, IsDeviceWearingInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_IS_WEAR_DETECTION_SUPPORTED, IsWearDetectionSupportedInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_SEND_DEVICE_SELECTION, SendDeviceSelectionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GET_PROFILE_STATUS, GetProfileStatusInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothAudioManagerStub::BluetoothAudioManagerStub()
{}

BluetoothAudioManagerStub::~BluetoothAudioManagerStub()
{}

int BluetoothAudioManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothAudioManager ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothAudioManagerStub);
}

int32_t BluetoothAudioManagerStub::EnableWearDetectionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothWearDetectionStub::EnableWearDetectionInner");
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = EnableWearDetection(addr);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return NO_ERROR;
}

int32_t BluetoothAudioManagerStub::DisableWearDetectionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothWearDetectionStub::EnableWearDetectionInner");
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = DisableWearDetection(addr);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return NO_ERROR;
}

int32_t BluetoothAudioManagerStub::GetWearDetectionStateInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothWearDetectionStub::EnableWearDetectionInner");
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t ability;
    int result = GetWearDetectionState(addr, ability);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (result == NO_ERROR) {
        if (!reply.WriteInt32(ability)) {
            HILOGE("reply writing failed.");
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }
    return NO_ERROR;
}

int32_t BluetoothAudioManagerStub::IsDeviceWearingInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothAudioManagerStub::IsDeviceWearingInner");
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device, BT_ERR_INTERNAL_ERROR, "Read device failed.");
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");

    int result = IsDeviceWearing(*device);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_INTERNAL_ERROR, "reply writing failed.");
    return BT_NO_ERROR;
}

int32_t BluetoothAudioManagerStub::SendDeviceSelectionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    int useA2dp;
    int useHfp;
    int userSelect;

    CHECK_AND_RETURN_LOG_RET(device, BT_ERR_INTERNAL_ERROR, "Read device failed.");
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(useA2dp), BT_ERR_INTERNAL_ERROR, "Read useA2dp failed.");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(useHfp), BT_ERR_INTERNAL_ERROR, "Read useHfp failed.");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(userSelect), BT_ERR_INTERNAL_ERROR, "Read userSelect failed.");

    int32_t result = SendDeviceSelection(*device, useA2dp, useHfp, userSelect);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_INTERNAL_ERROR, "reply writing failed.");
    return BT_NO_ERROR;
}

int32_t BluetoothAudioManagerStub::IsWearDetectionSupportedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device, BT_ERR_INTERNAL_ERROR, "Read device failed.");
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    bool isSupported = false;
    int32_t ret = IsWearDetectionSupported(*device, isSupported);

    CHECK_AND_RETURN_LOG_RET(reply.WriteBool(isSupported), BT_ERR_INTERNAL_ERROR, "reply writing failed.");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply ret failed.");
    return BT_NO_ERROR;
}

int32_t BluetoothAudioManagerStub::GetProfileStatusInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    uint8_t a2dpState;
    uint8_t hfpState;
    CHECK_AND_RETURN_LOG_RET(device, BT_ERR_INTERNAL_ERROR, "Read device failed.");
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(device->GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    
    int32_t ret = GetProfileStatus(*device, a2dpState, hfpState);

    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply ret failed.");
    CHECK_AND_RETURN_LOG_RET(reply.WriteUint8(a2dpState), BT_ERR_INTERNAL_ERROR, "reply a2dpState failed.");
    CHECK_AND_RETURN_LOG_RET(reply.WriteUint8(hfpState), BT_ERR_INTERNAL_ERROR, "reply hfpState failed.");
    return BT_NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
