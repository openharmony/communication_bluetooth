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
#define LOG_TAG "bt_ipc_ble_central_proxy"
#endif

#include "bluetooth_ble_central_manager_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
BluetoothBleCentralManagerProxy::BluetoothBleCentralManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothBleCentralManager>(impl)
{}

BluetoothBleCentralManagerProxy::~BluetoothBleCentralManagerProxy()
{}

void BluetoothBleCentralManagerProxy::RegisterBleCentralManagerCallback(int32_t &scannerId, bool enableRandomAddrMode,
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: write interface token failed.");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: write callback failed.");
        return;
    }
    if (!data.WriteBool(enableRandomAddrMode)) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: write enableRandomAddrMode failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: transact ErrCode=%{public}d", result);
    }
    scannerId = reply.ReadInt32();
}

void BluetoothBleCentralManagerProxy::DeregisterBleCentralManagerCallback(int32_t scannerId,
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: write interface token failed.");
        return;
    }
    if (!data.WriteInt32(scannerId)) {
        HILOGE("[DeregisterBleCentralManagerCallback] fail: write scannerId failed");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: write callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: transact ErrCode=%{public}d", result);
    }
}

int BluetoothBleCentralManagerProxy::StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings,
    const std::vector<BluetoothBleScanFilter> &filters)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StartScan] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[StartScan] fail: write scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteParcelable(&settings)) {
        HILOGW("[StartScan] fail:write settings failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(filters.size())) {
        HILOGE("[StartScan] fail: write vector size failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    for (uint32_t i = 0; i < filters.size(); i++) {
        if (!data.WriteParcelable(&filters[i])) {
            HILOGE("[StartScan] fail: write filter failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_START_SCAN, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StartScan] fail: transact ErrCode=%{public}d", result);
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::StopScan(int32_t scannerId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StopScan] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(scannerId)) {
        HILOGE("[StopScan] fail: write scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_STOP_SCAN, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StopScan] fail: transact ErrCode=%{public}d", result);
    }
    return reply.ReadInt32();
}

void BluetoothBleCentralManagerProxy::RemoveScanFilter(int32_t scannerId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RemoveScanFilter] fail: write interface token failed.");
        return;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[RemoveScanFilter] fail: write scannerId failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_REMOVE_SCAN_FILTER, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[RemoveScanFilter] fail: transact ErrCode=%{public}d", result);
    }
}

bool BluetoothBleCentralManagerProxy::FreezeByRss(std::set<int> pidSet, bool isProxy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[FreezeByRss] fail: write interface token failed.");
        return false;
    }

    std::vector<int> pidVec(pidSet.begin(), pidSet.end());
    if (!data.WriteInt32Vector(pidVec) || !data.WriteBool(isProxy)) {
        HILOGE("[FreezeByRss] fail: write data failed");
        return false;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_FREEZE_BY_RSS, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[FreezeByRss] fail: transact ErrCode=%{public}d", result);
        return false;
    }
    return true;
}

bool BluetoothBleCentralManagerProxy::ResetAllProxy()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[ResetAllProxy] fail: write interface token failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_RESET_ALL_PROXY, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[ResetAllProxy] fail: transact ErrCode=%{public}d", result);
        return false;
    }
    return true;
}

int BluetoothBleCentralManagerProxy::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval,
    int advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetLpDeviceAdvParam] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(duration)) {
        HILOGE("[SetLpDeviceAdvParam] fail: write duration failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(maxExtAdvEvents)) {
        HILOGE("[SetLpDeviceAdvParam] fail: write maxExtAdvEvents failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(window)) {
        HILOGE("[SetLpDeviceAdvParam] fail: write window failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(interval)) {
        HILOGE("[SetLpDeviceAdvParam] fail: write interval failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(advHandle)) {
        HILOGE("[SetLpDeviceAdvParam] fail: write advHandle failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_SET_LPDEVICE_ADV_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetLpDeviceAdvParam] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = reply.ReadInt32();
    return ret;
}

int BluetoothBleCentralManagerProxy::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetScanReportChannelToLpDevice] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[SetScanReportChannelToLpDevice] fail: write scannerId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteBool(enable)) {
        HILOGE("[SetScanReportChannelToLpDevice] fail: write enable failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_SET_SCAN_REPORT_CHANNEL_TO_LPDEVICE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetScanReportChannelToLpDevice] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = reply.ReadInt32();
    return ret;
}

int BluetoothBleCentralManagerProxy::EnableSyncDataToLpDevice()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[EnableSyncDataToLpDevice] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_ENABLE_SYNC_DATA_TO_LPDEVICE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[EnableSyncDataToLpDevice] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::DisableSyncDataToLpDevice()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[DisableSyncDataToLpDevice] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_DISABLE_SYNC_DATA_TO_LPDEVICE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[DisableSyncDataToLpDevice] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SendParamsToLpDevice] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUInt8Vector(dataValue)) {
        HILOGE("[SendParamsToLpDevice] fail: write dataValue failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(type)) {
        HILOGE("[SendParamsToLpDevice] fail: write type failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_SEND_PARAMS_TO_LPDEVICE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SendParamsToLpDevice] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

bool BluetoothBleCentralManagerProxy::IsLpDeviceAvailable()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[IsLpDeviceAvailable] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_IS_LPDEVICE_AVAILABLE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[IsLpDeviceAvailable] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadBool();
}

int WriteScanParcelData(const BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btScanSettings)) {
            HILOGE("[WriteScanParcelData] fail:write settings failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) != 0) {
        if (!data.WriteInt32(paramSet.btScanFilters.size())) {
            HILOGE("[WriteScanParcelData] fail: write filtersize failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        for (uint32_t i = 0; i < paramSet.btScanFilters.size(); i++) {
            if (!data.WriteParcelable(&paramSet.btScanFilters[i])) {
                HILOGE("[WriteScanParcelData] fail: write filter failed");
                return BT_ERR_INTERNAL_ERROR;
            }
        }
    }
    return BT_NO_ERROR;
}

int WriteAdvertiserParcelData(const BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btAdvSettings)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write settings failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btAdvData)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write advData failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btRespData)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write scanResponse failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

int WriteActiveDeviceInfoParcelData(const BluetoothLpDeviceParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
        if (!data.WriteInt32(paramSet.activeDeviceInfos.size())) {
            HILOGE("[WriteActiveDeviceInfoParcelData] fail: write activeDeviceInfos failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        for (uint32_t i = 0; i < paramSet.activeDeviceInfos.size(); i++) {
            if (!data.WriteInt8Vector(paramSet.activeDeviceInfos[i].deviceId)) {
                HILOGE("[WriteActiveDeviceInfoParcelData] fail: write deviceId failed.");
                return BT_ERR_INTERNAL_ERROR;
            }
            if (!data.WriteInt32(paramSet.activeDeviceInfos[i].status)) {
                HILOGE("[WriteActiveDeviceInfoParcelData] fail: write status failed");
                return BT_ERR_INTERNAL_ERROR;
            }
            if (!data.WriteInt32(paramSet.activeDeviceInfos[i].timeOut)) {
                HILOGE("[WriteActiveDeviceInfoParcelData] fail: write timeOut failed");
                return BT_ERR_INTERNAL_ERROR;
            }
        }
    }
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerProxy::SetLpDeviceParam(const BluetoothLpDeviceParamSet &paramSet)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetLpDeviceParam] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(paramSet.fieldValidFlagBit)) {
        HILOGE("[SetLpDeviceParam] fail: write fieldValidFlagBit failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = WriteScanParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    ret = WriteAdvertiserParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    ret = WriteActiveDeviceInfoParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    BluetoothUuid btUuid(paramSet.uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("[SetLpDeviceParam] fail:write uuid failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.advHandle)) {
        HILOGE("[SetLpDeviceParam] fail: write advHandle failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.duration)) {
        HILOGE("[SetLpDeviceParam] fail: write duration failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.deliveryMode)) {
        HILOGE("[SetLpDeviceParam] fail: write deliveryMode failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_SET_LPDEVICE_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetLpDeviceParam] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::RemoveLpDeviceParam(const bluetooth::Uuid &uuid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RemoveLpDeviceParam] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("[RemoveLpDeviceParam] fail: write uuid failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_REMOVE_LPDEVICE_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[RemoveLpDeviceParam] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings &settings,
    const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[ChangeScanParams] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGW("[ChangeScanParams] fail: write scannerId failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteParcelable(&settings)) {
        HILOGW("[ChangeScanParams] fail: write settings failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(filters.size())) {
        HILOGW("[ChangeScanParams] fail: write filters size failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    for (size_t i = 0; i < filters.size(); i++) {
        if (!data.WriteParcelable(&filters[i])) {
            HILOGW("[ChangeScanParams] fail: write filters failed.");
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    if (!data.WriteUint32(filterAction)) {
        HILOGW("[ChangeScanParams] fail: write filterAction failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_CHANGE_SCAN_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[ChangeScanParams] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int32_t BluetoothBleCentralManagerProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return NO_ERROR;
        }
        case DEAD_OBJECT: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return DEAD_OBJECT;
        }
        default: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS