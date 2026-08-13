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
#define LOG_TAG "bt_ipc_opp_stub"
#endif

#include "bluetooth_opp_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothOppInterfaceCode::code, {&BluetoothOppStub::func, perm}
namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

const int32_t OPP_FILE_SIZE_MAX = 300;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothOppStub::OppStubFuncPerm> BluetoothOppStub::memberFuncMap_ = {
    {STUB_FUNC(COMMAND_SEND_FILE, SendFileInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_SET_INCOMING_FILE_CONFIRMATION, SetIncomingFileConfirmationInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_CURRENT_TRANSFER_INFORMATION, GetCurrentTransferInformationInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_CANCEL_TRANSFER, CancelTransferInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_REGISTER_OBSERVER, RegisterObserverInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_DEREGISTER_OBSERVER, DeregisterObserverInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_DEVICE_STATE, GetDeviceStateInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_GET_DEVICES_BY_STATES, GetDevicesByStatesInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(COMMAND_SET_LAST_RECEIVED_URI, SetLastReceivedFileUriInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothOppStub::BluetoothOppStub()
{}

BluetoothOppStub::~BluetoothOppStub()
{}

int32_t BluetoothOppStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothOpp ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothOppStub);
};


int32_t BluetoothOppStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device != nullptr, BT_ERR_INTERNAL_ERROR, "Read BluetoothRawAddress failed");

    int32_t state;
    int32_t ret = GetDeviceState(*device, state);

    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(state), BT_ERR_INTERNAL_ERROR, "reply write state failed");
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> state;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32Vector(&state), BT_ERR_INTERNAL_ERROR, "Read state failed");

    std::vector<BluetoothRawAddress> rawAdds;
    int32_t ret = GetDevicesByStates(state, rawAdds);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(rawAdds.size()), BT_ERR_INTERNAL_ERROR, "reply write devices failed");

    // write devices
    for (auto rawAdd : rawAdds) {
        if (!reply.WriteParcelable(&rawAdd)) {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return ret;
}

int32_t BluetoothOppStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothOppObserver> observer = OHOS::iface_cast<IBluetoothOppObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothOppObserver> observer = OHOS::iface_cast<IBluetoothOppObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::SendFileInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = RET_NO_ERROR;
    std::string device = data.ReadString();
    bool result;
    int32_t fileSize = data.ReadInt32();
    if (fileSize < 0 || fileSize > OPP_FILE_SIZE_MAX) {
        reply.WriteInt32(BT_ERR_INVALID_PARAM);
        return BT_ERR_INVALID_PARAM;
    }
    std::vector<BluetoothIOppTransferFileHolder> fileHolders;
    for (int32_t i = 0; i < fileSize; i++) {
        std::string filePath = "";
        data.ReadString(filePath);
        int64_t fileSize = 0;
        data.ReadInt64(fileSize);
        if (fileSize < 0) {
            reply.WriteInt32(BT_ERR_INVALID_PARAM);
            return BT_ERR_INVALID_PARAM;
        }
        int fileFd = 0;
        fileFd = data.ReadFileDescriptor();
        if (fileFd < 0) {
            HILOGE("fileFd is invaild %{public}d", fileFd);
            reply.WriteInt32(BT_ERR_INVALID_PARAM);
            return BT_ERR_INVALID_PARAM;
        }
        fileHolders.push_back(BluetoothIOppTransferFileHolder(filePath, fileSize, fileFd));
    }
    ret = SendFile(device, fileHolders, result);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::SetIncomingFileConfirmationInner(MessageParcel &data, MessageParcel &reply)
{
    bool accept = data.ReadBool();
    int fd = data.ReadFileDescriptor();
    HILOGI("BluetoothOppStub::SetIncomingFileConfirmationInner fd is %{public}d", fd);
    int32_t ret = SetIncomingFileConfirmation(accept, fd);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::SetLastReceivedFileUriInner(MessageParcel &data, MessageParcel &reply)
{
    std::string uri = data.ReadString();
    int32_t ret = SetLastReceivedFileUri(uri);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::GetCurrentTransferInformationInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothOppStub::GetCurrentTransferInformationInner");
    BluetoothIOppTransferInformation oppInformation;
    int32_t ret = GetCurrentTransferInformation(oppInformation);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    CHECK_AND_RETURN_LOG_RET(reply.WriteParcelable(&oppInformation),
        BT_ERR_INTERNAL_ERROR, "reply write GetCurrentTransferInformation failed");
    return BT_NO_ERROR;
}

int32_t BluetoothOppStub::CancelTransferInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CancelTransfer();
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "reply write ret failed");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR, "internal error");
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS