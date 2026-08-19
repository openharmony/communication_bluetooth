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
#define LOG_TAG "bt_service_classic_adapter"
#endif

#include "classic_adapter.h"

#include <algorithm>
#include <sys/time.h>
#include <unistd.h>
#include "adapter_manager.h"
#include "base_observer_list.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_common_event_helper.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "class_creator.h"
#include "classic_defs.h"
#include "connect_strategy_manager.h"
#include "dialog_pair.h"
#include "common/bluetooth_hw_interface.h"
#include "hal_util.h"
#include "hw_bt_hwif.h"
#include "permission_manager.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"

#include "compat.h"
#include "remote_device_properties.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "common_util.h"
#include "securec.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "parameter.h"
#include "thread_util.h"
#include "avrcp_tg_service_manager.h"
#include "preferences_manager.h"
#include "refuse_play_helper.h"
#include "log_utils.h"
#include "directory_ex.h"
#include "cloud_device_manager.h"
#include "hfp_ag_service.h"
#include "bt_func_hook.h"
#include "bt_chr_car_key.h"
#include "profile_info.h"

#include "control_intercept_plugin.h"
#include "bluetooth_airplane_manager.h"
#ifdef BLUETOOTH_FASTSCAN_ENABLE
#include "bluetooth_fastscan_manager_loader.h"
#endif
#include "../sdp_adapter/sdp_adapter.h"

#define BLUETOOTH_PIN_NO_PASSKEY (0)

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
constexpr int32_t UID_AUDIO = 1041; // Depends on OHOS::AudioStandard::UID_AUDIO
const uint32_t DEVICE_MODEL_ID_LEN = 6;
const uint32_t DEVICE_SUBMODEL_ID_LEN = 2;
const uint32_t DEVICE_ICON_ID_LEN = 4;
const uint32_t DELAY_REMOVE_PAIR  = 1000;
const int DEVICE_DISCONNECTED_STATE = 3;
const std::map<DeviceType, int> customTypeToCod = {
    {DeviceType::DEVICE_TYPE_CAR, BluetoothDevice::AUDIO_VIDEO_CAR_AUDIO},
    {DeviceType::DEVICE_TYPE_HEADSET, BluetoothDevice::AUDIO_VIDEO_HEADPHONES},
    {DeviceType::DEVICE_TYPE_HEARING, BluetoothDevice::AUDIO_VIDEO_HEADPHONES},
    {DeviceType::DEVICE_TYPE_GLASSES, BluetoothDevice::WEARABLE_GLASSES},
    {DeviceType::DEVICE_TYPE_WATCH, BluetoothDevice::WEARABLE_WRIST_WATCH},
    {DeviceType::DEVICE_TYPE_SPEAKER, BluetoothDevice::AUDIO_VIDEO_LOUDSPEAKER},
};
const std::map<int, DeviceType> codToCustomType = {
    {BluetoothDevice::AUDIO_VIDEO_CAR_AUDIO, DeviceType::DEVICE_TYPE_CAR},
    {BluetoothDevice::AUDIO_VIDEO_HANDSFREE, DeviceType::DEVICE_TYPE_CAR},
    {BluetoothDevice::MAJOR_AUDIO_VIDEO_HEADPHONES, DeviceType::DEVICE_TYPE_HEADSET},
    {BluetoothDevice::AUDIO_VIDEO_WEARABLE_HEADSET, DeviceType::DEVICE_TYPE_HEADSET},
    {BluetoothDevice::WEARABLE_GLASSES, DeviceType::DEVICE_TYPE_GLASSES},
    {BluetoothDevice::MAJOR_WEARABLE_WRIST_WATCH, DeviceType::DEVICE_TYPE_WATCH},
    {BluetoothDevice::AUDIO_VIDEO_LOUDSPEAKER, DeviceType::DEVICE_TYPE_SPEAKER},
};
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";

struct ClassicAdapter::impl {
    BtRecursiveMutex pendingDeviceMutex_ = {};
    BtRecursiveMutex disconnectAclBeforeBondDeviceMutex_ = {};
    BaseObserverList<IAdapterClassicObserver> adapterObservers_ {};
    BaseObserverList<IClassicRemoteDeviceObserver> remoteObservers_ {};
    std::atomic_int startDiscoveryPid_ = 0;
    std::atomic_int startDiscoveryUid_ = 0;
};

template<typename T>
BTConnectState GetDeviceState(T* profile, const RawAddress &device)
{
    if (!profile) {
        return BTConnectState::DISCONNECTED;
    }
    return static_cast<BTConnectState>(profile->GetDeviceState(device));
}

ClassicAdapter::ClassicAdapter()
    : utility::Context(ADAPTER_NAME_CLASSIC, "5.0"),
    adapterProperties_(AdapterProperties::GetInstance()),
    remoteDeviceProperties_(RemoteDeviceProperties::GetInstance()),
    pimpl(std::make_unique<ClassicAdapter::impl>())
{
    HILOGI("%{public}s Constructor", Name().c_str());
    discoveryState_ = DISCOVERY_STOPED;
    scanMode_ = SCAN_MODE_NONE;
}

ClassicAdapter::~ClassicAdapter()
{
    HILOGI("%{public}s Destructor", Name().c_str());
}

void ClassicAdapter::Enable()
{
    HILOGI("%{public}s Classic Adapter Enable", Name().c_str());
    DoInClassicThread([this] {this->StartUp();});
}

void ClassicAdapter::StartUp()
{
    adapterProperties_->LoadConfigInfo();
    LoadPairedDeviceInfo();
    discoveryState_ = DISCOVERY_STOPED;
#ifdef BT_USE_OPEN_STACK
    // Open stack does not auto-apply scan mode after classic enable.
    constexpr int OPEN_STACK_SCAN_DURATION_MS = 120000;
    SetBtScanMode(SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, OPEN_STACK_SCAN_DURATION_MS);
#endif
    GetContext()->OnEnable(ADAPTER_NAME_CLASSIC, true);
}

void ClassicAdapter::Disable()
{
    HILOGI("%{public}s Disable", Name().c_str());
    DoInClassicThread([this] {this->ShutDown();});
}

void ClassicAdapter::ShutDown()
{
    adapterProperties_->UnregisterNameChangeObserver();
    GetContext()->OnDisable(ADAPTER_NAME_CLASSIC, true);
#ifdef BLUETOOTH_HFP_HF_ENABLE
    hfService_ = nullptr;
#endif
    agService_ = nullptr;
    a2dpSrcService_ = nullptr;
    hidHostService_ = nullptr;
    pbapService_ = nullptr;
    mapService_ = nullptr;
    panService_ = nullptr;
    basService_ = nullptr;
    if (IsBtDiscovering()) {
        CancelBtDiscovery();
    }
    remoteDeviceProperties_->FreeMemory();
}

void ClassicAdapter::PostEnable()
{
    DoInClassicThread([this] {this->ProcessPostEnable();});
}

void ClassicAdapter::ProcessPostEnable()
{
    agService_ = static_cast<IProfileHfpAg *>(IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_AG));
#ifdef BLUETOOTH_HFP_HF_ENABLE
    hfService_ = static_cast<IProfileHfpHf *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HFP_HF));
#endif
    a2dpSrcService_ = static_cast<IProfileA2dp *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC));
    hidHostService_ = static_cast<IProfileHidHost *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_HOST));
    pbapService_ = static_cast<IProfilePbapPse *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_PBAP_PSE));
    mapService_ = static_cast<IProfileMapMse *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_MAP_MSE));
    hearingAidService_ = static_cast<IProfileHearingAid *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HEARING_AID));
    panService_ = static_cast<IProfilePan *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_PAN));
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    basService_ = static_cast<IProfileBas *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_BAS));
#endif
}

bool ClassicAdapter::RegisterClassicAdapterObserver(IAdapterClassicObserver &observer) const
{
    return pimpl->adapterObservers_.Register(observer);
}

bool ClassicAdapter::DeregisterClassicAdapterObserver(IAdapterClassicObserver &observer) const
{
    if (pimpl == nullptr) {
        HILOGE("pimpl is nullptr");
        return false;
    }
    return pimpl->adapterObservers_.Deregister(observer);
}

void ClassicAdapter::LoadPairedDeviceInfo()
{
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        return;
    }
    std::vector<std::string> pairedAddrList = adapterProperties_->GetPairedAddrList();
    for (auto &pairedAddr : pairedAddrList) {
        std::string addr = pairedAddr;
        if (addr.empty() || addr == INVALID_MAC_ADDRESS) {
            continue;
        } else {
            RawAddress device = RawAddress(addr);
            std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
            remoteDevice->SetPairedStatus(PAIR_PAIRED);
            ConnectStrategyManager::GetInstance()->InitConnectStrategy(remoteDevice->GetAddress());
            BLUEDROID::RawAddress btAddr = ServiceUtil::AddrToBluedroid(device);
            btInterface->get_remote_device_properties(&btAddr);
            BtChrUpdateDeviceInfo(device.GetAddress(), remoteDevice->GetRemoteName(), remoteDevice->GetRssi(),
                remoteDevice->GetDeviceClass());
            HILOGI("get_remote addr %{public}s", btAddr.ToLogString().c_str());
        }
    }
}

std::string ClassicAdapter::GetLocalAddress() const
{
    std::string addr = adapterProperties_->GetDeviceAddress();
    HILOGI("LocalAddress: %{public}s", GetEncryptAddr(addr).c_str());
    return addr;
}

int32_t ClassicAdapter::GenerateLocalOobData(int32_t transport) const
{
    HILOGI("transport: %{public}d", transport);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    CHECK_AND_RETURN_LOG_RET(status == BT_NO_ERROR, BT_ERR_INTERNAL_ERROR,
        "[ClassicAdapter] Failed to open the Bluetooth module");
    int ret = btInterface->generate_local_oob_data(ServiceUtil::TransportToBlueroid(transport));
    CHECK_AND_RETURN_LOG_RET(ret == BT_STATUS_SUCCESS, BT_ERR_INTERNAL_ERROR, "GenerateLocalOobData failed");
    return BT_NO_ERROR;
}

void ClassicAdapter::GenerateLocalOobDataCb(int32_t status, const Bluetooth::BluetoothOobData &oobData)
{
    HILOGI("[classic adapter] GenerateLocalOobDataCb, status: %{public}d", status);
    int32_t ret = (status == BT_STATUS_SUCCESS) ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR;
    pimpl->adapterObservers_.ForEach([ret, oobData](IAdapterClassicObserver &observer) {
        observer.OnGenerateLocalOobData(ret, oobData);
    });
}

std::string ClassicAdapter::GetLocalName() const
{
    std::string name = adapterProperties_->GetDeviceName();
    return name;
}

bool ClassicAdapter::SetLocalName(const std::string &name) const
{
    return adapterProperties_->SetDeviceName(name);
}

int ClassicAdapter::GetLocalDeviceClass() const
{
    int cod = adapterProperties_->GetDeviceClass();
    return cod;
}

bool ClassicAdapter::SetLocalDeviceClass(int deviceClass) const
{
    return adapterProperties_->SetDeviceClass(deviceClass);
}

int ClassicAdapter::GetBtScanMode() const
{
    return scanMode_.load();
}

void ClassicAdapter::DisconnectAcl(const std::string &address)
{
    HILOGI("DisconnectAcl. address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    if (BluetoothHwInterface::GetInstance()->GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }
    BLUEDROID::RawAddress rawAddr;
    if (!BLUEDROID::RawAddress::FromString(address, rawAddr)) {
        HILOGE("[DisconnectAcl] addr error");
        return;
    }
    BluetoothHwInterface::GetInstance()->GetBtHwInterface()->hwBtsndHcicDisconnect(rawAddr);
}

void ClassicAdapter::CreateAclConnect(const std::string &address)
{
    HILOGI("CreateAclConnect. address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    if (AdapterManager::GetInstance()->getBluetoothInterface() == nullptr) {
        HILOGE("bluetoothInterface_ is null.");
        return;
    }
    BLUEDROID::RawAddress rawAddr;
    if (!BLUEDROID::RawAddress::FromString(address, rawAddr)) {
        HILOGE("[CreateAclConnect] addr error");
        return;
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(RawAddress(address));
    if (remoteDevice->GetDeviceType() == DEVICE_TYPE_LE ||
        remoteDevice->GetDeviceType() == DEVICE_TYPE_UNKNOWN) {
        HILOGE("[CreateAclConnect] only for BR device.");
        return;
    }

    AdapterManager::GetInstance()->getBluetoothInterface()->create_acl_connection(&rawAddr);
}

static void ScanModeTimeoutCallback()
{
    HILOGI("Timeout, set scan mode --> page scan");
    std::shared_ptr<ClassicAdapter> classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is nullptr");
    classicAdapter->SetBtScanMode(SCAN_MODE_CONNECTABLE, 0);
}

void ClassicAdapter::StartBtScanModeTimer(int mode, int duration)
{
    if (mode != SCAN_MODE_GENERAL_DISCOVERABLE || duration == 0) {
        HILOGW("scanMode is %{public}d", mode);
        return;
    }
    HILOGI("duration is %{public}d ms", duration);
    std::lock_guard<std::mutex> lock(btScanTimeoutMutex_);
    int32_t scanTime = static_cast<int32_t>(duration);
    btScanTimeout_ = std::make_shared<utility::Timer>(&ScanModeTimeoutCallback);
    btScanTimeout_->Start(scanTime);
}

void ClassicAdapter::StopBtScanModeTimer()
{
    std::lock_guard<std::mutex> lock(btScanTimeoutMutex_);
    if (btScanTimeout_ != nullptr) {
        HILOGI("StopBtScanTimer");
        btScanTimeout_->Stop();
        btScanTimeout_ = nullptr;
    }
}

bool ClassicAdapter::SetFastScanLevel(int level)
{
    HILOGI("level = %{public}d.", level);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d.", status);
        return false;
    }
    btInterface->setFastScan(level);
    return true;
}

bool ClassicAdapter::SetBtScanMode(int mode, int duration)
{
    StopBtScanModeTimer();
    if (AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        HILOGE("[ClassicAdapter]RestrictBluetooth state, refuse StartBtDiscovery");
        return false;
    }

    if (mode < SCAN_MODE_NONE || mode > SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE) {
        HILOGE("SetBtScanMode mode = %{public}d is invalid", mode);
        return false;
    }
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return false;
    }
    if (duration < INVALID_VALUE) {
        HILOGE("failed. Invalid parameter[duration].");
        return false;
    }
    HILOGI("SetBtScanMode = %{public}d", mode);
    scanMode_ = mode;
    int result = SetBtScanModeProperty(btInterface, mode);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("SetBtScanMode fail");
        return false;
    }
    StartBtScanModeTimer(mode, duration);
