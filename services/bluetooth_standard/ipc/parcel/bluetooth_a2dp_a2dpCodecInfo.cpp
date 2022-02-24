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

namespace OHOS {
namespace Bluetooth {
bool BluetoothA2dpCodecInfo::Marshalling(Parcel &parcel) const
{
    bool status = true;

    status = parcel.WriteInt32((int)codecPriority);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)codecType);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)sampleRate);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)bitsPerSample);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt32((int)channelMode);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecSpecific1);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecSpecific2);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecSpecific3);
    if (!status) { 
        return status;
    }

    status = parcel.WriteInt64(codecSpecific4);
    if (!status) { 
        return status;
    }

    return status;
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
    bool status = true;
    int tempInt;
    int64_t tempInt16;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecPriority = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    codecType = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    sampleRate = (uint32_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    bitsPerSample = (uint8_t)tempInt;

    status = parcel.ReadInt32(tempInt);
    if (!status) { 
        return status;
    }
    channelMode = (uint8_t)tempInt;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecSpecific1 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecSpecific2 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecSpecific3 = (uint64_t)tempInt16;

    status = parcel.ReadInt64(tempInt16);
    if (!status) { 
        return status;
    }
    codecSpecific4 = (uint64_t)tempInt16;
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS