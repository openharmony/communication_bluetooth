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

#ifndef PAN_SERVICE_IMPL_INTERFACE_H
#define PAN_SERVICE_IMPL_INTERFACE_H

#include <memory>
#include <string>
#include "bluetooth.h"
#include "bt_pan.h"
#include "raw_address.h"
#include "pan_message.h"


namespace OHOS {
namespace Bluetooth {

class PanServiceImplInterface {
public:
    virtual ~PanServiceImplInterface() = default;
    
    // 网络操作
    virtual int BringUpNetwork() = 0;
    virtual std::vector<bluetooth::RawAddress> GetDevicesByStates(std::vector<int> states) = 0;
    virtual int GetConnectState(void) = 0;
    
    // 状态机管理
    virtual void RemoveStateMachine(const std::string &device) = 0;
    virtual bool IsStateMachineEmpty() = 0;
    virtual void ClearStatemachine() = 0;
    virtual void ProcessConnect(const PanMessage &msg) = 0;
    virtual void ProcessDisconnect(const PanMessage &msg) = 0;
    virtual void ProcessConnectStateChange(const PanMessage &msg) = 0;
    virtual void ProcessConnectionTimeout(const PanMessage &msg) = 0;

    virtual void DllIsTetheringOnFunc(const std::function<bool()> IsTetheringOnFunc) = 0;
    virtual void DllNotifyStateChangedFunc(const std::function<void(
        const bluetooth::RawAddress &device, int state, int role)> NotifyStateChangedFunc) = 0;
    virtual void DllPostEventFunc(const std::function<void(const PanMessage &event)> PostEventFunc) = 0;
    virtual void DllGetDeviceStateFunc(const std::function<int(
        const bluetooth::RawAddress &device)> GetDeviceStateFunc) = 0;
    virtual void DllSetDevProactiveDisconnectFlagFunc(
        const std::function<void(const std::string &addr, uint32_t profileId, bool isProactive)>
        SetDevProactiveDisconnectFlagFunc) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PAN_SERVICE_IMPL_INTERFACE_H