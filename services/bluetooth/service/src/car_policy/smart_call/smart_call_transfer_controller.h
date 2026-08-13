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

#ifndef SMART_CALL_TRANSFER_CONTROLLER_H
#define SMART_CALL_TRANSFER_CONTROLLER_H

#include <string>

#include "device/car_device_type_resolver.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"

namespace OHOS {
namespace bluetooth {

/// 智能来电转移控制器：监听智能来电转移状态变化（IN/OUT），IN 时触发回连将通话音频切换到车机，OUT 时断开 HFP+A2DP 将通话切回手机。
class SmartCallTransferController {
public:
    static constexpr int TRANSFER_STATUS_IN = 6;
    static constexpr int TRANSFER_STATUS_OUT = 7;

    SmartCallTransferController(CarDeviceTypeResolver &resolver, DevicePriorityManager &dpm,
                                ReconnectEngine &reconnectEngine);
    ~SmartCallTransferController() = default;

    // 状态变更入口（由 CarPolicyManager 的 DataShareObserver 调用）
    void OnStatusChanged(int newStatus);
    int GetStatus() const { return status_; }

    // 供 SmartCallOutPredicate 的 provider 查询：当前为 OUT 状态且该设备是华为手机
    bool IsHwPhoneAndTransferOut(const std::string &addr) const;

private:
    void HandleTransferIn();
    void HandleTransferOut();

    CarDeviceTypeResolver &resolver_;
    DevicePriorityManager &dpm_;
    ReconnectEngine &reconnectEngine_;
    int status_{-1};
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // SMART_CALL_TRANSFER_CONTROLLER_H
