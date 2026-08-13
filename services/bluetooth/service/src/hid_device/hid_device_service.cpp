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
#define LOG_TAG "bt_service_hid_device"
#endif

#include "permission_manager.h"
#include "adapter_config.h"
#include "thread_util.h"
#include "adapter_manager.h"
#include "bt_chr_ue_manager.h"
#include "bt_chr_dft_exception.h"
#include "base_observer_list.h"
#include "class_creator.h"
#include "connect_strategy_manager.h"
#include "log.h"
#include "common_util.h"
#include "profile_config.h"
#include "hid_device_service.h"
#include "profile_service_manager.h"
#include "hitrace_meter.h"
#include "bluetooth_state_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

constexpr int MAX_CONNECT_NUM = 1;


HidDeviceService::HidDeviceService() : utility::Context(PROFILE_NAME_HID_DEVICE, "1.1.2"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{}

HidDeviceService::~HidDeviceService()
{
    HILOGI("~HidDeviceService()");
}

BaseObserverList<IHidDeviceObserver>& HidDeviceService::GetHidDeviceObservers()
{
    return hidDeviceObservers_;
}

HidDeviceService *HidDeviceService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    return static_cast<HidDeviceService *>(servManager->GetProfileService(PROFILE_NAME_HID_DEVICE));
}

void HidDeviceService::RegisterObserver(IHidDeviceObserver &hidDeviceObserver)
{
    hidDeviceObservers_.Register(hidDeviceObserver);
}

void HidDeviceService::DeregisterObserver(IHidDeviceObserver &hidDeviceObserver)
{
    hidDeviceObservers_.Deregister(hidDeviceObserver);
}

void HidDeviceService::LoadHidDeviceManagerInterfaceLib(void)
{
    std::lock_guard<std::mutex> lock(loadHidDeviceMutex_);
    if (!isLoaded_) {
        loader_.OpenLib();
    }
    serviceManagerIntfImpl_ = loader_.GetLibInstance();
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("load service impl failed!");
        return;
    }
    isLoaded_ = true;
    hidDeviceObserver_ = std::make_shared<HidDeviceObserver>();
    serviceManagerIntfImpl_->RegisterObserver(hidDeviceObserver_);
}

utility::Context *HidDeviceService::GetContext()
{
    return this;
}


void HidDeviceService::Enable()
{
    HILOGI("enable");
    HidDeviceMessage event(HID_DEVICE_SERVICE_STARTUP_EVT);
    PostEvent(event);
}

void HidDeviceService::Disable()
{
    HidDeviceMessage event(HID_DEVICE_SERVICE_SHUTDOWN_EVT);
    PostEvent(event);
}

int HidDeviceService::Connect(const RawAddress &device)
{
    CHECK_AND_RETURN_LOG_RET(IsAppRegistered(), BT_ERR_HID_APP_NOT_REGISTER, "hid device did not register before");
    if (IsConnected()) {
        HILOGE("app has connected");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGE("Connection Strategy is FORBIDDEN!");
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl_, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "hid device service manager impl not load");
    {
        std::lock_guard<std::mutex> lock(hidDeviceMutex_);
        device_ = device;
    }
    HidDeviceMessage event(HID_DEVICE_CONNECT_EVT);
    event.dev_ = device;
    PostEvent(event);
    return BT_NO_ERROR;
}

