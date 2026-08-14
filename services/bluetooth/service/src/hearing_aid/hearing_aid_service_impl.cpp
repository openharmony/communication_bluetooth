/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hearing_aid_impl"
#endif

#include "hearing_aid_service_impl.h"
#include "hearing_aid_service.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "log.h"
#include "common_util.h"
#include "hearing_aid_defines.h"
#include "thread_util.h"
#include "connect_strategy_manager.h"
#include "remote_device_properties.h"

#include "audio_general_manager.h"
#include "audio_device_info.h"
#include "bt_chr_transaction_manager.h"


namespace OHOS {
namespace bluetooth {

constexpr int VOLUME_DEFAULT = -64;

static HearingAidServiceImpl *g_serviceImpl = nullptr;

extern "C" HearingAidServiceImplInterface *CreateHearingAidServiceImplInterface(void)
{
    HILOGI("enter");
    if (g_serviceImpl == nullptr) {
        g_serviceImpl = new HearingAidServiceImpl();
    }
    return HearingAidServiceImpl::GetServiceImpl();
}

extern "C" void DestroyHearingAidServiceImplInterface(HearingAidServiceImplInterface *interface)
{
    HILOGI("enter");
    if (interface == nullptr) {
        HILOGE("Interface is nullptr");
        return;
    }
    delete interface;
    interface = nullptr;
}

HearingAidServiceImpl *HearingAidServiceImpl::GetServiceImpl()
{
    return g_serviceImpl;
}

int HearingAidServiceImpl::GetProcessingDeviceNum(HearingAidServiceData &data)
{
    std::list<RawAddress> devList;
    data.stateCache_.Iterator([&devList](const std::string device, const int state) {
        if (state != HEARING_AID_STATE_DISCONNECTED) {
            devList.emplace_back(RawAddress(device));
        }
    });
    return devList.size();
}

std::list<RawAddress> HearingAidServiceImpl::GetConnectDevices(HearingAidServiceData &data)
{
    std::list<RawAddress> devList;
    data.stateCache_.Iterator([&devList](const std::string device, const int state) {
        if (state == HEARING_AID_STATE_CONNECTED) {
            devList.emplace_back(RawAddress(device));
        }
    });
    return devList;
}

void HearingAidServiceImpl::SetGetDeviceNameFunc(std::function<std::string(const RawAddress &device)> func)
{
    getDeviceName_ = func;
}

void HearingAidServiceImpl::SetGetAllowConnectFunc(std::function<bool(const RawAddress &device)> func)
{
    getServiceAllowConnect_ = func;
}

void HearingAidServiceImpl::SetServiceDataFunc(std::function<HearingAidServiceData&()> func)
{
    getServiceData_ = func;
}

void HearingAidServiceImpl::SetServicePosteventFunc(std::function<void(const HearingAidMessage &event)> func)
{
    servicePostFunc_ = func;
}

void HearingAidServiceImpl::SetGetStorageVolumeFunc(std::function<bool(const std::string &addr, int &volume)> func)
{
    serviceGetVolumeFunc_ = func;
}

int HearingAidServiceImpl::SetActiveDevice(const RawAddress &device, HearingAidServiceData &data)
{
    HILOGI("SetActiveDevice: %{public}s", GET_ENCRYPT_ADDR(device));
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    if (data.bluetoothHearingAidInterface_ == nullptr) {
        HILOGE("data.bluetoothHearingAidInterface_ is nullptr");
        return HEARING_AID_FAILURE;
    }
    if (device.GetAddress() != INVALID_MAC_ADDRESS) {
        data.bluetoothHearingAidInterface_->StartSendingAudio(rawAddr);
    }
    uint64_t deviceHiSyncId = HI_SYNC_ID_INVALID;
    data.mDeviceHiSyncIdMap_.Find(device.GetAddress(), deviceHiSyncId);
    // hisyncId相同时，不通知外部
    if (deviceHiSyncId != data.mActiveDeviceHiSyncId_.load()) {
        // 正常情况下协议栈先报device avliable再报connected，一般都能找到
        data.mActiveDeviceHiSyncId_.store(deviceHiSyncId);
        NotifyActiveDeviceChanged(device, data);
    }
    return HEARING_AID_SUCCESS;
}

void HearingAidServiceImpl::ProcessEvent(const HearingAidMessage &event, HearingAidServiceData &data)
{
    HILOGI("process event %{public}s", GetHearingAidEventName(event.what_));
    uint64_t hiSyncId = HI_SYNC_ID_INVALID;
    if (event.msgAddr.length() == 0) {
        HILOGE("no address");
        return;
    }
    switch (event.what_) {
        case HEARING_AID_DISCONNECT_EVT:
        case HEARING_AID_CONNECT_EVT:
        case HEARING_AID_STACK_STATE_EVT:
        case HEARING_AID_DISABLE_EVT:
            ProcessStateMessage(event, data);
            break;
        case HEARING_AID_REMOVE_STATE_MACHINE_EVT:
            ProcessRemoveStateMachine(event.msgAddr, data);
            break;
        case HEARING_AID_SET_VOLUME_BY_ADDR:
            ProcessVolumeSet(event, data);
            break;
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
            ProcessConnectionTimeout(event, data);
            break;
        case HEARING_AID_SET_ACTIVE_DEVICE:
            SetActiveDevice(RawAddress(event.msgAddr), data);
            break;
        default:
            HILOGW("invalid event: %{public}d", event.what_);
            break;
    }
}

void HearingAidServiceImpl::ProcessRemoveStateMachine(const std::string &address, HearingAidServiceData &data)
{
    {
        std::lock_guard<BtRecursiveMutex> lck(data.mutex_);
        data.stateMachines_.insert_or_assign(address, nullptr);
        data.stateCache_.RemoveDevice(address);
    }
    if (data.isShuttingDown_) {
        HearingAidMessage event(HEARING_AID_SERVICE_SHUTDOWN_DONE_EVT);
        event.isAllDisconnected = false;
        servicePostFunc_(event);
    }
}

void HearingAidServiceImpl::ProcessVolumeSet(const HearingAidMessage &event, HearingAidServiceData &data)
{
    std::lock_guard<BtRecursiveMutex> lck(data.mutex_);
    BLUEDROID::RawAddress rawAddr;
    RawAddress addr(event.msgAddr);
    rawAddr = ServiceUtil::AddrToBluedroid(addr);
    auto it = data.stateMachines_.find(addr.GetAddress());
    if (it != data.stateMachines_.end() && it->second != nullptr && !it->second->IsRemoving()) {
        it->second->ProcessMessage(event);
    }
}

void HearingAidServiceImpl::ProcessConnectionTimeout(const HearingAidMessage &event, HearingAidServiceData &data)
{
    HILOGI("process connection timeout message");
    std::lock_guard<BtRecursiveMutex> lck(data.mutex_);
    RawAddress addr(event.msgAddr);
    auto it = data.stateMachines_.find(addr.GetAddress());
    if (it != data.stateMachines_.end() && it->second != nullptr && !it->second->IsRemoving()) {
        it->second->ProcessMessage(event);
    }
}

void HearingAidServiceImpl::ProcessStateMessage(const HearingAidMessage &event, HearingAidServiceData &data)
{
    RawAddress addr(event.msgAddr);
    int state = event.state;
    if (state >= BTHearingAidStateConnectionState::BTHEARING_AID_STATE_MAX ||
        state < BTHearingAidStateConnectionState::BTHEARING_AID_STATE_DISCONNECTED) {
            HILOGE("invalid state");
            return;
    }
    if (event.what_ == HearingAidEvent::HEARING_AID_STACK_STATE_EVT) {
        HILOGI("process stack state event %{public}d", state);
        if (state == BTHearingAidStateConnectionState::BTHEARING_AID_STATE_CONNECTED) {
            // do open hal
            HILOGI("deivce:%{public}s start load hal", GET_ENCRYPT_ADDR(addr));
            data.hdfLoadedDevice_.insert(addr.GetAddress());
            data.heariAidHalLoader_.OpenLib();
        }
        if (state == BTHearingAidStateConnectionState::BTHEARING_AID_STATE_DISCONNECTED) {
            if (std::find(data.hdfLoadedDevice_.begin(), data.hdfLoadedDevice_.end(), addr.GetAddress()) ==
                data.hdfLoadedDevice_.end()) {
                HILOGW("deivce:%{public}s, not load hal, skip unload", GET_ENCRYPT_ADDR(addr));
            } else {
                HILOGI("unload deivce:%{public}s", GET_ENCRYPT_ADDR(addr));
                data.hdfLoadedDevice_.erase(addr.GetAddress());
            }
        }
    }

    if (GetProcessingDeviceNum(data) <= data.maxConnectionsNum_) {
        std::lock_guard<BtRecursiveMutex> lk(data.mutex_);
        auto it = data.stateMachines_.find(addr.GetAddress());
        if (it != data.stateMachines_.end() && it->second != nullptr && it->second->IsRemoving()) {
            // peer device may send connect request before we remove statemachine for last connection.
            // so post this connect request, process it after we remove statemachine completely.
            HILOGW("deivce:%{public}s send connect while statemachine removing", GET_ENCRYPT_ADDR(addr));
            servicePostFunc_(event);
        } else if (it == data.stateMachines_.end() || it->second == nullptr) {
            if (event.what_ == HearingAidEvent::HEARING_AID_DISCONNECT_EVT) {
                HILOGI("no need to process %{public}s disconnect", GET_ENCRYPT_ADDR(addr));
                return;
            }
            data.stateMachines_[addr.GetAddress()] = std::make_unique<HearingAidStateMachine>(addr.GetAddress());
            data.stateMachines_[addr.GetAddress()]->Init();
            data.stateMachines_[addr.GetAddress()]->ProcessMessage(event);
        } else {
            it->second->ProcessMessage(event);
        }
    } else {
        HILOGE("Max connection has reached!");
        HearingAidMessage newEvent(HEARING_AID_DISCONNECT_EVT, addr.GetAddress());
        servicePostFunc_(newEvent);
    }
}

bool HearingAidServiceImpl::GetServiceAllowConnect(const RawAddress &device)
{
    return getServiceAllowConnect_(device);
}

HearingAidServiceData &HearingAidServiceImpl::GetServiceData()
{
    return getServiceData_();
}

void HearingAidServiceImpl::PostEventToService(const HearingAidMessage &event)
{
    servicePostFunc_(event);
}

std::string HearingAidServiceImpl::GetDeviceName(const RawAddress &device)
{
    return getDeviceName_(device);
}

bool HearingAidServiceImpl::IsConnectedPeerDevices(const RawAddress &device, HearingAidServiceData &data)
{
    uint64_t deviceHiSyncId = HI_SYNC_ID_INVALID;
    data.mDeviceHiSyncIdMap_.Find(device.GetAddress(), deviceHiSyncId);
    if (deviceHiSyncId == HI_SYNC_ID_INVALID) {
        return false;
    }
    std::list<RawAddress> connectedDevices = GetConnectDevices(data);
    for (auto it = connectedDevices.begin(); it != connectedDevices.end(); it++) {
        uint64_t connectedDeviceHiSyncId = HI_SYNC_ID_INVALID;
        data.mDeviceHiSyncIdMap_.Find(it->GetAddress(), connectedDeviceHiSyncId);
        if (deviceHiSyncId == connectedDeviceHiSyncId) {
            return true;
        }
    }
    return false;
}

void HearingAidServiceImpl::NotifyStateChanged(
    const RawAddress &device, int fromState, int toState, int reason, HearingAidServiceData &data)
{
    HILOGI("device:%{public}s, fromState:%{public}d, toState:%{public}d, reason:%{public}d", GET_ENCRYPT_ADDR(device),
        fromState, toState, reason);
    if (data.stateMap_.find(fromState) == data.stateMap_.end() ||
        data.stateMap_.find(toState) == data.stateMap_.end()) {
        HILOGE("state invalid!");
        return;
    }
    int btState = data.stateMap_.at(toState);
    data.stateCache_.UpdateDeviceState(device.GetAddress(), toState);
    data.hearingAidObservers_.ForEach([device, btState, reason](IHearingAidObserver &observer) {
        observer.OnConnectionStateChanged(device, btState, reason);
    });

    // 进入disconnected时状态机会把自己移除
    if (fromState == static_cast<int>(HEARING_AID_STATE_DISCONNECTED)) {
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_HEARING_AID_CONNECT,
            TRANSACTION_RESULT_TOTAL);
    }

