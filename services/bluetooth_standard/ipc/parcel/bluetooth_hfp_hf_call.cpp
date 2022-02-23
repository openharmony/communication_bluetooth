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

#include <string>
#include "bluetooth_hfp_hf_call.h"
#include "bluetooth_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

bool BluetoothHfpHfCall::Marshalling(Parcel &parcel) const 
{
    return WriteToParcel(parcel);
}

BluetoothHfpHfCall *BluetoothHfpHfCall::Unmarshalling(Parcel &parcel) 
{
    BluetoothHfpHfCall *call = new BluetoothHfpHfCall();
    if (!call->ReadFromParcel(parcel)) {
        delete call;
        return nullptr;
    }
    return call;
}

bool BluetoothHfpHfCall::WriteToParcel(Parcel &parcel) const 
{
    bool status = parcel.WriteString(string(device_.c_str(), device_.size()));
    if (!status) {
        return status;
    }
    status = parcel.WriteInt32(id_);
    if (!status) {
        return status;
    }
    status = parcel.WriteInt32(state_);
    if (!status) {
        return status;
    }
    status = parcel.WriteString(string(number_.c_str(), number_.size()));
    if (!status) {
        return status;
    }
    BluetoothUuid *bt_uuid = new BluetoothUuid(uuid_);
    status = parcel.WriteParcelable(bt_uuid);
    if (!status) {
        return status;
    }
    status = parcel.WriteBool(multiParty_);
    if (!status) {
        return status;
    }
    status = parcel.WriteBool(outgoing_);
    if (!status) {
        return status;
    }
    status = parcel.WriteBool(inBandRing_);
    if (!status) {
        return status;
    }
    status = parcel.WriteInt64(creationTime_);
    return status;
}

bool BluetoothHfpHfCall::ReadFromParcel(Parcel &parcel) {
    bool status = parcel.ReadString(device_);
    if (!status) {
        return status;
    }

    status = parcel.ReadInt32(id_);
    if (!status) {
        return status;
    }

    status = parcel.ReadInt32(state_);
    if (!status) {
        return status;
    }

    status = parcel.ReadString(number_);
    if (!status) {
        return status;
    }

    BluetoothUuid *bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
    if (!bt_uuid) {
        return false;
    }
    uuid_ = *bt_uuid;

    status = parcel.ReadBool(multiParty_);
    if (!status) {
        return status;
    }

    status = parcel.ReadBool(outgoing_);
    if (!status) {
        return status;
    }

    status = parcel.ReadBool(inBandRing_);
    if (!status) {
        return status;
    }

    int64_t value64 = creationTime_;
    status = parcel.ReadInt64(value64);
    if (!status) {
        return status;
    }
    return status;
}

}  // namespace Bluetooth
}  // namespace OHOS