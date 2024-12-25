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
#define LOG_TAG "bt_ipc_parcel_ble_advertiser"
#endif

#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_log.h"
#include "bt_uuid.h"

using OHOS::bluetooth::Uuid;
namespace OHOS {
namespace Bluetooth {
const int32_t BLE_ADV__SERVICE_READ_DATA_SIZE_MAX_LEN = 0x400;
bool BluetoothBleAdvertiserData::Marshalling(Parcel &parcel) const
{
    if (!WriteServiceUuids(parcel)) {
        return false;
    }
    if (!WriteManufacturerData(parcel)) {
        return false;
    }
    if (!WriteServiceData(parcel)) {
        return false;
    }
    if (!parcel.WriteUint8(advFlag_)) {
        return false;
    }
    if (!parcel.WriteString(payload_)) {
        return false;
    }
    if (!parcel.WriteBool(includeDeviceName_)) {
        return false;
    }
    if (!parcel.WriteBool(includeTxPower_)) {
        return false;
    }
    return true;
}

bool BluetoothBleAdvertiserData::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothBleAdvertiserData *BluetoothBleAdvertiserData::Unmarshalling(Parcel &parcel)
{
    BluetoothBleAdvertiserData *advertiserData = new BluetoothBleAdvertiserData();
    if (advertiserData != nullptr && !advertiserData->ReadFromParcel(parcel)) {
        delete advertiserData;
        advertiserData = nullptr;
    }
    return advertiserData;
}

bool BluetoothBleAdvertiserData::ReadFromParcel(Parcel &parcel)
{
    if (!ReadServiceUuids(serviceUuids_, parcel)) {
        return false;
    }
    if (!ReadManufacturerData(manufacturerSpecificData_, parcel)) {
        return false;
    }
    if (!ReadServiceData(serviceData_, parcel)) {
        return false;
    }
    if (!parcel.ReadUint8(advFlag_)) {
        return false;
    }
    if (!parcel.ReadString(payload_)) {
        return false;
    }
    if (!parcel.ReadBool(includeDeviceName_)) {
        return false;
    }
    if (!parcel.ReadBool(includeTxPower_)) {
        return false;
    }
    return true;
}

bool BluetoothBleAdvertiserData::WriteServiceUuids(Parcel &parcel) const
{
    if (!parcel.WriteInt32(serviceUuids_.size())) {
        return false;
    }
    for (auto &serviceUuids : serviceUuids_) {
        uint8_t buffer[Uuid::UUID128_BYTES_TYPE] = {0};
        if (!serviceUuids.ConvertToBytesLE(buffer, Uuid::UUID128_BYTES_TYPE)) {
            return false;
        }
        if (!parcel.WriteBuffer(static_cast<void *>(buffer), Uuid::UUID128_BYTES_TYPE)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleAdvertiserData::ReadServiceUuids(std::vector<Uuid> &serviceUuids, Parcel &parcel)
{
    int32_t serviceUuidSize = 0;
    if (!parcel.ReadInt32(serviceUuidSize) || serviceUuidSize > BLE_ADV__SERVICE_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    for (int i = 0; i < serviceUuidSize; ++i) {
        const uint8_t *buffer = parcel.ReadBuffer(static_cast<size_t>(Uuid::UUID128_BYTES_TYPE));
        if (buffer == nullptr) {
            HILOGE("ReadBuffer failed");
            return false;
        }
        Uuid uuid = Uuid::ConvertFromBytesLE(buffer, static_cast<size_t>(Uuid::UUID128_BYTES_TYPE));
        serviceUuids.push_back(uuid);
    }
    return true;
}

bool BluetoothBleAdvertiserData::WriteManufacturerData(Parcel &parcel) const
{
    if (!parcel.WriteInt32(manufacturerSpecificData_.size())) {
        return false;
    }
    for (auto &manufacturer : manufacturerSpecificData_) {
        if (!parcel.WriteUint16(manufacturer.first)) {
            return false;
        }
        if (!parcel.WriteString(manufacturer.second)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleAdvertiserData::ReadManufacturerData(std::map<uint16_t, std::string> &manufacturerData, Parcel &parcel)
{
    int manufacturerSize = 0;
    if (!parcel.ReadInt32(manufacturerSize) || manufacturerSize > BLE_ADV__SERVICE_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    uint16_t manufacturerId;
    std::string manufacturerDataValue;
    for (int i = 0; i < manufacturerSize; ++i) {
        if (!parcel.ReadUint16(manufacturerId)) {
            return false;
        }
        if (!parcel.ReadString(manufacturerDataValue)) {
            return false;
        }
        manufacturerData.emplace(manufacturerId, manufacturerDataValue);
    }
    return true;
}

bool BluetoothBleAdvertiserData::WriteServiceData(Parcel &parcel) const
{
    if (!parcel.WriteInt32(serviceData_.size())) {
        return false;
    }
    for (auto &service : serviceData_) {
        if (!parcel.WriteUint32(service.first.ConvertTo32Bits())) {
            return false;
        }
        if (!parcel.WriteString(service.second)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleAdvertiserData::ReadServiceData(std::map<Uuid, std::string> &serviceData, Parcel &parcel)
{
    int serviceDataSize = 0;
    if (!parcel.ReadInt32(serviceDataSize) || serviceDataSize > BLE_ADV__SERVICE_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    Uuid serviceDataId;
    std::string serviceDataData;
    for (int i = 0; i < serviceDataSize; ++i) {
        uint32_t uuid;
        if (parcel.ReadUint32(uuid)) {
            serviceDataId = Uuid::ConvertFrom32Bits(uuid);
        } else {
            return false;
        }
        if (!parcel.ReadString(serviceDataData)) {
            return false;
        }
        serviceData.emplace(serviceDataId, serviceDataData);
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
