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
#define LOG_TAG "bt_service_hid_device_service_impl"
#endif


#include "hid_device_service_impl.h"
#include "permission_manager.h"
#include "connect_strategy_manager.h"
#include "adapter_manager.h"
#include "profile_config.h"
#include "hid_device_defines.h"
#include "thread_util.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;   

static HidDeviceServiceImpl *g_serviceImpl = nullptr;

BthdCallbacks sBluetoothHidDeviceCallbacks = {
    sizeof(sBluetoothHidDeviceCallbacks),
    HidDeviceServiceImpl::OnAppStatusChanged,
    HidDeviceServiceImpl::OnConnectionStateChanged,
    HidDeviceServiceImpl::OnGetReport,
    HidDeviceServiceImpl::OnSetReport,
    HidDeviceServiceImpl::OnSetProtocol,
    HidDeviceServiceImpl::OnInterruptDataReceived,
    HidDeviceServiceImpl::OnVirtualCableUnplug
};

sptr<AppExecFwk::IAppMgr> HidAppStateObserver::GetAppMgrProxy()
 {
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
 
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        HILOGE("systemAbilityManager remote object is nullptr");
        return nullptr;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if (!appMgrProxy || !appMgrProxy->AsObject()) {
        HILOGE("get appMgrProxy failed!");
        return nullptr;
    }

    return appMgrProxy;
}

void HidAppStateObserver::setBundleName(std::string bundleName)
{
    if (hidAppStateAwareObserver_ != nullptr) {
        hidAppStateAwareObserver_->setBundleName(bundleName);
    }
}

std::string HidAppStateObserver::getBundleName()
{
    if(hidAppStateAwareObserver_ != nullptr) {
        return hidAppStateAwareObserver_->getBundleName();
    }
    return "";
}

bool HidAppStateObserver::SubscribeHidAppState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (hidAppStateAwareObserver_) {
        HILOGE("SubscribeHidAppState: hidAppStateAwareObserver_ has register");
        return false;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = GetAppMgrProxy();
    if (appMgrProxy == nullptr) {
        HILOGE("SubscribeAppState: appMgrProxy is nullptr");
        return false;
    }
    hidAppStateAwareObserver_ = new (std::nothrow)HidAppStateAwareObserver();
    if (hidAppStateAwareObserver_ == nullptr) {
        HILOGE("SubscribeAppState: hidAppStateAwareObserver_ is nullptr");
        return false;
    }
    auto err = appMgrProxy->RegisterApplicationStateObserver(hidAppStateAwareObserver_);
    if (err != 0) {
        HILOGE("SubscribeAppState error, code = %{public}d", err);
        hidAppStateAwareObserver_ = nullptr;
        return false;
    }
    return true;
}

bool HidAppStateObserver::UnSubscribeHidAppState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!hidAppStateAwareObserver_) {
        HILOGE("UnSubscribeHidAppState: HidAppStateObserver is nullptr");
        return false;
    }

    sptr<AppExecFwk::IAppMgr> appMgrProxy = GetAppMgrProxy();
    if (appMgrProxy) {
        appMgrProxy->UnregisterApplicationStateObserver(hidAppStateAwareObserver_);
        appMgrProxy = nullptr;
        hidAppStateAwareObserver_ = nullptr;
    }
    return true;
}

void HidAppStateObserver::HidAppStateAwareObserver::OnForegroundApplicationChanged(
    const AppExecFwk::AppStateData &appStateData)
{
    if (!ValidateAppStateData(appStateData)) {
        HILOGI("OnForegroundApplicationChanged, validate app state data failed");
        return;
    }
    if (appStateData.bundleName == bundleName_ &&
        appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND)) {
        HILOGI("App state background bundleName = %{public}s  uid = %{public}d",
            appStateData.bundleName.c_str(), appStateData.uid);
        HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
        if (hidDeviceServiceImpl == nullptr) {
            HILOGE("hidDeviceService is nullptr");
            return;
        }
        if (!hidDeviceServiceImpl->GetAppState()) {
            return;
        }
        hidDeviceServiceImpl->SetAppState(false);
        hidDeviceServiceImpl->UnregisterHidDevice();
    }
    else if (appStateData.bundleName == bundleName_ &&
        appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND)) {
        HILOGI("App state foreground bundleName = %{public}s  uid = %{public}d",
            appStateData.bundleName.c_str(), appStateData.uid);
        HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
        if (hidDeviceServiceImpl == nullptr) {
            HILOGE("hidDeviceService is nullptr");
            return;
        }
        hidDeviceServiceImpl->SetAppState(true);
    }
}

