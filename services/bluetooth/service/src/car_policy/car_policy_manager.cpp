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

#include "car_policy_manager.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_manager"
#endif
#include "log.h"

#include "dispatcher/car_policy_dispatcher.h"
#include "metrics/car_metrics_reporter.h"
#include "service/car_profile_service.h"

#include "adapter_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_os_account.h"
#include "classic_adapter.h"
#include "hands_free_unit_calls.h"
#include "raw_address.h"

#include "reconnect/reconnect_predicates.h"
#include <parameters.h>

#include "common_event_manager.h"
#include "want.h"

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

namespace {
// echo 8,9 NOTIFY_OUTPUT_PATH 的 a2dpOperation 取值（payload[2]）
constexpr uint8_t HIECHO_NO_USE_A2DP = 1;  // 不使用 A2DP → FA 卡片不在蓝牙音乐
constexpr uint8_t HIECHO_USE_A2DP = 2;     // 使用 A2DP → FA 卡片在蓝牙音乐

// SWITCH_BLUETOOTH_CARD 公共事件参数
constexpr const char *SWITCH_BLUETOOTH_CARD_ACTION = "usual.event.bluetooth.hiecho.switch_bluetooth_card";
constexpr const char *SWITCH_BLUETOOTH_CARD_EXTRA_KEY = "switch_bluetooth_card";
// extra value: 1=FA 卡片在蓝牙音乐, 0=FA 卡片不在蓝牙音乐
constexpr int SWITCH_BLUETOOTH_CARD_VALUE_ON = 1;
constexpr int SWITCH_BLUETOOTH_CARD_VALUE_OFF = 0;

// BT 设置界面状态：1=活跃（UI 显示），其他=非活跃
constexpr int BT_SETTING_STATUS_ON = 1;
}  // namespace

// ── CarPolicyManager 实现 ──

CarPolicyManager &CarPolicyManager::GetInstance()
{
    static CarPolicyManager instance;
    return instance;
}

CarPolicyManager::CarPolicyManager() = default;

CarPolicyManager::~CarPolicyManager()
{
    // Uninit() 已由 AdapterManager::UnLoadBluetoothSystemAbility（adapter_manager.cpp:1407）在 BT 关闭时显式调用；
    // 此处为防御性兜底，以防该路径漏调。initialized_ 守卫保证重复 Uninit 幂等。
    if (initialized_) {
        HILOGW("~CarPolicyManager: not Uninit, force Uninit");
        Uninit();
    }
}

void CarPolicyManager::Init()
{
    if (initialized_) {
        HILOGW("Init called twice, ignore");
        return;
    }

    // ── 构造 CarMetricsReporter（ReconnectEngine 依赖，先构造）──
    metricsReporter_ = std::make_unique<CarMetricsReporter>();

    // ── 构造 CarPolicyDispatcher（事件分发 + Observer 注册）──
    dispatcher_ = std::make_unique<CarPolicyDispatcher>(*this);

    // ── 构造 DevicePriorityManager（先于 ReconnectEngine，后者持有 DPM 引用）──
    dpm_ = std::make_unique<DevicePriorityManager>();
    dpm_->Init();

    // ── 构造 CarConnectionGuard（先于 ReconnectEngine，后者持有 Guard 引用）──
    connectionGuard_ = std::make_unique<CarConnectionGuard>();

    // ── 构造 ReconnectEngine（持有 Guard 引用，用于 HandleReconnect 中 Guard.Check + SinkLimit 抢占）──
    reconnectEngine_ = std::make_unique<ReconnectEngine>(
        config_, *dpm_, *dispatcher_, *metricsReporter_, *connectionGuard_);

    // ── 构造 PairableAdvController（CarPolicyManager 自身持有，用于可配对广播控制）──
    advController_ = std::make_unique<PairableAdvController>();

    // ── 构造 ProfileCoordinator ──
    profileCoordinator_ = std::make_unique<ProfileCoordinator>(*connectionGuard_, *dpm_, *dispatcher_);

    // ── 构造 CarDeviceTypeResolver ──
    deviceTypeResolver_ = std::make_unique<CarDeviceTypeResolver>();

    // ── 构造 SmartCallTransferController ──
    smartCallTransferController_ = std::make_unique<SmartCallTransferController>(
        *deviceTypeResolver_, *dpm_, *reconnectEngine_);

    // ── 接入 5 个回连谓词 ──
    AssembleReconnectPredicates();

    // ── 注册所有 Observer（Profile + FA/BtMusic 广播 + SCT DataShare）──
    dispatcher_->RegisterAll();

    initialized_ = true;
    HILOGI("CarPolicyManager initialized");
}

