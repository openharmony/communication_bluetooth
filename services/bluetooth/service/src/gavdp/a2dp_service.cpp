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
#define LOG_TAG "bt_service_a2dp"
#endif

#include "a2dp_service.h"

#include <cstring>
#include <dlfcn.h>
#include "log.h"
#include "common_util.h"
#include "adapter_config.h"
#include "adapter_manager.h"
#include "avrc_defs.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_audio_manager_interface.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_dialog.h"
#include "connect_strategy_manager.h"
#include "securec.h"
#include "permission_manager.h"
#include "profile_config.h"
#include "ipc_skeleton.h"
#include "profile_service_manager.h"
#include "idevmgr_hdi.h"
#include "hdf_remote_service.h"
#include "servmgr_hdi.h"
#include "../avrcp_tg/avrcp_tg_service_manager.h"
#include "common/bluetooth_hw_interface.h"
#include "bt_chr_business_event.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_dft_statictics.h"
#include "bt_chr_ue_manager.h"
#include "thread_util.h"
#include "hdf_device_class.h"
#include "hfp_ag_system_interface.h"

#ifdef BT_USE_OPEN_STACK
extern "C" bt_status_t btif_av_write_frame(const uint8_t *data, uint32_t size, uint32_t timeStamp);
#endif
#include "hfp_ag_service.h"
#include "hitrace_meter.h"
#include "refuse_play_helper.h"
#include "hdf_dynamic_library_loader.h"
#include "bluetooth_device_battery_manager.h"
#include "hw_interop.h"
#include "bluetooth_errorcode.h"
#include "control_intercept_plugin.h"
#include "bluetooth_audio_framework_adapter.h"
#include "bt_recursive_mutex.h"
#include "log_utils.h"
#include "a2dp_service_codec.h"
#include "cloud_device_manager.h"
#include "bluetooth_common_event_helper.h"
#include "bt_func_hook.h"
#include "a2dp_hfp_recover.h"
#include "bt_chr_transaction_manager.h"
#include "battery_srv_client.h"

constexpr const char *AUDIO_BLUETOOTH_SERVICE_NAME = "audio_bluetooth_hdi_service";
constexpr const char *AUDIO_HDI_SERVICE_NAME = "bluetooth_audio_session_service";
constexpr const uint16_t OFFLOAD_DELAY_STOP_INTERVAL_MS = 3000;
constexpr const uint8_t HW_A2DP_OFFLOAD_SOFTWARE_ENCODING = 1;
constexpr const uint8_t HW_A2DP_OFFLOAD_HARDWARE_ENCODING = 2;
constexpr const uint16_t AUDIO_MUTE_TIMEOUT_MS = 1000;
constexpr const uint16_t HDF_RELOAD_DELAY_MS = 300;
constexpr const uint16_t TIMER_FOR_RESTORE_VOLUME_IF_PAUSED_MS = 1000;
constexpr const int HDI_SERVICE_NOT_LOADED = -2;

namespace OHOS {
namespace bluetooth {
const uint64_t DELAY_A2DP_REACH_MAX_CONNECT = 1000;
using namespace OHOS::Bluetooth;

BtRecursiveMutex g_a2dpServiceMutex {};
HdfDynamicLibraryLoader& g_loader = HdfDynamicLibraryLoader::GetInstance();
struct ServiceStatusListener *g_listener = nullptr;
utility::SafeVector<RawAddress> g_addressQueue;

static void UpdateStateInformation(utility::Message &msg, int &state,
    const int msgCMD, const int stateValue)
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    HILOGI("msgCMD(%{public}d) stateValue(%{public}d)\n", msgCMD, stateValue);
    msg.what_ = msgCMD;
    state = stateValue;
}

static void UpdateVirtualDeviceByA2dpConnectState(const RawAddress& btAddr, int connectState) {
    int hwRemoteDeviceType = RemoteDeviceProperties::GetInstance()->GetHwRemoteDeviceType(btAddr);
    if (hwRemoteDeviceType != HW_CAR_DEVICE_TYPE) {
        return;
    }
    HfpAgService *hfpService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(hfpService != nullptr, "hfp ag service nullptr");

    int hfpState = static_cast<int>(HfpAgConnectState::HFP_AG_STATE_INVAILD);
    bool isFind = hfpService->GetRemoteDeviceConnectStatus(btAddr, hfpState);
    CHECK_AND_RETURN_LOG(isFind, "hfp ag service not find this device");

    if (connectState == static_cast<int>(BTConnectState::DISCONNECTED) &&
        hfpState == static_cast<int>(HfpAgConnectState::HFP_AG_STATE_CONNECTED)) {
        A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(service != nullptr, "a2dp source service nullptr");
        HILOGI("when a2dp disconnect hfp connect add virtual device");
        service->UpdateVirtualDevice(VIRTUAL_DEVICE_ADD, btAddr.GetAddress());
    }
}

static int ProcessConnectStateMessage(RawAddress btAddr, std::shared_ptr<A2dpDeviceInfo> deviceInfo,
    const int connectPolicy, const int state, const uint8_t role)
{
    HILOG_COMM_INFO("ProcessConnectStateMessage: enter");
    int connectState = RET_BAD_STATUS;
    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, role, &(btAddr));
    switch (state) {
        case BTAV_CONNECTION_STATE_CONNECTED:
            if (connectPolicy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
                UpdateStateInformation(
                    msg, connectState, A2DP_MSG_CONNECT_FORBIDDEN, static_cast<int>(BTConnectState::CONNECTED));
            } else {
                UpdateStateInformation(
                    msg, connectState, A2DP_MSG_PROFILE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED));
            }
            break;
        case BTAV_CONNECTION_STATE_CONNECTING:
            if (connectPolicy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
                UpdateStateInformation(
                    msg, connectState, A2DP_MSG_CONNECT_FORBIDDEN, static_cast<int>(BTConnectState::CONNECTING));
            } else {
                UpdateStateInformation(
                    msg, connectState, A2DP_MSG_PROFILE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING));
            }
            break;
        case BTAV_CONNECTION_STATE_DISCONNECTED:
            UpdateStateInformation(
                msg, connectState, A2DP_MSG_PROFILE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED));
            break;
        case BTAV_CONNECTION_STATE_DISCONNECTING:
            UpdateStateInformation(
                msg, connectState, A2DP_MSG_PROFILE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING));
            break;
        default:
            break;
    }
    if (connectState != RET_BAD_STATUS) {
        deviceInfo->SetConnectState(connectState);
        UpdateVirtualDeviceByA2dpConnectState(btAddr, connectState);
        if (connectState == static_cast<int>(BTConnectState::DISCONNECTED) && deviceInfo->GetPlayingState()) {
            deviceInfo->SetPlayingState(false);
            std::string callingName = PermissionManager::GetCallingName();
            BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(btAddr, false,
                deviceInfo->GetCodecStatus().codecInfo.codecType, callingName);
        }
        deviceInfo->GetStateMachine()->ProcessMessage(msg);
    }
    return connectState;
}

static void ProcessConnection(A2dpService* service, int state, const RawAddress &connectedDevice)
{
    service->ProcessConnectFrameworkCallback(state, connectedDevice);
    service->NotifyProfileState(connectedDevice);
    service->NotifyWearDetectionActionAfterConnected(connectedDevice);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_A2DP_CONN_CMP, connectedDevice, 0, 0);
    ConnectStrategyManager::GetInstance()->ProfileConnectionStateChange(connectedDevice.GetAddress(), "a2dpService");
}

static bool IsA2dpHostLoaded()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr, false, "HDIServiceManagerGet failed!");
    HdfRemoteService *hdiService = serviceMgr->GetService(serviceMgr, AUDIO_BLUETOOTH_SERVICE_NAME);
    if (hdiService != nullptr) {
        return true;
    }
    return false;
}

static void CheckHdiServiceLoaded()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr, "Audio Session HDIServiceManagerGet failed!");
    HdfRemoteService *hdiService = serviceMgr->GetService(serviceMgr, AUDIO_HDI_SERVICE_NAME);
    if (hdiService != nullptr) {
        return;
    }
    HILOGE("hdi service not loaded");
    BtChrDftEventWriteInt(CHR_A2DP_CONNECT_EXCEPTION,
        ServiceUtil::AddrFromBluedroid(BLUEDROID::RawAddress::kEmpty).GetAddress(),
        CHR_ERRCODE,
        HDI_SERVICE_NOT_LOADED);
    auto devmgr = OHOS::HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr != nullptr) {
        devmgr->LoadDevice(AUDIO_HDI_SERVICE_NAME);
    }
}

static void OnServiceStatusReceived(struct ServiceStatusListener *listener, struct ServiceStatus *serviceStatus)
{
    CHECK_AND_RETURN_LOG(serviceStatus != nullptr, "Invalid ServiceStatus!");
    std::string info = serviceStatus->info;
    HILOGI("OnServiceStatusReceived: [service name:%{public}s] [status:%{public}d]",
        serviceStatus->serviceName, serviceStatus->status);
    if (serviceStatus->serviceName != std::string(AUDIO_BLUETOOTH_SERVICE_NAME)) {
        return;
    }
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp source service nullptr");

    if (serviceStatus->status == SERVIE_STATUS_START) {
        HILOGI("Bluetooth hdi service started");
        CheckHdiServiceLoaded();
        auto func = [service]() {
            int connectState = static_cast<int>(BTConnectState::CONNECTED);
            std::vector<RawAddress> addressQueue = g_addressQueue.GetVector();
            for (const auto &addr : addressQueue) {
                ProcessConnection(service, connectState, addr);
            }
            g_addressQueue.Clear();
        };
        return DoInA2dpThread(func);
    }
    if (serviceStatus->status == SERVIE_STATUS_STOP) {
        HILOGI("Bluetooth hdi service stoped");
        service->UpdateActiveDevice(ServiceUtil::AddrFromBluedroid(BLUEDROID::RawAddress::kEmpty));
        btav_source_interface_t *bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
        if (bluetoothA2dpSrcInterface != nullptr) {
            bluetoothA2dpSrcInterface->set_active_device(BLUEDROID::RawAddress::kEmpty);
        }
    }
}

static void RegisterHdiServiceStatusListener()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr, "HDIServiceManagerGet failed!");
    g_listener = HdiServiceStatusListenerNewInstance();
    CHECK_AND_RETURN_LOG(g_listener != nullptr, "HdiServiceStatusListenerNewInstance failed.");
    g_listener->callback = OnServiceStatusReceived;
    int32_t status = serviceMgr->RegisterServiceStatusListener(serviceMgr, g_listener,
                                                                       DeviceClass::DEVICE_CLASS_AUDIO);
    CHECK_AND_RETURN_LOG(status == HDF_SUCCESS, "Register service status listener failed.");
    HILOGI("Hdi service RegisterHdiServiceStatusListener success!");
}

static void UnRegisterDeviceStatusListener()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    CHECK_AND_RETURN_LOG(serviceMgr != nullptr && g_listener != nullptr, "UnRegisterDeviceStatusListener failed!");
    serviceMgr->UnregisterServiceStatusListener(serviceMgr, g_listener);
    HILOGI("Hdi service UnregisterServiceStatusListener success!");
}

void A2dpService::ProcessA2dpHdfLoad(int state, const RawAddress &rawAddr)
{
    HILOGI("deivce:%{public}s, state:%{public}d \n", GET_ENCRYPT_ADDR(rawAddr), state);
    std::string device = rawAddr.GetAddress();
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("deivce:%{public}s start load a2dp_host", GET_ENCRYPT_ADDR(rawAddr));
        hdfLoadedDevice_.Push(device);
        g_loader.OpenLib();
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        if (!hdfLoadedDevice_.IsExist(device)) {
            HILOGI("deivce:%{public}s, not load hdf, skip unload", GET_ENCRYPT_ADDR(rawAddr));
            return;
        }
        hdfLoadedDevice_.Erase(device);
        g_loader.CloseLib();
    }
    if (hdfLoadedDevice_.Empty()) {
        HILOGI("all device disconnect, set empty device to stack");
        if (sBluetoothA2dpSrcInterface != nullptr) {
            sBluetoothA2dpSrcInterface->set_active_device(BLUEDROID::RawAddress::kEmpty);
        }
    }
}

