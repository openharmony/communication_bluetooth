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
#define LOG_TAG "bt_service_watch"
#endif

#include "watch_service.h"
#include "bt_def.h"
#include "a2dp_def.h"
#include "adapter_manager.h"
#include "service_util.h"
#include "profile_config.h"
#include "connect_strategy_manager.h"
#include "hfp_ag_system_interface.h"
#include "hfp_hf_service.h"
#include "common_util.h"
#include "bluetooth_common_event_helper.h"
#include "gavdp/a2dp_service.h"
#include "interface_profile_a2dp_src.h"
#include "bluetooth_datashare_utils.h"
#include "adapter_config.h"
#include "bt_chr_base.h"
#include "bt_chr_dft_exception.h"
#include "json_utils.h"
#include "watch_device_manager.h"
#include "preferences_manager.h"
#include "parameter.h"
#include <regex>

namespace OHOS {
namespace bluetooth {

void BluetoothWatchServiceSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    HILOGI("BluetoothWatchServiceSubscriber OnReceiveEvent:%{public}s", action.c_str());
    if (action == EVENT_BLUETOOTH_HOST_CONNECT_HFP) {
        auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
        if (deviceManager) {
            std::string addr = deviceManager->GetAddressLocked();
            DoInVendorThread([addr] {
                auto dm = GlobalDeviceManager::getInstance().GetDevice(addr);
                if (dm && dm->IsSaisDeviceLocked()) {
                    WatchService::GetInstance()->ConnectHfp(addr);
                }
            });
        }
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        WatchService::GetInstance()->ConnectBondedPhone();
    }
    if (action == ENTER_SYSTEM_STR_MODE) {
        WatchService::GetInstance()->ChangeStatusOfEnterStrMode();
    }
    if (action == EVENT_COMMON_LOCAL_DISCONNECT) {
        std::string remoteAddr = want.GetParams().GetStringParam("peerMac");
        CHECK_AND_RETURN_LOG(WatchService::GetInstance()->IsValidAddress(remoteAddr), "remoteAddr is invalid");
        CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is NULL");
        classicAdapter->DisconnectAcl(remoteAddr);
    }
}

WatchService *WatchService::GetInstance()
{
    static WatchService instance;
    return &instance;
}

void WatchService::Initialize()
{
    GetInstance()->Start();
}

void WatchService::Uninitialize()
{
    GetInstance()->Stop();
}

void WatchService::NotifyBluetoothTurningOff()
{
    if (!isBluetoothDisable_) {
        WatchService::GetInstance()->UpdateReconnectState(false);
        isBluetoothDisable_ = true;
        ChangeScanMode();
    }
}

WatchService::WatchService()
{
}

WatchService::~WatchService()
{
}

static void WatchDisconnectRssiCb(const OHOS::bluetooth::RawAddress* bdAddr, int rssi)
{
    RawAddress rawAddr = *bdAddr;
    HILOGI("watch_service: rssi=%{public}d", rssi);
    WatchService::GetInstance()->SetLinkLossRssi(rawAddr, rssi);
}

static void WatchSaisConnectionStateCb(const OHOS::bluetooth::RawAddress* bdAddr, bool isSaisConnected)
{
    HILOGI("watch_service: sais connected=%{public}d", isSaisConnected);
    RawAddress rawAddr = *bdAddr;
    DoInVendorThread([rawAddr, isSaisConnected] {
        WatchService::GetInstance()->SetSaisConnectionState(rawAddr, isSaisConnected);
    });
}

static void WatchA2dpOfldPlayStateCallCb(uint8_t playState)
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService == nullptr) {
        return;
    }
    RawAddress addr = a2dpService->GetActiveSinkDevice();
    std::vector<int32_t> sessionsId;
    sessionsId.push_back(WATCH_MCU_PLAY_ID);
    if (playState == MCU_START_PLAY) {
        std::vector<A2dpSrcStreamInfo> streamsInfo;
        A2dpSrcStreamInfo stream;
        stream.sessionId = WATCH_MCU_PLAY_ID;
        stream.streamType = StreamType::STREAM_MUSIC;
        streamsInfo.push_back(stream);
        int ret = a2dpService->A2dpOffloadSessionPathRequest(addr, streamsInfo);
        if (ret != HW_A2DP_OFFLOAD_HARDWARE_ENCODING) {
            return;
        }
        a2dpService->OffloadStartPlaying(addr, sessionsId);
    } else if (playState == MCU_STOP_PLAY) {
        a2dpService->OffloadStopPlaying(addr, sessionsId);
    } else if (playState == MCU_CHECK_DEVICE) {
        a2dpService->ForceStopOffloadPlaying(addr);
    } else {
        HILOGI("watch_service: mcu offload play do nothing");
    }
}

static void WatchSaisServerAddedCb()
{
    HILOGI("watch_service: SAIS ADD");
    WatchService::GetInstance()->SetSaisAddStatus(true);
}

static void HwWatchLeSaisEnableCb(const OHOS::bluetooth::RawAddress* bdAddr)
{
    HILOGI("watch_service: HwWatchLeSaisEnableCb");
    RawAddress rawAddr = *bdAddr;
    WatchService::GetInstance()->SetPhoneConnHfp(rawAddr.GetAddress(), false);
}

static void HwWatchBrHidCtrlCb(uint8_t act)
{
    BrHidControlAction userAction = static_cast<BrHidControlAction>(act);
    HILOGI("HwWatchBrHidCtrlCb: act: %{public}d, userAction: %{public}d", act, userAction);
    switch (userAction) {
        case BrHidControlAction::BR_HID_CONTROL_ACTION_CMD_VOLUME_INCREMENT:
            WatchService::GetInstance()->HidVolumeIncrement();
            break;
        case BrHidControlAction::BR_HID_CONTROL_ACTION_CMD_VOLUME_DECREMENT:
            WatchService::GetInstance()->HidVolumeDecrement();
            break;
        case BrHidControlAction::BR_HID_CONTROL_ACTION_CMD_DOUBLE_CLICK:
            WatchService::GetInstance()->HidDoubleClick();
            break;
        case BrHidControlAction::BR_HID_CONTROL_ACTION_CMD_SINGLE_CLICK:
            WatchService::GetInstance()->HidSingleClick();
            break;
        default:
            break;
    }
}

static void HwWatchLinkLossNotifyCb()
{
    WatchService::GetInstance()->SendConnectionTimtout();
}

static BtWatchCallbacks g_sBluetoothWatchCallbacks = {
    sizeof(g_sBluetoothWatchCallbacks),
    WatchDisconnectRssiCb,
    WatchSaisConnectionStateCb,
    WatchA2dpOfldPlayStateCallCb,
    WatchSaisServerAddedCb,
    HwWatchLeSaisEnableCb,
    HwWatchBrHidCtrlCb,
    HwWatchLinkLossNotifyCb,
};

static void HidConnectionStateCb(OHOS::bluetooth::RawAddress* bdAddr, BthdConnectionState state)
{
    RawAddress rawAddr = *bdAddr;
    auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
    HILOGI("watch_service: HID connection state=%{public}d, isIphone: %{public}d",
        state, WatchService::GetInstance()->IsIphone(rawAddr));
    if (deviceManager) {
        bool isConnected = deviceManager->IsConnectedLocked();
        std::string addr = deviceManager->GetAddressLocked();
        DoInVendorThread([rawAddr, state, isConnected, addr] {
            auto dm = GlobalDeviceManager::getInstance().GetDevice(addr);
            if (!dm) {
                return;
            }
            WatchService::GetInstance()->SetHidConnectionState(rawAddr, state);
            if (!WatchService::GetInstance()->IsIphone(rawAddr)) {
                if (state == BTHD_CONN_STATE_CONNECTED) {
                    WatchService::GetInstance()->HidUpdateMap();
                } else if (state == BTHD_CONN_STATE_DISCONNECTED) {
                    WatchService::GetInstance()->HidReconnectAfterUpdateMap(rawAddr, isConnected);
                }
            }
        });
    }
}

static void HidVirtualCableUnplugCb()
{
    HILOGI("WatchService::HidVirtualCableUnplugCb isHidMapUpdateing_: %{public}d",
        WatchService::GetInstance()->HidGetMapUpdateingState());
    if (WatchService::GetInstance()->HidGetMapUpdateingState()) {
        return;
    }
    WatchService::GetInstance()->ProcessRemovePair();
}

static BthdCallbacks g_sBluetootHDCallbacks = {
    sizeof(g_sBluetootHDCallbacks),
    nullptr,
    HidConnectionStateCb,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    HidVirtualCableUnplugCb,
};

void WatchService::RegisterHid()
{
    BthdAppParam appParam;
    BthdQosParam inQos;
    BthdQosParam outQos;
    appParam.name = NAME;
    appParam.description = DESCRIPTION;
    appParam.provider = PROVIDER;
    appParam.subclass = g_subclass;
    appParam.descList = const_cast<uint8_t*>(g_hidData);
    appParam.descListLen = sizeof(g_hidData);
    inQos.serviceType = 0x01;
    inQos.tokenRate = 0;
    inQos.tokenBucketSize = 0;
    inQos.peakBandwidth = 0;
    inQos.accessLatency = 0xffffffff;
    inQos.delayVariation = 0xffffffff;
    outQos.serviceType = 0x02;
    outQos.tokenRate = 0;
    outQos.tokenBucketSize = 0;
    outQos.peakBandwidth = 0;
    outQos.accessLatency = g_accessLatency;
    outQos.delayVariation = 0xffffffff;

    hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
    if (bluetoothHidInterface != nullptr) {
        BtStackStatus ret = bluetoothHidInterface->registerApp(&appParam, &inQos, &outQos);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("Failed to initialize Bluetooth hd register fail, status: %{public}d", ret);
        }
    } else {
        HILOGE("watch_service: bluetoothHidInterface is null");
    }
}

BtHwWatchInterface* WatchService::GetBluetoothWatchInterface() const
{
    return bluetoothWatchInterface;
}

void WatchService::Start()
{
    HILOGI("start");
    bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    CHECK_AND_RETURN_LOG(bluetoothInterface != nullptr, "bluetoothInterface is null!");

    bluetoothWatchInterface =  reinterpret_cast<BtHwWatchInterface*>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_VENDER_WATCH_ID)));
    CHECK_AND_RETURN_LOG(bluetoothWatchInterface != nullptr, "bluetoothWatchInterface is null!");

    bluetoothHidInterface = reinterpret_cast<BthdInterface*>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_PROFILE_HIDDEV_ID)));
    CHECK_AND_RETURN_LOG(bluetoothHidInterface != nullptr, "bluetoothHidInterface is null!");

    BtStackStatus status = bluetoothWatchInterface->Init(&g_sBluetoothWatchCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed to initialize Bluetooth watch, status: %{public}d", status);
        return;
    }
    BtStackStatus hdStatus = bluetoothHidInterface->init(&g_sBluetootHDCallbacks);
    if (hdStatus != BT_STATUS_SUCCESS) {
        HILOGE("Failed to initialize Bluetooth hd, status: %{public}d", hdStatus);
    }
    ChangeStatusExitStrMode();
}

void WatchService::BluetoothIsOn()
{
    classicRemoteDeviceObserver_ = std::make_unique<ClassicRemoteDeviceObserver>(this);
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicAdapter) {
        classicAdapter->RegisterRemoteDeviceObserver(*classicRemoteDeviceObserver_);
    }
    RegisterHid();
    WatchSetUnConnectable();
    GetBondedPhone();
    StartConnectPhone();
    SubscribeWatchCommonEvent();
    SubscribeDataShareEvent();
    StartSaisAddedTimer();
    UpdateReconnectState(true);
}

void WatchService::UpdateReconnectState(bool isBtOn)
{
    std::vector<std::string> addresses;
    addresses = GlobalDeviceManager::getInstance().GetAllAddresses();
    for (const auto &address : addresses) {
        RawAddress device(address);
        GetBluetoothWatchInterface()->updateReconnectState(device, isBtOn);
    }
}

bool WatchService::IsEnableBluetoothAfterExitStrMode()
{
    if (GetIntParameter(g_bluetoothSwitchStateProperty, 0) > 0) {
        return false;
    }

    int lastStrModeState = GetIntParameter(g_bluetoothWatchStrModeEnable, 0);
    HILOGI("watch_service: bluetooth status before entering STR mode %{public}d", lastStrModeState);
    if (lastStrModeState > 0) {
        bool bluetoothState = (GetIntParameter(g_bluetoothHalfOpenProperty, 0) ==
            BLUETOOTH_SWITCH_STATE_OFF) ? false : true;
        DoInAdapterManagerThread([bluetoothState]() {
            AdapterManager::GetInstance()->Enable(ADAPTER_BLE);
            AdapterManager::GetInstance()->SetBluetoothRestrictedFlag(bluetoothState);
        });
        SetParameter(g_bluetoothHalfOpenProperty, g_stateOff);
        return true;
    }
    return false;
}

void WatchService::ChangeStatusOfEnterStrMode()
{
#ifdef BLUETOOTH_HFP_HF_ENABLE
    HfpHfService* service = HfpHfService::GetService();
    std::vector<std::string> addresses = GlobalDeviceManager::getInstance().GetAllAddresses();
    for (const auto &address : addresses) {
        RawAddress device(address);
        if (service != nullptr && service->GetScoState(device) == HFP_HF_AUDIO_STATE_CONNECTED) {
            std::vector<std::pair<std::string, std::string>> vec = {
                std::pair<std::string, std::string>("hfp_enable", "false")};
            HILOGI("watch_service: set hfp_enable false to audio when change to str mode");
            BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters(AUDIO_PARAM_KEY, vec);
        }
        SendPhoneConnectStateChange(RawAddress(address), false);
    }
#endif

    ChangeStatusEnterStrMode();
}

void WatchService::StartConnectPhone()
{
    std::vector<std::string> addresses;
    addresses = GlobalDeviceManager::getInstance().GetAllAddresses();
    for (const auto &address : addresses) {
        auto deviceManager = GetDeviceManager(address);
        RawAddress device(address);
        if (deviceManager == nullptr) {
            HILOGE("watch_service: invalid deviceManager");
            return;
        }
 
        if (!deviceManager->IsSaisDeviceLocked()) {
            ConnectHfp(address);
        } else if ((IsIphone(device) || isSaisAdded_) && !deviceManager->IsConnectedLocked()) {
            ConnectHfp(address);
        }
    }
    ChangeScanMode();
}

void WatchService::SetSaisAddStatus(bool isAdd)
{
    if (isSaisAdded_) {
        return;
    }
    StopSaisAddedTimer();
    isSaisAdded_ = isAdd;
    StartConnectPhone();
}

void WatchService::SubscribeWatchCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT_BLUETOOTH_HOST_CONNECT_HFP);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EVENT_COMMON_LOCAL_DISCONNECT);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPermission(MANAGE_SETTINGS);
    if (subscriber_ == nullptr) {
        subscriber_ = std::make_shared<BluetoothWatchServiceSubscriber>(subscriberInfo);
    }
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    if (subscribeResult) {
        HILOGI("register SubscribeCommonEvent");
    }
}

void WatchService::SubscribeWatchEnterStrEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(ENTER_SYSTEM_STR_MODE);
    matchingSkills.AddEvent(EXIT_SYSTEM_STR_MODE);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPermission(RECEIVER_STARTUP_COMPLETED);
    if (strSubscriber_ == nullptr) {
        strSubscriber_ = std::make_shared<BluetoothWatchServiceSubscriber>(subscriberInfo);
    }
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(strSubscriber_);
    if (subscribeResult) {
        HILOGI("register SubscribeWatchEnterStrEvent");
    }
}

void WatchService::Stop()
{
    HILOGI("stop");
    if (bluetoothHidInterface) {
        bluetoothHidInterface->unregisterApp();
    }
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicAdapter) {
        classicAdapter->DeregisterRemoteDeviceObserver(*classicRemoteDeviceObserver_);
    }

    std::vector<std::string> addresses = GlobalDeviceManager::getInstance().GetAllAddresses();
    for (const auto &address : addresses) {
        StopConnectHfpTimer(address);
        StopControlScoConnectTimer(address);
        StopConnectHfpOnDemandTimer(address);
        StopDisconnectHfpOnDemandTimer();
    }
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
    UnSubscribeDataShareEvent();
}

void WatchService::GetBondedPhone()
{
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");

    std::vector<RawAddress> pairedDevices = classicAdapter->GetPairedDevices();
    if (pairedDevices.size() == 0) {
        HILOGI("watch_service: has no paired device");
        return;
    }
    for (RawAddress device : pairedDevices) {
        if (IsPhoneOrComputer(device)) {
            AddDeviceManager(device.GetAddress());
            bool value = false;
            value = PreferencesManager::Get(device.GetAddress(), value, PreferencesManagerType::WATCH_SAIS_DEVICE);
            HILOGI("watch_service: is sais %{public}d, device = %{public}s",
                value, GetEncryptAddr(device.GetAddress()).c_str());
            UpdateSaisDevice(device, value);
        }
    }
}

void WatchService::UpdateSaisDevice(const RawAddress &device, bool isSaisDevice)
{
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager == nullptr) {
        HILOGE("watch_service: UpdateSaisDevice %{public}d", isSaisDevice);
        return;
    }
    deviceManager->SetIsSaisDeviceLocked(isSaisDevice);
    GetBluetoothWatchInterface()->updateDevice(device, isSaisDevice);
}

void WatchService::UpdateHFStrategy(const RawAddress &device, int strategy)
{
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        GetBluetoothWatchInterface()->updateReconnectState(device, false);
    } else {
        GetBluetoothWatchInterface()->updateReconnectState(device, true);
    }
}

void WatchService::AddNewCallToWatch(const std::string& address, int callId, bool isScoConnected)
{
    SetCallID(address, callId);
    if (!isScoConnected && !IsDialingByWatch(address)) {
        SetAllowHfScoConnect(address, false);
    }
}

void WatchService::SetLinkLossRssi(const RawAddress &device, int rssi)
{
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager && deviceManager->IsSaisDeviceLocked()) {
        rssiLinkLoss_ = rssi;
    }
}

bool WatchService::IsSaisDevice(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager && deviceManager->IsSaisDeviceLocked()) {
        return true;
    }
    return false;
}

void WatchService::SetPhoneConnHfp(const std::string &address, bool isPhoneCon)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        deviceManager->SetPhoneConnHfpLocked(isPhoneCon);
        ConnectHfp(address);
    }
    phoneMac_ = address;
}

void WatchService::SetSaisConnectionState(const RawAddress &device, bool isConnected)
{
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager == nullptr) {
        return;
    }
    deviceManager->SetWatchHealthConnectedLocked(isConnected);
    if (isConnected) {
        deviceManager->SetIsSaisDeviceLocked(true);
        GetBluetoothWatchInterface()->updateDevice(device, true);
        PreferencesManager::Save(device.GetAddress(), true, WATCH_SAIS_DEVICE);
        deviceManager->SetJustConnectAclLocked(true);
        StartDisconnectHfpOnDemandTimer(device.GetAddress());
        ChangeScanMode();
        if (!IsIphone(device)) {
            SendPhoneConnectStateChange(device, true);
        }
        CheckDisconnectHid();
    } else {
        StartConnectHfpOnDemandTimer(device.GetAddress());
    }
}

void WatchService::SetHidConnectionState(const RawAddress &device, int state)
{
    hidConnectionState_ = state;
    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HIDSTATUS", state);
    if (state == BTHD_CONN_STATE_CONNECTED) {
        BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HIDCONNECTEDTIME");
        isNeedConnectHid_ = false;
    } else if (state == BTHD_CONN_STATE_DISCONNECTED) {
        if ((isNeedConnectHid_) && (bluetoothHidInterface != nullptr)) {
            OHOS::bluetooth::RawAddress rawAddr = device;
            bluetoothHidInterface->connect(&rawAddr);
        }
        isNeedConnectHid_ = false;
        BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HIDDISCONNECTEDTIME");
    } else if (state == BTHD_CONN_STATE_CONNECTING) {
        BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, device.GetAddress(), "HIDCONNECTTIME");
    }
}

void WatchService::ProcessRemovePair()
{
    HILOGI("enter");
    auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
    if (deviceManager) {
        std::string addr = deviceManager->GetAddressLocked();
        bool isConnected = deviceManager->IsConnectedLocked();
        DoInVendorThread([this, addr, isConnected] {
            auto dm = GlobalDeviceManager::getInstance().GetDevice(addr);
            if (!dm) {
                return;
            }
            RawAddress device(addr);
            this->ProcessRemovePairInner(device, isConnected);
        });
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, addr,
            "REMOVEPAIRRSCENE", CHR_PHONE_REMOVE_PAIR_ONLINE);
    }
}

void WatchService::ProcessRemovePairInner(const RawAddress &device, bool isAclConnect)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");
    HILOGI("watch_service: remove pair device: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    classicAdapter->RemovePair(BT_TRANSPORT_NONE, device);
    BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(), "CONNECTSTATUS", isAclConnect);
    BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(), "REMOVEPAIRINITIATOR", CHR_INITIATOR_PEER);
}

void WatchService::WatchSetUnConnectable()
{
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");
    int btScanMode = classicAdapter->GetBtScanMode();
#ifndef BLUETOOTH_INTERCEPT_PERIPHERAL_FIRST_PAIR_DIALOG
    if (btScanMode == SCAN_MODE_CONNECTABLE) {
        HILOGI("watch_service: watch service start and reset scan mode");
        classicAdapter->SetBtScanMode(SCAN_MODE_NONE, 0);
    }
#endif
}

void WatchService::ProcessAclStateChangeEvt(const RawAddress &device, bool isConnected)
{
    DoInVendorThread([this, device, isConnected] {this->ProcessAclStateChangeInner(device, isConnected);});
}

void WatchService::ProcessAclStateChangeInner(const RawAddress &device, bool isConnected)
{
    HILOGI("watch_service: acl connected: %{public}d", isConnected);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");
    if (isConnected) {
        BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, device.GetAddress(), "BTCONNECTSCENE", 0);
    }
    ChangeScanMode();
    if (!IsPhoneOrComputer(device)) {
        HILOGE("watch_service: is not phone connected: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        return;
    }
    if (classicAdapter->GetPairState(device) == PAIR_PAIRED) {
        AddDeviceManager(device.GetAddress());
    }
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager == nullptr) {
        return;
    }
    if (isConnected) {
        StopSaisAddedTimer();
        int delay = deviceManager->SetAclConnectedAndGetDelay();
        StartConnectHfpTimer(device.GetAddress(), delay);
    } else {
        StopConnectHfpOnDemandTimer(device.GetAddress());
        StopHidReconnectTimer();
        StopIphoneScoDisconnectTimer(device.GetAddress());
        SendPhoneConnectStateChange(device, isConnected);
        if (deviceManager->IsSaisDeviceLocked() && (hidConnectionState_ != BTHD_CONN_STATE_DISCONNECTED) &&
            (bluetoothHidInterface != nullptr)) {
            bluetoothHidInterface->disconnect();
        }
        GlobalDeviceManager::getInstance().ResetDeviceStatus(device.GetAddress());
    }
}

void WatchService::ProcessPairStateChangeEvt(const BTTransport transport, const RawAddress &device, int status)
{
    DoInVendorThread([this, transport, device, status] {this->ProcessPairChangeInner(transport, device, status);});
}

void WatchService::ProcessPairChangeInner(const BTTransport transport, const RawAddress &device, int status)
{
    HILOGI("watch_service: pair state: %{public}d", status);
    isRemovePair_ = ((status == PAIR_NONE) ? true : false);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");
    if (status == PAIR_NONE && (static_cast<uint32_t>(classicAdapter->GetDeviceClass(device)) &
        static_cast<uint32_t>(BITMASK)) == AUDIO_VIDEO) {
        ChrProcessRemovePair(device.GetAddress(), classicAdapter->IsAclConnected(device));
    }
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager != nullptr && status == PAIR_NONE) {
        HILOGI("watch_service: pair none device: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        ChrProcessRemovePair(device.GetAddress(), deviceManager->IsConnectedLocked());
        StopConnectHfpOnDemandTimer(device.GetAddress());
        if (deviceManager->IsSaisDeviceLocked() && (hidConnectionState_ != BTHD_CONN_STATE_DISCONNECTED) &&
            (bluetoothHidInterface != nullptr)) {
            bluetoothHidInterface->disconnect();
            hidConnectionState_ = BTHD_CONN_STATE_DISCONNECTED;
        }
        RemoveDeviceManager(device.GetAddress());
    } else if (status == PAIR_PAIRED && IsPhoneOrComputer(device)) {
        HILOGI("watch_service: paired device: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
        SetParameter(HID_MAP_UPDATE, g_stateOn);
        AddDeviceManager(device.GetAddress());
        deviceManager = GetDeviceManager(device.GetAddress());
        CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is null after AddDeviceManager.");
        deviceManager->SetPhoneCreateConnectLocked(true);
        deviceManager->SetPhoneConnHfpLocked(true);
        if (!deviceManager->IsWatchHealthConnectedLocked()) {
            StartConnectHfpTimer(device.GetAddress(), DELAY_TIME_CONNECT_HF_PHONE_CC);
        }
        if (!isSecondaryPair_) {
            UpdateSaisDevice(device, true);
            HILOGI("watch_service: save saisDevice: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
            PreferencesManager::Save(device.GetAddress(), true, WATCH_SAIS_DEVICE);
        } else {
            UpdateSaisDevice(device, false);
        }
        isSecondaryPair_ = false;
        GetBluetoothWatchInterface()->sendIsSecondPhonePair(isSecondaryPair_);
    }
    ChangeScanMode();
}

void WatchService::ChrProcessRemovePair(const std::string& address, const bool isAclConnect)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");
    if (isAclConnect) {
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, address, "DISCONNECTRESULT",
            classicAdapter->IsAclConnected(RawAddress(address)) ? CHR_DISCONNECT_FAIL : CHR_DISCONNECT_SUCCESS);
    } else {
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, address, "DISCONNECTRESULT", CHR_NOT_INVOLVED);
    }
    BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, address, "CONNECTSTATUS", isAclConnect);
    BtChrEventWriteTime(CHR_BT_WATCH_REMOVE_PAIR, address, "REMOVEPAIRTIME");
    BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, address, "REMOVEPAIRRESULT", CHR_REMOVE_PAIR_SUCCESS);
}

void WatchService::ProcessHfpStateChangeEvt(const RawAddress &device, int state)
{
    HILOGI("watch_service: hfp connection state: %{public}d", state);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");

    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager == nullptr) {
        return;
    }
    deviceManager->SetHfConnectionStateLocked(state);
    if (state == HFP_HF_STATE_CONNECTED) {
        deviceManager->SetIsConnectedLocked(true);
        if (bluetoothHidInterface != nullptr && hidConnectionState_ == BTHD_CONN_STATE_DISCONNECTED &&
            deviceManager->IsSaisDeviceLocked()) {
            OHOS::bluetooth::RawAddress rawAddr = device;
            bluetoothHidInterface->connect(&rawAddr);
        }
        SendPhoneConnectStateChange(device, true);
        deviceManager->SetJustConnectAclLocked(false);
        StopConnectHfpTimer(device.GetAddress());
        GetBluetoothWatchInterface()->sendHfpState2Hisi(device, false);
    } else if (state == HFP_HF_STATE_DISCONNECTED) {
        deviceManager->SetOnHfCallFalseLocked();
        AllHfCallFinished(device.GetAddress(), true);
        DialCallFinished();
        SetAllowHfScoConnect(device.GetAddress(), false);
        UpdataHighPowerMode(false);
        if (classicAdapter->IsAclConnected(device) && IsIphone(device)) {
            classicAdapter->DisconnectAcl(device.GetAddress());
        }
        GetBluetoothWatchInterface()->sendHfpState2Hisi(device, false);
    }
    deviceManager->SetHfConnectedLocked(state == HFP_HF_STATE_CONNECTED);
    PublishEventWithIntParam(EVENT_BLUETOOTH_HOST_HFP_CONNECTION_STATE, device.GetAddress(),
        PARAM_KEY_CONNECTION_STATE, state, false);
}

void WatchService::CheckDisconnectHid()
{
    HILOGI("watch_service: hid state: %{public}d", hidConnectionState_);
    if ((hidConnectionState_ != BTHD_CONN_STATE_CONNECTED)) {
        if (bluetoothHidInterface != nullptr) {
            bluetoothHidInterface->disconnect();
        }
        isNeedConnectHid_ = true;
    }
}

void WatchService::StartSaisAddedTimer()
{
    StopSaisAddedTimer();
    saisAddedTimer_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::WatchService::SaisAddedTimeout, this));
    saisAddedTimer_->Start(DELAY_TIME_SAIS_ADDED_TIMEOUT);
    HILOGI("watch_service: StartSaisAddedTimer");
}

void WatchService::StopSaisAddedTimer()
{
    if (saisAddedTimer_ != nullptr) {
        HILOGI("watch_service: StopSaisAddedTimer");
        saisAddedTimer_->Stop();
        saisAddedTimer_ = nullptr;
    }
}

void WatchService::SaisAddedTimeout()
{
    HILOGI("watch_service: SaisAddedTimeout: %{public}d", isSaisAdded_);
    StartConnectPhone();
}

void WatchService::StartHidReconnectTimer(const RawAddress &device)
{
    StopHidReconnectTimer();
    hidReconnectTimer_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::WatchService::HidReconnectTimeout, this, device));
    hidReconnectTimer_->Start(DELAY_TIME_SAIS_ADDED_TIMEOUT);
    HILOGI("watch_service: StartHidReconnectTimer");
}

void WatchService::StopHidReconnectTimer()
{
    if (hidReconnectTimer_ != nullptr) {
        HILOGI("watch_service: StopHidReconnectTimer");
        hidReconnectTimer_->Stop();
        hidReconnectTimer_ = nullptr;
    }
}

void WatchService::HidReconnectTimeout(const RawAddress &device)
{
    auto deviceManager = GetDeviceManager(device.GetAddress());
    if (deviceManager == nullptr) {
        HILOGE("watch_service: sais device is null");
        return;
    }
    HILOGI("watch_service: HidReconnectTimeout isHidMapUpdateing_: %{public}d, isHidMapUpdateing_: %{public}d",
        isHidMapUpdateing_, hidConnectionState_);
    isHidMapUpdateing_ = false;
    OHOS::bluetooth::RawAddress rawAddr = device;
    if (deviceManager->IsConnectedLocked() && (bluetoothHidInterface != nullptr)
        && (hidConnectionState_ == BTHD_CONN_STATE_DISCONNECTED)) {
        bluetoothHidInterface->connect(&rawAddr);
    }
}

bool WatchService::IsWatchStrMode()
{
    int mode = GetIntParameter(g_watchStrMode, 0);
    HILOGI("watch_service: str mode is %{public}d", mode);
    bool ret = (mode == 1) ? true : false;
    return ret;
}

void WatchService::ChangeStatusEnterStrMode()
{
    HILOGI("watch_service: change enter str param!");
    int lastBtState = GetIntParameter(g_bluetoothSwitchStateProperty, 0);
    if (lastBtState == BLUETOOTH_SWITCH_STATE_HALF) {
        SetParameter(g_bluetoothHalfOpenProperty, g_stateOn);
    } else {
        SetParameter(g_bluetoothHalfOpenProperty, g_stateOff);
    }
    SetParameter(g_bluetoothSwitchStateProperty, g_stateOff);
    SetParameter(g_bluetoothWatchStrModeEnable, g_stateOn);
    PromptUnloadBluetoothSystemAbility();
}

void WatchService::ChangeStatusExitStrMode()
{
    SubscribeWatchEnterStrEvent();
    if (GetIntParameter(g_bluetoothHalfOpenProperty, 0) > 0) {
        SetParameter(g_bluetoothSwitchStateProperty, g_stateHalf);
        SetParameter(g_bluetoothHalfOpenProperty, g_stateOff);
    }
}

void WatchService::PromptUnloadBluetoothSystemAbility()
{
    HILOGI("watch_service: %{public}s", __func__);
    RemoveParameterWatcher(COLLABORATION_SERVICE, nullptr, nullptr);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("watch_service: get samgr failed");
        return;
    }
    HoldRunningLock(WAKE_TIME);
    int32_t ret = samgrProxy->UnloadSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (ret != ERR_OK) {
        HILOGE("watch_service: unload system ability failed");
        return;
    }
}

int WatchService::StrModeAbnormalExitHandle(const BTTransport transport)
{
    if (IsWatchStrMode() && (transport == ADAPTER_BLE)) {
        ChangeStatusEnterStrMode();
        return BT_STATUS_FAIL;
    }

    if (transport == ADAPTER_BLE) {
        SetParameter(g_bluetoothWatchStrModeEnable, g_stateOff);
    }
    int lastStrModeState = GetIntParameter(g_bluetoothWatchStrModeEnable, 0);
    HILOGI("watch_service: the watch is in AP mode!, g_bluetoothWatchStrModeEnable: %d", lastStrModeState);
    return BT_STATUS_SUCCESS;
}

void WatchService::HoldRunningLock(int32_t timeOutMs)
{
    HILOGI("HoldRunningLock timeOutMs: %{public}d",  timeOutMs);
    if (runningLock_ == nullptr) {
        auto &powerMgrClient = PowerMgr::PowerMgrClient::GetInstance();
        std::string lockName = "EnterStrTurnOffBluetooth" ;
        runningLock_ =
            powerMgrClient.CreateRunningLock(lockName, PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    }
    if (runningLock_ == nullptr) {
        HILOGE("Create runningLock failed.");
        return;
    }
    HILOGI("HoldRunningLock Lock");
    int32_t res = runningLock_->Lock(timeOutMs);
    if (res != 0) {
        HILOGE("HoldRunningLock lock failed");
        return;
    }
    HILOGI("HoldRunningLock success");
}

void WatchService::ReleaseRunningLock()
{
    HILOGI("RunningLock UnLock");
    if (runningLock_ == nullptr) {
        HILOGE("ReleaseRunningLock runningLock_ is null");
        return;
    }
    int32_t res = runningLock_->UnLock();
    if (res != 0) {
        HILOGE("runningLock unlock failed");
        return;
    }
    runningLock_ = nullptr;
}

bool WatchService::IsAllowSystemPairDialog(const RawAddress &device)
{
    if (IsPhoneOrComputer(device)) {
        return false;
    }

    if (phoneMac_ == device.GetAddress()) {
        return false;
    }
    return true;
}

void WatchService::DeviceDisconnectCallBack(const RawAddress &device, int reason)
{
    if (!GlobalDeviceManager::getInstance().DeviceExists(device.GetAddress())) {
        HILOGE("watch_service: device is not bonded phone");
        return;
    }
    auto deviceManager = GetDeviceManager(device.GetAddress());
    HILOGI("watch_service: reason: %{public}d", reason);
    if (deviceManager && (reason == IS_PHONE_CREATE_CONNECT)) {
        deviceManager->SetPhoneCreateConnectLocked(true);
        deviceManager->SetPhoneConnHfpLocked(true);
        phoneMac_ = device.GetAddress();
        return;
    }
    StartBtDisconnectTimer(device.GetAddress(), reason);
    if (reason == PIN_OR_KEY_MISSING) {
        DoInVendorThread([this, device] { this->ProcessRemovePairInner(device, false); });
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(),
            "REMOVEPAIRRSCENE", CHR_PHONE_REMOVE_PAIR_OFFLINE);
    }
}

void WatchService::StartBtDisconnectTimer(const std::string& address, int reason)
{
    auto device = GetDeviceManager(address);
    if (device && device->IsBtDisconnectTimerNullLocked()) {
        StopBtDisconnectTimer(address);
        auto timer = std::make_unique<utility::Timer>(
            std::bind(&bluetooth::WatchService::ChrProcessDisconnect, this, address, reason));
        timer->Start(DELAY_TIME_CHR_BT_DISCONNECT_TIMEOUT);
        device->StartBtDisconnectTimerLocked(std::move(timer));

        HILOGI("Started disconnect timer for device: %{public}s, reason: %{public}d", address.c_str(), reason);
    }
}

void WatchService::StopBtDisconnectTimer(const std::string& address)
{
    auto device = GetDeviceManager(address);
    if (device && !device->IsBtDisconnectTimerNullLocked()) {
        device->StopBtDisconnectTimerLocked();
    }
}

void WatchService::ChrProcessDisconnect(const std::string& address, const int reason)
{
    auto device = GetDeviceManager(address);
    if (device) {
        BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, address, "BTCONNECTSCENE", (device->IsJustConnectAclLocked() ? 1 : 0));
        if (device->IsRemovePairLocked()) {
            BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, address, "DISCONNECTREASON", REMOVEPAIR_BT_DISCONNECT_ACL);
        } else {
            BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, address, "DISCONNECTREASON", reason);
        }
        device->SetJustConnectAclLocked(true);
        device->SetRemovePairLocked(false);
        StopBtDisconnectTimer(address);
    }
}

bool WatchService::IsIphone(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG_RET(classicAdapter != nullptr, false, "BT current state is not enabled!");

    std::vector<std::string> uuids = classicAdapter->GetDeviceUuids(device);
    if (uuids.size() == 0) {
        return false;
    }

    for (std::string uuid : uuids) {
        if (APPLE_IAP_UUID == uuid) {
            return true;
        }
    }
    return false;
}

bool WatchService::PublishEventWithIntParam(const std::string &eventAction, const std::string &device,
    const std::string &key, const int value, bool isSticky)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(BluetoothHelper::PARAM_KEY_DEVICE_ADDR, device);
    want.SetParam(key, value);
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSticky(isSticky);
    publishInfo.SetSubscriberPermissions({MANAGE_BLUETOOTH});
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    HILOGI("eventAction: %{public}s, publishResult = %{public}d", want.GetAction().c_str(), publishResult);
    return publishResult;
}

bool WatchService::IsPhoneOrComputer(const RawAddress &device)
{
    bool isPhone = GetBluetoothWatchInterface()->isPhoneMajorClass(device);
    HILOGI("watch_service: isPhone: %{public}d", isPhone);

    return isPhone;
}

void WatchService::SendConnectionTimtout()
{
    auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
    if (deviceManager == nullptr) {
        HILOGE("watch_service: deviceManager is null");
        return;
    }
    PublishEventWithIntParam(EVENT_BLUETOOTH_REMOTEDEVICE_DISCONNECT_REASON, deviceManager->GetAddressLocked(),
        PARAM_KEY_DISCONNECTION_REASON, ACL_CONNECTION_TIMEOUT, false);
}

void WatchService::ChangeScanMode()
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "BT current state is not enabled!");

    if (isBluetoothDisable_) {
        classicAdapter->SetBtScanMode(SCAN_MODE_NONE, 0);
        HILOGE("watch_service: bluetooth is being disable");
        return;
    }
    int btScanMode = classicAdapter->GetBtScanMode();
    size_t deviceSize = GlobalDeviceManager::getInstance().GetDeviceCount();
    std::vector<RawAddress> pairedDevices = classicAdapter->GetPairedDevices();
    if (pairedDevices.size() == 0 || ((deviceSize == 0) && isSaisAdded_) ||
        btScanMode == SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE) {
        HILOGE("watch_service: ChangeScanMode device is not paired");
        return;
    }

    size_t connectedDeviceNum = 0;
    size_t bleDeviceNum = 0;
    for (RawAddress device : pairedDevices) {
        if (classicAdapter->GetDeviceType(device) == DEVICE_TYPE_LE &&
            (static_cast<uint32_t>(classicAdapter->GetDeviceClass(device)) &
                static_cast<uint32_t>(BITMASK)) != AUDIO_VIDEO) {
            bleDeviceNum++;
            continue;
        }
        if (classicAdapter->IsAclConnected(device)) {
            connectedDeviceNum++;
        }
    }
    size_t brDeviceNum = pairedDevices.size() - bleDeviceNum;
    HILOGI("watch_service: brDeviceNum: %{public}d, connectedDeviceNum: %{public}d, pairedDeviceNum: %{public}d",
        brDeviceNum, connectedDeviceNum, pairedDevices.size());
    if (brDeviceNum == connectedDeviceNum && btScanMode != SCAN_MODE_NONE) {
        classicAdapter->SetBtScanMode(SCAN_MODE_NONE, 0);
    } else if (brDeviceNum != connectedDeviceNum && btScanMode != SCAN_MODE_CONNECTABLE) {
        classicAdapter->SetBtScanMode(SCAN_MODE_CONNECTABLE, 0);
    }
}

void WatchService::ConnectBondedPhone()
{
    std::vector<std::string> addresses = GlobalDeviceManager::getInstance().GetAllAddresses();
    bool isNeedConnect = false;
    for (const auto &address : addresses) {
        auto deviceManager = GetDeviceManager(address);
        if (deviceManager == nullptr) {
            continue;
        }
        if (!deviceManager->IsConnectedLocked()) {
            isNeedConnect = true;
        }
    }
    if (isNeedConnect) {
        GetBluetoothWatchInterface()->sendScreenOn2Mcu();
    }
}

bool WatchService::ConnectHfp(const std::string& address)
{
#ifdef BLUETOOTH_HFP_HF_ENABLE
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager == nullptr) {
        HILOGI("GlobalDeviceManager: device is not paired");
        return false;
    }
    StopConnectHfpTimer(address);
    RawAddress device(address);
    int strategy = ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGI("watch_service: device is not not allow to connect address %{public}s",
            GetEncryptAddr(device.GetAddress()).c_str());
        GetBluetoothWatchInterface()->updateReconnectState(device, false);
        return false;
    }
    if (deviceManager->GetHfConnectionStateLocked() != HFP_HF_STATE_DISCONNECTED) {
        HILOGI("watch_service: device is not in state of disconnected");
        return false;
    }

    HILOGI("watch_service: connecting HFP with device address %{public}s",
        GetEncryptAddr(device.GetAddress()).c_str());
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "service is null");
    GetBluetoothWatchInterface()->sendHfpState2Hisi(device, true);
    service->Connect(device);
    return true;
#else
    return false;
#endif
}

void WatchService::StartDisconnectHfpOnDemandTimer(const std::string& address)
{
    StopDisconnectHfpOnDemandTimer();
    disconnectHfpTimer_ =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::WatchService::DisconnectHfpOnDemand, this, address));
    disconnectHfpTimer_->Start(DELAY_TIME_DISCONNECT_HF);
}

void WatchService::StopDisconnectHfpOnDemandTimer()
{
    if (disconnectHfpTimer_ != nullptr) {
        disconnectHfpTimer_->Stop();
        disconnectHfpTimer_ = nullptr;
    }
}

void WatchService::DisconnectHfpOnDemand(const std::string& address)
{
#ifdef BLUETOOTH_HFP_HF_ENABLE
    RawAddress device(address);
    if (IsIphone(device)) {
        HILOGI("watch_service: iphone do not support On-Demand hfp");
        return;
    }
    HILOGI("watch_service: DisconnectHfpOnDemand address %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    auto deviceManager = GetDeviceManager(address);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is null.");
    HfpHfService *service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    std::vector<HandsFreeUnitCalls> callList = service->GetCurrentCallList(device);
    deviceManager->SetDisconnectHfpOnDemandHfCallLocked(callList.size() != 0);
    HILOGI("watch_service: DisconnectHfpOnDemand isHfConnected_ %{public}d, isWatchHealthConnected_ %{public}d, "
           "isOnHfcall %{public}d", deviceManager->IsHfConnectedLocked(), deviceManager->IsWatchHealthConnectedLocked(),
            deviceManager->IsHfOnCallLocked());
    if (deviceManager->ShouldDisconnectHfpOnDemand()) {
        service->Disconnect(device);
    }
#endif
}

void WatchService::StartConnectHfpOnDemandTimer(const std::string& address)
{
    auto device = GetDeviceManager(address);
    if (!device || !device->IsConnectHfpOnDemandTimerNullLocked()) {
        return;
    }
    auto timer =
        std::make_unique<utility::Timer>(std::bind(&bluetooth::WatchService::ConnectHfpOnDemand, this, address));
    timer->Start(DELAY_TIME_HEALTH_DISCONNECT_CONNECT_HF);
    device->StartConnectHfpOnDemandTimerLocked(std::move(timer));
}

void WatchService::StopConnectHfpOnDemandTimer(const std::string& address)
{
    auto device = GetDeviceManager(address);
    if (!device) {
        return;
    }
    if (!device->IsConnectHfpOnDemandTimerNullLocked()) {
        device->StopConnectHfpOnDemandTimerLocked();
    }
}

void WatchService::ConnectHfpOnDemand(const std::string& address)
{
#ifdef BLUETOOTH_HFP_HF_ENABLE
    StopConnectHfpOnDemandTimer(address);
    RawAddress device(address);
    int strategy =
        ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device, PROPERTY_HFP_CLIENT_CONNECTION_POLICY);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGI("watch_service: do not support connect hfp");
        return;
    }
    auto deviceManager = GetDeviceManager(address);
    if (!deviceManager) {
        return;
    }
    if (deviceManager->ShouldConnectHfpOnDemand()) {
        HfpHfService *service = HfpHfService::GetService();
        CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
        service->Connect(device);
    }
#endif
}

void WatchService::StopConnectHfpTimer(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager && !deviceManager->IsConnectHfpTimerNullLocked()) {
        deviceManager->StopConnectHfpTimerLocked();
    }
}

void WatchService::StartConnectHfpTimer(const std::string& address, int delayTime)
{
    auto deviceManager = GetDeviceManager(address);
    HILOGI("watch_service: delayTime: %{public}d", delayTime);
    if (deviceManager == nullptr || !deviceManager->IsConnectHfpTimerNullLocked()) {
        return;
    }
    auto timer = std::make_unique<utility::Timer>(std::bind(&bluetooth::WatchService::ConnectHfp, this, address));
    timer->Start(delayTime);
    deviceManager->StartConnectHfpTimerLocked(std::move(timer));
}

void WatchService::StartControlScoConnectTimer(const std::string& address)
{
    auto device = GetDeviceManager(address);
    if (device && device->IsControlScoConnectTimerNullLocked()) {
        auto timer = std::make_unique<utility::Timer>(
            std::bind(&bluetooth::WatchService::ControlScoConnectTimeout, this, address));
        timer->Start(DELAY_TIME_ROUTE_SCO);
        device->StartControlScoConnectTimerLocked(std::move(timer));
    }
}

void WatchService::StopControlScoConnectTimer(const std::string& address)
{
    auto device = GetDeviceManager(address);
    if (device && !device->IsControlScoConnectTimerNullLocked()) {
        device->StopControlScoConnectTimerLocked();
    }
}

void WatchService::ControlScoConnectTimeout(const std::string& address)
{
    StopControlScoConnectTimer(address);
    auto device = GetDeviceManager(address);
    if (device && device->IsRouteScoRunningLocked()) {
        device->ControlScoTimeoutLocked();
    }
}

void WatchService::SetCallID(const std::string& address, int callId)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        deviceManager->SetCallIDLocked(callId);
    }
}

void WatchService::SetAllowHfScoConnect(const std::string& address, bool isAllow)
{
    HILOGI("watch_service: isAllow=[%{public}d]", isAllow);
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        deviceManager->SetAllowHfScoConnectLocked(isAllow);
    }
}

bool WatchService::IsAllowHfScoConnect(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager == nullptr) {
        HILOGE("watch_service: device is null %{public}s", GetEncryptAddr(address).c_str());
        return false;
    }
    bool isAllow = deviceManager->CheckAndUpdateScoAllow(GetTimeStamp(), DELAY_TIME_ROUTE_SCO);
    if (isAllow) {
        UpdataHighPowerMode(true);
    }
    return isAllow;
}

bool WatchService::IsAllowAgCall(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        return deviceManager->IsAllowAgCallLocked();
    }
    return true;
}

void WatchService::SetDialedCallNumber(const std::string &dailNumber)
{
    dailNumber_ = dailNumber;
    isNeedDialAfterHFConnected_ = true;
}

std::string WatchService::GetDialedCallNumber()
{
    return dailNumber_;
}

void WatchService::DialCallFinished()
{
    dailNumber_.clear();
    isNeedDialAfterHFConnected_ = false;
}

bool WatchService::IsNeedDialCall()
{
    return isNeedDialAfterHFConnected_;
}

void WatchService::SetDialingByWatch(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        deviceManager->SetDialingByWatchLocked();
    }
}

bool WatchService::IsDialingByWatch(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        return deviceManager->IsDialingByWatchLocked();
    }
    return false;
}

void WatchService::SetIsHfOnCall(const std::string& address, bool isOnCall)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        deviceManager->SetIsHfOnCallLocked(isOnCall);
    }
}

bool WatchService::IsHfOnCall(const std::string& address)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        return deviceManager->IsHfOnCallLocked();
    }
    return false;
}

void WatchService::AllHfCallFinished(const std::string& address, bool isHfDisconnected)
{
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager) {
        StopControlScoConnectTimer(address);
        deviceManager->AllHfCallFinishedLocked(isHfDisconnected);
        RawAddress device(deviceManager->GetAddressLocked());
        if (!deviceManager->IsSaisDeviceLocked()) {
            return;
        }
        if (!IsIphone(device) && !isHfDisconnected && !deviceManager->isOnVoip_Locked()) {
            StartDisconnectHfpOnDemandTimer(address);
        }
    }
}

void WatchService::HandleVoipState(int voipState)
{
    HILOGI("watchVoipState is:%{public}d", voipState);
    auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
    if (deviceManager == nullptr || (!deviceManager->IsConnectedLocked() && voipState < 0)) {
        return;
    }
    RawAddress device(deviceManager->GetAddressLocked());
    if (voipState == VOIP_CALL_INCOMMING) {
        deviceManager->HandleVoipStateLocked(VOIP_CALL_INCOMMING);
        GetBluetoothWatchInterface()->sendTbsr(device, HEADPHONE_MODE);
    } else if (voipState == VOIP_CALL_ACCEPT) {
        deviceManager->HandleVoipStateLocked(VOIP_CALL_ACCEPT);
    } else {
        deviceManager->HandleVoipStateLocked(-1);
        GetBluetoothWatchInterface()->sendTbsr(device, BAND_MODE);
        DisconnectHfpOnDemand(deviceManager->GetAddressLocked());
    }
}

void WatchService::StopIphoneScoDisconnectTimer(const std::string& address)
{
    RawAddress device(address);
    auto deviceManager = GetDeviceManager(address);
    if ((!deviceManager) || (!IsIphone(device))) {
        return;
    }

    std::lock_guard<std::mutex> lock(disconnectScoTimerMutex_);
    if (disconnectScoTimer_ != nullptr) {
        HILOGI("watch_service: %s", __func__);
        disconnectScoTimer_->Stop();
        disconnectScoTimer_ = nullptr;
    }
}

void WatchService::StartIphoneScoDisconnectTimer(const std::string& address)
{
#ifdef BLUETOOTH_HFP_HF_ENABLE
    HfpHfService *service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null.");
    RawAddress device(address);
    if (IsIphone(device) && service->GetScoState(device) == HFP_HF_AUDIO_STATE_CONNECTED) {
        StopIphoneScoDisconnectTimer(address);
        std::lock_guard<std::mutex> lock(disconnectScoTimerMutex_);
        disconnectScoTimer_ = std::make_unique<utility::Timer>([service, device]() {
            service->DisconnectSco(device);
        });
        disconnectScoTimer_->Start(DELAY_TIME_DISCONNECT_SCO_TIMEOUT);
        HILOGI("watch_service: No call, disconnect the sco link");
    }
#endif
}

void WatchService::ProcessCallStateChangeEvt(const std::string& address, const HandsFreeUnitCalls &call)
{
    HILOGI("watch_service: call state=[%{public}d]", call.GetState());
    auto deviceManager = GetDeviceManager(address);
    if (deviceManager == nullptr) {
        HILOGE("watch_service: device is null%{public}s", GetEncryptAddr(address).c_str());
        return;
    }
    if (!deviceManager->ProcessCallStateCheckLocked(call.GetId())) {
        return;
    }

    if (call.GetState() == HFP_CALL_STATE_ACTIVE) {
        deviceManager->ProcessCallStateActiveLocked(GetTimeStamp());
        StartControlScoConnectTimer(address);
    } else if (call.GetState() == HFP_CALL_STATE_FINISHED && deviceManager->IsHfOnCallLocked()) {
        StartIphoneScoDisconnectTimer(address);
        deviceManager->ProcessCallStateFinishedLocked();
    }
}

void WatchService::SendPhoneConnectStateChange(const RawAddress &device, bool connected)
{
    auto deviceManager = GetDeviceManager(device.GetAddress());
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is null.");
    if (deviceManager->IsPhoneConnectedLocked() == connected) {
        return;
    }
    deviceManager->SetPhoneConnectStateLocked(connected);
    PublishEventWithIntParam(EVENT_PHONE_CONNECT_STATE_CHANGE, device.GetAddress(),
        PARAM_KEY_PHONE_CONNECTION_STATE, connected, true);
}

bool WatchService::BluetoothDataShareUpdate(const char *uriStr, const char *keyStr, std::string &inValue)
{
    CHECK_AND_RETURN_LOG_RET(uriStr && keyStr, false, "uri or key is nullptr");
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();

    Uri uri(uriStr);
    std::string key = keyStr;
    if (dataShareHelperUtils->Update(uri, key, inValue) != RET_NO_ERROR) {
        HILOGE("Query uri(%{public}s) key(%{public}s) fail", uriStr, keyStr);
        return false;
    }
    return true;
}

void WatchService::SubscribeDataShareEvent()
{
    HILOGI("enter");
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbilityManager failed.");
        return;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("BluetoothDataShareHelperUtils GetSystemAbility Service Failed.");
        return;
    }
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> helperPair =
        DataShare::DataShareHelper::Create(remoteObj, STATUS_SETTING_MODE_URI, SETTINGS_DATA_BASE_URI);
    if (helperPair.first != DataShare::E_OK) {
        HILOGE("DataShareHelper create failed, ret: %{public}d", helperPair.first);
        return;
    }
    chrDataShareHelper_ = helperPair.second;
    if (chrDataShareHelper_) {
        chrDataShareObserver_ = sptr<ConnectChrObserver>(new (std::nothrow)ConnectChrObserver(*this));
        if (chrDataShareObserver_ == nullptr) {
            HILOGE("chrDataShareObserver_ is nullptr.");
            return;
        }
        Uri uri(STATUS_SETTING_CHR_URI);
        chrDataShareHelper_->RegisterObserver(uri, chrDataShareObserver_);

        healthStatusObserver_ = sptr<HealthStatusObserver>(new (std::nothrow)HealthStatusObserver(*this));
        if (healthStatusObserver_ == nullptr) {
            HILOGE("healthStatusObserver_ is nullptr.");
            return;
        }
        Uri uriHealthStatus(STATUS_SETTING_HEALTH_CONNECTION_URI);
        chrDataShareHelper_->RegisterObserver(uriHealthStatus, healthStatusObserver_);

        secondaryPhoneObserver_ = sptr<SecondaryPhoneObserver>(new (std::nothrow)SecondaryPhoneObserver(*this));
        if (secondaryPhoneObserver_ == nullptr) {
            HILOGE("healthStatusObserver_ is nullptr.");
            return;
        }
        Uri urisecondaryPhone(STATUS_SETTING_SECONDARY_PHONE_URI);
        chrDataShareHelper_->RegisterObserver(urisecondaryPhone, secondaryPhoneObserver_);
    }
}

void WatchService::UnSubscribeDataShareEvent()
{
    HILOGI("enter");
    if (chrDataShareHelper_) {
        Uri uri(STATUS_SETTING_CHR_URI);
        chrDataShareHelper_->UnregisterObserver(uri, chrDataShareObserver_);
        chrDataShareObserver_ = nullptr;
        chrDataShareHelper_->UnregisterObserver(uri, healthStatusObserver_);
        healthStatusObserver_ = nullptr;
    }
}

void WatchService::UnSubscribeWatchEnterStrEvent()
{
    if (strSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(strSubscriber_);
        strSubscriber_ = nullptr;
    }
}

void WatchService::HandleConnectChrDataChanged()
{
    HILOGI("enter");
    Uri uri(STATUS_SETTING_CHR_URI);
    std::string key = STATUS_SETTING_CHR_KEY_URI;
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils->Query(uri, key, value) != RET_NO_ERROR) {
        HILOGE("Query key(%{public}s) fail", key.c_str());
        return;
    }

    if (value.empty()) {
        HILOGE("Query key(%{public}s) value is empty", key.c_str());
        return;
    }

    cJSON* chrJson = JsonUtil::ReadJsonContents(value);
    if (chrJson == nullptr || !cJSON_IsObject(chrJson)) {
        HILOGE("json value is not object.");
        return;
    }

    std::string connectResult = "";
    std::vector<BtChrEventParam> params;
    cJSON* it = nullptr;
    cJSON_ArrayForEach(it, chrJson) {
        std::string chrKey = std::string(it->string);
        std::string chrValue;
        if (cJSON_IsString(it)) {
            chrValue = std::string(cJSON_GetStringValue(it));
        } else if (cJSON_IsNumber(it)) {
            chrValue = std::to_string(cJSON_GetNumberValue(it));
        } else {
            continue;
        }
        params.push_back(BtChrEventParam(chrKey, chrValue));
        HILOGI("Param name: (%{public}s)", chrKey.c_str());
        if (chrKey == "BUSINESSPAIRRESULT") {
            connectResult = chrValue;
        }
    }
    cJSON_Delete(chrJson);
    auto deviceManager = GlobalDeviceManager::getInstance().GetSaisDevice();
    if (deviceManager) {
        BtChrEventWriteParams(CHR_BT_WATCH_CONNECT, deviceManager->GetAddressLocked(), params);
        if (connectResult == "0") { // business pair success
            BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, deviceManager->GetAddressLocked(), "DISCONNECTREASON", 0);
        }
    }
}

void WatchService::HandleHealthStatusChanged()
{
    std::string value = "";
    BluetoothDataShareQuery(STATUS_SETTING_HEALTH_CONNECTION_URI, STATUS_SETTING_HEALTH_CONNECTION_KEY_URI, value);
    HILOGI("Health Connection Status (%{public}s)", value.c_str());
    if (value == HEALTH_CONNECTED) {
        ChangeScanMode();
    }
}

void WatchService::SecondaryPhoneChange()
{
    std::string value = "";
    BluetoothDataShareQuery(STATUS_SETTING_SECONDARY_PHONE_URI, STATUS_SETTING_SECONDARY_PHONE_URI_KEY_URI, value);
    HILOGI("Health Connection Status (%{public}s)", value.c_str());
    if (value == SECONDARY_PHONE_PAIR) {
        isSecondaryPair_ = true;
    } else if (value == SECONDARY_PHONE_UNKNOW) {
        isSecondaryPair_ = false;
    }
    GetBluetoothWatchInterface()->sendIsSecondPhonePair(isSecondaryPair_);
}

void WatchService::UpdataHighPowerMode(bool enable)
{
    CHECK_AND_RETURN_LOG(bluetoothInterface != nullptr, "bluetoothInterface is null!");
    if (lastHighPowerEnable_ == enable) {
        return;
    }
    if (bluetoothInterface->enableBluetoothHighpower != nullptr) {
        HILOGI("UpdataHighPowerMode = %{public}d.", enable);
        lastHighPowerEnable_ = enable;
        bluetoothInterface->enableBluetoothHighpower(enable);
    }
}

bool WatchService::HidGetMapUpdateingState()
{
    return isHidMapUpdateing_;
}

void WatchService::HidUpdateMap()
{
    int32_t ret = 0;
    if (bluetoothHidInterface == nullptr) {
        HILOGE("WatchService::HidUpdateMap bluetoothHidInterface is null");
        return;
    }
    if (hidConnectionState_ != BTHD_CONN_STATE_CONNECTED) {
        HILOGE("WatchService::HidUpdateMap hidConnectionState_: %{public}d", hidConnectionState_);
        return;
    }
    int hidMapUpdateState = GetIntParameter(HID_MAP_UPDATE, 0);
    HILOGI("WatchService::HidUpdateMap hidMapUpdateState: %{public}d, hidConnectionState_: %{public}d",
        hidMapUpdateState, hidConnectionState_);
    if ((hidMapUpdateState > 0)) {
        HILOGI("WatchService::HidUpdateMap map has updated");
        return;
    }
    ret = bluetoothHidInterface->virtualCableUnplug();
    if (ret == BT_STATUS_SUCCESS) {
        isHidMapUpdateing_ = true;
        SetParameter(HID_MAP_UPDATE, g_stateOn);
    } else {
        HILOGE("WatchService::HidUpdateMap virtual_cable_unplug: %{public}d", ret);
    }
}

void WatchService::HidReconnectAfterUpdateMap(const RawAddress &device, bool isAclConnect)
{
    HILOGI("WatchService::HidReconnectAfterUpdateMap brHidReconnectCount_: %{public}d", brHidReconnectCount_);
    if (brHidReconnectCount_ >= BR_HID_RECONNECTED_COUNT_MAX) {
        return;
    }
    if (isAclConnect && (hidConnectionState_ == BTHD_CONN_STATE_DISCONNECTED)) {
        brHidReconnectCount_++;
        StartHidReconnectTimer(device);
    }
}

void WatchService::HidVolumeIncrement()
{
    int32_t ret = 0;
    HidConsumerControl hidVolumeIncrementData = {
        .reportId = HID_CONSUMER_CONTROL_REPORT_ID,
    };
    hidVolumeIncrementData.hidConsumerControlUsAgeData.dataType =
        1 << static_cast<uint8_t>(HidConsumerControlUsageBitField::HID_CONSUMER_CONTROL_USAGE_BIT_VOLUME_INCREMENT);
    hidVolumeIncrementData.hidConsumerControlUsAgeData.invalidData = 0;
    if (bluetoothHidInterface == nullptr) {
        HILOGE("WatchService::HidVolumeIncrement bluetoothHidInterface is null");
        return;
    }
    if (hidConnectionState_ != BTHD_CONN_STATE_CONNECTED) {
        HILOGE("WatchService::HidVolumeIncrement hidConnectionState_: %{public}d", hidConnectionState_);
        return;
    }
    HILOGI("WatchService::HidVolumeIncrement HidVolumeIncrementDataSize: %{public}d", sizeof(HidConsumerControlUsAge));
    HoldRunningLock(WAKE_TIME);
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidVolumeIncrementData.reportId,
        sizeof(HidConsumerControlUsAge), (uint8_t *)&hidVolumeIncrementData.hidConsumerControlUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidVolumeIncrement send_report: %{public}d", ret);
    }

    hidVolumeIncrementData.hidConsumerControlUsAgeData.dataType = 0;
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidVolumeIncrementData.reportId,
        sizeof(HidConsumerControlUsAge), (uint8_t *)&hidVolumeIncrementData.hidConsumerControlUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidVolumeIncrement send_report release: %{public}d", ret);
    }
    ReleaseRunningLock();
}

void WatchService::HidVolumeDecrement()
{
    int32_t ret = 0;
    HidConsumerControl hidVolumeDecrementlData = {
        .reportId = HID_CONSUMER_CONTROL_REPORT_ID,
    };
    hidVolumeDecrementlData.hidConsumerControlUsAgeData.dataType =
        1 << static_cast<uint8_t>(HidConsumerControlUsageBitField::HID_CONSUMER_CONTROL_USAGE_BIT_VOLUME_DECREMENT);
    hidVolumeDecrementlData.hidConsumerControlUsAgeData.invalidData = 0;
    if (bluetoothHidInterface == nullptr) {
        HILOGE("WatchService::HidVolumeDecrement bluetoothHidInterface is null");
        return;
    }
    if (hidConnectionState_ != BTHD_CONN_STATE_CONNECTED) {
        HILOGE("WatchService::HidVolumeDecrement hidConnectionState_: %{public}d", hidConnectionState_);
        return;
    }
    HILOGI("WatchService::HidVolumeDecrement HidVolumeDecrementDataSize: %{public}d", sizeof(HidConsumerControlUsAge));
    HoldRunningLock(WAKE_TIME);
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidVolumeDecrementlData.reportId,
        sizeof(HidConsumerControlUsAge), (uint8_t *)&hidVolumeDecrementlData.hidConsumerControlUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidVolumeDecrement send_report: %{public}d", ret);
    }

    hidVolumeDecrementlData.hidConsumerControlUsAgeData.dataType = 0;
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidVolumeDecrementlData.reportId,
        sizeof(HidConsumerControlUsAge), (uint8_t *)&hidVolumeDecrementlData.hidConsumerControlUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidVolumeDecrement send_report release: %{public}d", ret);
    }
    ReleaseRunningLock();
}

void WatchService::HidDoubleClick()
{
    int32_t ret = 0;
    HidTouchScreen hidClickData = {
        .reportId = HID_REPORT_ID,
    };
    hidClickData.hidTouchScreenUsAgeData.tipSwitch = 1;
    hidClickData.hidTouchScreenUsAgeData.inRange = 1;
    hidClickData.hidTouchScreenUsAgeData.contactIdentifier = 0;
    hidClickData.hidTouchScreenUsAgeData.touchValid = 1;
    hidClickData.hidTouchScreenUsAgeData.x = HID_PLAY_PAUSE_AXIS;
    hidClickData.hidTouchScreenUsAgeData.y = HID_PLAY_PAUSE_AXIS;
    if (bluetoothHidInterface == nullptr) {
        HILOGE("WatchService::HidDoubleClick bluetoothHidInterface is null");
        return;
    }
    if (hidConnectionState_ != BTHD_CONN_STATE_CONNECTED) {
        HILOGE("WatchService::HidDoubleClick hidConnectionState_: %{public}d", hidConnectionState_);
        return;
    }
    HILOGI("WatchService::HidDoubleClick hidClickDataSendDataSize: %{public}d", sizeof(HidTouchScreenUsAge));
    HoldRunningLock(WAKE_TIME);
    for (uint8_t i = 0; i < HID_CLICK_EVENT_TIMES; i++) {
        hidClickData.hidTouchScreenUsAgeData.tipSwitch = 1;
        hidClickData.hidTouchScreenUsAgeData.x = HID_PLAY_PAUSE_AXIS;
        hidClickData.hidTouchScreenUsAgeData.y = HID_PLAY_PAUSE_AXIS;
        ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidClickData.reportId,
            sizeof(HidTouchScreenUsAge), (uint8_t *)&hidClickData.hidTouchScreenUsAgeData);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("WatchService::HidDoubleClick send_report: %{public}d", ret);
        }
        hidClickData.hidTouchScreenUsAgeData.tipSwitch = 0;
        hidClickData.hidTouchScreenUsAgeData.x = 0;
        hidClickData.hidTouchScreenUsAgeData.y = 0;
        ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidClickData.reportId,
            sizeof(HidTouchScreenUsAge), (uint8_t *)&hidClickData.hidTouchScreenUsAgeData);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("WatchService::HidDoubleClick release send_report: %{public}d", ret);
        }
    }
    ReleaseRunningLock();
}

void WatchService::HidSingleClick()
{
    int32_t ret = 0;
    HidTouchScreen hidPlayPauseData = {
        .reportId = HID_REPORT_ID,
    };
    hidPlayPauseData.hidTouchScreenUsAgeData.tipSwitch = 1;
    hidPlayPauseData.hidTouchScreenUsAgeData.inRange = 1;
    hidPlayPauseData.hidTouchScreenUsAgeData.contactIdentifier = 0;
    hidPlayPauseData.hidTouchScreenUsAgeData.touchValid = 1;
    hidPlayPauseData.hidTouchScreenUsAgeData.x = HID_PLAY_PAUSE_AXIS;
    hidPlayPauseData.hidTouchScreenUsAgeData.y = HID_PLAY_PAUSE_AXIS;
    if (bluetoothHidInterface == nullptr) {
        HILOGE("WatchService::HidSingleClick bluetoothHidInterface is null");
        return;
    }
    if (hidConnectionState_ != BTHD_CONN_STATE_CONNECTED) {
        HILOGE("WatchService::HidSingleClick hidConnectionState_: %{public}d", hidConnectionState_);
        return;
    }
    HILOGI("WatchService::HidSingleClick HidPlayPauseSendDataSize: %{public}d", sizeof(HidTouchScreenUsAge));
    HoldRunningLock(WAKE_TIME);
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidPlayPauseData.reportId,
        sizeof(HidTouchScreenUsAge), (uint8_t *)&hidPlayPauseData.hidTouchScreenUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidSingleClick send_report: %{public}d", ret);
    }
    hidPlayPauseData.hidTouchScreenUsAgeData.tipSwitch = 0;
    hidPlayPauseData.hidTouchScreenUsAgeData.x = 0;
    hidPlayPauseData.hidTouchScreenUsAgeData.y = 0;
    ret = bluetoothHidInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, hidPlayPauseData.reportId,
        sizeof(HidTouchScreenUsAge), (uint8_t *)&hidPlayPauseData.hidTouchScreenUsAgeData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("WatchService::HidSingleClick release send_report: %{public}d", ret);
    }
    ReleaseRunningLock();
}

bool WatchService::IsValidAddress(const std::string &address)
{
    if (address.empty()) {
        HILOGE("WatchService::IsValidAddress address is empty.");
        return false;
    }
    const std::regex bdAddrRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(address, bdAddrRegex);
}

#ifdef __cplusplus
extern "C" {
#endif
/**********  Function hook  ***************/
void OnBluetoothOnHook(void)
{
    WatchService::GetInstance()->BluetoothIsOn();
}

void OnBluetoothOffHook(void)
{
    WatchService::GetInstance()->Uninitialize();
}

void OnBluetoothInitHook(void)
{
    WatchService::GetInstance()->Initialize();
}

void BluetoothTurningOffHook(void)
{
    WatchService::GetInstance()->NotifyBluetoothTurningOff();
}

bool IsAutoConnectEnabledHook(void)
{
#ifndef BLUETOOTH_INTERCEPT_PERIPHERAL_FIRST_PAIR_DIALOG
    return false;
#else
    return true;
#endif
}

void OnAclDisconnectReasonHook(const RawAddress &device, int reason)
{
    WatchService::GetInstance()->DeviceDisconnectCallBack(device, reason);
}

bool IsAllowHfpAgCallHook(std::string address)
{
    return WatchService::GetInstance()->IsAllowAgCall(address);
}

bool IsAllowConnectHfpAgScoHook(void)
{
    return GlobalDeviceManager::getInstance().IsAllowConnectHfpAgSco();
}

const char* GetBleMinConnectionIntervalHook()
{
    return PROPERTY_HIGH_PRIORITY_MAX_INTERVAL.c_str();
}

bool IsEnableBluetoothAfterExitStrModeHook()
{
    if (WatchService::GetInstance()->IsEnableBluetoothAfterExitStrMode()) {
        return true;
    }

    return false;
}

#ifdef BLUETOOTH_OOBE_DIALOG
bool IsAllowPairDialogHook(const RawAddress &device)
{
    return WatchService::GetInstance()->IsAllowSystemPairDialog(device);
}
#endif

int StrModeAbnormalExitHandleHook(const BTTransport transport)
{
    return WatchService::GetInstance()->StrModeAbnormalExitHandle(transport);
}

void OnBluetoothDestroyHook()
{
    WatchService::GetInstance()->UnSubscribeWatchEnterStrEvent();
}

bool QuickStopOffloadPlayHook()
{
    if (GlobalDeviceManager::getInstance().IsAllowConnectHfpAgSco()) {
        return false;
    }
    return true;
}

bool StartDialHook(std::string address, const int hfpState, const std::string &number)
{
    WatchService::GetInstance()->SetDialingByWatch(address);
    if ((hfpState == HFP_HF_STATE_DISCONNECTED && WatchService::GetInstance()->ConnectHfp(address)) ||
        (hfpState == HFP_HF_STATE_CONNECTING)) {
        WatchService::GetInstance()->SetDialedCallNumber(number);
        return false;
    }
    return true;
}
/**********  Function hook  ***************/
#ifdef __cplusplus
}
#endif
}
}
