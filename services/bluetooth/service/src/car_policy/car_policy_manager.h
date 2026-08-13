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

#ifndef CAR_POLICY_MANAGER_H
#define CAR_POLICY_MANAGER_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "car_policy_config.h"
#include "coordinator/profile_coordinator.h"
#include "adv/pairable_adv_controller.h"
#include "device/car_device_type_resolver.h"
#include "guard/car_connection_guard.h"
#include "metrics/car_metrics_reporter.h"
#include "reconnect/device_priority_manager.h"
#include "reconnect/reconnect_engine.h"
#include "reconnect/reconnect_predicates.h"
#include "smart_call/smart_call_transfer_controller.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyDispatcher;

/// 车机策略编排入口：接收 Dispatcher 投递的事件，协调 ReconnectEngine、ProfileCoordinator、
/// CarConnectionGuard 等组件完成回连、profile 协同、门控等策略。
/// 事件分发委托 CarPolicyDispatcher，自身专注策略逻辑。
class CarPolicyManager {
public:
    static CarPolicyManager &GetInstance();

    void Init();
    void Uninit();

    // 外部入口（由 a2dp_snk_service / hfp_hf_service 调用，切线程由 dispatcher 负责）
    void StartReconnect(ReconnectReason reason);
    void OnAclDisconnect(const std::string &addr, int rawReason);
    void OnBluetoothOn();
    /// 终止当前回连（供 A2DP SinkLimit 抢占后调用，避免被断开设备被立即重连）。
    void TerminateReconnect();

    // Observer 回调（由 CarPolicyDispatcher 在 car policy 线程调用，无需再切线程）
    void OnProfileStateChanged(const std::string &addr, ProfileType profile, int nextState, int prevState);
    void OnFaStatusChanged(int newStatus);
    void OnBtMusicStatusChanged(int newStatus);
    void OnSmartCallTransferStatusChanged(int newStatus);

    /// 收到 echo 8,9 NOTIFY_OUTPUT_PATH 时由协议层入口（bluetooth_hw_interface.cpp）
    void OnHiechoNotifyOutputPath(uint8_t a2dpOp, const std::string &addr);
    void OnBondStateChanged(const std::string &addr, int newState);
    void OnBtSettingsUiChanged(int newStatus);
    void OnCallChanged(const std::string &addr, int callState);
    void OnScoSwitchChanged(const std::string &addr, int scoState);

    CarConnectionGuard &GetConnectionGuard() { return *connectionGuard_; }
    CarDeviceTypeResolver &GetDeviceTypeResolver() { return *deviceTypeResolver_; }
    SmartCallTransferController &GetSmartCallTransferController() { return *smartCallTransferController_; }

    ReconnectEngine &GetReconnectEngine() { return *reconnectEngine_; }
    CarPolicyDispatcher &GetDispatcher() { return *dispatcher_; }

    int GetBtSettingsUiActive() const { return btSettingsUiActive_; }

    /// 扫描终止上报（委托给 CarMetricsReporter，供谓词 OnTerminate 调用）。
    void ReportScanTerminate(const std::string &reason);

private:
    CarPolicyManager();
    ~CarPolicyManager();
    CarPolicyManager(const CarPolicyManager &) = delete;
    CarPolicyManager &operator=(const CarPolicyManager &) = delete;

    void AssembleReconnectPredicates();
    void CheckAllProfilesDisconnected(const std::string &addr);
    void HandlePolicyChanged();
    void DisconnectAllA2dpSink();
    void AutoConnectA2dpByHfpState();
    /// BT 设置界面状态变化 → 控制可配对广播。
    /// UI 活跃时开启广播（便于用户配对新设备），UI 退出且无设备连接时关闭广播。
    /// isForce=true 时跳过状态比较强制刷新（解配对时使用）。
    void HandleBtSettingUiStatusChange(bool isForce);
    /// 是否已有车机设备连接或连接中。
    /// 用于决定是否需要开启可配对广播（已有设备连接时无需广播）。
    bool IsCarConnectedOrConnecting() const;

    /// 发送 SWITCH_BLUETOOTH_CARD 公共事件，通知 MC/FA 卡片管家切换卡片。
    /// value: 1=FA 卡片在蓝牙音乐（使用 A2DP），0=FA 卡片不在蓝牙音乐（不使用 A2DP）
    void PublishSwitchBluetoothCardBroadcast(int value);

    CarPolicyConfig config_;
    std::unique_ptr<CarMetricsReporter> metricsReporter_;
    std::unique_ptr<CarPolicyDispatcher> dispatcher_;
    std::unique_ptr<ReconnectEngine> reconnectEngine_;
    std::unique_ptr<DevicePriorityManager> dpm_;
    std::unique_ptr<CarConnectionGuard> connectionGuard_;
    std::unique_ptr<PairableAdvController> advController_;
    std::unique_ptr<ProfileCoordinator> profileCoordinator_;

    // per-device HFP/A2DP 上次状态缓存（用于判定全部 Profile 断开 + HFP 连接失败 prevState）
    std::unordered_map<std::string, int> prevHfpState_;
    std::unordered_map<std::string, int> prevA2dpState_;
    // per-device 上次 bond 状态缓存（observer 接口不提供 prevState，需自行维护）
    std::unordered_map<std::string, int> prevBondState_;

    int btSettingsUiActive_{0};
    int prevBtSettingsUiStatus_{-1};  // 上次 BT 设置界面状态（-1 表示未初始化，用于检测状态变化避免重复处理）

    // 通话质量上报数据：记录来电、接听、挂断时间戳和 SCO 切换记录，通话结束后一次性上报。
    std::unordered_map<std::string, std::string> callReportMap_;
    std::vector<std::string> scoSwitchList_;
    int64_t callIncomingTimeStampMs_{0};

    std::unique_ptr<CarDeviceTypeResolver> deviceTypeResolver_;
    std::unique_ptr<SmartCallTransferController> smartCallTransferController_;

    std::atomic<bool> initialized_{false};
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_MANAGER_H