void A2dpService::ProcessAvrcpDynamicLoad(int state, const RawAddress &rawAddr, std::string profileName)
{
    std::string device = rawAddr.GetAddress();
    HILOGI("device:%{public}s, state:%{public}d", GET_ENCRYPT_ADDR(rawAddr), state);

    auto handleLoad = [device, rawAddr, state](
        const std::string& profile, utility::SafeVector<std::string>& loadedDevices) {
        if (state == static_cast<int>(BTConnectState::CONNECTING) ||
            state == static_cast<int>(BTConnectState::CONNECTED)) {
            if (loadedDevices.IsExist(device)) {
                HILOGI("device:%{public}s, %{public}s has loaded avrcp, skip load",
                    GET_ENCRYPT_ADDR(rawAddr), profile.c_str());
                return;
            }
            HILOGI("device:%{public}s, %{public}s start load avrcp", GET_ENCRYPT_ADDR(rawAddr), profile.c_str());
            loadedDevices.Push(device);
            AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->LoadMediaInterfaceLib();
        }
    };

    if (profileName == "a2dp") {
        handleLoad("a2dp", a2dpLoadedDevice_);
    } else if (profileName == "avrcp") {
        handleLoad("avrcp", avrcpLoadedDevice_);
    }
}

static void HandleNotFindDeviceInfoEvent(A2dpService* service, RawAddress& rawAddr, btav_connection_state_t state)
{
    CHECK_AND_RETURN_LOG(service != nullptr, "Not find source service");
    if (state == BTAV_CONNECTION_STATE_DISCONNECTED || state == BTAV_CONNECTION_STATE_DISCONNECTING) {
        return;
    }
    int status = 0;
    if (state == BTAV_CONNECTION_STATE_CONNECTING) {
        status = static_cast<int>(BTConnectState::CONNECTING);
    } else {
        status = static_cast<int>(BTConnectState::CONNECTED);
    }
    service->ConnectManager().AddDevice(rawAddr, status);
}

static void bta2dp_connection_state_callback(const BLUEDROID::RawAddress& bd_addr, btav_connection_state_t state)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("device[%{public}s] state[%{public}d]\n", bd_addr.ToLogString().c_str(), state);

    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service, "Can't get the instance of service");
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bd_addr);
    int connectPolicy = service->GetConnectStrategy(rawAddr);
    btav_source_interface_t* bluetoothA2dpSrcInterface = service->getBluetoothA2dpSrcInterface();
    if (state == BTAV_CONNECTION_STATE_CONNECTING
        && connectPolicy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGW("rejected incoming A2dp connection");
        bluetoothA2dpSrcInterface->disconnect(bd_addr);
        return;
    }
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(rawAddr);
    if (deviceInfo == nullptr) {
        HandleNotFindDeviceInfoEvent(service, rawAddr, state);
        deviceInfo = service->GetDeviceFromList(rawAddr);
        CHECK_AND_RETURN_LOG(deviceInfo != nullptr, "Can't find the device");
    }
    int connectState = ProcessConnectStateMessage(rawAddr, deviceInfo, connectPolicy, state, A2DP_ROLE_SOURCE);
    HILOGI("current active device(%{public}s)", GetEncryptAddr(service->GetActiveSinkDevice().GetAddress()).c_str());
    A2dpHfpRecover::GetInstance().SetTimerForRecoverA2dpService(rawAddr.GetAddress());
    DoInA2dpThread([connectState, service, rawAddr]() {
        service->ProcessAvrcpDynamicLoad(connectState, rawAddr);
        if (connectState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            g_addressQueue.Erase(rawAddr);
            service->ProcessConnectFrameworkCallback(connectState, rawAddr);
            service->ProcessA2dpHdfLoad(connectState, rawAddr);
            if (strcmp(rawAddr.GetAddress().c_str(), service->GetActiveSinkDevice().GetAddress().c_str()) == 0) {
                HILOGI("Remove the active device\n");
                RawAddress removeActive("");
                service->UpdateActiveDevice(removeActive);
            }
            BtChrDftEventWriteInt(CHR_A2DP_CONNECT_EXCEPTION, rawAddr.GetAddress(), CHR_ERRCODE, -1);
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_A2DP_DISCONN_CMP, rawAddr, 0, 0);
            auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
            if (classicAdapter != nullptr && classicAdapter->GetPairState(rawAddr) == PAIR_NONE) {
                service->ConnectManager().DeleteDevice(rawAddr);
            }
            DoInAvrcpTgThread([rawAddr]() {
                AvrcpServiceManager::GetInstance().DisConnect(rawAddr);
            });
        } else if (connectState == static_cast<int>(BTConnectState::CONNECTED)) {
            if (IsA2dpHostLoaded()) {
                ProcessConnection(service, connectState, rawAddr);
            } else {
                g_addressQueue.Push(rawAddr);
            }
            service->ProcessA2dpHdfLoad(connectState, rawAddr);
            ConnectStrategyManager::GetInstance()->SaveLastConnectedDevice(rawAddr.GetAddress());
            service->ConnectManager().RemoveA2dpAbnormalVirtualDevice(rawAddr);
        }
    });

    service->ProcessChrA2dpConnectionState(rawAddr, connectState);
    service->CheckDisable();
}

static void bta2dp_audio_state_callback(const BLUEDROID::RawAddress& bd_addr, btav_audio_state_t state)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("device[%{public}s] state[%{public}d]\n", bd_addr.ToLogString().c_str(), state);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bd_addr);
    int error = RET_NO_ERROR;

    if (service == nullptr) {
        HILOGE("Can't get the instance of service");
        return;
    }

    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(rawAddr);
    if (deviceInfo != nullptr) {
        uint8_t codecType = deviceInfo->GetCodecStatus().codecInfo.codecType;
        std::string callingName = PermissionManager::GetCallingName();
        if (state == BTAV_AUDIO_STATE_LATENCY_CHANGED) {
            return;
        }
        if (state != BTAV_AUDIO_STATE_STARTED) {
            deviceInfo->SetPlayingState(false);
            BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(rawAddr, false, codecType, callingName);
        } else {
            deviceInfo->SetPlayingState(true);
            if (service->IsInWirelessCharging()) {
                service->SetWirelessChargingFlag(false);
                BtChrTransactionManager::GetInstance().WriteTransactionStatictics(
                    TRANSACTION_TYPE_WIRELESS_CHARGING_AUDIO, TRANSACTION_RESULT_SUCCESS);
            }
            BtChrAudioStateStart(bd_addr.ToString());
            BtChrUeManager::GetInstance()->WriteA2dpPlayStateUe(rawAddr, true, codecType, callingName);
            service->ChangeCodecWithSceneSwitch();
        }
        AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->NotifyA2dpPlayStateChange(); // 播放状态变化时，通知media变化
        const RawAddress device = service->GetActiveSinkDevice();
        if (device == rawAddr) {
            //播放状态变化时，通知pagescan变化
            service->HandleA2dpPlayingStateChange(state);
        }
        service->ProcessPlayingFrameworkCallback(state, error, rawAddr);
    } else {
        HILOGE("Not find the device");
    }
}

static void bta2dp_audio_config_callback(
    const BLUEDROID::RawAddress& bd_addr, btav_a2dp_codec_config_t codec_config,
    std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities,
    std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOG_COMM_INFO("bta2dp_audio_config_callback: enter");
    // 当蓝牙开关不为开时，不允许HandleNotFindDeviceInfoEvent设置connecting
    int status = AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR);
    CHECK_AND_RETURN_LOG(status == BTStateID::STATE_TURN_ON, "don't allow to config a2dp");
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bd_addr);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    int error = RET_NO_ERROR;
    CHECK_AND_RETURN_LOG(service != nullptr, "Can't get the instance of service");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(rawAddr);
    if (deviceInfo == nullptr) {
        HILOGI("Not find the device: %{public}s, add it", GET_ENCRYPT_ADDR(rawAddr));
       /*
        * 双发回连场景:
        * 1.协议栈不会上报connecting，无法在bta2dp_connection_state_callback中添加device;
        * 2.协议栈上报编码参数时，可以认为已经在connecting状态，此时添加device.
        */
        HandleNotFindDeviceInfoEvent(service, rawAddr, BTAV_CONNECTION_STATE_CONNECTING);
        deviceInfo = service->GetDeviceFromList(rawAddr);
    }
    A2dpSrcCodecStatus codecStatus;
    if (!ConvertAudioConfigCodec(codec_config, codecStatus.codecInfo)) {
        HILOGE("wrong codec info");
    }
    A2dpSrcCodecInfo a2dpSrcCodecInfo;
    for (auto local_codec_config : codecs_local_capabilities) {
        if (!ConvertAudioConfigCodec(local_codec_config, a2dpSrcCodecInfo)) {
            HILOGE("wrong codec info");
        }
        codecStatus.codecInfoLocalCap.push_back(a2dpSrcCodecInfo);
    }
    for (auto select_codec_config : codecs_selectable_capabilities) {
        if (!ConvertAudioConfigCodec(select_codec_config, a2dpSrcCodecInfo)) {
            HILOGE("wrong codec info");
        }
        codecStatus.codecInfoConfirmedCap.push_back(a2dpSrcCodecInfo);
    }
    codecStatus.codecInfo.codecSpecific1 = static_cast<uint64_t>(codec_config.codec_specific_1);
    codecStatus.codecInfo.codecSpecific2 = static_cast<uint64_t>(codec_config.codec_specific_2);
    codecStatus.codecInfo.codecSpecific3 = static_cast<uint64_t>(codec_config.codec_specific_3);
    codecStatus.codecInfo.codecSpecific4 = static_cast<uint64_t>(codec_config.codec_specific_4);
    CHECK_AND_RETURN_LOG(deviceInfo != nullptr, "deviceInfo is nullptr");
    deviceInfo->SetCodecStatus(codecStatus);
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpCodecUpdateEvent(rawAddr.GetAddress(),
        codecStatus.codecInfo);
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpCodecChangeEvent(rawAddr.GetAddress(),
        codecStatus.codecInfo);
    service->ProcessCodecFrameworkCallback(codecStatus.codecInfo, error, rawAddr);
}

static bool bta2dp_mandatory_codec_preferred_callback(const BLUEDROID::RawAddress& bd_addr)
{
    HILOGE("enter");
    // 目前没有setmandatorycodec接口,改为false，否则无法根据优先级选择编码器
    return false;
}

static btav_source_callbacks_t g_sBluetoothA2dpCallbacks = {
    sizeof(g_sBluetoothA2dpCallbacks),
    bta2dp_connection_state_callback,
    bta2dp_audio_state_callback,
    bta2dp_audio_config_callback,
    bta2dp_mandatory_codec_preferred_callback,
};

btav_source_interface_t* A2dpService::getBluetoothA2dpSrcInterface() const
{
    return sBluetoothA2dpSrcInterface;
}

btav_sink_interface_t* A2dpService::getBluetoothA2dpSnkInterface() const
{
    return sBluetoothA2dpSnkInterface;
}

A2dpService::A2dpService(const std::string& name, const std::string version, const uint8_t role)
    : utility::Context(name, version)
{
    name_ = name;
    version_ = version;
    role_ = role;
    int value = 0;
    profileId_ = PROFILE_ID_A2DP_SRC;

    if (role == A2DP_ROLE_SOURCE) {
        AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, value);
        profileId_ = PROFILE_ID_A2DP_SRC;
    } else {
        AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SNK_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, value);
        profileId_ = PROFILE_ID_A2DP_SINK;
    }

    HILOGI("role[%{public}u] config max devices(%{public}d)\n", role, value);
    if (value > 0 && value < A2DP_CONNECT_NUM_MAX) {
        maxConnectNumSnk_ = value;
    }
}

A2dpService::~A2dpService() {}

utility::Context *A2dpService::GetContext()
{
    return this;
}

void A2dpService::Enable()
{
    HILOGI("role_[%{public}u]\n", role_);
    DoInA2dpThread([this]() { this->EnableService(); });
}

void A2dpService::EnableService()
{
    bool ret = true;

    std::vector<btav_a2dp_codec_config_t> codec_priorities = prepareCodecPreferences();
    std::vector<btav_a2dp_codec_config_t> codec_offloading = prepareCodecPreferences();

    const bt_interface_t* bt_interface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bt_interface == nullptr) {
        HILOGE("Failed to open the Bluetooth module");
        return;
    }
    if (role_ == A2DP_ROLE_SOURCE) {
        sBluetoothA2dpSrcInterface = reinterpret_cast<btav_source_interface_t*>(
            const_cast<void *>(bt_interface->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID)));
        if (sBluetoothA2dpSrcInterface == nullptr) {
            // Open stack DM has not exported A2DP btif glue yet; still report enable so
            // classic adapter can reach STATE_ON without null-calling init().
            HILOGW("A2DP source profile unavailable, skip stack init");
            GetContext()->OnEnable(name_, true);
            return;
        }
#ifndef BT_MCU_PROXY_ENABLE
        sBluetoothA2dpSrcInterface->init(&g_sBluetoothA2dpCallbacks,
            maxConnectNumSnk_, codec_priorities, codec_offloading);
#endif
        const bthwif_interface_t *bthwif =
            reinterpret_cast<const bthwif_interface_t*>(bt_interface->get_profile_interface(BT_VENDER_INTERFACE_ID));
        BluetoothHwInterface::GetInstance()->InitBtHwInterface(bthwif);
        profileId_ = PROFILE_ID_A2DP_SRC;
    }
    RegisterHdiServiceStatusListener();
    GetContext()->OnEnable(name_, ret);
    HILOGI("register audio event start");
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioVolumeEvent();
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterSpatialAudioListener();
#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterCollaborativeAudioListener();
#endif
    HILOGI("register audio event end");
    BluetoothAudioManager::GetInstance().ReadLatencyConfig();
    SubscribeCommonEvent();
}

void A2dpService::Disable()
{
    DoInA2dpThread([this]() { this->DisableService(); });
}

void A2dpService::ClearDeviceMap()
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    a2dpDevices_.clear();
}

void A2dpService::DisableService()
{
    std::list<RawAddress> devices;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
            if (it->second != nullptr) {
                auto device = it->second->GetDevice();
                devices.emplace_back(device);
            }
        }
    }
    for (RawAddress device : devices) {
        ProcessConnectFrameworkCallback(static_cast<int>(BTConnectState::DISCONNECTED), device);
        ProcessA2dpHdfLoad(static_cast<int>(BTConnectState::DISCONNECTED), device);
    }
    ClearDeviceMap();
    RawAddress removeActive("");
    UpdateActiveDevice(removeActive);

#ifndef BT_MCU_PROXY_ENABLE
    if (role_ == A2DP_ROLE_SOURCE) {
        if (sBluetoothA2dpSrcInterface != nullptr) {
            sBluetoothA2dpSrcInterface->cleanup();
        }
    } else {
        if (sBluetoothA2dpSnkInterface != nullptr) {
            sBluetoothA2dpSnkInterface->cleanup();
        }
    }
#endif
    UnRegisterDeviceStatusListener();
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioFrameworkAdapterListener();
    BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioPreferredOutPutDeviceChangeListener();
#ifdef BLUETOOTH_AUDIO_COLLABORATION_SUPPORT
    BluetoothAudioFrameworkAdapter::GetInstance().UnRegisterCollaborativeAudioListener();
#endif
    GetContext()->OnDisable(name_, true);
    BluetoothAudioManager::GetInstance().ClearLatencyInfo();
    if (subscriber_ != nullptr) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

bool A2dpService::IsRemoteA2dpSinkSupported(const RawAddress &device)
{
    std::vector<Uuid> uuids = RemoteDeviceProperties::GetInstance()->GetDeviceUuids(device);
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(BLUETOOTH_UUID_A2DP_SINK)) != uuids.end() ||
        std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(ADV_AUDIO_DIST)) != uuids.end();
}

void A2dpService::NotifyConnStateChangedInner(const RawAddress &device, int state, int cause)
{
    a2dpFramworkCallback_.ForEach([device, state, cause](IA2dpObserver &observer) {
        observer.OnConnectionStateChanged(device, state, cause);
    });
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpConnectStateUpdateEvent(device.GetAddress(), state);
}

void A2dpService::NotifyCaptureConnStateChangedInner(const RawAddress &device)
{
    HILOGI("current device:%{public}s", GET_ENCRYPT_ADDR(device));
    std::shared_ptr<A2dpDeviceInfo> hdapDeviceInfo = GetHdapDeviceInfo();
    CHECK_AND_RETURN_LOG(hdapDeviceInfo != nullptr, "Not find the hdap device");
    RawAddress hdapDevice = hdapDeviceInfo->GetDevice();
    if (hdapDeviceInfo->IsHdapConnected() && hdapDevice.GetAddress() == device.GetAddress()) {
        HILOGI("%{public}s, hdap notify connected", __func__);
        NotifyCaptureConnStateChanged(hdapDevice, static_cast<int>(BTHdapConnectState::CONNECTED),
            hdapDeviceInfo->GetCodecInfo());
    } else {
        HILOGI("%{public}s, hdap notify disConnected", __func__);
        NotifyCaptureConnStateChanged(hdapDevice, static_cast<int>(BTHdapConnectState::DISCONNECTED),
            hdapDeviceInfo->GetCodecInfo());
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        BLUEDROID::RawAddress bdAddr = ServiceUtil::AddrToBluedroid(device);
        bluetoothHwSrcInterface->setHdapActive(bdAddr);
    }
}

void A2dpService::NotifyCaptureConnStateChanged(const RawAddress &device, int state,
    const bluetooth::A2dpSrcCodecInfo &info)
{
    HILOGI("%{public}s, status: %{public}d", __func__, state);
    a2dpFramworkCallback_.ForEach([device, state, info](IA2dpObserver &observer) {
        observer.OnCaptureConnectionStateChanged(device, state, info);
    });
}

int A2dpService::IsLocalDeviceConnectAllowed(const RawAddress &device)
{
    ControlInterceptMessage msg {
        .addr = device.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedA2dpConn(msg),
        BT_ERR_INTERNAL_ERROR, "Restricted by control intercept");
    std::string callingName = PermissionManager::GetCallingName();
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    if (!connectionManager->IsBrLinkAllowed(device.GetAddress())) {
        HILOGE("Failed connect acl link because of MAX links.");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE6, callingName);
        std::shared_ptr<BluetoothDevice> bluetoothDevice =
            RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        bluetoothDevice->SetIsConnectFromLocal(false);
        NotifyConnStateChangedInner(device, GetDeviceState(device),
            static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
        return  Bluetooth::BT_ERR_MAX_CONNECTION;
    }
    if (connectManager_.JudgeConnectExit(device, role_)) {
        HILOGE("The device is connected as another role");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE2, callingName);
        return  Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return Bluetooth::BT_NO_ERROR;
}

int A2dpService::Connect(const RawAddress &device)
{
    HILOGI("[address:%{public}s] role[%{public}u]", GET_ENCRYPT_ADDR(device), role_);
    int ret = IsLocalDeviceConnectAllowed(device);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGE("LocalDevice Connect not Allowed");
        return ret;
    }
    std::string callingName = PermissionManager::GetCallingName();
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    bool isReachMaxConnect = !connectManager_.JudgeConnectedNum();
    if (isReachMaxConnect && !connectionManager->DisconnectForDeviceLimit()) {
        return ConnectDeviceMax(device, callingName);
    }
    if ((GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTED)) ||
        (GetDeviceState(device) == static_cast<int>(BTConnectState::CONNECTING))) {
            HILOGE("Device have been connected");
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE4,
                callingName);
            return  Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (GetConnectStrategy(device) == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        HILOGE("Connection Strategy is FORBIDDEN!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE5, callingName);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (!IsRemoteA2dpSinkSupported(device)) {
        HILOGE("Cannot connect to %{public}s, no a2dp sink UUID ", GET_ENCRYPT_ADDR(device));
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE7, callingName);
        return Bluetooth::BT_ERR_PROFILE_DISABLED;
    }
    if (ProcCloudDeviceConnect(device)) {
        return RET_NO_ERROR;
    }

    isLocalConnect_ = true;
    postA2dpConnectEvent(isReachMaxConnect, device);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE1, callingName);
    return RET_NO_ERROR;
}

void A2dpService::postA2dpConnectEvent(bool isReachMaxConnect, const RawAddress &device)
{
    utility::Message event(A2DP_CONNECT_EVT);
    if (isReachMaxConnect) {
        HILOGI("a2dp connect reach max num, wait first device a2dp disconncted");
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_A2DP,
            [this, event, device]() { this->ProcessEvent(event, const_cast<RawAddress &>(device)); },
            DELAY_A2DP_REACH_MAX_CONNECT, "A2dpProcessEvent");
    } else {
        PostEvent(event, const_cast<RawAddress &>(device));
    }
}

int A2dpService::Disconnect(const RawAddress &device)
{
    HILOGI("[address:%{public}s] role[%{public}u]", GET_ENCRYPT_ADDR(device), role_);
    std::string callingName = PermissionManager::GetCallingName();
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        auto iter = a2dpDevices_.find(device.GetAddress().c_str());
        if (iter == a2dpDevices_.end()) {
            HILOGI("Can't find the statemachine");
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_DISCONN,
                device, UE_COMMON_SCENE_CASE2, callingName);
            return RET_BAD_STATUS;
        }
        info = iter->second;
    }
    if ((info->GetConnectState() == static_cast<int>(BTConnectState::DISCONNECTED)) ||
        (info->GetConnectState() == static_cast<int>(BTConnectState::DISCONNECTING))) {
        HILOGI("Device have been disconnected");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_DISCONN, device, UE_COMMON_SCENE_CASE3, callingName);
        return RET_BAD_STATUS;
    }
    RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
        PROFILE_ID_A2DP_SRC, true);

    utility::Message event(A2DP_DISCONNECT_EVT);
    PostEvent(event, const_cast<RawAddress &>(device));
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_DISCONN, device, UE_COMMON_SCENE_CASE1, callingName);
    return RET_NO_ERROR;
}

std::list<RawAddress> A2dpService::GetConnectingOrConnectionDevices()
{
    std::list<RawAddress> devList;
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
        if (it->second == nullptr) {
            continue;
        }
        connectionState = it->second->GetConnectState();
        if (connectionState == static_cast<int>(BTConnectState::CONNECTING) ||
            connectionState == static_cast<int>(BTConnectState::CONNECTED)) {
            devList.push_back(it->second->GetDevice());
        }
    }
    return devList;
}

std::list<RawAddress> A2dpService::GetConnectDevices()
{
    std::list<RawAddress> devList;
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
        if (it->second == nullptr) {
            continue;
        }
        connectionState = it->second->GetConnectState();
        if (connectionState == static_cast<int>(BTConnectState::CONNECTED)) {
            devList.push_back(it->second->GetDevice());
        }
    }
    return devList;
}

int A2dpService::GetRemoteDeviceConnectStatus(const RawAddress &device)
{
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    bool result = false;
    for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
        if (it->first != device.GetAddress() || it->second == nullptr) {
            continue;
        }
        state = it->second->GetConnectState();
    }
    return state;
}

int A2dpService::GetMaxConnectNum()
{
    return maxConnectNumSnk_;
}

void A2dpService::ProcessConnectFrameworkCallback(int state, const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<BluetoothDevice> bluetoothDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(bluetoothDevice != nullptr, "bluetoothDevice is nullptr");
    HILOGI("device:%{public}s, state:%{public}d", GET_ENCRYPT_ADDR(device), state);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        RemoteDeviceProperties::GetInstance()->SetDevProactiveDisconnectFlag(device.GetAddress(),
            PROFILE_ID_A2DP_SRC, false);
    }
    int cause = RemoteDeviceProperties::GetInstance()->GetDevConnStateChangeCause(device, PROFILE_ID_A2DP_SRC, state);
    cause = bluetoothDevice->DialogControlConvertCause(cause);
    bluetoothDevice->DialogControlRemoveConnectingProfileCount(state);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        cause = bluetoothDevice->IsConnectFromLocalChangeCause(cause);
    }
    NotifyConnStateChangedInner(device, state, cause);
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(service, "Can't get the instance of service");
        if (device.GetAddress() == service->GetActiveSinkDevice().GetAddress()) {
            BluetoothHelper::BluetoothCommonEventHelper::PublishActiveDeviceChangeEvent("", "a2dp", true);
        }
        std::shared_ptr<A2dpDeviceInfo> hdapDeviceInfo = GetHdapDeviceInfo();
        bluetoothDevice->SetIsConnectFromLocal(false);
        if (hdapDeviceInfo != nullptr && hdapDeviceInfo->GetDevice() == device) {
            int32_t isEnabled = -1;
            BluetoothAudioManager::GetInstance().GetWearState(device.GetAddress(), isEnabled);
            HILOGI("Hdap device isWearDetectionEnabled:%{public}d", isEnabled);
            if (hdapDeviceInfo->IsHdapConnected() || isEnabled != WEAR_DETECTION_ENABLED) {
                HILOGW("Hdap device disconnected, notify to upper. Device:%{public}s", GET_ENCRYPT_ADDR(device));
                hdapDeviceInfo->SetIsHdapConnected(false);
                NotifyCaptureConnStateChangedInner(RawAddress(EMPTY_ADDRESS));
            }
            // clear hdap device to prevent expired data from affecting the new active device
            HILOGI("Reset hdapDeviceInfo to nullptr. Device:%{public}s", GET_ENCRYPT_ADDR(device));
            SetHdapDeviceInfo(nullptr);
        }
    }
}


void A2dpService::ProcessChrA2dpConnectionState(const RawAddress rawAddr, const int connectState)
{
    if (connectState == static_cast<int>(BTConnectState::CONNECTED)) {
#ifdef BLUETOOTH_WATCH_ENABLE
        HfpAgService* hfpAgService = HfpAgService::GetService();
        CHECK_AND_RETURN_LOG(hfpAgService, "hfpAgService is nullptr.");
        if (hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED)) {
            BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, rawAddr.GetAddress(), "DISCONNECTREASON", 0);
        }
#endif
    } else if (connectState == static_cast<int>(BTConnectState::CONNECTING)) {
        BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, rawAddr.GetAddress(), "A2DPINITIATOR",
            isLocalConnect_ ? CHR_INITIATOR_LOCAL : CHR_INITIATOR_PEER);
        isLocalConnect_ = false;
    }
}

void PublishA2dpPlayStateChange(const RawAddress &device, const int playingState)
{
    int A2dpPlayState = -1;
    if (playingState == BTAV_AUDIO_STATE_STARTED) {
        A2dpPlayState = static_cast<int>(OHOS::BluetoothHelper::BroadcastA2dpState::A2DP_STATE_PLAY);
    } else {
        A2dpPlayState = static_cast<int>(OHOS::BluetoothHelper::BroadcastA2dpState::A2DP_STATE_PAUSE);
    }
    BluetoothHelper::BluetoothCommonEventHelper::PublishA2dpPlayStateChangeEvent(device.GetAddress(), A2dpPlayState);
}

void A2dpService::ProcessPlayingFrameworkCallback(int playingState, int error, const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    a2dpFramworkCallback_.ForEach([device, playingState, error](IA2dpObserver &observer) {
        observer.OnPlayingStatusChaned(device, playingState, error);
    });
    OHOS::bluetooth::PublishA2dpPlayStateChange(device, playingState);
}

void A2dpService::ProcessCodecFrameworkCallback(
    const bluetooth::A2dpSrcCodecInfo &info, int error, const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    a2dpFramworkCallback_.ForEach(
        [device, info, error](IA2dpObserver &observer) { observer.OnConfigurationChanged(device, info, error); });
}

void A2dpService::NotifyMediaStackChanged(int action, const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOG_COMM_INFO("NotifyMediaStackChanged: %{public}s, action: %{public}d", GET_ENCRYPT_ADDR(device), action);
    DoInA2dpThread([this, action, device]() { this->BluetoothNotifyMediaStackChanged(action, device); });
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_UPDATE_A2DP_OUTPUT, device, GetActiveSinkDevice(),
        action, callingName);
}

void A2dpService::BluetoothNotifyMediaStackChanged(int action, const RawAddress &device)
{
    a2dpFramworkCallback_.ForEach(
        [device, action](IA2dpObserver &observer) { observer.OnMediaStackChanged(device, action); });
}

bool A2dpService::IsActiveDeviceInPlayToPauseBlackList()
{
    return BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_CHANGE_PLAY_STATE_PLAY_TO_PAUSE, GetActiveSinkDevice());
}

void A2dpService::NotifyWearDetectionActionAfterConnected(const RawAddress &device)
{
    bool isSupport = false;
    int32_t isEnabled = -1;
    BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupport);
    BluetoothAudioManager::GetInstance().GetWearState(device.GetAddress(), isEnabled);
    HILOGI("DeviceAddr=%{public}s, isSupport=%{public}d, isEnabled=%{public}d",
        GET_ENCRYPT_ADDR(device), isSupport, isEnabled);
    if (!isSupport || isEnabled != WEAR_DETECTION_ENABLED) {
        return;
    }

    if (BluetoothAudioManager::GetInstance().IsDeviceWearing(device)) {
        NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_WEAR, device);
    } else {
        NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_UNWEAR, device);
    }
}

void A2dpService::NotifyProfileState(const RawAddress &device)
{
    uint8_t a2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    uint8_t hfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    bool ret = BluetoothAudioManager::GetInstance().GetProfileStatus(device.GetAddress(), a2dpState, hfpState);
    if (!ret) {
        return;
    }
    if (a2dpState == static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE)) {
        NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE, device);
    }
}

void A2dpService::ForceStopOffloadPlaying(const RawAddress &device)
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService, "a2dpService is nullptr.");

    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");

    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    bool isRunning = bluetoothHwSrcInterface->a2dpOffloadIsRunning(rawAddr);
    HILOGI("offload running is %{public}d", isRunning);
    if (isRunning) {
        std::shared_ptr<A2dpDeviceInfo> info = GetDeviceFromList(device);
        if (info == nullptr) {
            HILOGW("Device Not Find");
            return;
        }
        if (info->GetConnectState() == static_cast<int>(BTConnectState::CONNECTED) && info->GetPlayingState()) {
            utility::Message event(A2DP_AVDTP_EVT, A2DP_MSG_PROFILE_AUDIO_PLAY_STOP, nullptr);
            PostEvent(event, const_cast<RawAddress &>(device));
        }
    }
}

int A2dpService::GetConnectState()
{
    int result = 0;
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); ++it) {
        if (it->second == nullptr) {
            result |= PROFILE_STATE_DISCONNECTED;
            HILOGE("no a2dp deviceInfo exist");
            continue;
        }
        int connectState = it->second->GetConnectState();
        if (connectState == static_cast<int>(BTConnectState::CONNECTED)) {
            result |= PROFILE_STATE_CONNECTED;
        } else if (connectState == static_cast<int>(BTConnectState::CONNECTING)) {
            result |= PROFILE_STATE_CONNECTING;
        } else if (connectState == static_cast<int>(BTConnectState::DISCONNECTING)) {
            result |= PROFILE_STATE_DISCONNECTING;
        } else if (connectState == static_cast<int>(BTConnectState::DISCONNECTED)) {
            result |= PROFILE_STATE_DISCONNECTED;
        } else {
            HILOGE("error connection state: %{public}d", connectState);
        }
    }
    return result;
}

std::vector<RawAddress> A2dpService::GetDevicesByStates(std::vector<int>& states) const
{
    std::vector<RawAddress> devices = {};
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    for (auto it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
        int connectionState = 0;
        connectionState = it->second->GetConnectState();
        if (FindStateMatched(states, connectionState)) {
            devices.push_back(it->second->GetDevice());
        }
    }

    return devices;
}

bool A2dpService::FindStateMatched(std::vector<int> states, int connectState) const
{
    size_t length = states.size();
    for (size_t i = 0; i < length; i++) {
        int state = 0;
        state = states[i];
        if (connectState == state) {
            return true;
        }
    }

    return false;
}

int A2dpService::GetDeviceState(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    auto iter = a2dpDevices_.find(device.GetAddress().c_str());
    if (iter == a2dpDevices_.end()) {
        HILOGD("Can't find the statemachine");
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    if (IsInCloudBondingState(device)) {
        HILOGI("[CLOUD_PAIR] %{public}s is in cloud connecting", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return static_cast<int>(BTConnectState::CONNECTING);
    }
    return iter->second->GetConnectState();
}

int A2dpService::GetPlayingState(const RawAddress &device, int &state) const
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    auto iter = a2dpDevices_.find(device.GetAddress().c_str());
    if (iter == a2dpDevices_.end()) {
        HILOGE("Can't find the statemachine");
        return BT_ERR_INVALID_STATE;
    }

    state = iter->second->GetPlayingState() ? A2DP_IS_PLAYING : A2DP_NOT_PLAYING;
    return RET_NO_ERROR;
}

bool A2dpService::IsA2dpPlaying(const RawAddress &device)
{
    int playingsState {A2DP_NOT_PLAYING};
    int res = GetPlayingState(device, playingsState);
    if (res == RET_NO_ERROR && playingsState == A2DP_IS_PLAYING) {
        return true;
    }
    return false;
}

void A2dpService::SendCommandPause()
{
    int state = A2DP_NOT_PLAYING;
    GetPlayingState(GetActiveSinkDevice(), state);
    HILOGI("A2DP playing state=%{public}d", state);
    CHECK_AND_RETURN_LOG(
        state == A2DP_IS_PLAYING && isRenderActive_ && BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp(),
        "current device not playing.");
    auto avrcpService = AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader();
    CHECK_AND_RETURN_LOG(avrcpService != nullptr, "avrcpService is nullptr.");
    avrcpService->SendCtrlCommandToAvSession(AVRC_ID_PAUSE);
}

void A2dpService::SetCurrentDeviceMute()
{
    if (timerForRestoreVolumeIfPaused_ != nullptr && timerForRestoreVolumeIfPaused_->IsStarted()) {
        HILOGI("Another timer has started, stop it before set mute");
        timerForRestoreVolumeIfPaused_->Stop();
        BluetoothAudioFrameworkAdapter::GetInstance().SetMusicUnmute();
        timerForRestoreVolumeIfPaused_ = nullptr;
    }
    int result = BluetoothAudioFrameworkAdapter::GetInstance().SetMusicMuteWhenA2dpRelease();
    if (result != BT_NO_ERROR) {
        return;
    }
    //if music is paused, unmute after 1s
    bool isNavigationRunning = isNavigationRunning_;
    timerForRestoreVolumeIfPaused_ = std::make_shared<utility::Timer>([isNavigationRunning]() {
        bool isMusicActive = BluetoothAudioFrameworkAdapter::IsMusicActive(StreamType::STREAM_MUSIC);
        bool isA2dpOutputNow = BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp();
        bool isNeedUnmute = isA2dpOutputNow || !isMusicActive || isNavigationRunning;
        HILOGI("RestoreVolumeTimer: isMusicActive:%{public}d, isA2dpOutputNow:%{public}d, "
            "isNavigationRunning:%{public}d, isNeedUnmute:%{public}d",
            isMusicActive, isA2dpOutputNow, isNavigationRunning, isNeedUnmute);
        if (isNeedUnmute) {
            HILOGI("RestoreVolumeTimer: Unmute stream after %{public}d ms", TIMER_FOR_RESTORE_VOLUME_IF_PAUSED_MS);
            BluetoothAudioFrameworkAdapter::GetInstance().SetMusicUnmute();
        }
    });
    timerForRestoreVolumeIfPaused_->Start(TIMER_FOR_RESTORE_VOLUME_IF_PAUSED_MS);
}

void A2dpService::SetNavigationMode(bool navigationRunning)
{
    HILOGD("naivigation state:%{public}d", navigationRunning);
    isNavigationRunning_ = navigationRunning;
}

int A2dpService::GetMusicState()
{
    HILOGI("music state:%{public}d", musicRunning_);
    return musicRunning_;
}

int A2dpService::GetGameState()
{
    HILOGI("game state:%{public}d", gameRunning_);
    return gameRunning_;
}

void A2dpService::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    if (action == VIRTUAL_DEVICE_ADD && !virtualDevices_.IsExist(address)) {
        HILOGI("Add Virtual Device");
        virtualDevices_.Push(address);
        a2dpFramworkCallback_.ForEach([action, address](IA2dpObserver &observer) {
            observer.OnVirtualDeviceChanged(action, address);
        });
    }

    if (action == VIRTUAL_DEVICE_REMOVE && virtualDevices_.IsExist(address)) {
        HILOGI("Remove Virtual Device");
        virtualDevices_.Erase(address);
        a2dpFramworkCallback_.ForEach([action, address](IA2dpObserver &observer) {
            observer.OnVirtualDeviceChanged(action, address);
        });
    }
}

void A2dpService::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    auto virtualDevices = virtualDevices_.GetVector();

    for (const auto& device : virtualDevices) {
        devices.push_back(device);
    }
}

int A2dpService::SendDeviceSelection(const RawAddress &device, int useA2dp, int useHfp, int userSelect) {
    HILOG_COMM_INFO("SendDeviceSelection: enter");
    auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "classicService is nullptr");

    int hwRemoteDeviceType = classicService->GetHwRemoteDeviceType(device);
    if (hwRemoteDeviceType == HW_CAR_DEVICE_TYPE) {
        HILOGI("user seletion connect send echo89");
        return bluetooth::BluetoothAudioManagerInterface::SendDeviceSelection(device, useA2dp, useHfp, userSelect);
    }
    return RET_NO_ERROR;
}

