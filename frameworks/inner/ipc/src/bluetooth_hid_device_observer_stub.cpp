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
#define LOG_TAG "bt_ipc_hid_device_observer_stub"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_hid_device_observer_stub.h"

namespace OHOS {
namespace Bluetooth {
BluetoothHidDeviceObserverStub::BluetoothHidDeviceObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED)] =
        &BluetoothHidDeviceObserverStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_APP_STATUS_CHANGED)] =
        &BluetoothHidDeviceObserverStub::OnAppStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_GET_REPORT)] =
        &BluetoothHidDeviceObserverStub::OnGetReportInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_INTERRUPT_DATA)] =
        &BluetoothHidDeviceObserverStub::OnInterruptDataInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_SET_PROTOCOL)] =
        &BluetoothHidDeviceObserverStub::OnSetProtocolInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_SET_REPORT)] =
        &BluetoothHidDeviceObserverStub::OnSetReportInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_VIRTUAL_CABLE_UNPLUG)] =
        &BluetoothHidDeviceObserverStub::OnVirtualCableUnplugInner;
    HILOGD("ends.");
}

BluetoothHidDeviceObserverStub::~BluetoothHidDeviceObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothHidDeviceObserverStub::OnRemoteRequest(
    uint32_t code,
    MessageParcel &data,
    MessageParcel &reply,
    MessageOption &option)
{
    HILOGD("cmd = %{public}d, flags = %{public}d", code, option.GetFlags());
    if (BluetoothHidDeviceObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGI("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothHidDeviceObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int state = data.ReadInt32();
    OnConnectionStateChanged(*device, state);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnAppStatusChangedInner(MessageParcel &data, MessageParcel &reply)
{
    int state = data.ReadInt32();
    OnAppStatusChanged(state);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnGetReportInner(MessageParcel &data, MessageParcel &reply)
{
    int type = data.ReadInt32();
    int id = data.ReadInt32();
    uint32_t bufferSize = data.ReadUint32();
    OnGetReport(type, id, bufferSize);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnInterruptDataInner(MessageParcel &data, MessageParcel &reply)
{
    int reportId = data.ReadInt32();
    std::vector<uint8_t> interruptData = {};
    data.ReadUInt8Vector(&interruptData);
    OnInterruptDataReceived(reportId, interruptData);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnSetProtocolInner(MessageParcel &data, MessageParcel &reply)
{
    int protocol = data.ReadInt32();
    OnSetProtocol(protocol);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnSetReportInner(MessageParcel &data, MessageParcel &reply)
{
    int type = data.ReadInt32();
    int id = data.ReadInt32();
    std::vector<uint8_t> setReportData = {};
    data.ReadUInt8Vector(&setReportData);
    OnSetReport(type, id, setReportData);
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverStub::OnVirtualCableUnplugInner(MessageParcel &data, MessageParcel &reply)
{
    OnVirtualCableUnplug();
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS