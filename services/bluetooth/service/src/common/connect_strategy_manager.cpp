/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_conn_strategy"
#endif

#include "connect_strategy_manager.h"

#include "adapter_manager.h"
#include "bluetooth_device.h"
#include "bt_chr_dft_statictics.h"
#include "ipc_skeleton.h"
#include "gavdp/a2dp_service.h"
#include "hfp_ag/hfp_ag_service.h"
#ifdef BLUETOOTH_HEARINGAID_FEATURE_ENABLE
#include "hearing_aid/hearing_aid_service.h"
#endif
#include "interface_profile_manager.h"
#include "profile_config.h"
#include "common_util.h"
#include "bt_chr_dft_exception.h"
#include "hitrace_meter.h"
#include "control_intercept_plugin.h"
#include "bluetooth_connection_manager.h"
#include "common/bluetooth_hw_interface.h"
#include "hw_interop.h"
#include "parameters.h"
#include "bt_chr_ue_manager.h"

namespace OHOS {
namespace bluetooth {
constexpr int32_t RECONNECT_DELAY_MS = 6000;
constexpr int32_t RECONNECT_DELAY_MS_INCOMING = 10000;
constexpr uint8_t AUTOCONNECT_CNT = 0;
constexpr uint8_t AUTOCONNECT_FAIL_CNT = 1;

ConnectStrategyManager* ConnectStrategyManager::GetInstance()
{
    static ConnectStrategyManager instance;
    return &instance;
}

ConnectStrategyManager::ConnectStrategyManager()
{
    remoteDeviceProperties_ = RemoteDeviceProperties::GetInstance();
}

void ConnectStrategyManager::AutoConnect()
{
    HILOGI("auto connection on BT on...");
    std::string autoConnectDeviceAddr = "";
    CHECK_AND_RETURN_LOG(QueryAutoConnectDevice(autoConnectDeviceAddr), "get autoConnectDeviceAddr error.");
    RawAddress device(autoConnectDeviceAddr);
    HILOGI("AutoConnect device: %{public}s", GET_ENCRYPT_ADDR(device));

    ControlInterceptMessage msg {
        .addr = device.GetAddress(),
        // 此处获取蓝牙pid&uid
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedAclConn(msg)) {
        HILOGE("Restricted by control intercept");
        return;
    }

    if (AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        HILOGE("Bluetooth is in restricted mode, can't auto connect");
        return;
    }

    std::string nearlinkReconnDeviceBtAddr = OHOS::system::GetParameter(NEARLINK_RECONN_DEVICE_BT_ADDR, "");
    if (autoConnectDeviceAddr.compare(nearlinkReconnDeviceBtAddr) == 0) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_START_CONN, RawAddress(autoConnectDeviceAddr),
            UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE2);
        HILOGI("Bluetooth AutoConnect to device %{public}s is blocked by Nearlink",
            GetEncryptAddr(autoConnectDeviceAddr).c_str()); // 如果要回连蓝牙设备与星闪回连耳机的蓝牙地址相同则不回连
        return;
    }

    if (BluetoothHwInterface::GetInstance()->InteropMatch(INTEROP_DISABLE_AUTO_CONNECT_DEVICES, device)) {
        HILOGI("AutoConnect device %{public}s is in interop denylist, skip auto connect",
            GET_ENCRYPT_ADDR(device));
        return;
    }

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter, "classicAdapter is nullptr.");
    HILOGI("autoConnect: pagetimeout retry.");
    BtChrAddConnSceneInfo(autoConnectDeviceAddr, PAIR_TYPE_BLUETOOTH_SERVICE, "bluetooth_service", 0);
    if (!IsAutoConnectingAllowed(device)) {
        HILOGI("skipped auto-connect with device %{public}s", GET_ENCRYPT_ADDR(device));
        return;
    }
    classicAdapter->CreateAclConnect(autoConnectDeviceAddr);
    BluetoothConnectionManager::GetInstance()->SetVirtualAutoConnectType(device, CONN_REASON_AUTO_CONNECT_FLAG, 0);
    AutoConnectProfile(autoConnectDeviceAddr);
}

void ConnectStrategyManager::AutoConnectProfile(std::string autoConnectDeviceAddr)
{
    RawAddress device(autoConnectDeviceAddr);
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    A2dpService* a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService* hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    int hfpConnectStatus = Bluetooth::BT_ERR_INVALID_STATE;
    int a2dpConnectStatus = Bluetooth::BT_ERR_INVALID_STATE;
    bool hfpAllowed = false;
    if (hfpAgService) {
        hfpAllowed = hfpAgService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        if (hfpAllowed) {
            HILOGI("AutoConnect hfp");
            hfpConnectStatus = hfpAgService->Connect(device);
        }
    }
    bool a2dpAllowed = false;
    if (a2dpService) {
        a2dpAllowed = a2dpService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        if (a2dpAllowed) {
            HILOGI("AutoConnect a2dp");
            a2dpConnectStatus = a2dpService->Connect(device);
        }
    }
    bool hearingAidAllowed = false;
    int hearingAidConnectStatus = Bluetooth::BT_ERR_INVALID_STATE;
#ifdef BLUETOOTH_HEARINGAID_FEATURE_ENABLE
    HearingAidService* hearingAidService =
        static_cast<HearingAidService *>(serviceMgr->GetProfileService(PROFILE_NAME_HEARING_AID));
    if (hearingAidService) {
        hearingAidAllowed = hearingAidService->GetConnectStrategy(device) ==
            static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        if (hearingAidAllowed) {
            HILOGI("AutoConnect hearing aid");
            hearingAidConnectStatus = hearingAidService->ConnectAllDevice(autoConnectDeviceAddr);
        }
    }
#endif
    bool isHfpResult = IsProfileConnectFail(hfpConnectStatus, hfpAllowed);
    bool isA2dpResult = IsProfileConnectFail(a2dpConnectStatus, a2dpAllowed);
    bool isHearingAidResult = IsProfileConnectFail(hearingAidConnectStatus, hearingAidAllowed);
    ReportAutoConnectResultChr(autoConnectDeviceAddr, isHfpResult, isA2dpResult, isHearingAidResult);
}

bool ConnectStrategyManager::IsAutoConnectingAllowed(const RawAddress &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    bool isAllowed = true;
    if (!remoteDevice) {
        HILOGE("device not exist");
        return true;
    }

    int autoConnSwitchFlag = remoteDevice->GetAutoConnSwitch();
    if (autoConnSwitchFlag != 0) {
        HILOGW("set autoConnect disable, reason: %{public}d, address: %{public}s", autoConnSwitchFlag,
            GET_ENCRYPT_ADDR(device));
        isAllowed = false;
    }
    return isAllowed;
}

void ConnectStrategyManager::ReportAutoConnectResultChr(const std::string& peerAddr,
    bool isHfpResult, bool isA2dpResult, bool isHearingAidResult)
{
    BtChrDftStatictics::GetInstance()->WriteAutoConnectStatictics(AUTOCONNECT_CNT);
    if (isHfpResult || isA2dpResult || isHearingAidResult) {
        BtChrDftStatictics::GetInstance()->WriteAutoConnectStatictics(AUTOCONNECT_FAIL_CNT);
    }
}

bool ConnectStrategyManager::IsProfileConnectFail(int profileStatus, bool profileAllowed)
{
    if (!profileAllowed) {
        return false;
    }

    if (profileStatus == RET_NO_ERROR || profileStatus == Bluetooth::BT_ERR_PROFILE_DISABLED) {
        return false;
    }
    return true;
}

void ConnectStrategyManager::InitConnectStrategy(const std::string &addr)
{
    RawAddress device(addr);
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        HILOGE("device not exist");
        return;
    }
    std::vector<Uuid> uuids = remoteDevice->GetDeviceUuids();
    int connectAllowed = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
    if (IsA2dpNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_A2DP_CONNECTION_POLICY, connectAllowed);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "NEEDA2DP", connectAllowed);
    }
    if (IsHfpNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_HFP_CONNECTION_POLICY, connectAllowed);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "NEEDHFP", connectAllowed);
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (IsHfpHfNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_HFP_CLIENT_CONNECTION_POLICY, connectAllowed);
    }
#endif
    if (IsHidNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_HID_CONNECTION_POLICY, connectAllowed);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "NEEDHID", connectAllowed);
    }
    if (IsHidDeviceNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_HID_DEVICE_CONNECTION_POLICY, connectAllowed);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "NEEDHIDDEVICE", connectAllowed);
    }

    if (IsHearingAidNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_HEARING_AID_CONNECTION_POLICY, connectAllowed);
    }
    if (IsPanNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_PAN_CONNECTION_POLICY, connectAllowed);
    }
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    if (IsBasNeedInit(device, uuids)) {
        SaveConnectStrategy(device, PROPERTY_BAS_CONNECTION_POLICY, connectAllowed);
    }
#endif
}

void ConnectStrategyManager::BtChrWriteConnectStrategy(const std::string &addr,
    const std::string &profileName, const int strategyValue)
{
    static const std::unordered_map<std::string, std::string> profilesSupportMap = {
        {PROPERTY_HFP_CONNECTION_POLICY, "NEEDHFP"},
        {PROPERTY_A2DP_CONNECTION_POLICY, "NEEDA2DP"},
        {PROPERTY_HID_CONNECTION_POLICY, "NEEDHID"},
        {PROPERTY_HID_DEVICE_CONNECTION_POLICY, "NEEDHIDDEVICE"},
    };

    auto it = profilesSupportMap.find(profileName);
    if (it == profilesSupportMap.end()) {
        return;
    }

    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, it->second, strategyValue);
}

int ConnectStrategyManager::QueryConnectStrategy(const RawAddress &device, const std::string &profile)
{
    HILOGD("device: %{public}s, query strategy: %{public}s", GET_ENCRYPT_ADDR(device), profile.c_str());
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = 0;

    if (!config->GetValue(device.GetAddress(), SECTION_CONNECTION_POLICIES, profile, strategyValue)) {
        HILOGI("strategy not found");
        return static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
    }

    HILOGD("strategy: %{public}d", strategyValue);
    if (strategyValue != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    }
    BtChrWriteConnectStrategy(device.GetAddress(), profile, strategyValue);
    return strategyValue;
}

bool ConnectStrategyManager::SaveConnectStrategy(const RawAddress &device, const std::string &profile, int strategy)
{
    HILOGI("device: %{public}s, save strategy", GET_ENCRYPT_ADDR(device));
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (remoteDevice == nullptr || remoteDevice->IsPairNone()) {
        HILOGE("device not exist or not paired.");
        return false;
    }
    int oldStrategy = QueryConnectStrategy(device, profile);
    if (strategy == oldStrategy) {
        HILOGI("new strategy == oldStrategy");
        return true;
    }
    IProfileConfig *config = ProfileConfig::GetInstance();
    int strategyValue = 0;

    switch (BTStrategyType(strategy)) {
        case BTStrategyType::CONNECTION_ALLOWED:
            strategyValue = static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
            break;
        case BTStrategyType::CONNECTION_FORBIDDEN:
            strategyValue = static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
            break;
        default:
            HILOGE("strategy type error.");
            return false;
    }

    return config->SetValue(device.GetAddress(), SECTION_CONNECTION_POLICIES, profile, strategyValue);
}

bool ConnectStrategyManager::PublishAutoConnDevice(std::function<bool(std::string &)> func,
    const std::string &eventType)
{
    // func should be QueryLastActiveDevice or QueryLastConnectedDevice
    // eventType should be "lastActiveDevice" or "lastConnectedDevice"
    std::string device = "";
    bool publishResult = false;
    if (func(device) && IsValidAddr(device)) {
        HILOGI("publish %{public}s, address: %{public}s", eventType.c_str(), GET_ENCRYPT_STR_ADDR(device));
        publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithStringParam(
            BluetoothHelper::COMMON_EVENT_BLUETOOTH_AUTO_CONNECT_DEVICE, eventType, device);
    } else {
        HILOGI("publish clear %{public}s event", eventType.c_str());
        publishResult = BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithStringParam(
            BluetoothHelper::COMMON_EVENT_BLUETOOTH_AUTO_CONNECT_DEVICE, eventType, "00:00:00:00:00:00");
    }
    return publishResult;
}

bool ConnectStrategyManager::SaveLastActiveDevice(const std::string &addr)
{
    HILOGI("SaveLastActiveDevice: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (!config->SetValue(SECTION_AUTO_CONNECT_DEVICE, PROPERTY_LAST_ACTIVE_DEVICE_ADDRESS, addr)) {
        return false;
    }
    PublishAutoConnDevice(QueryLastActiveDevice, "lastActiveDevice");
    return true;
}

bool ConnectStrategyManager::QueryLastActiveDevice(std::string &addr)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    return config->GetValue(SECTION_AUTO_CONNECT_DEVICE, PROPERTY_LAST_ACTIVE_DEVICE_ADDRESS, addr);
}

void ConnectStrategyManager::UpdateAutoConnectDeivce(const std::string &addr)
{
    std::vector<RawAddress> pairdDevices = remoteDeviceProperties_->GetBtPairedDevices();
    // 如果不存在配对设备，清空，下次开关蓝牙不需要回连
    if (pairdDevices.size() == 0) {
        HILOGI("no paired device, clear");
        SaveLastActiveDevice("");
        SaveLastConnectedDevice("");
        return;
    }
    HILOGI("address: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    A2dpService *a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService* hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG((a2dpService != nullptr) && (hfpAgService != nullptr), "a2dp or hfp service null");
    std::string configDeviceAddr = "";
    // 如果当前激活设备取消配对，则更新最新的激活设备到配置文件
    if (QueryLastActiveDevice(configDeviceAddr) && configDeviceAddr == addr) {
        SaveLastActiveDevice("");
        std::string activeDevice = a2dpService->GetActiveSinkDevice().GetAddress();
        if (!IsValidAddr(activeDevice)) {
            activeDevice = hfpAgService->GetActiveDevice();
        }
        if (IsValidAddr(activeDevice) && activeDevice != addr) {
            SaveLastActiveDevice(activeDevice);
        }
    }

    // 如果当前激活设备取消配对，则更新最后连接的设备到配置文件
    if (QueryLastConnectedDevice(configDeviceAddr) && configDeviceAddr == addr) {
        std::string lastConnectDevice = "";
        if (!a2dpService->GetConnectDevices().empty()) {
            lastConnectDevice = a2dpService->GetConnectDevices().back().GetAddress();
        }
        if (!IsValidAddr(lastConnectDevice) && !hfpAgService->GetConnectDevices().empty()) {
            lastConnectDevice = hfpAgService->GetConnectDevices().back().GetAddress();
        }
        if (!IsValidAddr(lastConnectDevice)) {
            lastConnectDevice = pairdDevices.back().GetAddress();
        }
        if (IsValidAddr(lastConnectDevice) && lastConnectDevice != addr) {
            RawAddress device(lastConnectDevice);
            std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
            CHECK_AND_RETURN_LOG(remoteDevice, "device not exist");
            std::vector<Uuid> uuids = remoteDevice->GetDeviceUuids();
            bool isSupportConnectProfile = IsUuidSupport(uuids, BLUETOOTH_UUID_A2DP_SINK) ||
                IsUuidSupport(uuids, ADV_AUDIO_DIST) || IsUuidSupport(uuids, BLUETOOTH_UUID_HFP_HF) ||
                IsUuidSupport(uuids, BLUETOOTH_UUID_HSP_HS) || IsUuidSupport(uuids, BLUETOOTH_UUID_HEARING_AID);
            if (isSupportConnectProfile) {
                SaveLastConnectedDevice(lastConnectDevice);
            }
        }
    }
}

bool ConnectStrategyManager::SaveLastConnectedDevice(const std::string &addr)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
    IProfileConfig *config = ProfileConfig::GetInstance();
    if (!config->SetValue(SECTION_AUTO_CONNECT_DEVICE, PROPERTY_LAST_CONNECTED_DEVICE_ADDRESS, addr)) {
        return false;
    }
    PublishAutoConnDevice(QueryLastConnectedDevice, "lastConnectedDevice");
    return true;
}

bool ConnectStrategyManager::QueryLastConnectedDevice(std::string &addr)
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    return config->GetValue(SECTION_AUTO_CONNECT_DEVICE, PROPERTY_LAST_CONNECTED_DEVICE_ADDRESS, addr);
}

bool ConnectStrategyManager::QueryAutoConnectDevice(std::string &addr)
{
    // 优先回连激活设备，激活设备不存在时，回连最后连接的设备
    if (QueryLastActiveDevice(addr) && IsValidAddr(addr)) {
        return true;
    }
    if (QueryLastConnectedDevice(addr) && IsValidAddr(addr)) {
        return true;
    }
    return false;
}

bool ConnectStrategyManager::IsPanNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return IsUuidSupport(uuids, BLUETOOTH_UUID_PAN) && QueryConnectStrategy(device, PROPERTY_PAN_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

bool ConnectStrategyManager::IsA2dpNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_A2DP_SINK) || IsUuidSupport(uuids, ADV_AUDIO_DIST))
        && QueryConnectStrategy(device, PROPERTY_A2DP_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

bool ConnectStrategyManager::IsHfpNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_HFP_HF) || IsUuidSupport(uuids, BLUETOOTH_UUID_HSP_HS))
        && QueryConnectStrategy(device, PROPERTY_HFP_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

#ifdef BLUETOOTH_HFP_HF_ENABLE
bool ConnectStrategyManager::IsHfpHfNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_HFP_AG))
        && QueryConnectStrategy(device, PROPERTY_HFP_CLIENT_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}
#endif

bool ConnectStrategyManager::IsHidNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_HID_HOST) || IsUuidSupport(uuids, BLUETOOTH_UUID_HOGP))
        && QueryConnectStrategy(device, PROPERTY_HID_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

bool ConnectStrategyManager::IsHidDeviceNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_HID_DEVICE))
        && QueryConnectStrategy(device, PROPERTY_HID_DEVICE_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

bool ConnectStrategyManager::IsHearingAidNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_HEARING_AID))
        && QueryConnectStrategy(device, PROPERTY_HEARING_AID_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}

#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
bool ConnectStrategyManager::IsBasNeedInit(const RawAddress &device, std::vector<Uuid> &uuids)
{
    return (IsUuidSupport(uuids, BLUETOOTH_UUID_BAS))
        && QueryConnectStrategy(device, PROPERTY_BAS_CONNECTION_POLICY)
        == static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
}
#endif

void ConnectStrategyManager::ProfileConnectionStateChange(const std::string &addr, const std::string &profileName)
{
    RawAddress device(addr);
    HILOGI("profileName: %{public}s", profileName.c_str());
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter, "classicAdapter is nullptr.");
    if (!IsProfileNeedDelayConnect(device) && !classicAdapter->IsIncomingConnection(device)) {
        HILOGI("strategy no need to delay connect profile");
        StopProfileDelayConnectTimer(addr);
        StrategyConnectProfile(addr, profileName);
        return;
    }

    std::shared_ptr<utility::Timer> delayTimer = std::make_shared<utility::Timer>(
        [this, addr, profileName] {this->StrategyHandleProfileWaitConnect(addr, profileName);});
    if (delayReconnectTimer != delayTimer) {
        delayReconnectTimer = delayTimer;
        HILOGI("update delayReconnectTimer");
    }
    int32_t reconnectDelayTimer =
        (classicAdapter->IsIncomingConnection(device) == true) ? RECONNECT_DELAY_MS_INCOMING : RECONNECT_DELAY_MS;
    profileDelayConnect_.Insert(addr, delayReconnectTimer);
    int ret = delayReconnectTimer->Start(reconnectDelayTimer);
    HILOGI("strategy start connect delay %{public}d, ret:%{public}d", reconnectDelayTimer, ret);
}

bool ConnectStrategyManager::IsProfileNeedDelayConnect(const RawAddress &device)
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr, false, "serviceMgr null");

    A2dpService *a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService *hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG_RET((a2dpService != nullptr) && (hfpAgService != nullptr), false, "a2dp or hfp service null");

    bool isA2dpNeedConnect =
        (a2dpService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED) &&
            a2dpService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED));
    bool isHfpNeedConnect =
        (hfpAgService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED) &&
            hfpAgService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED));
    bool isProfileNeedConnect = (isA2dpNeedConnect || isHfpNeedConnect);

    auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, false, "bthwif is null");
    bool isRmoteDeviceNeedDelay = !bthwif->hwIsSupportEchoDevice(ServiceUtil::AddrToStack(device));

    return (isRmoteDeviceNeedDelay && isProfileNeedConnect);
}

bool ConnectStrategyManager::StrategyConnectProfile(const std::string addr, const std::string profileName)
{
    if (!IsAllProfilesDisconnect(addr)) {
        HILOGI("This device all profile disconnected, No need to connect to other profiles.");
        return false;
    }

    bool isdevicePreviouslyConnected = false;
    if (DevicePreviouslyConnectedMap_.Find(addr, isdevicePreviouslyConnected) && isdevicePreviouslyConnected == true) {
        HILOGI("This device has connected before.");
        return false;
    }
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr,  false, "serviceMgr null");
    A2dpService *a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService *hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG_RET((a2dpService != nullptr) && (hfpAgService != nullptr), false, "a2dp or hfp service null");

    RawAddress device(addr);
    HILOGI("hfp state:%{public}d, hfp strategy:%{public}d",
        hfpAgService->GetDeviceState(device),
        hfpAgService->GetConnectStrategy(device));
    if (hfpAgService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        hfpAgService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED) &&
        profileName == "a2dpService") {
        int ret = hfpAgService->Connect(device);
        DevicePreviouslyConnectedMap_.EnsureInsert(addr, true);
        HILOGI("StrategyConnectProfile hfp ret:%{public}d", ret);
        return true;
    }

    HILOGI("a2dpState:%{public}d, a2dp strategy:%{public}d",
        a2dpService->GetDeviceState(device),
        a2dpService->GetConnectStrategy(device));
    if (a2dpService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        a2dpService->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED) &&
        profileName == "hfpService") {
        int ret = a2dpService->Connect(device);
        DevicePreviouslyConnectedMap_.EnsureInsert(addr, true);
        HILOGI("StrategyConnectProfile a2dp ret:%{public}d", ret);
        return true;
    }
    return false;
}

void ConnectStrategyManager::StrategyHandleProfileWaitConnect(const std::string addr, const std::string profileName)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (profileDelayConnect_.IsEmpty()) {
        HILOGI("StrategyHandleProfileWaitConnect empty");
        return;
    }

    if (StrategyConnectProfile(addr, profileName)) {
        profileDelayConnect_.Erase(addr);
    }
}

bool ConnectStrategyManager::IsAllProfilesDisconnect(const std::string addr)
{
    RawAddress device(addr);
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr, false, "serviceMgr null");
    A2dpService *a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService *hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    CHECK_AND_RETURN_LOG_RET((a2dpService != nullptr) && (hfpAgService != nullptr), false, "a2dp or hfp service null");

    if (a2dpService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        hfpAgService->GetDeviceState(device) == static_cast<int>(BTConnectState::DISCONNECTED)) {
        return false;
    }
    return true;
}

void ConnectStrategyManager::StopProfileDelayConnectTimer(const std::string addr)
{
    std::shared_ptr<utility::Timer> tempTimer;
    if (!profileDelayConnect_.Find(addr, tempTimer)) {
        return;
    }
    HILOGI("strategy all profile connected remove wait connect");
    tempTimer->Stop();
    profileDelayConnect_.Erase(addr);
}

void ConnectStrategyManager::EraseDevicePreviouslyConnected(const std::string &addr)
{
    DevicePreviouslyConnectedMap_.Erase(addr);
}
}
}