int A2dpService::SetActiveSinkDevice(const RawAddress &device)
{
    HILOG_COMM_INFO("SetActiveSinkDevice: enter");
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);

    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    btav_source_interface_t* bluetoothA2dpSrcInterface = getBluetoothA2dpSrcInterface();
    RawAddress preDevice = activeDevice_;
    std::string callingName = PermissionManager::GetCallingName();
    if (device.GetAddress() == NULL_ADDRESS || device.GetAddress() == EMPTY_ADDRESS) {
        HILOGI("Address is %{public}s, clear active device.", GET_ENCRYPT_ADDR(device));
        UpdateActiveDevice(device);
        BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioFrameworkAdapterListener();
        OnRenderStateChange(false);
        BluetoothAudioFrameworkAdapter::GetInstance().UnregisterAudioPreferredOutPutDeviceChangeListener();
        NotifyCaptureConnStateChangedInner(device);
        return RET_NO_ERROR;
    }

    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioFrameworkAdapterListener();
    BluetoothAudioFrameworkAdapter::GetInstance().RegisterAudioPreferredOutPutDeviceChangeListener();
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        auto iter = a2dpDevices_.find(device.GetAddress().c_str());
        if (iter == a2dpDevices_.end()) {
            HILOGI("There is no statemachine");
            BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, device,
                UE_FAIL_SET_A2DP_ACTIVE_DEV_NULL, callingName);
            return RET_BAD_STATUS;
        } else {
            if (iter->second->GetConnectState() != static_cast<int>(BTConnectState::CONNECTED)) {
                HILOGE("The device is not connected");
                BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, device,
                    UE_FAIL_SET_A2DP_ACTIVE_DEV_NOT_CONN, callingName);
                return RET_BAD_STATUS;
            }
        }
    }
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHwSrcInterface != nullptr, RET_BAD_STATUS, "bluetoothHwSrcInterface is nullptr");
    bluetoothHwSrcInterface->hwSetSpatialAudioModeEnabled(
        BluetoothAudioFrameworkAdapter::IsSpatialAudioModeEnabled(device.GetAddress()));
    bluetoothHwSrcInterface->hwSetSpatialAudioHeadTrackingEnabled(
        BluetoothAudioFrameworkAdapter::IsSpatialAudioHeadTrackingEnabled(device.GetAddress()));
    BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::SetAdaptiveSwitchStatus(
        BluetoothAudioFrameworkAdapter::IsSpatialAudioAdaptiveSwitchEnabled(device.GetAddress()));

    if (device.GetAddress() == activeDevice_.GetAddress()) {
        HILOGE("The device is already active");
        NotifyCaptureConnStateChangedInner(device);
        AvrcpServiceManager::GetInstance().SwitchAbsVolumeDevice(device);
        BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, device,
            UE_FAIL_SET_A2DP_ACTIVE_DEV_NO_CHANGE, callingName);
        return RET_NO_ERROR;
    } else {
        HILOGE("ready to set_active_device from %{public}s to %{public}s", GET_ENCRYPT_ADDR(preDevice),
            GET_ENCRYPT_ADDR(device));
        UpdateActiveDevice(device);
        bluetoothA2dpSrcInterface->set_active_device(rawAddr);
        ConnectStrategyManager::GetInstance()->SaveLastActiveDevice(device.GetAddress());
        HILOGI("SaveLastActiveDevice");
    }

    AvrcpServiceManager::GetInstance().SwitchAbsVolumeDevice(device);
    BtChrUeManager::GetInstance()->WriteSetActiveDevUe(CHR_UE_SET_A2DP_ACTIVE_DEV, preDevice, device,
        UE_SUCCESS_SET_A2DP_ACTIVE_DEV_GENERAL, callingName);
    BluetoothAudioManager::GetInstance().UpdateBtAudioLatency(device);
    NotifyCaptureConnStateChangedInner(device);
    return RET_NO_ERROR;
}

const RawAddress &A2dpService::GetActiveSinkDevice() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    HILOGI_TIME_LIMIT(GetEncryptAddr(activeDevice_.GetAddress()).c_str(), "address(%{public}s)",
        GetEncryptAddr(activeDevice_.GetAddress()).c_str());

    return activeDevice_;
}

int A2dpService::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    bool res = ConnectStrategyManager::GetInstance()->SaveConnectStrategy(device,
        PROPERTY_A2DP_CONNECTION_POLICY, strategy);
    if (!res) {
        HILOGE("SaveConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    std::string callingName = PermissionManager::GetCallingName();
    if (strategy == static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) {
        auto classicService = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
        CHECK_AND_RETURN_LOG_RET(classicService != nullptr, BT_ERR_INTERNAL_ERROR, "classicService is nullptr");

        int hwRemoteDeviceType = classicService->GetHwRemoteDeviceType(device);
        if (hwRemoteDeviceType == HW_CAR_DEVICE_TYPE && PermissionManager::IsSystemHap()) {
            HILOGI("user seletion sends echo89 on settings");
            SendDeviceSelection(device, USE_A2DP, NO_CHANGE, USER_SELECT);
            BtChrUeManager::GetInstance()->WriteCommonUe(
                CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE10, callingName);
            return BT_NO_ERROR;
        }
        if ((GetDeviceState(device)) == static_cast<int>(BTConnectState::DISCONNECTED)) {
            Connect(device);
            HILOGI("connect a2dp.");
        }
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE8, callingName);
    } else if (strategy == static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        if ((GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTED) &&
            (GetDeviceState(device)) != static_cast<int>(BTConnectState::DISCONNECTING)) {
            Disconnect(device);
            HILOGI("disconnect a2dp.");
        }
        UpdateVirtualDevice(VIRTUAL_DEVICE_REMOVE, device.GetAddress());
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_DISCONN, device, UE_COMMON_SCENE_CASE4, callingName);
    } else {
        HILOGE("SetConnectStrategy failed.");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return Bluetooth::BT_NO_ERROR;
}

int A2dpService::GetConnectStrategy(const RawAddress &device) const
{
    return ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_A2DP_CONNECTION_POLICY);
}

int A2dpService::SendDelay(const RawAddress &device, uint16_t delayValue)
{
    return RET_NO_ERROR;
}

A2dpSrcCodecStatus A2dpService::GetCodecStatus(const RawAddress &device) const
{
    A2dpSrcCodecStatus codecStatus;
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    for (auto bdr : GetDeviceList()) {
        if (bdr.first == device.GetAddress().c_str()) {
            info = bdr.second;
            break;
        }
    }
    if (info != nullptr) {
        codecStatus = info->GetCodecStatus();
    } else {
        HILOGE("device is not found in list");
    }

    return codecStatus;
}

