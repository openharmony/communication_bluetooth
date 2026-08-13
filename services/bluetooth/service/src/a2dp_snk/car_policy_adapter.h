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

#ifndef CAR_POLICY_ADAPTER_H
#define CAR_POLICY_ADAPTER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "base_def.h"
#include "interface_profile_a2dp_snk.h"
#include "interface_profile_hfp_hf.h"

namespace OHOS {
namespace EventFwk {
class CommonEventSubscriber;
}  // namespace EventFwk
namespace bluetooth {

class A2dpSnkService;
class HfpHfService;

/// 车载蓝牙策略适配器（单例）。
///
/// 监听四类异步事件，仲裁 A2DP Sink 的连接与断开：
/// 1. FA 卡片广播（usual.event.carmediacenter.music_status）
///    faMediaStatus: 0=不在蓝牙音乐, 1=在蓝牙音乐，默认 0。
/// 2. 蓝牙音乐 APP 广播（usual.event.bluetooth.bt_music_status_change）
///    btMusicStatus: 0=后台, 1=前台。
/// 3. HFP-HF 连接状态变化（内部类 HfpHfObserver）。
/// 4. A2DP Sink 连接状态变化（内部类 A2dpSnkObserver）。
///
/// 仅当 FA 在蓝牙音乐或蓝牙音乐 APP 在前台时允许 A2DP Sink 连接；
/// 不允许时主动断开所有已连接的 A2DP Sink 设备。
/// HFP-HF 连接成功后延迟 15s 自动连接 A2DP Sink（等待 SLC 稳定）。
///
/// Init/Uninit 由 AdapterManager 在蓝牙开启/关闭后调用，
/// 保证 A2dpSnkService 和 HfpHfService 均已启动。
/// 所有逻辑运行在独立的 car policy 线程上。
class CarPolicyAdapter {
public:
    static constexpr int FA_STATUS_OFF = 0;       // FA 卡片不在蓝牙音乐
    static constexpr int FA_STATUS_BT_ON = 1;     // FA 卡片在蓝牙音乐
    static constexpr int BT_MUSIC_STATUS_OFF = 0;  // 蓝牙音乐 APP 在后台
    static constexpr int BT_MUSIC_STATUS_ON = 1;   // 蓝牙音乐 APP 在前台

    static CarPolicyAdapter &GetInstance();
    static void ResetInstanceForTest(std::unique_ptr<CarPolicyAdapter> newInstance);

    ~CarPolicyAdapter() = default;

    /// 注册广播订阅和 profile 观察者，在蓝牙开启后调用。
    void Init();
    /// 注销广播订阅和 profile 观察者，在蓝牙关闭后调用。
    void Uninit();

    /// 是否允许 A2DP Sink 连接（FA 在蓝牙音乐或蓝牙音乐 APP 在前台）。
    bool IsAllowConnA2dp() const;
    int GetFaStatus() const;
    int GetBtMusicStatus() const;

    /// 车载连接数限制检查（A2dpSink + HfpHf 合计不超过 CAR_MAX_CONN）。
    bool IsConnectionAllowedBySinkLimit(const std::string &address) const;
    /// 踢掉除指定设备外的所有 A2dpSink 和 HfpHf 连接。
    bool DisconnectOthersForSinkLimit(const std::string &address);

protected:
    CarPolicyAdapter();

private:
    // ── 广播订阅内部类 ──
    class FaMediaStatusSubscriber;
    class BtMusicStatusSubscriber;

    // ── profile 观察者内部类 ──
    class HfpHfObserver : public HfpHfServiceObserver {
    public:
        explicit HfpHfObserver(CarPolicyAdapter &owner) : owner_(owner) {}
        ~HfpHfObserver() override = default;
        void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override;
    private:
        CarPolicyAdapter &owner_;
    };
    class A2dpSnkObserver : public IA2dpSnkObserver {
    public:
        explicit A2dpSnkObserver(CarPolicyAdapter &owner) : owner_(owner) {}
        ~A2dpSnkObserver() override = default;
        void OnConnectionStateChanged(const RawAddress &remoteAddr, int state, int cause) override;
    private:
        CarPolicyAdapter &owner_;
    };

    // ── 状态变更处理（car policy 线程）──
    void HandleFaStatusChange(int newFaStatus);
    void HandleBtMusicStatusChange(int newStatus);
    void ProcessA2dpSinkStateChanged(const std::string &device, int state);
    void ProcessHfpHfStateChanged(const std::string &device, int state);

    // ── 连接仲裁（car policy 线程）──
    void OnPolicyChanged();
    void AutoConnectA2dp(const std::string &device);
    void DisconnectAllA2dpSink();
    void AutoConnectByHfpState();

    // ── 观察者注册 ──
    void SubscribeFaStatusBroadcast();
    void SubscribeBtMusicStatusBroadcast();
    void UnsubscribeBroadcasts();
    void RegisterHfpHfObserver();
    void DeregisterHfpHfObserver();
    void RegisterA2dpSnkObserver();
    void DeregisterA2dpSnkObserver();

    static constexpr int CAR_MAX_CONN = 1;
    // HFP-HF 连接成功后延迟自动连接 A2DP 的时间（毫秒），等待 SLC 稳定。
    static constexpr uint64_t RECONNECT_A2DP_DELAY_MS = 15000;

    int faStatus_{FA_STATUS_OFF};
    int btMusicStatus_{BT_MUSIC_STATUS_OFF};
    // 缓存各设备 HFP-HF 上次连接状态，用于推断 CONNECTING→DISCONNECTED（连接失败）转换。
    std::map<std::string, int> prevHfpState_;

    std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> faStatusSubscriber_{nullptr};
    std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> btMusicStatusSubscriber_{nullptr};
    std::unique_ptr<HfpHfObserver> hfpHfObserver_{nullptr};
    std::unique_ptr<A2dpSnkObserver> a2dpSnkObserver_{nullptr};

    static std::mutex instanceMutex_;
    static std::unique_ptr<CarPolicyAdapter> globalInstance_;

    BT_DISALLOW_COPY_AND_ASSIGN(CarPolicyAdapter);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_ADAPTER_H
