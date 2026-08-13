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

#ifndef A2DP_SNK_STATE_MACHINE_H
#define A2DP_SNK_STATE_MACHINE_H

#include <string>
#include <list>

#include "log.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "a2dp_snk_message.h"
#include "a2dp_snk_def.h"
#include "i_native_a2dp_adapter.h"
#include "interface_profile_a2dp_snk.h"

namespace OHOS {
namespace bluetooth {

class A2dpSnkService;

class A2dpSnkStateMachine : public utility::StateMachine {
public:
    explicit A2dpSnkStateMachine(const std::string &address, A2dpSnkService* service = nullptr);
    ~A2dpSnkStateMachine() = default;
    void Init();
    void NotifyStateChanged();
    int GetDeviceState() const;
    std::string GetDeviceName() const
    {
        return address_;
    }
    static std::string GetEventName(int what);

    bool ProcessConnectEvent();

    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void ConnectionTimeout() const;

    void AddDeferredMessage(const A2dpSnkMessage &msg);
    void ProcessDeferredMessage();
    // 检查 deferred 队列中是否含有指定事件
    bool HasDeferredMessage(int what) const;
    // 从 deferred 队列中移除所有指定事件
    void RemoveDeferredMessages(int what);

    // 收到用户 CONNECT/DISCONNECT 命令时，移除对立事件后入队（去重）。
    // 保证 deferred 队列中 CONNECT 和 DISCONNECT 互斥，同类型只保留一个。
    void DeferUserCommand(const A2dpSnkMessage &msg);

    bool IsRemoving() const;
    void SetRemoving(bool isRemoving);

    A2dpSnkCodecInfo GetAudioConfig() const { return decodeInfo_; }
    void SetAudioConfig(A2dpSnkCodecInfo info) { decodeInfo_ = info; }

    uint32_t GetSampleRate() const { return decodeInfo_.sampleRate; }
    uint8_t GetChannelCount() const { return decodeInfo_.channelMode; }

    A2dpSnkService* GetService() const { return service_; }

    // 检查 incoming 连接是否允许：service 判空 + 准入校验（配对/策略/FA 状态/连接数）。
    // 不允许时自动调 NativeDisconnect 通知底层断开，返回 false；允许则返回 true。
    bool AllowIncomingConnection(const std::string &address);

    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string CONNECTED = "Connected";
    inline static const std::string DISCONNECTING = "Disconnecting";

private:
    std::string address_;
    bool isRemoving_{false};
    A2dpSnkService* service_{nullptr};
    std::unique_ptr<utility::Timer> connTimer_{nullptr};
    inline static const int CONNECTION_TIMEOUT_MS{20000};  // 20
    std::list<A2dpSnkMessage> deferMsgs_{};
    A2dpSnkCodecInfo decodeInfo_{};
    int previousState_{A2DP_SNK_STATE_DISCONNECTED};
    BT_DISALLOW_COPY_AND_ASSIGN(A2dpSnkStateMachine);
};

class A2dpSnkState : public utility::StateMachine::State {
public:
    A2dpSnkState(const std::string &name, utility::StateMachine &stateMachine, int stateInt)
        : State(name, stateMachine), stateInt_(stateInt),
          stateMachine_(static_cast<A2dpSnkStateMachine &>(stateMachine))
    {}

    A2dpSnkState(const std::string &name, utility::StateMachine &stateMachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, stateMachine, parent), stateInt_(stateInt),
          stateMachine_(static_cast<A2dpSnkStateMachine &>(stateMachine))
    {}
    ~A2dpSnkState() override
    {}

    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_{A2DP_SNK_STATE_DISCONNECTED};
    A2dpSnkStateMachine &stateMachine_;
};

class A2dpSnkDisconnectedState : public A2dpSnkState {
public:
    A2dpSnkDisconnectedState(const std::string &name, utility::StateMachine &stateMachine)
        : A2dpSnkState(name, stateMachine, A2DP_SNK_STATE_DISCONNECTED){};
    ~A2dpSnkDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_{false};
};

class A2dpSnkConnectingState : public A2dpSnkState {
public:
    A2dpSnkConnectingState(const std::string &name, utility::StateMachine &stateMachine)
        : A2dpSnkState(name, stateMachine, A2DP_SNK_STATE_CONNECTING)
    {}
    ~A2dpSnkConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class A2dpSnkConnectedState : public A2dpSnkState {
public:
    A2dpSnkConnectedState(const std::string &name, utility::StateMachine &stateMachine)
        : A2dpSnkState(name, stateMachine, A2DP_SNK_STATE_CONNECTED)
    {}
    ~A2dpSnkConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class A2dpSnkDisconnectingState : public A2dpSnkState {
public:
    A2dpSnkDisconnectingState(const std::string &name, utility::StateMachine &stateMachine)
        : A2dpSnkState(name, stateMachine, A2DP_SNK_STATE_DISCONNECTING)
    {}
    ~A2dpSnkDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SNK_STATE_MACHINE_H