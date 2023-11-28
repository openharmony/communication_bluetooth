/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SOCKET_COC_INFO_H
#define BLUETOOTH_SOCKET_COC_INFO_H

#include "parcel.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothSocketCocInfo : public Parcelable {
public:
    BluetoothSocketCocInfo() = default;
    ~BluetoothSocketCocInfo() override = default;

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothSocketCocInfo *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);

    std::string addr;
    int32_t minInterval = 0;
    int32_t maxInterval = 0;
    int32_t peripheralLatency = 0;
    int32_t supervisionTimeout = 0;
    int32_t minConnEventLen = 0;
    int32_t maxConnEventLen = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SOCKET_COC_INFO_H