inline bool HidAppStateObserver::HidAppStateAwareObserver::ValidateAppStateData(
    const AppExecFwk::AppStateData &appStateData)
{
    return appStateData.uid > 0 && appStateData.bundleName.length() > 0;
}

void HidAppStateObserver::HidAppStateAwareObserver::setBundleName(std::string bundleName)
{
    bundleName_ = bundleName;
}

std::string HidAppStateObserver::HidAppStateAwareObserver::getBundleName()
{
    return bundleName_;
}

std::vector<OHOS::AppExecFwk::AppStateData> HidDeviceServiceImpl::GetForegroundApplications()
{
    std::vector<OHOS::AppExecFwk::AppStateData> apps;
    if (hidAppStateObserver_ == nullptr) {
        HILOGE("get appStateObserverImp_ failed!");
        return apps;
    }
    sptr<AppExecFwk::IAppMgr> appMgrProxy = hidAppStateObserver_->GetAppMgrProxy();
    if (appMgrProxy == nullptr) {
        HILOGE("get appMgrProxy failed!");
        return apps;
    }
    appMgrProxy->GetForegroundApplications(apps);
    return apps;
}

HidDeviceServiceImpl::HidDeviceServiceImpl()
{
    hidAppStateObserver_ = std::make_unique<HidAppStateObserver>();
    if (hidAppStateObserver_ != nullptr) {
        hidAppStateObserver_->SubscribeHidAppState();
    }
    BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        return;
    }
    bluetoothHidDeviceInterface = reinterpret_cast<BthdInterface*>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_PROFILE_HIDDEV_ID)));
    if (bluetoothHidDeviceInterface == nullptr) {
        return;
    }
    BtStackStatus status = bluetoothHidDeviceInterface->init(&sBluetoothHidDeviceCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGI("[HID_SERVICE]Failed to initialize Bluetooth HID Device, status: %{public}d", status);
        return;
    }
}

HidDeviceServiceImpl::~HidDeviceServiceImpl()
{
    if (bluetoothHidDeviceInterface != nullptr) {
        bluetoothHidDeviceInterface->cleanup();
        bluetoothHidDeviceInterface = nullptr;
    }
    HILOGI("HidDeviceService shutdown");
}


extern "C" HidDeviceServiceImplInterface *CreateHidDeviceServiceImplInterface(void)
{
    if (g_serviceImpl == nullptr) {
        g_serviceImpl = new HidDeviceServiceImpl();
    }
    return HidDeviceServiceImpl::GetServiceImpl();
}

extern "C" void DestroyHidDeviceServiceImplInterface(HidDeviceServiceImplInterface *interface)
{
    if (interface == nullptr) {
        HILOGE("Hid Device manager Interface is nullptr");
        return;
    }
    delete interface;
    interface = nullptr;
}

HidDeviceServiceImpl *HidDeviceServiceImpl::GetServiceImpl()
{
    return g_serviceImpl; 
}

void HidDeviceServiceImpl::RegisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver)
{
    hidDeviceObserver_ = hidDeviceObserver;
}

void HidDeviceServiceImpl::DeregisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver)
{
    hidDeviceObserver_ = nullptr;
    hidAppStateObserver_ = std::make_unique<HidAppStateObserver>();
    if (hidAppStateObserver_ != nullptr) {
        hidAppStateObserver_->UnSubscribeHidAppState();
    }
}