void CarPolicyManager::Uninit()
{
    if (!initialized_) {
        return;
    }
    initialized_ = false;
    // 先解除观察者/广播订阅（避免回调进入已销毁的对象）
    if (dispatcher_) {
        dispatcher_->UnregisterAll();
    }
    // 销毁顺序：dispatcher → coordinator → adv → sct → engine → metrics → dpm → resolver → guard
    // unique_ptr::reset() 自身对 nullptr 安全，无需冗余判空
    dispatcher_.reset();
    profileCoordinator_.reset();
    advController_.reset();
    smartCallTransferController_.reset();
    if (reconnectEngine_) {
        reconnectEngine_->Terminate();
    }
    reconnectEngine_.reset();
    metricsReporter_.reset();
    dpm_.reset();
    deviceTypeResolver_.reset();
    connectionGuard_.reset();
    prevHfpState_.clear();
    prevA2dpState_.clear();
    callReportMap_.clear();
    scoSwitchList_.clear();
    callIncomingTimeStampMs_ = 0;
    HILOGI("CarPolicyManager uninitialized");
}

void CarPolicyManager::AssembleReconnectPredicates()
{
    if (!reconnectEngine_) {
        return;
    }
    // 构造不会失败（unique_ptr 构造 + make_unique），统一不判空，与 IncomingConnPredicate 风格一致
    // IncomingConnPredicate 无外部依赖，直接读单例
    reconnectEngine_->AddPredicate(std::make_unique<IncomingConnPredicate>());

    // BtSettingsUiPredicate：注入 CarPolicyManager 引用查询缓存的 UI 状态
    // OnTerminate 内部直接调 AdapterManager 单例取消扫描 + ReportScanTerminate 上报
    reconnectEngine_->AddPredicate(std::make_unique<BtSettingsUiPredicate>(*this));

    // SmartCallOutPredicate：注入 SmartCallTransferController 引用
    reconnectEngine_->AddPredicate(std::make_unique<SmartCallOutPredicate>(*smartCallTransferController_));

    // CarplayMutexPredicate：注入 CarConnectionGuard 引用
    reconnectEngine_->AddPredicate(std::make_unique<CarplayMutexPredicate>(*connectionGuard_));

    HILOGI("Reconnect predicates assembled (Incoming+BtSettingsUi+SmartCallOut+CarplayMutex)");
}

void CarPolicyManager::HandlePolicyChanged()
{
    if (!connectionGuard_) {
        return;
    }
    if (!connectionGuard_->IsAllowConnA2dp()) {
        HILOGI("policy disallowed: disconnect all A2DP Sink");
        CarProfileService::NotifyPlaybackDisallowed();
        DisconnectAllA2dpSink();
        return;
    }
    HILOGI("policy allowed: try auto-connect A2DP by HFP state");
    AutoConnectA2dpByHfpState();
}

void CarPolicyManager::DisconnectAllA2dpSink()
{
    auto a2dpAddrs = connectionGuard_->GetA2dpConnectedAddrs();
    for (const auto &addr : a2dpAddrs) {
        HILOGI("disconnect A2DP Sink for policy change: %{public}s", addr.c_str());
        CarProfileService::Disconnect(ProfileType::A2DP_SINK, addr);
    }
}

void CarPolicyManager::AutoConnectA2dpByHfpState()
{
    auto hfpAddrs = connectionGuard_->GetHfpConnectedAddrs();
    for (const auto &addr : hfpAddrs) {
        if (CarProfileService::GetHfpDeviceState(addr) == static_cast<int>(BTConnectState::CONNECTED)) {
            HILOGI("auto-connect A2DP by HFP connected: %{public}s", addr.c_str());
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
            return;
        }
    }
    for (const auto &addr : hfpAddrs) {
        if (CarProfileService::GetHfpDeviceState(addr) == static_cast<int>(BTConnectState::CONNECTING)) {
            HILOGI("auto-connect A2DP by HFP connecting: %{public}s", addr.c_str());
            CarProfileService::Connect(ProfileType::A2DP_SINK, addr);
            return;
        }
    }
    HILOGI("no HFP connected/connecting device, skip auto-connect");
}

// ── 外部入口（切线程到 car policy 线程）──

void CarPolicyManager::StartReconnect(ReconnectReason reason)
{
    if (!initialized_ || !reconnectEngine_ || !dpm_ || !dispatcher_) {
        HILOGW("StartReconnect before Init");
        return;
    }
    // 在 car policy 线程内读取首个设备，避免 DevicePriorityManager 跨线程访问
    ReconnectEngine *engine = reconnectEngine_.get();
    DevicePriorityManager *dpm = dpm_.get();
    dispatcher_->Dispatch([engine, dpm, reason]() {
        std::string firstDevice = dpm->GetNextDevice("");
        if (firstDevice.empty()) {
            HILOGW("StartReconnect: no device in order list");
            return;
        }
        engine->StartReconnect(firstDevice, reason);
    });
}

void CarPolicyManager::OnBluetoothOn()
{
    HILOGI("OnBluetoothOn: trigger auto-reconnect (BT_ON)");
    // BT ON 时读取持久化的 FA/BtMusic 状态同步到 Guard。
    // DataShare 持久化存储确保蓝牙重启后仍能恢复上次状态，Guard 据此判断是否允许 A2DP 连接。
    // URI 常量由 CarPolicyDispatcher 统一持有，避免重复定义
    if (dispatcher_ && connectionGuard_) {
        int faStatus = CarConnectionGuard::STATUS_UNKNOWN;
        int btMusicStatus = CarConnectionGuard::STATUS_UNKNOWN;
        dispatcher_->LoadFaBtMusicStatus(faStatus, btMusicStatus);
        connectionGuard_->SetFaStatus(faStatus);
        connectionGuard_->SetBtMusicStatus(btMusicStatus);
    }
    // BT ON 时根据 BT 设置界面状态决定是否开可配对广播。
    HandleBtSettingUiStatusChange(false);
    StartReconnect(ReconnectReason::BT_ON_AUTO_CONN);
}

void CarPolicyManager::HandleBtSettingUiStatusChange(bool isForce)
{
    int newStatus = btSettingsUiActive_;
    HILOGI("HandleBtSettingUiStatusChange: isForce=%{public}d, lastStatus=%{public}d, newStatus=%{public}d",
           isForce, prevBtSettingsUiStatus_, newStatus);
    if (newStatus < 0) {
        return;
    }
    if (prevBtSettingsUiStatus_ == newStatus && !isForce) {
        return;
    }
    prevBtSettingsUiStatus_ = newStatus;

    // 已有车机设备连接/连接中时不操作广播（无需配对新设备）。
    if (IsCarConnectedOrConnecting()) {
        HILOGI("Car has connected or connecting device, skip adv");
        return;
    }
    if (!advController_) {
        return;
    }
    if (newStatus == BT_SETTING_STATUS_ON) {
        advController_->StartAdv();
    } else {
        advController_->StopAdv();
    }
}

