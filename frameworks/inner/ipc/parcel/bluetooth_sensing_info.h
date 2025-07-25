/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_SENSING_INFO_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_SENSING_INFO_H

#include "parcel.h"
#include "bt_sensing_info.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothSensingInfo : public Parcelable {
public:
    BluetoothSensingInfo() = default;
    ~BluetoothSensingInfo() = default;
    explicit BluetoothSensingInfo(const bluetooth::SensingInfo &info)
        : addr_(info.addr_), uuid_(info.uuid_), resourceId_(info.resourceId_), pkgName_(info.pkgName_),
        isServer_(info.isServer_), interval_(info.interval_), connectable_(info.connectable_),
        payloadLen_(info.payloadLen_), bussinessType_(info.bussinessType_), scanMode_(info.scanMode_)
    {}
    bool Marshalling(Parcel &parcel) const override;
    static BluetoothSensingInfo *Unmarshalling(Parcel &parcel);
    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);

    std::string addr_;
    std::string uuid_;
    uint32_t resourceId_;
    std::string pkgName_;
    bool isServer_;
    uint16_t interval_;
    bool connectable_;
    int payloadLen_;
    std::string bussinessType_;
    int scanMode_;
    int reportMode_ = 1;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_SENSING_INFO_H