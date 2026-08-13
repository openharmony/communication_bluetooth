/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_map_mse_service"
#endif

#include "map_mse_service.h"

#include "bt_def.h"
#include "class_creator.h"
#include "parameter.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "interface_profile_socket.h"
#include "interface_profile_manager.h"
#include "log.h"
#include "common_util.h"
#include "adapter_config.h"
#include "connect_strategy_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_dialog.h"
#include "thread_util.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const int32_t MAS_ID_SMS_MMS = 0;
const uint32_t REQUEST_PERMISSION_TIMEOUT_MS = 30000;  // 30s
const int32_t SDP_MAP_MSG_TYPE_SMS_GSM = 0x02;
const int32_t SDP_MAP_MSG_TYPE_SMS_CDMA = 0x04;
const int32_t SDP_MAP_MSG_TYPE_MMS = 0x08;
const int32_t MAP_RFCOMM_CHANNEL = 26;
const int32_t MAP_L2CAP_PSM = 0x1029;
const int32_t SDP_MAP_MAS_VERSION_1_2 = 0x0102;
const int32_t SDP_MAP_MAS_FEATURES_1_2 = 0x0000007F;
const int32_t SDP_MAP_MAS_VERSION_1_4 = 0x0104;
const int32_t SDP_MAP_MAS_FEATURES_1_4 = 0x000603FF;
const std::string MAP_MSE_SERVICE_NAME = "OBEX Message Access";
const std::string MNS_UUID = "00001133-0000-1000-8000-00805F9B34FB";
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
constexpr const char* PERSIST_SMS_CALLLOG_ENABLE = "persist.bluetooth.sync_sms_calllog_enable";

MapMseService::MapMseService() : utility::Context(PROFILE_NAME_MAP_MSE, "1.2"),
    loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{
}

MapMseService::~MapMseService()
{
}

void MapMseService::LoadMapMseServiceImplInterfaceLib(void)
{
    loader_.OpenLib();

    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceImplIntf, "Get map mse service impl interface failed");

    mapMseConnectionObserver_ = std::make_shared<MapMseConnectionObserver>();
    serviceImplIntf->RegisterObserver(mapMseConnectionObserver_);
    auto connectSocketFunc = [this](const std::string &addr, const Uuid &uuid, int securityFlag,
        int type, int psm) -> int {
        return this->ConnectSocket(addr, uuid, securityFlag, type, psm);
    };
    serviceImplIntf->DllRegisterConnectSocketFunc(connectSocketFunc);
    auto getDbIdentifierfunc = [this]() -> int {
        return this->GetDbIdentifier();
    };
    serviceImplIntf->DllRegisterGetDbIndetifierFunc(getDbIdentifierfunc);
    auto setIsNeedReconnClient = [this](const bool isNeedReconnClient) -> void {
        return this->SetIsNeedReconnClient(isNeedReconnClient);
    };
    serviceImplIntf->DllSetIsNeedReconnClientFunc(setIsNeedReconnClient);
}

void MapMseService::UnloadMapMseServiceImplInterfaceLib(void)
{
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceImplIntf, "Get map mse service impl interface failed");
    serviceImplIntf->DeregisterObserver(mapMseConnectionObserver_);
}

utility::Context *MapMseService::GetContext()
{
    return this;
}

MapMseService *MapMseService::GetService()
{
    auto servManager = IProfileManager::GetInstance();
    if (servManager == nullptr) {
        HILOGI("[MapMseService Service] servManager is nullptr.");
        return nullptr;
    }
    return static_cast<MapMseService *>(servManager->GetProfileService(PROFILE_NAME_MAP_MSE));
}

void MapMseService::Enable()
{
    DoInMapThread([this]() { this->EnableNative(); });
}

void MapMseService::Disable()
{
    DoInMapThread([this]() { this->DisableNative(); });
}

void MapMseService::EnableNative()
{
    StartSocketListener(MAS_ID_SMS_MMS);

    requestPermissionTimer_ =
        std::make_shared<utility::Timer>([this]() { this->RequestPermissionTimeout(); });

    GetContext()->OnEnable(PROFILE_NAME_MAP_MSE, true);
}