bool CarPolicyManager::IsCarConnectedOrConnecting() const
{
    if (!connectionGuard_) {
        return true;
    }
    auto a2dpAddrs = connectionGuard_->GetA2dpConnectedAddrs();
    auto hfpAddrs = connectionGuard_->GetHfpConnectedAddrs();
    return !a2dpAddrs.empty() || !hfpAddrs.empty();
}

void CarPolicyManager::TerminateReconnect()
{
    if (reconnectEngine_) {
        reconnectEngine_->Terminate();
    }
}

void CarPolicyManager::OnAclDisconnect(const std::string &addr, int rawReason)
{
    if (!initialized_ || !reconnectEngine_ || !dispatcher_) {
        return;
    }
    ReconnectEngine *engine = reconnectEngine_.get();
    dispatcher_->Dispatch([engine, addr, rawReason]() {
        engine->OnAclDisconnect(addr, rawReason);
    });
}

// ── Observer 回调（已在 car policy 线程，由 Dispatcher 调用）──

void CarPolicyManager::OnProfileStateChanged(const std::string &addr, ProfileType profile,
                                              int nextState, int prevState)
{
    if (!initialized_) {
        return;
    }
    // 仅处理 HFP_HF / A2DP_SINK，避免 PBAP_PCE 等误写状态缓存
    if (profile != ProfileType::HFP_HF && profile != ProfileType::A2DP_SINK) {
        HILOGI("OnProfileStateChanged: ignore non-HFP/A2DP profile=%{public}d", static_cast<int>(profile));
        return;
    }
    // 解析 prevState（调用方传 -1 时从缓存 map 查询）
    int resolvedPrev = prevState;
    std::unordered_map<std::string, int> &stateMap =
        (profile == ProfileType::HFP_HF) ? prevHfpState_ : prevA2dpState_;
    if (resolvedPrev == -1) {
        auto it = stateMap.find(addr);
        resolvedPrev = (it != stateMap.end()) ? it->second : static_cast<int>(BTConnectState::DISCONNECTED);
    }
    // 更新缓存：DISCONNECTED 清除，其他状态记录
    if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        stateMap.erase(addr);
    } else {
        stateMap[addr] = nextState;
    }

    HILOGI("OnProfileStateChanged: addr=%{public}s, profile=%{public}d, next=%{public}d, prev=%{public}d",
           addr.c_str(), static_cast<int>(profile), nextState, resolvedPrev);

    // HFP/A2DP 任何状态变化都停可配对广播（已有设备连接/连接中，无需继续广播）。
    if ((profile == ProfileType::HFP_HF || profile == ProfileType::A2DP_SINK) && advController_) {
        advController_->StopAdv();
    }

    // ── HFP/A2DP 任一 CONNECTED → 终止回连引擎 ──
    // 任何设备占用 SinkLimit 名额后，回连其他设备无意义且会引发"断开→重连→又断开"
    // 循环冲突，故直接终止回连引擎。不区分是否为回连目标。
    if ((profile == ProfileType::HFP_HF || profile == ProfileType::A2DP_SINK)
        && nextState == static_cast<int>(BTConnectState::CONNECTED)) {
        if (reconnectEngine_) {
            reconnectEngine_->Terminate();
        }
    }

    // ── 全部 Profile 断开判定 ──
    if (nextState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        CheckAllProfilesDisconnected(addr);
    }

    // ── 转发给 ProfileCoordinator 处理协同动作 ──
    if (profileCoordinator_) {
        profileCoordinator_->OnProfileStateChanged(addr, profile, nextState, resolvedPrev);
    }
}

