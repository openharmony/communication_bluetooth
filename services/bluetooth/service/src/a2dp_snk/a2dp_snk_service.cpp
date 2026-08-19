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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_a2dp_snk"
#endif

#include "a2dp_snk_service.h"
#include "class_creator.h"
#include "log.h"
#include "btif/a2dp_audio_render_interface.h"
#include "a2dp_snk_audio_render.h"
#include "car_policy_adapter.h"
#include "cdc_stream_state_machine.h"
#include "thread_util.h"
#include "common_util.h"
#include "interface_profile_manager.h"
#include "adapter_manager.h"
#include "bluetooth_errorcode.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "bluetooth_connection_manager.h"
#include "permission_manager.h"
#include "native_a2dp_adapter.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_define.h"
#include "bluetooth_def.h"
#include "bluetooth_common_event_helper.h"


#ifdef EDM_SERVICE_ENABLE
#include "common/bluetooth_edm_whitelist_manager.h"
#endif
namespace OHOS {
namespace bluetooth {
utility::Context *A2dpSnkService::GetContext()
{
    return this;
}

A2dpSnkService *A2dpSnkService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<A2dpSnkService *>(servManager->GetProfileService(PROFILE_NAME_A2DP_SINK));
}

A2dpSnkService::A2dpSnkService() : utility::Context(PROFILE_NAME_A2DP_SINK, "1.6.2")
{
}

A2dpSnkService::~A2dpSnkService()
{}

void A2dpSnkService::RegisterObserver(IA2dpSnkObserver *observer)
{
    if (!observer) {
        HILOGE("observer is nullptr!");
        return;
    }
    a2dpSnkFwObservers_.Register(*observer);
}

void A2dpSnkService::NotifyConnStateChanged(const RawAddress &device, int prevState, int state, int cause)
{
    HILOGI("Connection state changed: %{public}s, %{public}d -> %{public}d, cause=%{public}d",
        GET_ENCRYPT_ADDR(device), prevState, state, cause);
    a2dpSnkFwObservers_.ForEach([device, state, cause](IA2dpSnkObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
}

void A2dpSnkService::DeregisterObserver(IA2dpSnkObserver *observer)
{
    HILOGI("enter");
    a2dpSnkFwObservers_.Deregister(*observer);
}

static int CovertConnectStateFromBluedroid(btav_connection_state_t state)
{
    switch (state) {
        case BTAV_CONNECTION_STATE_DISCONNECTED:
            return A2DP_SNK_CONN_STACK_DISCONNECTED_EVT;

        case BTAV_CONNECTION_STATE_CONNECTING:
            return A2DP_SNK_CONN_STACK_CONNECTING_EVT;

        case BTAV_CONNECTION_STATE_CONNECTED:
            return A2DP_SNK_CONN_STACK_CONNECTED_EVT;

        case BTAV_CONNECTION_STATE_DISCONNECTING:
            return A2DP_SNK_CONN_STACK_DISCONNECTING_EVT;

        default:
            return A2DP_SNK_CONN_STACK_DISCONNECTED_EVT;
    }
}

// 辅助函数：将 RawAddress 转换为加密日志字符串。
static std::string GetEncryptedLogAddr(const RawAddress &addr)
{
    return GetEncryptAddr(addr.GetAddress());
}

// --- INativeA2dpSinkCallback 实现 ---

void A2dpSnkService::OnNativeConnectionStateChanged(const RawAddress &device, btav_connection_state_t state)
{
    HILOGI("[a2dpSnkStack] OnNativeConnectionStateChanged: address=[%{public}s], state:%{public}d",
        GetEncryptedLogAddr(device).c_str(), state);
    A2dpSnkMessage event(CovertConnectStateFromBluedroid(state));
    event.addr_ = device.GetAddress();
    PostEvent(event);
}

void A2dpSnkService::OnNativeAudioStateChanged(const RawAddress &device, btav_audio_state_t state)
{
    HILOGI("[a2dpSnkStack] OnNativeAudioStateChanged: address=[%{public}s], state:%{public}d",
        GetEncryptedLogAddr(device).c_str(), state);
    auto streamSm = GetStreamStateMachine();
    if (!streamSm) {
        return;
    }
    int eventType = 0;
    switch (state) {
        case BTAV_AUDIO_STATE_STARTED:
            eventType = A2DP_SNK_STREAM_AUDIO_STARTED_EVT;
            break;
        case BTAV_AUDIO_STATE_STOPPED:
        case BTAV_AUDIO_STATE_REMOTE_SUSPEND:
            eventType = A2DP_SNK_STREAM_AUDIO_STOPPED_EVT;
            break;
        default:
            HILOGW("[a2dpSnkStack] OnNativeAudioStateChanged: Unhandled audio state: %d for address [%s]",
                   state, GetEncryptedLogAddr(device).c_str());
            return;
    }

    A2dpSnkMessage event(eventType);
    event.addr_ = device.GetAddress();
    streamSm->PostEvent(event);
}

void A2dpSnkService::OnNativeAudioConfigChanged(const RawAddress &device, uint32_t sampleRate, uint8_t channelCount)
{
    HILOGI("[a2dpSnkStack] OnNativeAudioConfigChanged: address=[%{public}s], sampleRate:%{public}d, "
           "channelCount:%{public}d", GetEncryptedLogAddr(device).c_str(), sampleRate, channelCount);
    A2dpSnkMessage event(A2DP_SNK_CONN_STACK_AUDIO_CONFIG_EVT);
    event.addr_ = device.GetAddress();
    event.sampleRate_ = sampleRate;
    event.channelCount_ = channelCount;
    PostEvent(event);
}

void A2dpSnkService::Enable()
{
    HILOGI("enter");
    DoInA2dpSnkThread(std::bind(&A2dpSnkService::StartUp, this));
}

void A2dpSnkService::Disable()
{
    HILOGI("enter");
    DoInA2dpSnkThread(std::bind(&A2dpSnkService::ShutDown, this));
}

void A2dpSnkService::StartUp()
{
    if (isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_A2DP_SINK, true);
        HILOGW("A2dpSnkService has already been started before.");
        return;
    }
    bt_interface_t *bt_interface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (!bt_interface) {
        HILOGE("Failed to open the bt_interface");
#ifdef BT_USE_OPEN_STACK
        GetContext()->OnEnable(PROFILE_NAME_A2DP_SINK, true);
#endif
        return;
    }

    btAvSinkInterface_ =
        (btav_sink_interface_t *)(bt_interface->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_SINK_ID));
    if (!btAvSinkInterface_) {
#ifdef BT_USE_OPEN_STACK
        HILOGW("A2DP sink profile unavailable on open stack, skip stack init");
        GetContext()->OnEnable(PROFILE_NAME_A2DP_SINK, true);
#else
        HILOGE("Failed to get btAvSinkInterface_");
#endif
        return;
    }

    if (!nativeAdapter_) {
        nativeAdapter_ = std::make_shared<NativeA2dpAdapter>(btAvSinkInterface_);
    }
    nativeAdapter_->Init(this, maxConnectAudioDevices_);
    {
        std::lock_guard<BtRecursiveMutex> lk(streamSmMutex_);
        streamStateMachine_ = std::make_shared<CdcStreamStateMachine>();
        streamStateMachine_->Init();
    }

    auto audioRender = A2dpSnkAudioRender::GetInstance();
    IA2dpAudioRender::SetInstance(audioRender);

    GetContext()->OnEnable(PROFILE_NAME_A2DP_SINK, true);
    isStarted_ = true;
}

void A2dpSnkService::ShutDown()
{
    HILOGI("enter");
    if (!isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_A2DP_SINK, true);
        HILOGW("A2dpSnkService has already been shutdown before.");
        return;
    }

    isShuttingDown_ = true;
    bool isNeedDisconnected = false;
    std::list<std::string> disconnectAddress;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
            if ((it->second != nullptr) && (it->second->GetDeviceState() > A2DP_SNK_STATE_DISCONNECTED)) {
                disconnectAddress.push_back(it->first);
                isNeedDisconnected = true;
            }
        }
    }
    for (std::string address : disconnectAddress) {
        Disconnect(RawAddress(address));
        HILOGI("disconnect deviceAddr=%{public}s", address.c_str());
    }
    if (!isNeedDisconnected) {
        ShutDownDone(true);
    }
}

int A2dpSnkService::Connect(const RawAddress &device)
{
    if (!isStarted_) {
        HILOGW("Service not started, reject connect");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (isShuttingDown_) {
        HILOGW("Service is shutting down, reject connect");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    std::string address = device.GetAddress();

    // 1) EDM 白名单
    if (!CheckEdmWhitelist(address)) {
        return Bluetooth::BT_ERR_PROHIBITED_BY_EDM;
    }

    // 2) 连接策略 + 车载策略准入
    if (!CheckConnectionAdmission(device)) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    // 3) ACL 链路限制
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    if (!connectionManager->IsBrLinkAllowed(address)) {
        HILOGE("Failed connect acl link because of MAX links.");
        return Bluetooth::BT_ERR_MAX_CONNECTION;
    }

    // 4) 状态机查重
    if (!CheckDeviceNotConnected(address)) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    // 5) 投递连接事件
    A2dpSnkMessage event(A2DP_SNK_CONN_CONNECT_EVT);
    event.addr_ = address;
    PostEvent(event);
    return Bluetooth::BT_NO_ERROR;
}

bool A2dpSnkService::CheckEdmWhitelist(const std::string &address)
{
#ifdef EDM_SERVICE_ENABLE
    CHECK_AND_RETURN_LOG_RET(BluetoothEdmWhitelistManager::GetInstance().IsAllowedConnect(address),
        false, "Bluetooth edm whitelist is not allowed");
#endif
    return true;
}

bool A2dpSnkService::CheckConnectionAdmission(const RawAddress &device)
{
    // 1) 连接策略不能为 FORBIDDEN
    int strategy =
        ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device, PROPERTY_A2DP_SINK_CONNECTION_POLICY);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGW("Connection forbidden by strategy, addr=%{public}s", GET_ENCRYPT_ADDR(device));
        return false;
    }

    // 2) 车载策略准入检查（FA 状态）
    if (!CarPolicyAdapter::GetInstance().IsAllowConnA2dp()) {
        HILOGW("Car policy disallows A2DP, addr=%{public}s", GET_ENCRYPT_ADDR(device));
        return false;
    }

    // 3) 车载场景连接数限制（A2dpSink + HfpHf <= CAR_MAX_CONN）。
    //    允许抢占：若已达上限，先断开已有连接再连接新设备。
    std::string address = device.GetAddress();
    if (!CarPolicyAdapter::GetInstance().IsConnectionAllowedBySinkLimit(address)) {
        if (!CarPolicyAdapter::GetInstance().DisconnectOthersForSinkLimit(address)) {
            HILOGW("Sink connection limit reached, addr=%{public}s", GET_ENCRYPT_ADDR(device));
            return false;
        }
    }
    return true;
}

bool A2dpSnkService::CheckDeviceNotConnected(const std::string &address)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    if (it != stateMachines_.end() && it->second != nullptr) {
        int state = it->second->GetDeviceState();
        if (state == A2DP_SNK_STATE_CONNECTING || state == A2DP_SNK_STATE_CONNECTED) {
            HILOGE("Device have been connected");
            return false;
        }
    }
    return true;
}

int A2dpSnkService::Disconnect(const RawAddress &device)
{
    HILOGI("enter DeviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    if (!isStarted_) {
        HILOGW("Service not started, reject disconnect");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    std::string address = device.GetAddress();
    int state = A2DP_SNK_STATE_DISCONNECTED;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGE("can't find stateMachine");
            return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
        }

        state = it->second->GetDeviceState();
    }

    if ((state != A2DP_SNK_STATE_CONNECTING) && (state < A2DP_SNK_STATE_CONNECTED)) {
        HILOGE("This device isn't connected, state=%{public}d", state);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }

    A2dpSnkMessage event(A2DP_SNK_CONN_DISCONNECT_EVT);
    event.addr_ = address;
    PostEvent(event);
    return Bluetooth::BT_NO_ERROR;
}

void A2dpSnkService::ShutDownDone(bool isAllDisconnected)
{
    HILOGI("ShutDownDone %{public}d", isAllDisconnected);
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        if (!isAllDisconnected) {
            for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
                if ((it->second != nullptr) && (it->second->GetDeviceState() > A2DP_SNK_STATE_DISCONNECTED)) {
                    HILOGW("not all device disconnected.");
                    return;
                }
            }
        }
    }

    // 先清理流状态机 — 避免连接状态机析构时回调访问已 reset 的 streamStateMachine_
    {
        std::lock_guard<BtRecursiveMutex> lk(streamSmMutex_);
        if (streamStateMachine_) {
            streamStateMachine_->Cleanup();
            streamStateMachine_.reset();
        }
    }

    // 再清理连接状态机
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        stateMachines_.clear();
    }

    isStarted_ = false;
    isShuttingDown_ = false;
    GetContext()->OnDisable(PROFILE_NAME_A2DP_SINK, true);
    if (nativeAdapter_) {
        nativeAdapter_->Cleanup();
        nativeAdapter_.reset();
    }
    HILOGI("A2dpSnkService shutdowndone!");
}

void A2dpSnkService::PostEvent(const A2dpSnkMessage &event)
{
    DoInA2dpSnkThread(std::bind(&A2dpSnkService::ProcessEvent, this, event));
}

void A2dpSnkService::ProcessEvent(const A2dpSnkMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GetEncryptAddr(event.addr_).c_str(), event.what_);
    if (event.what_ >= A2DP_SNK_CONN_EVT_BEGIN && event.what_ < A2DP_SNK_CONN_EVT_END) {
        switch (event.what_) {
            case A2DP_SNK_CONN_CONNECT_EVT:
                ProcessConnectEvent(event);
                break;
            case A2DP_SNK_CONN_DISCONNECT_EVT:
                ProcessDisconnectEvent(event);
                break;
            default:
                // 栈回调事件（STACK_CONNECTING/CONNECTED/DISCONNECTING/DISCONNECTED，
                // AUDIO_CONFIG, CONNECT_TIMEOUT）转发给对应设备的状态机处理。
                ProcessStackEvent(event);
                break;
        }
    } else if (event.what_ >= A2DP_SNK_STREAM_EVT_BEGIN && event.what_ < A2DP_SNK_STREAM_EVT_END) {
        auto sm = GetStreamStateMachine();
        if (sm) {
            sm->PostEvent(event);
        }
    } else {
        HILOGW("Unknown event: %{public}d", event.what_);
    }
}

void A2dpSnkService::ProcessStackEvent(const A2dpSnkMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.addr_);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGW("ProcessStackEvent: no state machine for device, event=%{public}d", event.what_);
        return;
    }
    if (it->second->IsRemoving()) {
        if (event.retryCount_ < MAX_EVENT_RETRY) {
            A2dpSnkMessage retry = event;
            retry.retryCount_++;
            PostEvent(retry);
        } else {
            HILOGW("ProcessStackEvent: max retry reached, drop event=%{public}d for %{public}s",
                event.what_, GET_ENCRYPT_ADDR(RawAddress(event.addr_)));
        }
        return;
    }
    it->second->ProcessMessage(event);
}

void A2dpSnkService::ProcessConnectEvent(const A2dpSnkMessage &event)
{
    HILOGI("enter");
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.addr_);
    if (it != stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
        if (event.retryCount_ < MAX_EVENT_RETRY) {
            A2dpSnkMessage retry = event;
            retry.retryCount_++;
            PostEvent(retry);
        } else {
            HILOGW("ProcessConnectEvent: max retry reached, drop event=%{public}d for %{public}s",
                event.what_, GET_ENCRYPT_ADDR(RawAddress(event.addr_)));
        }
        return;
    }
    if (it == stateMachines_.end() || it->second == nullptr) {
        stateMachines_[event.addr_] = std::make_unique<A2dpSnkStateMachine>(event.addr_, this);
        stateMachines_[event.addr_]->Init();
    }
    stateMachines_[event.addr_]->ProcessMessage(event);
}

void A2dpSnkService::ProcessDisconnectEvent(const A2dpSnkMessage &event)
{
    HILOGI("enter");
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.addr_);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGW("stateMachine is null");
        return;
    }
    it->second->ProcessMessage(event);
}

bool A2dpSnkService::AcceptIncomingConnection(const RawAddress &device)
{
    // 1) 设备必须已配对
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (!classicAdapter || classicAdapter->GetPairState(device) != PAIR_PAIRED) {
        HILOGW("Reject incoming: device not paired, addr=%{public}s", GET_ENCRYPT_ADDR(device));
        return false;
    }

    // 2) 连接策略 + 车载策略准入检查
    if (!CheckConnectionAdmission(device)) {
        return false;
    }

    return true;
}

bool A2dpSnkService::NativeConnect(const RawAddress &device)
{
    if (nativeAdapter_) {
        HILOGI("connect addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        bt_status_t status = nativeAdapter_->Connect(device);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("Failed connect! status=%{public}d", status);
            return false;
        }
        return true;
    }
    return false;
}

int A2dpSnkService::GetAudioConfig(const RawAddress &device, A2dpSnkCodecInfo &a2dpSnkCodecInfo)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(device.GetAddress());
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGW("stateMachine is null");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    a2dpSnkCodecInfo = it->second->GetAudioConfig();
    return Bluetooth::BT_NO_ERROR;
}

void A2dpSnkService::SetAudioFocusState(int focusState)
{
    if (nativeAdapter_) {
        HILOGI("focusState: %{public}d", focusState);
        nativeAdapter_->SetAudioFocusState(focusState);
    }

}

bool A2dpSnkService::NativeDisconnect(const RawAddress &device)
{
    if (nativeAdapter_) {
        HILOGI("disconnect addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        bt_status_t status = nativeAdapter_->Disconnect(device);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("Failed disconnected! status=%{public}d", status);
            return false;
        }
        return true;
    }
    return false;
}

std::shared_ptr<IA2dpSnkStreamStateMachine> A2dpSnkService::GetStreamStateMachine() const
{
    std::lock_guard<BtRecursiveMutex> lk(streamSmMutex_);
    return streamStateMachine_;
}

void A2dpSnkService::NotifyUserPlayIntent()
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyUserPlayIntent();
    }
}

void A2dpSnkService::NotifyUserPauseIntent()
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyUserPauseIntent();
    }
}

void A2dpSnkService::NotifyPlaybackDisallowed()
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyPlaybackDisallowed();
    }
}

void A2dpSnkService::NotifyA2dpConnected(bool allowed)
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyA2dpConnected(allowed);
    }
}

bool A2dpSnkService::CanPlay() const
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        return sm->CanPlay();
    }
    return false;
}

void A2dpSnkService::NotifyDeviceDisconnected()
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyDeviceDisconnected();
    }
}

void A2dpSnkService::NotifyDelayedPause()
{
    auto sm = GetStreamStateMachine();
    if (sm) {
        sm->NotifyDelayedPause();
    }
}

int A2dpSnkService::SetActiveSrcDevice(const RawAddress &device)
{
    HILOGI("enter");
    if (device.GetAddress().empty()) {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        activeSrcDevice_ = device;
        HILOGI("device is empty");
        return Bluetooth::BT_NO_ERROR;
    }
    if (!isStarted_ || isShuttingDown_) {
        HILOGW("Service not started or shutting down, reject setActiveSrcDevice");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (nativeAdapter_) {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        if (nativeAdapter_->SetActiveDevice(device) == BT_STATUS_SUCCESS) {
            activeSrcDevice_ = device;
            return Bluetooth::BT_NO_ERROR;
        }
    }
    return Bluetooth::BT_ERR_INTERNAL_ERROR;
}

void A2dpSnkService::ProcessRemoveStateMachine(const std::string &device)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(device);
    if (it == stateMachines_.end()) {
        return;
    }
    stateMachines_.erase(it);
    HILOGI("Remove state machine:%{public}s", device.c_str());
    if (isShuttingDown_) {
        ShutDownDone(false);
    }
}

std::vector<RawAddress> A2dpSnkService::GetDevicesByStates(const std::vector<int32_t> &states)
{
    std::vector<RawAddress> result;

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (!classicAdapter) {
        HILOGE("classicAdapter is null");
        return result;
    }
    // 将 states 转为 set，避免 O(n*m) 嵌套查找
    std::unordered_set<int32_t> stateSet(states.begin(), states.end());

    std::vector<RawAddress> pairedDevices = classicAdapter->GetPairedDevices();
    for (auto &device : pairedDevices) {
        int deviceState = GetDeviceState(device);
        if (stateSet.count(deviceState) > 0) {
            result.push_back(device);
        }
    }
    return result;
}

int A2dpSnkService::GetDeviceState(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    int snkState = (it == stateMachines_.end() || it->second == nullptr)
        ? A2DP_SNK_STATE_DISCONNECTED
        : it->second->GetDeviceState();

    // 将 A2DP_SNK_STATE_* 内部枚举映射为 BTConnectState 对外枚举。
    // switch 优于 map 查找：零运行时开销，编译器可警告遗漏分支。
    switch (snkState) {
        case A2DP_SNK_STATE_DISCONNECTED:
            return static_cast<int>(BTConnectState::DISCONNECTED);
        case A2DP_SNK_STATE_CONNECTING:
            return static_cast<int>(BTConnectState::CONNECTING);
        case A2DP_SNK_STATE_CONNECTED:
            return static_cast<int>(BTConnectState::CONNECTED);
        case A2DP_SNK_STATE_DISCONNECTING:
            return static_cast<int>(BTConnectState::DISCONNECTING);
        default:
            HILOGW("Unknown A2DP_SNK_STATE: %{public}d, fallback to DISCONNECTED", snkState);
            return static_cast<int>(BTConnectState::DISCONNECTED);
    }
}

int A2dpSnkService::GetPlayingState(const RawAddress &device, int &state)
{
    std::string address = device.GetAddress();
    auto sm = GetStreamStateMachine();
    std::lock_guard<BtRecursiveMutex> lk(mutex_);

    // 1) 设备必须存在于 stateMachines_ 中且处于 CONNECTED 状态。
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGE("Device not connected: %{public}s", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_INVALID_STATE;
    }

    // 2) 流状态机必须存在。
    if (sm == nullptr) {
        HILOGE("Stream state machine is null");
        return Bluetooth::BT_ERR_INVALID_STATE;
    }

    // 3) 流状态机是全局的，仅反映活跃设备的状态。
    //    非活跃的已连接设备不会处于播放状态。
    if (address != activeSrcDevice_.GetAddress()) {
        state = A2DP_NOT_PLAYING;
        return RET_NO_ERROR;
    }

    // 4) 活跃设备：将 IsPlaying() 映射到 A2DP_PLAYING_STATE 枚举。
    state = sm->IsPlaying() ? A2DP_IS_PLAYING : A2DP_NOT_PLAYING;
    return RET_NO_ERROR;
}

int A2dpSnkService::SetConnectStrategy(const RawAddress &device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(
        device, PROPERTY_A2DP_SINK_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        int deviceState = GetDeviceState(device);
        if (deviceState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        int deviceState = GetDeviceState(device);
        if (deviceState != static_cast<int>(BTConnectState::DISCONNECTED) &&
            deviceState != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
        }
    } else {
        HILOGE("Strategy set failed");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    return Bluetooth::BT_NO_ERROR;
}

int A2dpSnkService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device, PROPERTY_A2DP_SINK_CONNECTION_POLICY);
}

int A2dpSnkService::SendDelay(const RawAddress &device, int32_t delayValue)
{
    // 底层 btav_sink_interface_t 无 delay 接口，与 a2dp_src 侧保持一致返回成功。
    // 真正实现需扩展 INativeA2dpAdapter 和 Bluedroid 栈接口。
    return RET_NO_ERROR;
}

std::list<RawAddress> A2dpSnkService::GetConnectDevices()
{
    std::list<RawAddress> devList;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (const auto &entry : stateMachines_) {
        if (entry.second != nullptr && entry.second->GetDeviceState() >= A2DP_SNK_STATE_CONNECTED) {
            devList.push_back(RawAddress(entry.first));
        }
    }
    return devList;
}

int A2dpSnkService::GetConnectState()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    uint32_t result = 0;
    for (const auto &entry : stateMachines_) {
        if (entry.second == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
            continue;
        }
        int state = entry.second->GetDeviceState();
        if (state >= A2DP_SNK_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (state == A2DP_SNK_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (state == A2DP_SNK_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    }
    return static_cast<int>(result);
}

int A2dpSnkService::GetMaxConnectNum()
{
    return maxConnectAudioDevices_;
}

void A2dpSnkService::SetNativeAdapter(std::shared_ptr<INativeA2dpAdapter> adapter)
{
    nativeAdapter_ = std::move(adapter);
}

std::shared_ptr<INativeA2dpAdapter> A2dpSnkService::GetNativeAdapter() const
{
    return nativeAdapter_;
}

void A2dpSnkService::SetStreamStateMachine(std::shared_ptr<IA2dpSnkStreamStateMachine> sm)
{
    std::lock_guard<BtRecursiveMutex> lk(streamSmMutex_);
    streamStateMachine_ = std::move(sm);
}
REGISTER_CLASS_CREATOR(A2dpSnkService);
}  // namespace bluetooth
}  // namespace OHOS
