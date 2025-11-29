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

#ifndef BLUETOOTH_PARCEL_GATT_RSP_CONTEXT_H
#define BLUETOOTH_PARCEL_GATT_RSP_CONTEXT_H

#include "parcel.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattRspContext : public Parcelable {
public:
    BluetoothGattRspContext() = default;

    explicit BluetoothGattRspContext(const int64_t timeStamp) : timeStamp_(timeStamp)
    {}

    ~BluetoothGattRspContext() override = default;

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothGattRspContext *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);

    bool ReadFromParcel(Parcel &parcel);

    int64_t GetTimeStamp() const
    {
        return timeStamp_;
    }

    void SetTimeStamp(int64_t timeStamp)
    {
        timeStamp_ = timeStamp;
    }

    int64_t timeStamp_ = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_GATT_RSP_CONTEXT_H