int A2dpService::GetCodecPreference(const RawAddress &device, A2dpSrcCodecInfo &a2dpSrcCodecInfo)
{
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    for (auto bdr : GetDeviceList()) {
        if (bdr.first == device.GetAddress()) {
            info = bdr.second;
            break;
        }
    }
    if (info == nullptr) {
        HILOGE("device is not found in list");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    a2dpSrcCodecInfo = info->GetCodecStatus().codecInfo;
    return Bluetooth::BT_NO_ERROR;
}

static void WriteCodecConfigUe(const RawAddress &device, btav_a2dp_codec_config_t codec_config)
{
    CodecInfo codecInfo;
    codecInfo.codecType = codec_config.codec_type;
    codecInfo.sampleRate = codec_config.sample_rate;
    codecInfo.bitsPerSample = codec_config.bits_per_sample;
    codecInfo.channelMode = codec_config.channel_mode;
    codecInfo.codecSpecific1 = codec_config.codec_specific_1;
    codecInfo.codecSpecific2 = codec_config.codec_specific_2;
    codecInfo.codecSpecific3 = codec_config.codec_specific_1;
    codecInfo.codecSpecific3 = codec_config.codec_specific_4;

    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteCodecConfigUe(device, codecInfo, callingName);
}

int A2dpService::SetCodecPreference(const RawAddress &device, const A2dpSrcCodecInfo &info)
{
    if (GetDeviceState(device) != static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGE("device: %{public}s isn't connected, can't set codec", GET_ENCRYPT_ADDR(device));
        return Bluetooth::BT_ERR_INVALID_STATE;
    }
    HILOGI("SetCodecPreference, device: %{public}s", GET_ENCRYPT_ADDR(device));
    HILOG_COMM_INFO("SetCodecPreference starts, codecPriority = %{public}u,"
           "codecType = %{public}u, sampleRate = %{public}u, bitsPerSample = %{public}d,"
           "channelMode = %{public}d",
           info.codecPriority, info.codecType, info.sampleRate, info.bitsPerSample, info.channelMode);
    A2dpSrcCodecStatus codecStatus = GetCodecStatus(device);
    if (info.codecType == codecStatus.codecInfo.codecType && info.sampleRate == codecStatus.codecInfo.sampleRate
        && info.bitsPerSample == codecStatus.codecInfo.bitsPerSample
        && info.channelMode == codecStatus.codecInfo.channelMode
        && info.codecSpecific4 == codecStatus.codecInfo.codecSpecific4) {
        HILOGE("setCodecConfigPreference: codec not changed.");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    if (!IsConfirmCodecInfo(codecStatus, info)) {
        HILOGE("SetCodecPreference: invalid codec");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    btav_a2dp_codec_config_t codec_config = {
        .codec_type = static_cast<btav_a2dp_codec_index_t>(info.codecType),
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(BTAV_A2DP_CODEC_PRIORITY_HIGHEST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(info.sampleRate),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(info.bitsPerSample),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(info.channelMode),
        .codec_specific_1 = info.codecSpecific1,
        .codec_specific_2 = info.codecSpecific2,
        .codec_specific_3 = info.codecSpecific3,
        .codec_specific_4 = info.codecSpecific4};
    ConvertCodecConfig(codec_config, info);
    std::vector<btav_a2dp_codec_config_t> codec_preferences;
    codec_preferences.push_back(codec_config);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    bt_status_t status = sBluetoothA2dpSrcInterface->config_codec(rawAddr, codec_preferences);
    CHECK_AND_RETURN_LOG_RET(status == BT_STATUS_SUCCESS, Bluetooth::BT_ERR_INVALID_PARAM,
        "Failed codec configuration");
    WriteCodecConfigUe(device, codec_config);
    BtChrDftStatictics::GetInstance()->WriteCodecStatictics(codec_config.codec_type);
    return Bluetooth::BT_NO_ERROR;
}

bool A2dpService::IsConfirmCodecInfo(A2dpSrcCodecStatus codecStatus, A2dpSrcCodecInfo codecInfo) const
{
    for (A2dpSrcCodecInfo codecInfoConfirmed : codecStatus.codecInfoConfirmedCap) {
        if (codecInfo.codecType != codecInfoConfirmed.codecType) {
            continue;
        }
        if ((codecInfo.sampleRate & codecInfoConfirmed.sampleRate) == 0
            && codecInfo.sampleRate != A2DP_SAMPLE_RATE_NONE_USER) {
            continue;
        }
        if ((codecInfo.bitsPerSample & codecInfoConfirmed.bitsPerSample) == 0
            && codecInfo.bitsPerSample != A2DP_SAMPLE_BITS_NONE_USER) {
            continue;
        }
        if ((codecInfo.channelMode & codecInfoConfirmed.channelMode) == 0
            && codecInfo.channelMode != A2DP_CHANNEL_MODE_NONE_USER) {
            continue;
        }
        return true;
    }
    return false;
}

void A2dpService::ConvertCodecConfig(btav_a2dp_codec_config_t &codec_config, const A2dpSrcCodecInfo &codecInfo)
{
    if (codecInfo.codecType == A2DP_CODEC_TYPE_AAC_USER) {
        codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_AAC;
    } else if (codecInfo.codecType == A2DP_CODEC_TYPE_LDAC_USER) {
        codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC;
    } else if (codecInfo.codecType == A2DP_CODEC_TYPE_L2HCST_USER) {
        ConvertL2hcStBitrate(codec_config, codecInfo);
    } else if (codecInfo.codecType == A2DP_CODEC_TYPE_L2HCV2_USER) {
        ConvertL2hcV2Bitrate(codec_config, codecInfo);
    }
    if (codecInfo.channelMode == A2DP_SBC_CHANNEL_MODE_MONO_USER) {
        codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    }
    if (codecInfo.sampleRate == A2DP_SBC_SAMPLE_RATE_44100_USER) {
        codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    }
    if (codecInfo.bitsPerSample == A2DP_SAMPLE_BITS_16_USER) {
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
    }
    if (codecInfo.bitsPerSample == A2DP_SAMPLE_BITS_32_USER) {
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32;
    }
}

void A2dpService::SwitchOptionalCodecs(const RawAddress &device, bool isEnable) {}

int A2dpService::GetOptionalCodecsSupportState(const RawAddress &device) const
{
    return static_cast<int>(A2DP_OPTIONAL_NOT_SUPPORT);
}

int A2dpService::StartPlaying(const RawAddress &device)
{
    return RET_NO_ERROR;
}

int A2dpService::SuspendPlaying(const RawAddress &device)
{
    return RET_NO_ERROR;
}

int A2dpService::StopPlaying(const RawAddress &device)
{
    return RET_NO_ERROR;
}

void A2dpService::RegisterObserver(IA2dpObserver *observer)
{
    a2dpFramworkCallback_.Register(*observer);
}

void A2dpService::DeregisterObserver(IA2dpObserver *observer)
{
    a2dpFramworkCallback_.Deregister(*observer);
}

int A2dpService::WriteFrame(const uint8_t *data, uint32_t size)
{
#ifdef BT_USE_OPEN_STACK
    /* Open-stack path: btif_av → AVDT_WriteReq (contract P1). */
    static uint32_t s_ts = 0;
    s_ts += 1;
    bt_status_t st = btif_av_write_frame(data, size, s_ts);
    if (st != BT_STATUS_SUCCESS) {
        HILOGD("btif_av_write_frame status=%{public}d size=%{public}u", static_cast<int>(st), size);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    return RET_NO_ERROR;
#else
    (void)data;
    (void)size;
    return RET_NO_ERROR;
#endif
}

int A2dpService::GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                                   uint32_t &timeStamp)
{
    if (device.GetAddress() == activeDevice_.GetAddress()) {
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG_RET(bluetoothHwSrcInterface != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR,
                                 "interface nullptr");
        delayValue = bluetoothHwSrcInterface->a2dpOffloadGetLatency();
        HILOGD("%{public}s, latencyMs: %{public}d", __func__, delayValue);
        return Bluetooth::BT_NO_ERROR;
    } else {
        HILOGE("The device is not active");
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }
}

int A2dpService::OffloadStartPlaying(const RawAddress &device, const std::vector<int> &sessionsId)
{
    HILOGI("device: %{public}s, sessions size:%{public}d", GET_ENCRYPT_ADDR(device), sessionsId.size());
    int ret = Bluetooth::BT_NO_ERROR;
    CHECK_AND_RETURN_LOG_RET(sessionsId.size() != 0, Bluetooth::BT_ERR_INVALID_PARAM, "session size zero.");
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        auto iter = a2dpDevices_.find(device.GetAddress());
        if (iter == a2dpDevices_.end()) {
            HILOGI("Can't find the deivce");
            return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
        }
        info = iter->second;
    }
    A2dpOffloadUpdateSessionsState(sessionsId, true);
    if (info == nullptr) {
        HILOGI("Can't find the deivce");
        return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
    }
    HILOGI("connect state:%{public}d, isPlaying:%{public}d, stream size:%{public}d",
        info->GetConnectState(), info->GetPlayingState(), a2dpStreamSessions_.size());
    if (info->GetConnectState() == static_cast<int>(BTConnectState::CONNECTED) && a2dpStreamSessions_.size() != 0) {
        if (offloadDelayStopTimer_ != nullptr) {
            offloadDelayStopTimer_->Stop();
            offloadDelayStopTimer_ = nullptr;
        }
        utility::Message event(A2DP_AVDTP_EVT, A2DP_MSG_PROFILE_AUDIO_PLAY_START, nullptr);
        PostEvent(event, const_cast<RawAddress &>(device));
    } else {
        ret = Bluetooth::BT_ERR_INVALID_STATE;
    }
    return ret;
}

void A2dpService::HwSetActiveMode()
{
    auto addr = GetActiveSinkDevice();
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = GetDeviceFromList(addr);
    if (deviceInfo && (deviceInfo->GetPlayingState() == false)) {
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
        BLUEDROID::RawAddress rawAddress = ServiceUtil::AddrToBluedroid(addr);
        HILOGD("HwSetActiveMode");
        bluetoothHwSrcInterface->hwSetActiveMode(rawAddress);
    }
}

int A2dpService::OffloadStopPlaying(const RawAddress &device, const std::vector<int> &sessionsId)
{
    HILOGD("device: %{public}s, stop sessions size:%{public}d", GET_ENCRYPT_ADDR(device), sessionsId.size());
    int ret = Bluetooth::BT_NO_ERROR;
    bool hasSessionPlaying = A2dpOffloadIsSessionOnPlaying();
    CHECK_AND_RETURN_LOG_RET(sessionsId.size() != 0, Bluetooth::BT_ERR_INVALID_PARAM, "session size zero.");

    if (!hasSessionPlaying) {
        HILOGI("No Session On playing, no need handle stop");
        return ret;
    }
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        auto iter = a2dpDevices_.find(device.GetAddress());
        if (iter == a2dpDevices_.end()) {
            return Bluetooth::BT_ERR_DEVICE_DISCONNECTED;
        }
        info = iter->second;
    }

    A2dpOffloadUpdateSessionsState(sessionsId, false);
    if (offloadDelayStopTimer_ != nullptr) {
        offloadDelayStopTimer_->Stop();
        offloadDelayStopTimer_ = nullptr;
    }
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    if (!systemInterface.IsCallIdle()) {
        HILOGI("audio mode not idle");
        A2dpOffloadHandleDelayStop(device);
        return ret;
    }
    hasSessionPlaying = A2dpOffloadIsSessionOnPlaying();
    if (QuickStopOffloadPlayHook()) {
        HILOGI("stop offload");
        A2dpOffloadHandleDelayStop(device);
        return ret;
    }
    HILOGD("hasSessionPlaying:%{public}d, playingState:%{public}d, streams size:%{public}d", hasSessionPlaying,
        info->GetPlayingState(), a2dpStreamSessions_.size());
    std::shared_ptr<utility::Timer> delayStopTimer = std::make_shared<utility::Timer>(
        [this, device]() { this->A2dpOffloadHandleDelayStop(device); });
    offloadDelayStopTimer_ = delayStopTimer;
    delayStopTimer->Start(OFFLOAD_DELAY_STOP_INTERVAL_MS);
    return ret;
}

int A2dpService::A2dpOffloadSessionPathRequest(const RawAddress &device,
    const std::vector<A2dpSrcStreamInfo> &streamsInfo)
{
    int ret = HW_A2DP_OFFLOAD_SOFTWARE_ENCODING;
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHwSrcInterface != nullptr, ret, "interface nullptr");

    if (BluetoothAudioFrameworkAdapter::BluetoothAdaptiveSpatialRenderingEnabledChangeListener::
        GetAdaptiveSwitchStatus()) {
        HILOGI_TIME_LIMIT(__func__, "(SessionPathRequest)offload turned off, encodePath:%{public}d", ret);
        return ret;
    }
    if (streamsInfo.size() == 0) {
        BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
        ret = bluetoothHwSrcInterface->a2dpOffloadGetSinkStreamEncodingPath(rawAddr, a2dpStreamSessions_, false);
        return ret;
    }

    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        auto iter = a2dpDevices_.find(device.GetAddress());
        if (iter == a2dpDevices_.end()) {
            HILOGI("Can't find the statemachine");
            return ret;
        }
        info = iter->second;
    }
    if (info != nullptr && !info->GetPlayingState()) {
        A2dpOffloadClearNotActiveSessions(streamsInfo);
    }
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    if (!systemInterface.IsCallIdle()) {
        BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
        ret = bluetoothHwSrcInterface->a2dpOffloadGetSinkStreamEncodingPath(rawAddr, a2dpStreamSessions_, false);
        HILOGI("audio mode not idle and ret: %{public}d", ret);
        return ret;
    }
    A2dpOffloadAddRequestSessions(streamsInfo);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    ret = bluetoothHwSrcInterface->a2dpOffloadGetSinkStreamEncodingPath(rawAddr, a2dpStreamSessions_, true);
    if (ret != HW_A2DP_OFFLOAD_HARDWARE_ENCODING) {
        bool hasSessionPlaying = A2dpOffloadIsSessionOnPlaying();
        a2dpStreamSessions_.clear();
        BtChrBusinessEvent::GetInstance().SetOffloadFlag(0);
        if (hasSessionPlaying && streamsInfo.size() != 0) {
            A2dpOffloadHandleDelayStop(device);
        }
    } else {
        BluetoothHwInterface::GetInstance()->UpdateA2dpOffloadCodecStatus(device);
        BtChrBusinessEvent::GetInstance().SetOffloadFlag(1);
    }
    return ret;
}

A2dpSrcOffloadCodecStatus A2dpService::GetA2dpOffloadCodecStatus(const RawAddress &device)
{
    A2dpSrcOffloadCodecStatus offloadCodecStatus;
    std::shared_ptr<A2dpDeviceInfo> info = nullptr;
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        for (auto bdr : GetDeviceList()) {
            if (bdr.first == device.GetAddress()) {
                info = bdr.second;
                break;
            }
        }
    }
    if (info != nullptr) {
        offloadCodecStatus = info->GetOffloadCodecStatus();
    } else {
        HILOGE("device is not found in list");
    }
    return offloadCodecStatus;
}

std::shared_ptr<A2dpDeviceInfo> A2dpService::GetDeviceFromList(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    auto iter = a2dpDevices_.find(device.GetAddress().c_str());
    if (iter == a2dpDevices_.end()) {
        HILOGI_TIME_LIMIT(__func__, "Can't find the statemachine");
        return nullptr;
    } else {
        return iter->second;
    }
}

std::map<std::string, std::shared_ptr<A2dpDeviceInfo>> A2dpService::GetDeviceList() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    return a2dpDevices_;
}

void A2dpService::AddDeviceToList(std::string address, std::shared_ptr<A2dpDeviceInfo> deviceInfo)
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    a2dpDevices_.insert(std::make_pair(address, deviceInfo));
}

void A2dpService::DeleteDeviceFromList(const RawAddress &device)
{
    std::map<std::string, std::shared_ptr<A2dpDeviceInfo>>::iterator it;
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);

    for (it = a2dpDevices_.begin(); it != a2dpDevices_.end(); it++) {
        if (strcmp(it->first.c_str(), device.GetAddress().c_str()) == RET_NO_ERROR) {
            a2dpDevices_.erase(it);
            break;
        }
        HILOGI("device[%{public}s]\n", GetEncryptAddr(it->first).c_str());
    }
}

void A2dpService::PostEvent(utility::Message event, RawAddress &device)
{
    DoInA2dpThread([this, event, device]() { this->ProcessEvent(event, const_cast<RawAddress &>(device)); });
}

void A2dpService::ProcessEvent(utility::Message event, RawAddress &device)
{
    HILOGI("peerAddr(%{public}s)", GET_ENCRYPT_ADDR(device));

    switch (event.what_) {
        case A2DP_CONNECT_EVT:
            if (connectManager_.A2dpConnect(device)) {
                HILOGI("Start connect peer\n");
                {
                    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
                    auto iter = a2dpDevices_.find(device.GetAddress().c_str());
                    if (iter != a2dpDevices_.end()) {
                        iter->second->SetTryReconnectA2dp(true);
                        std::shared_ptr<BluetoothDevice> bluetoothDevice =
                            RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
                        CHECK_AND_RETURN_LOG(bluetoothDevice != nullptr, "bluetoothDevice is nullptr");
                        bluetoothDevice->DialogControlAddConnectingProfileCount();
                    }
                }
            } else {
                HILOGE("Start connect failed\n");
            }
            break;
        case A2DP_DISCONNECT_EVT:
            if (connectManager_.A2dpDisconnect(device)) {
                HILOGI("Start disconnect peer\n");
            } else {
                HILOGE("Start disconnect failed\n");
            }
            break;
        case A2DP_AVDTP_EVT:
            if (connectManager_.A2dpStreamControl(device, event.arg1_)) {
                HILOGI("avdtp contrl peer\n");
            } else {
                HILOGE("avdtp contrl failed\n");
            }
            break;
        default:
            break;
    }
}

A2dpConnectManager& A2dpService::ConnectManager()
{
    return connectManager_;
}

void A2dpService::CheckDisable()
{
    HILOGI("isDoDisable(%{public}d), state(%{public}d)\n", isDoDisable, GetConnectState());
    if (isDoDisable && (GetConnectState() == PROFILE_STATE_DISCONNECTED)) {
        {
            ClearDeviceMap();
            isDoDisable = false;
        }
        GetContext()->OnDisable(name_, true);
    }
}

