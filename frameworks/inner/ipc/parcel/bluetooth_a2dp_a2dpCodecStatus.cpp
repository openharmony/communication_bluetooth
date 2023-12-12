/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_a2dp_a2dpCodecStatus.h"
#include <memory>
#include <vector>
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
const uint32_t A2DP_A2DPCODECSTATUS_PARCEL_SIZE_MAX = 0x400;
bool BluetoothA2dpCodecStatus::Marshalling(Parcel &parcel) const
{
    BluetoothA2dpCodecInfo info = bluetooth::CodecInfo(codecInfo);
    if (!parcel.WriteParcelable(&info)) {
        return false;
    }
    if (!parcel.WriteUint32(codecInfoLocalCap.size())) {
        return false;
    }
    for (auto &infoLocalCap : codecInfoLocalCap) {
        info = bluetooth::CodecInfo(infoLocalCap);
        if (!parcel.WriteParcelable(&info)) {
            return false;
        }
    }
    if (!parcel.WriteUint32(codecInfoConfirmCap.size())) {
        return false;
    }
    for (auto &infoConfirmCap : codecInfoConfirmCap) {
        info = bluetooth::CodecInfo(infoConfirmCap);
        if (!parcel.WriteParcelable(&info)) {
            return false;
        }
    }
    return true;
}

bool BluetoothA2dpCodecStatus::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothA2dpCodecStatus *BluetoothA2dpCodecStatus::Unmarshalling(Parcel &parcel)
{
    BluetoothA2dpCodecStatus *codecData = new BluetoothA2dpCodecStatus();
    if (codecData != nullptr && !codecData->ReadFromParcel(parcel)) {
        delete codecData;
        codecData = nullptr;
    }
    return codecData;
}

bool BluetoothA2dpCodecStatus::ReadFromParcel(Parcel &parcel)
{
    std::shared_ptr<BluetoothA2dpCodecInfo> info(parcel.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (!info) {
        return false;
    }
    codecInfo = bluetooth::CodecInfo(*info);
    uint32_t size = 0;
    if (!parcel.ReadUint32(size) || size > A2DP_A2DPCODECSTATUS_PARCEL_SIZE_MAX) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<BluetoothA2dpCodecInfo> infoLocalCap(parcel.ReadParcelable<BluetoothA2dpCodecInfo>());
        if (!infoLocalCap) {
            return false;
        }
        codecInfoLocalCap.push_back(*infoLocalCap);
    }
    if (!parcel.ReadUint32(size) || size > A2DP_A2DPCODECSTATUS_PARCEL_SIZE_MAX) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<BluetoothA2dpCodecInfo> infoConfirmCap(parcel.ReadParcelable<BluetoothA2dpCodecInfo>());
        if (!infoConfirmCap) {
            return false;
        }
        codecInfoConfirmCap.push_back(*infoConfirmCap);
    }
    return true;
}

bool BluetoothA2dpOffloadCodecStatus::Marshalling(Parcel &parcel) const
{
    BluetoothA2dpOffloadCodecInfo info = BluetoothA2dpOffloadCodecInfo(bluetooth::OffloadCodecInfo(offloadInfo));
    CHECK_AND_RETURN_LOG_RET(parcel.WriteParcelable(&info), false, "write parcel err.");
    return true;
}

bool BluetoothA2dpOffloadCodecStatus::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothA2dpOffloadCodecStatus *BluetoothA2dpOffloadCodecStatus::Unmarshalling(Parcel &parcel)
{
    BluetoothA2dpOffloadCodecStatus *offloadCodecData = new BluetoothA2dpOffloadCodecStatus();
    if (offloadCodecData != nullptr && !offloadCodecData->ReadFromParcel(parcel)) {
        delete offloadCodecData;
        offloadCodecData = nullptr;
    }
    return offloadCodecData;
}

bool BluetoothA2dpOffloadCodecStatus::ReadFromParcel(Parcel &parcel)
{
    std::shared_ptr<BluetoothA2dpOffloadCodecInfo> info(parcel.ReadParcelable<BluetoothA2dpOffloadCodecInfo>());
    CHECK_AND_RETURN_LOG_RET(info, false, "Read Parcel err.");
    offloadInfo = bluetooth::OffloadCodecInfo(*info);
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