void CarPolicyManager::CheckAllProfilesDisconnected(const std::string &addr)
{
    auto hfpIt = prevHfpState_.find(addr);
    bool hfpDisconnected = (hfpIt == prevHfpState_.end()) ||
                            (hfpIt->second == static_cast<int>(BTConnectState::DISCONNECTED));
    auto a2dpIt = prevA2dpState_.find(addr);
    bool a2dpDisconnected = (a2dpIt == prevA2dpState_.end()) ||
                             (a2dpIt->second == static_cast<int>(BTConnectState::DISCONNECTED));
    if (hfpDisconnected && a2dpDisconnected) {
        HILOGI("All profiles disconnected: %{public}s", addr.c_str());
        if (reconnectEngine_) {
            reconnectEngine_->OnAllProfilesDisconnected(addr);
        }
    }
}

void CarPolicyManager::OnFaStatusChanged(int newStatus)
{
    if (!initialized_ || !connectionGuard_) {
        return;
    }
    connectionGuard_->SetFaStatus(newStatus);
    HandlePolicyChanged();
}

void CarPolicyManager::OnBtMusicStatusChanged(int newStatus)
{
    if (!initialized_ || !connectionGuard_) {
        return;
    }
    connectionGuard_->SetBtMusicStatus(newStatus);
    HandlePolicyChanged();
}

void CarPolicyManager::OnSmartCallTransferStatusChanged(int newStatus)
{
    if (!initialized_) {
        return;
    }
    if (smartCallTransferController_) {
        smartCallTransferController_->OnStatusChanged(newStatus);
    }
}

void CarPolicyManager::OnHiechoNotifyOutputPath(uint8_t a2dpOp, const std::string &addr)
{
    HILOGI("OnHiechoNotifyOutputPath: a2dpOp=%{public}u, addr=%{public}s", a2dpOp, addr.c_str());

    // 优先检查初始化状态，避免未初始化时构造广播 payload
    if (!initialized_ || !dispatcher_) {
        HILOGW("OnHiechoNotifyOutputPath: not initialized, drop");
        return;
    }

    // 将 echo 8,9 payload[2] 的 a2dpOperation 映射为广播 extra value：
    //   2(USE_A2DP)     → 1（FA 卡片在蓝牙音乐）
    //   1(NO_USE_A2DP)  → 0（FA 卡片不在蓝牙音乐）
    //   其它值忽略，不发送广播。
    int value = 0;
    if (a2dpOp == HIECHO_USE_A2DP) {
        value = SWITCH_BLUETOOTH_CARD_VALUE_ON;
    } else if (a2dpOp == HIECHO_NO_USE_A2DP) {
        value = SWITCH_BLUETOOTH_CARD_VALUE_OFF;
    } else {
        HILOGW("OnHiechoNotifyOutputPath: invalid a2dpOp=%{public}u, ignore", a2dpOp);
        return;
    }

    // 切到 car policy 线程串行执行，避免与其它策略事件并发竞争。
    dispatcher_->Dispatch([this, value]() { PublishSwitchBluetoothCardBroadcast(value); });
}

void CarPolicyManager::PublishSwitchBluetoothCardBroadcast(int value)
{
    HILOGI("PublishSwitchBluetoothCardBroadcast: value=%{public}d", value);

    // 构造公共事件：action + extra(key=switch_bluetooth_card, int value)
    // 参考 map_mse_content_observer.cpp::PublishRefreshSmsListEvent 的写法。
    AAFwk::Want want;
    want.SetAction(SWITCH_BLUETOOTH_CARD_ACTION);
    want.SetParam(SWITCH_BLUETOOTH_CARD_EXTRA_KEY, value);

    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(value);

    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSticky(false);

    bool result = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    HILOGI("PublishSwitchBluetoothCardBroadcast: action=%{public}s, result=%{public}d",
           SWITCH_BLUETOOTH_CARD_ACTION, result);
}

