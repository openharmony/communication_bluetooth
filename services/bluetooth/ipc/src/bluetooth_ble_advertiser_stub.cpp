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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_ble_advertise_stub"
#endif

#include <map>

#include "bluetooth_ble_advertiser_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) \
    BluetoothBleAdvertiserInterfaceCode::code, {&BluetoothBleAdvertiserStub::func, perm} \

namespace OHOS {
namespace Bluetooth {
namespace {
const int32_t BLE_INVALID_ADVERTISING_HANDLE = 0xFF;
}

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const BluetoothBleAdvertiserStub::BleAdvertiserStubFuncMap BluetoothBleAdvertiserStub::memberFuncMap_ = {
    {STUB_FUNC(BLE_REGISTER_BLE_ADVERTISER_CALLBACK, RegisterBleAdvertiserCallbackInner, nullptr)},
    {STUB_FUNC(BLE_DE_REGISTER_BLE_ADVERTISER_CALLBACK, DeregisterBleAdvertiserCallbackInner, nullptr)},
    {STUB_FUNC(BLE_START_ADVERTISING, StartAdvertisingInner,
        CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_ENABLE_ADVERTISING, EnableAdvertisingInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_DISABLE_ADVERTISING, DisableAdvertisingInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_STOP_ADVERTISING, StopAdvertisingInner,
        CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_CLOSE, CloseInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_GET_ADVERTISER_HANDLE, GetAdvertiserHandleInner,
        CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_SET_ADVERTISING_DATA, SetAdvertisingDataInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_CHANGE_ADVERTISING_PARAMS, ChangeAdvertisingParamsInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_GET_ADVPOWER_FOR_RANGING, GetAdvPowerForRangingBusinessInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_RESTORE_RANGING_ANT_SWITCH, RestoreRangingAntSwitchInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothBleAdvertiserStub::BluetoothBleAdvertiserStub()
{}

BluetoothBleAdvertiserStub::~BluetoothBleAdvertiserStub()
{}

int32_t BluetoothBleAdvertiserStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothBleAdvertiser ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothBleAdvertiserStub);
}

int32_t BluetoothBleAdvertiserStub::RegisterBleAdvertiserCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleAdvertiseCallback> callBack = OHOS::iface_cast<IBluetoothBleAdvertiseCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callBack != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    RegisterBleAdvertiserCallback(callBack);
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::DeregisterBleAdvertiserCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleAdvertiseCallback> callBack = OHOS::iface_cast<IBluetoothBleAdvertiseCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callBack != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    DeregisterBleAdvertiserCallback(callBack);
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::StartAdvertisingInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothBleAdvertiserSettings> settings(data.ReadParcelable<BluetoothBleAdvertiserSettings>());
    if (settings == nullptr) {
        HILOGW("read settings failed");
        return TRANSACTION_ERR;
    }

    std::shared_ptr<BluetoothBleAdvertiserData> advData(data.ReadParcelable<BluetoothBleAdvertiserData>());
    if (advData == nullptr) {
        HILOGW("read advData failed");
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothBleAdvertiserData> scanResponse(data.ReadParcelable<BluetoothBleAdvertiserData>());
    if (scanResponse == nullptr) {
        HILOGW("read scanResponse failed");
        return TRANSACTION_ERR;
    }

    int32_t advHandle = data.ReadInt32();
    uint16_t duration = data.ReadUint16();
    bool isRawData = data.ReadBool();
    int32_t ret = StartAdvertising(*settings, *advData, *scanResponse, advHandle, duration, isRawData);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::EnableAdvertisingInner(MessageParcel &data, MessageParcel &reply)
{
    uint8_t advHandle = data.ReadUint8();
    uint16_t duration = data.ReadUint16();
    int32_t ret = EnableAdvertising(advHandle, duration);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::DisableAdvertisingInner(MessageParcel &data, MessageParcel &reply)
{
    uint8_t advHandle = data.ReadUint8();
    int32_t ret = DisableAdvertising(advHandle);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::StopAdvertisingInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t advHandle = data.ReadInt32();
    int32_t ret = StopAdvertising(advHandle);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::CloseInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t advHandle = data.ReadInt32();
    Close(advHandle);
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::GetAdvertiserHandleInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleAdvertiseCallback> callBack = OHOS::iface_cast<IBluetoothBleAdvertiseCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callBack != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");

    int advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    int result = GetAdvertiserHandle(advHandle, callBack);
    bool resultRet = reply.WriteInt32(result);
    bool advHandleRet = reply.WriteInt32(advHandle);
    if (!(resultRet && advHandleRet)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::SetAdvertisingDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothBleAdvertiserData> advData(data.ReadParcelable<BluetoothBleAdvertiserData>());
    if (advData == nullptr) {
        HILOGW("read advData failed");
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothBleAdvertiserData> scanResponse(data.ReadParcelable<BluetoothBleAdvertiserData>());
    if (scanResponse == nullptr) {
        HILOGW("read scanResponse failed");
        return TRANSACTION_ERR;
    }

    int32_t advHandle = data.ReadInt32();
    int32_t advType = data.ReadInt32();
    if (advType < bluetooth::SET_ADV_DATA_BOTH || advType > bluetooth::SET_ADV_DATA_ONLY_RSP) {
        HILOGE("invalid in type");
        return ERR_INVALID_VALUE;
    }
    bluetooth::SetAdvDataType type = static_cast<bluetooth::SetAdvDataType>(advType);
    SetAdvertisingData(*advData, *scanResponse, advHandle, type);
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::ChangeAdvertisingParamsInner(MessageParcel &data, MessageParcel &reply)
{
    uint8_t advHandle = data.ReadUint8();
    std::shared_ptr<BluetoothBleAdvertiserSettings> settings(data.ReadParcelable<BluetoothBleAdvertiserSettings>());
    if (settings == nullptr) {
        HILOGE("Read settings failed.");
        return TRANSACTION_ERR;
    }
    ChangeAdvertisingParams(advHandle, *settings);
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::GetAdvPowerForRangingBusinessInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appType = data.ReadInt32();
    int8_t advPower = INNER_BLE_RANGING_INVALID_ADVPOWER;
    int32_t errorCode = GetAdvPowerForRangingBusiness(static_cast<bluetooth::BleAppType>(appType), advPower);
    if (!reply.WriteInt32(errorCode)) {
        HILOGE("reply writing errorCode failed.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(advPower)) {
        HILOGE("reply writing advPower failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleAdvertiserStub::RestoreRangingAntSwitchInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appType = data.ReadInt32();
    int32_t ret = BleRestoreRangingAntSwitch(static_cast<bluetooth::BleAppType>(appType));
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