int HidDeviceServiceImpl::Connect(const RawAddress &device)
{
    OHOS::bluetooth::RawAddress rawAddr(device);
    if (bluetoothHidDeviceInterface == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    HILOGI("connect device: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    return bluetoothHidDeviceInterface->connect(&rawAddr);
}

int HidDeviceServiceImpl::Disconnect(const RawAddress &device)
{
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return bluetoothHidDeviceInterface->disconnect();
}

int HidDeviceServiceImpl::RegisterHidDevice(BluetoothHidDeviceSdp sdp,  BluetoothHidDeviceQos inQos,
    BluetoothHidDeviceQos outQos, std::string bundleName)
{
    if (!IsAppForeground(bundleName)) {
        HILOGI("hid device is not in the forground");
        return BT_ERR_HID_APPLICATION_NOT_IN_FOREGROUND;
    }
    SetAppState(true);
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    HILOGI("RegisterHidDevice bundleName = %{public}s", bundleName.c_str());
    hidAppStateObserver_->setBundleName(bundleName);

    DoInHidDeviceThread([this, sdp, inQos, outQos] {this->ProcessRegisterHIdDeviceEvent(sdp, inQos, outQos);});
    return BT_NO_ERROR;
}

int HidDeviceServiceImpl::UnregisterHidDevice()
{
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    BtStackStatus ret = bluetoothHidDeviceInterface->unregisterApp();
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Failed to UnregisterHidDevice, status: %{public}d", ret);
    }
    return BT_NO_ERROR;
}

int32_t HidDeviceServiceImpl::SendReport(int id, const std::vector<uint8_t> &data)
{
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    uint16_t length = static_cast<uint16_t>(data.size());
    uint8_t *pData = const_cast<uint8_t*>(data.data());
    BtStackStatus ret = bluetoothHidDeviceInterface->sendReport(BTHD_REPORT_TYPE_INTRDATA, id, length, pData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Failed to SendReport, status: %{public}d", ret);
    }
    return BT_NO_ERROR;
}

int32_t HidDeviceServiceImpl::ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data)
{
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    uint16_t length = static_cast<uint16_t>(data.size());
    uint8_t *pData = const_cast<uint8_t*>(data.data());
    BtStackStatus ret = bluetoothHidDeviceInterface->sendReport(
        static_cast<BthdReportType>(type), id, length, pData);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Failed to ReplyReport, status: %{public}d", ret);
    }
    return BT_NO_ERROR;
}

int32_t HidDeviceServiceImpl::ReportError(uint8_t type)
{
    if (bluetoothHidDeviceInterface == nullptr) {
        HILOGE("HidDeviceServiceImpl: bluetoothHidInterface is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    BtStackStatus ret = bluetoothHidDeviceInterface->reportError(type);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Failed to ReportError, status: %{public}d", ret);
    }
    return BT_NO_ERROR;
}

bool HidDeviceServiceImpl::GetAppState()
{
    return IsAppForeground_.load();
}

bool HidDeviceServiceImpl::IsAppForeground(std::string bundleName)
{
    std::vector<OHOS::AppExecFwk::AppStateData> foregroundApps = GetForegroundApplications();
    for(const auto &foregroundApp : foregroundApps) {
        if (foregroundApp.bundleName == bundleName) {
            return true;
        }
    }
    return false;
}

void HidDeviceServiceImpl::SetAppState(bool isForeground)
{
    IsAppForeground_.store(isForeground);
}

int HidDeviceServiceImpl::ConvertAppStatusChangedFromStack(BthdApplicationState state)
{
    if (state == BTHD_APP_STATE_NOT_REGISTERED) {
        return HID_DEVICE_APP_STATE_NOT_REGISTERED;
    }
    if (state == BTHD_APP_STATE_REGISTERED) {
        return HID_DEVICE_APP_STATE_REGISTERED;
    }
    return HID_DEVICE_APP_STATE_NOT_REGISTERED;
}

int HidDeviceServiceImpl::ConvertConnectStateFromStack(BthdConnectionState state)
{
    if (state == BTHD_CONN_STATE_CONNECTING) {
        return static_cast<int>(BTConnectState::CONNECTING);
    }
    if (state == BTHD_CONN_STATE_DISCONNECTING) {
        return static_cast<int>(BTConnectState::DISCONNECTING);
    }
    if (state == BTHD_CONN_STATE_CONNECTED) {
        return static_cast<int>(BTConnectState::CONNECTED);
    }
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

void HidDeviceServiceImpl::OnAppStatusChangedInner(int state)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnAppStatusChanged(state);
    }
}

void HidDeviceServiceImpl::OnConnectionStateChangedInner(bluetooth::RawAddress rawAddr, int state)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnConnectionStateChanged(rawAddr, state);
    }
}

void HidDeviceServiceImpl::OnGetReportInner(int type, int id, uint16_t buffer_size)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnGetReport(type, id, buffer_size);
    }
}

void HidDeviceServiceImpl::OnInterruptDataInner(int report_id, std::vector<uint8_t> data)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnInterruptDataReceived(report_id, data);
    }
}

void HidDeviceServiceImpl::OnSetProtocolInner(int protocol)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnSetProtocol(protocol);
    }
}

void HidDeviceServiceImpl::OnSetReportInner(int type, int id, std::vector<uint8_t> data)
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnSetReport(type, id, data);
    }
}

void HidDeviceServiceImpl::OnVirtualCableUnplugInner()
{
    if (hidDeviceObserver_ != nullptr) {
        hidDeviceObserver_->OnVirtualCableUnplug();
    }
}

