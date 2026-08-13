/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACE_PROFILE_BAS_H
#define INTERFACE_PROFILE_BAS_H

#include "interface_profile.h"
#include <map>

namespace OHOS {
namespace bluetooth {

class IBasObserver {
public:
    virtual ~IBasObserver() = default;
    virtual void OnGetBatteryLevelEvent(const RawAddress &deviceAddress, int32_t batteryLevel) = 0;
    virtual void OnBatteryLevelChanged(const RawAddress &deviceAddress, int32_t batteryLevel) = 0;
};

class IProfileBas : public IProfile {
public:
    virtual void RegisterObserver(std::shared_ptr<IBasObserver> observer) = 0;
    virtual void DeregisterObserver(std::shared_ptr<IBasObserver> observer) = 0;
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) = 0;
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;
    virtual int32_t SetConnectStrategy(const RawAddress &device, int strategy) = 0;
    virtual int32_t GetConnectStrategy(const RawAddress &device) = 0;
    virtual int32_t GetBatteryLevel(const RawAddress &device) = 0;
    virtual std::map<RawAddress, int32_t> GetConnectedDeviceBatteryInfos() = 0;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // INTERFACE_PROFILE_BAS_H