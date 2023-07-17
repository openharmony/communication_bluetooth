/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "string_ex.h"

namespace OHOS {
namespace Bluetooth {
const int32_t BLE_CENTRAL_MANAGER_READ_DATA_SIZE_MAX_LEN = 0x100;
const std::map<uint32_t,
    std::function<ErrCode(BluetoothBleCentralManagerCallBackStub *, MessageParcel &, MessageParcel &)>>
    BluetoothBleCentralManagerCallBackStub::memberFuncMap_ = {
        {BluetoothBleCentralManagerCallbackInterfaceCode::BT_BLE_CENTRAL_MANAGER_CALLBACK,
            std::bind(&BluetoothBleCentralManagerCallBackStub::OnScanCallbackInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothBleCentralManagerCallbackInterfaceCode::BT_BLE_CENTRAL_MANAGER_BLE_BATCH_CALLBACK,
            std::bind(&BluetoothBleCentralManagerCallBackStub::OnBleBatchScanResultsEventInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothBleCentralManagerCallbackInterfaceCode::BT_BLE_CENTRAL_MANAGER_CALLBACK_SCAN_FAILED,
            std::bind(&BluetoothBleCentralManagerCallBackStub::OnStartOrStopScanEventInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothBleCentralManagerCallbackInterfaceCode::BT_BLE_LPDEVICE_CALLBACK_NOTIFY_MSG_REPORT,
            std::bind(&BluetoothBleCentralManagerCallBackStub::OnNotifyMsgReportFromLpDeviceInner,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
};

BluetoothBleCentralManagerCallBackStub::BluetoothBleCentralManagerCallBackStub()
{
    HILOGI("start.");
}

BluetoothBleCentralManagerCallBackStub::~BluetoothBleCentralManagerCallBackStub()
{
    HILOGI("start.");
}

int BluetoothBleCentralManagerCallBackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothBleCentralManagerCallBackStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    if (BluetoothBleCentralManagerCallBackStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }
    HILOGW("BluetoothBleCentralManagerCallBackStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothBleCentralManagerCallBackStub::OnScanCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothBleScanResult> result(data.ReadParcelable<BluetoothBleScanResult>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    OnScanCallback(*result);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerCallBackStub::OnBleBatchScanResultsEventInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t infoSize = 0;
    if (!data.ReadInt32(infoSize) || infoSize > BLE_CENTRAL_MANAGER_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return ERR_INVALID_STATE;
    }

    std::vector<BluetoothBleScanResult> parcelableInfos;
    for (int32_t index = 0; index < infoSize; index++) {
        sptr<BluetoothBleScanResult> info(data.ReadParcelable<BluetoothBleScanResult>());
        if (info == nullptr) {
            HILOGI("read Parcelable infos failed.");
            return ERR_INVALID_STATE;
        }
        parcelableInfos.emplace_back(*info);
    }
    OnBleBatchScanResultsEvent(parcelableInfos);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerCallBackStub::OnStartOrStopScanEventInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    bool isStartScan = data.ReadBool();
    OnStartOrStopScanEvent(resultCode, isStartScan);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerCallBackStub::OnNotifyMsgReportFromLpDeviceInner(MessageParcel &data,
    MessageParcel &reply)
{
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("[OnNotifyMsgReportFromLpDeviceInner] read uuid failed");
        return ERR_INVALID_VALUE;
    }
    bluetooth::Uuid btUuid = bluetooth::Uuid(*uuid);
    int32_t msgType = data.ReadInt32();
    std::vector<uint8_t> dataValue;
    if (!data.ReadUInt8Vector(&dataValue)) {
        HILOGE("[OnNotifyMsgReportFromLpDeviceInner] read dataValue failed");
        return ERR_INVALID_VALUE;
    }
    OnNotifyMsgReportFromLpDevice(btUuid, msgType, dataValue);
    return NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