void MapMseService::DisableNative()
{
    StopSocketListener();
    masIdToSdpHandle_.clear();
    GetContext()->OnDisable(PROFILE_NAME_MAP_MSE, true);
}

void MapMseService::StartSocketListener(int masId)
{
    if (obexServerSocket_ != nullptr) {
        HILOGI("obexServerSocket_ is alread create.");
        return;
    }
    socketObserver_ = std::make_shared<MapMseSocketObserver>();
    obexServerSocket_ = std::make_shared<ObexServerSocket>(
        MAP_MSE_SERVICE_NAME, MAP_RFCOMM_CHANNEL, MAP_L2CAP_PSM, EMPTY_UUID, socketObserver_);
    int securityFlags = 3; // SOCK_FLAG_AUTHENTICATION | SOCK_FLAG_ENCRYPTION;
    obexServerSocket_->Startup(securityFlags);

    int32_t msgTypes = SDP_MAP_MSG_TYPE_SMS_GSM | SDP_MAP_MSG_TYPE_SMS_CDMA | SDP_MAP_MSG_TYPE_MMS;
    MapMasSdpCreateParam param{"SMS/MMS",
        masId,
        MAP_RFCOMM_CHANNEL,
        MAP_L2CAP_PSM,
        SDP_MAP_MAS_VERSION_1_4,
        msgTypes,
        SDP_MAP_MAS_FEATURES_1_4};
    int sdpHandle = SdpAdapter::GetInstance().CreateMapMasSdpRecord(param);
    if (sdpHandle == -1) {
        HILOGE("CreateMapMasSdpRecord error retry.");
        sdpHandle = SdpAdapter::GetInstance().CreateMapMasSdpRecord(param);
    }
    masIdToSdpHandle_.insert({masId, sdpHandle});
    UpdateDbIdentifier();
}

void MapMseService::UpdateDbIdentifier()
{
    time_t time_seconds = time(nullptr);
    struct tm now_time;
    localtime_r(&time_seconds, &now_time);
    dbIndetifier_ = time_seconds;
}

int MapMseService::GetDbIdentifier()
{
    return dbIndetifier_;
}

void MapMseService::StopSocketListener()
{
    HILOGI("Close.");
    if (obexServerSocket_ != nullptr) {
        obexServerSocket_->Shutdown();
        obexServerSocket_ = nullptr;
    }
    for (auto iter = masIdToSdpHandle_.begin(); iter != masIdToSdpHandle_.end(); iter++) {
        SdpAdapter::GetInstance().RemoveSdpRecord(iter->second);
    }
}

int MapMseService::Connect(const RawAddress &device)
{
    HILOGI("device %{public}s.", GET_ENCRYPT_ADDR(device));
    return RET_NO_ERROR;
}

int MapMseService::Disconnect(const RawAddress &device)
{
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceImplIntf, BT_ERR_INTERNAL_ERROR, "Get map mse service impl interface failed");
    serviceImplIntf->Disconnect(device);

    return RET_NO_ERROR;
}

std::list<RawAddress> MapMseService::GetConnectDevices()
{
    std::list<RawAddress> devices;
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceImplIntf, devices, "Get map mse service impl interface failed");

    return serviceImplIntf->GetConnectDevices();
}

int MapMseService::GetConnectState()
{
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceImplIntf, PROFILE_STATE_DISCONNECTED, "Get map mse service impl interface failed");

    return serviceImplIntf->GetConnectState();
}

int MapMseService::GetMaxConnectNum()
{
    return 1;
}

std::vector<RawAddress> MapMseService::GetDevicesByStates(const std::vector<int> &states)
{
    return std::vector<RawAddress>{};
}

int MapMseService::GetDeviceState(const RawAddress &device)
{
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG_RET(serviceImplIntf, static_cast<int32_t>(BTConnectState::DISCONNECTED),
        "Get map mse service impl interface failed");

    return serviceImplIntf->GetDeviceState(device);
}

int MapMseService::SetConnectionStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res =
        ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device, PROPERTY_MAP_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if (GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED)) {
            Disconnect(device);
            HILOGI("disconnect device.");
        }
    }
    return BT_NO_ERROR;
}

int MapMseService::GetConnectionStrategy(const RawAddress &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device, PROPERTY_MAP_CONNECTION_POLICY);
}

int32_t MapMseService::SetMessageAccessAuthorization(const RawAddress &device, int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d", GET_ENCRYPT_ADDR(device), accessAuthorization);
    BluetoothDialog::DismissCurAndShowNext();
    bool result = SetMessagePermission(device.GetAddress(), accessAuthorization);
    if (!result) {
        HILOGE("SetMessageAccessAuthorization fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    const char* isHasPermission = HasPbapOrMapPermission() ? "1" : "0";
    SetParameter(PERSIST_SMS_CALLLOG_ENABLE, isHasPermission);
    StopRequestPermissionTimer();

    if (socketDevice_ != nullptr && socketDevice_->mDeviceAddress == device.GetAddress()) {
        CheckOrGetPermission(device.GetAddress());
    }

    return BT_NO_ERROR;
}

int32_t MapMseService::GetMessageAccessAuthorization(const RawAddress &device, int32_t &accessAuthorization)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    bool result = GetMessagePermission(device.GetAddress(), accessAuthorization);
    if (!result) {
        HILOGE("GetMessageAccessAuthorization fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

void MapMseService::RegisterObserver(IMapMseObserver &observer)
{
}

void MapMseService::DeregisterObserver(IMapMseObserver &observer)
{
}

void MapMseService::OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice, int32_t masInstanceId)
{
    DoInMapThread([this, socketDevice, masInstanceId]() { this->DealOnConnect(socketDevice, masInstanceId); });
}

void MapMseService::ReStartWhenSocketServerException(int32_t masInstanceId)
{
    DoInMapThread([this]() { this->StopSocketListener(); });
    DoInMapThread([this, masInstanceId]() { this->StartSocketListener(masInstanceId); });
}

void MapMseService::SetIsNeedReconnClient(const bool isNeedReconnClient)
{
    HILOGI("isNeedReconnClient is %{public}d", isNeedReconnClient);
    SdpAdapter::GetInstance().SetMapMnsSdpFoundCallback([this](auto param) { this->SdpFoundCallback(param); });
    SdpAdapter::GetInstance().StartRemoteSdpSearch(socketDevice_->mDeviceAddress, MNS_UUID);
    isNeedReconnClient_ = isNeedReconnClient;
}

void MapMseService::SdpFoundCallback(const MapMnsSdpFoundRecord &record)
{
    if (record.status != -1) {
        // set remote feature
    }
    
    if (sdpSearchDone_) {
        LoadMapMseServiceImplInterfaceLib();
    }
    auto serviceImplIntf = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(serviceImplIntf, "Get map mse service impl interface failed");
    if (sdpSearchDone_) {
        serviceImplIntf->StartUp(record, socketDevice_, false);
    }
    if (!sdpSearchDone_ && isNeedReconnClient_) {
        HILOGI("isNeedReconnClient_ %{public}d", isNeedReconnClient_);
        serviceImplIntf->StartUp(record, socketDevice_, isNeedReconnClient_);
        isNeedReconnClient_ = false;
    }
    sdpSearchDone_ = false;
}

void MapMseService::Clear()
{
    socketDevice_ = nullptr;
}

void MapMseService::DealOnConnect(const std::shared_ptr<ObexSocketDevice> &socketDevice, int32_t masInstanceId)
{
    if (!masIdToSdpHandle_.count(masInstanceId)) {
        return;
    }
    if (socketDevice_ != nullptr) {
        HILOGW("socketDevice_ is not null");
        if (socketDevice_->mDeviceAddress != socketDevice->mDeviceAddress) {
            HILOGE("only allow connect one device");
            return;
        }
    } else {
        socketDevice_ = socketDevice;
        CheckOrGetPermission(socketDevice_->mDeviceAddress);
    }
}

void MapMseService::CheckOrGetPermission(const std::string &address)
{
    HILOGI("address: %{public}s", GetEncryptAddr(address).c_str());
    int permission = 0;
    GetMessagePermission(address, permission);
    if (permission == static_cast<int>(BTPermissionType::ACCESS_ALLOWED)) {
        SdpAdapter::GetInstance().SetMapMnsSdpFoundCallback([this](auto param) { this->SdpFoundCallback(param); });
        SdpAdapter::GetInstance().StartRemoteSdpSearch(socketDevice_->mDeviceAddress, MNS_UUID);
        sdpSearchDone_ = true;
    } else if (permission == static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN)) {
        RejectConnection();
    } else if (permission == static_cast<int>(BTPermissionType::ACCESS_UNKNOWN)) {
        DialogInfo dialog{address, MAP_AUTH_DIALOG, REQUEST_PERMISSION_TIMEOUT_MS, requestPermissionTimer_};
        BluetoothDialog::RequestAuthDialog(dialog);
    } else {
        HILOGE("Get Permission is error, device: %{public}s", GET_ENCRYPT_STR_ADDR(address));
    }
}

void MapMseService::RejectConnection()
{
    auto obexTransport = std::make_shared<ObexServerTransport>(socketDevice_);
    auto obexRejectServer = std::make_shared<ObexRejectServer>();
    std::lock_guard<std::mutex> lock(obexRejectServerSessionMutex_);
    if (obexRejectServerSession_ != nullptr) {
        HILOGE("obexRejectServerSession_ is nullptr");
        return;
    }
    obexRejectServerSession_ = std::make_shared<ObexServerSession>(obexTransport, obexRejectServer);
    obexRejectServerSession_->Start();

    auto taskFunc = [this]() {
        std::lock_guard<std::mutex> lock(obexRejectServerSessionMutex_);
        if (obexRejectServerSession_ != nullptr) {
            obexRejectServerSession_->Stop();
        }
        obexRejectServerSession_ = nullptr;
        socketDevice_ = nullptr;
    };
    ThreadUtil::GetInstance().PostTask(THREAD_ID_MAP, taskFunc, DISCONNECT_DELAY_TIME_MS, "RejectConnection");
}

void MapMseService::RequestPermissionTimeout()
{
    RejectConnection();
    // dismiss map dialog
    BluetoothDialog::DismissCurAndShowNext();
}

void MapMseService::StopRequestPermissionTimer()
{
    HILOGI("Stop RequestPermission timer!");
    requestPermissionTimer_->Stop();
}

int MapMseService::ConnectSocket(const std::string &addr, const Uuid &uuid, int securityFlag, int type, int psm)
{
    IProfileSocket *socketService =
        (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
    CHECK_AND_RETURN_LOG_RET(socketService, -1, "socketService is null");
    return socketService->Connect(addr, uuid, securityFlag, type, psm);
}

void MapMseSocketObserver::OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    MapMseService *mapService = MapMseService::GetService();
    CHECK_AND_RETURN_LOG(mapService != nullptr, "mapService is null");
    if (socketDevice == nullptr) {
        HILOGE("socket server error, restart socket listener");
        mapService->ReStartWhenSocketServerException(MAS_ID_SMS_MMS);
        return;
    }
    HILOGI("MapMseSocketObserver OnConnect, device : %{public}s, fd : %{public}d",
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress), socketDevice->mSocketFd);
    mapService->OnConnect(socketDevice, MAS_ID_SMS_MMS);
}

void MapMseConnectionObserver::OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause)
{
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_MAP_MSE, false);
    }
    cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device, PROFILE_ID_MAP_MSE, state);
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        MapMseService *mapService = MapMseService::GetService();
        CHECK_AND_RETURN_LOG(mapService != nullptr, "mapService is null");
        mapService->Clear();
        DoInMapThread([]() {
            auto service = MapMseService::GetService();
            if (service != nullptr) {
                service->UnloadMapMseServiceImplInterfaceLib();
            }
        });
    }
}

REGISTER_CLASS_CREATOR(MapMseService);
}  // namespace bluetooth
}  // namespace OHOS