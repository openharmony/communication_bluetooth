/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "bluetooth_gatt_rsp_context_parcel.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattRspContext::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(timeStamp_)) {
        return false;
    }
    return true;
}

BluetoothGattRspContext *BluetoothGattRspContext::Unmarshalling(Parcel &parcel)
{
    BluetoothGattRspContext *rspContext = new BluetoothGattRspContext();
    if (rspContext != nullptr && !rspContext->ReadFromParcel(parcel)) {
        delete rspContext;
        rspContext = nullptr;
    }
    return rspContext;
}

bool BluetoothGattRspContext::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattRspContext::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt64(timeStamp_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS