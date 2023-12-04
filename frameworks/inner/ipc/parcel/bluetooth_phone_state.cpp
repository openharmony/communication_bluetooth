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

#include "bluetooth_phone_state.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothPhoneState::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(activeNum_)) {
        return false;
    }
    if (!parcel.WriteInt32(heldNum_)) {
        return false;
    }
    if (!parcel.WriteInt32(callState_)) {
        return false;
    }
    if (!parcel.WriteString(number_)) {
        return false;
    }
    if (!parcel.WriteInt32(callType_)) {
        return false;
    }
    if (!parcel.WriteString(name_)) {
        return false;
    }
    return true;
}

bool BluetoothPhoneState::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothPhoneState *BluetoothPhoneState::Unmarshalling(Parcel &parcel)
{
    BluetoothPhoneState *phoneState = new BluetoothPhoneState();
    if (phoneState != nullptr && !phoneState->ReadFromParcel(parcel)) {
        delete phoneState;
        phoneState = nullptr;
    }
    return phoneState;
}

bool BluetoothPhoneState::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(activeNum_)) {
        return false;
    }
    if (!parcel.ReadInt32(heldNum_)) {
        return false;
    }
    if (!parcel.ReadInt32(callState_)) {
        return false;
    }
    if (!parcel.ReadString(number_)) {
        return false;
    }
    if (!parcel.ReadInt32(callType_)) {
        return false;
    }
    if (!parcel.ReadString(name_)) {
        return false;
    }
    return true;
}

int BluetoothPhoneState::GetActiveNum()
{
    return activeNum_;
}

void BluetoothPhoneState::SetActiveNum(int activeNum)
{
    activeNum_ = activeNum;
}

int BluetoothPhoneState::GetHeldNum()
{
    return heldNum_;
}

void BluetoothPhoneState::SetHeldNum(int heldNum)
{
    heldNum_ = heldNum;
}

int BluetoothPhoneState::GetCallState()
{
    return callState_;
}

void BluetoothPhoneState::SetCallState(int callState)
{
    callState_ = callState;
}

std::string BluetoothPhoneState::GetNumber()
{
    return number_;
}

void BluetoothPhoneState::SetNumber(std::string number)
{
    number_ = number;
}

int BluetoothPhoneState::GetCallType()
{
    return callType_;
}

void BluetoothPhoneState::SetCallType(int callType)
{
    callType_ = callType;
}

std::string BluetoothPhoneState::GetName()
{
    return name_;
}

void BluetoothPhoneState::SetName(std::string name)
{
    name_ = name;
}
}  // namespace Bluetooth
}  // namespace OHOS