    if (toState == static_cast<int>(HEARING_AID_STATE_CONNECTED)) {
        int volume;
        if (serviceGetVolumeFunc_(device.GetAddress(), volume)) {
            HearingAidMessage event(HEARING_AID_SET_VOLUME_BY_ADDR, device.GetAddress());
            event.volume = volume;
            servicePostFunc_(event);
        } else {
            HearingAidMessage event(HEARING_AID_SET_VOLUME_BY_ADDR, device.GetAddress());
            event.volume = VOLUME_DEFAULT;
            servicePostFunc_(event);
        }
        SetActiveDevice(device, data);
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_HEARING_AID_CONNECT,
            TRANSACTION_RESULT_SUCCESS);
    } else {
        if (fromState == static_cast<int>(HEARING_AID_STATE_CONNECTING)) {
            BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_HEARING_AID_CONNECT,
                TRANSACTION_RESULT_FAIL, 1, TRANSACTION_SCENECODE_1, 1);
        }
    }

    if (fromState == static_cast<int>(HEARING_AID_STATE_CONNECTED)) {
        if (GetConnectDevices(data).size() == 0) {
            SetActiveDevice(RawAddress(INVALID_MAC_ADDRESS), data);
        } else {
            RawAddress connectedAddr = GetConnectDevices(data).front();
            IsTheSameIdWithActiveDevice(connectedAddr, data) ? (void)0 : (void)SetActiveDevice(connectedAddr, data);
        }
    }
}

constexpr int AUDIO_HEARING_AID = 30;
void HearingAidServiceImpl::NotifyActiveDeviceChanged(const RawAddress &device, HearingAidServiceData &data)
{
    HILOGI("change active device to:%{public}s", GET_ENCRYPT_ADDR(device));

    // 通知音频
    RawAddress audioInformAddress = (device.GetAddress() == INVALID_MAC_ADDRESS ? data.lastNotifyDevice_ : device);
    data.lastNotifyDevice_ = device;
    std::string deviceName = getDeviceName_(device);
#ifdef AUDIO_FRAMEWORK
    auto deviceDesc = std::make_shared<AudioStandard::AudioDeviceDescriptor>();
    deviceDesc->SetDeviceInfo(deviceName, audioInformAddress.GetAddress());
    deviceDesc->deviceType_ = static_cast<AudioStandard::DeviceType>(AUDIO_HEARING_AID);
    deviceDesc->deviceRole_ = AudioStandard::DeviceRole::OUTPUT_DEVICE;
    deviceDesc->deviceCategory_ = AudioStandard::DeviceCategory::BT_HEARAID;
    AudioStandard::DeviceStreamInfo audioStreamInfo = {
        AudioStandard::AudioSamplingRate::SAMPLE_RATE_16000, AudioStandard::ENCODING_PCM,
        AudioStandard::SAMPLE_S16LE, AudioStandard::AudioChannel::STEREO
    };
    int32_t channelMask = 1;
    deviceDesc->SetDeviceCapability(std::list<AudioStandard::DeviceStreamInfo>{audioStreamInfo}, channelMask);
    AudioStandard::AudioGeneralManager::GetInstance()->SetDeviceConnectionStatus(deviceDesc,
        device.GetAddress() != INVALID_MAC_ADDRESS);
#endif
    data.hearingAidObservers_.ForEach([device, deviceName](IHearingAidObserver &observer) {
         observer.OnActiveDeviceChanged(device, deviceName);
    });
}

const char* HearingAidServiceImpl::GetHearingAidEventName(int what)
{
    switch (what) {
        case HEARING_AID_SERVICE_STARTUP_EVT:
            return "HEARING_AID_SERVICE_STARTUP_EVT";
        case HEARING_AID_SERVICE_SHUTDOWN_EVT:
            return "HEARING_AID_SERVICE_SHUTDOWN_EVT";
        case HEARING_AID_SERVICE_SHUTDOWN_DONE_EVT:
            return "HEARING_AID_SERVICE_SHUTDOWN_DONE_EVT";
        case HEARING_AID_DISCONNECT_EVT:
            return "HEARING_AID_DISCONNECT_EVT";
        case HEARING_AID_CONNECT_EVT:
            return "HEARING_AID_CONNECT_EVT";
        case HEARING_AID_STACK_STATE_EVT:
            return "HEARING_AID_STACK_STATE_EVT";
        case HEARING_AID_REMOVE_STATE_MACHINE_EVT:
            return "HEARING_AID_REMOVE_STATE_MACHINE_EVT";
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
            return "HEARING_AID_CONNECT_TIMEOUT_EVT";
        case HEARING_AID_SET_VOLUME_BY_ADDR:
            return "HEARING_AID_SET_VOLUME_BY_ADDR";
        case HEARING_AID_DISABLE_EVT:
            return "HEARING_AID_DISABLE_EVT";
        default:
            return "Unknown";
    }
}

// only used for TDD test case
void HearingAidServiceImpl::SetServiceImpl(HearingAidServiceImpl *serviceImpl)
{
    g_serviceImpl = serviceImpl;
}

bool HearingAidServiceImpl::IsTheSameIdWithActiveDevice(const RawAddress &device, HearingAidServiceData &data)
{
    uint64_t deviceHiSyncId = HI_SYNC_ID_INVALID;
    data.mDeviceHiSyncIdMap_.Find(device.GetAddress(), deviceHiSyncId);
    return (deviceHiSyncId == data.mActiveDeviceHiSyncId_.load());
}

}  // namespace bluetooth
}  // namespace OHOS
