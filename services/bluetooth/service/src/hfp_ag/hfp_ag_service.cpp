/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hfp_ag"
#endif

#include <condition_variable>
#include "hfp_ag_service.h"

#include "log.h"
#include "a2dp_service.h"
#include "adapter_config.h"
#include "adapter_device_config.h"
#include "adapter_manager.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_errorcode.h"
#include "connect_strategy_manager.h"
#include "class_creator.h"
#include "ipc_skeleton.h"
#include "common_util.h"
#include "profile_service_manager.h"
#include "stub/telephone_service.h"
#include "permission_manager.h"
#include "profile_config.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_dft_exception.h"
#include "thread_util.h"
#include "bluetooth_device_battery_manager.h"
#include "preferences_manager.h"
#include "adapter_manager.h"
#include "bluetooth_hw_interface.h"
#include "hw_interop.h"
#include "a2dp_def.h"
#include "a2dp_service.h"
#include "a2dp_hfp_recover.h"
#ifdef BLUETOOTH_HFP_AG_ENABLE
#include "call_manager_client.h"
#endif
#include "interface_profile_a2dp_src.h"
#include "hitrace_meter.h"
#include "system_ability_manager_utils.h"
#include "control_intercept_plugin.h"
#include "a2dp_service.h"
#include "bt_func_hook.h"
#include "cloud_device_manager.h"
#include "bluetooth_common_event_helper.h"

namespace {
    // Wake up voice recognition param
    const std::string VOICE_RECOGNITION_BUNDLE_NAME = "com.huawei.voicerecognition";
    const std::string VOICE_RECOGNITION_ABILITY_NAME = "WakeUpExtAbility";
    const std::string VOICE_RECOGNITION_LANUCH_TYPE = "launch_type";
    const std::string VOICE_RECOGNITION_BLUETOOTH = "bluetooth_key";

    const int WAIT_FOR_STOP_VIRTUAL_CALL_TIMEOUT = 2 * 1000;
    const int WAIT_FOR_SCO_DISCONNECTED_TIMEOUT = 2 * 1000;
    const int32_t BLUETOOTH_UID = 1002;
}

namespace OHOS {
namespace bluetooth {
const uint64_t DELAY_HFP_REACH_MAX_CONNECT = 1000;
const uint64_t DELAY_TO_RETRY_CONNECT = 500;
const uint64_t DELAY_CONNECT_VOIP_SCO = 1000;
const uint64_t DELAY_CALL_STATE = 100;
const uint64_t DELAY_CONNECT_CELLULAR_SCO = 1000;
using namespace OHOS::Bluetooth;
HfpAgService::HfpAgService() : utility::Context(PROFILE_NAME_HFP_AG, "1.7.1")
{
    HILOGI("ProfileService:%{public}s Create", Name().c_str());
    hfpAgServiceCallback_ = std::make_unique<HfpAgServiceCallbacks>();
}

HfpAgService::~HfpAgService()
{
    HILOGI("ProfileService:%{public}s Release", Name().c_str());
}

utility::Context *HfpAgService::GetContext()
{
    return this;
}

HfpAgService *HfpAgService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<HfpAgService *>(servManager->GetProfileService(PROFILE_NAME_HFP_AG));
}

void HfpAgService::Enable()
{
    HfpAgMessage event(HFP_AG_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void HfpAgService::Disable()
{
    HfpAgMessage event(HFP_AG_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

int HfpAgService::GetMaxConnectionDevicesNum() const
{
    int number = MAX_DEFAULT_CONNECTIONS_NUM;
    return number;
}

bool HfpAgService::IsInbandRingingEnabled()
{
    RawAddress rawAddr(activeAddr_);
    bool isDisableInbandRing = BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_DISABLE_INBAND_RING, rawAddr);
    if (isDisableInbandRing) {
        HILOGI("inbandring is disable for this device");
        return false;
    } else {
        return localFeatures & HFP_AG_FEATURES_IN_BAND_RING;
    }
}

void HfpAgService::CallDetailsChanged(int callId, int callState)
{
    HILOGI("callId:%{public}d, callState:%{public}d", callId, callState);
    if (callState == HFP_AG_CALL_STATE_DISCONNECTED) {
        HfpAgSystemInterface::GetInstance().RemoveClccIndex(callId);
    }
}

int HfpAgService::IsVgsSupported(const RawAddress &device, bool &isSupported)
{
    bool result = GetHfpAgVgsSupport(device.GetAddress(), isSupported);
    if (!result) {
        isSupported = false;
    }
    HILOGI("device:%{public}s, result:%{public}d, isSupported:%{public}d", GET_ENCRYPT_ADDR(device),
        result, isSupported);
    return BT_NO_ERROR;
}

void HfpAgService::EnableBtCallLog(bool state)
{
    HILOGI("The switch status is %{public}d", state);
    if (state) {
        PreferencesManager::Save("CALL_LOG", ENABLE_CALL_LOG, PreferencesManagerType::CALL_LOG);
    } else {
        PreferencesManager::Save("CALL_LOG", DISABLE_CALL_LOG, PreferencesManagerType::CALL_LOG);
    }

    if (AdapterManager::GetInstance()->IsBetaVersion()) {
        AdapterManager::GetInstance()->EnableHisiPcm(state);
    }
}

void HfpAgService::StartUp()
{
    if (isStarted_ == true) {
        GetContext()->OnEnable(PROFILE_NAME_HFP_AG, true);
        BtChrBtExcpEvent("", BTOPT_HFP, CHR_SUB_ERRCODE_CASE2);
        HILOGW("HfpAgService has already been started before.");
        return;
    }
    bt_interface_t* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        BtChrBtExcpEvent("", BTOPT_HFP, CHR_SUB_ERRCODE_CASE3);
        HILOGE("Start up failed, bluetoothInterface is null.");
        return;
    }
#ifndef BT_MCU_PROXY_ENABLE
    bluetoothHfpInterface = reinterpret_cast<::bluetooth::headset::Interface*>(
        const_cast<void *>(bluetoothInterface->get_profile_interface(BT_PROFILE_HANDSFREE_ID)));
    if (bluetoothHfpInterface == nullptr) {
        BtChrBtExcpEvent("", BTOPT_HFP, CHR_SUB_ERRCODE_CASE4);
        HILOGE("Start up failed, bluetoothHfpInterface is null.");
        return;
    }
#endif
    maxConnectedNum_ = GetMaxConnectionDevicesNum();
    bool isEnabled = IsInbandRingingEnabled();
#ifndef BT_MCU_PROXY_ENABLE
    bt_status_t status = bluetoothHfpInterface->Init((::bluetooth::headset::Callbacks *)hfpAgServiceCallback_.get(),
        maxConnectedNum_ + 1, isEnabled);
    if (status != BT_STATUS_SUCCESS) {
        BtChrBtExcpEvent("", BTOPT_HFP, CHR_SUB_ERRCODE_CASE5);
        HILOGE("Failed to initialize Bluetooth HFP AG, status: %{public}d", status);
        return;
    }
#endif
    HfpAgSystemEventProcesser::Start();
    GetContext()->OnEnable(PROFILE_NAME_HFP_AG, true);
    isStarted_ = true;
    HILOGI("Hfp ag startUp register audio event");
    BluetoothAudioFrameworkAdapter::GetInstance().UpdateLocalVoiceCombineFeatureState();
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioNnStateEventListener();
    BtChrBtExcpEvent("", BTOPT_HFP, CHR_SUB_ERRCODE_CASE6);
}

void HfpAgService::ShutDown()
{
    if (isStarted_ == false) {
        GetContext()->OnDisable(PROFILE_NAME_HFP_AG, true);
        HILOGW("HfpAgService has already been shutdown before.");
        return;
    }

    isShuttingDown_ = true;
    bool isDisconnected = false;
    std::list<std::string> disconnectAddress;
    disconnectAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return state > HFP_AG_STATE_DISCONNECTED;
    });
    for (std::string address : disconnectAddress) {
        Disconnect(RawAddress(address));
        isDisconnected = true;
    }
    if (!isDisconnected) {
        ShutDownDone(true);
    }
}

void HfpAgService::ShutDownDone(bool isAllDisconnected)
{
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        if (!isAllDisconnected) {
            for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
                if ((it->second != nullptr) && (it->second->GetStateInt() > HFP_AG_STATE_DISCONNECTED)) {
                    return;
                }
            }
        }
        stateMachines_.clear();
    }

    maxConnectedNum_ = MAX_DEFAULT_CONNECTIONS_NUM;
    SetActiveDevice(RawAddress(NULL_ADDRESS));

    isStarted_ = false;
    isShuttingDown_ = false;
    isInbandRinging_ = false;
    isVrOpened_ = false;
    isVirtualCallStarted_ = false;

    StopDialingTimer();

    if (voiceRecognitionTimeout_ != nullptr) {
        voiceRecognitionTimeout_->Stop();
        voiceRecognitionTimeout_ = nullptr;
    }
    if (bccBlockTimer_ != nullptr) {
        bccBlockTimer_->Stop();
        bccBlockTimer_ = nullptr;
        isBccBlockTimerActive_.store(false);
    }
    if (bluetoothHfpInterface != nullptr) {
        bluetoothHfpInterface->Cleanup();
        bluetoothHfpInterface = nullptr;
    }
    HfpAgSystemEventProcesser::Stop();
    GetContext()->OnDisable(PROFILE_NAME_HFP_AG, true);
}

void HfpAgService::StopDialingTimer()
{
    if (dialingOutTimeout_ != nullptr) {
        dialingOutTimeout_->Stop();
        dialingOutTimeout_ = nullptr;
    }
}

int HfpAgService::GetConnectedDeviceNum()
{
    std::list<std::string> connectAddress;
    connectAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return ((state == HFP_AG_STATE_CONNECTING) || (state >= HFP_AG_STATE_CONNECTED));
    });
    return connectAddress.size();
}

int HfpAgService::GetProcessingDeviceNum()
{
    std::list<std::string> connectAddress;
    connectAddress = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return (state != HFP_AG_STATE_DISCONNECTED);
    });
    return connectAddress.size();
}

bool HfpAgService::IsRemoteHfpSupported(const RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    bool ret = (std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_HF)) != uuids.end()) ||
        (std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HSP_HS)) != uuids.end());
    return ret;
}

bool HfpAgService::GetRemoteDeviceConnectStatus(const RawAddress &device, int &state)
{
    bool result = BluetoothStateManager::GetInstance()->
        FindRemoteDeviceHfpAgStateByAddr(device.GetAddress(), state);
    return result;
}

void HfpAgService::UpdateVirtualDeviceByHfpConnectState(const RawAddress& device, int state)
{
    int hwRemoteDeviceType = RemoteDeviceProperties::GetInstance()->GetHwRemoteDeviceType(device);
    if (hwRemoteDeviceType != HW_CAR_DEVICE_TYPE) {
        return;
    }
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp source service nullptr");

    if (state == static_cast<int>(BTConnectState::DISCONNECTED) &&
        service->GetRemoteDeviceConnectStatus(device) == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("when hfp disconnect a2dp is disconnection, remove virtual device");
        service->UpdateVirtualDevice(VIRTUAL_DEVICE_REMOVE, device.GetAddress());
    } else if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("when hfp connect, add virtual device");
        service->UpdateVirtualDevice(VIRTUAL_DEVICE_ADD, device.GetAddress());
    }
}