#ifdef BLUETOOTH_FASTSCAN_ENABLE
    auto SetFastScanEnableFunc = BluetoothFastScanManagerLoader::GetInstance().getSetFastScanEnabledHandle();
    if (SetFastScanEnableFunc != nullptr) {
        SetFastScanEnableFunc(false);
    }
#endif
    return true;
}

bool ClassicAdapter::SetBtScanModeProperty(const bt_interface_t *btInterface, int mode)
{
    bt_scan_mode_t scanMode = BT_SCAN_MODE_NONE;
    bt_property_t property;
    property.len = sizeof(bt_scan_mode_t);
    property.type = static_cast<bt_property_type_t>(BLUEDROID::BT_PROPERTY_ADAPTER_SCAN_MODE);
    switch (mode) {
        case SCAN_MODE_CONNECTABLE:
            scanMode =  BT_SCAN_MODE_CONNECTABLE;
            break;
        case SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE:
        case SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE:
        case SCAN_MODE_GENERAL_DISCOVERABLE:
        case SCAN_MODE_LIMITED_DISCOVERABLE:
            scanMode = BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE;
            break;
        case SCAN_MODE_NONE:
        default:
            scanMode = BT_SCAN_MODE_NONE;
            break;
    }

    property.val = &scanMode;
    return btInterface->set_adapter_property(&property);
}

int ClassicAdapter::GetBondableMode() const
{
    return 0;
}

bool ClassicAdapter::SetBondableMode(int mode) const
{
    return true;
}

void ClassicAdapter::CancelDiscoveryIfInCloudBonding(const RawAddress &device)
{
    std::string callingName = PermissionManager::GetCallingName();
    if(GetCloudBondState(device) == CLOUD_BOND_BONDING && IsBtDiscovering()) {
        HILOGI("[ClassicAdapter] Cancelbtdiscovery because cloud state is bonding");
        CancelBtDiscovery();
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_BT_DISCOVER, UE_COMMON_SCENE_CASE5, callingName);
    }
}

int32_t ClassicAdapter::StartBtDiscovery()
{
    const bt_interface_t *btInterface = nullptr;
    std::string callingName = PermissionManager::GetCallingName();

    if (AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        HILOGE("[ClassicAdapter]RestrictBluetooth state, refuse StartBtDiscovery");
        return BT_ERR_RESTRICT_STATE;
    }
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE2, callingName);
        return BT_ERR_INTERNAL_ERROR;
    }
    auto cloudDeviceManager = CloudDeviceManager::GetInstance();
    if (cloudDeviceManager != nullptr && cloudDeviceManager->IsAnyDeviceInCloudPairing()) {
        HILOGI("[ClassicAdapter]Skipping start discovery because cloud device is bonding");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE7, callingName);
        return BT_ERR_CLOUD_DEVICE_BONDING;
    }
    if (IsBtDiscovering()) {
        HILOGI("already discovering state=%{public}d, return success", discoveryState_.load());
        return BT_NO_ERROR;
    }
#ifdef BT_USE_OPEN_STACK
    if (btInterface->pairing_is_busy != nullptr && btInterface->pairing_is_busy()) {
        HILOGW("pairing in progress, skip start discovery");
        return BT_NO_ERROR;
    }
#endif
    pimpl->startDiscoveryPid_ = IPCSkeleton::GetCallingPid();
    pimpl->startDiscoveryUid_ = IPCSkeleton::GetCallingUid();
    int ret = btInterface->start_discovery();
    if (ret == BT_STATUS_SUCCESS) {
        struct timeval tv {};
        gettimeofday(&tv, nullptr);
        long currentTime = (tv.tv_sec * MILLISECOND_UNIT + tv.tv_usec / MILLISECOND_UNIT);
        discoveryEndMs_ = currentTime + DEFAULT_DISCOVERY_TIMEOUT_MS;
        // Do not wait only for stack callback; mark started so timeout/end tracking works.
        discoveryState_ = DISCOVERY_STARTED;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE1, callingName);
    } else {
        HILOGE("failed, because of StartBtDiscovery failed!");
        discoveryState_ = DISCOVERY_STOPED;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_BT_DISCOVER, UE_COMMON_SCENE_CASE4, callingName);
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

bool ClassicAdapter::CancelBtDiscovery()
{
    const bt_interface_t *btInterface = nullptr;
    std::string callingName = PermissionManager::GetCallingName();
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_BT_DISCOVER, UE_COMMON_SCENE_CASE2, callingName);
        return false;
    }
    bool ret = false;
    if (discoveryState_ == DISCOVERY_STOPED) {
        HILOGI("already stopped, treat cancel as success");
        return true;
    }
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
#ifdef BT_USE_OPEN_STACK
    if (bluetoothHwSrcInterface != nullptr && bluetoothHwSrcInterface->isBondingOrSdp()) {
        HILOGW("bonding or sdp, no cancel discovery");
        return false;
    }
#else
    CHECK_AND_RETURN_LOG_RET(bluetoothHwSrcInterface != nullptr, false, "interface nullptr");
    CHECK_AND_RETURN_LOG_RET(!bluetoothHwSrcInterface->isBondingOrSdp(), false, "bonding or sdp, no cancel discovery");
#endif
    int result = btInterface->cancel_discovery();
    // Always clear local state: stack may already be idle (InquiryComplete missed),
    // while discoveryState_ is still DISCOVERY_STARTED and blocks the next Start.
    discoveryState_ = DISCOVERY_STOPED;
    if (result != BT_STATUS_SUCCESS) {
        HILOGW("CancelBtDiscovery stack ret=%{public}d, local state cleared", result);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_BT_DISCOVER, UE_COMMON_SCENE_CASE4, callingName);
        return true;
    }
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_BT_DISCOVER, UE_COMMON_SCENE_CASE1, callingName);
    return true;
}

bool ClassicAdapter::IsBtDiscovering() const
{
    HILOGI("discoveryState %{public}d", discoveryState_.load());

    return (discoveryState_.load() < DISCOVERY_STOPED);
}

long ClassicAdapter::GetBtDiscoveryEndMillis() const
{
    return discoveryEndMs_.load();
}

std::string ClassicAdapter::GetDeviceName(const RawAddress &device, bool alias) const
{
    return remoteDeviceProperties_->GetDeviceName(device, alias);
}

std::vector<std::string> ClassicAdapter::GetDeviceUuids(const RawAddress &device) const
{
    std::vector<Uuid> uuids = remoteDeviceProperties_->GetDeviceUuids(device);
    std::vector<std::string> res;
    for (auto uuid : uuids) {
        res.push_back(ServiceUtil::UuidToBluedroid(uuid).ToString());
    }
    return res;
}

std::vector<std::string> ClassicAdapter::GetLocalProfileUuids() const
{
    std::vector<Uuid> uuids = adapterProperties_->GetServiceUuids();
    std::vector<std::string> res;
    for (auto uuid : uuids) {
        res.push_back(ServiceUtil::UuidToBluedroid(uuid).ToString());
    }
    return res;
}

bool ClassicAdapter::IsAclConnected(const RawAddress &device) const
{
    return BluetoothConnectionManager::GetInstance()->IsBrConnected(device.GetAddress()) ||
        BluetoothConnectionManager::GetInstance()->IsBleConnected(device.GetAddress());
}

bool ClassicAdapter::IsAclEncrypted(const RawAddress &device) const
{
    if (remoteDeviceProperties_ == nullptr) {
        HILOGE("remoteDeviceProperties_ is nullptr.");
        return false;
    }
    return remoteDeviceProperties_->IsAclEncrypted(device);
}

utility::Context *ClassicAdapter::GetContext()
{
    return this;
}

bool ClassicAdapter::IsBondedFromLocal(const RawAddress &device) const
{
    return remoteDeviceProperties_->IsBondedFromLocal(device);
}

bool ClassicAdapter::StartRemoteSdpSearch(const std::string &address, const std::string &uuid)
{
    HILOGI("StartRemoteSdpSearch");
    return SdpAdapter::GetInstance().StartRemoteSdpSearch(address, uuid);
}

bool ClassicAdapter::GetRemoteServices(const std::string &address)
{
    HILOGI("GetRemoteServices. address: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    if (AdapterManager::GetInstance()->getBluetoothInterface() == nullptr) {
        HILOGE("bluetoothInterface_ is null.");
        return false;
    }
    BLUEDROID::RawAddress rawAddr;
    if (!BLUEDROID::RawAddress::FromString(address, rawAddr)) {
        HILOGE("[GetRemoteServices] addr error");
        return false;
    }
    int ret = AdapterManager::GetInstance()->getBluetoothInterface()->get_remote_services(&rawAddr);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("get_remote_services error, error code: %{public}d", ret);
    }
    return ret == BT_STATUS_SUCCESS;
}

int32_t ClassicAdapter::SetConnectionPriority(const std::string &address, int32_t priority)
{
    return BluetoothConnectionManager::GetInstance()->SetConnectionPriority(address, priority);
}

std::vector<RawAddress> ClassicAdapter::GetPairedDevices() const
{
    std::vector<RawAddress> pairedDevices;
    if (remoteDeviceProperties_ == nullptr) {
        HILOGE("remoteDeviceProperties_ is nullptr.");
        return pairedDevices;
    }
    pairedDevices = remoteDeviceProperties_->GetBtPairedDevices();
    HILOGD("pairedDeviceSize=%{public}d", pairedDevices.size());
    return pairedDevices;
}

static void WriteStartPairUeEvent(const RawAddress &device, int sceneCase,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    BtChrDeviceInfo deviceInfo;
    deviceInfo.btDeviceName = remoteDevice->GetRemoteName();
    deviceInfo.btDeviceClass = remoteDevice->GetDeviceClass() & CLASS_OF_DEVICE_MASK;
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_PAIR, device, deviceInfo, sceneCase, callingName);
    if (sceneCase == UE_COMMON_SCENE_CASE1) {
        BtChrUpdateDeviceInfo(device.GetAddress(), remoteDevice->GetRemoteName(), remoteDevice->GetRssi(),
            remoteDevice->GetDeviceClass());
    }
}

void ClassicAdapter::ReleaseBtChrInfos()
{
    std::vector<std::string> notPairNoneDevices = remoteDeviceProperties_->GetNotPairNoneDevices();
    HILOGD("notPairNoneDevices Size=%{public}d", notPairNoneDevices.size());
    BtChrReleaseByAddrs(notPairNoneDevices);
}

void ConvertToBtOobData(const Bluetooth::BluetoothOobData &oobData, bt_oob_data_t &btOobData)
{
    if (!oobData.HasOobData()) { // confirmationHash and addressWithType must be given
        HILOGE("invalid oob data");
        return;
    }

    std::vector<uint8_t> addressWithType = oobData.GetAddressWithType();
    if (addressWithType.size() != OOB_ADDRESS_SIZE) {
        HILOGE("invalid addressWithType size %{public}d", addressWithType.size());
        return;
    }
    std::copy(addressWithType.begin(), addressWithType.end(), btOobData.address);

    std::vector<uint8_t> confirmHash = oobData.GetConfirmationHash();
    if (confirmHash.size() != OOB_C_SIZE) {
        HILOGE("invalid confirmHash size %{public}d", confirmHash.size());
        return;
    }
    std::copy(confirmHash.begin(), confirmHash.end(), btOobData.c);

    if (oobData.HasRandomHash()) {
        std::vector<uint8_t> randomHash = oobData.GetRandomizerHash();
        if (randomHash.size() != OOB_R_SIZE) {
            HILOGE("invalid randomHash size %{public}d", randomHash.size());
            return;
        }
        std::copy(randomHash.begin(), randomHash.end(), btOobData.r);
    }
    if (oobData.HasDeviceName()) {
        std::string deviceName = oobData.GetDeviceName();
        size_t copyLen = deviceName.length();
        if (copyLen > OOB_NAME_MAX_SIZE) {
            HILOGE("invalid deviceName length %{public}d", copyLen);
            return;
        }
        std::copy(deviceName.begin(), deviceName.begin() + copyLen, btOobData.device_name);
    }
    btOobData.is_valid = true;
    return;
}

int ClassicAdapter::StartPairInner(const RawAddress &device, int32_t transport,
    const Bluetooth::BluetoothOobData &oobData, const bt_interface_t *btInterface)
{
    int ret = BT_STATUS_FAIL;
    BLUEDROID::RawAddress btAddr = ServiceUtil::AddrToBluedroid(device);
    if (oobData.HasOobData()) { // pairDeviceOutOfBand
        bt_oob_data_t btOobData;
        ConvertToBtOobData(oobData, btOobData);
        if (!btOobData.is_valid) {
            HILOGE("use pairDeviceOOB but oobData invalid.");
            return ret;
        }
        bt_oob_data_t empty_data;
        (void)memset_s(&empty_data, sizeof(empty_data), 0, sizeof(empty_data));
        if (oobData.GetOobDataType() == OobDataType::P192) { // use p192Data
            ret = btInterface->create_bond_out_of_band(&btAddr, transport, &btOobData, &empty_data);
        } else { // use p256Data
            ret = btInterface->create_bond_out_of_band(&btAddr, transport, &empty_data, &btOobData);
        }
    } else { // pairDevice
        ret = btInterface->create_bond(&btAddr, transport);
    };
    return ret;
}

bool ClassicAdapter::StartPair(int32_t transport, const RawAddress &device, const Bluetooth::BluetoothOobData &oobData)
{
    std::string addr = device.GetAddress();
    HILOG_COMM_INFO("StartPair: %{public}s", GetEncryptAddr(addr).c_str());
    ControlInterceptMessage msg {
        .addr = addr,
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedPairConn(msg), false, "Restricted by control intercept");
    ReleaseBtChrInfos();
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
    if (!BluetoothConnectionManager::GetInstance()->IsBrLinkAllowed(addr)) {
        HILOGE("[ClassicAdapter] Failed connect acl link because of MAX links.");
        WriteStartPairUeEvent(device, UE_COMMON_SCENE_CASE6, remoteDevice);
        SendPairStatusChanged(static_cast<BTTransport> (transport), device, PAIR_NONE, UNBOND_CAUSE_INTERNAL_ERROR,
            UNBOND_MSG_MAX_LINK);
        return false;
    }
    CHECK_AND_RETURN_LOG_RET(
        !AdapterManager::GetInstance()->IsBluetoothRestricted(), false, "RestrictBluetooth state, refuse pair device");
    const bt_interface_t *btInterface = nullptr;
    if (hal_util_load_bt_library(&btInterface)) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        WriteStartPairUeEvent(device, UE_COMMON_SCENE_CASE2, remoteDevice);
        return false;
    }
    if (StartCloudPair(device)) {
        HILOGI("[CLOUD_PAIR] device:%{public}s in cloud bonding", GetEncryptAddr(addr).c_str());
        return true;
    }
    int pairStatus = remoteDevice->GetPairedStatus();
    if (pairStatus == PAIR_PAIRING || pairStatus == PAIR_CANCELING) {
        /* Open-stack pairing can stall in PAIR_PAIRING (no SSP complete / watchdog).
         * Allow a fresh StartPair by cancelling the stale bond request first. */
        HILOGW("StartPair: clear stale pairStatus=%{public}d before retry", pairStatus);
        (void)remoteDeviceProperties_->CancelPairing(device);
        remoteDevice->SetPairedStatus(PAIR_NONE);
    }
    CancelBtDiscovery();
    if (BluetoothConnectionManager::GetInstance()->IsBrConnected(device.GetAddress())) {
        HILOGI("create bond, but an existing connection exists, disconnect current device ACL.");
        DisconnectAcl(device.GetAddress());
        AddDisconnectAclBeforeBondDevice(device.GetAddress());
        return true;
    }
    BluetoothConnectionManager::GetInstance()->SetVirtualAutoConnectType(device, 0, 0);
    int ret = StartPairInner(device, ServiceUtil::TransportToBlueroid(transport), oobData, btInterface);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("StartPair failed!");
        BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, addr, CHR_SUB_ERRCODE, SUBERRCODE_FAIL_CREATEBOND_NATIVE);
        WriteStartPairUeEvent(device, UE_COMMON_SCENE_CASE5, remoteDevice);
        if (remoteDeviceProperties_->GetCloudBondState(device) == CLOUD_BOND_BONDED) {
            remoteDeviceProperties_->SetCloudBondState(device, CLOUD_BOND_NONE);
            BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
                static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
                static_cast<int>(UeConnCloudPairSubSceneCode::CREATE_BOND_FAIL));
        }
        return false;
    }
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    if (!IsInCloudBondingState(remoteDevice)) {
        SendPairStatusChanged(static_cast<BTTransport> (transport), device, PAIR_PAIRING, PAIR_COMMON_BOND_CAUSE,
            BOND_MSG_NO_ERROR);
    }
    remoteDevice->SetBondedFromLocal(true);
    remoteDevice->DialogControlSetActivePair(true);
    remoteDevice->SetNeedDelayConnectFlag(false);
    remoteDevice->SetIsConnectFromLocal(true);
    WriteStartPairUeEvent(device, UE_COMMON_SCENE_CASE1, remoteDevice);
    return true;
}

bool ClassicAdapter::StartCrediblePair(int32_t transport, const RawAddress &device)
{
    remoteDeviceProperties_->AddPairWhiteList(device);
    Bluetooth::BluetoothOobData dummyOobData;
    if (!StartPair(transport, device, dummyOobData)) {
        if (remoteDeviceProperties_->CheckPairWhiteList(device)) {
            remoteDeviceProperties_->RemovePairWhiteList(device);
        }
        return false;
    }
    return true;
}

bool ClassicAdapter::CancelPairing(const RawAddress &device)
{
    return remoteDeviceProperties_->CancelPairing(device);
}

bool ClassicAdapter::RemovePair(int32_t transport, const RawAddress &device)
{
    bool isConnected = (a2dpSrcService_ != nullptr &&
        a2dpSrcService_->GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED)) ||
        (agService_ != nullptr &&
        agService_->GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED));
#ifdef BLUETOOTH_HFP_HF_ENABLE
    bool isHfphfConnected = hfService_ != nullptr &&
        hfService_->GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED);
    isConnected = isConnected || isHfphfConnected;
#endif
    if (isConnected) {
        HILOGI("Remove pairing after waiting for disconnection.");
        DisconnectAllowedProfiles(device);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, device.GetAddress(), "DISCONNECTREASON",
            REMOVEPAIR_BT_DISCONNECT_ACL);
        std::string taskName = "RemovePair-" + GetEncryptAddr((device).GetAddress());
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_CLASSIC, [this, transport, device] {this->RemovePairInner(transport, device);},
            DELAY_REMOVE_PAIR, taskName);
    } else {
        return RemovePairInner(transport, device);
    }
    return true;
}

bool ClassicAdapter::RemovePairInner(int32_t transport, const RawAddress &device) const
{
    if (!remoteDeviceProperties_->RemovePair(device)) {
        return false;
    }
    if (transport == BT_TRANSPORT_NONE) {
        std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
        int deviceType = remoteDeviceProperties_->GetDeviceType(device);
        transport = DeviceTypeToTransport(deviceType);
    }
    HILOGI("device transport is: %{public}d", transport);
    std::vector<RawAddress> removeDevices;
    removeDevices.push_back(device);
    AdapterManager::GetInstance()->OnPairDevicesRemoved(removeDevices);
    SendPairStatusChanged(static_cast<BTTransport> (transport), device, PAIR_NONE, UNBOND_CAUSE_USER_REMOVED,
        UNBOND_MSG_USER_REMOVED);
    BtChrEventWriteInt(CHR_BT_WATCH_SPP_DISCONNECT, device.GetAddress(), "DISCONNECTSCENE", CHR_REMOVE_PAIR);
    return true;
}

bool ClassicAdapter::RemoveAllPairs()
{
    std::vector<RawAddress> removeDevices = remoteDeviceProperties_->removeAllDevicesFromMap();
    if (!removeDevices.empty()) {
        AdapterManager::GetInstance()->OnPairDevicesRemoved(removeDevices);
    }
    return true;
}

int ClassicAdapter::GetPairState(const RawAddress &device) const
{
    return remoteDeviceProperties_->GetPairState(device);
}

bool ClassicAdapter::SetDevicePairingConfirmation(const RawAddress &device, bool accept) const
{
    return remoteDeviceProperties_->SetDevicePairingConfirmation(device, accept);
}

void ClassicAdapter::UserConfirmAutoReply(const RawAddress &device, int reqType, bool accept) const
{
    auto remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (remoteDevice != NULL) {
        remoteDevice->SetPairConfirmState(PAIR_CONFIRM_STATE_USER_CONFIRM_REPLY);
        remoteDevice->SetPairConfirmType(PAIR_CONFIRM_TYPE_INVALID);
    }

    switch (reqType) {
        case PIN_TYPE_ENTER_PIN_CODE: {
            HILOGI("PIN_TYPE_ENTER_PIN_CODE, do nothing!");
            break;
        }
        case PIN_TYPE_NOTIFY_PASSKEY: {
            HILOGI("PIN_TYPE_NOTIFY_PASSKEY, do nothing!!");
            break;
        }
        case PIN_TYPE_ENTER_PASSKEY: {
            HILOGI("PIN_TYPE_ENTER_PASSKEY");
            int passkey = 0;
            SetDevicePasskey(device, passkey, accept);
            break;
        }
        case PIN_TYPE_CONFIRM_PASSKEY:
        case PIN_TYPE_NO_PASSKEY_CONSENT: {
            HILOGI("PIN_TYPE_CONFIRM_PASSKEY or PIN_TYPE_NO_PASSKEY_CONSENT");
            SetDevicePairingConfirmation(device, accept);
            break;
        }
        default:
            HILOGI("default case. reqType = %{public}d", reqType);
            break;
    }
}

bool ClassicAdapter::SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const
{
    return false;
}

bool ClassicAdapter::PairRequestReply(const RawAddress &device, bool accept) const
{
    return false;
}

bool ClassicAdapter::SetDevicePin(const RawAddress &device, const std::string &pinCode) const
{
    HILOGI("pincode:%{public}s", GetEncryptPinCode(pinCode).c_str());
    return remoteDeviceProperties_->SetDevicePin(device, pinCode);
}

int ClassicAdapter::ClassicAdapter::GetDeviceType(const RawAddress &device) const
{
    return remoteDeviceProperties_->GetDeviceType(device);
}

int ClassicAdapter::GetDeviceClass(const RawAddress &device) const
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (remoteDevice != nullptr && remoteDevice->isBandDevice()) {
        if (remoteDevice->isBandMode()) {
            return BluetoothDevice::WEARABLE_WRIST_WATCH;
        } else {
            return BluetoothDevice::AUDIO_VIDEO_HEADPHONES;
        }
    }
    int32_t uid = PermissionManager::GetCallingUid();
    int32_t deviceCustomType = remoteDeviceProperties_->GetCustomType(device);
    if (IsNeededReturnCodForAudio(uid, deviceCustomType)) {
        auto it = customTypeToCod.find(static_cast<DeviceType>(deviceCustomType));
        return it->second;
    }
    int cod = remoteDeviceProperties_->GetDeviceClass(device);
    if (cod == static_cast<int>(CLASS_OF_DEVICE_UNCLASSIFIED)) {
        GetPeripheralCod(device, cod);
    }
    if (cod == 0) {
        bool wearDetectionSupported = false;
        BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, wearDetectionSupported);
        if (wearDetectionSupported) {
            HILOGW("device %{public}s support wear detection, treat as headset", GET_ENCRYPT_ADDR(device));
            cod = BluetoothDevice::AUDIO_VIDEO_HEADPHONES;
        }
    }
    return cod;
}

std::string ClassicAdapter::GetAliasName(const RawAddress &device) const
{
    return remoteDeviceProperties_->GetAliasName(device);
}

int ClassicAdapter::GetHwRemoteDeviceType(const RawAddress &device) const
{
    return remoteDeviceProperties_->GetHwRemoteDeviceType(device);
}

bool ClassicAdapter::SetAliasName(const RawAddress &device, const std::string &name)
{
    return remoteDeviceProperties_->SetAliasName(device, name);
}

bool ClassicAdapter::RegisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const
{
    return pimpl->remoteObservers_.Register(observer);
}

bool ClassicAdapter::DeregisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const
{
    if (pimpl == nullptr) {
        HILOGE("pimpl is nullptr");
        return false;
    }
    return pimpl->remoteObservers_.Deregister(observer);
}

int ClassicAdapter::GetRemoteDeviceBatteryInfo(const RawAddress &device, BatteryInfo &batteryInfo) const
{
    BatteryDetailInfo batteryDetailInfo;
    if (remoteDeviceProperties_->GetDeviceBatteryInfo(device.GetAddress(), batteryDetailInfo)) {
        batteryInfo = batteryDetailInfo.batteryInfo_;
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int ClassicAdapter::SetRemoteDeviceBatteryInfo(const RawAddress &device, const BatteryInfo &batteryInfo) const
{
    HILOGI("[classic_adapter] address(%{public}s)", GetEncryptAddr(device.GetAddress()).c_str());
    BluetoothDeviceBatteryManager *bluetoothDeviceBatteryManager = BluetoothDeviceBatteryManager::GetInstance();
    if (bluetoothDeviceBatteryManager != nullptr) {
        bluetoothDeviceBatteryManager->SetRemoteDeviceBatteryInfo(device, batteryInfo);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

void ClassicAdapter::SendRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    pimpl->remoteObservers_.ForEach([device, batteryInfo](IClassicRemoteDeviceObserver &observer) {
        observer.OnRemoteBatteryChanged(device, batteryInfo);
    });
}

void ClassicAdapter::SendRemoteEchoInfo(const RawAddress &device, const std::vector<uint8_t> &value)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    pimpl->remoteObservers_.ForEach([device, value](IClassicRemoteDeviceObserver &observer) {
        observer.OnRemoteDeviceCommonInfoReport(device, value);
    });
}

void ClassicAdapter::DiscoveryStateChanged(bt_discovery_state_t state)
{
    DoInClassicThread([this, state] {this->DiscoveryStateChangedInner(state);});
}

void ClassicAdapter::DiscoveryStateChangedInner(bt_discovery_state_t state)
{
    HILOGI("state: %{public}d", state);
    if (state == BT_DISCOVERY_STOPPED) {
        discoveryState_ = DISCOVERY_STOPED;
        struct timeval tv {};
        gettimeofday(&tv, nullptr);
        long currentTime = (tv.tv_sec * MILLISECOND_UNIT + tv.tv_usec / MILLISECOND_UNIT);
        discoveryEndMs_ = currentTime;
        SendDiscoveryStateChanged(discoveryState_.load());
        BluetoothHelper::BluetoothCommonEventHelper::PublishDiscoveryFinishedEvent(discoveryState_.load());
    } else if (state == BT_DISCOVERY_STARTED) {
        discoveryState_ = DISCOVERY_STARTED;
        SendDiscoveryStateChanged(discoveryState_.load());
        BluetoothHelper::BluetoothCommonEventHelper::PublishDiscoveryStartedEvent(discoveryState_.load());
        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "DISCOVERY_STATE",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "PID", pimpl->startDiscoveryPid_.load(),
            "UID", pimpl->startDiscoveryUid_.load(), "STATE", DISCOVERY_STARTED);
    } else {
        HILOGE("DISCOVERYING");
    }
}

void ClassicAdapter::BondStateChanged(bt_status_t status, BLUEDROID::RawAddress* bd_addr, bt_bond_state_t state)
{
    if (bd_addr == nullptr) {
        HILOGE("wrong addr");
        return;
    }
    BLUEDROID::RawAddress addr = *bd_addr;
    DoInClassicThread([this, status, addr, state] {
        this->BondStateChangedInner(status, addr, state);
        this->NotifyBondStateChanged(status, addr, state);
    });
}

bool ClassicAdapter::NeedWaitForSdpComplete(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    if ((remoteDevice->GetDeviceType() == DEVICE_TYPE_BREDR ||
        remoteDevice->GetDeviceType() == DEVICE_TYPE_DUAL_MONO) &&
        remoteDevice->GetDeviceUuids().empty()) {
        return true;
    }

    return false;
}

static void ClearSavedDeviceInfo(BLUEDROID::RawAddress rawAddr, const RawAddress &device)
{
    BluetoothAudioManager::GetInstance().ClearWearDetectionSupport(device.GetAddress());

    PreferencesManager::Delete(rawAddr.ToString(), PreferencesManagerType::ABS_VOLUME_SWITCH);
    PreferencesManager::Delete(rawAddr.ToString(), PreferencesManagerType::ABS_VOLUME);
    PreferencesManager::Delete(rawAddr.ToString(), PreferencesManagerType::NON_ABS_VOLUME);
    PreferencesManager::Delete(device.GetAddress(), PreferencesManagerType::HFP_VOLUME);
    PreferencesManager::Delete(device.GetAddress(), PreferencesManagerType::VIRTUAL_AUTO_CONN_SWITCH);
    PreferencesManager::Delete(device.GetAddress(), PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE);
    PreferencesManager::Delete(device.GetAddress(), PreferencesManagerType::WATCH_SAIS_DEVICE);
    // device unpaired，set ability as unknow to stack
    AvrcpServiceManager::GetInstance().SetDeviceAbsVolumeProperty(rawAddr,
        DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT);
    RefusePlayHelper::GetInstance()->ClearRecord(device.GetAddress());
    BluetoothAudioManager::GetInstance().ClearLatencyInfoByAddr(device);
    ConnectStrategyManager::GetInstance()->UpdateAutoConnectDeivce(device.GetAddress());
}

std::string ClassicAdapter::CovertUnbondMessage(bt_status_t status)
{
    std::string outMessage = UNBOND_MSG_UNKNOWN;
    switch (status) {
        case BT_STATUS_BUSY:
            outMessage = UNBOND_MSG_STATUS_BUSY;
            break;
        case BT_STATUS_NOT_READY:
            outMessage = UNBOND_MSG_NOT_READY;
            break;
        case BT_STATUS_UNSUPPORTED:
            outMessage = UNBOND_MSG_UNSUPPORTED;
            break;
        case BT_STATUS_PARM_INVALID:
            outMessage = UNBOND_MSG_PARM_INVALID;
            break;
        case BT_STATUS_JNI_ENVIRONMENT_ERROR:
        case BT_STATUS_JNI_THREAD_ATTACH_ERROR:
            outMessage = UNBOND_MSG_JNI_ERROR;
            break;
        default:
            outMessage = UNBOND_MSG_UNKNOWN;
            break;
    }
    return outMessage;
}

void ClassicAdapter::CovertUnbondCause(bt_status_t status, int &unbondCause, std::string &causeMessage)
{
    switch (status) {
        case BT_STATUS_RMT_DEV_DOWN:
            unbondCause = UNBOND_CAUSE_REMOTE_DEVICE_DOWN;
            causeMessage = UNBOND_MSG_NOT_EXIST;
            break;
        case BT_STATUS_AUTH_FAILURE:
            unbondCause = UNBOND_CAUSE_AUTH_FAILURE;
            causeMessage = UNBOND_MSG_AUTH_FAILURE;
            break;
        case BT_STATUS_AUTH_REJECTED:
            unbondCause = UNBOND_CAUSE_AUTH_REJECTED;
            causeMessage = UNBOND_MSG_AUTH_REJECTED;
            break;
        case BT_STATUS_SUCCESS:
            unbondCause = PAIR_COMMON_BOND_CAUSE;
            causeMessage = BOND_MSG_NO_ERROR;
            break;
        default:
            unbondCause = UNBOND_CAUSE_INTERNAL_ERROR;
            causeMessage = CovertUnbondMessage(status);
            break;
    }
}

void ClassicAdapter::BondStateChangedInner(bt_status_t status, BLUEDROID::RawAddress bd_addr, bt_bond_state_t state)
{
    RawAddress device = ServiceUtil::AddrFromBluedroid(bd_addr);
    HILOGI("device: %{public}s status: %{public}d bondState: %{public}d", GET_ENCRYPT_ADDR(device), status, state);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        HILOGE("device not exist");
        SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_NONE, UNBOND_CAUSE_REMOTE_DEVICE_DOWN, UNBOND_MSG_NOT_EXIST);
        return;
    }
    if (status != BT_STATUS_SUCCESS) {
        HandleBondStateFailed(status, remoteDevice, device);
        return;
    }
    if (remoteDevice->IsCloudDevice() && (remoteDevice->GetCloudBondState() == CLOUD_BOND_BONDING)
        && state != BT_BOND_STATE_BONDED && !remoteDevice->IsBondedFromLocal()) {
        HILOGI("[CLOUD_PAIR]bondStateChangeCallback drop, device is cloud pairing");
        return;
    }
    if (state == BT_BOND_STATE_BONDING) {
        HandleBondStateBonding(remoteDevice, device);
    } else if (state == BT_BOND_STATE_NONE) {
        HandleBondStateBondNone(status, bd_addr, remoteDevice, device, state);
    } else if (state == BT_BOND_STATE_BONDED) {
        remoteDevice->SetPairConfirmState(PAIR_CONFIRM_STATE_INVALID);
        remoteDevice->SetPairConfirmType(PAIR_CONFIRM_TYPE_INVALID);
        if (remoteDevice->IsCloudDevice()) {
            if (remoteDevice->GetCloudBondState() != CLOUD_BOND_NONE) {
                BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
                    static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
                    static_cast<int>(UeConnCloudPairSubSceneCode::PAIR_SUCCESS));
            }
            remoteDevice->SetCloudBondState(CLOUD_BOND_BONDED);
        }
        if (remoteDevice->GetPairedStatus() == PAIR_PAIRING) {
            remoteDevice->SetPairedStatus(PAIR_PAIRED);
            adapterProperties_->AddPairedAddrList(device);
            std::vector<std::string> pairedAddrList = adapterProperties_->GetPairedAddrList();
            BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), "BONDEDCNT", pairedAddrList.size());
            BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), CHR_ERRCODE, ERRCODE_PAIR_SUCCESS);
            if (NeedWaitForSdpComplete(remoteDevice)) {
                /* Open stack does not auto-SDP after CreateBond; kick search while ACL
                 * is still up. Still notify PAIR_PAIRED so Settings leaves "pairing…".
                 * UUID callback will refresh pending/connect strategy when ready. */
                HILOGW("device bonded, start SDP (do not block UI on wait)");
                {
                    std::lock_guard<BtRecursiveMutex> lk(pimpl->pendingDeviceMutex_);
                    pendingPairedDevices_.insert(remoteDevice->GetAddress());
                }
                (void)GetRemoteServices(device.GetAddress());
            }
            SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_PAIRED, PAIR_COMMON_BOND_CAUSE, BOND_MSG_NO_ERROR);
        }
    }
}

void ClassicAdapter::SspRequest(BLUEDROID::RawAddress* remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod,
    bt_ssp_variant_t pairingVariant, uint32_t passKey)
{
    if (remote_bd_addr == nullptr) {
        HILOGE("wrong addr");
        return;
    }
    BLUEDROID::RawAddress addr = *remote_bd_addr;
    DoInClassicThread([this, addr, bd_name, cod, pairingVariant,
        passKey] {this->SspRequestInner(addr, bd_name, cod, pairingVariant, passKey);});
}

void ClassicAdapter::SspRequestInner(BLUEDROID::RawAddress remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod,
    bt_ssp_variant_t pairingVariant, uint32_t passKey)
{
    RawAddress device = ServiceUtil::AddrFromBluedroid(remote_bd_addr);
    HILOGI("address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int pinType = PIN_TYPE_NO_PASSKEY_CONSENT;
    if (pairingVariant == BT_SSP_VARIANT_PASSKEY_CONFIRMATION) {
        pinType = PIN_TYPE_CONFIRM_PASSKEY;
    } else if (pairingVariant == BT_SSP_VARIANT_CONSENT) {
        pinType = PIN_TYPE_NO_PASSKEY_CONSENT;
    } else if (pairingVariant == BT_SSP_VARIANT_PASSKEY_NOTIFICATION) {
        pinType = PIN_TYPE_NOTIFY_PASSKEY;
    } else if (pairingVariant == BT_SSP_VARIANT_PASSKEY_ENTRY) {
        pinType = PIN_TYPE_ENTER_PASSKEY;
    } else {
        HILOGE("SSP Pairing variant not present");
        return;
    }

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        HILOGW("SSP device not in map, auto-accept confirm");
        const bt_interface_t *btInterface = nullptr;
        if (hal_util_load_bt_library(&btInterface) == 0 && btInterface != nullptr && btInterface->ssp_reply != nullptr) {
            BLUEDROID::RawAddress address = remote_bd_addr;
            (void)btInterface->ssp_reply(&address, pairingVariant, true, passKey);
        }
        return;
    }
    remoteDevice->SetPairConfirmState(PAIR_CONFIRM_STATE_USER_CONFIRM);
    remoteDevice->SetPairConfirmType(pinType);
    remoteDevice->SetPasskey(passKey);
    remoteDevice->SetSspVariant(pairingVariant);
    if (!remoteDevice->IsBondedFromLocal()) {
        remoteDevice->SetNeedDelayConnectFlag(true);
    }
    if (SetPairingConfirmationIfNeed(remoteDevice->GetAddress()) && remoteDevice->IsBondedFromLocal()) {
        SetDevicePairingConfirmation(device, true);
        HILOGI("confirm credible device pair");
        return;
    }
    /* Headset / Just-Works: auto-confirm so pairing does not hang waiting for UI. */
    if (pinType == PIN_TYPE_CONFIRM_PASSKEY || pinType == PIN_TYPE_NO_PASSKEY_CONSENT) {
        HILOGI("auto confirm SSP pinType=%{public}d", pinType);
        SetDevicePairingConfirmation(device, true);
        return;
    }
    SendPairConfirmed(device, pinType, static_cast<int>(passKey));
}

void ClassicAdapter::PinRequest(BLUEDROID::RawAddress* remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod,
    bool min16Digit)
{
    if (remote_bd_addr == nullptr) {
        HILOGE("wrong addr");
        return;
    }
    RawAddress device = ServiceUtil::AddrFromBluedroid(*remote_bd_addr);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        HILOGE("device not exist");
        return;
    }
    BLUEDROID::RawAddress addr = *remote_bd_addr;
    DoInClassicThread([this, addr, bd_name, cod,
        min16Digit] {this->PinRequestInner(addr, bd_name, cod, min16Digit);});
}

void ClassicAdapter::PinRequestInner(BLUEDROID::RawAddress remote_bd_addr, bt_bdname_t* bd_name, uint32_t cod,
    bool min16Digit)
{
    RawAddress device = ServiceUtil::AddrFromBluedroid(remote_bd_addr);
    HILOGI("address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int pinType = min16Digit ? PIN_TYPE_PIN_16_DIGITS : PIN_TYPE_ENTER_PIN_CODE;
    int pinCode = 0; // 0 为默认pincode

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        HILOGE("device not exist");
        return;
    }
    uint32_t majorMinorClass = remoteDevice->GetMajorMinorClass();
    if (majorMinorClass == BluetoothDevice::PERIPHERAL_KEYBOARD_POINTING ||
        majorMinorClass == BluetoothDevice::PERIPHERAL_KEYBOARD) {
        pinType = PIN_TYPE_DISPLAY_PIN_CODE;
        pinCode = GenerateRandomNum(100000, 999999); // 100000 and 999999 are boundary values.
    }
    remoteDevice->SetPairConfirmState(PAIR_CONFIRM_STATE_USER_CONFIRM);
    remoteDevice->SetPairConfirmType(pinType);
    remoteDevice->SetPasskey(pinCode);
    if (!remoteDevice->IsBondedFromLocal()) {
        remoteDevice->SetNeedDelayConnectFlag(true);
    }

    SendPairConfirmed(device, pinType, pinCode);
}

void ClassicAdapter::SendPairConfirmed(const RawAddress &device, int reqType, int number)
{
    int type = remoteDeviceProperties_->GetDeviceType(device);
    HILOG_COMM_INFO("SendPairConfirmed: %{public}s, reqType: %{public}d, number: %{public}s, device type : %{public}d",
        GET_ENCRYPT_ADDR(device), reqType, GetEncryptPinCode(number).c_str(), type);
    BTTransport transport = ADAPTER_BREDR;
    if (type == DEVICE_TYPE_LE) {
        transport = ADAPTER_BLE;
    } else {
        transport = ADAPTER_BREDR;
    }
    if (!SetPairingConfirmationIfNeed(device.GetAddress())) {
#ifdef BLUETOOTH_INTERCEPT_PERIPHERAL_FIRST_PAIR_DIALOG
        if ((reqType == PIN_TYPE_NO_PASSKEY_CONSENT) && (number == BLUETOOTH_PIN_NO_PASSKEY)) {
            SetDevicePairingConfirmation(device, true);
            HILOG_COMM_INFO("Received PIN_TYPE_NO_PASSKEY_CONSENT: skipping pairing dialog.");
        } else {
            DialogPair::RequestBluetoothPairDialog(device, reqType, number);
        }
#else
        DialogPair::RequestBluetoothPairDialog(device, reqType, number);
#endif
    }
    pimpl->adapterObservers_.ForEach([transport, device, reqType, number](IAdapterClassicObserver &observer) {
        observer.OnPairConfirmed(transport, device, reqType, number);
    });
}

bool ClassicAdapter::FindPendingPairedDevice(const std::string device)
{
    std::lock_guard<BtRecursiveMutex> lk(pimpl->pendingDeviceMutex_);
    return pendingPairedDevices_.find(device) != pendingPairedDevices_.end();
}

void ClassicAdapter::DeletePendingPairedDevice(const std::string device)
{
    std::lock_guard<BtRecursiveMutex> lk(pimpl->pendingDeviceMutex_);
    pendingPairedDevices_.erase(device);
}

void ClassicAdapter::SaveRemoteDeviceUuids(
    std::shared_ptr<BluetoothDevice> remote, const std::vector<Uuid> &uuids)
{
    if ((remote == nullptr) || (uuids.empty())) {
        HILOGI("wrong uuids");
        return;
    }

    std::vector<Uuid> prevUuids = remote->GetDeviceUuids();
    bool isEuqal = IsUuidsEqual(uuids, prevUuids);
    if (isEuqal) {
        HILOGI("no change, ignore uuids update for device: %{public}s", GET_ENCRYPT_STR_ADDR(remote->GetAddress()));
        return;
    }
    remote->SetDeviceUuids(uuids);
    SendRemoteUuidChanged(RawAddress(remote->GetAddress()), uuids);
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteUuidsChangedEvent(remote->GetAddress(),
        ServiceUtil::UuidsToUuidStrs(uuids));
    ConnectStrategyManager::GetInstance()->InitConnectStrategy(remote->GetAddress());
    RawAddress device(remote->GetAddress());
    if (FindPendingPairedDevice(remote->GetAddress())) {
        DeletePendingPairedDevice(remote->GetAddress());
        SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_PAIRED, PAIR_COMMON_BOND_CAUSE, BOND_MSG_NO_ERROR);
        CheckDeviceUuidsAndSendChr(remote);
        HILOGI("paired device: %{public}s", GET_ENCRYPT_STR_ADDR(remote->GetAddress()));
    } else {
        HILOGI("ignore, pendingPairedDevices_ contain no device: %{public}s",
            GET_ENCRYPT_STR_ADDR(remote->GetAddress()));
    }
    IsDeviceReadyConnect(device);
}

bool ClassicAdapter::IsUuidsEqual(const std::vector<Uuid> &uuids1, const std::vector<Uuid> &uuids2) const
{
    if (uuids1.size() != uuids2.size()) {
        return false;
    }

    for (auto uuid : uuids1) {
        auto it = std::find(uuids2.begin(), uuids2.end(), uuid);
        if (it == uuids2.end()) {
            return false;
        }
    }

    for (auto uuid : uuids2) {
        auto it = std::find(uuids1.begin(), uuids1.end(), uuid);
        if (it == uuids1.end()) {
            return false;
        }
    }

    return true;
}

void ClassicAdapter::SendAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus)
{
    HILOG_COMM_INFO("SendAclStateChanged: %{public}s, state: %{public}d, reason: %{public}u",
        GetEncryptAddr(device.GetAddress()).c_str(), state, reason);
    if (state == CONNECTION_STATE_DISCONNECTED && IsDisconnectAclBeforeBond(device.GetAddress())) {
        Bluetooth::BluetoothOobData dummyOobData;
        StartPair(LINK_TYPE_BREDR, device, dummyOobData);
        DelDisconnectAclBeforeBondDevice(device.GetAddress());
        return;
    }
    pimpl->remoteObservers_.ForEach([device, state, reason, pairStatus](IClassicRemoteDeviceObserver &observer) {
        observer.OnAclStateChanged(device, state, reason, pairStatus);
    });
}

void ClassicAdapter::SendPairStatusChanged(const BTTransport transport, const RawAddress &device,
    int status, int cause, const std::string &causeMessage) const
{
    HILOGI("address: %{public}s, status: %{public}d, cause: %{public}d, message: %{public}s",
        GET_ENCRYPT_ADDR(device), status, cause, causeMessage.c_str());
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (status == PAIR_PAIRED) {
        BLUEDROID::RawAddress bdaddr = ServiceUtil::AddrToBluedroid(device);
        remoteDeviceProperties_->GetRemoteDeviceProperty(
            bdaddr, static_cast<bt_property_type_t>(HW_BT_PROPERTY_RMT_IO_CAP_KEY));
#ifdef BLUETOOTH_FASTSCAN_ENABLE
        DoInLowPriorityThread([]() {BluetoothFastScanManagerLoader::GetInstance().Init();});
#endif
        int majorMinorClass = 0; //0代表默认值
        if (remoteDevice != nullptr) {
            majorMinorClass = remoteDevice->GetMajorMinorClass();
        }
        int32_t deviceCustomType = remoteDeviceProperties_->GetCustomType(device);
        auto it = codToCustomType.find(majorMinorClass);
        if (it != codToCustomType.end() && deviceCustomType == DeviceType::DEVICE_TYPE_DEFAULT) {
            SetDeviceCustomType(device, it->second);
        }
    }
    if (status == PAIR_NONE) {
        if (remoteDeviceProperties_->CheckPairWhiteList(device)) {
            remoteDeviceProperties_->RemovePairWhiteList(device);
        }
        if (remoteDevice != nullptr) {
            remoteDevice->SetNeedDelayConnectFlag(false);
            remoteDevice->SetBondedFromLocal(false);
            remoteDevice->DialogControlSetActivePair(false);
        }
    }
    pimpl->remoteObservers_.ForEach([transport, device, status, cause, causeMessage]
        (IClassicRemoteDeviceObserver &observer) {
            observer.OnPairStatusChanged(transport, device, status, cause, causeMessage);
    });
}

void ClassicAdapter::SendDiscoveryStateChanged(int discoveryState) const
{
    HILOGI("discoveryState: %{public}d", discoveryState);
    pimpl->adapterObservers_.ForEach([discoveryState](IAdapterClassicObserver &observer) {
        observer.OnDiscoveryStateChanged(discoveryState);
    });
}

void ClassicAdapter::SendRemoteCodChanged(const RawAddress &device, int cod) const
{
    HILOGI("address: %{public}s, cod: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), cod);
    pimpl->remoteObservers_.ForEach([device, cod](IClassicRemoteDeviceObserver &observer) {
        observer.OnRemoteCodChanged(device, cod);
    });
}

void ClassicAdapter::SendRemoteNameChanged(const RawAddress &device, const std::string &deviceName) const
{
    HILOGI("address: %{public}s, deviceName: %{public}s", GetEncryptAddr(device.GetAddress()).c_str(),
        GET_ENCRYPT_DEVICE_NAME(deviceName));
    pimpl->remoteObservers_.ForEach([device, deviceName](IClassicRemoteDeviceObserver &observer) {
        observer.OnRemoteNameChanged(device, deviceName);
    });
}

void ClassicAdapter::SendRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) const
{
    HILOGD("address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    pimpl->remoteObservers_.ForEach([device, uuids](IClassicRemoteDeviceObserver &observer) {
        observer.OnRemoteUuidChanged(device, uuids);
    });
}

void ClassicAdapter::SendScanModeChanged(const int scanMode) const
{
    HILOGI("scanMode change to %{public}d", scanMode);
    pimpl->adapterObservers_.ForEach([scanMode](IAdapterClassicObserver &observer) {
        observer.OnScanModeChanged(scanMode);
    });
}

void ClassicAdapter::SendDiscoveryResult(
    const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) const
{
    pimpl->adapterObservers_.ForEach([device, rssi, deviceName, deviceClass](IAdapterClassicObserver &observer) {
        observer.OnDiscoveryResult(device, rssi, deviceName, deviceClass);
    });
}

void ClassicAdapter::UpdateDiscovertState(int discoveryState)
{
    discoveryState_ = discoveryState;
}

bool ClassicAdapter::SetFastScan(bool isEnable)
{
    HILOGE("isEnable: %{public}d.", isEnable);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module, status: %{public}d. ", status);
        return false;
    }
    btInterface->enable_fast_scan(isEnable);

    return true;
}

bool ClassicAdapter::SetPairingConfirmationIfNeed(const std::string &address)
{
    HILOGI("address: %{public}s", GetEncryptAddr(address).c_str());
    RawAddress device(address);
    if (remoteDeviceProperties_->CheckPairWhiteList(device)) {
        remoteDeviceProperties_->RemovePairWhiteList(device);
        HILOGI("need auto SetPairingConfirmation");
        return true;
    }
    return false;
}

bool ClassicAdapter::IsAnyProfilesAllowed(const RawAddress &device) const
{
    if (agService_ != nullptr &&
        agService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (hfService_ != nullptr &&
        hfService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
#endif
    if (a2dpSrcService_ != nullptr &&
        a2dpSrcService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
    if (hidHostService_ != nullptr &&
        hidHostService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
    if (panService_ != nullptr &&
        panService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    if (basService_ != nullptr &&
        basService_->GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        return true;
    }
#endif

    return false;
}

bool ClassicAdapter::IsProfileSupported(uint32_t profile,
    std::vector<Uuid> &localUuids, std::vector<Uuid> &remoteUuids) const
{
    if (remoteUuids.empty()) {
        HILOGE("remote device uuids empty");
        return false;
    }

    if (profile == PROFILE_ID_HFP_AG) {
        return (IsUuidSupport(localUuids, BLUETOOTH_UUID_HSP_AG) &&
                IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HSP_HS)) ||
                (IsUuidSupport(localUuids, BLUETOOTH_UUID_HFP_AG) &&
                IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HFP_HF));
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (profile == PROFILE_ID_HFP_HF) {
        return (IsUuidSupport(localUuids, BLUETOOTH_UUID_HSP_HS) &&
                IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HSP_AG)) ||
                (IsUuidSupport(localUuids, BLUETOOTH_UUID_HFP_HF) &&
                IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HFP_AG));
    }
#endif
    if (profile == PROFILE_ID_A2DP_SRC) {
        return IsUuidSupport(remoteUuids, ADV_AUDIO_DIST) ||
            IsUuidSupport(remoteUuids, BLUETOOTH_UUID_A2DP_SINK);
    }

    if (profile == PROFILE_ID_HID_HOST) {
        return IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HID_HOST) ||
            IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HOGP);
    }
    if (profile == PROFILE_ID_PAN) {
        return IsUuidSupport(remoteUuids, BLUETOOTH_UUID_PAN);
    }
    if (profile == PROFILE_ID_HEARING_AID) {
        return IsUuidSupport(remoteUuids, BLUETOOTH_UUID_HEARING_AID);
    }
    if (profile == PROFILE_ID_BAS) {
        return IsUuidSupport(remoteUuids, BLUETOOTH_UUID_BAS);
    }
    return false;
}

bool ClassicAdapter::IsProfileAllowed(uint32_t profileId, const RawAddress &device) const
{
    switch (profileId) {
        case PROFILE_ID_HFP_AG:
            return agService_ != nullptr && agService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
#ifdef BLUETOOTH_HFP_HF_ENABLE
        case PROFILE_ID_HFP_HF:
            return hfService_ != nullptr && hfService_->GetConnectStrategy(device) ==
            static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
#endif
        case PROFILE_ID_A2DP_SRC:
            return a2dpSrcService_ != nullptr && a2dpSrcService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        case PROFILE_ID_HID_HOST:
            return hidHostService_ != nullptr && hidHostService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        case PROFILE_ID_HEARING_AID:
            return hearingAidService_ != nullptr && hearingAidService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
        case PROFILE_ID_PAN:
            return panService_ != nullptr && panService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
        case PROFILE_ID_BAS:
            return basService_ != nullptr && basService_->GetConnectStrategy(device) ==
                static_cast<int>(BTStrategyType::CONNECTION_ALLOWED);
#endif
        default:
            HILOGE("unknown profileId: %{public}d", profileId);
            return false;
    }
}

bool ClassicAdapter::GetProfileSupportState(uint32_t profileId, const RawAddress &device,
    bool isConnectMoreProfileEnabled) const
{
    std::vector<Uuid> localUuids = adapterProperties_->GetServiceUuids();
    std::vector<Uuid> remoteUuids = remoteDeviceProperties_->GetDeviceUuids(device);
    return isConnectMoreProfileEnabled && IsProfileSupported(profileId, localUuids, remoteUuids) &&
        IsProfileAllowed(profileId, device);
}

void ClassicAdapter::ConnectAnyAllowedProfiles(const RawAddress &device, 
                                                bool isConnectMoreProfileEnabled) const
{
    HILOGI("ConnectAnyAllowedProfiles for device: %{public}s", GET_ENCRYPT_ADDR(device));
    int32_t numProfilesConnected = 0;
    ConnectClassicProfiles(device, isConnectMoreProfileEnabled, numProfilesConnected);
    ConnectBleProfiles(device, isConnectMoreProfileEnabled, numProfilesConnected);

    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    CHECK_AND_RETURN_LOG(remoteDevice != nullptr, "remoteDevice not find");
    remoteDevice->SetNeedDelayConnectFlag(false);
    HILOGI("device: %{public}s and num of profiles: %{public}d", GET_ENCRYPT_ADDR(device), numProfilesConnected);
}

void ClassicAdapter::ConnectHfpProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled, 
    int32_t &numProfilesConnected) const
{
    if (GetProfileSupportState(PROFILE_ID_HFP_AG, device, isConnectMoreProfileEnabled)) {
        HILOGI("HFP_AG connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        agService_->Connect(device);
        numProfilesConnected++;
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (GetProfileSupportState(PROFILE_ID_HFP_HF, device, isConnectMoreProfileEnabled)) {
        HILOGI("HFP_HF connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hfService_->Connect(device);
        numProfilesConnected++;
    }
#endif
}

void ClassicAdapter::ConnectA2dpProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled,
    int32_t &numProfilesConnected) const
{
    if (GetProfileSupportState(PROFILE_ID_A2DP_SRC, device, isConnectMoreProfileEnabled)) {
        HILOGI("A2DP_SRC connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (numProfilesConnected != 0) {
            // 50 means that increase the 50ms a2dp connection delay and ensure to connect hfp first.
            ThreadUtil::GetInstance().PostTask(
                THREAD_ID_CLASSIC, [this, device]() { a2dpSrcService_->Connect(device); },
                50, "ConnectA2dp");
        } else {
            a2dpSrcService_->Connect(device);
        }
        std::shared_ptr<BluetoothDevice> bluetoothDevice =
            RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        bluetoothDevice->SetIsConnectFromLocal(true);
        numProfilesConnected++;
    }
}

void ClassicAdapter::ConnectClassicProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled, 
    int32_t &numProfilesConnected) const
{
    ConnectHfpProfiles(device, isConnectMoreProfileEnabled, numProfilesConnected);
    ConnectA2dpProfiles(device, isConnectMoreProfileEnabled, numProfilesConnected);
    if (GetProfileSupportState(PROFILE_ID_HID_HOST, device, isConnectMoreProfileEnabled)) {
        HILOGI("HID_HOST connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hidHostService_->Connect(device);
        numProfilesConnected++;
    }
    if (GetProfileSupportState(PROFILE_ID_PAN, device, isConnectMoreProfileEnabled)) {
        HILOGI("PAN connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        panService_->Connect(device);
        numProfilesConnected++;
    }
}

void ClassicAdapter::ConnectBleProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled, 
    int32_t &numProfilesConnected) const
{
    if (GetProfileSupportState(PROFILE_ID_HEARING_AID, device, isConnectMoreProfileEnabled)) {
        HILOGI("HEARING_AID connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hearingAidService_->Connect(device);
        numProfilesConnected++;
    }
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    if (GetProfileSupportState(PROFILE_ID_BAS, device, isConnectMoreProfileEnabled)) {
        HILOGI("BAS connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        basService_->Connect(device);
        numProfilesConnected++;
    }
#endif
}

void ClassicAdapter::ConnectAllowedProfiles(const RawAddress &device, bool isConnectAllEnabled)
{
    BtChrAddConnSceneInfo(device.GetAddress(), PAIR_TYPE_USER_CONNECT, PermissionManager::GetCallingName(), 0);
    if (IsAnyProfilesAllowed(device)) {
        HILOGI("any pofile connectStrategy is allowed, device: %{public}s", GET_ENCRYPT_ADDR(device));
        return ConnectAnyAllowedProfiles(device, isConnectAllEnabled);
    }
    this->SetConnectStrategy(device);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    CHECK_AND_RETURN_LOG(remoteDevice != nullptr, "remoteDevice not find");
    remoteDevice->SetNeedDelayConnectFlag(false);
    ConnectAnyAllowedProfiles(device, isConnectAllEnabled);
}

void ClassicAdapter::SetHfpConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
    std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies) {
    if (agService_ != nullptr && IsProfileSupported(PROFILE_ID_HFP_AG, localUuids, remoteUuids)) {
        HILOG_COMM_INFO("HFP_AG connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        agService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (hfService_ != nullptr && IsProfileSupported(PROFILE_ID_HFP_HF, localUuids, remoteUuids)) {
        HILOGI("HFP_HF connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hfService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
#endif
}

void ClassicAdapter::SetA2dpConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
    std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies) {
    if (a2dpSrcService_ != nullptr && IsProfileSupported(PROFILE_ID_A2DP_SRC, localUuids, remoteUuids)) {
        HILOG_COMM_INFO("A2DP_SRC connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        a2dpSrcService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
}

void ClassicAdapter::SetClassicConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
    std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies) {
    SetHfpConnectStrategy(device, isConnectAllEnabled, localUuids, remoteUuids, numSetProfilesStrategies);
    SetA2dpConnectStrategy(device, isConnectAllEnabled, localUuids, remoteUuids, numSetProfilesStrategies);
    if (hidHostService_ != nullptr && IsProfileSupported(PROFILE_ID_HID_HOST, localUuids, remoteUuids)
        && isConnectAllEnabled) {
        HILOGI("HID_HOST connect device, device: %{public}s", GET_ENCRYPT_ADDR(device));
        hidHostService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
    if (panService_ != nullptr && IsProfileSupported(PROFILE_ID_PAN, localUuids, remoteUuids)
        && isConnectAllEnabled) {
        HILOGI("PAN connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        panService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
}

void ClassicAdapter::SetBleConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
    std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies) {
    if (hearingAidService_ != nullptr && IsProfileSupported(PROFILE_ID_HEARING_AID, localUuids, remoteUuids)
        && isConnectAllEnabled) {
        HILOGI("HEARING_AID connect device, device: %{public}s", GET_ENCRYPT_ADDR(device));
        hearingAidService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    if (basService_ != nullptr && IsProfileSupported(PROFILE_ID_BAS, localUuids, remoteUuids)
        && isConnectAllEnabled) {
        HILOGI("BAS connect device: %{public}s", GET_ENCRYPT_ADDR(device));
        basService_->SetConnectStrategy(device, static_cast<int>(BTStrategyType::CONNECTION_ALLOWED));
        numSetProfilesStrategies++;
    }
#endif
}

void ClassicAdapter::SetConnectStrategy(const RawAddress &device, bool isConnectAllEnabled) {
    // if profile is supported, then connect profile by 'SetConnectStrategy(CONNECTION_ALLOWED) interface'
    std::vector<Uuid> localUuids = adapterProperties_->GetServiceUuids();
    std::vector<Uuid> remoteUuids = remoteDeviceProperties_->GetDeviceUuids(device);
    int32_t numSetProfilesStrategies = 0;
    SetClassicConnectStrategy(device, isConnectAllEnabled, localUuids, remoteUuids, numSetProfilesStrategies);
    SetBleConnectStrategy(device, isConnectAllEnabled, localUuids, remoteUuids, numSetProfilesStrategies);
    HILOGI("device: %{public}s and num of profiles: %{public}d", GET_ENCRYPT_ADDR(device), numSetProfilesStrategies);
}

void ClassicAdapter::DisconnectHfpProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    if (GetDeviceState<IProfileHfpAg>(agService_, device) == BTConnectState::CONNECTED) {
        HILOG_COMM_INFO("HFP_AG disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        agService_->Disconnect(device);
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (GetDeviceState<IProfileHfpHf>(hfService_, device) == BTConnectState::CONNECTED) {
        HILOGI("HFP_HF disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hfService_->Disconnect(device);
    }
#endif
}

void ClassicAdapter::DisconnectA2dpProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    if (GetDeviceState<IProfileA2dp>(a2dpSrcService_, device) == BTConnectState::CONNECTED) {
        HILOG_COMM_INFO("A2DP_SRC disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        a2dpSrcService_->Disconnect(device);
    }
}

void ClassicAdapter::DisconnectClassicProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    DisconnectHfpProfiles(device, isDisconnectAllEnabled);
    DisconnectA2dpProfiles(device, isDisconnectAllEnabled);
    if (GetDeviceState<IProfileHidHost>(hidHostService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("HID_HOST disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hidHostService_->Disconnect(device);
    }
    if (GetDeviceState<IProfilePbapPse>(pbapService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("PBAP_PSE disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        pbapService_->Disconnect(device);
    }
    if (GetDeviceState<IProfileMapMse>(mapService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("MAP_MSE disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        mapService_->Disconnect(device);
    }
    if (GetDeviceState<IProfilePan>(panService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("PAN disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        panService_->Disconnect(device);
    }
}

void ClassicAdapter::DisconnectBleProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    if (GetDeviceState<IProfileHearingAid>(hearingAidService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("HEARING_AID disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        hearingAidService_->Disconnect(device);
    }
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    if (GetDeviceState<IProfileBas>(basService_, device) == BTConnectState::CONNECTED
        && isDisconnectAllEnabled) {
        HILOGI("BAS disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        basService_->Disconnect(device);
    }
#endif
}

void ClassicAdapter::DisconnectProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (remoteDevice != nullptr && remoteDevice->GetIoCapability() != GAP_IO_DISPLAYYESNO &&
        remoteDevice->GetCustomType() != static_cast<int>(DEVICE_TYPE_CAR) &&
        SatisfyDisconnectAclCondition(device, isDisconnectAllEnabled)) {
        HILOG_COMM_INFO("ACL disconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        DisconnectAcl(device.GetAddress());
    }
    DisconnectClassicProfiles(device, isDisconnectAllEnabled);
    DisconnectBleProfiles(device, isDisconnectAllEnabled);
}

void ClassicAdapter::CheckAndSetVirtualAutoConnSwitch(const RawAddress &device)
{
    HILOGI("GetState:%{public}d, GetRestrictedState:%{public}d",
        AdapterManager::GetInstance()->GetState(bluetooth::BTTransport::ADAPTER_BREDR),
        AdapterManager::GetInstance()->GetRestrictedState(bluetooth::BTTransport::ADAPTER_BREDR));
    if (AdapterManager::GetInstance()->IsSupportVirtualAutoConnect(device.GetAddress())) {
        BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device.GetAddress(), false);
    }
}

void ClassicAdapter::DisconnectAllowedProfiles(const RawAddress &device, bool isDisconnectAllEnabled)
{
    DisconnectProfiles(device, isDisconnectAllEnabled);
    CheckAndSetVirtualAutoConnSwitch(device);
}

void ClassicAdapter::DisconnectAllowedProfilesNotSetVirtualConnSwitch(const RawAddress &device)
{
    DisconnectProfiles(device);
}

void ClassicAdapter::ResumeBleStackDevice()
{
    hidHostService_ = static_cast<IProfileHidHost *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_HOST));
    if (hidHostService_ != nullptr) {
        hidHostService_->ReStartStackHidProfile();
    }
    hearingAidService_ = static_cast<IProfileHearingAid *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HEARING_AID));
    if (hearingAidService_ != nullptr) {
        hearingAidService_->StartStackProfile();
    }
}

int32_t ClassicAdapter::FactoryReset()
{
    HILOGI("Start FactoryReset");
    DeleteDeviceInfoFiles();
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    CHECK_AND_RETURN_LOG_RET(status == BT_NO_ERROR,
        BT_ERR_INTERNAL_ERROR, "[ClassicAdapter] Failed to open the Bluetooth module");
    int ret = btInterface->config_clear();
    CHECK_AND_RETURN_LOG_RET(ret == BT_STATUS_SUCCESS, BT_ERR_INTERNAL_ERROR, "clear config failed");
    return BT_NO_ERROR;
}

void ClassicAdapter::DeleteDeviceInfoFiles()
{
    std::string filePath = "/data/service/el1/public/bluetooth";
    std::vector<std::string> files;
    GetDirFiles(filePath, files);
    for (auto &file : files) {
        if (file.rfind(".xml") != std::string::npos) {
            remove(file.c_str());
        }
    }
}

std::string UintToHex(uint32_t input)
{
    std::string dictionary = "0123456789ABCDEF";
    std::string retStr = "";
    uint32_t remain = 0;
    uint32_t hex_step = 16;
    if (input == 0) {
        retStr = "0";
    }
    while (input != 0) {
        remain = input % hex_step;
        retStr = dictionary[remain] + retStr;
        input = input / hex_step;
    }
    return retStr;
}

std::string FormatModelIdInfo(const std::string &input, uint32_t len)
{
    std::string ret = "";
    if (input.size() > len) {
        ret = input.substr(0, len);
        return ret;
    }
    for (uint32_t i = 0; i < len - input.size(); i++) {
        ret += "0";
    }
    ret += input;
    return ret;
}

bool ClassicAdapter::GetHwPropertyInfo(const RawAddress &device, bt_property_type_t type, std::string &property,
    bool toHex)
{
    auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, false, "Failed to get hw bluetooth interface");

    std::string info = "";
    uint32_t value = 0;
    bt_property_t prop;
    prop.type = type;

    if (type == BT_PROPERTY_REMOTE_NEW_MODEL_ID) {
        prop.len = sizeof(uint32_t);
        prop.val = &info;
    } else {
        prop.len = sizeof(uint32_t);
        prop.val = &value;
    }

    BLUEDROID::RawAddress dev = ServiceUtil::AddrToBluedroid(device);
    if (!bthwif->hwGetDeviceInfo(dev, prop)) {
        return false;
    }
    if (type == BT_PROPERTY_REMOTE_NEW_MODEL_ID) {
        property = info;
    } else {
        property = toHex ? UintToHex(value) : std::to_string(value);
    }
    return true;
}

std::string ClassicAdapter::GetDeviceModelId(const RawAddress &device)
{
    // modeId和newModeId是互斥的,两者只会存在一个
    std::string modeId = "";
    bool result = GetHwPropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, modeId);
    if (!result || modeId == "0") { // 000000 表示获取到的modeiId为空，改为获取newmodelId
        if (!GetHwPropertyInfo(device, BT_PROPERTY_REMOTE_NEW_MODEL_ID, modeId)) {
            modeId = "FFFFFF";
        }
    } else {
        modeId = FormatModelIdInfo(modeId, DEVICE_MODEL_ID_LEN);
    }

    std::string subModelId = "";
    if (!GetHwPropertyInfo(device, BT_PROPERTY_REMOTE_SUB_MODEL_ID, subModelId)) {
        subModelId = "FF";
    } else {
        subModelId = FormatModelIdInfo(subModelId, DEVICE_SUBMODEL_ID_LEN);
    }

    std::string iconId = "";
    if (!GetHwPropertyInfo(device, BT_PROPERTY_DEVICE_INFO, iconId)) {
        iconId = "FFFF";
    } else {
        iconId = FormatModelIdInfo(iconId, DEVICE_ICON_ID_LEN);
    }
    std::string productId = modeId + "_" + subModelId + "_" + iconId;
    HILOGI_TIME_LIMIT(GET_ENCRYPT_ADDR(device), "addr: %{public}s productId: %{public}s",
        GET_ENCRYPT_ADDR(device), productId.c_str());
    return productId;
}

bool ClassicAdapter::GetRemoteDevicePropertyInfo(const RawAddress &device, bt_property_type_t type,
    std::string &property)
{
    return GetHwPropertyInfo(device, type, property, false);
}

int32_t ClassicAdapter::SetDeviceCustomType(const RawAddress &device, int32_t customType) const
{
    CHECK_AND_RETURN_LOG_RET(
        IsCustomTypeSupported(device, customType), BT_ERR_INVALID_PARAM, "device not support custom");
    HILOG_COMM_INFO("SetDeviceCustomType: %{public}s, customtype: %{public}d", GET_ENCRYPT_ADDR(device), customType);
    int32_t res = remoteDeviceProperties_->SetCustomType(device, customType);
    CHECK_AND_RETURN_LOG_RET(res == BT_NO_ERROR, BT_ERR_INVALID_PARAM, "set custom type failed");
    HfpAgService *hfpService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG_RET(hfpService != nullptr, BT_ERR_INVALID_PARAM, "get hfp service failed!");
    hfpService->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_UPDATE_DEVICE_TYPE, device);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG_RET(a2dpService != nullptr, BT_ERR_INVALID_PARAM, "get a2dp service failed!");
    a2dpService->NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_UPDATE_DEVICE_TYPE, device);
    return res;
}

bool ClassicAdapter::IsCustomTypeSupported(const RawAddress &device, int32_t customType) const
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    std::vector<Uuid> localUuids = adapterProperties_->GetServiceUuids();
    std::vector<Uuid> remoteUuids = remoteDeviceProperties_->GetDeviceUuids(device);
    if (remoteDevice == nullptr || remoteDevice->GetPairedStatus() != PAIR_PAIRED) {
        HILOGI_TIME_LIMIT(__func__, "(IsCustomTypeSupported)device not found or not paired");
        return false;
    }
    if (customType < DEVICE_TYPE_DEFAULT || customType > DEVICE_TYPE_SPEAKER) {
        HILOGE("custom type is invalid");
        return false;
    }
    if (!(IsProfileSupported(PROFILE_ID_HFP_AG, localUuids, remoteUuids) ||
        IsProfileSupported(PROFILE_ID_A2DP_SRC, localUuids, remoteUuids))) {
        HILOGE("device not support hfp and a2dp");
        return false;
    }
    return true;
}

bool ClassicAdapter::IsIncomingConnection(const RawAddress &device)
{
    auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, false, "Failed to get hw bluetooth interface");

    BLUEDROID::RawAddress dev = ServiceUtil::AddrToBluedroid(device);
    bool ret = bthwif->isIncomingConnection(dev);
    return ret;
}

int32_t ClassicAdapter::GetRemoteDeviceInfo(const RawAddress &device,
    Bluetooth::BluetoothRemoteDeviceInfo &deviceInfo)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG_RET(remoteDevice != nullptr, BT_ERR_INVALID_PARAM, "device not exist");

    deviceInfo.vendorId_ = remoteDevice->GetVendorId();
    deviceInfo.productId_ = remoteDevice->GetProductId();
    deviceInfo.modelId_ = GetDeviceModelId(device);
    deviceInfo.deviceOfType_ = remoteDevice->GetHwRemoteDeviceType();
    
    auto manufacturerData = remoteDevice->GetManufacturerSpecificData();
    deviceInfo.manufacturerData_ = manufacturerData;

    if (IsCustomTypeSupported(device, deviceInfo.customType_)) {
        deviceInfo.customType_ = remoteDevice->GetCustomType();
        HILOGI_TIME_LIMIT(GET_ENCRYPT_ADDR(device), "addr: %{public}s, customtype: %{public}d",
            GET_ENCRYPT_ADDR(device), deviceInfo.customType_);
    } else {
        HILOGI_TIME_LIMIT(GET_ENCRYPT_ADDR(device), "GetRemoteDeviceInfo: device not support custom");
    }

    return BT_NO_ERROR;
}

bool ClassicAdapter::IsNeededReturnCodForAudio(int32_t uid, int32_t customType) const
{
    if (uid == UID_AUDIO) {
        auto it = customTypeToCod.find(static_cast<DeviceType>(customType));
        if (it != customTypeToCod.end()) {
            return true;
        }
    }
    return false;
}

void ClassicAdapter::DisconnectPairedDeviceProfiles()
{
    std::vector<RawAddress> rawAddrVec = GetPairedDevices();
    for (auto &device : rawAddrVec) {
        DoInClassicThread([this, device] {this->DisconnectAllowedProfiles(device);});
    }
    return;
}

void ClassicAdapter::PassivePairToDelayConnect(const RawAddress &device)
{
    int delayTimeConnectAfterPassivePaired = 3000; // delay time to connect after passive paired. 3000 ms
    ThreadUtil::GetInstance().PostTask(THREAD_ID_CLASSIC,
        [this, device] {this->ConnectAllowedProfiles(device);},
        delayTimeConnectAfterPassivePaired, "BluetoothPassivePairedDelayConnect");
}

void ClassicAdapter::GetPeripheralCod(const RawAddress &device, int32_t &cod) const
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
    if (remoteDevice == nullptr || remoteDevice->GetVendorId() != VENDOR_ID_OF_HW_DEVICE) {
        return;
    }

    int32_t productId = remoteDevice->GetProductId();
    HILOGD("device %{public}s productId = %{public}d ", GET_ENCRYPT_ADDR(device), productId);

    auto isPeripheral = [productId](const std::vector<int32_t>& it) {
        return std::find(it.begin(), it.end(), productId) != it.end();
    };
    if (isPeripheral(PENCIL_PRODUCT_ID_LIST)) {
        cod = CLASS_OF_DEVICE_HID_PENCIL;
        return;
    }
    if (isPeripheral(KEYBOARD_PRODUCT_ID_LIST)) {
        cod = CLASS_OF_DEVICE_HID_KEYBOARD;
        return;
    }
    if (isPeripheral(MOUSE_PRODUCT_ID_LIST)) {
        cod = CLASS_OF_DEVICE_HID_POINTING;
        return;
    }
}

bool ClassicAdapter::IsNeedDelayConnect(const RawAddress &device)
{
    return remoteDeviceProperties_->IsNeedDelayConnect(device);
}

bool ClassicAdapter::IsDisconnectAclBeforeBond(const std::string device) const
{
    std::lock_guard<BtRecursiveMutex> lk(pimpl->disconnectAclBeforeBondDeviceMutex_);
    return disconnectAclBeforeBondDevices_.find(device) != disconnectAclBeforeBondDevices_.end();
}

void ClassicAdapter::AddDisconnectAclBeforeBondDevice(const std::string device)
{
    std::lock_guard<BtRecursiveMutex> lk(pimpl->disconnectAclBeforeBondDeviceMutex_);
    disconnectAclBeforeBondDevices_.insert(device);
}

void ClassicAdapter::DelDisconnectAclBeforeBondDevice(const std::string device)
{
    std::lock_guard<BtRecursiveMutex> lk(pimpl->disconnectAclBeforeBondDeviceMutex_);
    disconnectAclBeforeBondDevices_.erase(device);
}

void ClassicAdapter::CheckDeviceUuidsAndSendChr(std::shared_ptr<BluetoothDevice> remoteDevice)
{
    if (remoteDevice == nullptr) {
        return;
    }
    std::vector<Uuid> deviceUuids = remoteDevice->GetDeviceUuids();
    if (deviceUuids.empty() ||
        (deviceUuids.size() == 1 && deviceUuids[0].ToString().compare(EMPTY_UUID) == 0)) {
        HILOGI("remoteDevice: %{public}s", GET_ENCRYPT_STR_ADDR(remoteDevice->GetAddress()));
        BtChrBtExcpEvent(remoteDevice->GetAddress(), BTOPT_DEVICE_UUIDS_EMPTY, CHR_SUB_ERRCODE_CASE1);
    }
}

int32_t ClassicAdapter::ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
    uint32_t controlTypeVal, uint32_t controlObject)
{
    HILOGI("deviceId: %{public}s, controlType: %{public}u, controlTypeVal: %{public}u, \
        controlObject: %{public}u", GetEncryptAddr(deviceId).c_str(), controlType, controlTypeVal, controlObject);
    uint8_t addrBuf[BD_ADDR_LEN];
    uint8_t *addrStream = addrBuf;
    RawAddress remoteAddr(deviceId);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(remoteAddr);
    if (rawAddr.IsEmpty()) {
        HILOGI("rawAddr is empty.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (memcpy_s(addrStream, BD_ADDR_LEN, rawAddr.address, BD_ADDR_LEN) != EOK) {
        HILOGI("memcpy_s addrBuf failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothHwInterface::GetInstance()->HwHiechoDeviceCtrl(addrBuf, controlType, controlTypeVal, controlObject);
    return BT_NO_ERROR;
}

int32_t ClassicAdapter::GetLastConnectionTime(const RawAddress &device, int64_t &connectionTime)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG_RET(remoteDevice != nullptr, BT_ERR_INVALID_PARAM, "device not exist");
    connectionTime = remoteDevice->GetLastConnectionTime();
    HILOGI("remoteDevice: %{public}s %{public}ld", GET_ENCRYPT_ADDR(device), connectionTime);
    return BT_NO_ERROR;
}

void ClassicAdapter::SetConnectionTime(const RawAddress &device, int64_t connectionTime)
{
    remoteDeviceProperties_->SetConnectionTime(device, connectionTime);
    HILOGI("remoteDevice: %{public}s %{public}ld", GET_ENCRYPT_ADDR(device), connectionTime);
}

int32_t ClassicAdapter::GetCloudBondState(const RawAddress &device, int32_t &cloudBondState)
{
    cloudBondState = IsCloudDevice(device) ? remoteDeviceProperties_->GetCloudBondState(device) : 0;
    return BT_NO_ERROR;
}

int32_t ClassicAdapter::GetDeviceTransport(const RawAddress &device, int32_t &transport)
{
    int deviceType = remoteDeviceProperties_->GetDeviceType(device);
    switch (deviceType) {
        case DEVICE_TYPE_BREDR:
            transport = TRANSPORT_BR_EDR;
            break;
        case DEVICE_TYPE_LE:
            transport = TRANSPORT_LE;
            break;
        case DEVICE_TYPE_DUAL_MONO:
            transport = TRANSPORT_DUAL;
            break;
        default:
            transport = TRANSPORT_UNKNOWN;
            break;
    }
    return BT_NO_ERROR;
}

int32_t ClassicAdapter::UpdateCloudBluetoothDevice(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList)
{
    DoInClassicThread([this, cloudList] {
        this->UpdateCloudBluetoothDeviceInner(
            const_cast<std::vector<Bluetooth::BluetoothTrustPairDevice> &>(cloudList));
    });
    return BT_NO_ERROR;
}

bool ClassicAdapter::StartCloudPair(const RawAddress &device)
{
    if (!remoteDeviceProperties_->IsCloudDevice(device)) {
        return false;
    }
    int32_t cloudBondState = remoteDeviceProperties_->GetCloudBondState(device);
    if (cloudBondState == CLOUD_BOND_REMOVING || cloudBondState == CLOUD_BOND_CREATE_BOND) {
        HILOGI("[CLOUD_PAIR] %{public}s cloud bond removing or bonding, state=%{public}d",
            GET_ENCRYPT_ADDR(device), cloudBondState);
        return true;
    } else if (cloudBondState == CLOUD_BOND_BONDING) {
        remoteDeviceProperties_->SetCloudBondState(device, CLOUD_BOND_CREATE_BOND);
        DisconnectAcl(device.GetAddress());
        HILOGI("[CLOUD_PAIR] %{public}s disconnect acl first", GET_ENCRYPT_ADDR(device));
        return true;
    }
    return false;
}

void ClassicAdapter::SetCloudBondState(const RawAddress &device, int32_t cloudBondState)
{
    remoteDeviceProperties_->SetCloudBondState(device, cloudBondState);
}

int32_t ClassicAdapter::GetCloudBondState(const RawAddress &device) const
{
    return remoteDeviceProperties_->GetCloudBondState(device);
}

bool ClassicAdapter::IsStartCloudPair(const RawAddress &device)
{
    return remoteDeviceProperties_->IsStartCloudPair(device);
}

bool ClassicAdapter::IsCloudDevice(const RawAddress &device)
{
    return remoteDeviceProperties_->IsCloudDevice(device);
}

void ClassicAdapter::ClearCloudDevice(std::shared_ptr<BluetoothDevice> &device)
{
    if (device == nullptr || !device->IsCloudDevice() || CloudDeviceManager::GetInstance() == nullptr) {
        return;
    }
    HILOGI("[CLOUD_PAIR] device:%{public}s, current state:%{public}d",
        GetEncryptAddr(device->GetAddress()).c_str(), device->GetCloudBondState());
    if (device->GetCloudBondState() == CLOUD_BOND_BONDED) {
        device->SetCloudBondState(CLOUD_BOND_NONE);
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(RawAddress(device->GetAddress()),
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::RESET_DEV_CLOUD_PAIR_STATE));
    }
    CloudDeviceManager::GetInstance()->DelCloudDeviceProperties(device);
}

bool ClassicAdapter::IsInCloudBondingState(std::shared_ptr<BluetoothDevice> &device)
{
    if (device == nullptr || !device->IsCloudDevice()) {
        return false;
    }
    if (device->GetCloudBondState() == CLOUD_BOND_BONDED || device->GetCloudBondState() == CLOUD_BOND_NONE) {
        HILOGI("[CLOUD_PAIR] %{public}s now in CloudBondingState, current state:%{public}d",
            GetEncryptAddr(device->GetAddress()).c_str(), device->GetCloudBondState());
        return true;
    }
    return false;
}

void ClassicAdapter::NotifyDisconnectState(const RawAddress &device, int32_t cause)
{
    auto a2dpPtr = static_cast<A2dpService *>(a2dpSrcService_);
    if (a2dpPtr == nullptr) {
        return;
    }
    a2dpPtr->NotifyConnStateChanged(device, static_cast<int>(BTConnectState::DISCONNECTED), cause);
}

bool ClassicAdapter::CheckBondStateAndReturn(std::shared_ptr<BluetoothDevice> &remoteDevice, int unbondCause)
{
    if (remoteDevice == nullptr || !remoteDevice->IsCloudDevice()) {
        return false;
    }
    if (unbondCause == UNBOND_CAUSE_USER_REMOVED) {
        ClearCloudDevice(remoteDevice);
    } else {
        int32_t preCloudBondState = remoteDevice->GetCloudBondState();
        remoteDevice->SetCloudBondState(CLOUD_BOND_NONE);
        remoteDevice->SetPairedStatus(PAIR_PAIRED);
        NotifyDisconnectState(RawAddress(remoteDevice->GetAddress()));
        if (preCloudBondState != CLOUD_BOND_NONE) {
            BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(RawAddress(remoteDevice->GetAddress()),
                static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
                static_cast<int>(UeConnCloudPairSubSceneCode::BONDING_FAIL));
        }
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(RawAddress(remoteDevice->GetAddress()),
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::RESET_DEV_CLOUD_PAIR_STATE));
        return true;
    }
    return false;
}

void ClassicAdapter::HandleBondStateFailed(bt_status_t status, std::shared_ptr<BluetoothDevice> remoteDevice,
    const RawAddress &device)
{
    if (remoteDevice == nullptr) {
        return;
    }
    int unbondCause = UNBOND_CAUSE_INTERNAL_ERROR;
    std::string causeMessage = UNBOND_MSG_UNKNOWN;
    CovertUnbondCause(status, unbondCause, causeMessage);
    if (remoteDevice->IsCloudDevice()) {
        if (remoteDevice->GetCloudBondState() == CLOUD_BOND_BONDED) {
            BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(RawAddress(remoteDevice->GetAddress()),
                static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
                static_cast<int>(UeConnCloudPairSubSceneCode::BONDING_FAIL));
        }
        remoteDevice->SetBondedFromLocal(false);
        remoteDevice->SetCloudBondState(CLOUD_BOND_NONE);
        remoteDevice->SetPairedStatus(PAIR_PAIRED);
        NotifyDisconnectState(device);
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(RawAddress(remoteDevice->GetAddress()),
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::RESET_DEV_CLOUD_PAIR_STATE));
        BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), CHR_ERRCODE, ERRCODE_PAIR_FAILED);
        return;
    }
    remoteDevice->SetPairedStatus(PAIR_NONE);
    SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_NONE, unbondCause, causeMessage);
    BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), CHR_ERRCODE, ERRCODE_PAIR_FAILED);
    BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), CHR_SUB_ERRCODE, unbondCause);
}

void ClassicAdapter::HandleBondStateBonding(std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device)
{
    int preState = remoteDevice->GetPairedStatus();
    remoteDevice->SetPairedStatus(PAIR_PAIRING);
    if (!IsInCloudBondingState(remoteDevice)) {
        SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_PAIRING, PAIR_COMMON_BOND_CAUSE, BOND_MSG_NO_ERROR);
    }
    if (preState == PAIR_PAIRED) {
        HILOGI("preState: %{public}d, reset pbap map premission.", preState);
        IAdapterManager::GetInstance()->SetPhonebookPermission(device.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
        IAdapterManager::GetInstance()->SetMessagePermission(device.GetAddress(), BTPermissionType::ACCESS_UNKNOWN);
    }
}

void ClassicAdapter::HandleBondStateBondNone(bt_status_t status, BLUEDROID::RawAddress bd_addr,
    std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device, bt_bond_state_t state)
{
    if (remoteDevice == nullptr) {
        return;
    }
    int preState = remoteDevice->GetPairedStatus();
    HILOGI("preState: %{public}d ; bond_state: %{public}d ", preState, state);
    int unbondCause = UNBOND_CAUSE_INTERNAL_ERROR;
    std::string causeMessage = UNBOND_MSG_UNKNOWN;
    CovertUnbondCause(status, unbondCause, causeMessage);
    if (CheckBondStateAndReturn(remoteDevice, unbondCause)) {
        return;
    }
    if (preState == PAIR_PAIRED) {
        BtChrDftEventWriteInt(CHR_BT_PAIR_EXCEPTION, device.GetAddress(), CHR_ERRCODE, ERRCODE_REMOVE_PAIR);
        remoteDevice->SetAliasName(INVALID_NAME);
        remoteDevice->SetCustomType(DEVICE_TYPE_DEFAULT);
    }
    remoteDevice->SetPairedStatus(PAIR_NONE);
    ClearSavedDeviceInfo(bd_addr, device);
    SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_NONE, PAIR_COMMON_BOND_CAUSE, causeMessage);
}

int32_t ClassicAdapter::UpdateCloudBluetoothDeviceInner(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList)
{
    CloudDeviceManager::GetInstance()->DelCloudBluetoothDev(cloudList);
    CloudDeviceManager::GetInstance()->AddCloudBluetoothDev(cloudList);
    return BT_NO_ERROR;
}

void ClassicAdapter::SetBondState(bt_status_t status, BLUEDROID::RawAddress bd_addr, bt_bond_state_t state)
{
    RawAddress device = ServiceUtil::AddrFromBluedroid(bd_addr);
    HILOGI("device: %{public}s status: %{public}d SetBondState: %{public}d", GET_ENCRYPT_ADDR(device), status, state);
    std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties_->GetBluetoothDeviceFromMap(device);
    if (!remoteDevice) {
        return;
    }
    remoteDevice->SetPairConfirmState(PAIR_CONFIRM_STATE_INVALID);
    remoteDevice->SetPairConfirmType(PAIR_CONFIRM_TYPE_INVALID);
    remoteDevice->SetPairedStatus(PAIR_PAIRED);
    adapterProperties_->AddPairedAddrList(device);
    SendPairStatusChanged(ADAPTER_BREDR, device, PAIR_PAIRED, PAIR_COMMON_BOND_CAUSE, BOND_MSG_NO_ERROR);
}

std::string ClassicAdapter::GetCarKeyDfxData() const
{
    return CarKey::GetChrData();
}

void ClassicAdapter::SetCarKeyCardData(const std::string &address, int32_t action)
{
    CarKey::SetCardChrInfo(address, action);
}

void ClassicAdapter::InitCarKeyInfo()
{
    CarKey::InitCardInfo();
}

void ClassicAdapter::AddPendingConnectDevice(const RawAddress &address, uint32_t profileId)
{
    deviceProfileAddressMap_.ChangeValueByLambda<>(address,
        [profileId](std::set<int> &profileSet) {
            profileSet.insert(profileId);
        });
    HILOGI("address: %{public}s, profileId: %{public}d", GET_ENCRYPT_ADDR(address), profileId);
}

IProfile* ClassicAdapter::GetProfileService(uint32_t profileId) const
{
    std::string profileName = SupportProfilesInfo::IdToName(profileId);
    if (profileName.empty()) {
        HILOGE("unknown profileId: %{public}d", profileId);
        return nullptr;
    }
    return IProfileManager::GetInstance()->GetProfileService(profileName);
}

void ClassicAdapter::IsDeviceReadyConnect(const RawAddress &address)
{
    std::set<int> profileSet = {};
    if (!deviceProfileAddressMap_.Find(address, profileSet)) {
        HILOGE("device:%{public}s not need to connect Profile.", GET_ENCRYPT_ADDR(address));
        return;
    }
    std::vector<Uuid> localUuids = adapterProperties_->GetServiceUuids();
    std::vector<Uuid> remoteUuids = remoteDeviceProperties_->GetDeviceUuids(address);
    deviceProfileAddressMap_.ChangeValueByLambda<>(address,
        [this, &address, &localUuids, &remoteUuids](std::set<int> &profileSet) {
            for (const auto& profileId : profileSet) {
                if (!IsProfileSupported(profileId, localUuids, remoteUuids)) {
                    continue;
                }
                IProfile* profileService = GetProfileService(profileId);
                if (profileService != nullptr) {
                    profileService->Connect(address);
                }
            }
            profileSet.clear();
        });
    deviceProfileAddressMap_.Erase(address);
}

void ClassicAdapter::RemovePendingConnectDevice(const RawAddress &address)
{
    deviceProfileAddressMap_.Erase(address);
}

bool ClassicAdapter::SatisfyDisconnectAclCondition(const RawAddress &device, bool isSystemHap)
{
    if (isSystemHap) {
        return true;
    }
    if (GetDeviceState<IProfileHidHost>(hidHostService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
    if (GetDeviceState<IProfilePbapPse>(pbapService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
    if (GetDeviceState<IProfileMapMse>(mapService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
    if (GetDeviceState<IProfileHearingAid>(hearingAidService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
    if (GetDeviceState<IProfilePan>(panService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
#ifdef BLUETOOTH_HFP_HF_ENABLE
    if (GetDeviceState<IProfileHfpHf>(hfService_, device) == BTConnectState::CONNECTED) {
        return false;
    }
#endif
    return GetDeviceState<IProfileA2dp>(a2dpSrcService_, device) == BTConnectState::CONNECTED &&
        GetDeviceState<IProfileHfpAg>(agService_, device) == BTConnectState::CONNECTED;
}

void ClassicAdapter::NotifyBondStateChanged(bt_status_t status, BLUEDROID::RawAddress bd_addr,
    bt_bond_state_t state)
{
    auto a2dpPtr = static_cast<A2dpService *>(a2dpSrcService_);
    if (a2dpPtr != nullptr) {
        RawAddress device = ServiceUtil::AddrFromBluedroid(bd_addr);
        a2dpPtr->NotifyBondStateChanged(status, device, state);
    }
}

REGISTER_CLASS_CREATOR(ClassicAdapter);
}  // namespace bluetooth
}  // namespace OHOS