void HidDeviceServiceImpl::OnAppStatusChanged(OHOS::bluetooth::RawAddress* bdAddr, BthdApplicationState state)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    if (bdAddr == nullptr) {
        HILOGI("bdAddr is nullptr");
    }
    int connectState = static_cast<int>(state);
    hidDeviceServiceImpl->OnAppStatusChangedInner(connectState);
}

void HidDeviceServiceImpl::OnConnectionStateChanged(OHOS::bluetooth::RawAddress* bdAddr, BthdConnectionState state)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    HILOGI("[HID_SERVICE]HidConnectState = %{public}d", state);
    if (bdAddr == nullptr) {
        HILOGE("bdAddr == nullptr");
        return;
    }
    RawAddress rawAddr = *bdAddr;
    int connectState = HidDeviceServiceImpl::ConvertConnectStateFromStack(state);
    HILOGI("OnConnectionStateChanged state = %{public}d", connectState);
    hidDeviceServiceImpl->OnConnectionStateChangedInner(rawAddr, connectState);
}

void HidDeviceServiceImpl::OnGetReport(uint8_t type, uint8_t id, uint16_t buffer_size)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    hidDeviceServiceImpl->OnGetReportInner(type, id, buffer_size);
}

void HidDeviceServiceImpl::OnInterruptDataReceived(uint8_t report_id, uint16_t len, uint8_t* p_data)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    std::vector<uint8_t> data(p_data, p_data + len);
    hidDeviceServiceImpl->OnInterruptDataInner(report_id, data);
}

void HidDeviceServiceImpl::OnSetProtocol(uint8_t protocol)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    hidDeviceServiceImpl->OnSetProtocolInner(protocol);
}

void HidDeviceServiceImpl::OnSetReport(uint8_t type, uint8_t id, uint16_t len, uint8_t* p_data)
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    std::vector<uint8_t> data(p_data, p_data + len);
    hidDeviceServiceImpl->OnSetReportInner(type, id, data);
}

void HidDeviceServiceImpl::OnVirtualCableUnplug()
{
    HidDeviceServiceImpl *hidDeviceServiceImpl = HidDeviceServiceImpl::GetServiceImpl();
    if (hidDeviceServiceImpl == nullptr) {
        HILOGE("hidDeviceService is null");
        return;
    }
    hidDeviceServiceImpl->OnVirtualCableUnplugInner();
}

void HidDeviceServiceImpl::ProcessRegisterHIdDeviceEvent(BluetoothHidDeviceSdp sdp,
    BluetoothHidDeviceQos inQos, BluetoothHidDeviceQos outQos)
{
    BthdAppParam appParam;
    BthdQosParam inQosParam;
    BthdQosParam outQosParam;
    appParam.name = sdp.name_.c_str();
    appParam.description = sdp.description_.c_str();
    appParam.provider = sdp.provider_.c_str();
    appParam.subclass = static_cast<uint8_t>(sdp.subclass_);
    appParam.descList = sdp.sdpSettings_.data();
    appParam.descListLen = static_cast<int>(sdp.sdpSettings_.size());

    inQosParam.serviceType = static_cast<uint8_t>(inQos.serviceType_);
    inQosParam.tokenRate = static_cast<uint32_t>(inQos.tokenRate_);
    inQosParam.accessLatency = static_cast<uint32_t>(inQos.latency_);
    inQosParam.delayVariation = static_cast<uint32_t>(inQos.delayVariation_);
    inQosParam.peakBandwidth = static_cast<uint32_t>(inQos.peakBandwidth_);
    inQosParam.tokenBucketSize = static_cast<uint32_t>(inQos.tokenBucketSize_);

    outQosParam.serviceType = static_cast<uint8_t>(outQos.serviceType_);
    outQosParam.tokenRate = static_cast<uint32_t>(outQos.tokenRate_);
    outQosParam.accessLatency = static_cast<uint32_t>(outQos.latency_);
    outQosParam.delayVariation = static_cast<uint32_t>(outQos.delayVariation_);
    outQosParam.peakBandwidth = static_cast<uint32_t>(outQos.peakBandwidth_);
    outQosParam.tokenBucketSize = static_cast<uint32_t>(outQos.tokenBucketSize_);
    BtStackStatus ret = bluetoothHidDeviceInterface->registerApp(&appParam, &inQosParam, &outQosParam);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Failed to initialize Bluetooth hd register, status: %{public}d", ret);
    } else {
        HILOGI("register hid devcice success");
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
