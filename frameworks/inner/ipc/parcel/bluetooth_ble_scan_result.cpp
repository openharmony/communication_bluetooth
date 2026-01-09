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
#define LOG_TAG "bt_ipc_parcel_ble_scan_result"
#endif

#include "bluetooth_ble_scan_result.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
const int32_t BLE_SCAN_READ_DATA_SIZE_MAX_LEN = 0x400;
bool BluetoothBleScanResult::Marshalling(Parcel &parcel) const
{
    if (!WriteServiceUuidsToParcel(parcel)) {
        return false;
    }
    if (!WriteManufacturerDataToParcel(parcel)) {
        return false;
    }
    if (!WriteServiceDataToParcel(parcel)) {
        return false;
    }
    if (!parcel.WriteString(addr_.GetAddress())) {
        return false;
    }
    if (!parcel.WriteUint8(addressType_)) {
        return false;
    }
    if (!parcel.WriteUint8(rawAddressType_)) {
        return false;
    }
    if (!parcel.WriteInt8(rssi_)) {
        return false;
    }
    if (!parcel.WriteBool(connectable_)) {
        return false;
    }
    if (!parcel.WriteUint8(advertiseFlag_)) {
        return false;
    }
    if (!parcel.WriteString(payload_)) {
        return false;
    }
    if (!parcel.WriteString(name_)) {
        return false;
    }
    if (!parcel.WriteUint16(eventType_)) {
        return false;
    }
    if (!parcel.WriteInt8(txPowerLevel_)) {
        return false;
    }
    if (!WriteAdvertisingDataToParcel(parcel)) {
        return false;
    }
    return true;
}

BluetoothBleScanResult *BluetoothBleScanResult::Unmarshalling(Parcel &parcel)
{
    BluetoothBleScanResult *settings = new BluetoothBleScanResult();
    if (settings != nullptr && !settings->ReadFromParcel(parcel)) {
        delete settings;
        settings = nullptr;
    }
    return settings;
}

bool BluetoothBleScanResult::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothBleScanResult::ReadFromParcel(Parcel &parcel)
{
    if (!ReadServiceUuidsFromParcel(parcel)) {
        return false;
    }
    if (!ReadManufacturerDataFromParcel(parcel)) {
        return false;
    }
    if (!ReadServiceDataFromParcel(parcel)) {
        return false;
    }
    std::string address = "";
    if (parcel.ReadString(address)) {
        addr_ = bluetooth::RawAddress(address);
    } else {
        return false;
    }
    if (!parcel.ReadUint8(addressType_)) {
        return false;
    }
    if (!parcel.ReadUint8(rawAddressType_)) {
        return false;
    }
    if (!parcel.ReadInt8(rssi_)) {
        return false;
    }
    if (!parcel.ReadBool(connectable_)) {
        return false;
    }
    if (!parcel.ReadUint8(advertiseFlag_)) {
        return false;
    }
    if (!parcel.ReadString(payload_)) {
        return false;
    }
    if (!parcel.ReadString(name_)) {
        return false;
    }
    if (!parcel.ReadUint16(eventType_)) {
        return false;
    }
    if (!parcel.ReadInt8(txPowerLevel_)) {
        return false;
    }
    if (!ReadAdvertisingDataFromParcel(parcel)) {
        return false;
    }
    return true;
}

bool BluetoothBleScanResult::WriteServiceUuidsToParcel(Parcel &parcel) const
{
    if (!parcel.WriteInt32(serviceUuids_.size())) {
        return false;
    }
    for (auto &serviceUuid : serviceUuids_) {
        uint8_t buffer[bluetooth::Uuid::UUID128_BYTES_TYPE] = {0};
        if (!serviceUuid.ConvertToBytesLE(buffer, bluetooth::Uuid::UUID128_BYTES_TYPE)) {
            return false;
        }
        if (!parcel.WriteBuffer(static_cast<void *>(buffer), bluetooth::Uuid::UUID128_BYTES_TYPE)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleScanResult::ReadServiceUuidsFromParcel(Parcel &parcel)
{
    int32_t uuidSize = 0;
    if (!parcel.ReadInt32(uuidSize) || uuidSize > BLE_SCAN_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    for (int i = 0; i < uuidSize; ++i) {
        const uint8_t *buffer = parcel.ReadBuffer(static_cast<size_t>(bluetooth::Uuid::UUID128_BYTES_TYPE));
        if (buffer == nullptr) {
            HILOGE("ReadBuffer failed");
            return false;
        }
        bluetooth::Uuid uuid = bluetooth::Uuid::ConvertFromBytesLE(
            buffer, static_cast<size_t>(bluetooth::Uuid::UUID128_BYTES_TYPE));
        serviceUuids_.push_back(uuid);
    }
    return true;
}

bool BluetoothBleScanResult::WriteManufacturerDataToParcel(Parcel &parcel) const
{
    if (!parcel.WriteInt32(manufacturerSpecificData_.size())) {
        return false;
    }
    for (auto iter = manufacturerSpecificData_.begin(); iter != manufacturerSpecificData_.end(); ++iter) {
        if (!parcel.WriteUint16(iter->first)) {
            return false;
        }
        if (!parcel.WriteString(iter->second)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleScanResult::ReadManufacturerDataFromParcel(Parcel &parcel)
{
    int32_t manuSize = 0;
    if (!parcel.ReadInt32(manuSize) || manuSize > BLE_SCAN_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    for (int i = 0; i < manuSize; i++) {
        uint16_t manufacturerId = 0;
        std::string manufacturedData;
        if (!parcel.ReadUint16(manufacturerId)) {
            return false;
        }
        if (!parcel.ReadString(manufacturedData)) {
            return false;
        }
        manufacturerSpecificData_.emplace(manufacturerId, manufacturedData);
    }
    return true;
}

bool BluetoothBleScanResult::WriteServiceDataToParcel(Parcel &parcel) const
{
    if (!parcel.WriteInt32(serviceData_.size())) {
        return false;
    }
    for (auto iter = serviceData_.begin(); iter != serviceData_.end(); ++iter) {
        if (!parcel.WriteUint32(iter->first.ConvertTo32Bits())) {
            return false;
        }
        if (!parcel.WriteString(iter->second)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleScanResult::ReadServiceDataFromParcel(Parcel &parcel)
{
    int32_t serviceDataSize = 0;
    if (!parcel.ReadInt32(serviceDataSize) || serviceDataSize > BLE_SCAN_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    for (int i = 0; i < serviceDataSize; i++) {
        uint32_t serviceId;
        std::string serviceData;
        if (!parcel.ReadUint32(serviceId)) {
            return false;
        }
        if (!parcel.ReadString(serviceData)) {
            return false;
        }
        serviceData_.emplace(bluetooth::Uuid::ConvertFrom32Bits(serviceId), serviceData);
    }
    return true;
}

bool BluetoothBleScanResult::WriteAdvertisingDataToParcel(Parcel &parcel) const
{
    if (!parcel.WriteInt32(advertisingData_.size())) {
        return false;
    }
    for (auto iter = advertisingData_.begin(); iter != advertisingData_.end(); ++iter) {
        if (!parcel.WriteUint8(iter->first)) {
            return false;
        }
        if (!parcel.WriteString(iter->second)) {
            return false;
        }
    }
    return true;
}

bool BluetoothBleScanResult::ReadAdvertisingDataFromParcel(Parcel &parcel)
{
    int32_t advSize = 0;
    if (!parcel.ReadInt32(advSize) || advSize > BLE_SCAN_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return false;
    }
    for (int i = 0; i < advSize; i++) {
        uint8_t advType = 0;
        std::string advData {};
        if (!parcel.ReadUint8(advType)) {
            return false;
        }
        if (!parcel.ReadString(advData)) {
            return false;
        }
        advertisingData_.emplace(advType, advData);
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS