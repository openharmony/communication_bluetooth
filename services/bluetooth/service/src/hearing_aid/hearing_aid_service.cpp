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
#define LOG_TAG "bt_service_hearing_aid_service"
#endif

#include "hearing_aid_service.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "adapter_manager.h"
#include "log.h"
#include "common_util.h"
#include "hearing_aid_defines.h"
#include "thread_util.h"
#include "hitrace_meter.h"

#include "class_creator.h"
#include "connect_strategy_manager.h"
#include "remote_device_properties.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "bluetooth_state_manager.h"
#include "bluetooth_datashare_utils.h"

namespace OHOS {
namespace bluetooth {

constexpr const char *SETTINGS_DATASHARE_URI_HEARING_AID_SWITCH_LEFT =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=left_ear_switch_key";
constexpr const char *SETTINGS_DATASHARE_KEY_HEARING_AID_SWITCH_LEFT = "left_ear_switch_key";
constexpr const char *SETTINGS_DATASHARE_URI_HEARING_AID_SWITCH_RIGHT =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=right_ear_switch_key";
constexpr const char *SETTINGS_DATASHARE_KEY_HEARING_AID_SWITCH_RIGHT = "right_ear_switch_key";
constexpr const char *SETTINGS_DATASHARE_URI_HEARING_AID_VOLUME_LEFT =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=left_ear_slider_value";
constexpr const char *SETTINGS_DATASHARE_KEY_HEARING_AID_VOLUME_LEFT = "left_ear_slider_value";
constexpr const char *SETTINGS_DATASHARE_URI_HEARING_AID_VOLUME_RIGHT =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=right_ear_slider_value";
constexpr const char *SETTINGS_DATASHARE_KEY_HEARING_AID_VOLUME_RIGHT = "right_ear_slider_value";

constexpr int VOLUME_MAX = 100;
constexpr int VOLUME_MIN = 0;
constexpr float VOLUME_COEFFICIENT = 1.28;
constexpr float VOLUME_DB_BASE = 128;

bool HearingAidService::GetVolume(const std::string &addr, int &volume)
{
    bool isLeft;
    if (!IsLeftDevice(isLeft, addr)) {
        HILOGE("device side unknown");
        return false;
    }

    std::string volumePercent = "";
    if (!GetVolumeFromSettingsData(isLeft, volumePercent)) {
        return false;
    }

    if (!ConvertStrToDigit(volumePercent, volume)) {
        HILOGE("volume convert failed:%{public}s", volumePercent.c_str());
        return false;
    }
    if (volume < VOLUME_MIN || volume > VOLUME_MAX) {
        HILOGE("volume invalid:%{public}d", volume);
        return false;
    }
    volume = VOLUME_COEFFICIENT * volume - VOLUME_DB_BASE;
    return true;
}

bool HearingAidService::GetVolumeFromSettingsData(bool isLeft, std::string& volumePercent)
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGE("settingHelper is null");
        return false;
    }

    std::string hearingAidSwitch = "";
    const char * const uriSwitchStr = isLeft ? SETTINGS_DATASHARE_URI_HEARING_AID_SWITCH_LEFT :
                                               SETTINGS_DATASHARE_URI_HEARING_AID_SWITCH_RIGHT;
    const char * const keySwitchStr = isLeft ? SETTINGS_DATASHARE_KEY_HEARING_AID_SWITCH_LEFT :
                                               SETTINGS_DATASHARE_KEY_HEARING_AID_SWITCH_RIGHT;
    Uri uriSwitch(uriSwitchStr);
    if (dataShareHelperUtils->Query(uriSwitch, keySwitchStr, hearingAidSwitch) == RET_NO_ERROR &&
        hearingAidSwitch == "0") {
        HILOGI("%{public}s switch turned off", (isLeft ? "left" : "right"));
        volumePercent = "0";
        return true;
    }

    const char * const uriVolumeStr = isLeft ? SETTINGS_DATASHARE_URI_HEARING_AID_VOLUME_LEFT :
                                               SETTINGS_DATASHARE_URI_HEARING_AID_VOLUME_RIGHT;
    const char * const keyVolumeStr = isLeft ? SETTINGS_DATASHARE_KEY_HEARING_AID_VOLUME_LEFT :
                                               SETTINGS_DATASHARE_KEY_HEARING_AID_VOLUME_RIGHT;
    Uri uriVolume(uriVolumeStr);
    if (dataShareHelperUtils->Query(uriVolume, keyVolumeStr, volumePercent) != RET_NO_ERROR) {
        HILOGE("%{public}s volume not found", (isLeft ? "left" : "right"));
        return false;
    }

    return true;
}

HearingAidService::HearingAidService() : utility::Context(PROFILE_NAME_HEARING_AID, "1.0"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{
    HILOGI("hearing aid service create");
}

HearingAidService::~HearingAidService() {}

utility::Context *HearingAidService::GetContext()
{
    return this;
}

HearingAidService *HearingAidService::GetService()
{
    HILOGD("hearing aid GetService enter");
    auto servManager = IProfileManager::GetInstance();
    if (servManager == nullptr) {
        HILOGI("servManager is nullptr.");
        return nullptr;
    }
    return static_cast<HearingAidService *>(servManager->GetProfileService(PROFILE_NAME_HEARING_AID));
}

int HearingAidService::Connect(const RawAddress &device)
{
    HILOGI("hearing aid connect to %{public}s", GET_ENCRYPT_ADDR(device));
    if (!AllowConnect(device)) {
        HILOGE("reject connect to %{public}s", GET_ENCRYPT_ADDR(device));
        return HEARING_AID_FAILURE;
    }
    uint64_t hiSyncId = HI_SYNC_ID_INVALID;
    serviceData_.mDeviceHiSyncIdMap_.Find(device.GetAddress(), hiSyncId);
    if (hiSyncId != serviceData_.mActiveDeviceHiSyncId_.load() &&
        hiSyncId != HI_SYNC_ID_INVALID &&
        serviceData_.mActiveDeviceHiSyncId_.load() != HI_SYNC_ID_INVALID) {
        HILOGW("another device connected, disconnect connected devices");
        for (RawAddress addr : GetConnectDevices()) {
            HearingAidMessage event(HEARING_AID_DISCONNECT_EVT, addr.GetAddress());
            PostEvent(event);
        }
    }
    HearingAidMessage event(HEARING_AID_CONNECT_EVT, device.GetAddress());
    PostEvent(event);
    return HEARING_AID_SUCCESS;
}

int HearingAidService::ConnectAllDevice(const std::string &address)
{
    uint64_t deviceHiSyncId;
    if (!serviceData_.mDeviceHiSyncIdMap_.Find(address, deviceHiSyncId)) {
        HILOGE("hearing aid device not found");
        return HEARING_AID_FAILURE;
    }
    std::vector<std::string> pairedDevices;
    serviceData_.mDeviceHiSyncIdMap_.Iterate([&pairedDevices, deviceHiSyncId](std::string addr, uint64_t hiSyncId) {
        if (hiSyncId == deviceHiSyncId) {
            pairedDevices.emplace_back(addr);
        }
    });
    HILOGI("found pairedDevices size %{public}d", pairedDevices.size());
    int result = 0;
    for (const auto& addr : pairedDevices) {
        RawAddress device(addr);
        int ret = Connect(device);
        if (ret != HEARING_AID_SUCCESS) {
            HILOGE("Failed to connect device: %{public}s", GET_ENCRYPT_ADDR(device));
            result++;
        }
    }
    if (result > 0) {
        return HEARING_AID_FAILURE;
    } else {
        return HEARING_AID_SUCCESS;
    }
}

void HearingAidService::AddHearingAidDeviceInfo(const RawAddress &device, uint8_t capabilities, uint64_t hiSyncId)
{
    serviceData_.mDeviceCapabilitiesMap_.EnsureInsert(device.GetAddress(), capabilities);
    serviceData_.mDeviceHiSyncIdMap_.EnsureInsert(device.GetAddress(), hiSyncId);
}

bool HearingAidService::IsLeftDevice(bool &isLeft, const std::string &addr)
{
    uint8_t capabilities;
    if (serviceData_.mDeviceCapabilitiesMap_.Find(addr, capabilities)) {
        isLeft = ((capabilities & 1) == 0);
        return true;
    }
    return false;
}

bool HearingAidService::IsLe2MPhySupported()
{
    return AdapterProperties::GetInstance()->GetLe2mPhySupported();
}

int HearingAidService::Disconnect(const RawAddress &device)
{
    return DisconnectImpl(device, HEARING_AID_DISCONNECT_EVT);
}

int HearingAidService::DisconnectImpl(const RawAddress &device, const int eventAction)
{
    uint64_t deviceHiSyncId = HI_SYNC_ID_INVALID;
    serviceData_.mDeviceHiSyncIdMap_.Find(device.GetAddress(), deviceHiSyncId);
    if (deviceHiSyncId != HI_SYNC_ID_INVALID) {
        serviceData_.mDeviceHiSyncIdMap_.Iterate([this, deviceHiSyncId, device,
            eventAction](std::string addr, uint64_t hiSyncId) {
            if (addr == device.GetAddress()) {
                return;
            }
            if (hiSyncId == deviceHiSyncId) {
                HearingAidMessage event(eventAction, addr);
                PostEvent(event);
            }
        });
    }
    HearingAidMessage event(eventAction, device.GetAddress());
    PostEvent(event);
    return HEARING_AID_SUCCESS;
}

std::list<RawAddress> HearingAidService::GetConnectDevices(void)
{
    std::list<RawAddress> devList;
    serviceData_.stateCache_.Iterator([&devList](const std::string device, const int state) {
        if (state != HEARING_AID_STATE_DISCONNECTED) {
            devList.emplace_back(RawAddress(device));
        }
    });
    return devList;
}

bool HearingAidService::AllowConnect(const RawAddress &device)
{
    int strategy = GetConnectStrategy(device);
    int bondState = RemoteDeviceProperties::GetInstance()->GetPairState(device);
    if (bondState != PAIR_PAIRED) {
        HILOGW("device not bond, not allow connect");
        return false;
    }
    if (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        HILOGW("device strategy is %{public}d, not allow connect", strategy);
        return false;
    }
    return true;
}

int HearingAidService::GetConnectState(void)
{
    uint8_t result = 0;
    serviceData_.stateCache_.Iterator([&result](const std::string device, const int state) {
        if (state == HEARING_AID_STATE_CONNECTED) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (state == HEARING_AID_STATE_CONNECTING) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (state == HEARING_AID_STATE_DISCONNECTING) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (state == HEARING_AID_STATE_DISCONNECTED) {
            result |= PROFILE_STATE_DISCONNECTED;
        }
    });
    return static_cast<int>(result);
}

int HearingAidService::GetMaxConnectNum(void)
{
    return serviceData_.maxConnectionsNum_;
}

std::vector<RawAddress> HearingAidService::GetDevicesByStates(std::vector<int> &states)
{
    std::vector<RawAddress> devices;
    serviceData_.stateCache_.Iterator([&states, &devices](const std::string device, const int state) {
        if (std::find(states.begin(), states.end(), state) != states.end()) {
            devices.emplace_back(RawAddress(device));
        }
    });
    return devices;
}

int HearingAidService::GetDeviceState(const RawAddress &device)
{
    int state = serviceData_.stateCache_.GetDeviceStateCache(device.GetAddress());
    HILOGI("%{public}s state is %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), state);
    if (state > HEARING_AID_STATE_DISCONNECTING) {
        // invalid state return disconnected
        return serviceData_.stateMap_.at(HEARING_AID_STATE_DISCONNECTED);
    } else {
        return serviceData_.stateMap_.at(state);
    }
}

int HearingAidService::SetActiveDevice(const RawAddress &device)
{
    HearingAidMessage event(HEARING_AID_SET_ACTIVE_DEVICE, device.GetAddress());
    PostEvent(event);
    return HEARING_AID_SUCCESS;
}

void HearingAidService::GetActiveDevice(RawAddress &addr, std::string &name)
{
    // 返回找到的第一个hiSyncId符合active的已连接设备
    if (serviceData_.mActiveDeviceHiSyncId_.load() == HI_SYNC_ID_INVALID) {
        addr = RawAddress(INVALID_MAC_ADDRESS);
        return;
    }
    std::string result = INVALID_MAC_ADDRESS;
    serviceData_.mDeviceHiSyncIdMap_.Iterate([this, &result](std::string addr, uint64_t hiSyncId) {
        if (this->GetDeviceState(RawAddress(addr)) != HEARING_AID_STATE_CONNECTED) {
            return;
        }
        if (hiSyncId == this->serviceData_.mActiveDeviceHiSyncId_.load()) {
            result = addr;
            return;
        }
    });
    addr = RawAddress(result);
    if (result != INVALID_MAC_ADDRESS) {
        name = RemoteDeviceProperties::GetInstance()->GetDeviceName(addr);
    }
}

int HearingAidService::SetVolumeByAddr(const RawAddress &device, int volume)
{
    HearingAidMessage event(HEARING_AID_SET_VOLUME_BY_ADDR, device.GetAddress());
    event.volume = volume;
    PostEvent(event);
    return HEARING_AID_SUCCESS;
}

int HearingAidService::GetHearingAidDeviceInfo(const RawAddress &device, int &side, int mode, int64_t &hiSyncId)
{
    uint8_t capabilities;
    uint64_t deviceHiSyncId;
    if (serviceData_.mDeviceCapabilitiesMap_.Find(device.GetAddress(), capabilities) &&
        serviceData_.mDeviceHiSyncIdMap_.Find(device.GetAddress(), deviceHiSyncId)) {
        side = capabilities & 1;
        mode = (capabilities >> 1) & 1;
        hiSyncId = static_cast<int64_t>(deviceHiSyncId);
        return HEARING_AID_SUCCESS;
    }
    HILOGE("%{public}s info not found", GetEncryptAddr(device.GetAddress()).c_str());
    return HEARING_AID_FAILURE;
}

int HearingAidService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("SetConnectStrategy device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HEARING_AID_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if ((GetDeviceState(device)) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect hearing aid.");
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if ((GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTED) &&
            (GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
            HILOGI("disconnect hearing aid.");
        }
    } else {
        HILOGE("SetConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return Bluetooth::BT_NO_ERROR;
}

int HearingAidService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HEARING_AID_CONNECTION_POLICY);
}

void HearingAidService::RegisterObserver(IHearingAidObserver &hearingAidObserver)
{
    serviceData_.hearingAidObservers_.Register(hearingAidObserver);
}

void HearingAidService::DeregisterObserver(IHearingAidObserver &hearingAidObserver)
{
    serviceData_.hearingAidObservers_.Deregister(hearingAidObserver);
}

void HearingAidService::Enable(void)
{
    HearingAidMessage event(HEARING_AID_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void HearingAidService::Disable(void)
{
    HearingAidMessage event(HEARING_AID_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

::bluetooth::hearing_aid::HearingAidInterface* HearingAidService::getBluetoothHearingAidInterface() const
{
    return serviceData_.bluetoothHearingAidInterface_;
}

void HearingAidService::RemoveStateMachine(const std::string &device)
{
    HearingAidMessage event(HEARING_AID_REMOVE_STATE_MACHINE_EVT, device);
    PostEvent(event);
}

void HearingAidService::PostEvent(const HearingAidMessage &event)
{
    DoInHearingAidThread([this, event] {this->ProcessEvent(event);});
}

void HearingAidService::ProcessEvent(const HearingAidMessage &event)
{
    uint64_t hiSyncId = HI_SYNC_ID_INVALID;
    switch (event.what_) {
        case HEARING_AID_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case HEARING_AID_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case HEARING_AID_SERVICE_SHUTDOWN_DONE_EVT:
            ShutDownDone(event.arg1_);
            break;
        case HEARING_AID_DISCONNECT_EVT:
        case HEARING_AID_CONNECT_EVT:
        case HEARING_AID_STACK_STATE_EVT:
        case HEARING_AID_REMOVE_STATE_MACHINE_EVT:
        case HEARING_AID_SET_VOLUME_BY_ADDR:
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
        case HEARING_AID_SET_ACTIVE_DEVICE:
        case HEARING_AID_DISABLE_EVT:
            if (serviceImpl_ == nullptr) {
                LoadServiceImpl("ProcessEvent");
            }
            if (serviceImpl_ != nullptr) {
                serviceImpl_->ProcessEvent(event, serviceData_);
            }
            break;
        default:
            HILOGW("invalid event: %{public}d", event.what_);
            break;
    }
}

void HearingAidService::StartUp()
{
    HILOGI("[HEARING_AID Service]==========<start>==========");
    if (serviceData_.isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_HEARING_AID, true);
        HILOGW("[HEARING_AID Service]:HearingAidService has already been started before.");
        return;
    }

    bt_interface_t* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("Start up failed, bluetoothInterface is null.");
        return;
    }
    serviceData_.bluetoothHearingAidInterface_ = reinterpret_cast<::bluetooth::hearing_aid::HearingAidInterface*>(
        const_cast<void *>(bluetoothInterface->get_profile_interface(BT_PROFILE_HEARING_AID_ID)));
    if (serviceData_.bluetoothHearingAidInterface_ == nullptr) {
        HILOGE("Start up failed, bluetoothHearingAidInterface is null.");
        return;
    }
    if (!AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        HILOGI("Non-half-closed state, initialize protocol stack.");
        StartStackProfile();
    }

    HILOGI("[HEARING_AID Service]==========<start success>==========");
    GetContext()->OnEnable(PROFILE_NAME_HEARING_AID, true);
    serviceData_.isStarted_ = true;
}

void HearingAidService::ShutDown()
{
    if (!serviceData_.isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_HEARING_AID, true);
        HILOGW("HearingAidService has already been shutdown before.");
        return;
    }

    serviceData_.isShuttingDown_ = true;
    bool isDisconnected = false;
    serviceData_.stateCache_.Iterator([&](const std::string device, const int state) {
        if (std::find(serviceData_.hdfLoadedDevice_.begin(), serviceData_.hdfLoadedDevice_.end(), device) !=
            serviceData_.hdfLoadedDevice_.end()) {
            serviceData_.hdfLoadedDevice_.erase(device);
            DisconnectImpl(RawAddress(device), HEARING_AID_DISABLE_EVT);
            isDisconnected = true;
        }
    });

    if (!isDisconnected) {
        ShutDownDone(true);
    }
}

void HearingAidService::ShutDownDone(bool isAllDisconnected)
{
    {
        std::lock_guard<BtRecursiveMutex> lk(serviceData_.mutex_);
        if (!isAllDisconnected) {
            bool anyDeviceConnected = false;
            serviceData_.stateCache_.Iterator([&anyDeviceConnected](const std::string device, const int state) {
                if (state > HEARING_AID_STATE_DISCONNECTED) {
                    anyDeviceConnected = true;
                }
            });
            if (anyDeviceConnected) {
                return;
            }
        }
        serviceData_.stateMachines_.clear();
    }

    if (serviceImpl_ != nullptr) {
        serviceImpl_->SetActiveDevice(RawAddress(INVALID_MAC_ADDRESS), serviceData_);
    }

    serviceData_.isShuttingDown_ = false;
    serviceData_.isStarted_ = false;

    std::lock_guard<std::mutex> lock(initMutex_);
    if (serviceData_.bluetoothHearingAidInterface_ != nullptr && serviceData_.isInit_) {
        serviceData_.bluetoothHearingAidInterface_->Cleanup();
        serviceData_.bluetoothHearingAidInterface_ = nullptr;
        serviceData_.isInit_ = false;
    }

    GetContext()->OnDisable(PROFILE_NAME_HEARING_AID, true);
    HILOGI("[HEARING_AID Service]HearingAidService shutdown");
}

void HearingAidService::LoadServiceImpl(const char *func)
{
    if (serviceImpl_ != nullptr) {
        return;
    }
    HILOGI("%{public}s load service impl", func);
    loader_.OpenLib();
    serviceImpl_ = loader_.GetLibInstance();
    if (serviceImpl_ == nullptr) {
        HILOGE("load service impl failed!");
        return;
    }
    serviceImpl_->SetServicePosteventFunc([this](const HearingAidMessage&event) { this->PostEvent(event); });
    serviceImpl_->SetGetDeviceNameFunc([](const RawAddress audioInformAddress) -> std::string {
        return RemoteDeviceProperties::GetInstance()->GetDeviceName(audioInformAddress);
    });
    serviceImpl_->SetGetAllowConnectFunc(
        [this](const RawAddress &device) -> bool { return this->AllowConnect(device); });
    serviceImpl_->SetGetStorageVolumeFunc([this](const std::string &addr, int &volume) -> bool {
        return HearingAidService::GetVolume(addr, volume);
    });
    serviceImpl_->SetServiceDataFunc([this]() -> HearingAidServiceData& { return this->serviceData_; });

    CHECK_AND_RETURN_LOG(serviceImpl_, "Get hearing aid service impl failed");
    HILOGI("Hearing aid service impl loaded");
}

void HearingAidService::StartStackProfile()
{
    std::lock_guard<std::mutex> lock(initMutex_);
    if (serviceData_.bluetoothHearingAidInterface_ != nullptr && !serviceData_.isInit_) {
        bluetoothHearingAidCallbacks_ = std::make_unique<OHHearingAidCallbacks>();
        serviceData_.bluetoothHearingAidInterface_->Init(bluetoothHearingAidCallbacks_.get());
        serviceData_.isInit_ = true;
    }
}

void OHHearingAidCallbacks::OnConnectionState(::bluetooth::hearing_aid::ConnectionState state,
    const BLUEDROID::RawAddress& address)
{
    HILOGI("[HEARING_AID Service] stack state=%{public}d, addr = %{public}s",
        state, GetEncryptAddr(address.ToString()).c_str());
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(address);

    if (static_cast<int>(state) < 0 || static_cast<int>(state) >= BTHEARING_AID_STATE_MAX) {
        HILOGE("[HEARING_AID Service] state invalid!");
        return;
    } else if (static_cast<int>(state) == BTHEARING_AID_STATE_CONNECTED) {
        ConnectStrategyManager::GetInstance()->SaveLastConnectedDevice(rawAddr.GetAddress());
        ConnectStrategyManager::GetInstance()->SaveLastActiveDevice(rawAddr.GetAddress());
    }
    HearingAidMessage event(HEARING_AID_STACK_STATE_EVT, rawAddr.GetAddress(), static_cast<int>(state));
    HearingAidService *service = HearingAidService::GetService();
    if (service == nullptr) {
        HILOGE("service is null!");
        return;
    }
    service->PostEvent(event);
}

void OHHearingAidCallbacks::OnDeviceAvailable(uint8_t capabilities, uint64_t hiSyncId,
    const BLUEDROID::RawAddress &address)
{
    HILOGI("[HEARING_AID Service] stack device avliable %{public}s", GetEncryptAddr(address.ToString()).c_str());
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(address);
    HearingAidService *service = HearingAidService::GetService();
    if (service == nullptr) {
        HILOGE("service is null!");
        return;
    }
    service->AddHearingAidDeviceInfo(rawAddr, capabilities, hiSyncId);
}


REGISTER_CLASS_CREATOR(HearingAidService);
}  // namespace bluetooth
}  // namespace OHOS