int HidDeviceService::DisconnectHidDevice()
{
    CHECK_AND_RETURN_LOG_RET(IsAppRegistered(), BT_ERR_HID_APP_NOT_REGISTER, "hid device did not register before");
    if (!IsConnected()) {
        return BT_NO_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(serviceManagerIntfImpl_, Bluetooth::BT_ERR_INTERNAL_ERROR,
        "hid device service manager impl not load");
    HidDeviceMessage event(HID_DEVICE_DISCONNECT_EVT);
    {
        std::lock_guard<std::mutex> lock(hidDeviceMutex_);
        event.dev_ = device_;
    }
    PostEvent(event);
    return BT_NO_ERROR;
}

int HidDeviceService::Disconnect(const RawAddress &device)
{
    return BT_NO_ERROR;
}

std::list<RawAddress> HidDeviceService::GetConnectDevices()
{
    if (!IsConnected()) {
        return {};
    }
    std::lock_guard<std::mutex> lock(hidDeviceMutex_);
    return {device_};
}

void HidDeviceService::SetIsForeground(bool isForeground)
{
    isForeground_.store(isForeground);
}

bool  HidDeviceService::GetIsForeground()
{
    return isForeground_.load();
}

void HidDeviceService::SetIsConnected(bool isConnected)
{
    isConnected_.store(isConnected);
}

bool HidDeviceService::IsConnected()
{
    return isConnected_.load();
}

int HidDeviceService::GetConnectState()
{
    auto connectState = BluetoothStateManager::GetInstance()->GetProfileConnectState(PROFILE_NAME_HID_DEVICE);
    return (connectState == BTConnectState::CONNECTED ? PROFILE_STATE_CONNECTED : PROFILE_STATE_DISCONNECTED);
}

int HidDeviceService::GetConnectionState(const bluetooth::RawAddress &device)
{
    std::lock_guard<std::mutex> lock(hidDeviceMutex_);
    if (IsConnected() && device == device_) {
        return static_cast<int>(BTConnectState::CONNECTED);
    }
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

int HidDeviceService::GetMaxConnectNum()
{
    return MAX_CONNECT_NUM;
}

bool HidDeviceService::IsRemoteHidHostSupported(const bluetooth::RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HID_HOST)) != uuids.end() ||
        std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_HOGP)) != uuids.end();
}

int HidDeviceService::RegisterHidDevice(BluetoothHidDeviceSdp sdp, 
        BluetoothHidDeviceQos inQos, BluetoothHidDeviceQos outQos)
{
    CHECK_AND_RETURN_LOG_RET(!IsAppRegistered(), BT_ERR_HID_APP_HAS_BEEN_REGISTERED,
        "hid device has registered before");
    {
        std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
        hidDeviceAppInfo_ = GetHidDeviceAppInfo();
    }
   
    LoadHidDeviceManagerInterfaceLib();
    std::string bundleName = PermissionManager::GetCallingName();
    HILOGI("%{public}s app register hid device", bundleName.c_str());
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("load service impl failed!");
        return BT_ERR_INTERNAL_ERROR;
    }
    return serviceManagerIntfImpl_->RegisterHidDevice(sdp, inQos, outQos, bundleName);
}

int HidDeviceService::UnregisterHidDevice()
{
    if (!IsAppRegistered()) {
        HILOGI("hid device did not registered before");
        return BT_NO_ERROR;
    }
    HidDeviceAppInfo info = GetHidDeviceAppInfo();
    {
        std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
        CHECK_AND_RETURN_LOG_RET(info == hidDeviceAppInfo_, BT_ERR_INTERNAL_ERROR,
            "dismatched hid device app info");
    }
    HidDeviceMessage event(HID_DEVICE_UNREGISTER_HID_DEVICE_EVT);
    PostEvent(event);
    return BT_NO_ERROR;
}

int HidDeviceService::AutoUnregisterHidDevice()
{
    HidDeviceMessage event(HID_DEVICE_AUTOUNREGISTER_HID_DEVICE_EVT);
    PostEvent(event);
    return BT_NO_ERROR;
}

int32_t HidDeviceService::SendReport(int id, const std::vector<uint8_t> &data)
{
    CHECK_AND_RETURN_LOG_RET(IsAppRegistered(), BT_ERR_HID_APP_NOT_REGISTER,
        "hid device did not registered before");
    HidDeviceAppInfo info = GetHidDeviceAppInfo();
    {
        std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
        CHECK_AND_RETURN_LOG_RET(info == hidDeviceAppInfo_, BT_ERR_INTERNAL_ERROR,
            "dismatched hid device app info");
    }
    if (!IsConnected()) {
        HILOGE("device not connected");
        return BT_ERR_HID_DEVICE_NOT_CONNECTED;
    }
    HidDeviceMessage event(HID_DEVICE_SEND_REPORT_EVT);
    event.data_ = data;
    event.id_ = id;
    PostEvent(event);
    return BT_NO_ERROR;
}

int32_t HidDeviceService::ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data)
{
    CHECK_AND_RETURN_LOG_RET(IsAppRegistered(), BT_ERR_HID_APP_NOT_REGISTER,
        "hid device did not registered before");
    HidDeviceAppInfo info = GetHidDeviceAppInfo();
    {
        std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
        CHECK_AND_RETURN_LOG_RET(info == hidDeviceAppInfo_, BT_ERR_INTERNAL_ERROR,
            "dismatched hid device app info");
    }
    if (!IsConnected()) {
        HILOGE("device not connected");
        return BT_ERR_HID_DEVICE_NOT_CONNECTED;
    }
    HidDeviceMessage event(HID_DEVICE_REPLY_REPORT_EVT);
    event.data_ = data;
    event.id_ = id;
    event.type_ = type;
    PostEvent(event);
    return BT_NO_ERROR;

}

int32_t HidDeviceService::ReportError(uint8_t type)
{
    CHECK_AND_RETURN_LOG_RET(IsAppRegistered(), BT_ERR_HID_APP_NOT_REGISTER,
        "hid device did not registered before");
    HidDeviceAppInfo info = GetHidDeviceAppInfo();
    {
        std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
        CHECK_AND_RETURN_LOG_RET(info == hidDeviceAppInfo_, BT_ERR_INTERNAL_ERROR,
            "dismatched hid device app info");
    }
    if (!IsConnected()) {
        HILOGE("device not connected");
        return BT_ERR_HID_DEVICE_NOT_CONNECTED;
    }
    HidDeviceMessage event(HID_DEVICE_REPORT_ERROR_EVT);
    event.type_ = type;
    PostEvent(event);
    return BT_NO_ERROR;
}

int HidDeviceService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("SetConnectStrategy device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_HID_DEVICE_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t connectionState = GetConnectionState(device);
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        if (connectionState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect hid device.");
        }
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (connectionState != static_cast<int>(BTConnectState::DISCONNECTED)) {
            DisconnectHidDevice();
            HILOGI("disconnect hid device.");
        }
    } else {
        HILOGE("Strategy set failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int HidDeviceService::GetConnectStrategy(const RawAddress &device)
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HID_DEVICE_CONNECTION_POLICY);
}

void HidDeviceService::PostEvent(const HidDeviceMessage &event)
{
    DoInHidDeviceThread([this, event] {this->ProcessEvent(event);});
}

void HidDeviceService::ProcessEvent(const HidDeviceMessage &event)
{
    switch (event.what_) {
        case HID_DEVICE_SERVICE_STARTUP_EVT:
            StartUp();
            break;
        case HID_DEVICE_SERVICE_SHUTDOWN_EVT:
            ShutDown();
            break;
        case HID_DEVICE_CONNECT_EVT:
            ProcessConnectEvent(event);
            break;
        case HID_DEVICE_DISCONNECT_EVT:
            ProcessDisconnectEvent(event);
            break;
        case HID_DEVICE_SEND_REPORT_EVT:
            ProcessSendReportEvent(event);
            break;
        case HID_DEVICE_REPLY_REPORT_EVT:
            ProcessReplyReportEvent(event);
            break;
        case HID_DEVICE_REPORT_ERROR_EVT:
            ProcessReportErrorEvent(event);
            break;
        case HID_DEVICE_UNREGISTER_HID_DEVICE_EVT:
        case HID_DEVICE_AUTOUNREGISTER_HID_DEVICE_EVT:
            ProcessUnregisterHidDeviceEvent(event);
            break;
        default:
            HILOGE("Invalid hid device message");
            break;
    }
}

void HidDeviceService::StartUp()
{
    HILOGI("onenable hid deivce");
    GetContext()->OnEnable(PROFILE_NAME_HID_DEVICE, true);
}

void HidDeviceService::ShutDown()
{
    HILOGI("onedisnable hid deivce");
    GetContext()->OnDisable(PROFILE_NAME_HID_DEVICE, true);
}

void HidDeviceService::ProcessConnectEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    bluetooth::RawAddress device = event.dev_;
    serviceManagerIntfImpl_->Connect(device);
}

void HidDeviceService::ProcessDisconnectEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    bluetooth::RawAddress device = event.dev_;
    serviceManagerIntfImpl_->Disconnect(device);
}

void HidDeviceService::ProcessSendReportEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    std::vector<uint8_t> data = event.data_;
    int32_t id = event.id_;
    serviceManagerIntfImpl_->SendReport(id, data);
}

void HidDeviceService::ProcessReplyReportEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    std::vector<uint8_t> data = event.data_;
    int32_t id = event.id_;
    uint8_t type = event.type_;
    serviceManagerIntfImpl_->ReplyReport(type, id, data);
}

void HidDeviceService::ProcessReportErrorEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    uint8_t type = event.type_;
    serviceManagerIntfImpl_->ReportError(type);
}

void HidDeviceService::ProcessUnregisterHidDeviceEvent(const HidDeviceMessage &event)
{
    if (serviceManagerIntfImpl_ == nullptr) {
        HILOGE("hid device service manager impl not load");
        return;
    }
    serviceManagerIntfImpl_->UnregisterHidDevice();
}

void HidDeviceObserver::OnAppStatusChanged(int state)
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    if (state == HID_DEVICE_UNREGISTERED) {
        hidDeviceService->SetHidDeviceAppInfoEmpty();
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([state](IHidDeviceObserver &observer) {
        observer.OnAppStatusChanged(state);
    });
}
void HidDeviceObserver::OnConnectionStateChanged(bluetooth::RawAddress device, int state)
{
    HILOGI("[HID_SERVICE]HidConnectState = %{public}d", state);
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        hidDeviceService->SetIsConnected(false);
    } else if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        hidDeviceService->SetIsConnected(true);
    }
    hidDeviceService->HidProcessBtChrEvent(state);
    hidDeviceService->GetHidDeviceObservers().ForEach([device, state](IHidDeviceObserver &observer) {
        observer.OnConnectionStateChanged(device, state);
    });
}
void HidDeviceObserver::OnGetReport(int type, int id, uint16_t buffer_size)
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([type, id, buffer_size](IHidDeviceObserver &observer) {
        observer.OnGetReport(type, id, buffer_size);
    });
}
void HidDeviceObserver::OnInterruptDataReceived(int report_id, std::vector<uint8_t> data)
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([report_id, data](IHidDeviceObserver &observer) {
        observer.OnInterruptDataReceived(report_id, data);
    });
}
void HidDeviceObserver::OnSetProtocol(int protocol)
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([protocol](IHidDeviceObserver &observer) {
        observer.OnSetProtocol(protocol);
    });
}
void HidDeviceObserver::OnSetReport(int type, int id, std::vector<uint8_t> data)
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([type, id, data](IHidDeviceObserver &observer) {
        observer.OnSetReport(type, id, data);
    });
}
void HidDeviceObserver::OnVirtualCableUnplug()
{
    HidDeviceService *hidDeviceService = HidDeviceService::GetService();
    if (hidDeviceService == nullptr) {
        HILOGE("hidDeviceService is nullptr");
        return;
    }
    hidDeviceService->GetHidDeviceObservers().ForEach([](IHidDeviceObserver &observer) {
        observer.OnVirtualCableUnplug();
    });
}

bool HidDeviceService::IsAppRegistered()
{
    std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
    return hidDeviceAppInfo_.tokenId != 0 && !hidDeviceAppInfo_.appId.empty() && !hidDeviceAppInfo_.name.empty();
}

void HidDeviceService::SetHidDeviceAppInfoEmpty()
{
    std::lock_guard<std::mutex> lock(hidDeviceAppInfoMutex_);
    hidDeviceAppInfo_.tokenId = 0;
    hidDeviceAppInfo_.appId.clear();
    hidDeviceAppInfo_.name.clear();
}

void HidDeviceService::HidProcessBtChrEvent(int state)
{
    std::string addr;
    {
        std::lock_guard<std::mutex> lock(hidDeviceMutex_);
        addr = device_.GetAddress();
    }
    BtChrDftEventWriteInt(CHR_USER_DISCONNECT, addr, "HIDTYPE", HIDDEVICETYPE);
    if (state == static_cast<int>(BTConnectState::CONNECTING)) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDCONNECTIONTIME");
    } else if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDCONNECTEDTIME");
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, addr, "HIDDISCONNECTTIME");
    }
}

HidDeviceAppInfo HidDeviceService::GetHidDeviceAppInfo()
{
    HidDeviceAppInfo hidDeviceAppInfo;
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (accessRet != 0) {
        HILOGI("current hap is not hap application");
    }
    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    std::string appId = GetHapAppIdentifier(userId, bundleName);
    hidDeviceAppInfo.tokenId = tokenId;
    hidDeviceAppInfo.appId = appId;
    hidDeviceAppInfo.name = PermissionManager::GetCallingName();
    return hidDeviceAppInfo;
}

void HidDeviceService::SetAddress(const std::string &device)
{
    std::lock_guard<std::mutex> lock(hidDeviceMutex_);
    device_.SetAddress(device);
}

REGISTER_CLASS_CREATOR(HidDeviceService);
}  // namespace Bluetooth
}  // namespace OHOS
