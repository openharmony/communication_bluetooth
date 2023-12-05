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

#ifndef BLUETOOTH_PHONE_STATE_H
#define BLUETOOTH_PHONE_STATE_H

#include "parcel.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPhoneState : public Parcelable {
public:
    BluetoothPhoneState() = default;
    ~BluetoothPhoneState() override = default;

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothPhoneState *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);

    int GetActiveNum();
    void SetActiveNum(int activeNum);
    int GetHeldNum();
    void SetHeldNum(int heldNum);
    int GetCallState();
    void SetCallState(int callState);
    std::string GetNumber();
    void SetNumber(std::string number);
    int GetCallType();
    void SetCallType(int callType);
    std::string GetName();
    void SetName(std::string name);
private:
    int activeNum_ {0};
    int heldNum_ {0};
    int callState_ {0};
    std::string number_ {""};
    int callType_ {0};
    std::string name_ {""};
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PHONE_STATE_H