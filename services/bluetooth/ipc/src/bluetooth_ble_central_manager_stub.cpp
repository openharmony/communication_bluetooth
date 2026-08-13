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
#define LOG_TAG "bt_ipc_ble_central_manager_stub"
#endif

#include <map>
#include <vector>

#include "bluetooth_ble_central_manager_stub.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) \
    BluetoothBleCentralManagerInterfaceCode::code, {&BluetoothBleCentralManagerStub::func, perm}

namespace OHOS {
namespace Bluetooth {

const int32_t BLE_CENTRAL_MANAGER_STUB_READ_DATA_SIZE_MAX_LEN = 0x100;
const int32_t BLE_CENTRAL_MANAGER_STUB_ADV_DEVICE_MAX_NUM = 0xFF;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const BluetoothBleCentralManagerStub::BleCentralManagerStubFuncMap BluetoothBleCentralManagerStub::memberFuncMap_ = {
    {STUB_FUNC(BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, RegisterBleCentralManagerCallbackInner, nullptr)},
    {STUB_FUNC(BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, DeregisterBleCentralManagerCallbackInner, nullptr)},
    {STUB_FUNC(BLE_START_SCAN, StartScanInner, nullptr)},
    {STUB_FUNC(BLE_REMOVE_SCAN_FILTER, RemoveScanFilterInner,
        CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_STOP_SCAN, StopScanInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BLE_FREEZE_BY_RSS, FreezeByRssInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_RESET_ALL_PROXY, ResetAllProxyInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_SET_LPDEVICE_ADV_PARAM, SetLpDeviceAdvParamInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_SET_SCAN_REPORT_CHANNEL_TO_LPDEVICE, SetScanReportChannelToLpDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_ENABLE_SYNC_DATA_TO_LPDEVICE, EnableSyncDataToLpDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_DISABLE_SYNC_DATA_TO_LPDEVICE, DisableSyncDataToLpDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_SEND_PARAMS_TO_LPDEVICE, SendParamsToLpDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_IS_LPDEVICE_AVAILABLE, IsLpDeviceAvailableInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_SET_LPDEVICE_PARAM, SetLpDeviceParamInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_REMOVE_LPDEVICE_PARAM, RemoveLpDeviceParamInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_CHANGE_SCAN_PARAM, ChangeScanParamsInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BLE_IS_VALID_SCANNERID, IsValidScannerIdInner, nullptr)},
    {STUB_FUNC(BLE_FLUSH_BATCH_SCAN_RESULTS, FlushBatchScanResultsInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothBleCentralManagerStub::BluetoothBleCentralManagerStub()
{}

BluetoothBleCentralManagerStub::~BluetoothBleCentralManagerStub()
{}

int32_t BluetoothBleCentralManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothBleCentralManager ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothBleCentralManagerStub);
}

int32_t BluetoothBleCentralManagerStub::RegisterBleCentralManagerCallbackInner(
    MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleCentralManagerCallback> callBack =
        OHOS::iface_cast<IBluetoothBleCentralManagerCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callBack != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    bool enableRandomAddrMode = data.ReadBool();
    int32_t scannerId = 0;
    RegisterBleCentralManagerCallback(scannerId, enableRandomAddrMode, callBack);
    if (!reply.WriteInt32(scannerId)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::DeregisterBleCentralManagerCallbackInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleCentralManagerCallback> callBack =
        OHOS::iface_cast<IBluetoothBleCentralManagerCallback>(remote);
    CHECK_AND_RETURN_LOG_RET(callBack != nullptr, ERR_INVALID_VALUE, "callBack is nullptr");
    DeregisterBleCentralManagerCallback(scannerId, callBack);
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::StartScanInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    std::shared_ptr<BluetoothBleScanSettings> settings(data.ReadParcelable<BluetoothBleScanSettings>());
    if (settings == nullptr) {
        HILOGW("[StartScanWithSettingsInner] fail: read settings failed");
        return TRANSACTION_ERR;
    }

    std::vector<BluetoothBleScanFilter> filters {};
    int32_t itemsSize = 0;
    if (!data.ReadInt32(itemsSize) || itemsSize > BLE_CENTRAL_MANAGER_STUB_READ_DATA_SIZE_MAX_LEN || itemsSize < 0) {
        HILOGE("read Parcelable size failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    for (int32_t i = 0; i < itemsSize; i++) {
        std::shared_ptr<BluetoothBleScanFilter> res(data.ReadParcelable<BluetoothBleScanFilter>());
        if (res == nullptr) {
            HILOGE("null pointer");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        BluetoothBleScanFilter item = *(res);
        filters.push_back(item);
    }

    bool isNewApi = data.ReadBool();
    int32_t ret = StartScan(scannerId, *settings, filters, isNewApi);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::StopScanInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    int32_t ret = StopScan(scannerId);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::RemoveScanFilterInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();

    RemoveScanFilter(scannerId);
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::FreezeByRssInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int> pidVec {};
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32Vector(&pidVec), BT_ERR_IPC_TRANS_FAILED, "ipc failed");
    bool isProxy = data.ReadBool();
    uint8_t freezeType = data.ReadUint8();

    std::set<int> pidSet(pidVec.begin(), pidVec.end());
    bool ret = FreezeByRss(pidSet, isProxy, freezeType);
    if (!reply.WriteBool(ret)) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::ResetAllProxyInner(MessageParcel &data, MessageParcel &reply)
{
    bool ret = ResetAllProxy();
    if (!reply.WriteBool(ret)) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::SetLpDeviceAdvParamInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t duration = data.ReadInt32();
    int32_t maxExtAdvEvents = data.ReadInt32();
    int32_t window = data.ReadInt32();
    int32_t interval = data.ReadInt32();
    int32_t advHandle = data.ReadInt32();
    int32_t ret = SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::SetScanReportChannelToLpDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    bool enable = data.ReadBool();
    int32_t ret = SetScanReportChannelToLpDevice(scannerId, enable);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::EnableSyncDataToLpDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = EnableSyncDataToLpDevice();
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::DisableSyncDataToLpDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = DisableSyncDataToLpDevice();
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::SendParamsToLpDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint8_t> dataValue;
    if (!data.ReadUInt8Vector(&dataValue)) {
        HILOGE("read dataValue failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t type = data.ReadInt32();
    int32_t ret = SendParamsToLpDevice(std::move(dataValue), type);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::IsLpDeviceAvailableInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsLpDeviceAvailable();
    if (!reply.WriteBool(result)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t ReadScanParcelData(BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) != 0) {
        std::shared_ptr<BluetoothBleScanSettings> scanSettings(data.ReadParcelable<BluetoothBleScanSettings>());
        if (scanSettings == nullptr) {
            HILOGE("[ReadScanParcelData]: read settings failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        paramSet.btScanSettings = *scanSettings;
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) != 0) {
        int32_t itemsSize = 0;
        if (!data.ReadInt32(itemsSize) || itemsSize > BLE_CENTRAL_MANAGER_STUB_READ_DATA_SIZE_MAX_LEN
            || itemsSize < 0) {
            HILOGE("[ReadScanParcelData]: read Parcelable size failed.");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        HILOGD("filtersize: %{public}d", itemsSize);
        for (int32_t i = 0; i < itemsSize; i++) {
            std::shared_ptr<BluetoothBleScanFilter> res(data.ReadParcelable<BluetoothBleScanFilter>());
            if (res == nullptr) {
                HILOGE("[ReadScanParcelData]: read Parcelable filter failed.");
                return BT_ERR_IPC_TRANS_FAILED;
            }
            BluetoothBleScanFilter item = *(res);
            paramSet.btScanFilters.push_back(item);
        }
    }
    return BT_NO_ERROR;
}

int32_t ReadAdvertiserParcelData(BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        std::shared_ptr<BluetoothBleAdvertiserSettings> advSettings(
            data.ReadParcelable<BluetoothBleAdvertiserSettings>());
        if (advSettings == nullptr) {
            HILOGE("[ReadAdvertiserParcelData] fail: read settings failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        paramSet.btAdvSettings = *advSettings;
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        std::shared_ptr<BluetoothBleAdvertiserData> advData(data.ReadParcelable<BluetoothBleAdvertiserData>());
        if (advData == nullptr) {
            HILOGE("[ReadAdvertiserParcelData] fail: read advData failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        paramSet.btAdvData.SetPayload(advData->GetPayload());
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        std::shared_ptr<BluetoothBleAdvertiserData> scanResponse(data.ReadParcelable<BluetoothBleAdvertiserData>());
        if (scanResponse == nullptr) {
            HILOGE("[ReadAdvertiserParcelData] fail: read scanResponse failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        paramSet.btRespData.SetPayload(scanResponse->GetPayload());
    }
    return BT_NO_ERROR;
}

int32_t ReadAdvDeviceInfoParcelData(BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
        int32_t deviceSize = 0;
        if (!data.ReadInt32(deviceSize) || deviceSize > BLE_CENTRAL_MANAGER_STUB_ADV_DEVICE_MAX_NUM
            || deviceSize < 0) {
            HILOGE("[ReadAdvDeviceInfoParcelData]: read Parcelable size failed.");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        for (int32_t i = 0; i < deviceSize; i++) {
            BluetoothActiveDeviceInfo deviceInfo;
            if (!data.ReadInt8Vector(&deviceInfo.deviceId)) {
                HILOGE("read dataValue failed");
                return BT_ERR_IPC_TRANS_FAILED;
            }
            deviceInfo.status = data.ReadInt32();
            deviceInfo.timeOut = data.ReadInt32();
            paramSet.activeDeviceInfos.push_back(deviceInfo);
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::SetLpDeviceParamInner(MessageParcel &data, MessageParcel &reply)
{
    BluetoothLpDeviceParamSet paramSet;
    paramSet.fieldValidFlagBit = data.ReadUint32();
    int32_t ret = ReadScanParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    ret = ReadAdvertiserParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    ret = ReadAdvDeviceInfoParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("[SetLpDeviceParamInner] fail: read uuid failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    paramSet.uuid = bluetooth::Uuid(*uuid);
    paramSet.advHandle = data.ReadInt32();
    paramSet.duration = data.ReadInt32();
    paramSet.deliveryMode = data.ReadInt32();

    ret = SetLpDeviceParam(paramSet);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::RemoveLpDeviceParamInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (uuid == nullptr) {
        HILOGE("fail: read uuid failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bluetooth::Uuid btUuid = bluetooth::Uuid(*uuid);
    int32_t ret = RemoveLpDeviceParam(btUuid);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::ChangeScanParamsInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    std::shared_ptr<BluetoothBleScanSettings> settings(data.ReadParcelable<BluetoothBleScanSettings>());
    CHECK_AND_RETURN_LOG_RET(settings != nullptr, BT_ERR_IPC_TRANS_FAILED, "settings is nullptr");

    std::vector<BluetoothBleScanFilter> filters{};
    int32_t filterSize = 0;
    if (!data.ReadInt32(filterSize) || filterSize > BLE_CENTRAL_MANAGER_STUB_READ_DATA_SIZE_MAX_LEN
        || filterSize < 0) {
        HILOGE("read Parcelable size failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    for (int32_t i = 0; i < filterSize; i++) {
        std::shared_ptr<BluetoothBleScanFilter> filter(data.ReadParcelable<BluetoothBleScanFilter>());
        if (filter == nullptr) {
            HILOGE("filter null pointer");
            return BT_ERR_IPC_TRANS_FAILED;
        }
        BluetoothBleScanFilter item = *filter;
        filters.push_back(item);
    }
    uint32_t filterAction = 0;
    if (!data.ReadUint32(filterAction)) {
        HILOGE("read Parcelable filterAction failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t ret = ChangeScanParams(scannerId, *settings, filters, filterAction);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::FlushBatchScanResultsInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    int32_t ret = FlushBatchScanResults(scannerId);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothBleCentralManagerStub::IsValidScannerIdInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scannerId = data.ReadInt32();
    bool isValid = true;
    int32_t ret = IsValidScannerId(scannerId, isValid);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    if (!reply.WriteBool(isValid)) {
        HILOGE("reply writing failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
