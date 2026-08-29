/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hfp_hf"
#endif

#include "hfp_hf_service.h"
#include "adapter_config.h"
#include "adapter_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_connection_manager.h"
#include "class_creator.h"
#include "connect_strategy_manager.h"
#include "common_util.h"
#include "profile_service_manager.h"
#include "permission_manager.h"
#include "stub/telephone_client_service.h"
#include "thread_util.h"
#include "bluetooth_errorcode.h"
#include "call_manager_client.h"
#include "bt_chr_base.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bt_func_hook.h"
#include "profile_config.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

HfpHfService *HfpHfService::GetService()
{
    auto srevManager = IProfileManager::GetInstance();
    return static_cast<HfpHfService *>(srevManager->GetProfileService(PROFILE_NAME_HFP_HF));
}

HfpHfService::HfpHfService() : utility::Context(PROFILE_NAME_HFP_HF, "1.7.1")
{
}

HfpHfService::~HfpHfService()
{
}

utility::Context *HfpHfService::GetContext()
{
    return this;
}

static int CovertConnectStateFromStack(BthfClientConnectionState state)
{
    if (state == BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED) {
        return HFP_HF_SLC_ESTABLISHED_EVT;
    } else if (state == BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED) {
        return HFP_HF_DISCONNECTED_EVT;
    } else if (state == BTHF_CLIENT_CONNECTION_STATE_CONNECTED) {
        return HFP_HF_CONNECTED_EVT;
    } else if (state == BTHF_CLIENT_CONNECTION_STATE_CONNECTING) {
        return HFP_HF_CONNECTING_EVT;
    }
    return HFP_HF_DISCONNECTING_EVT;
}

static void BtHfConnectionStateCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientConnectionState state, unsigned int peerFeat, unsigned int chldFeat)
{
    HILOGI("[HFP HF]ConnectState=%{public}d", state);
    int connectState = CovertConnectStateFromStack(state);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(connectState, peerFeat);
    event.dev_ = rawAddr.GetAddress();
    event.arg3_ = static_cast<int>(chldFeat);
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static int CovertAudioStateFromStack(BthfClientAudioState state)
{
    if (state == BTHF_CLIENT_AUDIO_STATE_DISCONNECTED) {
        return HFP_HF_AUDIO_DISCONNECTED_EVT;
    } else if (state == BTHF_CLIENT_AUDIO_STATE_CONNECTED) {
        return HFP_HF_AUDIO_CONNECTED_EVT;
    } else if (state == BTHF_CLIENT_AUDIO_STATE_CONNECTING) {
        return HFP_HF_AUDIO_CONNECTING_EVT;
    }
    return HFP_HF_AUDIO_CONNECTED_MSBC_EVT;
}

static void BtHfAudioStateCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientAudioState state)
{
    HILOGI("[HFP HF]AudioState=%{public}d", state);
    RawAddress rawAddr = *bdAddr;
    int audioState = CovertAudioStateFromStack(state);
    HfpHfMessage event(audioState);
    event.dev_ = rawAddr.GetAddress();
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfVrCmdCb(const OHOS::bluetooth::RawAddress* bdAddr, BthfClientVrState state)
{
    HILOGI("[HFP HF]VRState=%{public}d", state);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED, state);
    event.dev_ = rawAddr.GetAddress();
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfNetworkStateCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientNetworkState state)
{
    HILOGI("[HFP HF]networkState=%{public}d", state);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, state);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_NETWORK_STATE;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfNetworkRoamingCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientServiceType type)
{
    HILOGI("[HFP HF]networkType=%{public}d", type);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, type);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_NETWORK_ROAM;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfNetworkSignalCb(const OHOS::bluetooth::RawAddress* bdAddr, int signal)
{
    HILOGI("[HFP HF]networkSignal=%{public}d", signal);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, signal);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_NETWORK_SIGNAL;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfBatteryLevelCb(const OHOS::bluetooth::RawAddress* bdAddr, int level)
{
    HILOGI("[HFP HF]batteryLevel=%{public}d", level);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, level);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_BATTERY_LEVEL;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCurrentOperatorCb(const OHOS::bluetooth::RawAddress* bdAddr, const char* name)
{
    HILOGI("[HFP HF]BtHfCurrentOperatorCb");
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CURRENT_OPERATOR;
    CHECK_AND_RETURN_LOG(name != nullptr, "[HFP HF]BtHfCurrentOperatorCb name is null");
    size_t len = strlen(name);
    event.str_ = std::string(name, len);
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCallCb(const OHOS::bluetooth::RawAddress* bdAddr, BthfClientCall call)
{
    HILOGI("[HFP HF]call=%{public}d", call);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, call);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CALL_STATE;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCallSetupCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientCallsetup callsetup)
{
    HILOGI("[HFP HF]callsetup=%{public}d", callsetup);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CALL_SETUP_STATE;
    event.call_.status = callsetup;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCallHeldCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientCallheld callheld)
{
    HILOGI("[HFP HF]callheld=%{public}d", callheld);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, callheld);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CALL_HELD_STATE;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfRespAndHoldCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientRespAndHold respAndHold)
{
    HILOGI("[HFP HF]respAndHold=%{public}d", respAndHold);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, respAndHold);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_HOLD_RESULT;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfClipCb(const OHOS::bluetooth::RawAddress* bdAddr, const char* number)
{
    HILOGI("[HFP HF]bthf_clip_cb");
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION;
    CHECK_AND_RETURN_LOG(number != nullptr, "[HFP HF]BtHfClipCb number is null");
    size_t len = strlen(number);
    event.call_.number = std::string(number, len);
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCallWaitingCb(const OHOS::bluetooth::RawAddress* bdAddr, const char* number)
{
    HILOGI("[HFP HF]bthf_call_waiting_cb");
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CALL_WAITING;
    CHECK_AND_RETURN_LOG(number != nullptr, "[HFP HF]BtHfCallWaitingCb number is null");
    size_t len = strlen(number);
    event.call_.number = std::string(number, len);
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCurrentCallsCb(const OHOS::bluetooth::RawAddress* bdAddr, int index,
    BthfClientCallDirection dir, BthfClientCallState state,
    BthfClientCallMptyType mpty, const char* number)
{
    HILOGI("[HFP HF]callstate=%{public}d", state);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_CURRENT_CALLS;
    CHECK_AND_RETURN_LOG(number != nullptr, "[HFP HF]BtHfCurrentCallsCb number is null");
    size_t len = strlen(number);
    event.call_.number = std::string(number, len);
    event.call_.index = index;
    event.call_.status = state;
    event.call_.dir = dir;
    event.call_.mprty = mpty;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfVolumeChangeCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientVolumeType type, int volume)
{
    HILOGI("[HFP HF]change volume=%{public}d, volume type=%{public}d", volume, type);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, type);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_SET_VOLUME;
    event.arg3_ = volume;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfCmdCompleteCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientCmdComplete type, int cme)
{
    HILOGI("[HFP HF]bthf_cmd_complete_cb=%{public}d", type);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, type);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_AT_CMD_RESULT;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfSubscriberInfoCb(const OHOS::bluetooth::RawAddress* bdAddr, const char* name,
    BthfClientSubscriberServiceType type)
{
    HILOGI("[HFP HF]BthfClientSubscriberServiceType=%{public}d", type);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, type);
    event.dev_ = rawAddr.GetAddress();
    CHECK_AND_RETURN_LOG(name != nullptr, "[HFP HF]BtHfSubscriberInfoCb name is null");
    size_t len = strlen(name);
    event.str_ = std::string(name, len);
    event.type_ = HFP_HF_TYPE_SUBSCRIBER_NUMBER;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfInBandRingCb(const OHOS::bluetooth::RawAddress* bdAddr,
    BthfClientInBandRingState inBand)
{
    HILOGI("[HFP HF]BthfClientInBandRingState=%{public}d", inBand);
    RawAddress rawAddr = *bdAddr;
    HfpHfMessage event(HFP_HF_INTERACTIVE_EVT, inBand);
    event.dev_ = rawAddr.GetAddress();
    event.type_ = HFP_HF_TYPE_INBAND_RING;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    service->PostEvent(event);
}

static void BtHfLastVoiceTagNumberCb(const OHOS::bluetooth::RawAddress* bdAddr,
    const char* number)
{
    HILOGI("[HFP HF]last voice tag number not supported");
}

static void BtHfRingIndicationCb(const OHOS::bluetooth::RawAddress* bdAddr)
{
    /* Ringing is not handled at this indication and rather should be
       implemented (by the client of this service). Use the
       CALL_STATE_INCOMING (and similar) handle ringing. */
}

static void BtHfUnknownEventCb(const OHOS::bluetooth::RawAddress* bdAddr,
    const char* eventString)
{
    HILOGI("[HFP HF]BtHfUnknownEventCb");
}

static BthfClientCallbacks g_sBluetoothHFCallbacks = {
    sizeof(g_sBluetoothHFCallbacks),
    BtHfConnectionStateCb,
    BtHfAudioStateCb,
    BtHfVrCmdCb,
    BtHfNetworkStateCb,
    BtHfNetworkRoamingCb,
    BtHfNetworkSignalCb,
    BtHfBatteryLevelCb,
    BtHfCurrentOperatorCb,
    BtHfCallCb,
    BtHfCallSetupCb,
    BtHfCallHeldCb,
    BtHfRespAndHoldCb,
    BtHfClipCb,
    BtHfCallWaitingCb,
    BtHfCurrentCallsCb,
    BtHfVolumeChangeCb,
    BtHfCmdCompleteCb,
    BtHfSubscriberInfoCb,
    BtHfInBandRingCb,
    BtHfLastVoiceTagNumberCb,
    BtHfRingIndicationCb,
    BtHfUnknownEventCb,
};

void HfpHfService::Enable()
{
    HILOGI("[HFP HF] Enable");
    DoInHfpThread([this]() { this->StartUp(); });
}

BthfClientInterface* HfpHfService::GetBluetoothHfInterface() const
{
    return bluetoothHfInterface;
}

void HfpHfService::Disable()
{
    HILOGI("[HFP HF] Disable");
    DoInHfpThread([this]() { this->ShutDown(); });
}

void HfpHfService::StartUp()
{
    HILOGI("[HFP HF] OnEnable");
    if (isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_HFP_HF, true);
        HILOGW("[HFP HF]HfpHfService has already been started before.");
        return;
    }
    BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("[HFP HF]bluetoothInterface is null.");
        return;
    }

    bluetoothHfInterface = reinterpret_cast<BthfClientInterface*>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_PROFILE_HANDSFREE_CLIENT_ID)));
    if (bluetoothHfInterface == nullptr) {
        HILOGE("[HFP HF]bluetoothHfInterface is null.");
        return;
    }

    BtStackStatus status = bluetoothHfInterface->init(&g_sBluetoothHFCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP HF]Failed to initialize Bluetooth HFP AG, status: %{public}d", status);
        return;
    }

    DelayedSingleton<Telephony::CallManagerClient>::GetInstance()->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    GetContext()->OnEnable(PROFILE_NAME_HFP_HF, true);
    isStarted_ = true;
}

void HfpHfService::ShutDown()
{
    HILOGI("[HFP HF] ShutDown");
    if (!isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_HFP_HF, true);
        HILOGW("[HFP HF]HfpHfService has already been shutdown before.");
        return;
    }

    isShuttingDown_ = true;
    bool isDisconnected = false;
    std::list<std::string> disconnectAddress;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
            if ((it->second != nullptr) && (it->second->GetDeviceState() > HFP_HF_STATE_DISCONNECTED)) {
                disconnectAddress.push_back(it->first);
            }
        }
    }
    for (std::string address : disconnectAddress) {
        Disconnect(RawAddress(address));
        isDisconnected = true;
        HILOGI("[HFP HF]disconnect deviceAddr=%{public}s", GetEncryptAddr(address).c_str());
    }
    if (!isDisconnected) {
        ShutDownDone(true);
    }
}

void HfpHfService::ShutDownDone(bool isAllDisconnected)
{
    HILOGI("[HFP HF] ShutDownDone %{public}d", isAllDisconnected);
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        if (!isAllDisconnected) {
            for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
                if ((it->second != nullptr) && (it->second->GetDeviceState() > HFP_HF_STATE_DISCONNECTED)) {
                    return;
                }
            }
        }
        stateMachines_.clear();
    }
    isStarted_ = false;
    isShuttingDown_ = false;
    GetContext()->OnDisable(PROFILE_NAME_HFP_HF, true);
}

int HfpHfService::GetConnectedDeviceNum()
{
    int size = 0;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto iter = stateMachines_.begin(); iter != stateMachines_.end(); ++iter) {
        if (iter->second != nullptr) {
            auto connectionState = iter->second->GetDeviceState();
            if ((connectionState == HFP_HF_STATE_CONNECTING) || (connectionState >= HFP_HF_STATE_CONNECTED)) {
                size++;
            }
        }
    }
    return size;
}

bool HfpHfService::IsRemoteHfpSupported(const RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    bool ret = (std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HFP_AG)) != uuids.end()) ||
        (std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HSP_AG)) != uuids.end());
    return ret;
}

int HfpHfService::Connect(const RawAddress &device)
{
    HILOGI("[HFP HF]deviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    ConnectStrategyManager::GetInstance()->ProfileConnectionStateChange(address, "hfpService");
    if (!connectionManager->IsBrLinkAllowed(address)) {
        HILOGE("[HFP HF]Failed connect acl link because of MAX links.");
        return Bluetooth::BT_ERR_SERVICE_MAX_CONNECTION;
    }

    int size = GetConnectedDeviceNum();
    if (size >= maxConnectedNum_.load()) {
        HILOGE("[HFP HF]Max connection has reached!");
        return Bluetooth::BT_ERR_SERVICE_MAX_CONNECTION;
    }
    int state = HFP_HF_STATE_DISCONNECTED;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if ((it != stateMachines_.end()) && (it->second != nullptr)) {
            state = it->second->GetDeviceState();
        }
    }

    if ((state >= HFP_HF_STATE_CONNECTED) || (state == HFP_HF_STATE_CONNECTING)) {
        HILOGE("[HFP HF]This device has connected, currentConnectedState=%{public}d", state);
        return Bluetooth::BT_ERR_DEVICE_HAS_CONNECTED;
    }

    if (!IsRemoteHfpSupported(device)) {
        HILOGE("Cannot connect to %{public}s, no headset UUID ", GET_ENCRYPT_STR_ADDR(address));
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }

    HfpHfMessage event(HFP_HF_CONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);

    return BT_NO_ERROR;
}

int HfpHfService::Disconnect(const RawAddress &device)
{
    HILOGI("[HFP HF]DeviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    int slcState = HFP_HF_STATE_DISCONNECTED;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGE("[HFP HF]can't find stateMachine");
            return Bluetooth::BT_ERR_DEVICE_IS_NOT_CONNECTED;
        }

        slcState = it->second->GetDeviceState();
    }

    if ((slcState != HFP_HF_STATE_CONNECTING) && (slcState < HFP_HF_STATE_CONNECTED)) {
        HILOGE("[HFP HF]This device isn't connected, slcState=%{public}d", slcState);
        return Bluetooth::BT_ERR_DEVICE_IS_NOT_CONNECTED;
    }

    HfpHfMessage event(HFP_HF_DISCONNECT_EVT);
    event.dev_ = address;
    PostEvent(event);
    return BT_NO_ERROR;
}

std::list<RawAddress> HfpHfService::GetConnectDevices()
{
    std::list<RawAddress> devList;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if ((it->second != nullptr) && (it->second->GetDeviceState() >= HFP_HF_STATE_CONNECTED)) {
            devList.push_back(RawAddress(it->first));
        }
    }
    return devList;
}

int HfpHfService::GetConnectState()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    uint32_t result = 0;
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else if (it->second->GetDeviceState() >= HFP_HF_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (it->second->GetDeviceState() == HFP_HF_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (it->second->GetDeviceState() == HFP_HF_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (it->second->GetDeviceState() == HFP_HF_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    }
    return static_cast<int>(result);
}

int HfpHfService::GetMaxConnectNum()
{
    return maxConnectedNum_.load();
}

bool HfpHfService::ConnectSco(const RawAddress &device)
{
    HILOGI("[HFP HF]active device=%{public}s", GET_ENCRYPT_ADDR(device));
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        IProfileA2dpSrc *a2dpSrcService = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
        if (a2dpSrcService != nullptr) {
            a2dpSrcService->ForceStopOffloadPlaying(device);
        }
    }
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    int slcState = HFP_HF_STATE_DISCONNECTED;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGE("[HFP HF]can't find stateMachine");
            return false;
        }

        slcState = it->second->GetDeviceState();
    }

    if (slcState != HFP_HF_STATE_CONNECTED) {
        HILOGE("[HFP HF]This device isn't connected, slcState=%{public}d", slcState);
        return false;
    }

    HfpHfMessage event(HFP_HF_CONNECT_AUDIO_EVT);
    event.dev_ = device.GetAddress();
    PostEvent(event);
    return true;
}

bool HfpHfService::DisconnectSco(const RawAddress &device)
{
    HILOGI("[HFP HF]DeviceAddr=%{public}s", GET_ENCRYPT_ADDR(device));
    std::string callingName = PermissionManager::GetCallingName();
    std::string address = device.GetAddress();
    int slcState = HFP_HF_STATE_DISCONNECTED;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGE("[HFP HF]can't find stateMachine");
            return false;
        }

        slcState = it->second->GetDeviceState();
    }

    if ((slcState != HFP_HF_AUDIO_STATE_CONNECTING) && (slcState < HFP_HF_AUDIO_STATE_CONNECTED)) {
        HILOGE("[HFP HF]This device isn't connected, slcState=%{public}d", slcState);
        return false;
    }

    HfpHfMessage event(HFP_HF_DISCONNECT_AUDIO_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

std::vector<RawAddress> HfpHfService::GetDevicesByStates(std::vector<int> states)
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

int HfpHfService::GetDeviceState(const RawAddress &device)
{
    std::string address = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        return stateMap_.at(HFP_HF_STATE_DISCONNECTED);
    }

    if (it->second->GetDeviceState() >= HFP_HF_STATE_CONNECTED) {
        return stateMap_.at(HFP_HF_STATE_CONNECTED);
    } else {
        return stateMap_.at(it->second->GetDeviceState());
    }
}

bool HfpHfService::IsConnected(const std::string &address)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGE("[HFP HF] Invalid Device address:%{public}s", GetEncryptAddr(address).c_str());
        return false;
    }
    if (it->second->GetDeviceState() < HFP_HF_STATE_CONNECTED) {
        HILOGE("[HFP HF] It's not connected!");
        return false;
    }
    return true;
}

int HfpHfService::GetScoState(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    std::string address = device.GetAddress();
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        HILOGW("[HFP HF]The statemachine is not available.");
        return HFP_HF_AUDIO_STATE_DISCONNECTED;
    }

    if (it->second->GetDeviceState() <= HFP_HF_AUDIO_STATE_DISCONNECTED) {
        return HFP_HF_AUDIO_STATE_DISCONNECTED;
    } else {
        return it->second->GetDeviceState();
    }
}

bool HfpHfService::SendDTMFTone(const RawAddress &device, uint8_t code)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }
    HfpHfMessage event(HFP_HF_SEND_DTMF_EVT, code);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

std::vector<HandsFreeUnitCalls> HfpHfService::GetCurrentCallList(const RawAddress &device)
{
    std::vector<HandsFreeUnitCalls> calls;
    std::string address = device.GetAddress();
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(address);
    if (it == stateMachines_.end() || it->second == nullptr) {
        return calls;
    }

    return it->second->GetCurrentCallList();
}

bool HfpHfService::AcceptIncomingCall(const RawAddress &device, int flag)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }
    HfpHfMessage event(HFP_HF_ACCEPT_CALL_EVT, flag);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::HoldActiveCall(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_HOLD_CALL_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::RejectIncomingCall(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_REJECT_CALL_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::SendKeyPressed(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_SEND_KEY_PRESSED);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::HandleIncomingCall(const RawAddress &device, int flag)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_HANDLE_INCOMING_CALL_EVT, flag);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::HandleMultiCall(const RawAddress &device, int flag, int index)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_HANDLE_MULTI_CALL_EVT, flag);
    event.arg3_ = index;
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::DialLastNumber(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_DIAL_LAST_NUMBER);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::DialMemory(const RawAddress &device, int index)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_DIAL_MEMORY, index);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::SendVoiceTag(const RawAddress &device, int index)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_SEND_VOICE_TAG, index);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::FinishActiveCall(const RawAddress &device, const HandsFreeUnitCalls &call)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_FINISH_CALL_EVT);
    event.dev_ = address;
    event.calls_ = call;
    PostEvent(event);
    return true;
}

void HfpHfService::QueryCall(const RawAddress &device, int64_t delayTime)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return;
    }

    HfpHfMessage event(HFP_HF_QUERY_CURRENT_CALLS);
    event.dev_ = address;
    PostDelayEvent(event, delayTime);
}

void HfpHfService::PostEvent(const HfpHfMessage &event)
{
    HILOGI("[HFP HF]PostEvent event_Id=[%{public}d]", event.what_);
    DoInHfThread([this, event]() { this->ProcessEvent(event); });
}

void HfpHfService::PostDelayEvent(const HfpHfMessage &event, int64_t delayTime)
{
    ThreadUtil::GetInstance().PostTask(
        THREAD_ID_HF, [this, event]() { this->ProcessEvent(event); }, delayTime, "HfpHfProcessEvent");
}

void HfpHfService::ProcessEvent(const HfpHfMessage &event)
{
    HILOGI("[HFP HF]Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);
    switch (event.what_) {
        case HFP_HF_CONNECT_EVT:
        case HFP_HF_DISCONNECT_EVT:
        case HFP_HF_CONNECTED_EVT:
        case HFP_HF_DISCONNECTED_EVT:
        case HFP_HF_CONNECTING_EVT:
        case HFP_HF_SLC_ESTABLISHED_EVT:
            ProcessConnectEvent(event);
            break;
        case HFP_HF_SET_VOLUME_EVT:
            SendEventToEachStateMachine(event);
            break;
        default:
            ProcessDefaultEvent(event);
            break;
    }
}

void HfpHfService::ProcessConnectEvent(const HfpHfMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if (it != stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
        // peer device may send connect request before we remove statemachine for last connection.
        // so post this connect request, process it after we remove statemachine completely.
        PostEvent(event);
    } else if (it == stateMachines_.end() || it->second == nullptr) {
        stateMachines_[event.dev_] = std::make_unique<HfpHfStateMachine>(event.dev_);
        stateMachines_[event.dev_]->Init();
        stateMachines_[event.dev_]->ProcessMessage(event);
    } else {
        it->second->ProcessMessage(event);
    }
}

void HfpHfService::ProcessRemoveStateMachine(const std::string &address)
{
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        stateMachines_.insert_or_assign(address, nullptr);
    }
    if (isShuttingDown_) {
        ShutDownDone(false);
    }
}

void HfpHfService::SendEventToEachStateMachine(const HfpHfMessage &event)
{
    HfpHfMessage curEvent = event;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second != nullptr) {
            curEvent.dev_ = it->first;
            it->second->ProcessMessage(curEvent);
        }
    }
}

void HfpHfService::ProcessDefaultEvent(const HfpHfMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if ((it != stateMachines_.end()) && (it->second != nullptr)) {
        it->second->ProcessMessage(event);
    } else {
        HILOGE("[HFP HF]Invalid address[%{public}s]", GET_ENCRYPT_STR_ADDR(event.dev_));
    }
}

std::optional<HandsFreeUnitCalls> HfpHfService::StartDial(const RawAddress &device, const std::string &number)
{
    std::string address = device.GetAddress();
    int state = HFP_HF_STATE_DISCONNECTED;
    bool isInBandRing = false;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(address);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGI("[HFP HF]The state machine is not available!");
        } else {
            state = it->second->GetDeviceState();
            isInBandRing = it->second->IsInbandRing();
        }
    }

    if (!StartDialHook(address, state, number)) {
        return std::nullopt;
    }

    if (state < HFP_HF_STATE_CONNECTED) {
        HILOGI("[HFP HF]It's not connected!");
        return std::nullopt;
    }

    HandsFreeUnitCalls calls(
        address, HFP_HF_OUTGOING_CALL_ID, HFP_CALL_STATE_DIALING, number, false, true, isInBandRing);
    HfpHfMessage event(HFP_HF_DIAL_CALL_EVT);
    event.dev_ = address;
    event.calls_ = calls;
    PostEvent(event);
    return calls;
}

void HfpHfService::RegisterObserver(HfpHfServiceObserver &observer)
{
    observers_.Register(observer);
}

void HfpHfService::DeregisterObserver(HfpHfServiceObserver &observer)
{
    observers_.Deregister(observer);
}

void HfpHfService::SetHfVolume(int volume, int type)
{
    HfpHfMessage event(HFP_HF_SET_VOLUME_EVT, volume);
    event.arg3_ = type;
    PostEvent(event);
    return;
}

bool HfpHfService::OpenVoiceRecognition(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_OPEN_VOICE_RECOGNITION_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

bool HfpHfService::CloseVoiceRecognition(const RawAddress &device)
{
    std::string address = device.GetAddress();
    if (!IsConnected(address)) {
        return false;
    }

    HfpHfMessage event(HFP_HF_CLOSE_VOICE_RECOGNITION_EVT);
    event.dev_ = address;
    PostEvent(event);
    return true;
}

ConnChangeCause HfpHfService::ConvertStateToCause(int state) {
    switch (state) {
        case HFP_HF_STATE_DISCONNECTED:
            return ConnChangeCause::DISCONNECT_CAUSE_USER_DISCONNECT;
        case HFP_HF_STATE_CONNECTING:
            return ConnChangeCause::CONNECT_FROM_LOCAL;
        case HFP_HF_STATE_DISCONNECTING:
            return ConnChangeCause::DISCONNECT_CAUSE_USER_DISCONNECT;
        case HFP_HF_STATE_CONNECTED:
            return ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE;
        default:
            return ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL;
    }
}

void HfpHfService::BatteryLevelChanged(int batteryLevel)
{
    HfpHfMessage event(HFP_HF_BATTERY_LEVEL_CHANGED_EVT, batteryLevel);
    PostEvent(event);
}

void HfpHfService::EnhancedDriverSafety(int state)
{
    HfpHfMessage event(HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT, state);
    PostEvent(event);
}

void HfpHfService::NotifyStateChanged(const RawAddress &device, int state)
{
    observers_.ForEach([device, state, this](HfpHfServiceObserver &observer) {
        observer.OnConnectionStateChanged(device, stateMap_.at(state),
            static_cast<int>(ConvertStateToCause(state)));
    });
    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HFPSTATUS", state);
    if (state == HFP_HF_STATE_CONNECTED) {
        std::string address = device.GetAddress();
        HILOGI("[HFP HF]register call manager callback");
#ifdef BLUETOOTH_WATCH_ENABLE
        int32_t phoneIndex = WatchService::GetInstance()->IsSaisDevice(device.GetAddress()) ? 0 : 1;
        btCallCallbackPtr_ =
            DelayedSingleton<Telephony::CallManagerClient>::GetInstance()->RegisterTwoPhoneCallManagerCallbackPtr(
                phoneIndex, address);
#else
        btCallCallbackPtr_ =
            DelayedSingleton<Telephony::CallManagerClient>::GetInstance()->RegisterBluetoothCallManagerCallbackPtr(
                address);
#endif
        if (btCallCallbackPtr_ == nullptr) {
            HILOGE("[HFP HF]call manager CallCallbackPtr is null.");
        }
        BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HFPCONNECTEDTIME");
    } else if (state == HFP_HF_STATE_DISCONNECTED) {
        std::vector<HandsFreeUnitCalls> callList = GetCurrentCallList(device);
        for (HandsFreeUnitCalls &call : callList) {
            call.SetState(HFP_CALL_STATE_FINISHED);
            ReportCallInfo(call, device);
        }
        BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HFPDISCONNECTTIME");
    }

#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr) {
        watchService->ProcessHfpStateChangeEvt(device, state);
    }
#endif
}

void HfpHfService::NotifyScoStateChanged(const RawAddress &device, int state)
{
    observers_.ForEach([device, state](HfpHfServiceObserver &observer) {
        observer.OnScoStateChanged(device, state);
    });
}

void HfpHfService::NotifyCallChanged(const RawAddress &device, const HandsFreeUnitCalls &call)
{
    observers_.ForEach([device, call](HfpHfServiceObserver &observer) {
        observer.OnCallChanged(device, call);
    });

    ReportCallInfo(call, device);
}

void HfpHfService::ReportCallInfo(const HandsFreeUnitCalls &call, const RawAddress &device)
{
    OHOS::Telephony::CallReportInfo callReportInfo;
    callReportInfo.callType = OHOS::Telephony::CallType::TYPE_BLUETOOTH;
    errno_t ret = strcpy_s(callReportInfo.accountNum, Telephony::kMaxNumberLen, call.GetNumber().c_str());
    if (ret != EOK) {
        HILOGI("[HFP HF] strcpy_s error.");
        return;
    }
    callReportInfo.index = call.GetId();
    if (call.GetState() == HFP_CALL_STATE_FINISHED) {
        callReportInfo.state = Telephony::TelCallState::CALL_STATUS_DISCONNECTED;
    } else {
        callReportInfo.state = static_cast<Telephony::TelCallState>(call.GetState());
    }
    callReportInfo.mpty = static_cast<int32_t>(call.IsMultiParty());
#ifdef BLUETOOTH_WATCH_ENABLE
    callReportInfo.phoneIndex = WatchService::GetInstance()->IsSaisDevice(device.GetAddress()) ? 0 : 1;
#endif
    if (btCallCallbackPtr_ != nullptr) {
        btCallCallbackPtr_->UpdateCallReportInfo(callReportInfo);
    } else {
        HILOGE("[HFP HF]call manager CallCallbackPtr is null.");
    }
}

void HfpHfService::NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel)
{
    observers_.ForEach([device, batteryLevel](HfpHfServiceObserver &observer) {
        observer.OnBatteryLevelChanged(device, batteryLevel);
    });
}

void HfpHfService::NotifySignalStrengthChanged(const RawAddress &device, int signal)
{
    observers_.ForEach([device, signal](HfpHfServiceObserver &observer) {
        observer.OnSignalStrengthChanged(device, signal);
    });
}

void HfpHfService::NotifyRegistrationStatusChanged(const RawAddress &device, int status)
{
    observers_.ForEach([device, status](HfpHfServiceObserver &observer) {
        observer.OnRegistrationStatusChanged(device, status);
    });
}

void HfpHfService::NotifyRoamingStatusChanged(const RawAddress &device, int status)
{
    observers_.ForEach([device, status](HfpHfServiceObserver &observer) {
        observer.OnRoamingStatusChanged(device, status);
    });
}

void HfpHfService::NotifyOperatorSelectionChanged(const RawAddress &device, const std::string &name)
{
    observers_.ForEach([device, name](HfpHfServiceObserver &observer) {
        observer.OnOperatorSelectionChanged(device, name);
    });
}

void HfpHfService::NotifySubscriberNumberChanged(const RawAddress &device, const std::string &number)
{
    observers_.ForEach([device, number](HfpHfServiceObserver &observer) {
        observer.OnSubscriberNumberChanged(device, number);
    });
}

void HfpHfService::NotifyVoiceRecognitionStatusChanged(const RawAddress &device, int status)
{
    observers_.ForEach([device, status](HfpHfServiceObserver &observer) {
        observer.OnVoiceRecognitionStatusChanged(device, status);
    });
}

void HfpHfService::NotifyInBandRingTone(const RawAddress &device, int status)
{
    observers_.ForEach([device, status](HfpHfServiceObserver &observer) {
        observer.OnInBandRingToneChanged(device, status);
    });
}

void HfpHfService::SetAudioParameters()
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second != nullptr) {
            auto audioState = it->second->GetDeviceState();
            if (audioState == HFP_HF_AUDIO_STATE_CONNECTED) {
                it->second->SetAudioConnectedParameters();
            }
        }
    }
}

int HfpHfService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::string callingName = PermissionManager::GetCallingName();
    int deviceState = GetDeviceState(device);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if (deviceState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (deviceState != static_cast<int>(BTConnectState::DISCONNECTED) &&
            deviceState != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
        }
    } else {
        HILOGE("Strategy set failed");
        return BT_ERR_INTERNAL_ERROR;
    }
#ifdef BLUETOOTH_WATCH_ENABLE
    WatchService::GetInstance()->UpdateHFStrategy(device, strategy);
#endif
    return BT_NO_ERROR;
}

int HfpHfService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY);
}
REGISTER_CLASS_CREATOR(HfpHfService);
}  // namespace bluetooth
}  // namespace OHOS