void A2dpService::UpdateActiveDevice(const RawAddress &device)
{
    {
        std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
        activeDevice_ = device;
    }
    if (device.GetAddress() == NULL_ADDRESS || device.GetAddress() == EMPTY_ADDRESS) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishActiveDeviceChangeEvent("", "a2dp", true);
    } else {
        BluetoothHelper::BluetoothCommonEventHelper::PublishActiveDeviceChangeEvent(device.GetAddress(), "a2dp", true);
    }
    DoInAvrcpTgThread([device]() {
        AvrcpServiceManager::GetInstance().SetActiveDevice(device);
    });
    BluetoothDeviceBatteryManager::GetInstance()->ProcessActiveDeviceChanged(
        PROFILE_NAME_A2DP_SRC, device.GetAddress());
}

void A2dpService::A2dpOffloadAddRequestSessions(const std::vector<A2dpSrcStreamInfo> &streamsInfo)
{
    HwBtA2dpSteamSessionInfo sessionInfo = {};
    for (auto stream : streamsInfo) {
        auto itor = a2dpStreamSessions_.begin();
        for (; itor != a2dpStreamSessions_.end(); itor++) {
            if (itor->sessionId == stream.sessionId) {
                itor->streamType = stream.streamType;
                itor->sampleRate = stream.sampleRate;
                itor->isSpatialAudio = stream.isSpatialAudio;
                break;
            }
        }
        if (itor == a2dpStreamSessions_.end()) {
            sessionInfo.sessionId = stream.sessionId;
            sessionInfo.streamType = stream.streamType;
            sessionInfo.sampleRate = stream.sampleRate;
            sessionInfo.isSpatialAudio = stream.isSpatialAudio;
            sessionInfo.isPlaying = false;
            a2dpStreamSessions_.push_back(sessionInfo);
        }
    }
}

void A2dpService::A2dpOffloadClearNotActiveSessions(const std::vector<A2dpSrcStreamInfo> &streamsInfo)
{
    /* device not on playing, clear reserved request but not started. */
    for (auto itor = a2dpStreamSessions_.begin(); itor != a2dpStreamSessions_.end();) {
        bool found = false;
        for (auto stream : streamsInfo) {
            if (stream.sessionId == itor->sessionId) {
                found = true;
                break;
            }
        }
        if (!found) {
            itor = a2dpStreamSessions_.erase(itor);
        } else {
            itor++;
        }
    }
}

void A2dpService::A2dpOffloadUpdateSessionsState(std::vector<int> sessionsId, bool isPlaying)
{
    if (sessionsId.size() == 0) {
        LOG_INFO("[A2dpService] %{public}s session 0, isPlaying:%{public}d\n", __func__, isPlaying);
        return;
    }
    for (auto itor = a2dpStreamSessions_.begin(); itor != a2dpStreamSessions_.end();) {
        int sessionId = itor->sessionId;
        if (std::find(sessionsId.begin(), sessionsId.end(), sessionId) == sessionsId.end()) {
            itor++;
            continue;
        }
        if (isPlaying) {
            itor->isPlaying = true;
            itor++;
        } else {
            itor = a2dpStreamSessions_.erase(itor);
        }
    }
}

bool A2dpService::A2dpOffloadIsSessionOnPlaying()
{
    for (auto itor : a2dpStreamSessions_) {
        if (itor.isPlaying) {
            return true;
        }
    }
    return false;
}

void A2dpService::A2dpOffloadHandleDelayStop(const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<A2dpDeviceInfo> info = GetDeviceFromList(device);
    if (info == nullptr) {
        HILOGW("Device Not Find");
        return;
    }
    bool hasSessionPlaying = A2dpOffloadIsSessionOnPlaying();
    HILOGI("hasSessionPlaying:%{public}d, playingState:%{public}d, streams size:%{public}d", hasSessionPlaying,
        info->GetPlayingState(), a2dpStreamSessions_.size());
    if (info->GetConnectState() == static_cast<int>(BTConnectState::CONNECTED) && !hasSessionPlaying &&
        info->GetPlayingState()) {
        utility::Message event(A2DP_AVDTP_EVT, A2DP_MSG_PROFILE_AUDIO_PLAY_STOP, nullptr);
        PostEvent(event, const_cast<RawAddress &>(device));
    }
    if (hasSessionPlaying && a2dpStreamSessions_.size() != 0) {
        /* session stop, should request path again to judge whether reserved streams need to swith encode path */
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
        BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
        bluetoothHwSrcInterface->a2dpOffloadGetSinkStreamEncodingPath(rawAddr, a2dpStreamSessions_, true);
    }
}

void PreferenceAacCodec(std::vector<btav_a2dp_codec_config_t> &codec_preferences)
{
    int aacValue;
    AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_CODEC_AAC_SUPPORT, aacValue);
    if (!aacValue) {
        HILOGE("GetValue error[%{public}d]", aacValue);
        return;
    }
    btav_a2dp_codec_config_t codec_configAac = {
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_AAC),
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_AAC),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(BTAV_A2DP_CODEC_SAMPLE_RATE_44100 |
            BTAV_A2DP_CODEC_SAMPLE_RATE_48000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 |
            BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(BTAV_A2DP_CODEC_CHANNEL_MODE_MONO |
            BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
        .codec_specific_1 = 0,
        .codec_specific_2 = 0,
        .codec_specific_3 = 0,
        .codec_specific_4 = 0};
    codec_preferences.push_back(codec_configAac);
}

void PreferenceLdacCodec(std::vector<btav_a2dp_codec_config_t> &codec_preferences)
{
    int ldacValue;
    AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_CODEC_LDAC_SUPPORT, ldacValue);
    if (!ldacValue) {
        HILOGE("GetValue error[%{public}d]", ldacValue);
        return;
    }
    btav_a2dp_codec_config_t codec_configLdac = {
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC),
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(BLUETOOTH_A2DP_CODEC_PRIORITY_LDAC),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(BTAV_A2DP_CODEC_SAMPLE_RATE_44100 |
            BTAV_A2DP_CODEC_SAMPLE_RATE_48000|BTAV_A2DP_CODEC_SAMPLE_RATE_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 |
            BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(BTAV_A2DP_CODEC_CHANNEL_MODE_MONO |
            BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
        .codec_specific_1 = 0,
        .codec_specific_2 = 0,
        .codec_specific_3 = 0,
        .codec_specific_4 = 0};
    codec_preferences.push_back(codec_configLdac);
}

void PreferenceL2hcStCodec(std::vector<btav_a2dp_codec_config_t> &codec_preferences)
{
    int l2hcStValue;
    AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_CODEC_L2HCST_SUPPORT, l2hcStValue);
    if (!l2hcStValue) {
        HILOGE("GetValue error[%{public}d]", l2hcStValue);
        return;
    }
    btav_a2dp_codec_config_t codec_configL2hcSt = {
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(BLUETOOTH_A2DP_CODEC_L2HCST_PRIORITY),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(BTAV_A2DP_CODEC_SAMPLE_RATE_48000 |
            BTAV_A2DP_CODEC_SAMPLE_RATE_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 |
            BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(BTAV_A2DP_CODEC_CHANNEL_MODE_MONO |
            BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
        .codec_specific_1 = 0,
        .codec_specific_2 = 0,
        .codec_specific_3 = 0,
        .codec_specific_4 = 0};
    codec_preferences.push_back(codec_configL2hcSt);
}

std::vector<btav_a2dp_codec_config_t> A2dpService::prepareCodecPreferences()
{
    std::vector<btav_a2dp_codec_config_t> codec_preferences;
    int value;
    AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_CODEC_SBC_SUPPORT, value);
    HILOGI("SRC SBC value[%{public}d]", value);
    if (value) {
        btav_a2dp_codec_config_t codec_config = {
            .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_SBC),
            .codec_priority = static_cast<btav_a2dp_codec_priority_t>(BTAV_A2DP_CODEC_PRIORITY_DEFAULT),
            .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(BTAV_A2DP_CODEC_SAMPLE_RATE_44100 |
                BTAV_A2DP_CODEC_SAMPLE_RATE_48000),
            .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 |
                BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
            .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(BTAV_A2DP_CODEC_CHANNEL_MODE_MONO |
                BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
            .codec_specific_1 = 0,
            .codec_specific_2 = 0,
            .codec_specific_3 = 0,
            .codec_specific_4 = 0};
        codec_preferences.push_back(codec_config);
    }
    int l2hcV2Value;
    AdapterConfig::GetInstance()->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_CODEC_L2HCV2_SUPPORT, l2hcV2Value);
    HILOGI("SRC L2HCV2 value[%{public}d]", l2hcV2Value);
    if (l2hcV2Value) {
        btav_a2dp_codec_config_t codec_configL2hcV2 = {
            .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2),
            .codec_priority = static_cast<btav_a2dp_codec_priority_t>(BLUETOOTH_A2DP_CODEC_L2HCV2_PRIORITY),
            .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(BTAV_A2DP_CODEC_SAMPLE_RATE_48000 |
                BTAV_A2DP_CODEC_SAMPLE_RATE_96000),
            .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 |
                BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
            .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(BTAV_A2DP_CODEC_CHANNEL_MODE_MONO |
                BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO),
            .codec_specific_1 = 0,
            .codec_specific_2 = 0,
            .codec_specific_3 = 0,
            .codec_specific_4 = 0};
        codec_preferences.push_back(codec_configL2hcV2);
    }
    PreferenceAacCodec(codec_preferences);
    PreferenceLdacCodec(codec_preferences);
    PreferenceL2hcStCodec(codec_preferences);
    return codec_preferences;
}

int A2dpService::EnableAutoPlay(const RawAddress &device)
{
    HILOGI("user enable auto play, device: %{public}s", GET_ENCRYPT_ADDR(device));
    std::string addr = device.GetAddress();
    SaveAutoPlayValue(addr, AUTO_PLAY_ALLLOW);
    if (RefusePlayHelper::GetInstance()->IsNeedToSendPlay(addr)) {
        AvrcpServiceManager::GetInstance().GetAvrcpMediaLoader()->SendCtrlCommandToAvSession(AVRC_ID_PLAY);
    }
    BluetoothDialog::DismissCurAndShowNext();
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, device,
        AVRC_ID_PLAY, UE_COMMON_SCENE_CASE4);
    return BT_NO_ERROR;
}

int A2dpService::DisableAutoPlay(const RawAddress &device, const int duration)
{
    HILOGI("user disable auto play, device: %{public}s, duration: %{public}d", GET_ENCRYPT_ADDR(device), duration);
    SaveAutoPlayValue(device.GetAddress(), AUTO_PLAY_NOT_ALLLOW);
    // 因当前settings不支持拦截时长配置，这里先固定大小为6s，保证大部分车机能够正常拦截
    SetRestrictedDuration(device.GetAddress(), DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS);
    RefusePlayHelper::GetInstance()->ClearAuthingDevice(device.GetAddress());
    BluetoothDialog::DismissCurAndShowNext();
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, device,
        AVRC_ID_PLAY, UE_COMMON_SCENE_CASE3);
    return BT_NO_ERROR;
}

int A2dpService::GetAutoPlayDisabledDuration(const RawAddress &device, int &duration)
{
    int autoPlayValue = AUTO_PLAY_UNKNOWN;
    GetAutoPlayValue(device.GetAddress(), autoPlayValue);
    if (autoPlayValue == AUTO_PLAY_NOT_ALLLOW) {
        GetRestrictedDuration(device.GetAddress(), duration);
        if (duration == 0) {
            duration = DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS;
        }
    } else {
        duration = DEFAULT_VALUE;
    }
    HILOGI("device: %{public}s, autoPlayValue: %{public}d, duration: %{public}d",
        GET_ENCRYPT_ADDR(device), autoPlayValue, duration);
    return BT_NO_ERROR;
}

void A2dpService::OnRenderStateChange(bool isRenderActive)
{
    if (isRenderActive_ == isRenderActive) {
        HILOGI_TIME_LIMIT(__func__, "OnRenderStateChange: same isRenderActive");
        return;
    }
    HILOGI("new render state:%{public}d", isRenderActive);
    isRenderActive_ = isRenderActive;
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
    bluetoothHwSrcInterface->notifyAudioRenderState(isRenderActive_);
    if (isRenderActive_) {
        BluetoothAudioManager::GetInstance().UpdateBtAudioLatency(activeDevice_);
    }
}

