/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_pc_power"
#endif

#include "pc_power_manager.h"
#include <sstream>
#include <vector>
#include "adapter_manager.h"
#include "bt_def.h"
#include "interface_adapter_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hw_interface.h"
#include "service_util.h"
#include "connect_strategy_manager.h"
#include "bluetooth_common_event_subscriber.h"
#include "thread_util.h"
#include "battery_srv_client.h"

namespace OHOS {
namespace bluetooth {
CurrentPowerState IPowerState::state_;
static const uint8_t ENABLED_FILTER = 0x01;
static const uint8_t DISABLED_FILTER = 0x00;
constexpr uint32_t CORE_EVENT_PRIORITY = 1;
const uint8_t HW_ECHO_PRODUCT_TABLET = 0x07;
static const std::vector<std::string> NO_AUTO_CONNECT_LIST = {
    "311",    // Watch Buds
    "367"     // Watch Buds 2
};
std::mutex g_stateMutex;

void IPowerState::AddEvent(const uint8_t event)
{
    HILOGI("Add event %{public}u.", event);
    std::unique_lock<std::mutex> lock(g_stateMutex);
    switch (event) {
        case static_cast<uint8_t>(PowerEvent::STATE_SWITCH_ON):
            state_.switchState = PowerEvent::STATE_SWITCH_ON;
            break;
        case static_cast<uint8_t>(PowerEvent::STATE_SWITCH_OFF):
            state_.switchState = PowerEvent::STATE_SWITCH_OFF;
            break;
        case static_cast<uint8_t>(PowerEvent::STATE_INSERT_SCREEN):
            state_.extrernalScreenState = PowerEvent::STATE_INSERT_SCREEN;
            break;
        case static_cast<uint8_t>(PowerEvent::STATE_REMOVEING_SCREEN):
            state_.extrernalScreenState = PowerEvent::STATE_REMOVEING_SCREEN;
            break;
        case static_cast<uint8_t>(PowerEvent::STATE_ENTER_FORCESLEEP):
            state_.forcesleepState = PowerEvent::STATE_ENTER_FORCESLEEP;
            break;
        case static_cast<uint8_t>(PowerEvent::STATE_EXIT_FORCESLEEP):
            state_.forcesleepState = PowerEvent::STATE_EXIT_FORCESLEEP;
            break;
        default:
            break;
    }
}

SwitchEvent::SwitchEvent()
{}

SwitchEvent::~SwitchEvent()
{}

PowerEvent SwitchEvent::State()
{
    std::unique_lock<std::mutex> lock(g_stateMutex);
    return state_.switchState;
}

void SwitchEvent::HandleEvent(const std::string &eventData)
{
    HILOGI("Handle switch event, %{public}s.", eventData.c_str());
    std::unique_lock<std::mutex> lock(g_stateMutex);
    auto existExternalScreen = ExistExternalScreen(eventData);
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    if (!existExternalScreen && state_.switchState == PowerEvent::STATE_SWITCH_OFF) {
        HILOGI("enable filter,not exist external screen and recive switch off event.");
        CmdFilterHdiToHisi(ENABLED_FILTER);
    } else if (!existExternalScreen && state_.switchState == PowerEvent::STATE_SWITCH_ON) {
        HILOGI("disable filter,not exist external screen and recive switch on event.");
        CmdFilterHdiToHisi(DISABLED_FILTER);
    } else if (existExternalScreen) {
        HILOGI("disable filter,exist external screen.");
        CmdFilterHdiToHisi(DISABLED_FILTER);
    }
#endif
}

ExternalScreenEvent::ExternalScreenEvent()
{}

ExternalScreenEvent::~ExternalScreenEvent()
{}

PowerEvent ExternalScreenEvent::State()
{
    std::unique_lock<std::mutex> lock(g_stateMutex);
    return state_.extrernalScreenState;
}

void ExternalScreenEvent::HandleEvent(const std::string &eventData)
{
    HILOGI("Handle ExternalScreen event, %{public}s.", eventData.c_str());
    std::unique_lock<std::mutex> lock(g_stateMutex);
    bool exsitExternalScreen = ExistExternalScreen(eventData);
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    if (!exsitExternalScreen && state_.switchState == PowerEvent::STATE_SWITCH_OFF) {
        HILOGI("not exist external screen and switch off state, enable filter.");
        CmdFilterHdiToHisi(ENABLED_FILTER);
    } else {
        HILOGI("switchState %{public}d, exsitExternalScreen %{public}d. disable filter",
            state_.switchState, exsitExternalScreen);
        CmdFilterHdiToHisi(DISABLED_FILTER);
    }
#endif
}

ForceSleep::ForceSleep()
{}

ForceSleep::~ForceSleep()
{}

PowerEvent ForceSleep::State()
{
    std::unique_lock<std::mutex> lock(g_stateMutex);
    return state_.forcesleepState;
}

void ForceSleep::HandleEnterForceSleepEvent()
{
    HILOGI("handle enter force sleep");
    std::unique_lock<std::mutex> lock(g_stateMutex);
    if ((state_.forcesleepState == PowerEvent::STATE_ENTER_FORCESLEEP) && !isForceSleep_) {
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
        DisconnectExceptMouseAndKeyboard();
#endif
        isForceSleep_ = true;
    }
    StopHandleEnterForceSleepEventTimer();
}

void ForceSleep::StartHandleEnterForceSleepEventTimer()
{
    HILOGI("start handle enter force sleep event timer");
    std::lock_guard<std::mutex> lock(handleEnterForceSleepEventTimeoutMutex_);
    handleEnterForceSleepEventTimeout_ = std::make_shared<utility::Timer>(
        std::bind(&bluetooth::ForceSleep::HandleEnterForceSleepEvent, this));
    handleEnterForceSleepEventTimeout_->Start(TIMEOUT_HANDLE_ENTER_FORCE_SLEEP_EVENT);
}

void ForceSleep::StopHandleEnterForceSleepEventTimer()
{
    HILOGI("stop handle enter force sleep event timer");
    std::lock_guard<std::mutex> lock(handleEnterForceSleepEventTimeoutMutex_);
    if (handleEnterForceSleepEventTimeout_ != nullptr) {
        handleEnterForceSleepEventTimeout_->Stop();
    }
    handleEnterForceSleepEventTimeout_ = nullptr;
}

bool ForceSleep::IsHandleEnterForceSleepEventTimerStarted()
{
    std::lock_guard<std::mutex> lock(handleEnterForceSleepEventTimeoutMutex_);
    return handleEnterForceSleepEventTimeout_ != nullptr;
}

void ForceSleep::SetLeConnectionScanToFast(bool enable)
{
    HILOGI("SetLeConnectionScanToFast:%{public}d", enable);
    if (IsPairedMouseOrKeyboard()) {
        const bthwif_interface_t *bthwifInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bthwifInterface != nullptr, "bthwifInterface nullptr");
        bthwifInterface->hwSetLeConnectionScanToFast(enable);
    }
}

void ForceSleep::HandleEvent(const std::string &eventData)
{
    HILOGI("Handle force sleep event.");
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    std::unique_lock<std::mutex> lock(g_stateMutex);
    if ((state_.forcesleepState == PowerEvent::STATE_ENTER_FORCESLEEP) && !isForceSleep_) {
        if (IsHandleEnterForceSleepEventTimerStarted()) {
            StopHandleEnterForceSleepEventTimer();
        }
        StartHandleEnterForceSleepEventTimer();
    } else if ((state_.forcesleepState == PowerEvent::STATE_EXIT_FORCESLEEP) && isForceSleep_) {
        if (IsHandleEnterForceSleepEventTimerStarted()) {
            StopHandleEnterForceSleepEventTimer();
        }
        ForceSleepAutoBtConnect();
        isForceSleep_ = false;
    }
#endif
}

bool ForceSleep::IsPairedMouseOrKeyboard()
{
    std::vector<RawAddress> rawAddrVec;
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, false, "classicService nullptr");
    rawAddrVec = classicService->GetPairedDevices();
    for (const auto &realAddr : rawAddrVec) {
        int remoteCod = classicService->GetDeviceClass(realAddr);
        if (CheckCod(remoteCod, COD_HID_POINTING) || CheckCod(remoteCod, COD_HID_KEYBOARD)) {
            HILOGI("is mouse or keyboard device %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
            return true;
        }
    }
    return false;
}

void ForceSleep::DisconnectExceptMouseAndKeyboard()
{
    std::vector<RawAddress> rawAddrVec;
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (!classicService) {
        HILOGE("classicService is nullptr.");
        return;
    }
    rawAddrVec = classicService->GetPairedDevices();
    for (const auto &realAddr : rawAddrVec) {
        if (!classicService->IsAclConnected(realAddr)) {
            continue;
        }
        int remoteCod = classicService->GetDeviceClass(realAddr);
        if (CheckCod(remoteCod, COD_HID_POINTING) || CheckCod(remoteCod, COD_HID_KEYBOARD)) {
            HILOGE("is mouse or keyboard device %{public}s,continue", GetEncryptAddr(realAddr.GetAddress()).c_str());
            continue;
        }

        HILOGI("Disconnect allowed profiles %{public}s.", GetEncryptAddr(realAddr.GetAddress()).c_str());
        classicService->DisconnectAllowedProfilesNotSetVirtualConnSwitch(realAddr);
    }
}

bool IPowerState::CheckCod(uint32_t remoteCod, const uint32_t &cod)
{
    remoteCod &= COD_MASK;
    return remoteCod == cod;
}

void ForceSleep::ForceSleepStopBtScan()
{
    HILOGI("force sleep:stop bt scan.");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (classicService && classicService->CancelBtDiscovery()) {
        HILOGI("stop br scan success.");
    }

    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService) {
        HILOGI("force sleep:stop ble scan.");
        bleService->StopScan();
    }
}

void ForceSleep::ForceSleepStartBtScan()
{
    auto bleService = IAdapterManager::GetInstance()->GetBleAdapterInterface();
    if (bleService) {
        HILOGI("force sleep:start ble scan.");
        bleService->StartScan(PowerMgrHelper::GetInstance()->GetBleSettings());
    }
}

void ForceSleep::ForceSleepAutoBtConnect()
{
    HILOGI("Force sleep: bt device auto connect.");
    ConnectStrategyManager::GetInstance()->AutoConnect();
}

HidHostCallback::HidHostCallback() {}

HidHostCallback::~HidHostCallback() {}

void HidHostCallback::OnConnectionStateChanged(const RawAddress &device, int state, int cause)
{
    HILOGI("device %{public}s OnConnectionStateChanged state: %{public}d, cause: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), state, cause);
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicService != nullptr, "classicService nullptr");
    int remoteCod = classicService->GetDeviceClass(device);
    if (!ForceSleep::GetInstance()->CheckCod(remoteCod, COD_HID_POINTING) &&
        !ForceSleep::GetInstance()->CheckCod(remoteCod, COD_HID_KEYBOARD)) {
        HILOGI("not mouse or keyboard");
        return;
    }
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    // pc处理逻辑
    if (state == static_cast<int>(BTConnectState::CONNECTED) &&
        ExternalScreenEvent::GetInstance()->State() == PowerEvent::STATE_REMOVEING_SCREEN &&
        SwitchEvent::GetInstance()->State() == PowerEvent::STATE_SWITCH_OFF) {
        HILOGI("enable filter, not exist external screen and switch off event.");
        IPowerState::GetInstance()->CmdFilterHdiToHisi(ENABLED_FILTER);
    }
#endif
    // pc&平板处理逻辑
    if (state == static_cast<int>(BTConnectState::DISCONNECTED) && PowerMgrHelper::GetInstance()->isPowerOff_.load()
        && !PowerMgrHelper::GetInstance()->isFastScan_.load()) {
        // 不插电场景,若此时扫描策略为slowscan，有设备断链要修改连接为slow mode 10%扫描占空比
        HILOGI("pc/tablet power disconnected state hid device disconnect");
        ForceSleep::GetInstance()->SetLeConnectionScanToFast(false);
    }
}

