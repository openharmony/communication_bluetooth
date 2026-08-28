/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @file a2dp_snk_service.h
 *
 * @brief 声明 A2DP Sink profile 服务类，包含属性和方法。
 *
 * @since 6
 */

#ifndef A2DP_SNK_SERVICE_H
#define A2DP_SNK_SERVICE_H

#include <cstdint>
#include <cstring>
#include <unordered_set>

#include "bt_av.h"
#include "context.h"
#include "a2dp_snk_def.h"
#include "bt_recursive_mutex.h"
#include "a2dp_snk_message.h"
#include "base_observer_list.h"
#include "interface_profile_a2dp_snk.h"
#include "a2dp_snk_state_machine.h"
#include "service_util.h"
#include "bt_def.h"
#include "i_a2dp_snk_stream_state_machine.h"
#include "i_native_a2dp_adapter.h"

namespace OHOS {
namespace bluetooth {

class A2dpSnkService : public IProfileA2dpSnk, public utility::Context, public INativeA2dpSinkCallback {
public:
    utility::Context *GetContext() override;

    static A2dpSnkService *GetService();

    A2dpSnkService();

    ~A2dpSnkService() override;

    void Enable() override;

    void Disable() override;

    int Connect(const RawAddress &device) override;

    int Disconnect(const RawAddress &device) override;

    void RegisterObserver(IA2dpSnkObserver *observer) override;

    virtual void NotifyConnStateChanged(const RawAddress &device, int prevState, int state, int cause);

    void DeregisterObserver(IA2dpSnkObserver *observer) override;

    virtual bool NativeConnect(const RawAddress &device);
    virtual bool NativeDisconnect(const RawAddress &device);
    void SetAudioFocusState(int focusState);

    virtual void PostEvent(const A2dpSnkMessage &msg);

    int SetActiveSrcDevice(const RawAddress &device) override;

    void ProcessRemoveStateMachine(const std::string &device);

    int GetAudioConfig(const RawAddress &device, A2dpSnkCodecInfo &a2dpSnkCodecInfo);

    virtual bool AcceptIncomingConnection(const RawAddress &device);

    std::shared_ptr<INativeA2dpAdapter> GetNativeAdapter() const;
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) override;

    int GetDeviceState(const RawAddress &device) override;
    int GetPlayingState(const RawAddress &device, int &state) override;
    int SetConnectStrategy(const RawAddress &device, int32_t strategy) override;
    int GetConnectStrategy(const RawAddress &device) override;
    int SendDelay(const RawAddress &device, int32_t delayValue) override;

    void SetNativeAdapter(std::shared_ptr<INativeA2dpAdapter> adapter);
    std::shared_ptr<IA2dpSnkStreamStateMachine> GetStreamStateMachine() const;


    void SetStreamStateMachine(std::shared_ptr<IA2dpSnkStreamStateMachine> sm);

    // 活跃源设备地址访问器（供流状态机广播时获取附加信息使用）
    std::string GetActiveDeviceAddr() const
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        return activeSrcDevice_.GetAddress();
    }

    void NotifyUserPlayIntent();
    void NotifyUserPauseIntent();
    void NotifyPlaybackDisallowed();
    void NotifyA2dpConnected(bool allowed);
    void NotifyDeviceDisconnected();
    void NotifyDelayedPause();
    bool CanPlay() const;

    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState() override;
    int GetMaxConnectNum() override;

    // --- INativeA2dpSinkCallback 实现 ---
    void OnNativeConnectionStateChanged(const RawAddress &device, BtavConnectionState state) override;
    void OnNativeAudioStateChanged(const RawAddress &device, BtavAudioState state) override;
    void OnNativeAudioConfigChanged(const RawAddress &device, uint32_t sampleRate, uint8_t channelCount) override;

private:
    void StartUp();
    void ShutDown();
    void ShutDownDone(bool isAllDisconnected);
    void ProcessEvent(const A2dpSnkMessage &event);
    void ProcessConnectEvent(const A2dpSnkMessage &event);
    void ProcessDisconnectEvent(const A2dpSnkMessage &event);
    void ProcessStackEvent(const A2dpSnkMessage &event);

    // 连接准入检查：策略 + 车载策略 + 连接数限制（Connect 和 AcceptIncomingConnection 公共逻辑）
    bool CheckConnectionAdmission(const RawAddress &device);
    // EDM 白名单检查
    bool CheckEdmWhitelist(const std::string &address);
    // 查状态机确认设备未在连接中/已连接
    bool CheckDeviceNotConnected(const std::string &address);

    bool isStarted_{false};
    BtavSinkInterface *btAvSinkInterface_{nullptr};
    int maxConnectAudioDevices_{A2DP_SNK_CONNECT_NUM_MAX};
    bool isShuttingDown_{false};
    BaseObserverList<IA2dpSnkObserver> a2dpSnkFwObservers_{};
    // 活跃源设备（通过 SetActiveSrcDevice 设置）。
    // 全局流状态机反映该设备的播放状态。
    RawAddress activeSrcDevice_{""};
    BtRecursiveMutex mutable mutex_{};
    std::unordered_map<std::string, std::unique_ptr<A2dpSnkStateMachine>> stateMachines_{};

    std::shared_ptr<IA2dpSnkStreamStateMachine> streamStateMachine_{nullptr};
    // streamStateMachine_ 专用锁 — 与 mutex_ 分离，避免连接状态机 map
    // 操作与流状态机通知之间的锁竞争。
    // 使用递归锁，确保 Notify* 方法在同线程重入 Service 时
    // （例如流状态机回调 NotifyConnStateChanged）仍然安全。
    mutable BtRecursiveMutex streamSmMutex_{};
    std::shared_ptr<INativeA2dpAdapter> nativeAdapter_{nullptr};
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // A2DP_SNK_SERVICE_H
