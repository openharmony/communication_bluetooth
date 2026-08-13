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

#ifndef RECONNECT_PREDICATES_H
#define RECONNECT_PREDICATES_H

#include <string>

#include "car_policy_config.h"

namespace OHOS {
namespace bluetooth {

class CarConnectionGuard;
class CarPolicyManager;
class SmartCallTransferController;

// ── 回连触发原因 ──
enum class ReconnectReason {
    BT_ON_AUTO_CONN,            // 蓝牙开启自动回连
    ACL_CONNECTION_TIMEOUT,     // ACL 链路超时 (reason=8)，唯一会触发回连的 ACL 断开原因
    SMART_CALL_TRANSFER_IN,     // 智能来电转移-进入车内
};

// ── 回连终止条件谓词接口 ──
class IReconnectPredicate {
public:
    virtual ~IReconnectPredicate() = default;
    /// 是否应终止回连。addr: 当前回连设备；retryTimes: 已重试次数。
    virtual bool ShouldTerminate(const std::string &addr, int retryTimes) = 0;
    /// 谓词名称，用于日志。
    virtual std::string GetName() const = 0;
    /// 终止时的副作用钩子（如取消扫描、上报指标）。默认 no-op。
    /// 由 ReconnectEngine 在 ShouldTerminate 返回 true 后调用。
    virtual void OnTerminate() {}
};

/// 2. Incoming 连接且 HFP 未断：终止避免双向冲突。
/// 任一已配对设备有 incoming 连接或 profile 处于 CONNECTING/CONNECTED 时，
/// 主动回连会与对端发起的连接相互冲突，因此终止。
/// 直接调 AdapterManager 单例 + CarProfileService 静态方法。
class IncomingConnPredicate : public IReconnectPredicate {
public:
    IncomingConnPredicate() = default;
    bool ShouldTerminate(const std::string &addr, int retryTimes) override;
    std::string GetName() const override { return "IncomingConnPredicate"; }
};

/// 3. BT 设置 UI 活跃且 retry>=1：终止避免扫描冲突。
/// UI 活跃时通常用户正在扫描/配对，回连与扫描共享射频资源会互相拖慢，
/// 首次重试放行（给一次连接机会），之后命中即终止。
/// 依赖 CarPolicyManager::GetBtSettingsUiActive() 查询缓存的 UI 状态。
/// OnTerminate 直接调 AdapterManager 单例取消扫描 + Mgr.ReportScanTerminate 上报。
class BtSettingsUiPredicate : public IReconnectPredicate {
public:
    explicit BtSettingsUiPredicate(CarPolicyManager &mgr);
    bool ShouldTerminate(const std::string &addr, int retryTimes) override;
    std::string GetName() const override { return "BtSettingsUiPredicate"; }
    void OnTerminate() override;
private:
    CarPolicyManager &mgr_;
};

/// 4. 华为手机+智能来电转移 OUT：转移期间终止回连，避免与通话转移流程冲突。
/// 依赖 SmartCallTransferController::IsHwPhoneAndTransferOut()。
class SmartCallOutPredicate : public IReconnectPredicate {
public:
    explicit SmartCallOutPredicate(SmartCallTransferController &sct);
    bool ShouldTerminate(const std::string &addr, int retryTimes) override;
    std::string GetName() const override { return "SmartCallOutPredicate"; }
private:
    SmartCallTransferController &sct_;
};

/// 5. CarPlay 为最后连接：CarPlay 与 HFP 互斥，最后连接是 CarPlay 时不再回连 HFP。
/// 依赖 CarConnectionGuard::IsCarPlayMutex()。
class CarplayMutexPredicate : public IReconnectPredicate {
public:
    explicit CarplayMutexPredicate(CarConnectionGuard &guard);
    bool ShouldTerminate(const std::string &addr, int retryTimes) override;
    std::string GetName() const override { return "CarplayMutexPredicate"; }
private:
    CarConnectionGuard &guard_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // RECONNECT_PREDICATES_H
