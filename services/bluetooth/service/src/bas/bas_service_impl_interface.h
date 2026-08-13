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

#ifndef BAS_SERVICE_IMPL_INTERFACE_H
#define BAS_SERVICE_IMPL_INTERFACE_H

#include <map>
#include <string>
#include <vector>
#include "bas_message.h"
#include "raw_address.h"
#include "interface_profile_bas.h"

namespace OHOS {
namespace bluetooth {
class BasServiceImplInterface {
public:
    BasServiceImplInterface() = default;
    virtual ~BasServiceImplInterface() = default;

    virtual void Init(void) = 0;
    virtual void DeInit(void) = 0;
    virtual void RegisterObserver(std::shared_ptr<IBasObserver> observer) = 0;
    virtual void DeregisterObserver(std::shared_ptr<IBasObserver> observer) = 0;
    virtual int Connect(const RawAddress &device) = 0;
    virtual int Disconnect(const RawAddress &device) = 0;
    virtual int GetBatteryLevel(const RawAddress &device) = 0;
    virtual int GetDeviceState(const RawAddress &device) = 0;
    virtual std::list<RawAddress> GetConnectDevices() = 0;
    virtual int GetConnectState(void) = 0;
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) = 0;
    virtual std::map<RawAddress, int32_t> GetConnectedDeviceBatteryInfos() = 0;
    virtual void PostEvent(const BasMessage &event) = 0;
    virtual void RemoveStateMachine(const std::string &device) = 0;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BAS_SERVICE_IMPL_INTERFACE_H