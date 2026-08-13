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

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_adapter"
#endif

#include "car_policy_adapter.h"

#include <set>
#include <vector>

#include "a2dp_snk_def.h"
#include "a2dp_snk_service.h"
#include "bluetooth_def.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "hfp_hf_service.h"
#include "log.h"
#include "raw_address.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {

// FA 卡片音乐状态广播 action，携带 int code: 0=不在蓝牙音乐, 1=在蓝牙音乐
static const char *const ACTION_FA_MUSIC_STATUS_CHANGE = "usual.event.carmediacenter.music_status";
// 蓝牙音乐 APP 状态广播 action，携带 int code: 0=后台, 1=前台
static const char *const ACTION_BT_MUSIC_STATUS_CHANGE = "usual.event.bluetooth.bt_music_status_change";

std::mutex CarPolicyAdapter::instanceMutex_;
std::unique_ptr<CarPolicyAdapter> CarPolicyAdapter::globalInstance_{nullptr};

// ── 广播订阅内部类：接收 FA 卡片音乐状态广播，切到 car policy 线程处理 ──
class CarPolicyAdapter::FaMediaStatusSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    FaMediaStatusSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info, CarPolicyAdapter &owner)
        : OHOS::EventFwk::CommonEventSubscriber(info), owner_(owner) {}
    ~FaMediaStatusSubscriber() override = default;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override
    {
        // CommonEvent 回调运行在公共事件线程，需切到 car policy 线程处理
        int newFaStatus = data.GetCode();
        DoInCarPolicyThread([this, newFaStatus]() { owner_.HandleFaStatusChange(newFaStatus); });
    }
private:
    CarPolicyAdapter &owner_;
};

// ── 广播订阅内部类：接收蓝牙音乐 APP 状态广播，切到 car policy 线程处理 ──
class CarPolicyAdapter::BtMusicStatusSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    BtMusicStatusSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info, CarPolicyAdapter &owner)
        : OHOS::EventFwk::CommonEventSubscriber(info), owner_(owner) {}
    ~BtMusicStatusSubscriber() override = default;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override
    {
        // CommonEvent 回调运行在公共事件线程，需切到 car policy 线程处理
        int newStatus = data.GetCode();
        DoInCarPolicyThread([this, newStatus]() { owner_.HandleBtMusicStatusChange(newStatus); });
    }
private:
    CarPolicyAdapter &owner_;
};

// ── 单例 ──

CarPolicyAdapter::CarPolicyAdapter()
{
    HILOGI("CarPolicyAdapter created");
}

CarPolicyAdapter &CarPolicyAdapter::GetInstance()
{
    std::lock_guard<std::mutex> lk(instanceMutex_);
    if (!globalInstance_) {
        globalInstance_.reset(new CarPolicyAdapter());
    }
    return *globalInstance_;
}

void CarPolicyAdapter::ResetInstanceForTest(std::unique_ptr<CarPolicyAdapter> newInstance)
{
    std::lock_guard<std::mutex> lk(instanceMutex_);
    globalInstance_ = std::move(newInstance);
}

// ── Init / Uninit ──

void CarPolicyAdapter::Init()
{
    // 蓝牙开启后由 AdapterManager::ExecuteTaskWhenBluetoothOn 调用，
    // 此时 A2dpSnkService 和 HfpHfService 均已启动完成
    HILOGI("CarPolicyAdapter init, faStatus=%{public}d, btMusicStatus=%{public}d", faStatus_, btMusicStatus_);
    SubscribeFaStatusBroadcast();
    SubscribeBtMusicStatusBroadcast();
    RegisterHfpHfObserver();
    RegisterA2dpSnkObserver();
}

void CarPolicyAdapter::Uninit()
{
    // 蓝牙关闭后由 AdapterManager::UnLoadBluetoothSystemAbility 调用
    HILOGI("CarPolicyAdapter uninit");
    UnsubscribeBroadcasts();
    DeregisterHfpHfObserver();
    DeregisterA2dpSnkObserver();
    prevHfpState_.clear();
}

// ── 查询接口 ──

bool CarPolicyAdapter::IsAllowConnA2dp() const
{
    // FA 卡片在蓝牙音乐 或 蓝牙音乐 APP 在前台时允许 A2DP Sink 连接
    if (faStatus_ == FA_STATUS_BT_ON || btMusicStatus_ == BT_MUSIC_STATUS_ON) {
        return true;
    }

    HILOGI("not allowed, faStatus: %{public}d, btMusicStatus: %{public}d", faStatus_, btMusicStatus_);
    return false;
}

int CarPolicyAdapter::GetFaStatus() const { return faStatus_; }
int CarPolicyAdapter::GetBtMusicStatus() const { return btMusicStatus_; }

// ── 状态变更处理（car policy 线程）──

void CarPolicyAdapter::HandleFaStatusChange(int newFaStatus)
{
    HILOGI("FA status changed, prev: %{public}d, new: %{public}d, btMusicStatus: %{public}d",
        faStatus_, newFaStatus, btMusicStatus_);
    faStatus_ = newFaStatus;
    // FA 离开蓝牙音乐后，蓝牙音乐 APP 的前后台状态不再有意义，重置为后台
    if (faStatus_ != FA_STATUS_BT_ON && btMusicStatus_ == BT_MUSIC_STATUS_ON) {
        HILOGI("FA left BT music, reset btMusicStatus to OFF");
        btMusicStatus_ = BT_MUSIC_STATUS_OFF;
    }
    // 策略变为不允许时，通知流状态机禁止播放
    auto *service = A2dpSnkService::GetService();
    if (!IsAllowConnA2dp() && service != nullptr) {
        HILOGI("policy disallowed, notify playback disallowed");
        service->NotifyPlaybackDisallowed();
    }
    OnPolicyChanged();
}

void CarPolicyAdapter::HandleBtMusicStatusChange(int newStatus)
{
    HILOGI("BT music status changed, prev: %{public}d, new: %{public}d, faStatus: %{public}d",
        btMusicStatus_, newStatus, faStatus_);
    btMusicStatus_ = newStatus;
    // 策略变为不允许时（仅当 FA 也不在蓝牙音乐时），通知流状态机禁止播放
    auto *service = A2dpSnkService::GetService();
    if (!IsAllowConnA2dp() && service != nullptr) {
        HILOGI("policy disallowed, notify playback disallowed");
        service->NotifyPlaybackDisallowed();
    }
    OnPolicyChanged();
}

void CarPolicyAdapter::ProcessA2dpSinkStateChanged(const std::string &device, int state)
{
    HILOGI("A2DP Sink state: device=%{public}s, state=%{public}d, faStatus=%{public}d, btMusicStatus=%{public}d",
        GetEncryptAddr(device).c_str(), state, faStatus_, btMusicStatus_);
    auto *service = A2dpSnkService::GetService();
    if (service == nullptr) {
        HILOGE("A2dpSnkService is nullptr");
        return;
    }
    if (state == A2DP_SNK_STATE_CONNECTED) {
        // A2DP Sink 连接成功：先踢掉其他设备以满足车载单连接限制
        HILOGI("A2DP Sink connected, disconnect others for sink limit");
        DisconnectOthersForSinkLimit(device);
        if (IsAllowConnA2dp()) {
            // 策略允许：通知流状态机 A2DP 已连接，可请求音频焦点
            HILOGI("policy allowed, notify A2DP connected with focus");
            service->NotifyA2dpConnected(true);
        } else {
            // 策略不允许：立即断开刚连接上的 A2DP Sink
            HILOGI("policy not allowed, disconnect device=%{public}s", GetEncryptAddr(device).c_str());
            service->Disconnect(RawAddress(device));
        }
    } else if (state == A2DP_SNK_STATE_DISCONNECTED) {
        // A2DP Sink 断开：通知流状态机清理播放资源
        HILOGI("A2DP Sink disconnected, notify device disconnected");
        service->NotifyDeviceDisconnected();
    }
}

void CarPolicyAdapter::ProcessHfpHfStateChanged(const std::string &device, int state)
{
    // HfpHfServiceObserver 不提供 prevState，用 prevHfpState_ 缓存推断
    int prevState = static_cast<int>(BTConnectState::DISCONNECTED);
    auto it = prevHfpState_.find(device);
    if (it != prevHfpState_.end()) {
        prevState = it->second;
    }

    // 更新缓存：断开后清除，其他状态记录
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        prevHfpState_.erase(it);
    } else {
        prevHfpState_[device] = state;
    }
    HILOGI("HFP-HF state: device=%{public}s, prev=%{public}d, state=%{public}d, faStatus=%{public}d, btMusicStatus=%{public}d",
        GetEncryptAddr(device).c_str(), prevState, state, faStatus_, btMusicStatus_);

    if (state == static_cast<int>(BTConnectState::CONNECTED)) {

        // HFP-HF 连接成功：踢掉其他设备以满足车载单连接限制
        HILOGI("HFP-HF connected, disconnect others for sink limit");
        DisconnectOthersForSinkLimit(device);
        if (IsAllowConnA2dp()) {
            // 策略允许：延迟 15s 自动连接 A2DP Sink，等待 HFP SLC 稳定后再拉起 A2DP
            HILOGI("policy allowed, schedule A2DP auto-connect for %{public}s after %{public}llu ms",
                GetEncryptAddr(device).c_str(), static_cast<unsigned long long>(RECONNECT_A2DP_DELAY_MS));
            DoInCarPolicyThread([this, device]() { AutoConnectA2dp(device); }, RECONNECT_A2DP_DELAY_MS);
        } else {
            HILOGI("policy not allowed, skip A2DP auto-connect");
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {

        // HFP 连接失败（CONNECTING→DISCONNECTED）：双框架逻辑中会立即尝试连接 A2DP
        if (prevState == static_cast<int>(BTConnectState::CONNECTING) && IsAllowConnA2dp()) {
            HILOGI("HFP connect failed (CONNECTING→DISCONNECTED), immediately auto-connect A2DP for %{public}s",
                GetEncryptAddr(device).c_str());
            AutoConnectA2dp(device);
        } else {
            HILOGI("HFP disconnected, prev=%{public}d, no auto-connect action", prevState);
        }
    }
}

void CarPolicyAdapter::OnPolicyChanged()
{
    // FA/BtMusic 策略变化后的仲裁入口
    if (!IsAllowConnA2dp()) {
        // 策略不允许：主动断开所有正在连接/已连接的 A2DP Sink 设备
        HILOGI("policy not allowed, disconnect all A2DP sink devices");
        DisconnectAllA2dpSink();
        return;
    }
    // 策略允许：按 HFP-HF 连接状态自动连接 A2DP Sink
    HILOGI("policy allowed, try auto-connect A2DP by HFP-HF state");
    AutoConnectByHfpState();
}

void CarPolicyAdapter::AutoConnectA2dp(const std::string &device)
{
    auto *service = A2dpSnkService::GetService();
    if (service == nullptr) {
        HILOGW("A2dpSnkService is nullptr, cannot connect to %{public}s", GetEncryptAddr(device).c_str());
        return;
    }

    if (!IsAllowConnA2dp()) {
        HILOGW("auto-connect skipped, FA/BtMusic policy not allowed, device=%{public}s",
            GetEncryptAddr(device).c_str());
        return;
    }
    HILOGI("auto-connect A2DP to %{public}s", GetEncryptAddr(device).c_str());
    service->Connect(RawAddress(device));
}

void CarPolicyAdapter::DisconnectAllA2dpSink()
{
    auto *service = A2dpSnkService::GetService();
    if (service == nullptr) {
        HILOGW("A2dpSnkService is nullptr, cannot disconnect");
        return;
    }
    std::vector<int32_t> states = {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED)
    };
    auto devices = service->GetDevicesByStates(states);
    HILOGI("found %{public}zu A2DP sink device(s) to disconnect", devices.size());
    for (const auto &dev : devices) {
        HILOGI("disconnect A2DP sink device=%{public}s", GetEncryptAddr(dev.GetAddress()).c_str());
        service->Disconnect(dev);
    }
}

void CarPolicyAdapter::AutoConnectByHfpState()
{
    auto *hfpHfService = HfpHfService::GetService();
    if (hfpHfService == nullptr) {
        HILOGW("HfpHfService is nullptr, no auto-connect candidate");
        return;
    }

    // 优先选择 HFP-HF 已连接的设备
    std::vector<int32_t> connectedStates = { static_cast<int32_t>(BTConnectState::CONNECTED) };
    auto connected = hfpHfService->GetDevicesByStates(connectedStates);
    if (!connected.empty()) {
        HILOGI("auto-connect by HFP-HF connected device=%{public}s",
            GetEncryptAddr(connected.front().GetAddress()).c_str());
        AutoConnectA2dp(connected.front().GetAddress());
        return;
    }

    // 其次选择 HFP-HF 正在连接的设备
    std::vector<int32_t> connectingStates = { static_cast<int32_t>(BTConnectState::CONNECTING) };
    auto connecting = hfpHfService->GetDevicesByStates(connectingStates);
    if (!connecting.empty()) {
        HILOGI("auto-connect by HFP-HF connecting device=%{public}s",
            GetEncryptAddr(connecting.front().GetAddress()).c_str());
        AutoConnectA2dp(connecting.front().GetAddress());
        return;
    }
    HILOGI("no HFP-HF connected/connecting device, skip auto-connect");
}

// ── 观察者注册 ──

void CarPolicyAdapter::SubscribeFaStatusBroadcast()
{
    if (faStatusSubscriber_ != nullptr) {
        HILOGW("FA status broadcast already subscribed");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(ACTION_FA_MUSIC_STATUS_CHANGE);
    OHOS::EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    faStatusSubscriber_ = std::make_shared<FaMediaStatusSubscriber>(subscribeInfo, *this);
    if (!OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(faStatusSubscriber_)) {
        HILOGE("Subscribe FA music status broadcast failed");
        faStatusSubscriber_.reset();
    } else {
        HILOGI("FA music status broadcast subscribed");
    }
}

void CarPolicyAdapter::SubscribeBtMusicStatusBroadcast()
{
    if (btMusicStatusSubscriber_ != nullptr) {
        HILOGW("BT music status broadcast already subscribed");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(ACTION_BT_MUSIC_STATUS_CHANGE);
    OHOS::EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    btMusicStatusSubscriber_ = std::make_shared<BtMusicStatusSubscriber>(subscribeInfo, *this);
    if (!OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(btMusicStatusSubscriber_)) {
        HILOGE("Subscribe BT music status broadcast failed");
        btMusicStatusSubscriber_.reset();
    } else {
        HILOGI("BT music status broadcast subscribed");
    }
}

void CarPolicyAdapter::UnsubscribeBroadcasts()
{
    if (faStatusSubscriber_ != nullptr) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(faStatusSubscriber_);
        faStatusSubscriber_.reset();
        HILOGI("FA music status broadcast unsubscribed");
    }
    if (btMusicStatusSubscriber_ != nullptr) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(btMusicStatusSubscriber_);
        btMusicStatusSubscriber_.reset();
        HILOGI("BT music status broadcast unsubscribed");
    }
}

void CarPolicyAdapter::RegisterHfpHfObserver()
{
    if (hfpHfObserver_ != nullptr) {
        return;
    }
    auto *hfpHfService = HfpHfService::GetService();
    if (hfpHfService == nullptr) {
        HILOGE("HfpHfService is nullptr, observer not registered");
        return;
    }
    hfpHfObserver_ = std::make_unique<HfpHfObserver>(*this);
    hfpHfService->RegisterObserver(*hfpHfObserver_);
    HILOGI("HFP-HF observer registered");
}

void CarPolicyAdapter::DeregisterHfpHfObserver()
{
    if (hfpHfObserver_ == nullptr) {
        return;
    }
    auto *hfpHfService = HfpHfService::GetService();
    if (hfpHfService != nullptr) {
        hfpHfService->DeregisterObserver(*hfpHfObserver_);
    }
    hfpHfObserver_.reset();
    HILOGI("HFP-HF observer deregistered");
}

void CarPolicyAdapter::RegisterA2dpSnkObserver()
{
    if (a2dpSnkObserver_ != nullptr) {
        return;
    }
    auto *a2dpSnkService = A2dpSnkService::GetService();
    if (a2dpSnkService == nullptr) {
        HILOGE("A2dpSnkService is nullptr, observer not registered");
        return;
    }
    a2dpSnkObserver_ = std::make_unique<A2dpSnkObserver>(*this);
    a2dpSnkService->RegisterObserver(a2dpSnkObserver_.get());
    HILOGI("A2DP Sink observer registered");
}

