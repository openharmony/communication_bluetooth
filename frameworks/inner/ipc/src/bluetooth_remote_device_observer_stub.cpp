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

#include "bluetooth_remote_device_observer_stub.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
constexpr uint16_t UUID_SIZE_MAX = 1024;
const std::map<uint32_t, std::function<ErrCode(BluetoothRemoteDeviceObserverstub *, MessageParcel &, MessageParcel &)>>
    BluetoothRemoteDeviceObserverstub::memberFuncMap_ = {
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_ACL_STATE,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnAclStateChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_PAIR_STATUS,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnPairStatusChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_UUID,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_NAME,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteNameChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_ALIAS,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_COD,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_LEVEL,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteBatteryChangedInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {BluetoothRemoteDeviceObserverInterfaceCode::BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_INFO_REPORT,
            std::bind(&BluetoothRemoteDeviceObserverstub::OnRemoteDeviceCommonInfoReportInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
};

BluetoothRemoteDeviceObserverstub::BluetoothRemoteDeviceObserverstub()
{
    HILOGD("start.");
}

BluetoothRemoteDeviceObserverstub::~BluetoothRemoteDeviceObserverstub()
{
    HILOGD("start.");
}

int BluetoothRemoteDeviceObserverstub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("OnRemoteRequest, cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothRemoteDeviceObserverstub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("BluetoothHostStub::OnRemoteRequest, local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothRemoteDeviceObserverstub::OnAclStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    int32_t state = data.ReadInt32();
    uint32_t reason = data.ReadUint32();
    OnAclStateChanged(*result, state, reason);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnPairStatusChangedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    int32_t status = data.ReadInt32();
    int32_t cause = data.ReadInt32();
    OnPairStatusChanged(transport, *result, status, cause);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    uint16_t uuidSize = data.ReadUint16();
    if (uuidSize > UUID_SIZE_MAX) {
        HILOGE("uuidSize = %{public}hu exceeds the maximum 1024.", uuidSize);
        return INVALID_DATA;
    }
    std::vector<bluetooth::Uuid> uuids;
    for (uint16_t i = 0; i < uuidSize; ++i) {
        std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
        if (!uuid) {
            return TRANSACTION_ERR;
        }
        uuids.push_back(*uuid);
    }
    OnRemoteUuidChanged(*result, uuids);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteNameChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    std::string deviceName = data.ReadString();
    OnRemoteNameChanged(*result, deviceName);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    std::string alias = data.ReadString();
    OnRemoteAliasChanged(*result, alias);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> result(data.ReadParcelable<BluetoothRawAddress>());
    if (!result) {
        return TRANSACTION_ERR;
    }
    int32_t cod = data.ReadInt32();
    OnRemoteCodChanged(*result, cod);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteBatteryChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET((device != nullptr), BT_ERR_INTERNAL_ERROR, "Read device error");
    std::shared_ptr<BluetoothBatteryInfo> batteryInfo(data.ReadParcelable<BluetoothBatteryInfo>());
    CHECK_AND_RETURN_LOG_RET((batteryInfo != nullptr), BT_ERR_INTERNAL_ERROR, "Read batteryInfo error");

    OnRemoteBatteryChanged(*device, *batteryInfo);
    return BT_NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteDeviceCommonInfoReportInner(MessageParcel &data,
    MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET((device != nullptr), BT_ERR_INTERNAL_ERROR, "Read device error");
    std::vector<uint8_t> dataValue;
    CHECK_AND_RETURN_LOG_RET(data.ReadUInt8Vector(&dataValue), BT_ERR_INTERNAL_ERROR, "Read dataValue error");

    OnRemoteDeviceCommonInfoReport(*device, dataValue);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS