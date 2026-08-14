/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <string>

namespace OHOS {
namespace Bluetooth {

/* Phone state snapshot dispatched to the HFP AG service. */
class BluetoothPhoneState {
public:
    BluetoothPhoneState() = default;
    ~BluetoothPhoneState() = default;

    int GetActiveNum() const { return activeNum; }
    int GetHeldNum() const { return heldNum; }
    int GetCallState() const { return callState; }
    int GetCallType() const { return type; }
    const std::string &GetNumber() const { return number; }
    const std::string &GetName() const { return name; }

    void SetActiveNum(int num) { activeNum = num; }
    void SetHeldNum(int num) { heldNum = num; }
    void SetCallState(int state) { callState = state; }
    void SetCallType(int callType) { type = callType; }
    void SetNumber(const std::string &num) { number = num; }
    void SetName(const std::string &n) { name = n; }

    int activeNum = 0;
    int heldNum = 0;
    int callState = 0;
    int type = 0;
    std::string number;
    std::string name;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PHONE_STATE_H