void CarPolicyAdapter::DeregisterA2dpSnkObserver()
{
    if (a2dpSnkObserver_ == nullptr) {
        return;
    }
    auto *a2dpSnkService = A2dpSnkService::GetService();
    if (a2dpSnkService != nullptr) {
        a2dpSnkService->DeregisterObserver(a2dpSnkObserver_.get());
    }
    a2dpSnkObserver_.reset();
    HILOGI("A2DP Sink observer deregistered");
}

// ── profile 观察者内部类实现 ──

void CarPolicyAdapter::HfpHfObserver::OnConnectionStateChanged(const RawAddress &device, int state, int cause)
{
    std::string address = device.GetAddress();
    HILOGI("HFP-HF observer: device=%{public}s, state=%{public}d, cause=%{public}d",
        GetEncryptAddr(address).c_str(), state, cause);
    // HFP-HF 回调运行在 HFP-HF 线程，切到 car policy 线程处理
    DoInCarPolicyThread([this, address, state]() { owner_.ProcessHfpHfStateChanged(address, state); });
}

void CarPolicyAdapter::A2dpSnkObserver::OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause)
{
    std::string address = remoteAddr.GetAddress();
    HILOGI("A2DP Sink observer: device=%{public}s, state=%{public}d, cause=%{public}d",
        GetEncryptAddr(address).c_str(), state, cause);
    // A2DP Sink 回调运行在 A2DP Sink 线程，切到 car policy 线程处理
    DoInCarPolicyThread([this, address, state]() { owner_.ProcessA2dpSinkStateChanged(address, state); });
}

// ── 连接数限制 ──

bool CarPolicyAdapter::IsConnectionAllowedBySinkLimit(const std::string &address) const
{
    // 统计当前 A2dpSink + HfpHf 正在连接/已连接的设备总数
    std::set<std::string> connectedSet;
    std::vector<int32_t> states = {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED)
    };
    auto *a2dpSnkService = A2dpSnkService::GetService();
    if (a2dpSnkService) {
        for (const auto &dev : a2dpSnkService->GetDevicesByStates(states)) {
            connectedSet.insert(dev.GetAddress());
        }
    }
    auto *hfpHfService = HfpHfService::GetService();
    if (hfpHfService) {
        for (const auto &dev : hfpHfService->GetDevicesByStates(states)) {
            connectedSet.insert(dev.GetAddress());
        }
    }
    // 目标设备已在连接集合中，直接允许
    if (connectedSet.count(address) > 0) {
        return true;
    }
    // 未达车载最大连接数才允许
    return static_cast<int>(connectedSet.size()) < CAR_MAX_CONN;
}

bool CarPolicyAdapter::DisconnectOthersForSinkLimit(const std::string &address)
{
    // 踢掉除目标设备外所有正在连接/已连接的 A2dpSink 和 HfpHf 设备
    auto *a2dpSnkService = A2dpSnkService::GetService();
    auto *hfpHfService = HfpHfService::GetService();
    if (a2dpSnkService == nullptr || hfpHfService == nullptr) {
        HILOGE("service is nullptr, a2dp=%{public}d, hfp=%{public}d",
            a2dpSnkService != nullptr, hfpHfService != nullptr);
        return false;
    }
    std::vector<int32_t> states = {
        static_cast<int32_t>(BTConnectState::CONNECTING),
        static_cast<int32_t>(BTConnectState::CONNECTED)
    };
    // 断开其他 A2DP Sink 设备
    for (const auto &dev : a2dpSnkService->GetDevicesByStates(states)) {
        if (dev.GetAddress() != address) {
            HILOGI("disconnect other A2DP sink: %{public}s", GetEncryptAddr(dev.GetAddress()).c_str());
            a2dpSnkService->Disconnect(dev);
        }
    }
    // 断开其他 HFP-HF 设备
    for (const auto &dev : hfpHfService->GetDevicesByStates(states)) {
        if (dev.GetAddress() != address) {
            HILOGI("disconnect other HFP-HF: %{public}s", GetEncryptAddr(dev.GetAddress()).c_str());
            hfpHfService->Disconnect(dev);
        }
    }
    // FSL_TODO: pbap profile disconnect, and terminateAutoConnect
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