void HfpAgService::NotifyConnStateChangedInner(const RawAddress &device, int state, int cause)
{
    bool isNeedRetryConnect = true;
    std::string address = device.GetAddress();
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if ((it == stateMachines_.end()) || (it->second == nullptr) || (!it->second->GetTryReconnectHfp())) {
            isNeedRetryConnect = false;
        }
    }
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(bluetoothDevice != nullptr, "bluetoothDevice is nullptr");
    A2dpHfpRecover::GetInstance().SetTimerForRecoverHfpService(address);
    isNeedRetryConnect = isNeedRetryConnect && GetDevicePreState(device) ==
        static_cast<int>(BTConnectState::CONNECTING) && state == static_cast<int>(BTConnectState::DISCONNECTED) &&
        bluetoothDevice->IsAclConnected();
    if (isNeedRetryConnect) {
        bluetoothDevice->DialogControlAddConnectingProfileCount();
        HfpAgMessage event(HFP_AG_RETRY_CONNECT_EVT);
        event.dev_ = address;
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_HFP, [this, event]() { this->ProcessEvent(event); }, DELAY_TO_RETRY_CONNECT, "HfpAgProcessEvent");
    }
    observers_.ForEach([device, state, cause](HfpAgServiceObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
    BluetoothHelper::BluetoothCommonEventHelper::PublishHfpConnectStateUpdateEvent(device.GetAddress(), state);
    UpdateVirtualDeviceByHfpConnectState(device, state);
}

int HfpAgService::Connect(const RawAddress &device)
{
    HILOGI("deviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    ControlInterceptMessage msg {
        .addr = device.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedHfpConn(msg),
        BT_ERR_INTERNAL_ERROR, "Restricted by control intercept");
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();

    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE0, callingName);
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    if (!connectionManager->IsBrLinkAllowed(address)) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE5, callingName);
        NotifyConnStateChangedInner(device, GetDeviceState(device),
            static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
        return Bluetooth::BT_ERR_MAX_CONNECTION;
    }

    int state = HFP_AG_STATE_DISCONNECTED;
    BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state);

    if ((state >= HFP_AG_STATE_CONNECTED) || (state == HFP_AG_STATE_CONNECTING)) {
        HILOGE("This device has connected, currentConnectedState=%{public}d", state);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE2, callingName);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    bool isReachMaxConnect = GetProcessingDeviceNum() >= maxConnectedNum_;
    if (isReachMaxConnect && !connectionManager->DisconnectForDeviceLimit()) {
        HILOGE("Max connection has reached!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE3, callingName);
        NotifyConnStateChangedInner(device, GetDeviceState(device),
            static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
        return Bluetooth::BT_ERR_MAX_CONNECTION;
    }
    if (GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGE("Connection Strategy is FORBIDDEN!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE4, callingName);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (!IsRemoteHfpSupported(device)) {
        HILOGE("Cannot connect to %{public}s, no headset UUID ", GET_ENCRYPT_STR_ADDR(address));
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE6, callingName);
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    if (ProcCloudDeviceConnect(device)) {
        return BT_NO_ERROR;
    }
    postHfpConnectEvent(isReachMaxConnect, address);
    return HFP_AG_SUCCESS;
}

void HfpAgService::postHfpConnectEvent(bool isReachMaxConnect, const std::string address)
{
    HfpAgMessage event(HFP_AG_CONNECT_EVT);
    event.dev_ = address;
    if (isReachMaxConnect) {
        HILOGI("hfp connect reach max num, wait first device hfp disconncted");
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_HFP, [this, event]() { this->ProcessEvent(event); },
            DELAY_HFP_REACH_MAX_CONNECT, "HfpAgProcessEvent");
    } else {
        PostEvent(event);
    }
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            stateMachines_[address] = std::make_unique<HfpAgStateMachine>(address);
            stateMachines_[address]->Init();
            stateMachines_[address]->SetTryReconnectHfp(true);
        } else {
            it->second->SetTryReconnectHfp(true);
        }
    }
    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, event.dev_, "HFPINITIATOR", CHR_INITIATOR_LOCAL);
}

int HfpAgService::Disconnect(const RawAddress &device)
{
    HILOGI("DeviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    int slcState = HFP_AG_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, slcState)) {
        HILOGE("can't find stateMachine");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_DISCONN,
            device, UE_COMMON_SCENE_CASE2, callingName);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }

    if ((slcState != HFP_AG_STATE_CONNECTING) && (slcState < HFP_AG_STATE_CONNECTED)) {
        HILOGE("This device isn't connected, slcState=%{public}d", slcState);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_DISCONN, device, UE_COMMON_SCENE_CASE3, callingName);
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }
    RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(), PROFILE_ID_HFP_AG, true);

    HfpAgMessage event(HFP_AG_DISCONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_DISCONN, device, UE_COMMON_SCENE_CASE1, callingName);
    return HFP_AG_SUCCESS;
}

std::list<RawAddress> HfpAgService::GetConnectingOrConnectionDevices()
{
    std::list<std::string> addressList;
    addressList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return ((state >= HFP_AG_STATE_CONNECTED || state == HFP_AG_STATE_CONNECTING));
    });
    std::list<RawAddress> devList;
    for (std::string address : addressList) {
        devList.push_back(RawAddress(address));
    }
    return devList;
}

std::list<RawAddress> HfpAgService::GetConnectDevices()
{
    std::list<std::string> addressList;
    addressList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return state >= HFP_AG_STATE_CONNECTED;
    });
    std::list<RawAddress> devList;
    for (std::string address : addressList) {
        devList.push_back(RawAddress(address));
    }
    return devList;
}

int HfpAgService::GetConnectState()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    unsigned int result = 0;
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (it->second->GetStateInt() >= HFP_AG_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (it->second->GetStateInt() == HFP_AG_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (it->second->GetStateInt() == HFP_AG_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (it->second->GetStateInt() == HFP_AG_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    }
    return result;
}

int HfpAgService::GetDeviceState(const RawAddress &device)
{
    std::string address = device.GetAddress();
    int state = HFP_AG_STATE_DISCONNECTED;
    if (IsInCloudBondingState(device)) {
        HILOGI("[CLOUD_PAIR] %{public}s is in cloud connecting", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return static_cast<int>(BTConnectState::CONNECTING);
    }
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state)) {
        return stateMap_.at(HFP_AG_STATE_DISCONNECTED);
    }
    if (state >= HFP_AG_STATE_CONNECTED) {
        return stateMap_.at(HFP_AG_STATE_CONNECTED);
    } else {
        return stateMap_.at(state);
    }
}

int HfpAgService::GetDevicePreState(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    std::string address = device.GetAddress();
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        return stateMap_.at(HFP_AG_STATE_DISCONNECTED);
    }

    if (it->second->GetPreState() >= HFP_AG_STATE_CONNECTED) {
        return stateMap_.at(HFP_AG_STATE_CONNECTED);
    } else {
        return stateMap_.at(it->second->GetPreState());
    }
}

bool HfpAgService::IsConnected(const std::string &address)
{
    int state = HFP_AG_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state)) {
        HILOGE("Invalid Device address:%{public}s", GET_ENCRYPT_STR_ADDR(address));
        return false;
    }
    if (state < HFP_AG_STATE_CONNECTED) {
        HILOGE("Device not connected! address:%{public}s, state[%{public}d]",
            GET_ENCRYPT_STR_ADDR(address), state);
        return false;
    }
    return true;
}

bool HfpAgService::IsAudioIdle() const
{
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    if (isVrOpened_ || isVirtualCallStarted_ || !systemInterface.IsCallIdle()) {
        HILOGI("isAudioModeIdle: not idle");
        return false;
    }
    return true;
}

void HfpAgService::SetResumeDevice(std::string activeAddr)
{
    std::lock_guard<std::mutex> lock(resumeScoMutex_);
    resumeDevice_ = activeAddr;
}

std::string HfpAgService::GetResumeDevice()
{
    std::lock_guard<std::mutex> lock(resumeScoMutex_);
    return resumeDevice_;
}

bool HfpAgService::IsNoNeedConnectSco()
{
    if (!IsAllowConnectHfpAgScoHook()) {
        HILOGW("Connect sco is disallowed by hook");
        return true;
    }
    uint8_t a2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    uint8_t hfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    if (!BluetoothAudioManager::GetInstance().GetProfileStatus(activeAddr_, a2dpState, hfpState) ||
        a2dpState <= static_cast<uint8_t>(A2DP_STATUS_ENABLE)) {
        return false;
    }
    BtAudioScene scene = BluetoothAudioFrameworkAdapter::GetAudioScene();
    bool isRinging = !HfpAgSystemInterface::GetInstance().IsInCall() &&
        (HfpAgSystemInterface::GetInstance().IsRinging() || (scene == AUDIO_SCENE_RINGING) ||
        (scene == AUDIO_SCENE_VOICE_RINGING));
    bool isInBandEnabled = IsInbandRingingEnabled();
#ifdef BLUETOOTH_HFP_AG_ENABLE
    auto callManagerClient = DelayedSingleton<Telephony::CallManagerClient>::GetInstance();
    if (callManagerClient != nullptr) {
        callManagerClient->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
        int32_t callState = callManagerClient->GetCallState();
        HILOGI("callstate=%{public}d, isRinging=%{public}d", callState, isRinging);
        if (!isVirtualCallStarted_ && (callState == static_cast<int32_t>(Telephony::CallStateToApp::CALL_STATE_RINGING)
            || isRinging) && isInBandEnabled) {
            return true;
        }
    } else {
        HILOGE("CallManagerClient is null, cannot get call state");
        return false;
    }
#else
    if (!isVirtualCallStarted_ && isRinging && isInBandEnabled) {
        return true;
    }
#endif
    return false;
}

void HfpAgService::CheckAndResumeSco(int numActive, int numHeld, int callState)
{
    //  only first active call need to check
    if (numActive != 1 || numHeld != 0 || callState != HFP_AG_CALL_STATE_IDLE) {
        return;
    }

    std::string resumeDevice = GetResumeDevice();
    if (resumeDevice != NULL_ADDRESS && resumeDevice == activeAddr_) {
        HILOGI("need resume sco, notify SCO connect");
        RawAddress device(resumeDevice);
        NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, device);
    }
    SetResumeDevice(NULL_ADDRESS);
}

int HfpAgService::GetMaxConnectNum()
{
    return maxConnectedNum_;
}

void HfpAgService::PostEvent(const HfpAgMessage &event)
{
    DoInHfpThread([this, event]() { this->ProcessEvent(event); });
}

void HfpAgService::ProcessEvent(const HfpAgMessage &event)
{
    HILOGI("Address:%{public}s, Id:%{public}d, type:%{public}d", GET_ENCRYPT_STR_ADDR(event.dev_),
        event.what_, event.type_);
    switch (event.what_) {
        case HFP_AG_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case HFP_AG_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case HFP_AG_CONNECT_EVT:
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectEvent(event);
            break;
        case HFP_AG_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event.dev_);
            break;
        case HFP_AG_VOICE_RECOGNITION_TIME_OUT_EVT:
            ProcessVoiceRecognitionTimeOut(event);
            break;
        case HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT:
            ProcessResponseClccTimeOut(event);
            break;
        case HFP_AG_DIAL_TIME_OUT_EVT:
            ProcessDialOutTimeOut();
            break;
        case HFP_AG_SET_VOLUME_EVT:
        case HFP_AG_CALL_STATE_CHANGE:
        case HFP_AG_SEND_CCLC_RESPONSE:
        case HFP_AG_DIALING_OUT_RESULT:
        case HFP_AG_NOTIFY_DEVICE_STATE_CHANGED:
            SendEventToEachStateMachine(event);
            break;
        default:
            ProcessDefaultEvent(event);
            break;
    }
}

void HfpAgService::ProcessConnectEvent(const HfpAgMessage &event)
{
    if (GetProcessingDeviceNum() <= maxConnectedNum_) {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(event.dev_);
        if (it != stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
            // peer device may send connect request before we remove statemachine for last connection.
            // so post this connect request, process it after we remove statemachine completely.
            PostEvent(event);
        } else if (it == stateMachines_.end() || it->second == nullptr) {
            stateMachines_[event.dev_] = std::make_unique<HfpAgStateMachine>(event.dev_);
            stateMachines_[event.dev_]->Init();
            stateMachines_[event.dev_]->ProcessMessage(event);
        } else {
            it->second->ProcessMessage(event);
        }
    } else {
        HILOGE("Max connection has reached!");
        RawAddress device = RawAddress(event.dev_);
        Disconnect(device);
        std::string callingName = PermissionManager::GetCallingName();
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE3, callingName);
        NotifyConnStateChangedInner(device, HFP_AG_STATE_DISCONNECTED,
            static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
    }
}

void HfpAgService::ProcessDefaultEvent(const HfpAgMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if ((it != stateMachines_.end()) && (it->second != nullptr)) {
        it->second->ProcessMessage(event);
    } else {
        HILOGE("Invalid address[%{public}s]", GET_ENCRYPT_STR_ADDR(event.dev_));
    }
}

void HfpAgService::ProcessRemoveStateMachine(const std::string &address)
{
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        stateMachines_.insert_or_assign(address, nullptr);
    }
    BluetoothStateManager::GetInstance()->EraseRemoteDeviceHfpAgState(address);
    if (isShuttingDown_) {
        ShutDownDone(false);
    }
}

void HfpAgService::SendEventToEachStateMachine(const HfpAgMessage &event)
{
    HfpAgMessage curEvent = event;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second != nullptr) {
            curEvent.dev_ = it->first;
            it->second->ProcessMessage(curEvent);
        }
    }
}

bool HfpAgService::CloseVoiceRecognition(const RawAddress &device)
{
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, device,
        UE_LOCAL_STOP_VR_RECV_REQ, callingName);
    if (!isVrOpened_) {
        HILOGE("Current VR state is not opened, did not close it!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, device,
            UE_LOCAL_STOP_VR_FAIL_VR_NOT_OPENED, callingName);
        return false;
    }

    std::string address = device.GetAddress();
    if (IsConnected(address) == false) {
        HILOGW("Device is not connected!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, device,
            UE_LOCAL_STOP_VR_FAIL_AG_NOT_CONNECTED, callingName);
        return false;
    }

    if (IsActiveDevice(address) == false) {
        HILOGW("Device is active device!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, device,
            UE_LOCAL_STOP_VR_FAIL_NOT_ACTIVE_DEV, callingName);
        return false;
    }
    HILOGI("DeviceAddr=%{public}s", GET_ENCRYPT_STR_ADDR(address));
    bool ret = DisconnectSingleSco(address);
    if (!ret) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, device,
            UE_LOCAL_STOP_VR_FAIL_DISCONN_SCO, callingName);
    }
    return true;
}

void HfpAgService::SendCloseVoiceEvent(const std::string &address, bool disconnectSco)
{
    HILOGI("Send close voice evt");
    HfpAgMessage evt1(HFP_AG_CLOSE_VOICE_RECOGNITION_EVT);
    evt1.dev_ = address;
    PostEvent(evt1);
    if (disconnectSco) {
        HfpAgMessage evt2(HFP_AG_DISCONNECT_AUDIO_EVT);
        evt2.dev_ = address;
        PostEvent(evt2);
    }
    isVrOpened_ = false;
}

void HfpAgService::ProcessVoiceRecognitionTimeOut(const HfpAgMessage &event)
{
    if (voiceRecognitionTimeout_ != nullptr) {
        voiceRecognitionTimeout_ = nullptr;
    }

    SendEventToEachStateMachine(event);
}

void HfpAgService::ProcessResponseClccTimeOut(const HfpAgMessage &event)
{
    SendEventToEachStateMachine(event);
}

std::vector<RawAddress> HfpAgService::GetDevicesByStates(std::vector<int> states)
{
    std::vector<RawAddress> devices;
    std::vector<RawAddress> allDevices;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
            allDevices.push_back(RawAddress(it->first));
        }
    }

    for (RawAddress device : allDevices) {
        for (size_t i = 0; i < states.size(); i++) {
            if (GetDeviceState(device) == states[i]) {
                devices.push_back(device);
                break;
            }
        }
    }
    return devices;
}

bool HfpAgService::IsAudioConnected()
{
    std::list<std::string> addressList = {};
    addressList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return ((state > HFP_AG_AUDIO_STATE_DISCONNECTED));
    });
    if (!addressList.empty()) {
        return true;
    }
    return false;
}

bool HfpAgService::IsAudioConnected(const std::string &address)
{
    int state = HFP_AG_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state)) {
        HILOGE("Invalid Device address:%{public}s", GET_ENCRYPT_STR_ADDR(address));
        return false;
    }
    if (state < HFP_AG_AUDIO_STATE_CONNECTED) {
        HILOGE("Device audio is not connected! address:%{public}s, state[%{public}d]",
            GET_ENCRYPT_STR_ADDR(address), state);
        return false;
    }
    return true;
}

bool HfpAgService::IsScoAcceptableExceptADevice(const std::string &address)
{
    /* if audio device in idle state, ag should terminate sco open. */
    HILOGI("device:%{public}s IsAudioIdle:%{public}d", GET_ENCRYPT_STR_ADDR(address), IsAudioIdle());
    return !IsAudioIdle();
}

int32_t HfpAgService::ConnectSco(uint8_t callType)
{
    HILOG_COMM_INFO("ConnectSco: callType=%{public}d, active device=%{public}s", callType,
        GET_ENCRYPT_STR_ADDR(activeAddr_));
    StopA2dpOffload();
    int32_t result = BT_NO_ERROR;
    switch (callType) {
        case static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR):
            currentCallType_ = static_cast<int>(BTCallType::CALL_TYPE_CELLULAR);
            result = StartCellularCallSco();
            break;
        case static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL):
            currentCallType_ = static_cast<int>(BTCallType::CALL_TYPE_VIRTUAL);
            result = StartVirtualCallSco();
            break;
        default:
            result = BT_ERR_INTERNAL_ERROR;
            HILOGE("error call type");
            break;
    }
    return result;
}

int32_t HfpAgService::DisconnectSco(uint8_t callType)
{
    HILOG_COMM_INFO("DisconnectSco: callType=%{public}d, active device=%{public}s", callType,
        GET_ENCRYPT_STR_ADDR(activeAddr_));
    int32_t result = BT_NO_ERROR;
    switch (callType) {
        case static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR):
            result = StopCellularCallSco();
            break;
        case static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL):
            result = StopVirtualCallSco();
            break;
        default:
            result = BT_ERR_INTERNAL_ERROR;
            HILOGE("error call type");
            break;
    }
    return result;
}

int32_t HfpAgService::StartCellularCallSco()
{
    RawAddress rawAddr(activeAddr_);
    std::string callingName = PermissionManager::GetCallingName();

    if (isVirtualCallStarted_) {
        HILOGI("stop virtual voice call because there is an incoming cs call");
        StopVirtualCallSco();
    }

    int32_t result = IsAcceptCellularCall();
    if (result != BT_NO_ERROR) {
        HILOG_COMM_ERROR("Not accept Cellular Call.");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_SCO_NOT_ACCEPT, callingName);
        return result;
    }

    bool isDelayStartSco = BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_DELAY_CELLULAR_CREATE_SCO, rawAddr);
    WaitForScoDisconnected();
    UpdateLocalVoiceCombineStateForScoStateChanged(activeAddr_, true);

    HfpAgMessage event(HFP_AG_CONNECT_AUDIO_EVT);
    event.dev_ = activeAddr_;
    if (isDelayStartSco) {
        HILOGI("delay start Sco for this device");
        ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP, [this, event] { this->PostEvent(event); }, 
            DELAY_CONNECT_CELLULAR_SCO, "delay_start_ceelular_sco");
    } else {
        PostEvent(event);
    }

    HILOGI("start cellular call sco success");
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_SUCCESS,
        UE_COMMON_SCENE_CASE1, callingName);
    return BT_NO_ERROR;
}

int32_t HfpAgService::IsAcceptCellularCall()
{
    RawAddress rawAddr(activeAddr_);
    std::string callingName = PermissionManager::GetCallingName();

    if (activeAddr_ == NULL_ADDRESS) {
        HILOGW("The active device is null!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_NO_ACTIVE_HFP_DEVICE, callingName);
        return BT_ERR_NO_ACTIVE_HFP_DEVICE;
    }

    int state = HFP_AG_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(activeAddr_, state)) {
        HILOGE("Invalid Device");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_NULL_STATE_MACHINE, callingName);
        return BT_ERR_NULL_HFP_STATE_MACHINE;
    }

    if (state < HFP_AG_STATE_CONNECTED) {
        HILOGE("Device not connected! state[%{public}d]", state);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_HFP_NOT_CONN, callingName);
        return BT_ERR_HFP_NOT_CONNECT;
    }

    if (state == HFP_AG_AUDIO_STATE_CONNECTED) {
        HILOGE("Device audio has connected! state[%{public}d]", state);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_SUCCESS_SCO_NOT_DISCONN, callingName);
        return BT_ERR_SCO_HAS_BEEN_CONNECTED;
    }
    return BT_NO_ERROR;
}

int32_t HfpAgService::StopCellularCallSco()
{
    bool result = false;
    std::list<std::string> disconnectList;
    disconnectList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return state > HFP_AG_AUDIO_STATE_DISCONNECTED;
    });
    for (std::string address : disconnectList) {
        if (DisconnectCsCallSco(address)) {
            result = true;
            HILOGD("disconnect cs call sco from: %{public}s success", GET_ENCRYPT_STR_ADDR(address));
        } else {
            HILOGE("disconnect cs call sco from: %{public}s failed", GET_ENCRYPT_STR_ADDR((address)));
        }
    }

    return result ? BT_NO_ERROR : BT_ERR_DISCONNECT_SCO_FAILED;
}

bool HfpAgService::DisconnectCsCallSco(std::string address)
{
    RawAddress rawAddr(address);
    std::string callingName = PermissionManager::GetCallingName();

    HILOGI("address:%{public}s", GET_ENCRYPT_STR_ADDR(address));
    int state = HFP_AG_AUDIO_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state)) {
        HILOGE("can not find state machine.");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_DISCONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_NULL_STATE_MACHINE, callingName);
        return false;
    }

    if (state == HFP_AG_AUDIO_STATE_DISCONNECTED) {
        HILOGW("Sco is already disconnected! state[%{public}d]", state);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_DISCONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_SCO_DISCONN, callingName);
    } else {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CS_DISCONN_SCO, rawAddr, UE_SCENE_CODE_SUCCESS,
            UE_COMMON_SCENE_CASE1, callingName);
    }
 
    HfpAgMessage event(HFP_AG_DISCONNECT_AUDIO_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

int32_t HfpAgService::StartVirtualCallSco()
{
    RawAddress rawAddr(activeAddr_);
    int32_t res = IsAcceptVirtualCall();
    if (res != BT_NO_ERROR) {
        HILOGE("Not accept virtual Call.");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_VIRCALL_SCO_NOT_ACCEPT);
        return res;
    }

    {
        std::unique_lock<std::mutex> lock(syncMutex_);
        isVirtualCallStarted_ = true;
    }

    bool isDelayConnectVoipSco = BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_DELAY_CONNECT_VOIP_SCO, rawAddr);
    if (isDelayConnectVoipSco) {
        HILOGI("delay connect voip sco, address:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_HFP, [this] { this->ConnectVirtualCallSco(); },
            DELAY_CONNECT_VOIP_SCO, "HfpAgConnectVirtualCallSco");
    } else {
        ConnectVirtualCallSco();
    }
    return BT_NO_ERROR;
}

void HfpAgService::ConnectVirtualCallSco()
{
    RawAddress rawAddr(activeAddr_);
    {
        std::lock_guard<std::mutex> lock(callstatemachineMutex_);
        if (callStateMachine_ == nullptr) {
            callStateMachine_ = std::make_unique<CallStateMachine>(activeAddr_);
        } else {
            callStateMachine_->ReInit();
        }
    }
    BluetoothPhoneState phoneState;
    // Send virtual phone Dialing state changed
    phoneState.SetCallState(HFP_AG_CALL_STATE_DIALING);
    PhoneStateChanged(phoneState, true);
    // Send virtual phone Alerting state changed
    phoneState.SetCallState(HFP_AG_CALL_STATE_ALERTING);
    PhoneStateChanged(phoneState, true);
    // Send virtual phone Idle state changed
    phoneState.SetActiveNum(1);
    phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
    PhoneStateChanged(phoneState, true);
    UpdateLocalVoiceCombineStateForScoStateChanged(activeAddr_, true);

    HfpAgMessage event(HFP_AG_CONNECT_AUDIO_EVT);
    event.dev_ = activeAddr_;
    PostEvent(event);

    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_SUCCESS,
        UE_COMMON_SCENE_CASE1);
}

int32_t HfpAgService::IsAcceptVirtualCall()
{
    RawAddress rawAddr(activeAddr_);
    if (activeAddr_ == NULL_ADDRESS) {
        HILOGE("The active device is null!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_NO_ACTIVE_HFP_DEVICE);
        BtChrCallExcpEvent(activeAddr_, ERRCODE_START_VIRTUAL_VOICE_CALL, CHR_SUB_ERRCODE_CASE1, -1);
        return BT_ERR_NO_ACTIVE_HFP_DEVICE;
    }

    if (isVrOpened_) {
        bool res = CloseVoiceRecognition(rawAddr);
        HILOGW("voice recognition is still active, just called CloseVoiceRecognition,"
            "returned %{public}d, please try again", res);
        isVrOpened_ = false;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_VOICE_REC_STARTED);
        return BT_ERR_VR_HAS_BEEN_STARTED;
    }

    if (isVirtualCallStarted_) {
        HILOGW("audio mode not idle, activeAddr_:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_AUDIO_MODE_NOT_IDLE);
        return BT_ERR_AUDIO_NOT_IDLE;
    }

    // Audio should not be connected when no audio mode is active
    if (IsAudioConnected()) {
        HILOGW("audio is still active, not allow connect sco.");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_AUDIO_ON);
        return BT_ERR_SCO_HAS_BEEN_CONNECTED;
    }

    if (!IsConnected(activeAddr_)) {
        HILOGW("Device is not connected!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_CONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_HFP_NOT_CONN);
        return BT_ERR_HFP_NOT_CONNECT;
    }
    return BT_NO_ERROR;
}

int32_t HfpAgService::StopVirtualCallSco()
{
    RawAddress rawAddr(activeAddr_);
    std::string callingName = PermissionManager::GetCallingName();
    if (!isVirtualCallStarted_) {
        HILOGW("virtual call not started");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_DISCONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_VIR_CALL_NOT_STARTED, callingName);
        return BT_ERR_VIRTUAL_CALL_NOT_STARTED;
    }

    std::unique_lock<std::mutex> lock(syncMutex_);
    isVirtualCallStarted_ = false;

    std::list<std::string> disconnectList;
    disconnectList = BluetoothStateManager::GetInstance()->FindRemoteDeviceByHfpConnState<std::string,
        int>([](const std::string address, int &state) -> bool {
            return state > HFP_AG_AUDIO_STATE_DISCONNECTED;
    });
    for (const std::string& address : disconnectList) {
        if (!DisconnectSingleSco(address)) {
            HILOG_COMM_ERROR("disconnect sco failed");
            condition_.notify_all();
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_DISCONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
                UE_FAIL_DISCONN_SINGLE_SCO, callingName);
        }
    }

    BluetoothPhoneState phoneState;
    phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
    PhoneStateChanged(phoneState, true);

    condition_.notify_all();

    HILOGI("stop virtual call sco success");
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_DISCONN_SCO, rawAddr, UE_SCENE_CODE_SUCCESS,
        UE_COMMON_SCENE_CASE1, callingName);
    return BT_NO_ERROR;
}

bool HfpAgService::ConnectSco()
{
    std::string activeDeviceAddr = GetActiveDevice();
    if (activeDeviceAddr == NULL_ADDRESS) {
        HILOGW("The active device is null!");
        return false;
    }

    if (IsConnected(activeDeviceAddr) == false) {
        HILOGW("Device is not connected!");
        return false;
    }

    if (IsAudioConnected()) {
        HILOGW("Sco is not idle!");
        return false;
    }
    HILOGI("DeviceAddr=%{public}s", GET_ENCRYPT_STR_ADDR(activeDeviceAddr));
    UpdateLocalVoiceCombineStateForScoStateChanged(activeDeviceAddr, false);
    HfpAgMessage event(HFP_AG_CONNECT_AUDIO_EVT);
    event.dev_ = activeDeviceAddr;
    PostEvent(event);
    return true;
}

bool HfpAgService::DisconnectSco()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    bool ret = false;
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if ((it->second != nullptr) && (it->second->GetStateInt() > HFP_AG_AUDIO_STATE_DISCONNECTED) &&
            (DisconnectSingleSco(it->first))) {
            ret = true;
        }
    }
    HILOGI("Result=%{public}d", ret);
    return ret;
}

bool HfpAgService::DisconnectSingleSco(const std::string &address)
{
    RawAddress rawAddr(address);
    std::string callingName = PermissionManager::GetCallingName();
    int state = HFP_AG_AUDIO_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(address, state)) {
        HILOGE("Invalid Device address:%{public}s", GET_ENCRYPT_STR_ADDR(address));
        return false;
    }
    if (state == HFP_AG_AUDIO_STATE_DISCONNECTED) {
        HILOGW("Sco is already disconnected! address:%{public}s, state[%{public}d]",
            GET_ENCRYPT_STR_ADDR(address), state);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_VOIP_DISCONN_SCO, rawAddr, UE_SCENE_CODE_FAIL,
            UE_FAIL_SCO_DISCONNECTED, callingName);
    }
    HILOG_COMM_INFO("DisconnectSingleSco: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    HfpAgMessage event(HFP_AG_DISCONNECT_AUDIO_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

int HfpAgService::GetScoState(const RawAddress &device)
{
    int state = HFP_AG_AUDIO_STATE_DISCONNECTED;
    if (!BluetoothStateManager::GetInstance()->FindRemoteDeviceHfpAgStateByAddr(device.GetAddress(), state)) {
        HILOGW("The statemachine is not available.");
        return HFP_AG_AUDIO_STATE_DISCONNECTED;
    }

    if (state <= HFP_AG_AUDIO_STATE_DISCONNECTED) {
        return HFP_AG_AUDIO_STATE_DISCONNECTED;
    } else {
        return state;
    }
}

bool HfpAgService::ShouldCallAudioBeActive()
{
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool isIncall = systemInterface.IsInCall();
    bool isRing = systemInterface.IsRinging();
    bool isInBandEnabled = IsInbandRingingEnabled();

    HILOGI("suspend phonestatechange, isIncall=%{public}d, isRing=%{public}d, isInBandEnabled=%{public}d",
        isIncall, isRing, isInBandEnabled);
    return isIncall || (isRing && isInBandEnabled);
}

void HfpAgService::HandleA2dpStateAfterCall(int callState, bool shouldActiveBefore)
{
    bool shouldActiveAfter = ShouldCallAudioBeActive();
    bool isAudioConnected = IsAudioConnected();
    HILOGI("suspend phonestatechange, Before=%{public}d, ActiveAfter=%{public}d, isAudioConnected=%{public}d",
        shouldActiveBefore, shouldActiveAfter, isAudioConnected);
    // Suspend A2DP when call is about to become active
    if (callState != HFP_AG_CALL_STATE_DISCONNECTED && callState != HFP_AG_CALL_STATE_DISCONNECTING
        && shouldActiveAfter && !shouldActiveBefore) {
        HfpAgSystemInterface::GetInstance().SetAudioParameters("A2dpSuspended", "1");
    }
    // Resume A2DP when call ended and SCO is not connected
    if (callState == HFP_AG_CALL_STATE_IDLE && !shouldActiveAfter && !isAudioConnected) {
        HfpAgSystemInterface::GetInstance().SetAudioParameters("A2dpSuspended", "0");
        SetResumeDevice(NULL_ADDRESS);
    }
}

void HfpAgService::HandleBccBlockTimer(bool shouldStart)
{
    if (shouldStart) {
        std::lock_guard<BtRecursiveMutex> lock(bccBlockTimerMutex_);
        if (!isBccBlockTimerActive_.load()) {
            bccBlockTimer_ = std::make_unique<utility::Timer>([this]() { this->BccBlockTimerTimeout(); });
            isBccBlockTimerActive_.store(true);
            bccBlockTimer_->Start(BCC_BLOCK_TIME);
        }
    } else {
        std::unique_ptr<utility::Timer> timerToStop = nullptr;
        {
            std::lock_guard<BtRecursiveMutex> lock(bccBlockTimerMutex_);
            if (bccBlockTimer_ != nullptr && isBccBlockTimerActive_.load()) {
                timerToStop = std::move(bccBlockTimer_);
                bccBlockTimer_.reset();
                isBccBlockTimerActive_.store(false);
            }
        }
        if (timerToStop != nullptr) {
            timerToStop->Stop();
        }
    }
}

void HfpAgService::SendCallStateEvent(bool isDelay, int64_t delayTime, const HfpAgMessage &curEvent,
    const std::string &taskName)
{
    if (isDelay) {
        HILOGI("delay CIEV 2 0 1s, address:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
        ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP, [this, curEvent]() { this->PostEvent(curEvent); },
            delayTime, taskName);
    } else {
        PostEvent(curEvent);
    }
}

void HfpAgService::HandleRealCallStateChange(int numActive, int callState, bool isDelayCiev, bool isDelayCallState,
    const HfpAgMessage &curEvent)
{
    bool shouldStartBccTimer = (numActive == 1 && callState == HFP_AG_CALL_STATE_IDLE
        && !isBccBlockTimerActive_.load());
    bool shouldStopBccTimer = (numActive == 0 && callState == HFP_AG_CALL_STATE_IDLE);

    if (shouldStopBccTimer) {
        HandleBccBlockTimer(false);
        if (isDelayCiev) {
            SendCallStateEvent(true, DELAY_CONNECT_VOIP_SCO, curEvent, "delay_voip_call_state");
        } else {
            PostEvent(curEvent);
        }
    } else {
        if (shouldStartBccTimer) {
            HandleBccBlockTimer(true);
        }
        if (isDelayCallState) {
            HILOGI("delay call state for this device, address:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
            SendCallStateEvent(true, DELAY_CALL_STATE, curEvent, "delay call state for this device");
        } else {
            PostEvent(curEvent);
        }
    }
}

void HfpAgService::HandleCallStateChange(BluetoothPhoneState &phoneState, bool isVirtualCall)
{
    bool shouldActiveBefore = ShouldCallAudioBeActive();
    int numActive = phoneState.GetActiveNum();
    int numHeld = phoneState.GetHeldNum();
    int callState = phoneState.GetCallState();
    HfpAgSystemInterface::GetInstance().SetActiveCallNumber(numActive);
    HfpAgSystemInterface::GetInstance().SetHeldCallNumber(numHeld);
    HfpAgSystemInterface::GetInstance().SetCallState(callState);
    RawAddress rawAddr(activeAddr_);
    bool isDelayCiev = BluetoothHwInterface::GetInstance()->InteropMatch(INTEROP_DELAY_CIEV, rawAddr);
    bool isDelayCallState = BluetoothHwInterface::GetInstance()->InteropMatch(INTEROP_DELAY_CALL_STATE, rawAddr);
    HfpAgMessage curEvent(HFP_AG_CALL_STATE_CHANGE);
    curEvent.state_ = {numActive, numHeld, callState, phoneState.GetNumber(), phoneState.GetCallType(),
            phoneState.GetName()};

    if (isVirtualCall) {
        std::lock_guard<std::mutex> lock(callstatemachineMutex_);
        if (callStateMachine_ != nullptr) {
            callStateMachine_->DealVoipCallStateChanged(curEvent);
        }
    } else {
        HandleRealCallStateChange(numActive, callState, isDelayCiev, isDelayCallState, curEvent);
    }
    HandleA2dpStateAfterCall(callState, shouldActiveBefore);
}

void HfpAgService::SendBsirValue()
{
    std::list<RawAddress> audioConnectableDevices = GetConnectingOrConnectionDevices();
    bool isInbandRingingEnabled = IsInbandRingingEnabled();

    HILOGI("send BSIR, activeAddr_ is %{public}s, audioConnectableDevicesSize is %{public}d",
        GET_ENCRYPT_STR_ADDR(activeAddr_), audioConnectableDevices.size());

    if (audioConnectableDevices.size() > 1 && activeAddr_ != NULL_ADDRESS &&
        activeAddr_ != EMPTY_ADDRESS && isInbandRingingEnabled) {
        for (RawAddress device : GetConnectDevices()) {
            if (device.GetAddress() == activeAddr_) {
                HfpAgMessage event(HFP_AG_SET_INBAND_RING_TONE_EVT, HFP_AG_INBAND_RING_ENABLE);
                event.dev_ = device.GetAddress();
                PostEvent(event);
                BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_INBAND_RING_SUPPORT, device,
                    UE_COMMON_SCENE_CASE3, "");
            } else {
                HfpAgMessage event(HFP_AG_SET_INBAND_RING_TONE_EVT, HFP_AG_INBAND_RING_DISABLE);
                event.dev_ = device.GetAddress();
                PostEvent(event);
                BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_INBAND_RING_SUPPORT, device,
                    UE_COMMON_SCENE_CASE4, "");
            }
        }
    }
}

void HfpAgService::SendBsirToLastRemainingDevice()
{
    std::list<RawAddress> audioConnectableDevices = GetConnectingOrConnectionDevices();
    bool isInbandRingingEnabled = IsInbandRingingEnabled();
    if (audioConnectableDevices.size() <= 1 && isInbandRingingEnabled) {
        HILOGI("Send Bsir enable");
        HfpAgMessage event(HFP_AG_SET_INBAND_RING_TONE_EVT, HFP_AG_INBAND_RING_ENABLE);
        SendEventToEachStateMachine(event);
        if (!audioConnectableDevices.empty()) {
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_INBAND_RING_SUPPORT, audioConnectableDevices.front(),
                UE_COMMON_SCENE_CASE3, "");
        }
    }
}

void HfpAgService::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    if (action == VIRTUAL_DEVICE_ADD && !virtualDevices_.IsExist(address)) {
        HILOGI("Add Virtual Device");
        virtualDevices_.Push(address);
        observers_.ForEach([action, address](HfpAgServiceObserver &observer) {
            observer.OnVirtualDeviceChanged(action, address);
        });
    }

    if (action == VIRTUAL_DEVICE_REMOVE && virtualDevices_.IsExist(address)) {
        HILOGI("Remove Virtual Device");
        virtualDevices_.Erase(address);
        observers_.ForEach([action, address](HfpAgServiceObserver &observer) {
            observer.OnVirtualDeviceChanged(action, address);
        });
    }
}

void HfpAgService::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    auto virtualDevices = virtualDevices_.GetVector();

    for (const auto& device : virtualDevices) {
        devices.push_back(device);
    }
}

void HfpAgService::PhoneStateChanged(BluetoothPhoneState &phoneState, bool isVirtualCall)
{
    int numActive = phoneState.GetActiveNum();
    int numHeld = phoneState.GetHeldNum();
    int callState = phoneState.GetCallState();
    RawAddress device(activeAddr_);

    if (!IsAllowHfpAgCallHook(activeAddr_)) {
        HILOGI("hfp ag call is disabled by product");
        return;
    }

    if (callState == HFP_AG_CALL_STATE_INCOMING) {
        bool isNotSendBsir = BluetoothHwInterface::GetInstance()->
            InteropMatch(INTEROP_DISABLE_INBAND_RING, device);
        if (isNotSendBsir) {
            HILOGI("not send Bsir, address:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
        } else {
            SendBsirValue();
        }
    }

    if ((callState == HFP_AG_CALL_STATE_INCOMING) || (callState == HFP_AG_CALL_STATE_DIALING)) {
        StopA2dpOffload();
    }

    if (dialingOutTimeout_ != nullptr) {
        if ((callState == HFP_AG_CALL_STATE_ACTIVE) || (callState == HFP_AG_CALL_STATE_IDLE)) {
            StopDialingTimer();
        } else if (callState == HFP_AG_CALL_STATE_DIALING) {
            StopDialingTimer();
            HfpAgMessage event(HFP_AG_DIALING_OUT_RESULT, HFP_AG_RESULT_OK);
            PostEvent(event);
        }
    }

    if ((numActive > 0) || (numHeld > 0) || (callState != HFP_AG_CALL_STATE_IDLE)) {
        if (!isVirtualCall && isVirtualCallStarted_) {
            // Before Telecom call setup update, need stop virtual voice SCO first.
            WaitForStopVirtualCall();
        }

        if (isVrOpened_) {
            HILOGI("Close the voice recognition");
            CloseVoiceRecognition(RawAddress(GetActiveDevice()));
        }
    }
    HandleCallStateChange(phoneState, isVirtualCall);
    CheckAndResumeSco(numActive, numHeld, callState);

    HILOGI("numActive=%{public}d, numHeld=%{public}d, callState=%{public}d, type=%{public}d, isVirtualCall=%{public}d,"
        "activeAddr_=%{public}s", numActive, numHeld, callState, phoneState.GetCallType(), isVirtualCall,
        GET_ENCRYPT_STR_ADDR(activeAddr_));
}

void HfpAgService::ClccResponse(
    int index, int direction, int status, int mode, bool mpty, const std::string &number, int type)
{
    HfpAgMessage curEvent(HFP_AG_SEND_CCLC_RESPONSE);
    curEvent.call_.index = index;
    curEvent.call_.dir = direction;
    curEvent.call_.state = status;
    curEvent.call_.mode = mode;
    curEvent.call_.mpty = mpty;
    curEvent.call_.type = type;
    curEvent.call_.number = number;
    PostEvent(curEvent);
}

bool HfpAgService::IsVoiceRecognitionAvailable(const std::string &address)
{
    std::string callingName = PermissionManager::GetCallingName();
    if (!IsAudioIdle()) {
        HILOGW("Sco is not idle!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_FAIL_AUDIO_NOT_IDLE, callingName);
        return false;
    }

    if (IsAudioConnected()) {
        HILOGE("audio is still active, please wait for audio to be disconnected");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_FAIL_AUDIO_CONNECTED, callingName);
        return false;
    }

    if (!IsConnected(address)) {
        HILOGW("Device is not connected!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_FAIL_AG_NOT_CONNECTED, callingName);
        return false;
    }

    return true;
}

void HfpAgService::SendOpenVoiceEvent(const std::string &address, bool isRequestByHf)
{
    HILOGI("DeviceAddr=%{public}s, isRequestByHf=%{public}d", GET_ENCRYPT_STR_ADDR(address), isRequestByHf);
    if (isRequestByHf) {
        HfpAgMessage evt(HFP_AG_VOICE_RECOGNITION_RESULT_EVT, HFP_AG_RESULT_OK);
        evt.dev_ = address;
        PostEvent(evt);
    }

    int requestSource = isRequestByHf ? HFP_START_VOICE_REG_BY_HF : HFP_START_VOICE_REG_BY_AG;
    HfpAgMessage evt(HFP_AG_OPEN_VOICE_RECOGNITION_EVT, requestSource);
    evt.dev_ = address;
    PostEvent(evt);
    isVrOpened_ = true;
}

bool HfpAgService::IsVirtualCallStarted()
{
    HILOGI("isVirtualCallStarted_=%{public}d", isVirtualCallStarted_);
    return isVirtualCallStarted_;
}

bool HfpAgService::OpenVoiceRecognition(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::string callingName = PermissionManager::GetCallingName();

    if (isVrOpened_) {
        bool status = CloseVoiceRecognition(RawAddress(activeAddr_));
        HILOGW("voice recognition is still active,"
            "just called stopVoiceRecognition, status:%{public}d , please try again", status);
        isVrOpened_ = false;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_FAIL_VR_IS_OPENED, callingName);
        return false;
    }

    if (!IsVoiceRecognitionAvailable(address)) {
        HILOG_COMM_WARN("Voice Recognition is not Available");
        return false;
    }
    currentCallType_ = static_cast<int>(BTCallType::CALL_TYPE_RECOGNITION);
    UpdateLocalVoiceCombineStateForScoStateChanged(address, false);
    if (voiceRecognitionTimeout_ != nullptr) {
        // VR is opening by handsfree unit, send result to handsfree unit
        voiceRecognitionTimeout_->Stop();
        voiceRecognitionTimeout_ = nullptr;
        SendOpenVoiceEvent(address, true);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_HF_REQ_ACCEPT, callingName);
    } else {
        // VR is opening by audio gateway, notify handsfree unit
        SendOpenVoiceEvent(address, false);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_LOCAL_START_VR_REQ_ACCEPT, callingName);
    }

    return true;
}

bool HfpAgService::SetActiveDeviceToStack(const RawAddress &device)
{
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("bluetoothHfpInterface is null");
        return false;
    }
    bluetoothHfpInterface->SetActiveDevice(&rawAddr);
    return true;
}

bool HfpAgService::SetActiveDevice(const RawAddress &device)
{
    std::string address = device.GetAddress();
    RawAddress preDevice(activeAddr_);
    std::string callingName = PermissionManager::GetCallingName();
    if (address == NULL_ADDRESS || address == EMPTY_ADDRESS) {
        ClearActiveDevice();
        if (!SetActiveDeviceToStack(device)) {
            BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
                UE_FAIL_SET_HFP_ACTIVE_DEV_NULL_NATIVE, callingName);
            return false;
        }
        HILOGW("Address is null");
        BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
            UE_SUCCESS_SET_HFP_ACTIVE_DEV_NULL, callingName);
        return true;
    }

    if (IsConnected(address) == false) {
        HILOGW("Device is not connected");
        BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
            UE_FAIL_SET_HFP_ACTIVE_DEV_NOT_CONN, callingName);
        return false;
    }
    HILOGI("DeviceAddr=%{public}s", GET_ENCRYPT_STR_ADDR(address));
    if (IsActiveDevice(address)) {
        BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
            UE_SUCCESS_SET_HFP_ACTIVE_DEV_NO_CHANGE, callingName);
        return true;
    }
    if (!UpdateActiveDevice(address)) {
        return false;
    }
    return true;
}

void HfpAgService::ClearActiveDevice()
{
    std::string preActiveDevice = GetActiveDevice();
    if (isVrOpened_) {
        HfpAgMessage evt(HFP_AG_CLOSE_VOICE_RECOGNITION_EVT);
        evt.dev_ = preActiveDevice;
        PostEvent(evt);
    }
    activeAddr_ = NULL_ADDRESS;
    NotifyCurrentActiveDevice(RawAddress(NULL_ADDRESS));
}

bool HfpAgService::IsActiveDevice(const std::string &address)
{
    std::string activeDeviceAddr = GetActiveDevice();
    if (activeDeviceAddr == address) {
        return true;
    } else {
        HILOGW("Device is not active device, address:%{public}s, activeDeviceAddr:%{public}s",
            GET_ENCRYPT_STR_ADDR(address), GET_ENCRYPT_STR_ADDR(activeDeviceAddr));
        return false;
    }
}

bool HfpAgService::UpdateActiveDevice(const std::string &newAddress)
{
    std::string preActiveDeviceAddr = GetActiveDevice();
    HILOGI("PreActiveDeviceAddr:%{public}s, newActiveDeviceAddress:%{public}s",
        GET_ENCRYPT_STR_ADDR(preActiveDeviceAddr), GET_ENCRYPT_STR_ADDR(newAddress));
    RawAddress preDevice(activeAddr_);
    RawAddress device(newAddress);
    std::string callingName = PermissionManager::GetCallingName();
    if (!SetActiveDeviceToStack(device)) {
        BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
            UE_FAIL_SET_HFP_ACTIVE_DEV_NATIVE, callingName);
        return false;
    }
    activeAddr_ = newAddress;
    if (isVrOpened_) {
        SendCloseVoiceEvent(preActiveDeviceAddr);
    }
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    if (systemInterface.IsRinging()) {
        bool isNotSendBsir = BluetoothHwInterface::GetInstance()->
            InteropMatch(INTEROP_DISABLE_INBAND_RING, device);
        if (isNotSendBsir) {
            HILOGI("not send Bsir, address:%{public}s", GET_ENCRYPT_STR_ADDR(activeAddr_));
        } else {
            SendBsirValue();
        }
    }

    NotifyCurrentActiveDevice(device);
    ConnectStrategyManager::GetInstance()->SaveLastActiveDevice(newAddress);
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_HFP_ACTIVE_DEV, preDevice, device,
        UE_SUCCESS_SET_HFP_ACTIVE_DEV_GENERAL, callingName);
    return true;
}

bool HfpAgService::IsIncall()
{
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool isInbandRingingEnabled = IsInbandRingingEnabled();
    return (systemInterface.IsInCall() || (systemInterface.IsRinging() && isInbandRingingEnabled) ||
        isVirtualCallStarted_);
}

bool HfpAgService::IsInActualcall()
{
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    return (systemInterface.IsInCall() || isVirtualCallStarted_);
}

std::string HfpAgService::GetActiveDevice()
{
    return activeAddr_;
}

void HfpAgService::NotifyAgIndicatorStateChanged(int what, const HfpAgTransferData &data)
{
    HfpAgMessage curEvent(what);
    curEvent.data_ = data;
    PostEvent(curEvent);
}

void HfpAgService::RegisterObserver(HfpAgServiceObserver &observer)
{
    observers_.Register(observer);
}

void HfpAgService::DeregisterObserver(HfpAgServiceObserver &observer)
{
    observers_.Deregister(observer);
}

int HfpAgService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::string callingName = PermissionManager::GetCallingName();
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if ((GetDeviceState(device)) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
        }
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, device, UE_COMMON_SCENE_CASE7, callingName);
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if ((GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTED) &&
            (GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
        }
        UpdateVirtualDevice(VIRTUAL_DEVICE_REMOVE, device.GetAddress());
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_DISCONN, device, UE_COMMON_SCENE_CASE4, callingName);
    } else {
        HILOGE("Strategy set failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

int HfpAgService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HFP_CONNECTION_POLICY);
}

void HfpAgService::ClearVirtualCallInfo(const RawAddress &device)
{
    std::unique_lock<std::mutex> lock(syncMutex_);
    HILOGI("device:%{public}s, isVirtualCallStarted_:%{public}d", GET_ENCRYPT_ADDR(device), isVirtualCallStarted_);
    if (!isVirtualCallStarted_ || device.GetAddress() != activeAddr_) {
        return;
    }
    isVirtualCallStarted_ = false;
}

void HfpAgService::NotifySlcStateChanged(const RawAddress &device, int toState)
{
    HILOGI("Device:%{public}s, toState:%{public}d", GET_ENCRYPT_ADDR(device), toState);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(bluetoothDevice != nullptr, "bluetoothDevice is nullptr");
    if (stateMap_.at(toState) == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_HFP_AG, false);
    } else if (stateMap_.at(toState) == static_cast<int>(BTConnectState::CONNECTING)) {
        {
            std::lock_guard<BtRecursiveMutex> lk(mutex_);
            auto it = stateMachines_.find(device.GetAddress());
            if (it != stateMachines_.end() && it->second != nullptr && it->second->GetTryReconnectHfp()) {
                bluetoothDevice->DialogControlAddConnectingProfileCount();
            }
        }
    }
    int cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device,
        PROFILE_ID_HFP_AG, stateMap_.at(toState));
    cause = bluetoothDevice->DialogControlConvertCause(cause);
    bluetoothDevice->DialogControlRemoveConnectingProfileCount(stateMap_.at(toState));
    if (toState == static_cast<int>(HFP_AG_STATE_CONNECTED)) {
        ConnectStrategyManager::GetInstance()->SaveLastConnectedDevice(device.GetAddress());
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_HFP_CONN_CMP, device, 0, 0);
        BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpConnected(device.GetAddress());
        isAllowConnectSco_ = true;
    }
    if (toState == static_cast<int>(HFP_AG_STATE_DISCONNECTED)) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_HFP_DISCONN_CMP, device, 0, 0);
        BluetoothDeviceBatteryManager::GetInstance()->ProcessHfpDisconnected(device.GetAddress());
        ClearVirtualCallInfo(device);
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService != nullptr, "a2dpService is nullptr");
        a2dpService->ConnectManager().RemoveA2dpAbnormalVirtualDevice(device);
    }
    NotifyConnStateChangedInner(device, stateMap_.at(toState), cause);
}

void HfpAgService::ReportStackScoStateChangeUeEvent(const RawAddress &device, int reason)
{
    switch (reason) {
        case HFP_AG_SCO_LOCAL_USER_TERMINATED:
            BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(CHR_UE_STACK_DISCONN_SCO, device,
                UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE3);
            return;
        case HFP_AG_SCO_REMOTE_USER_TERMINATED:
            BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(CHR_UE_STACK_DISCONN_SCO, device,
                UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE4);
            return;
        case HFP_AG_SCO_LOCAL_USER_SET_UP:
            BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(CHR_UE_STACK_CONN_SCO, device,
                UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE2);
            return;
        case HFP_AG_SCO_REMOTE_USER_SET_UP:
            BtChrUeManager::GetInstance()->WriteStackScoStateChangeUe(CHR_UE_STACK_CONN_SCO, device,
                UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE3);
            return;
        default:
            return;
    }
}

void PublishScoConnectStateChange(const RawAddress &device, int toState)
{
    int scoConnectState = -1;
    if (toState == OHOS::bluetooth::HfpAgAudioState::HFP_AG_AUDIO_STATE_DISCONNECTED) {
        scoConnectState = static_cast<int>(OHOS::BluetoothHelper::BroadcastScoState::SCO_STATE_DISCONNECTED);
    } else if (toState == OHOS::bluetooth::HfpAgAudioState::HFP_AG_AUDIO_STATE_CONNECTED) {
        scoConnectState = static_cast<int>(OHOS::BluetoothHelper::BroadcastScoState::SCO_STATE_CONNECTED);
    } else {
        return;
    }
    OHOS::BluetoothHelper::BluetoothCommonEventHelper::PublishScoConnectStateChangeEvent(
        device.GetAddress(), scoConnectState);
}

void HfpAgService::NotifyAudioStateChanged(const RawAddress &device, int toState, int reason)
{
    HILOGI("Device:%{public}s, toState:%{public}d, reason:%{public}d", GET_ENCRYPT_ADDR(device), toState, reason);
    observers_.ForEach([device, toState, reason](HfpAgServiceObserver &observer) {
        observer.OnScoStateChanged(device, toState, reason);
    });
    OHOS::bluetooth::PublishScoConnectStateChange(device, toState);
    ReportStackScoStateChangeUeEvent(device, reason);
}

void HfpAgService::NotifyCurrentActiveDevice(const RawAddress &device)
{
    HILOGI("DeviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    observers_.ForEach([device](HfpAgServiceObserver &observer) {
        observer.OnActiveDeviceChanged(device);
    });
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged(PROFILE_NAME_HFP_AG, device.GetAddress());
}

void HfpAgService::NotifyHfEnhancedDriverSafety(const RawAddress &device, int indValue)
{
    HILOGI("DeviceAddr=%{public}s, indValue=%{public}d",
        GET_ENCRYPT_ADDR(device), indValue);
    observers_.ForEach([device, indValue](HfpAgServiceObserver &observer) {
        observer.OnHfEnhancedDriverSafetyChanged(device, indValue);
    });
}

void HfpAgService::NotifyHfpStackChanged(int action, const RawAddress &device)
{
    HILOGI("DeviceAddr=%{public}s, action=%{public}d", GET_ENCRYPT_ADDR(device), action);
    DoInHfpThread([this, action, device]() { this->BluetoothNotifyHfpStackChanged(action, device); });
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_UPDATE_HFP_OUTPUT, device, RawAddress(GetActiveDevice()),
        action, callingName);
}

void HfpAgService::BluetoothNotifyHfpStackChanged(int action, const RawAddress &device)
{
    if (action == static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE) &&
        device.GetAddress() == GetResumeDevice()) {
        SetResumeDevice(NULL_ADDRESS);
    }
    observers_.ForEach([device, action](HfpAgServiceObserver &observer) {
        observer.OnHfpStackChanged(device, action);
    });
}

void HfpAgService::NotifyWearDetectionActionAfterConnected(const RawAddress &device)
{
    bool isSupport = false;
    int32_t isEnabled = -1;
    BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupport);
    BluetoothAudioManager::GetInstance().GetWearState(device.GetAddress(), isEnabled);
    HILOGI("DeviceAddr=%{public}s, isSupport=%{public}d, isEnabled=%{public}d",
        GET_ENCRYPT_ADDR(device), isSupport, isEnabled);
    if (!isSupport || isEnabled != WEAR_DETECTION_ENABLED) {
        return;
    }

    if (BluetoothAudioManager::GetInstance().IsDeviceWearing(device)) {
        NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_WEAR, device);
    } else {
        NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_UNWEAR, device);
    }
}

void HfpAgService::NotifyProfileState(const RawAddress &device)
{
    uint8_t a2dpState = 0;
    uint8_t hfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    bool ret = BluetoothAudioManager::GetInstance().GetProfileStatus(device.GetAddress(), a2dpState, hfpState);
    if (!ret) {
        return;
    }
    if (hfpState == static_cast<uint8_t>(HFP_STATUS_DISABLE)) {
        NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE, device);
    }
}

void HfpAgService::ProcessDialOutTimeOut()
{
    if (dialingOutTimeout_ != nullptr) {
        dialingOutTimeout_ = nullptr;
    }

    HfpAgMessage event(HFP_AG_DIALING_OUT_RESULT, HFP_AG_RESULT_ERROR);
    PostEvent(event);
}

bool HfpAgService::DialOutCallByHf(const std::string &address)
{
    if (dialingOutTimeout_ != nullptr) {
        HILOGE("Already dialing out!");
        return false;
    }

    if (!SetActiveDevice(RawAddress(address))) {
        HILOGE("It's failed to set active device to %{public}s", GET_ENCRYPT_STR_ADDR(address));
        return false;
    }

    dialingOutTimeout_ = std::make_unique<utility::Timer>([this]() { this->DialOutCallTimeOut(); });
    dialingOutTimeout_->Start(DIALING_OUT_TIMEOUT_TIME);
    HILOGI("Start dial timer!");
    return true;
}

void HfpAgService::DialOutCallTimeOut()
{
    HITRACE_METER(BT_TRACE_TAG);
    HfpAgMessage event(HFP_AG_DIAL_TIME_OUT_EVT);
    PostEvent(event);
}

bool HfpAgService::OpenVoiceRecognitionByHf(const std::string &address)
{
    std::string callingName = PermissionManager::GetCallingName();
    if (isVrOpened_) {
        bool status = CloseVoiceRecognition(RawAddress(activeAddr_));
        HILOGW("voice recognition is still active,"
            "just called stopVoiceRecognition, status:%{public}d , please try again", status);
        isVrOpened_ = false;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_REMOTE_START_VR_FAIL_VR_IS_OPENED, callingName);
        return false;
    }

    if (address == NULL_ADDRESS) {
        HILOGE("address is null");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(EMPTY_ADDRESS),
            UE_REMOTE_START_VR_FAIL_DEV_ADDR_NULL, callingName);
        return false;
    }

    // VR opened or in call
    if (!IsAudioIdle()) {
        HILOGE("Sco is not idle!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_REMOTE_START_VR_FAIL_AUDIO_NOT_IDLE, callingName);
        return false;
    }

    if (IsAudioConnected()) {
        HILOGE("audio is still active, please wait for audio to be disconnected");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_REMOTE_START_VR_FAIL_AUDIO_CONNECTED, callingName);
        return false;
    }

    // VR opening
    if (voiceRecognitionTimeout_ != nullptr) {
        HILOGE("Voice recognition is opening");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_REMOTE_START_VR_FAIL_VR_IS_OPENING, callingName);
        return false;
    }

    int32_t ret = WakeUpVoiceRecognition(address);
    if (ret != BT_NO_ERROR) {
        HILOGE("Wake up voice recognition failed, reason:%{public}d", ret);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
            UE_REMOTE_START_VR_FAIL_WAKEUP_VR, callingName);
        return false;
    }
    currentCallType_ = static_cast<int>(BTCallType::CALL_TYPE_RECOGNITION);
    voiceRecognitionTimeout_ = std::make_unique<utility::Timer>([this]() { this->VoiceRecognitionTimeOut(); });
    voiceRecognitionTimeout_->Start(VOICE_RECOGNITION_TIMEOUT_TIME);
    HILOGI("Wake up voice recognition success!");

    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address),
        UE_REMOTE_START_VR_REQ_ACCEPT, callingName);
    return true;
}

bool HfpAgService::CloseVoiceRecognitionByHf(const std::string &address)
{
    std::string callingName = PermissionManager::GetCallingName();
    if (!IsActiveDevice(address)) {
        HILOGE("%{public}s is not active, active device is: %{public}s",
            GET_ENCRYPT_STR_ADDR(address), GET_ENCRYPT_STR_ADDR(GetActiveDevice()));
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, RawAddress(address),
            UE_REMOTE_STOP_VR_FAIL_NOT_ACTIVE_DEV, callingName);
        return false;
    }

    // VR not opening or opened
    if (!isVrOpened_ && (voiceRecognitionTimeout_ == nullptr)) {
        HILOGE("Voice recognition is not opening or opened!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, RawAddress(address),
            UE_REMOTE_STOP_VR_FAIL_VR_NOT_OPENED, callingName);
        return false;
    }

    // VR opening
    if (voiceRecognitionTimeout_ != nullptr) {
        voiceRecognitionTimeout_->Stop();
        voiceRecognitionTimeout_ = nullptr;
    }

    // VR opened
    if (isVrOpened_) {
        if (!DisconnectSingleSco(address)) {
            HILOGW("Failed to disconnect audio from %{public}s", GET_ENCRYPT_STR_ADDR(address));
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, RawAddress(address),
                UE_REMOTE_STOP_VR_FAIL_DISCONN_SCO, callingName);
            return false;
        }
        isVrOpened_ = false;
    }

    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, RawAddress(address),
        UE_REMOTE_STOP_VR_REQ_ACCEPT, callingName);
    return true;
}

void HfpAgService::VoiceRecognitionTimeOut()
{
    HITRACE_METER(BT_TRACE_TAG);
    HfpAgMessage event(HFP_AG_VOICE_RECOGNITION_TIME_OUT_EVT);
    PostEvent(event);
}

void HfpAgService::ResponseClccTimeOut()
{
    HITRACE_METER(BT_TRACE_TAG);
    HfpAgMessage event(HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT);
    PostEvent(event);
}

void HfpAgService::SlcStateChanged(const std::string &address, int toState)
{
    if (toState == HFP_AG_STATE_DISCONNECTED) {
        if (GetActiveDevice() == address) {
            SetActiveDevice(RawAddress(NULL_ADDRESS));
        }
    }
}

void HfpAgService::StopVoiceRecognitionToStack(const std::string &address)
{
    HILOGI("Hfp device address[%{public}s]", GetEncryptAddr(address).c_str());
    RawAddress device(address);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    CHECK_AND_RETURN_LOG(bluetoothHfpInterface != nullptr, "BluetoothHfpInterface is null.");

    bt_status_t status = bluetoothHfpInterface->StopVoiceRecognition(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Fail StopVoiceRecognition, status: %{public}d", status);
        return;
    }
    isVrOpened_ = false;
}

void HfpAgService::ScoStateChanged(const std::string &address, int toState)
{
    if (toState == HFP_AG_AUDIO_STATE_DISCONNECTED) {
        if (voiceRecognitionTimeout_) {
            if (!CloseVoiceRecognitionByHf(address)) {
                HILOGE("Failed to close the voice recognition!");
            }
        }
        HfpAgSystemInterface::GetInstance().SetAudioParameters("A2dpSuspended", "0");
        StopVoiceRecognitionToStack(address);
        if (isVirtualCallStarted_ && address == activeAddr_) {
            HILOGW("stop virtual voice call because device: %{public}s sco disconnected",
                GET_ENCRYPT_STR_ADDR(address));
            StopVirtualCallSco();
        }
        // allow connect sco when device sco state is disconnected
        isAllowConnectSco_ = true;
        scoCondition_.notify_all();
        UpdateLocalVoiceCombineStateForScoStateChanged(address, false);
        ClearCallStateMachine();
    } else {
        isAllowConnectSco_ = false;
        if (IsActiveDevice(address) && toState == HFP_AG_AUDIO_STATE_CONNECTING) {
            auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
            CHECK_AND_RETURN_LOG(bthwif != nullptr, "bthwInterface_ is null");
            STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(RawAddress(address));
            bool voiceCombineAbility = bthwif->hwGetRemoteVoiceCombineAbility(rawAddr);
            BluetoothAudioFrameworkAdapter::GetInstance().SetActiveDeviceVoiceCombineAbility(voiceCombineAbility);
        }
    }
}

void HfpAgService::SetInbandRing(bool action)
{
    isInbandRinging_ = action;
    HfpAgMessage curEvent(HFP_AG_SET_INBAND_RING_TONE_EVT, action);
    PostEvent(curEvent);
}

::bluetooth::headset::Interface* HfpAgService::getBluetoothHfpInterface() const
{
    return bluetoothHfpInterface;
}

int HfpAgService::CovertConnectStateFromStack(::bluetooth::headset::bthf_connection_state_t state)
{
    if (state == ::bluetooth::headset::BTHF_CONNECTION_STATE_DISCONNECTING) {
        return HFP_AG_STATE_DISCONNECTING;
    }
    if (state == ::bluetooth::headset::BTHF_CONNECTION_STATE_CONNECTING) {
        return HFP_AG_STATE_CONNECTING;
    }
    if (state == ::bluetooth::headset::BTHF_CONNECTION_STATE_CONNECTED) {
        return HFP_AG_STATE_CONNECTING;
    }
    if (state == ::bluetooth::headset::BTHF_CONNECTION_STATE_SLC_CONNECTED) {
        return HFP_AG_STATE_CONNECTED;
    }
    return HFP_AG_STATE_DISCONNECTED;
}

void HfpAgService::HfpAgServiceCallbacks::ConnectionStateCallback(::bluetooth::headset::bthf_connection_state_t state,
    STACK::RawAddress* bdAddr)
{
    HILOGI("ConnectState=%{public}d", state);
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    if (state == ::bluetooth::headset::BTHF_CONNECTION_STATE_DISCONNECTED) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, rawAddr.GetAddress(), "HFPDISCONNECTTIME");
    }
    int connectState = HfpAgService::CovertConnectStateFromStack(state);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null");
    service->ConnectionStateCallbackInner(rawAddr, connectState);
    if (connectState == static_cast<int>(HfpAgConnectState::HFP_AG_STATE_CONNECTED)) {
        ConnectStrategyManager::GetInstance()->ProfileConnectionStateChange(rawAddr.GetAddress(), "hfpService");
    } else if (connectState < static_cast<int>(HfpAgConnectState::HFP_AG_STATE_CONNECTED) &&
        rawAddr.GetAddress() == service->GetResumeDevice()) {
        service->SetResumeDevice(NULL_ADDRESS);
    }
    service->ProcessChrAgConnectionState(rawAddr, connectState);
}


void HfpAgService::ProcessChrAgConnectionState(const RawAddress rawAddr, const int connectState)
{
    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, rawAddr.GetAddress(), "HFPSTATUS", connectState);
    if (connectState == static_cast<int>(HfpAgConnectState::HFP_AG_STATE_CONNECTED)) {
#ifdef BLUETOOTH_WATCH_ENABLE
        A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(service != nullptr, "service is null");
        if (service->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED)) {
            BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, rawAddr.GetAddress(), "DISCONNECTREASON", 0);
        }
#endif
    }
}


void HfpAgService::ConnectionStateCallbackInner(RawAddress rawAddr, int state)
{
    if (!IsAcceptConnection(rawAddr, state)) {
        if (bluetoothHfpInterface != nullptr) {
            STACK::RawAddress device = ServiceUtil::AddrToStack(rawAddr);
            bluetoothHfpInterface->Disconnect(&device);
        }
        return;
    }
    HfpAgMessage event(HFP_AG_CONNECTION_STATE_CHANGED_EVT, state);
    event.dev_ = rawAddr.GetAddress();
    PostEvent(event);
}

bool HfpAgService::IsAcceptConnection(RawAddress &rawAddr, int state)
{
    int connectStrategy = GetConnectStrategy(rawAddr);
    if (state == HFP_AG_STATE_CONNECTING
        && connectStrategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGW("rejected incoming HF connection");
        return false;
    }
    return true;
}

int HfpAgService::CovertAudioStateFromStack(::bluetooth::headset::bthf_audio_state_t state)
{
    if (state == ::bluetooth::headset::BTHF_AUDIO_STATE_CONNECTING) {
        return HFP_AG_AUDIO_STATE_CONNECTING;
    }
    if (state == ::bluetooth::headset::BTHF_AUDIO_STATE_DISCONNECTING) {
        return HFP_AG_AUDIO_STATE_DISCONNECTING;
    }
    if (state == ::bluetooth::headset::BTHF_AUDIO_STATE_CONNECTED) {
        return HFP_AG_AUDIO_STATE_CONNECTED;
    }
    return HFP_AG_AUDIO_STATE_DISCONNECTED;
}

void HfpAgService::HfpAgServiceCallbacks::AudioStateCallback(::bluetooth::headset::bthf_audio_state_t state,
    STACK::RawAddress* bdAddr)
{
    HILOGI("AudioState=%{public}d for %{public}s", state, GetEncryptAddr(bdAddr->ToStringForLogging()).c_str());
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    int audioState = HfpAgService::CovertAudioStateFromStack(state);
    HfpAgMessage event(HFP_AG_AUDIO_STATE_CHANGED_EVT, audioState);
    event.dev_ = rawAddr.GetAddress();
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

int HfpAgService::CovertVRStateFromStack(::bluetooth::headset::bthf_vr_state_t state)
{
    if (state == ::bluetooth::headset::BTHF_VR_STATE_STOPPED) {
        return HFP_AG_HF_VR_ClOSED;
    }
    if (state == ::bluetooth::headset::BTHF_VR_STATE_STARTED) {
        return HFP_AG_HF_VR_OPENED;
    }
    return HFP_AG_HF_VR_ClOSED;
}

void HfpAgService::HfpAgServiceCallbacks::VoiceRecognitionCallback(::bluetooth::headset::bthf_vr_state_t state,
    STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_VR_CHANGED;
    event.arg1_ = CovertVRStateFromStack(state);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AnswerCallCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_ANSWER_CALL;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::HangupCallCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_HANGUP_CALL;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

int HfpAgService::CovertVolumeControlTypeFromStack(::bluetooth::headset::bthf_volume_type_t type)
{
    if (type == ::bluetooth::headset::BTHF_VOLUME_TYPE_SPK) {
        return HFP_AG_VOLUME_TYPE_SPK;
    }
    if (type == ::bluetooth::headset::BTHF_VOLUME_TYPE_MIC) {
        return HFP_AG_VOLUME_TYPE_MIC;
    }
    return HFP_AG_VOLUME_TYPE_SPK;
}

void HfpAgService::HfpAgServiceCallbacks::VolumeControlCallback(::bluetooth::headset::bthf_volume_type_t type,
    int volume, STACK::RawAddress* bdAddr)
{
    int volumeControleType = CovertVolumeControlTypeFromStack(type);
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT, volumeControleType);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_VOLUME_CHANGED;
    event.arg3_ = volume;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::DialCallCallback(char* number, STACK::RawAddress* bdAddr)
{
    CHECK_AND_RETURN_LOG(number != nullptr, "number is nullptr.");
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_DIAL_CALL;
    size_t len = strlen(number);
    event.str_ = std::string(number, len);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::DtmfCmdCallback(char dtmf, STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_SEND_DTMF;
    event.arg1_ = dtmf;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

bool HfpAgService::CovertNoiseReductionFromStack(::bluetooth::headset::bthf_nrec_t nrec)
{
    if (nrec == ::bluetooth::headset::BTHF_NREC_START) {
        return true;
    }
    return false;
}

void HfpAgService::HfpAgServiceCallbacks::NoiseReductionCallback(::bluetooth::headset::bthf_nrec_t nrec,
    STACK::RawAddress* bdAddr)
{
    HILOGI("Nrec=%{public}d for %{public}s", nrec, GET_ENCRYPT_STR_ADDR(bdAddr->ToStringForLogging()));
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_NOISE_REDUCTION;
    event.arg1_ = CovertNoiseReductionFromStack(nrec) ? 1 : 0;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

int HfpAgService::ConvetWbsConfigFromStack(::bluetooth::headset::bthf_wbs_config_t wbsConfig)
{
    if (wbsConfig == ::bluetooth::headset::BTHF_WBS_NO) {
        return HFP_AG_WBS_NO;
    }
    if (wbsConfig == ::bluetooth::headset::BTHF_WBS_YES) {
        return HFP_AG_WBS_YES;
    }
    if (wbsConfig == ::bluetooth::headset::BTHF_WBS_SUPER) {
        return HFP_AG_SWB_YES;
    }
    return HFP_AG_WBS_NONE;
}

void HfpAgService::HfpAgServiceCallbacks::WbsCallback(::bluetooth::headset::bthf_wbs_config_t wbsConfig,
    STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_WBS;
    event.arg1_ = ConvetWbsConfigFromStack(wbsConfig);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtChldCallback(::bluetooth::headset::bthf_chld_type_t chld,
    STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_CHLD;
    event.arg1_ = static_cast<int>(chld);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtCnumCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtCindCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_CIND;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtCopsCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_COPS;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtClccCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_CLCC;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::UnknownAtCallback(char* atString, STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_UNKNOWN;
    size_t len = strlen(atString);
    event.str_ = std::string(atString, len);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::KeyPressedCallback(STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_KEY_PRESSED;
}

void HfpAgService::HfpAgServiceCallbacks::AtBindCallback(char* atString, STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_BIND;
    size_t len = strlen(atString);
    event.str_ = std::string(atString, len);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

#ifdef BLUETOOTH_SCO_NORMALIZED_FEATURE_ENABLE
void HfpAgService::HfpAgServiceCallbacks::AtBccCallback(STACK::RawAddress* bdAddr)
{
    CHECK_AND_RETURN_LOG(bdAddr != nullptr, "HfpAgServiceCallbacks::AtBccCallback bdAddr is null.");
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_BCC;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}
#endif

#ifdef COMMUNICATION_L2
void HfpAgService::HfpAgServiceCallbacks::AtBrsfCallback(uint32_t features, STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_BRSF;
    event.arg1_ = static_cast<int>(features);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}
#endif

int HfpAgService::CovertBievValueFromStack(::bluetooth::headset::bthf_hf_ind_type_t indId)
{
    if (indId == ::bluetooth::headset::BTHF_HF_IND_ENHANCED_DRIVER_SAFETY) {
        return HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID;
    }
    if (indId == ::bluetooth::headset::BTHF_HF_IND_BATTERY_LEVEL_STATUS) {
        return HFP_AG_HF_INDICATOR_BATTERY_LEVEL_ID;
    }
    return HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID;
}

void HfpAgService::HfpAgServiceCallbacks::AtBievCallback(::bluetooth::headset::bthf_hf_ind_type_t indId,
    int indValue, STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    int bievValue = CovertBievValueFromStack(indId);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT, bievValue);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_BIEV;
    event.arg3_ = indValue;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgService::HfpAgServiceCallbacks::AtBiaCallback(bool service, bool roam, bool signal, bool battery,
    STACK::RawAddress* bdAddr)
{
    RawAddress rawAddr = ServiceUtil::AddrFromStack(*bdAddr);
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_AG_MSG_TYPE_AT_BIA;
    event.data_.service = service;
    event.data_.roam = roam;
    event.data_.signal = signal;
    event.data_.battery = battery;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

bool HfpAgService::IntoMock(int state)
{
    return false;
}

bool HfpAgService::SendNoCarrier(const RawAddress &device)
{
    return false;
}

void HfpAgService::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    HfpAgMessage event(HFP_AG_SET_VOLUME_EVT, streamType);
    event.arg3_ = volume;
    PostEvent(event);
}

int HfpAgService::GetVirtualCallActiveNum(void)
{
    std::lock_guard<std::mutex> lock(callstatemachineMutex_);
    if (callStateMachine_ != nullptr) {
        return callStateMachine_->GetActiveNum();
    }
    return 0;
}

int HfpAgService::GetVirtualCallState(void)
{
    std::lock_guard<std::mutex> lock(callstatemachineMutex_);
    if (callStateMachine_ != nullptr) {
        return callStateMachine_->GetCallState();
    }
    return HFP_AG_CALL_STATE_IDLE;
}

void HfpAgService::ClearCallStateMachine(void)
{
    std::lock_guard<std::mutex> lock(callstatemachineMutex_);
    if (callStateMachine_ != nullptr) {
        callStateMachine_->RemoveTask();
        callStateMachine_ = nullptr;
    }
}

void HfpAgService::NotifyPhonebookAuthResult(const RawAddress& device, int32_t accessAuthorization,
    int32_t pbapLoadFlag)
{
    HfpAgMessage event(HFP_AG_PHONEBOOK_AUTH_RESULT, accessAuthorization);
    event.dev_ = device.GetAddress();
    event.arg3_ = pbapLoadFlag;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

int32_t HfpAgService::WakeUpVoiceRecognition(const std::string &address)
{
    AAFwk::Want want;
    want.SetElementName(VOICE_RECOGNITION_BUNDLE_NAME, VOICE_RECOGNITION_ABILITY_NAME);
    want.SetParam(VOICE_RECOGNITION_LANUCH_TYPE, VOICE_RECOGNITION_BLUETOOTH);
    std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> currentAudioDevice =
        AudioStandard::AudioDevicesClientManager::GetInstance().
            GetDevices(AudioStandard::DeviceFlag::INPUT_DEVICES_FLAG);
    if (currentAudioDevice.empty()) {
        HILOGE("currentAudioDevice is empty!");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> audioDeviceDescriptor;
    for (const auto &desc : currentAudioDevice) {
        if (desc == nullptr) {
            HILOGE("[HFP_EVENT_PROCESSER]desc is null.");
            continue;
        }
        if (desc->deviceType_ == AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
            desc->macAddress_ = address;
            audioDeviceDescriptor.push_back(desc);
            break;
        }
    }
    sptr<AudioStandard::AudioCapturerFilter> audioCapturerFilter =
        sptr<AudioStandard::AudioCapturerFilter>::MakeSptr();
    audioCapturerFilter->capturerInfo.sourceType = AudioStandard::SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    audioCapturerFilter->uid = BLUETOOTH_UID;
    AudioStandard::AudioDevicesClientManager::GetInstance().
        SelectInputDevice(audioCapturerFilter, audioDeviceDescriptor);
    return SystemAbilityManagerUtils::StartAbility(want);
}

void HfpAgService::ProcessHwDeviceInfoCmd(const std::string &addr, const std::string &commandStr)
{
    HfpAgSystemEventProcesser processer(addr);
    // echo 4,3 不依赖 HFP，直接调用解析，否则可能由于HFP未建立导致消息丢失
    processer.ProcessVendorSpecificAt(commandStr, addr);
}

void HfpAgService::StopA2dpOffload()
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        IProfileA2dpSrc *a2dpSrcService = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
        if (a2dpSrcService != nullptr) {
            RawAddress device = a2dpSrcService->GetActiveSinkDevice();
            a2dpSrcService->ForceStopOffloadPlaying(device);
        }
    }
}

void HfpAgService::WaitForStopVirtualCall()
{
    std::unique_lock<std::mutex> lock(syncMutex_);
    if (!condition_.wait_for(lock, std::chrono::milliseconds(WAIT_FOR_STOP_VIRTUAL_CALL_TIMEOUT),
        [this]() { return !isVirtualCallStarted_; })) {
        HILOGW("stop virtual call timeout.");
    }
}

void HfpAgService::WaitForScoDisconnected()
{
    std::unique_lock<std::mutex> lock(scoSyncMutex_);
    if (!scoCondition_.wait_for(lock, std::chrono::milliseconds(WAIT_FOR_SCO_DISCONNECTED_TIMEOUT),
        [this]() { return isAllowConnectSco_.load(); })) {
        HILOGW("wait sco disconnect timeout.");
    }
}

void HfpAgService::UpdateLocalVoiceCombineStateForScoStateChanged(const std::string &address, const bool isCall)
{
    auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bthwif != nullptr, "bthwInterface_ is null");

    int voiceCombineState = bthwif->hwGetLocalVoiceCombineFeatureState();
    HILOGI("voiceCombineState=%{public}d, isCall=%{public}d", voiceCombineState, isCall);
    // Ensure that the voice combine cap read from audio fwk is enabled.
    if (voiceCombineState > 0) {
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(RawAddress(address));
        bthwif->hwSetVoiceCombineScenarioEnabled(rawAddr, isCall);
    }
}

bool HfpAgService::ProcCloudDeviceConnect(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    if (classicAdapter->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        HILOGI("[CLOUD_PAIR] profile cloud connect bonding, device:%{public}s",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        NotifyConnStateChangedInner(device, static_cast<int>(BTConnectState::CONNECTING), 0);
        return true;
    }
    if (classicAdapter->IsStartCloudPair(device)) {
        HILOGI("[CLOUD_PAIR] profile cloud connect create acl first, device:%{public}s",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        classicAdapter->SetCloudBondState(device, CLOUD_BOND_BONDING);
        classicAdapter->CancelDiscoveryIfInCloudBonding(device);
        classicAdapter->CreateAclConnect(device.GetAddress());
        NotifyConnStateChangedInner(device, static_cast<int>(BTConnectState::CONNECTING), 0);
        CloudDeviceManager::GetInstance()->StopBtAclTimer(device);
        CloudDeviceManager::GetInstance()->StartBtAclTimer(device);
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
            static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
            static_cast<int>(UeConnCloudPairSubSceneCode::BEGIN_CLOUD_PAIR));
        return true;
    }
    return false;
}

bool HfpAgService::IsInCloudBondingState(const RawAddress &device) const
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    if (classicAdapter->IsCloudDevice(device) &&
        classicAdapter->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        return true;
    }
    return false;
}

void HfpAgService::SetAudioParameters() const
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second != nullptr) {
            auto audioState = it->second->GetStateInt();
            if (audioState == HFP_AG_AUDIO_STATE_CONNECTED) {
                it->second->SetAudioParameters();
            }
        }
    }
}

int HfpAgService::GetCurrentCallType(int &callType)
{
    BtChrCallExcpEvent(activeAddr_, ERRCODE_CALLTYPE_STATUS, CHR_SUB_ERRCODE_CASE1, -1);
    callType = currentCallType_;
    return callType;
}

void HfpAgService::BccBlockTimerTimeout()
{
    std::unique_ptr<utility::Timer> timerToDestroy = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(bccBlockTimerMutex_);
        isBccBlockTimerActive_.store(false);
        timerToDestroy = std::move(bccBlockTimer_);
        bccBlockTimer_.reset();
    }
    if (timerToDestroy != nullptr) {
        timerToDestroy->Stop();
    }
}

int HfpAgService::IsVoiceRecognitionSupported(const RawAddress &device,  bool &isSupported)
{
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG_RET(bluetoothHfpInterface != nullptr, false, "BluetoothHfpInterface is null.");
    bt_status_t status = bluetoothHfpInterface->isVoiceRecognitionSupported(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGI("Hfp device address[%{public}s] is not supported voice recognition", GET_ENCRYPT_ADDR(device));
        BtChrCallExcpEvent(device.GetAddress(), ERRCODE_VOICE_RECOGNITION, CHR_SUB_ERRCODE_CASE2, -1);
        isSupported = false;
    } else {
        isSupported = true;
    }
    return BT_NO_ERROR;
}

bool HfpAgService::IsBccBlockTimerActive() const
{
    return isBccBlockTimerActive_.load();
}

REGISTER_CLASS_CREATOR(HfpAgService);

void HfpAgService::SendAtCmdByTopApptype(AppCategory topApptype)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(activeAddr_);
    if (it == stateMachines_.end() || it->second == nullptr) {
        BtChrCallExcpEvent(activeAddr_, ERRCODE_STATEMACHINES_STATUS, CHR_SUB_ERRCODE_CASE1, -1);
        HILOGW("Cannot find the stateMachine");
        return;
    }
    it->second->SendAtCommandByEventProcessor(topApptype);
}

}  // namespace bluetooth
}  // namespace OHOS