void CarPolicyManager::OnBondStateChanged(const std::string &addr, int newState)
{
    if (!initialized_) {
        return;
    }
    int prevState = PAIR_NONE;
    auto it = prevBondState_.find(addr);
    if (it != prevBondState_.end()) {
        prevState = it->second;
    }
    HILOGI("OnBondStateChanged: addr=%{public}s, newState=%{public}d, prevState=%{public}d",
           addr.c_str(), newState, prevState);
    prevBondState_[addr] = newState;

    if (newState == PAIR_NONE && prevState == PAIR_PAIRED) {
        // 解配对：SinkLimit 允许时强制刷新可配对广播。
        // 设备解配对后释放 SinkLimit 名额，若仍有配对容量则重新开启广播允许配对新设备。
        if (connectionGuard_ && connectionGuard_->IsConnectionAllowedBySinkLimit(addr)) {
            HandleBtSettingUiStatusChange(true);
        }
    } else if (newState == PAIR_PAIRED) {
        // 配对成功：仅停止可配对广播，不触发回连。
        // 回连由 BT ON 或 ACL 断开等事件触发，配对成功本身不触发回连。
        if (advController_) {
            advController_->StopAdv();
        }
    }
}

void CarPolicyManager::OnBtSettingsUiChanged(int newStatus)
{
    if (!initialized_) {
        return;
    }
    HILOGI("OnBtSettingsUiChanged: newStatus=%{public}d", newStatus);
    btSettingsUiActive_ = newStatus;
    // BT 设置界面状态变化 → 控制可配对广播。
    HandleBtSettingUiStatusChange(false);
}

void CarPolicyManager::ReportScanTerminate(const std::string &reason)
{
    if (metricsReporter_) {
        metricsReporter_->ReportScanTerminate(reason);
    }
}

void CarPolicyManager::OnCallChanged(const std::string &addr, int callState)
{
    if (!initialized_) {
        return;
    }
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string nowStr = std::to_string(nowMs);
    HILOGI("OnCallChanged: addr=%{public}s, callState=%{public}d", addr.c_str(), callState);
    if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_INCOMING)) {
        callIncomingTimeStampMs_ = nowMs;
        callReportMap_["CALL_INCOMING"] = nowStr;
    } else if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_ACTIVE)) {
        if (callReportMap_.find("CALL_INCOMING") == callReportMap_.end()) {
            callIncomingTimeStampMs_ = nowMs;
            callReportMap_["CALL_INCOMING"] = nowStr;
        }
        callReportMap_["CALL_START"] = nowStr;
    } else if (callState == static_cast<int>(HfpCallState::HFP_CALL_STATE_FINISHED)) {
        callReportMap_["CALL_END"] = nowStr;
        if (callReportMap_.find("CALL_START") != callReportMap_.end()) {
            if (metricsReporter_) {
                metricsReporter_->ReportCallRecord(callReportMap_, scoSwitchList_);
            }
            HILOGI("Call record reported, clearing maps");
        }
        callReportMap_.clear();
        scoSwitchList_.clear();
        callIncomingTimeStampMs_ = 0;
    }
}

void CarPolicyManager::OnScoSwitchChanged(const std::string &addr, int scoState)
{
    if (!initialized_ || callIncomingTimeStampMs_ == 0) {
        return;
    }
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    // 防御时钟回拨：nowMs 小于来电时间戳时跳过，避免负数导致 SCO 切换记录异常
    if (nowMs < callIncomingTimeStampMs_) {
        HILOGW("OnScoSwitchChanged: clock rollback, nowMs=%{public}lld < incoming=%{public}lld, skip",
               static_cast<long long>(nowMs), static_cast<long long>(callIncomingTimeStampMs_));
        return;
    }
    int64_t switchSec = (nowMs - callIncomingTimeStampMs_) / 1000;
    scoSwitchList_.push_back(std::to_string(scoState) + ":" + std::to_string(switchSec));
    HILOGI("OnScoSwitchChanged: addr=%{public}s, state=%{public}d, sec=%{public}lld",
           addr.c_str(), scoState, static_cast<long long>(switchSec));
}

}  // namespace bluetooth
}  // namespace OHOS