bool A2dpService::IsA2dpSceneChanged(bool isRenderStreamChanged, bool anyRenderRunning, CodecChangeAppScene lastScene)
{
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bluetoothHwSrcInterface == nullptr) {
        HILOGW("bluetoothHwSrcInterface is null");
        return lastScene != codecChangeScene_;
    }

    int topAppType = bluetoothHwSrcInterface->hwGetCurrentSceneType();
    HILOGD("topAppType=%{public}d, lastIwareAppType_=%{public}d", topAppType, lastIwareAppType_);
    if (lastIwareAppType_ == topAppType) {
        return lastScene != codecChangeScene_;
    }

    if (!isRenderStreamChanged) {
        HILOGI("stream not changed, topAppType=%{public}d, lastAppType=%{public}d", topAppType, lastIwareAppType_);
    }

    // 前台应用类型跟上次保存的类型不一致的场景，先保存新的类型，再结合流状态做进一步判断
    lastIwareAppType_ = topAppType;

    // 没有流运行，则直接认为场景无变化。codec和场景切换推迟到下一次起播时候按实际场景切换
    if (!anyRenderRunning) {
        return false;
    }

    // 场景1：前台应用变了且流类型也变了，则直接进行codec切换检查
    // 场景2：前台应用变了但流类型无变化，则只针对前台变为游戏的时候才做codec切换检查，确保音乐中打开使用非低时延流的游戏，
    // 也能正常进游戏模式
    // 其他场景，如游戏进入后台但是流类型无变化的，不需结合前台应用类型判断，避免频繁切换场景
    if (isRenderStreamChanged || topAppType == DDM_IWARE_GAME) {
        return true;
    }
    return lastScene != codecChangeScene_;
}

CodecChangeAppScene A2dpService::FetchCurrentRenderScene(bool isRenderStreamChanged, bool anyRenderRunning,
    bool gameRunning, bool musicRunning)
{
    if (isRenderStreamChanged) {
        HILOGI("render=%{public}d new=%{public}d, game=%{public}d new="
            "%{public}d, music=%{public}d new=%{public}d", anyRenderRunning_, anyRenderRunning,
            gameRunning_, gameRunning, musicRunning_, musicRunning);
    } else {
        HILOGD("render=%{public}d new=%{public}d, game=%{public}d new="
            "%{public}d, music=%{public}d new=%{public}d", anyRenderRunning_, anyRenderRunning,
            gameRunning_, gameRunning, musicRunning_, musicRunning);
        // 无变化，返回当前全局保存的场景
        return codecChangeScene_;
    }

    // 游戏第一次或者重新打开
    if (!gameRunning_ && gameRunning) {
        return CodecChangeAppScene::APP_TYPE_GAME;
    }
    // 没有其他流的情况下，音乐第一次或者重新打开
    if (!anyRenderRunning_ && musicRunning) {
        return CodecChangeAppScene::APP_TYPE_MUSIC;
    }
    // 没有流运行，或者说game和music都没运行
    if (!anyRenderRunning || (!gameRunning && !musicRunning)) {
        return CodecChangeAppScene::APP_TYPE_OTHER;
    }

    return codecChangeScene_;
}

void A2dpService::OnRenderSceneChange(bool anyRenderRunning, bool gameRunning, bool musicRunning)
{
    /*
      需要change codec 的场景
      1、游戏第一次或者重新打开
      2、没有其他流的情况下，音乐第一次或者重新打开
    */
    {
        std::lock_guard<std::mutex> lock(renderSenceChangeMutex_);
        bool isRenderStreamChanged = (anyRenderRunning_ != anyRenderRunning || gameRunning_ != gameRunning ||
            musicRunning_ != musicRunning);

        CodecChangeAppScene lastScene = codecChangeScene_;
        codecChangeScene_ = FetchCurrentRenderScene(isRenderStreamChanged, anyRenderRunning, gameRunning, musicRunning);
        anyRenderRunning_ = anyRenderRunning;
        gameRunning_ = gameRunning;
        musicRunning_ = musicRunning;

        bool isA2dpSceneChanged = IsA2dpSceneChanged(isRenderStreamChanged, anyRenderRunning, lastScene);

        if (isRenderStreamChanged || isA2dpSceneChanged) {
            HILOGI("last:%{public}d, current:%{public}d, isA2dpSceneChanged:%{public}d",
                static_cast<int>(lastScene), static_cast<int>(codecChangeScene_), isA2dpSceneChanged);
        } else {
            HILOGD("last:%{public}d, current:%{public}d, isA2dpSceneChanged:%{public}d",
                static_cast<int>(lastScene), static_cast<int>(codecChangeScene_), isA2dpSceneChanged);
        }
        
        // 场景或前台应用没有变化，不需要切codec
        if (!isA2dpSceneChanged) {
            return;
        }
    }
    auto device = GetActiveSinkDevice();
    int32_t state = A2DP_PLAYING_STATE::A2DP_NOT_PLAYING;
    GetPlayingState(device, state);
    // 如果已经开始播放，则直接切codec，否则等到播放后再切codec
    if (state == A2DP_PLAYING_STATE::A2DP_IS_PLAYING) {
        ChangeCodecWithSceneSwitch();
    }
}

void A2dpService::ChangeCodecWithSceneSwitch()
{
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");

    // Offload场景，不切采样率
    auto device = GetActiveSinkDevice();
    if (bluetoothHwSrcInterface->a2dpOffloadIsRunning(ServiceUtil::AddrToBluedroid(device))) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(renderSenceChangeMutex_);
        CodecChangeAppScene preferedScene = codecChangeScene_;
        int topAppType = bluetoothHwSrcInterface->hwGetCurrentSceneType();
        bool isGameOrVideoAppType = (topAppType == DDM_IWARE_GAME) || (topAppType == DDM_IWARE_VIDEO);
        HILOG_COMM_INFO("current scene=%{public}d, topAppType=%{public}d, isGameOrVideoAppType=%{public}d",
            static_cast<int>(preferedScene), topAppType, isGameOrVideoAppType);
        if (preferedScene == CodecChangeAppScene::APP_TYPE_OTHER && !isGameOrVideoAppType) {
            return;
        }

        // 前台应用是游戏，但是流类型不是游戏的，也需要走游戏音频场景以保证耳机低时延效果
        if (topAppType == DDM_IWARE_GAME && preferedScene != CodecChangeAppScene::APP_TYPE_GAME) {
            preferedScene = CodecChangeAppScene::APP_TYPE_GAME;
            HILOGI("topAppType is game, revise to game scene %{public}d", preferedScene);
        }
        uint8_t fastFlag = false;
        btav_a2dp_codec_sample_rate_t sampleRate;
        if (preferedScene == CodecChangeAppScene::APP_TYPE_GAME) {
            fastFlag = 1;
            sampleRate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
            if (!isLastOnLowLatency_) {
                bluetoothHwSrcInterface->sendLowLatencyStatus(true);
                isLastOnLowLatency_ = true;
            }
        }
        if (preferedScene == CodecChangeAppScene::APP_TYPE_MUSIC) {
            fastFlag = 0;
            sampleRate = BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
            if (isLastOnLowLatency_) {
                bluetoothHwSrcInterface->sendLowLatencyStatus(false);
                isLastOnLowLatency_ = false;
            }
        }
        bluetoothHwSrcInterface->changeCodec(preferedScene, static_cast<int32_t>(sampleRate), fastFlag);
    }
}

const std::shared_ptr<A2dpDeviceInfo> &A2dpService::GetHdapDeviceInfo() const
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    return hdapDevice_;
}

void A2dpService::SetHdapDeviceInfo(const std::shared_ptr<A2dpDeviceInfo> &hdapDevice)
{
    std::lock_guard<BtRecursiveMutex> lock(g_a2dpServiceMutex);
    hdapDevice_ = hdapDevice;
}

A2dpService *GetServiceInstance(uint8_t role)
{
    HILOGD("role(%{public}u)", role);
    IProfileManager *servManager = IProfileManager::GetInstance();
    A2dpService *service = nullptr;

    if (role == A2DP_ROLE_SOURCE) {
        service = static_cast<A2dpService *>(servManager->GetProfileService(PROFILE_NAME_A2DP_SRC));
    } else {
        service = static_cast<A2dpService *>(servManager->GetProfileService(PROFILE_NAME_A2DP_SINK));
    }
    return service;
}

bool A2dpService::ProcCloudDeviceConnect(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    if (classicAdapter->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        HILOG_COMM_INFO("[CLOUD_PAIR] profile cloud connect is bonding, device:%{public}s",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        NotifyConnStateChangedInner(device, static_cast<int>(BTConnectState::CONNECTING), 0);
        return true;
    }
    if (classicAdapter->IsStartCloudPair(device)) {
        HILOG_COMM_INFO("[CLOUD_PAIR] profile cloud connect create acl first, device:%{public}s",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        classicAdapter->SetCloudBondState(device, CLOUD_BOND_BONDING);
        classicAdapter->CancelDiscoveryIfInCloudBonding(device);
        classicAdapter->CreateAclConnect(device.GetAddress());
        NotifyConnStateChangedInner(device, static_cast<int>(BTConnectState::CONNECTING), 0);
        CloudDeviceManager::GetInstance()->StopBtAclTimer(device);
        CloudDeviceManager::GetInstance()->StartBtAclTimer(device);
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
            static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
            static_cast<int>(UeConnCloudPairSubSceneCode::BEGIN_CLOUD_PAIR));
        return true;
    }
    return false;
}

bool A2dpService::IsInCloudBondingState(const RawAddress &device) const
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    if (classicAdapter->IsCloudDevice(device) &&
        classicAdapter->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        return true;
    }
    return false;
}

void A2dpService::NotifyConnStateChanged(const RawAddress &device, int state, int cause)
{
    NotifyConnStateChangedInner(device, state, cause);
}

void A2dpService::NotifyBondStateChanged(bt_status_t status, const RawAddress &device, bt_bond_state_t state)
{
    HILOGI("device: %{public}s status: %{public}d bondState: %{public}d", GET_ENCRYPT_ADDR(device), status, state);
    if (state != BT_BOND_STATE_NONE || GetDeviceState(device) != static_cast<int>(BTConnectState::DISCONNECTED)) {
        return;
    }
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service, "Can't get the instance of service");
    DoInA2dpThread([device, service] () { service->ConnectManager().DeleteDevice(device); });
}

int A2dpService::ConnectDeviceMax(const RawAddress &device, const std::string &callingName)
{
    HILOGE("The count of connected device is max");
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, device, UE_COMMON_SCENE_CASE3, callingName);
    NotifyConnStateChangedInner(device, GetDeviceState(device),
        static_cast<int>(ConnChangeCause::DISCONNECT_TOO_MANY_CONNECTED_DEVICES));
    return  Bluetooth::BT_ERR_MAX_CONNECTION;
}

void A2dpService::SetWirelessChargingFlag(bool value)
{
    isInWirelessCharging_ = value;
}

bool A2dpService::IsInWirelessCharging()
{
    return isInWirelessCharging_;
}

void A2dpService::SetA2dpPlayingStateCallback(std::function<void(int)> callbackFunction)
{
    a2dpStatecallback_ = callbackFunction;
}
 
void A2dpService::HandleA2dpPlayingStateChange(int state)
{
    if (a2dpStatecallback_ != nullptr) {
        a2dpStatecallback_(state);
    }
    if (state == BTAV_AUDIO_STATE_STOPPED && isLastOnLowLatency_) {
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
        bluetoothHwSrcInterface->sendLowLatencyStatus(false);
        isLastOnLowLatency_ = false;
    }
}

bool A2dpService::SubscribeCommonEvent()
{
    HILOGI("bluetooth subscribe power event.");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // 1 means CORE_EVENT_PRIORITY
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ReceiveMessage>(subscribeInfo, *this);
    if (subscriber_ == nullptr) {
        return false;
    }
    return EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void A2dpService::ReceiveMessage::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const auto &action = eventData.GetWant().GetAction();
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        int32_t type = eventData.GetCode();
        if (type != static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS)) {
            return;
        }
        BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_WIRELESS_CHARGING_AUDIO,
            TRANSACTION_RESULT_TOTAL);
        auto device = a2dpService_.GetActiveSinkDevice();
        // 如果已经开始播放
        if (a2dpService_.IsA2dpPlaying(device)) {
            BtChrTransactionManager::GetInstance().WriteTransactionStatictics(TRANSACTION_TYPE_WIRELESS_CHARGING_AUDIO,
                TRANSACTION_RESULT_SUCCESS);
        } else {
            a2dpService_.SetWirelessChargingFlag(true);
        }
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        a2dpService_.SetWirelessChargingFlag(false);
    }
}
}
}// namespace bluetooth