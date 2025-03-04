/*
 * Copyright (C) Huawei Device Co., Ltd. 2025-2025. All rights reserved.
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

#include "bluetooth_log.h"
#include "bluetooth_trust_pair_device.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace OHOS {
namespace Bluetooth {
bool BluetoothTrustPairDevice::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(macAddress_)) {
        return false;
    }
    if (!parcel.WriteString(deviceName_)) {
        return false;
    }
    if (!MarshallingVecStrSafe(parcel, uuids_)) {
        return false;
    }
    if (!parcel.WriteInt32(bluetoothClass_)) {
        return false;
    }
    if (!MarshallingVecSafe(parcel, token_)) {
        return false;
    }
    if (!MarshallingVecSafe(parcel, secureAdvertisingInfo_)) {
        return false;
    }
    return true;
}

BluetoothTrustPairDevice *BluetoothTrustPairDevice::Unmarshalling(Parcel &parcel)
{
    BluetoothTrustPairDevice *trustDevice = new BluetoothTrustPairDevice();
    if (trustDevice != nullptr && !trustDevice->ReadFromParcel(parcel)) {
        delete trustDevice;
        trustDevice = nullptr;
    }
    return trustDevice;
}

bool BluetoothTrustPairDevice::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothTrustPairDevice::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(macAddress_)) {
        return false;
    }
    if (!parcel.ReadString(deviceName_)) {
        return false;
    }
    if (!ReadParcelVecStrSafe(parcel, uuids_)) {
        return false;
    }
    if (!parcel.ReadInt32(bluetoothClass_)) {
        return false;
    }
    if (!ReadParcelVecSafe(parcel, token_)) {
        return false;
    }
    if (!ReadParcelVecSafe(parcel, secureAdvertisingInfo_)) {
        return false;
    }
    return true;
}

bool BluetoothTrustPairDevice::MarshallingVecSafe(Parcel &parcel, const std::vector<uint8_t> &vec) const
{
    if (vec.size() > TRUST_PAIR_DEVICE_SIZE_MAX) {
        return false;
    }
    if (!parcel.WriteInt32(vec.size())) {
        return false;
    }
    for (auto &data : vec) {
        if (!parcel.WriteUint8(data)) {
            return false;
        }
    }
    return true;
}

bool BluetoothTrustPairDevice::ReadParcelVecSafe(Parcel &parcel, std::vector<uint8_t> &res)
{
    int32_t vecSize = 0;
    if (!parcel.ReadInt32(vecSize) || vecSize > TRUST_PAIR_DEVICE_SIZE_MAX) {
        return false;
    }
    uint8_t data = 0;
    for (auto i = 0; i < vecSize; i++) {
        if (!parcel.ReadUint8(data)) {
            return false;
        }
        res.push_back(data);
    }
    return true;
}

bool BluetoothTrustPairDevice::MarshallingVecStrSafe(Parcel &parcel, const std::vector<std::string> &vec) const
{
    if (vec.size() > TRUST_PAIR_DEVICE_SIZE_MAX) {
        return false;
    }
    if (!parcel.WriteInt32(vec.size())) {
        return false;
    }
    for (auto &data : vec) {
        if (!parcel.WriteString(data)) {
            return false;
        }
    }
    return true;
}

bool BluetoothTrustPairDevice::ReadParcelVecStrSafe(Parcel &parcel, std::vector<std::string> &res)
{
    int32_t vecSize = 0;
    if (!parcel.ReadInt32(vecSize) || vecSize > TRUST_PAIR_DEVICE_SIZE_MAX) {
        return false;
    }
    std::string data;
    for (auto i = 0; i < vecSize; i++) {
        if (!parcel.ReadString(data)) {
            return false;
        }
        res.push_back(data);
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
