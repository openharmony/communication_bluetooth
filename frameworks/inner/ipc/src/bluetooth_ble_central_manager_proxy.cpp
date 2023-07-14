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

void BluetoothBleCentralManagerProxy::RegisterBleCentralManagerCallback(int32_t &scannerId,
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
        HILOGE("[DeregisterBleCentralManagerCallback] fail: write scannerId failed.");
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

int BluetoothBleCentralManagerProxy::StartScan(int32_t scannerId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StartScan] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[StartScan] fail: write scannerId failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_START_SCAN, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StartScan] fail: transact ErrCode=%{public}d", result);
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StartScan] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[StartScan] fail: write scannerId failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteParcelable(&settings)) {
        HILOGW("[StartScan] fail:write settings failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_START_SCAN_WITH_SETTINGS, option, data, reply);
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
        HILOGE("[StopScan] fail: write scannerId failed.");
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

int BluetoothBleCentralManagerProxy::ConfigScanFilter(
    int32_t scannerId, const std::vector<BluetoothBleScanFilter> &filters)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[ConfigScanFilter] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[ConfigScanFilter] fail: write clientId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(filters.size())) {
        HILOGE("[ConfigScanFilter] fail: write vector size failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    for (uint32_t i = 0; i < filters.size(); i++) {
        if (!data.WriteParcelable(&filters[i])) {
            HILOGE("[ConfigScanFilter] fail: write filter failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_CONFIG_SCAN_FILTER, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[ConfigScanFilter] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = reply.ReadInt32();
    scannerId = reply.ReadInt32();
    return ret;
}

void BluetoothBleCentralManagerProxy::RemoveScanFilter(int32_t scannerId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RemoveScanFilter] fail: write interface token failed.");
        return;
    }

    if (!data.WriteInt32(scannerId)) {
        HILOGE("[RemoveScanFilter] fail: write clientId failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_REMOVE_SCAN_FILTER, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[RemoveScanFilter] fail: transact ErrCode=%{public}d", result);
    }
}

bool BluetoothBleCentralManagerProxy::ProxyUid(int32_t uid, bool isProxy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[ProxyUid] fail: write interface token failed.");
        return false;
    }

    if (!data.WriteInt32(uid) || !data.WriteBool(isProxy)) {
        HILOGE("[ProxyUid] fail: write data failed");
        return false;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = InnerTransact(BLE_PROXY_UID, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[ProxyUid] fail: transact ErrCode=%{public}d", result);
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
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = InnerTransact(BLE_RESET_ALL_PROXY, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[ResetAllProxy] fail: transact ErrCode=%{public}d", result);
        return false;
    }
    return true;
}

int BluetoothBleCentralManagerProxy::SetBurstParam(int duration, int maxExtAdvEvents, int burstWindow,
    int burstInterval, int advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetBurstParam] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(duration)) {
        HILOGE("[SetBurstParam] fail: write duration failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(maxExtAdvEvents)) {
        HILOGE("[SetBurstParam] fail: write maxExtAdvEvents failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(burstWindow)) {
        HILOGE("[SetBurstParam] fail: write burstWindow failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(burstInterval)) {
        HILOGE("[SetBurstParam] fail: write burstInterval failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(advHandle)) {
        HILOGE("[SetBurstParam] fail: write advHandle failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_SET_BURST_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetBurstParam] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = reply.ReadInt32();
    return ret;
}

int BluetoothBleCentralManagerProxy::SetScanReportChannelToSensorHub(const int clientId, const int isToAp)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetScanReportChannelToSensorHub] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(clientId)) {
        HILOGE("[SetScanReportChannelToSensorHub] fail: write clientId failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(isToAp)) {
        HILOGE("[SetScanReportChannelToSensorHub] fail: write isAdd failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t result = InnerTransact(BLE_SET_SCAN_REPORT_CHANNEL_TO_SH, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetScanReportChannelToSensorHub] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = reply.ReadInt32();
    return ret;
}

int BluetoothBleCentralManagerProxy::StartScanInShSync()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[StartScanInShSync] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_START_SCAN_IN_SH_SYNC, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[StartScanInShSync] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::StopScanInShSync()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[StopScanInShSync] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_STOP_SCAN_IN_SH_SYNC, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[StopScanInShSync] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::SendParamsToSensorhub(const std::vector<uint8_t> &dataValue, int32_t type)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SendParamsToSensorhub] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUInt8Vector(dataValue)) {
        HILOGE("[SendParamsToSensorhub] fail: write dataValue failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(type)) {
        HILOGE("[SendParamsToSensorhub] fail: write type failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_SEND_PARAMS_TO_SH, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SendParamsToSensorhub] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

bool BluetoothBleCentralManagerProxy::IsSupportSensorAdvertiseFilter()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[IsSupportSensorAdvertiseFilter] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_IS_SUPPORT_SENSOR_ADVERTISER_FILTER, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[IsSupportSensorAdvertiseFilter] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadBool();
}

int WriteScanParcelData(const BluetoothBleFilterParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_SCAN_SETTING_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btScanSettings)) {
            HILOGE("[WriteScanParcelData] fail:write settings failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_SCAN_FILTER_VALID_BIT) != 0) {
        HILOGD("filtersize: %{public}zu", paramSet.btScanFilters.size());
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

int WriteAdvertiserParcelData(const BluetoothBleFilterParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_ADV_SETTING_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btAdvSettings)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write settings failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_ADVDATA_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btAdvData)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write advData failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }

    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_RESPDATA_VALID_BIT) != 0) {
        if (!data.WriteParcelable(&paramSet.btRespData)) {
            HILOGE("[WriteAdvertiserParcelData] fail:write scanResponse failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return BT_NO_ERROR;
}

int WriteAdvDeviceInfoParcelData(const BluetoothBleFilterParamSet &paramSet, MessageParcel &data)
{
    if ((paramSet.fieldValidFlagBit & bluetooth::BLE_SH_ADV_DEVICEINFO_VALID_BIT) != 0) {
        if (!data.WriteInt32(paramSet.advDeviceInfos.size())) {
            HILOGE("[WriteAdvDeviceInfoParcelData] fail: write advDeviceInfos failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        for (uint32_t i = 0; i < paramSet.advDeviceInfos.size(); i++) {
            if (!data.WriteInt8Vector(paramSet.advDeviceInfos[i].advDeviceId)) {
                HILOGE("[WriteAdvDeviceInfoParcelData] fail: write advDeviceId failed.");
                return BT_ERR_INTERNAL_ERROR;
            }
            if (!data.WriteInt32(paramSet.advDeviceInfos[i].status)) {
                HILOGE("[WriteAdvDeviceInfoParcelData] fail: write status failed");
                return BT_ERR_INTERNAL_ERROR;
            }
            if (!data.WriteInt32(paramSet.advDeviceInfos[i].timeOut)) {
                HILOGE("[WriteAdvDeviceInfoParcelData] fail: write timeOut failed");
                return BT_ERR_INTERNAL_ERROR;
            }
        }
    }
    return BT_NO_ERROR;
}

int BluetoothBleCentralManagerProxy::SetAdvFilterParam(const BluetoothBleFilterParamSet &paramSet)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGE("[SetAdvFilterParam] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.fieldValidFlagBit)) {
        HILOGE("[SetAdvFilterParam] fail: write fieldValidFlagBit failed.");
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
    ret = WriteAdvDeviceInfoParcelData(paramSet, data);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    BluetoothUuid btUuid(paramSet.uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("[SetAdvFilterParam] fail:write uuid failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.advHandle)) {
        HILOGE("[SetAdvFilterParam] fail: write advHandle failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.duration)) {
        HILOGE("[SetAdvFilterParam] fail: write duration failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(paramSet.deliveryMode)) {
        HILOGE("[SetAdvFilterParam] fail: write deliveryMode failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_SET_SH_ADV_FILTER_PARAM, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[SetAdvFilterParam] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothBleCentralManagerProxy::RemoveAdvFilter(const bluetooth::Uuid &uuid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RemoveAdvFilter] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("[RemoveAdvFilter] fail: write uuid failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_REMOVE_SH_ADV_FILTER, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[RemoveAdvFilter] fail: transact ErrCode=%{public}d", result);
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