AdapterStateCallback::~AdapterStateCallback() {}

void AdapterStateCallback::OnBluetoothStateChanged(int state)
{
    HILOGI("OnBluetoothStateChanged state: %{public}d, isPowerOff_:%{public}d, isFastScan_:%{public}d", state,
        PowerMgrHelper::GetInstance()->isPowerOff_.load(), PowerMgrHelper::GetInstance()->isFastScan_.load());
    // pc&平板不插电场景,若此时扫描策略为slowscan，蓝牙打开时要修改连接为slow mode 10%扫描占空比
    if (state == BluetoothSwitchState::STATE_ON && PowerMgrHelper::GetInstance()->isPowerOff_.load()
        && !PowerMgrHelper::GetInstance()->isFastScan_.load()) {
        HILOGI("pc/tablet power disconnected state bluetooth on");
        ForceSleep::GetInstance()->SetLeConnectionScanToFast(false);
    }
}

void IPowerState::CmdFilterHdiToHisi(const uint8_t &enabled)
{
    std::vector<RawAddress> rawAddrVec;
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    if (!classicService) {
        HILOGE("classicService is nullptr.");
        return;
    }
    rawAddrVec = classicService->GetPairedDevices();
    for (const auto &realAddr : rawAddrVec) {
        int32_t transport = DeviceTypeToTransport(classicService->GetDeviceType(realAddr));
        HILOGI("Filter device %{public}s,deviceType %{public}d.",
            GetEncryptAddr(realAddr.GetAddress()).c_str(),
            transport);
        if (!classicService->IsAclConnected(realAddr)) {
            HILOGE("Not connected device %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
            continue;
        }
        int remoteCod = classicService->GetDeviceClass(realAddr);
        if (!CheckCod(remoteCod, COD_HID_POINTING) && !CheckCod(remoteCod, COD_HID_KEYBOARD)) {
            HILOGE("Not mouse or keyboard device %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
            continue;
        }
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(realAddr);
        ForceSleepFilterHidData filterHidData = {enabled, transport, rawAddr};
        UnionCmdToHisi unionCmd;
        unionCmd.hidData = filterHidData;
        SpecificCmdToHisi cmd = {unionCmd, HCI_FILTER_MOUSE_OR_KEYBOARD_OPCODE};
        bluetoothHwSrcInterface->hwVendorSpecificCommand(&cmd);
    }
}

bool IPowerState::ExistExternalScreen(const std::string &eventData)
{
    std::stringstream sEventData(eventData);
    int interEventData = 0;
    sEventData >> interEventData;
    if (interEventData == 0) {
        state_.extrernalScreenState = PowerEvent::STATE_REMOVEING_SCREEN;
        return false;
    } else {
        state_.extrernalScreenState = PowerEvent::STATE_INSERT_SCREEN;
        return true;
    }
}

void IPowerState::HandleEvent(const std::string &eventData)
{
    auto exsitExternalScreen = ExistExternalScreen(eventData);
    HILOGI("IPowerState: ExternalScreen:%{public}d", static_cast<int>(exsitExternalScreen));
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    std::unique_lock<std::mutex> lock(g_stateMutex);
    if (exsitExternalScreen) {
        CmdFilterHdiToHisi(DISABLED_FILTER);
    }
#endif
}

PowerEvent IPowerState::State()
{
    return PowerEvent::STATE_POWER_START;
}

PowerMgrHelper::PowerMgrHelper()
{}

PowerMgrHelper::~PowerMgrHelper()
{
    UnRegisterPowermgrCommonEvent();
}

void PowerMgrHelper::RegisterPowermgrCommonEvent()
{
    HILOGI("PowerMgrHelper:RegisterPowermgrCommonEvent");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    subscriberInfo.SetPermission("ohos.permission.ACCESS_BLUETOOTH");
    subscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        HILOGE("PowerMgrHelper RegisterPowermgrCommonEvent failed");
    } else {
        HILOGI("PowerMgrHelper RegisterPowermgrCommonEvent OK");
    }
    EventFwk::MatchingSkills screenMatchingSkills;
    screenMatchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    screenMatchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    screenMatchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventSubscribeInfo screenSubscriberInfo(screenMatchingSkills);
    screenSubscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    screenSubscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(screenSubscriberInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(screenSubscriber_)) {
        HILOGE("PowerMgrHelper RegisterPowermgrCommonEvent failed");
    }
    if (hidHostService_ == nullptr) {
        hidHostService_ =  static_cast<IProfileHidHost *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_HOST));
    }
    CHECK_AND_RETURN_LOG(hidHostService_ != nullptr, "hidHostService_ nullptr");
    if (hidHostCallback_ == nullptr) {
        hidHostCallback_ = new(std::nothrow)HidHostCallback();
    }
    CHECK_AND_RETURN_LOG(hidHostCallback_ != nullptr, "hidHostCallback_ nullptr");
    hidHostService_->RegisterObserver(*hidHostCallback_);
    // 增加监测蓝牙状态回调（存在拔电场景电源管理关开蓝牙情况）
    auto pluggedType = PowerMgr::BatterySrvClient::GetInstance().GetPluggedType();
    if (pluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE &&
        pluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT) {
        HILOGI("PowerMgrHelper:set power connect state");
        isPowerOff_.store(false);
    } else {
        HILOGI("PowerMgrHelper:set power disconnect state");
        isFastScan_.store(false);
    }
    if (adapterStateCallback_ == nullptr) {
        adapterStateCallback_ = new(std::nothrow)AdapterStateCallback();
    }
    CHECK_AND_RETURN_LOG(adapterStateCallback_ != nullptr, "adapterStateCallback_ nullptr");
    IAdapterManager::GetInstance()->RegisterStateObserver(*adapterStateCallback_);
    HILOGI("PowerMgrHelper:finish RegisterObserver");
}

void PowerMgrHelper::UnRegisterPowermgrCommonEvent()
{
    HILOGI("PowerMgrHelper:UnRegisterPowermgrCommonEvent");
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
    if (hidHostService_ != nullptr) {
        hidHostService_->DeregisterObserver(*hidHostCallback_);
        hidHostService_ = nullptr;
    }
    if (hidHostCallback_ != nullptr) {
        delete(hidHostCallback_);
        hidHostCallback_ = nullptr;
    }
    if (adapterStateCallback_ != nullptr) {
        delete(adapterStateCallback_);
        adapterStateCallback_ = nullptr;
    }
    HILOGI("PowerMgrHelper:finish DeregisterObserver");
}

void PowerMgrHelper::OnPowerStateChanged(const uint8_t &state, const std::string &eventData)
{
#ifdef PC_FEATURE_FORCESLEEP_SUPPORT
    HILOGI("PowerMgrHelper: Power State Changed");
    auto powerStatePtr = PowerStateFactory::Instance()->CreatePowerState(state);
    if (!powerStatePtr) {
        HILOGE("PowerStatePtr is nullptr.");
        return;
    }
    powerStatePtr->AddEvent(state);
    powerStatePtr->HandleEvent(eventData);
#endif
}

void PowerMgrHelper::OnPowerConnectStateChanged(const std::string &action)
{
    // pc&平板逻辑生效
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        HILOGI("pc/tablet recv power connect state");
        isPowerOff_.store(false);
        // 插电,修改连接为fast mode 50%扫描占空比
        if (!isFastScan_.load()) {
            ForceSleep::GetInstance()->SetLeConnectionScanToFast(true);
            isFastScan_.store(true);
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        HILOGI("pc/tablet recv power disconnect state");
        isPowerOff_.store(true);
        // 拔电,修改连接为fast mode 10%扫描占空比
        if (!isFastScan_.load()) {
            return;
        }
        ForceSleep::GetInstance()->SetLeConnectionScanToFast(false);
        isFastScan_.store(false);
    }
}

void PowerMgrHelper::OnScreenStateChanged(const std::string &action)
{
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        HILOGI("recv screen on state");
        if (needReconnectActiveDevice_.load()) {
            HILOGI("needReconnectActiveDevice");
            IAdapterManager::GetInstance()->SetQuietMode(false);
            if (!CheckIfNeedAutoConnect()) {
                needReconnectActiveDevice_.store(false);
                HILOGI("no need auto connect");
                return;
            }
            ForceSleep::GetInstance()->ForceSleepAutoBtConnect();
            needReconnectActiveDevice_.store(false);
        }
    }
}

bool PowerMgrHelper::CheckIfNeedAutoConnect()
{
    std::string autoConnectDeviceAddr = "";
    CHECK_AND_RETURN_LOG_RET(ConnectStrategyManager::GetInstance()->QueryAutoConnectDevice(autoConnectDeviceAddr),
        false, "get autoConnectDeviceAddr error.");
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        RawAddress device(autoConnectDeviceAddr);
        std::string remoteModelId = "";
        if (!classicAdapter->GetRemoteDevicePropertyInfo(device, BT_PROPERTY_REMOTE_MODEL_ID, remoteModelId)) {
            return true;
        }
        HILOGI("remoteModelId: %{public}s", remoteModelId.c_str());
        if (std::find(NO_AUTO_CONNECT_LIST.begin(), NO_AUTO_CONNECT_LIST.end(), remoteModelId)
            != NO_AUTO_CONNECT_LIST.end()) {
            HILOGI("Force sleep: current device not allowed auto connect");
            return false;
        }
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS