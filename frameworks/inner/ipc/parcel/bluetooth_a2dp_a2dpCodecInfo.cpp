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

#include "bluetooth_a2dp_a2dpCodecInfo.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothA2dpCodecInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint8(codecPriority)) {
        return false;
    }
    if (!parcel.WriteUint8(codecType)) {
        return false;
    }
    if (!parcel.WriteUint32(sampleRate)) {
        return false;
    }
    if (!parcel.WriteUint8(bitsPerSample)) {
        return false;
    }
    if (!parcel.WriteUint8(channelMode)) {
        return false;
    }
    if (!parcel.WriteUint64(codecSpecific1)) {
        return false;
    }
    if (!parcel.WriteUint64(codecSpecific2)) {
        return false;
    }
    if (!parcel.WriteUint64(codecSpecific3)) {
        return false;
    }
    if (!parcel.WriteUint64(codecSpecific4)) {
        return false;
    }
    return true;
}

bool BluetoothA2dpCodecInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothA2dpCodecInfo *BluetoothA2dpCodecInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothA2dpCodecInfo *codecData = new BluetoothA2dpCodecInfo();
    if (codecData != nullptr && !codecData->ReadFromParcel(parcel)) {
        delete codecData;
        codecData = nullptr;
    }
    return codecData;
}

bool BluetoothA2dpCodecInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint8(codecPriority)) {
        return false;
    }
    if (!parcel.ReadUint8(codecType)) {
        return false;
    }
    if (!parcel.ReadUint32(sampleRate)) {
        return false;
    }
    if (!parcel.ReadUint8(bitsPerSample)) {
        return false;
    }
    if (!parcel.ReadUint8(channelMode)) {
        return false;
    }
    if (!parcel.ReadUint64(codecSpecific1)) {
        return false;
    }
    if (!parcel.ReadUint64(codecSpecific2)) {
        return false;
    }
    if (!parcel.ReadUint64(codecSpecific3)) {
        return false;
    }
    if (!parcel.ReadUint64(codecSpecific4)) {
        return false;
    }
    return true;
}

bool BluetoothA2dpOffloadCodecInfo::Marshalling(Parcel &parcel) const
{
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(mediaPacketHeader), false, "write mediaPacketHeader failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(mPt), false, "write mPt failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint32(ssrc), false, "write ssrc failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(boundaryFlag), false, "write boundaryFlag failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(broadcastFlag), false, "write broadcastFlag failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint32(codecType), false, "write codecType failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(maxLatency), false, "write maxLatency failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(scmsTEnable), false, "write scmsTEnable failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint32(sampleRate), false, "write sampleRate failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint32(encodedAudioBitrate), false, "write encodedAudioBitrate failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(bitsPerSample), false, "write bitsPerSample failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(chMode), false, "write chMode failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(aclHdl), false, "write aclHdl failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(l2cRcid), false, "write l2cRcid failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint16(mtu), false, "write mtu failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific0), false, "write codecSpecific0 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific1), false, "write codecSpecific1 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific2), false, "write codecSpecific2 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific3), false, "write codecSpecific3 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific4), false, "write codecSpecific4 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific5), false, "write codecSpecific5 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific6), false, "write codecSpecific6 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.WriteUint8(codecSpecific7), false, "write codecSpecific7 failed.");
    return true;
}

bool BluetoothA2dpOffloadCodecInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothA2dpOffloadCodecInfo *BluetoothA2dpOffloadCodecInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothA2dpOffloadCodecInfo *offloadCodecData = new BluetoothA2dpOffloadCodecInfo();
    if (offloadCodecData != nullptr && !offloadCodecData->ReadFromParcel(parcel)) {
        delete offloadCodecData;
        offloadCodecData = nullptr;
    }
    return offloadCodecData;
}

bool BluetoothA2dpOffloadCodecInfo::ReadFromParcel(Parcel &parcel)
{
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(mediaPacketHeader), false, "read mediaPacketHeader failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(mPt), false, "read mPt failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint32(ssrc), false, "read ssrc failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(boundaryFlag), false, "read boundaryFlag failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(broadcastFlag), false, "read broadcastFlag failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint32(codecType), false, "read codecType failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(maxLatency), false, "read maxLatency failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(scmsTEnable), false, "read scmsTEnable failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint32(sampleRate), false, "read sampleRate failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint32(encodedAudioBitrate), false, "read encodedAudioBitrate failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(bitsPerSample), false, "read bitsPerSample failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(chMode), false, "read chMode failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(aclHdl), false, "read aclHdl failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(l2cRcid), false, "read l2cRcid failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint16(mtu), false, "read mtu failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific0), false, "read codecSpecific0 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific1), false, "read codecSpecific1 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific2), false, "read codecSpecific2 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific3), false, "read codecSpecific3 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific4), false, "read codecSpecific4 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific5), false, "read codecSpecific5 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific6), false, "read codecSpecific6 failed.");
    CHECK_AND_RETURN_LOG_RET(parcel.ReadUint8(codecSpecific7), false, "read codecSpecific7 failed.");
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
