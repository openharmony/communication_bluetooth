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
#include <vector>
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothA2dpCodecStatus::Marshalling(Parcel &parcel) const
{
    bool status = true;

    status = parcel.WriteInt32((int)codecInfo.codecPriority);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)codecInfo.codecType);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)codecInfo.sampleRate);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)codecInfo.bitsPerSample);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)codecInfo.channelMode);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecInfo.codecSpecific1);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecInfo.codecSpecific2);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecInfo.codecSpecific3);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecInfo.codecSpecific4);
    if (!status) { 
        return status;
    }

    std::vector<BluetoothA2dpCodecInfo> localCodecInfo;
    BluetoothA2dpCodecInfo codecInfo {};
    for (auto& iter : codecInfoLocalCap) {
        codecInfo.codecPriority = iter.codecPriority;
        codecInfo.codecType = iter.codecType;
        codecInfo.sampleRate = iter.sampleRate;
        codecInfo.bitsPerSample = iter.bitsPerSample;
        codecInfo.channelMode = iter.channelMode;
        codecInfo.codecSpecific1 = iter.codecSpecific1;
        codecInfo.codecSpecific2 = iter.codecSpecific2;
        codecInfo.codecSpecific3 = iter.codecSpecific3;
        codecInfo.codecSpecific4 = iter.codecSpecific4;
        localCodecInfo.push_back(codecInfo);
    }
    status = WriteParcelableCodecVector(localCodecInfo, parcel);
    if (!status) {
        return status;
    }

    std::vector<BluetoothA2dpCodecInfo> confirmCodecInfo;
    for (auto& iter : codecInfoConfirmCap) {
        codecInfo.codecPriority = iter.codecPriority;
        codecInfo.codecType = iter.codecType;
        codecInfo.sampleRate = iter.sampleRate;
        codecInfo.bitsPerSample = iter.bitsPerSample;
        codecInfo.channelMode = iter.channelMode;
        codecInfo.codecSpecific1 = iter.codecSpecific1;
        codecInfo.codecSpecific2 = iter.codecSpecific2;
        codecInfo.codecSpecific3 = iter.codecSpecific3;
        codecInfo.codecSpecific4 = iter.codecSpecific4;
        confirmCodecInfo.push_back(codecInfo);
    }
    status = WriteParcelableCodecVector(confirmCodecInfo, parcel);
    if (!status) {
        return status;
    }

    return status;
}

bool BluetoothA2dpCodecStatus::WriteParcelableCodecVector(
    const std::vector<BluetoothA2dpCodecInfo> &localCodecInfo, Parcel &parcel) const
{
    if (!parcel.WriteInt32(localCodecInfo.size())) {
        HILOGE("write WriteParcelableVector failed");
        return false;
    }

    for (auto codeInfo : localCodecInfo) {
        if (!parcel.WriteParcelable(&codeInfo)) {
            HILOGE("write WriteParcelableVector failed");
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
    bool status = true;
    int tempInt;
    int64_t tempInt16;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecInfo.codecPriority = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecInfo.codecType = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecInfo.sampleRate = (uint32_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecInfo.bitsPerSample = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecInfo.channelMode = (uint8_t)tempInt;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecInfo.codecSpecific1 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecInfo.codecSpecific2 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecInfo.codecSpecific3 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecInfo.codecSpecific4 = (uint64_t)tempInt16;

    std::vector<BluetoothA2dpCodecInfo> localCodecInfo;
    status = ReadParcelableVector(localCodecInfo, parcel);
    if (!status) {
        return status;
    }
    BluetoothA2dpCodecInfo codecInfo {};
    for (auto& iter : localCodecInfo) {   
        codecInfo.codecPriority = iter.codecPriority;
        codecInfo.codecType = iter.codecType;
        codecInfo.sampleRate = iter.sampleRate;
        codecInfo.bitsPerSample = iter.bitsPerSample;
        codecInfo.channelMode = iter.channelMode;
        codecInfo.codecSpecific1 = iter.codecSpecific1;
        codecInfo.codecSpecific2 = iter.codecSpecific2;
        codecInfo.codecSpecific3 = iter.codecSpecific3;
        codecInfo.codecSpecific4 = iter.codecSpecific4;
        codecInfoLocalCap.push_back(codecInfo);
    }

    std::vector<BluetoothA2dpCodecInfo> confirmCodecInfo;
    status = ReadParcelableVector(confirmCodecInfo, parcel);
    if (!status) {
        return status;
    }
    for (auto& iter : confirmCodecInfo) {
        codecInfo.codecPriority = iter.codecPriority;
        codecInfo.codecType = iter.codecType;
        codecInfo.sampleRate = iter.sampleRate;
        codecInfo.bitsPerSample = iter.bitsPerSample;
        codecInfo.channelMode = iter.channelMode;
        codecInfo.codecSpecific1 = iter.codecSpecific1;
        codecInfo.codecSpecific2 = iter.codecSpecific2;
        codecInfo.codecSpecific3 = iter.codecSpecific3;
        codecInfo.codecSpecific4 = iter.codecSpecific4;
        codecInfoConfirmCap.push_back(codecInfo);
    }
    return true;
}

bool BluetoothA2dpCodecStatus::ReadParcelableVector(
    const std::vector<BluetoothA2dpCodecInfo> &localCodecInfo, Parcel &parcel) const
{
    int size = localCodecInfo.size();
    if (!parcel.ReadInt32(size)) {
        HILOGE("read ReadParcelableVector failed");
        return false;
    }

    for (auto codeInfo : localCodecInfo) {
        if (&codeInfo != parcel.ReadParcelable<BluetoothA2dpCodecInfo>()) {
            HILOGE("read ReadParcelableVector failed");
            return false;
        }
    }
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS