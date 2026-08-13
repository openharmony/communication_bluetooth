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
#define LOG_TAG "bt_service_hw_interface"
#endif

#include "remote_device_properties.h"
#include "hal_util.h"
#include "bluetooth_hw_interface.h"
#include "log.h"
#include "common_util.h"
#include "bt_hwif.h"
#include "gavdp/a2dp_service.h"
#include "hfp_ag/hfp_ag_service.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "bluetooth_connection_manager.h"
#include "securec.h"
#include "bt_types.h"
#include "classic_adapter.h"
#include "hiecho.h"
#include "hw_bt_hwif.h"
#include "bluetooth_device.h"
#include "adapter_manager.h"
#include "util/safe_vector.h"
#include "thread_util.h"
#include "refuse_play_helper.h"
#ifdef BLUETOOTH_OS_ACCOUNT
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#endif
#include "openssl/sha.h"
#include "hitrace_meter.h"
#include "bt_chr_ue_manager.h"
#include "hw_bt_utils.h"
#include "bluetooth_audio_framework_adapter.h"
#include "bt_func_hook.h"
namespace OHOS {
namespace bluetooth {

#define WPTR_CBACK_SOCKET_OBSERVER(cbWptr, func, ...)                             \
do {                                                                                   \
    DoInSocketThread([cbWptr, __VA_ARGS__]() {                                           \
        std::shared_ptr<SocketConnectionObserver> cbSptr = (cbWptr).lock();            \
        if (cbSptr) {                                                                  \
            cbSptr->func(__VA_ARGS__);                                                 \
        } else {                                                                       \
            BluetoothHwInterface::GetInstance()->ClearSocketInValidObserver();         \
        }});                                                                           \
} while (0)

const int HIECHO_ARGS_LEN = 8;
const int HIECHO_SEND_HEADER_LEN = 3;
const int WEAR_STATUS_LEN = 3;
const int HIECHO_SUCCESS = 0;
const int HIECHO_PARSE_MSG_TYPE_INDEX = 0;
const int HIECHO_SEND_DATA_LEN = 2;
const int HIECHO_SERVICE_ADDR_LEN = 6;
const int HIECHO_ARGS_ID_INDEX = 6;
const int HIECHO_ARGS_TRANS_INDEX = 7;
const int HIECHO_BYTE_ARRAY_LEN = 4;
const int HIECHO_ONE_BYTE_OFFSET = 1;
const int HIECHO_TWO_BYTE_OFFSET = 2;
const int CONFIG_CHANNEL_TWO = 2;
/* echo 5,D send systype */
const uint8_t HIECHO_OS_TYPE_HWOS_6 = 0x06;
/* echo resquest type */
const uint8_t HIECHO_REP_TYPE = 0x01;
const uint16_t DTS_HEADER_LEN = 3;
const uint8_t QUERY_BUSINESS_LEN = 2;
const uint8_t AUTO_CONN_SWITCH_DATA_LEN = 1;
const uint8_t DEVICE_FOUND_CONN_LEN = 2;
const uint8_t DEVICE_FOUND_STATE_LEN = 2;
const uint8_t DEVICE_FOUND_DESCRIPTION_LEN = 21;

const uint8_t DTS_CAP = 0x05; // echo4,1
const uint8_t WEAR_STATUS = 0x06; // echo4,2
const uint8_t DTS_REPORT = 0x07; // echo4,3
const uint8_t AUDIO_CAP_QUERY = 0x2D; // echo8,A
const uint8_t QUERY_BUSINESS = 0x10; // echo5,A
const uint8_t AUTO_CONN_SWITCH = 0x2E; // echo5,E
const uint8_t DEVICE_DESCRIP = 0x23; // echo9,1
const uint8_t DEVICE_CONN = 0x24; // echo9,2
const uint8_t DEVICE_CTRL = 0x25; // echo9,3
const uint8_t DEVICE_STATE = 0x26; // echo9,4

const uint8_t DEFAULT_FIND_DEVICE_CAPACITY = 1;
const uint8_t INVALID_VAL = 0xFF;
const uint8_t DEVICE_CTRL_OBJECT_OFFSET = 0x01; // echo协议中控制对象与接口定义有偏移值
const uint8_t DEVICE_CTRL_CMD_PLAY = 0x00;
const uint8_t DEVICE_CTRL_CMD_VIBRATE = 0x01;
const uint8_t DEVICE_CTRL_CMD_FLASH = 0x02;
const uint8_t DEVICE_CTRL_CMD_LOCK = 0x03;
const uint8_t DEVICE_CTRL_CMD_ERASE = 0x04;
const uint8_t DEVICE_CTRL_TYPE_OFFSET = 0x01; // echo协议 第二个字节放第一个控制类型的值

const uint8_t HIECHO_REQ = 0x01;
const uint8_t HIECHO_RSP = 0x02;
const uint16_t MAX_DATA_LENGTH = 1000;
const uint16_t AUDIO_CAP_QUERY_LEN = 13;
const uint8_t AUDIO_CAP_QUERY_JOIN_INTERVAL_LOW_BYTE = 5;
const uint8_t AUDIO_CAP_QUERY_JOIN_INTERVAL_HIGH_BYTE = 6;
const uint8_t MOVE_EIGHTI_BIT = 8;
const uint8_t HIECHO_COMMON_RSP_VALUE_LEN = 1;
const uint8_t HIECHO_COMMON_RSP_TLV_LEN = 4;
const uint16_t HIECHO_RSP_DATA_LEN = 1;
constexpr const char *BOOL_TRUE_VALUE = "True";
constexpr const char *BOOL_FALSE_VALUE = "False";

using SensorhubObserver = BluetoothHwInterface::SensorhubObserver;
using SocketConnectionObserver = BluetoothHwInterface::SocketConnectionObserver;

static std::unique_ptr<BaseObserverList<SensorhubObserver>> g_sensorhubObservers = nullptr;
static utility::SafeVector<std::weak_ptr<SocketConnectionObserver>> g_socketObservers;

BluetoothHwInterface::BluetoothHwInterface(void) : bthwInterface_(nullptr)
{
    g_sensorhubObservers = std::make_unique<BaseObserverList<SensorhubObserver>>();
}

void BluetoothHwInterface::AddSensorhubObserver(SensorhubObserver &observer)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->Register(observer);
    }
}

void BluetoothHwInterface::RemoveSensorhubObserver(SensorhubObserver &observer)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->Deregister(observer);
    }
}

void BluetoothHwInterface::AddSocketObserver(std::shared_ptr<SocketConnectionObserver> observer)
{
    g_socketObservers.Push(observer);
}

void BluetoothHwInterface::RemoveSocketObserver(std::shared_ptr<SocketConnectionObserver> observer)
{
    g_socketObservers.EraseIf([observer](std::weak_ptr<SocketConnectionObserver> &serverObs) {
        std::shared_ptr<SocketConnectionObserver> sptr = serverObs.lock();
        return sptr == observer;
    });
}

void BluetoothHwInterface::ClearSocketInValidObserver()
{
    g_socketObservers.EraseIf([](std::weak_ptr<SocketConnectionObserver> &clientObs) {
        return clientObs.lock() == nullptr;
    });
}

bool BluetoothHwInterface::GetSha256EncryptHwHashAccount(uint8_t *outHashArray, uint32_t arrayLen)
{
#ifdef BLUETOOTH_OS_ACCOUNT
    if (arrayLen != HW_ACCOUNT_HASH_LEN) {
        return false;
    }
    /* get uid begin */
    std::pair<bool, OHOS::AccountSA::OhosAccountInfo> accountInfo =
        OHOS::AccountSA::OhosAccountKits::GetInstance().QueryOhosAccountInfo();
    if (!accountInfo.first || accountInfo.second.name_.empty()) {
        HILOGW("failed");
        return false;
    }
    const char *accountUid = accountInfo.second.name_.c_str();
    if (accountInfo.second.name_.compare("ohosAnonymousName") == 0) {
        return false;
    }
    // Calculate the SHA-256 hash of the input string
    uint8_t *ret = SHA256(reinterpret_cast<const unsigned char *>(accountUid), strlen(accountUid), outHashArray);
    if (ret == nullptr) {
        HILOGI("SHA256 encrypt failed");
        return false;
    }
    // During SHA256 encryption, a negative value may be inserted into outHashArray in hexadecimal format.
    // As a result, the sent hash value is incorrect.
    // In this case, need to ensure that the sent hash value is positive value.
    for (int i = 0; i < HW_ACCOUNT_HASH_LEN; i++) {
        int8_t value = static_cast<int8_t>(outHashArray[i]);
        outHashArray[i] = value >= 0 ? value : -value;
    }
    return true;
#else
    return false;
#endif
}

bool BluetoothHwInterface::InteropMatch(const uint16_t feature, const RawAddress &device)
{
    HILOGI("feature: %{public}d, device: %{public}s", feature, GET_ENCRYPT_ADDR(device));
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    CHECK_AND_RETURN_LOG_RET(bthwInterface_ != nullptr, false, "bthwInterface_ is null");
    CHECK_AND_RETURN_LOG_RET(bthwInterface_->interopMatch != nullptr, false, "interopMatch is null");
    return bthwInterface_->interopMatch(feature, rawAddr);
}

void BluetoothHwInterface::CleanHfpScoOccupied(const RawAddress &device)
{
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    CHECK_AND_RETURN_LOG(bthwInterface_ != nullptr, "bthwInterface_ is null");
    CHECK_AND_RETURN_LOG(bthwInterface_->hwBtifHfpScoOccupiedClean != nullptr, "hwBtifHfpScoOccupiedClean is null");
    bthwInterface_->hwBtifHfpScoOccupiedClean(rawAddr);
}

__attribute__((no_sanitize("cfi")))
void BluetoothHwInterface::KeepBleScanInConn(const std::string &pkgName, int uid,
    uint8_t transport, const BLUEDROID::RawAddress &addr)
{
    /* pkgName, bundleId */
    const std::vector<std::pair<std::string, int>> keepBleScanCallers = {
        { "softbus_server", 1024 },
    };

    int bundleId = uid;
#ifdef BLUETOOTH_OS_ACCOUNT
    (void)OHOS::AccountSA::OsAccountManager::GetBundleIdFromUid(uid, bundleId);
#else
    constexpr int uidTransformDivisor = 200000;
    bundleId = uid % uidTransformDivisor;
#endif
    HILOGI("pkg name: %{public}s, uid: %{public}d, bundleId: %{public}d",
        pkgName.c_str(), uid, bundleId);

    bool keepBleScan = false;
    for (auto it : keepBleScanCallers) {
        if (it.first == pkgName && it.second == bundleId) {
            HILOGI("should keep ble scan");
            keepBleScan = true;
            break;
        }
    }

    if (!keepBleScan) {
        HILOGI("no need to keep ble scan");
        return;
    }

    if (bthwInterface_ == nullptr) {
        HILOGE("bthwif is null");
        return;
    }

    HwConnAttr attr = { .type = HW_CONN_ATTR_TYPE_KEEP_BLE_SCAN_IN_CONN, .keepBleScanInConn = true };
    bthwInterface_->hwConnAttrSet(transport, addr, &attr);
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_KEEP_BLE_SCAN_IN_CONNECTION,
        ServiceUtil::AddrFromBluedroid(addr), (uid == bundleId) ? 0 : 1, pkgName);
}

BluetoothHwInterface* BluetoothHwInterface::GetInstance(void)
{
    static BluetoothHwInterface singleton;
    return &singleton;
}

const bthwif_interface_t* BluetoothHwInterface::GetBtHwInterface(void)
{
    return bthwInterface_;
}

// only used for TDD test case
void BluetoothHwInterface::SetBtHwInterface(const bthwif_interface_t* interface)
{
    bthwInterface_ = interface;
}

void HwSensorhubResetCb(uint32_t state)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->ForEach([state](SensorhubObserver &observer) {
            observer.SensorhubResetCallback(state);
        });
    }
}

void HwGetSensorhubDevInfoCb(uint8_t *buffer, int length)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->ForEach([buffer, length](SensorhubObserver &observer) {
            observer.SensorhubDevInfoCallback(buffer, length);
        });
    }
}

void ProcessA2dpProfileState(RawAddress addr, uint8_t a2dpState)
{
    if (a2dpState == static_cast<uint8_t>(A2DP_STATUS_INVALID)) {
        return;
    }

    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (service == nullptr) {
        HILOGI("Can't get the instance of a2dp service\n");
        return;
    }

    int action = (a2dpState == static_cast<uint8_t>(A2DP_STATUS_ENABLE)) ?
        static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE) :
        static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE);
    service->NotifyMediaStackChanged(action, addr);
    if ((action == UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE) &&
        (addr.GetAddress() == service->GetActiveSinkDevice().GetAddress())) {
        service->SendCommandPause();
        HILOGI("disable action, hdap notify disConnected");
        A2dpSrcCodecInfo a2DpSrcCodecInfo{};
        service->NotifyCaptureConnStateChanged(addr, static_cast<int>(BTHdapConnectState::DISCONNECTED),
            a2DpSrcCodecInfo);
    }
    if (action == static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE)) {
        service->ForceStopOffloadPlaying(addr);
    }
}

void ProcessHfpProfileState(RawAddress addr, uint8_t hfpState)
{
    if (hfpState == static_cast<uint8_t>(HFP_STATUS_INVALID)) {
        return;
    }

    HfpAgService *hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        HILOGE("Can't get the instance of hfp service");
        return;
    }

    int action = (hfpState == static_cast<uint8_t>(HFP_STATUS_ENABLE)) ?
            static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE) :
            static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE);
    hfpAgService->NotifyHfpStackChanged(action, addr);
}

void BluetoothHwInterface::ReportProfileStatus(const RawAddress &addr, uint8_t a2dpState, uint8_t hfpState)
{
    HILOGI("a2dpState: %{public}d, hfpState: %{public}d", a2dpState, hfpState);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "service nullptr");
    int action = (a2dpState == static_cast<uint8_t>(A2DP_STATUS_ENABLE)) ?
        static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE) :
        static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE);
    service->NotifyMediaStackChanged(action, addr);

    HfpAgService *hfpAgService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(hfpAgService != nullptr, "hfpAgService nullptr");
    action = (hfpState == static_cast<uint8_t>(HFP_STATUS_ENABLE)) ?
        static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE) :
        static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE);
    hfpAgService->NotifyHfpStackChanged(action, addr);
}

/**
 * 是否有其它蓝牙外设在媒体栈栈顶且在播放音乐中
*/
bool IsOtherA2dpDeviceInPlayingMusic(const CapsuleParam &capsuleParam)
{
    if (capsuleParam.isAudioOutputToA2dp && capsuleParam.isMusicActive &&
        !capsuleParam.isA2dpActiveDeviceEqual) {
        HILOGI("exist bluetooth device in top media stack, and musice is active");
        return true;
    }
    return false;
}

/**
 * 是否有其它蓝牙外设在通话栈栈顶且在通话中
*/
bool IsOtherScoDeviceInCalling(const CapsuleParam &capsuleParam)
{
    if (capsuleParam.isAudioOutputToSco && capsuleParam.isInActualcall &&
        !capsuleParam.isHfpActiveDeviceEqual) {
        HILOGI("exist bluetooth device in top hfp stack, and call is active");
        return true;
    }
    return false;
}

void ReportCapsuleInfo(const RawAddress &addr, const uint8_t profileState, const std::string &targetBtDevice)
{
    auto classicAdapter = IAdapterManager::GetInstance()->GetClassicAdapterInterface();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is null");
    std::string capsuleInfo = "+PROFILESTATE=";
    capsuleInfo.append(std::to_string(profileState));
    capsuleInfo.append(",");
    HILOGI("capsuleInfo: %{public}s,", capsuleInfo.c_str());
    if (!targetBtDevice.empty()) {
        capsuleInfo.append(targetBtDevice);
    }
    std::vector<uint8_t> vec(capsuleInfo.begin(), capsuleInfo.end());
    classicAdapter->SendRemoteEchoInfo(addr, vec);
    if (profileState == static_cast<uint8_t>(A2DP_STATUS_ENABLE)) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_TWS_DUAL_CONN_HAIR_CAPSL, addr,
            UE_COMMON_SCENE_CASE1, AUDIO_MANAGER_PKG_NAME);
    } else {
        BtChrUeManager::GetInstance()->ReportPublishProfileDisableUe(CHR_UE_TWS_DUAL_CONN_HAIR_CAPSL, addr,
            targetBtDevice, AUDIO_MANAGER_PKG_NAME, UE_COMMON_SCENE_CASE2);
    }
}

void NotifyAudioManagerShowEnableCapsule(const CapsuleParam &capsuleParam)
{
    if (capsuleParam.isInActualcall &&
        capsuleParam.isHfpActiveDeviceEqual &&
        capsuleParam.isAudioOutputToSco) { // 校验type，防止多次弹胶囊
        ReportCapsuleInfo(capsuleParam.device, static_cast<uint8_t>(A2DP_STATUS_ENABLE), "");
        HILOGI("current in call, send enbale");
    } else {
        if (capsuleParam.isA2dpActiveDeviceEqual &&
            capsuleParam.isMusicActive &&
            capsuleParam.isA2dpPlaying) {
            ReportCapsuleInfo(capsuleParam.device, static_cast<uint8_t>(A2DP_STATUS_ENABLE), "");
            HILOGI("current in playing, send enable");
        }
    }
}

void NotifyAudioManagerShowDisableCapsule(const CapsuleParam &capsuleParam)
{
    std::string targetBtDevice = capsuleParam.targetBtDevice;
    if (targetBtDevice.empty()) {
        targetBtDevice = "Unknown Device";
    }
    if (capsuleParam.isA2dpPlaying && capsuleParam.isMusicActive && capsuleParam.isAudioOutputToA2dp) {
        ReportCapsuleInfo(capsuleParam.device, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE), targetBtDevice);
        HILOGI("current in playing, send disable");
    }
    if (capsuleParam.isAudioConnected) {
        ReportCapsuleInfo(capsuleParam.device, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE), targetBtDevice);
        HILOGI("current in audio connected, send disable");
    } else if (RefusePlayHelper::GetInstance()->IsShowCapsuleAfterHangUp(capsuleParam.device.GetAddress())) {
        ReportCapsuleInfo(capsuleParam.device, static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE), targetBtDevice);
        HILOGI("show capsule after hang up, send disable");
    } else {
        HILOGI("not show disalbe capsule");
    }
}

static const char* BoolToStr(const bool param)
{
    return param ? BOOL_TRUE_VALUE : BOOL_FALSE_VALUE;
}

void PrintCapsuleParam(const CapsuleParam &param)
{
    HILOGI("isAudioOutputToA2dp: %{public}s, isAudioOutputToSco: %{public}s, isMusicActive: %{public}s, "
        "isA2dpPlaying: %{public}s, isInActualcall: %{public}s, isAudioConnected: %{public}s, isA2dpActiveDeviceEqual:"
        " %{public}s, isHfpActiveDeviceEqual: %{public}s", BoolToStr(param.isAudioOutputToA2dp),
        BoolToStr(param.isAudioOutputToSco), BoolToStr(param.isMusicActive), BoolToStr(param.isA2dpPlaying),
        BoolToStr(param.isInActualcall), BoolToStr(param.isAudioConnected), BoolToStr(param.isA2dpActiveDeviceEqual),
        BoolToStr(param.isHfpActiveDeviceEqual));
}

void NotifyAudioManagerShowCapsule(const RawAddress &addr, const uint8_t a2dpState,
    const uint8_t hfpState, std::string &targetBtDevice, bool isEmptyAduio)
{
    std::string addrStr = addr.GetAddress();
    HILOGI("addr: %{public}s, a2dpState: %{public}d, hfpState: %{public}d, isEmptyAduio: %{public}d",
        GET_ENCRYPT_ADDR(addr), a2dpState, hfpState, isEmptyAduio ? 1 : 0);
    HfpAgService *hfpAgService = HfpAgService::GetService();
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG((hfpAgService != nullptr) && (a2dpService != nullptr), "Not find a2dp or hfp ag service");

    CapsuleParam capsuleParam;
    capsuleParam.device = addr;
    capsuleParam.targetBtDevice = targetBtDevice;
    capsuleParam.isAudioOutputToA2dp = BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp();
    capsuleParam.isAudioOutputToSco = BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothSco();
    capsuleParam.isMusicActive = BluetoothAudioFrameworkAdapter::IsMusicActive(StreamType::STREAM_MUSIC);
    capsuleParam.isA2dpPlaying = a2dpService->IsA2dpPlaying(addr);
    capsuleParam.isInActualcall = hfpAgService->IsInActualcall();
    capsuleParam.isAudioConnected = hfpAgService->IsAudioConnected(addrStr);
    capsuleParam.isA2dpActiveDeviceEqual = (a2dpService->GetActiveSinkDevice().GetAddress() == addrStr);
    capsuleParam.isHfpActiveDeviceEqual = (hfpAgService->GetActiveDevice() == addrStr);
    capsuleParam.isStreamEmptyAduio = isEmptyAduio;
    PrintCapsuleParam(capsuleParam);
    // 如果当前A2DP业务类型是空流，不显示胶囊
    if (capsuleParam.isStreamEmptyAduio) {
        HILOGI("Empty Audio Stream, do not display capsule.");
        return;
    }
    // enable胶囊
    if (a2dpState == static_cast<uint8_t>(A2DP_STATUS_ENABLE)) {
        // 如果媒体栈栈顶是其它蓝牙外设，且有音乐播放，此时不切出声栈，不显示胶囊
        // 如果通话栈栈顶是其它蓝牙外设，且在通话中，此时不切出声栈，不显示胶囊
        if (IsOtherA2dpDeviceInPlayingMusic(capsuleParam) || IsOtherScoDeviceInCalling(capsuleParam)) {
            return;
        }
        HILOGI("Abnormal display capsule.");
        NotifyAudioManagerShowEnableCapsule(capsuleParam);
    }

    // disable胶囊
    if (a2dpState == static_cast<uint8_t>(A2DP_STATUS_DISABLE_NO_SEIZE) ||
        a2dpState == static_cast<uint8_t>(A2DP_DISABLE_SEIZE)) {
        NotifyAudioManagerShowDisableCapsule(capsuleParam);
    }
}

void HwProfileStateCallback(BLUEDROID::RawAddress *addr, uint8_t a2dpState,
    uint8_t hfpState, std::string targetBtDevice, uint8_t a2dpServiceType)
{
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*addr);

    BluetoothAudioManager &bluetoothAudioManager = BluetoothAudioManager::GetInstance();
    uint8_t preA2dpState = static_cast<uint8_t>(A2DP_STATUS_ENABLE);
    uint8_t preHfpState = static_cast<uint8_t>(HFP_STATUS_ENABLE);
    bool ret = bluetoothAudioManager.GetProfileStatus(rawAddr.GetAddress(), preA2dpState, preHfpState);
    if (!ret) {
        bluetoothAudioManager.SetProfileStatus(rawAddr.GetAddress(), preA2dpState, preHfpState);
    }
    uint8_t newA2dpState = (a2dpState == static_cast<uint8_t>(A2DP_STATUS_INVALID)) ? preA2dpState : a2dpState;
    uint8_t newHfpState = (hfpState == static_cast<uint8_t>(HFP_STATUS_INVALID)) ? preHfpState : hfpState;
    uint8_t isEmptyAduio = (a2dpServiceType == static_cast<uint8_t>(A2DP_SERVICE_EMPTY_STREAM));
    HILOGI("a2dpState: %{public}d hfpState: %{public}d preA2dpState: %{public}d, preHfpState: "
        "%{public}d newA2dpState: %{public}d, newHfpState: %{public}d, a2dpServiceType: %{public}d",
        a2dpState, hfpState, preA2dpState, preHfpState, newA2dpState, newHfpState, a2dpServiceType);
    BluetoothAudioManager::ProfileStatus newProfileStatus(newA2dpState, newHfpState);
    BluetoothAudioManager::ProfileStatus preProfileStatus(preA2dpState, preHfpState);
    bluetoothAudioManager.profileStatusMap_.FindOldAndSetNew(rawAddr.GetAddress(), preProfileStatus, newProfileStatus);
    if (preA2dpState != newA2dpState || preHfpState != newHfpState) {
        NotifyAudioManagerShowCapsule(rawAddr, newA2dpState, newHfpState, targetBtDevice, isEmptyAduio);
    } else {
        HILOGI("repeat echo88, ignore");
    }
    ProcessHfpProfileState(rawAddr, newHfpState);
    ProcessA2dpProfileState(rawAddr, newA2dpState);
}

void IntToByteArray(int num, uint8_t *byteArray)
{
    int intNumber = 4;
    int byteNumber = 8;
    for (int i = 0; i < intNumber; i++) {
        byteArray[i] = (num >> (i * byteNumber)) & 0xFF;
    }
}

void HwHiechoCombRspData(uint8_t msgType, uint8_t *data, uint16_t dataLen, uint8_t *args)
{
    HILOGI("msgType is: (%{public}u), length is: (%{public}d)", msgType, dataLen);

    if (data == nullptr || dataLen > MAX_DATA_LENGTH) {
        HILOGE("invalid data");
        return;
    }

    std::vector<uint8_t> echoData(HIECHO_SEND_HEADER_LEN, 0);
    std::vector<uint8_t> sourceData(data, data + dataLen);

    echoData[HIECHO_PARSE_MSG_TYPE_INDEX] = msgType;
    echoData[HIECHO_PARSE_MSG_TYPE_INDEX + HIECHO_ONE_BYTE_OFFSET] = dataLen & 0xff;
    echoData[HIECHO_PARSE_MSG_TYPE_INDEX + HIECHO_TWO_BYTE_OFFSET] = (dataLen >> 8) & 0xff; // 8:one byte offset

    echoData.insert(echoData.end(), sourceData.begin(), sourceData.end());

    bt_interface_t *btIf = AdapterManager::GetInstance()->getBluetoothInterface();
    if (btIf == nullptr) {
        HILOGE("Failed to get btIf interface handle");
        return;
    }
    const bthwif_interface_t *bthwif
         = reinterpret_cast<const bthwif_interface_t*>(btIf->get_profile_interface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("Failed to get bthwif interface handle");
        return;
    }
    bthwif->sendHiechoData(HIECHO_RSP, echoData.data(), echoData.size(), args);
}

void BluetoothHwInterface::HwHiechoCombRepData(uint8_t msgType, uint8_t *data, int dataLen, uint8_t *remoteAddr)
{
    HILOGI(
        "[HwHiechoCombRepData]%{public}s: msgType is:%{public}u, length is:%{public}d\n", __func__, msgType, dataLen);
    int len = dataLen + HIECHO_SEND_HEADER_LEN;
    uint8_t echoData[HIECHO_SEND_HEADER_LEN + dataLen];
    echoData[0] = msgType;
    uint8_t byteArray[HIECHO_BYTE_ARRAY_LEN];
    IntToByteArray(dataLen, byteArray);
    uint8_t *dataStream = echoData;
    dataStream++;
    errno_t ret = memcpy_s(dataStream, len - 1, byteArray, HIECHO_SEND_DATA_LEN);
    if (ret != EOK) {
        HILOGI("HwHiechoCombRepData HIECHO_SEND_DATA_LEN memcpy_s failed, ret = %d\n", ret);
        return;
    }
    dataStream += HIECHO_SEND_DATA_LEN;
    ret = memcpy_s(dataStream, dataLen, data, dataLen);
    if (ret != EOK) {
        HILOGI("HwHiechoCombRspData dataLen memcpy_s failed, ret = %d\n", ret);
        return;
    }
    uint8_t args[HIECHO_ARGS_LEN];
    uint8_t *argsStream = args;
    if (memcpy_s(argsStream, HIECHO_ARGS_LEN, remoteAddr, HIECHO_SERVICE_ADDR_LEN)) {
        HILOGI("%s memcpy_s remote_addr failed", __func__);
        return;
    }
    args[HIECHO_ARGS_ID_INDEX] = 0;
    args[HIECHO_ARGS_TRANS_INDEX] = 0;

    const bthwif_interface_t *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bthwif == nullptr) {
        HILOGI("Failed to get bthwif interface handle");
        return;
    }
    bthwif->sendHiechoData(HIECHO_REP_TYPE, echoData, len, args);
}

void BluetoothHwInterface::HwHiechoDeviceCtrl(uint8_t *remoteAddr, uint32_t controlType,
    uint32_t controlTypeVal, uint32_t controlObject)
{
    HILOGI("HwHiechoDeviceCtrl: controlType is %{public}u controlTypeVal %{public}u", controlType, controlTypeVal);
    uint8_t mode[DEVICE_CTRL_REQ_LEN];
    for (int i = 0; i < DEVICE_CTRL_REQ_LEN; i++) {
        mode[i] = INVALID_VAL; // 255 is invalid
    }
    mode[0] = static_cast<uint8_t>(controlObject) + DEVICE_CTRL_OBJECT_OFFSET;
    switch (static_cast<uint8_t>(controlType)) {
        case DEVICE_CTRL_CMD_PLAY:
            mode[DEVICE_CTRL_CMD_PLAY + DEVICE_CTRL_TYPE_OFFSET] = static_cast<uint8_t>(controlTypeVal);
            break;
        case DEVICE_CTRL_CMD_VIBRATE:
            mode[DEVICE_CTRL_CMD_VIBRATE + DEVICE_CTRL_TYPE_OFFSET] = static_cast<uint8_t>(controlTypeVal);
            break;
        case DEVICE_CTRL_CMD_FLASH:
            mode[DEVICE_CTRL_CMD_FLASH + DEVICE_CTRL_TYPE_OFFSET] = static_cast<uint8_t>(controlTypeVal);
            break;
        case DEVICE_CTRL_CMD_LOCK:
            mode[DEVICE_CTRL_CMD_LOCK + DEVICE_CTRL_TYPE_OFFSET] = static_cast<uint8_t>(controlTypeVal);
            break;
        case DEVICE_CTRL_CMD_ERASE:
            mode[DEVICE_CTRL_CMD_ERASE + DEVICE_CTRL_TYPE_OFFSET] = static_cast<uint8_t>(controlTypeVal);
            break;
        default:
            break;
    }
    BluetoothHwInterface::GetInstance()->HwHiechoCombRepData(DEVICE_CTRL, mode, DEVICE_CTRL_REQ_LEN, remoteAddr);
}

void HwA2dpOffloadStateCallback(BLUEDROID::RawAddress *addr, bool isOffload) {}

void HwDeviceCallback(BLUEDROID::RawAddress *addr, int deviceType)
{
    HILOGI("HwDeviceCallback deviceType is %{public}d", deviceType);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*addr);
    RemoteDeviceProperties::GetInstance()->UpdateRemoteHwDeviceType(rawAddr, deviceType);
}

void HwHdapConnectCallback(BLUEDROID::RawAddress *bdAddr, bool isConnected, uint8_t featureBit,
    HdapConfigCallback config)
{
    HILOGI("hdap isConnected %{public}d", isConnected);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*bdAddr);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "Not find source service");
    std::shared_ptr<A2dpDeviceInfo> hdapDeviceInfo = service->GetHdapDeviceInfo();
    if (hdapDeviceInfo == nullptr) {
        HILOGI("init hdapDeviceInfo");
        hdapDeviceInfo = std::make_shared<A2dpDeviceInfo>(rawAddr);
        service->SetHdapDeviceInfo(hdapDeviceInfo);
    }
    if (hdapDeviceInfo->IsHdapConnected() && isConnected) {
        HILOGI("Hdap Redundant Connected");
        return;
    }
    if (!hdapDeviceInfo->IsHdapConnected() && !isConnected) {
        HILOGI("Hdap Redundant Disconnected");
        return;
    }
    hdapDeviceInfo->SetIsHdapConnected(isConnected);
    A2dpSrcCodecInfo info{};
    info.codecType = A2DP_CODEC_TYPE_L2HCV2_USER;
    info.sampleRate = A2DP_L2HCV2_SAMPLE_RATE_48000_USER;
    info.channelMode = A2DP_SBC_CHANNEL_MODE_MONO_USER;
#ifdef COMMUNICATION_L2
    if (config.channel == CONFIG_CHANNEL_TWO) {
        info.channelMode = A2DP_SBC_CHANNEL_MODE_STEREO_USER;
    }
#endif
    info.bitsPerSample = A2DP_SAMPLE_BITS_16_USER;
    hdapDeviceInfo->SetCodecInfo(info);
    if (isConnected) {
        HILOGI("load a2dp host");
        service->ProcessA2dpHdfLoad(static_cast<int>(BTConnectState::CONNECTED), rawAddr);
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
        bluetoothHwSrcInterface->hdapStartSession(config.sampleRate, config.bitWidth, config.channel);
    }
    if (strcmp(rawAddr.GetAddress().c_str(), service->GetActiveSinkDevice().GetAddress().c_str()) != 0) {
        return;
    }
    int connectState = isConnected ? static_cast<int>(BTHdapConnectState::CONNECTED) :
                                     static_cast<int>(BTHdapConnectState::DISCONNECTED);
    HILOGI("hdap connectState %{public}u", connectState);
    service->NotifyCaptureConnStateChanged(rawAddr, connectState, info);
}

void HwA2dpOffloadCodecConfigCallback(BLUEDROID::RawAddress *bdAddr, A2dpOffloadConfigCallback config)
{
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*bdAddr);
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "Not find source service");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = service->GetDeviceFromList(rawAddr);
    CHECK_AND_RETURN_LOG(deviceInfo != nullptr, "Not find the device");
    A2dpSrcOffloadCodecStatus offloadStatus;
    offloadStatus.codecInfo.mediaPacketHeader = static_cast<uint16_t>(config.media_packet_header);
    offloadStatus.codecInfo.mPt = static_cast<uint8_t>(config.m_pt);
    offloadStatus.codecInfo.ssrc = static_cast<uint32_t>(config.ssrc);
    offloadStatus.codecInfo.boundaryFlag = static_cast<uint8_t>(config.boundary_flag);
    offloadStatus.codecInfo.broadcastFlag = static_cast<uint8_t>(config.broadcast_flag);
    offloadStatus.codecInfo.codecType = static_cast<uint32_t>(config.codec_type);
    offloadStatus.codecInfo.maxLatency = static_cast<uint16_t>(config.max_latency);
    offloadStatus.codecInfo.scmsTEnable = static_cast<uint16_t>(config.scms_t_enable);
    offloadStatus.codecInfo.sampleRate = static_cast<uint32_t>(config.sample_rate);
    offloadStatus.codecInfo.encodedAudioBitrate = static_cast<uint32_t>(config.encoded_audio_bitrate);
    offloadStatus.codecInfo.bitsPerSample = static_cast<uint8_t>(config.bits_per_sample);
    offloadStatus.codecInfo.chMode = static_cast<uint8_t>(config.ch_mode);
    offloadStatus.codecInfo.aclHdl = static_cast<uint16_t>(config.acl_hdl);
    offloadStatus.codecInfo.l2cRcid = static_cast<uint16_t>(config.l2c_rcid);
    offloadStatus.codecInfo.mtu = static_cast<uint16_t>(config.mtu);
    offloadStatus.codecInfo.codecSpecific0 = static_cast<uint8_t>(config.codec_specific_0);
    offloadStatus.codecInfo.codecSpecific1 = static_cast<uint8_t>(config.codec_specific_1);
    offloadStatus.codecInfo.codecSpecific2 = static_cast<uint8_t>(config.codec_specific_2);
    offloadStatus.codecInfo.codecSpecific3 = static_cast<uint8_t>(config.codec_specific_3);
    offloadStatus.codecInfo.codecSpecific4 = static_cast<uint8_t>(config.codec_specific_4);
    offloadStatus.codecInfo.codecSpecific5 = static_cast<uint8_t>(config.codec_specific_5);
    offloadStatus.codecInfo.codecSpecific6 = static_cast<uint8_t>(config.codec_specific_6);
    offloadStatus.codecInfo.codecSpecific7 = static_cast<uint8_t>(config.codec_specific_7);
    deviceInfo->SetOffloadCodecStatus(offloadStatus);
    HILOGI("codecType:%{public}x, mtu:%{public}d, bitRate:%{public}d", offloadStatus.codecInfo.codecType,
        offloadStatus.codecInfo.mtu, offloadStatus.codecInfo.encodedAudioBitrate);
}

void HwHiechoSendPerServiceCapReqData(uint8_t cap, const BLUEDROID::RawAddress &remoteAddr)
{
    uint8_t addrBuf[BD_ADDR_LEN];
    uint8_t *addrStream = addrBuf;
    if (memcpy_s(addrStream, BD_ADDR_LEN, remoteAddr.address, BD_ADDR_LEN) != EOK) {
        HILOGI("%{public}s, memcpy_s addrBuf failed", __func__);
        return;
    }

    uint8_t mode[DEVICE_DESCRIPTION_REQ_LEN];
    mode[0] = static_cast<uint8_t>(cap);
    BluetoothHwInterface::GetInstance()->HwHiechoCombRepData(DEVICE_DESCRIP, mode,
        DEVICE_DESCRIPTION_REQ_LEN, addrBuf);
}

bool HwIsSupportMlinkDevice()
{
    bool isSupport = false;
    auto* bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bthwif == nullptr) {
        HILOGW("bthwif is null, return not support");
        return isSupport;
    }
    uint8_t productType = bthwif->hwGetLocalDeviceEchoType();
    HILOGI("current device echo type:%{public}x", productType);
    if (productType == static_cast<uint8_t>(HW_ECHO_PRODUCT::PHONE) ||
        productType == static_cast<uint8_t>(HW_ECHO_PRODUCT::TABLET) ||
        productType == static_cast<uint8_t>(HW_ECHO_PRODUCT::LAPTOP)) {
        isSupport = true;
    }
    return isSupport;
}

void HwHiechoSendHwAccountHashToTwsCb(const BLUEDROID::RawAddress &remoteAddr)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("%{public}s", __func__);

    uint8_t addrBuf[BD_ADDR_LEN];
    uint8_t *addrStream = addrBuf;
    if (memcpy_s(addrStream, BD_ADDR_LEN, remoteAddr.address, BD_ADDR_LEN) != EOK) {
        HILOGI("%{public}s, memcpy_s addrBuf failed", __func__);
        return;
    }

    std::vector<uint8_t> accountHash(HW_ACCOUNT_HASH_LEN + 1, 0);
    accountHash[0] = HIECHO_OS_TYPE_HWOS_6;
#ifdef BLUETOOTH_OS_ACCOUNT
    uint8_t cipherHash[HW_ACCOUNT_HASH_LEN] = {0};
    if (HwIsSupportMlinkDevice()) {
        if (BluetoothHwInterface::GetInstance()->GetSha256EncryptHwHashAccount(cipherHash, HW_ACCOUNT_HASH_LEN)) {
            std::copy(std::begin(cipherHash), std::end(cipherHash), accountHash.begin() + 1);
        }
    } else {
        HILOGI("%{public}s, not support mlink device, don't fill cipherHash", __func__);
    }
#endif
    HILOGI("%{public}s accountHash = %{public}X", __func__, accountHash[1]);
    BluetoothHwInterface::GetInstance()->HwHiechoCombRepData(HW_ACCOUNT_HASH, accountHash.data(),
        accountHash.size(), addrBuf);
}
/* Send echo 5D req end */

#ifdef CONTEXTHUB_BLE_V3
static void HwSensorhubCollaborationCb(const std::vector<uint8_t> &notifyValue)
{
    if (g_sensorhubObservers) {
        g_sensorhubObservers->ForEach([data = notifyValue](SensorhubObserver &observer) {
            observer.SensorhubCollaborationCallback(data);
        });
    }
}
#endif

void HwHiechoDeviceAclEncryptionChangedCb(const BLUEDROID::RawAddress &remoteAddr)
{
    HwHiechoSendHwAccountHashToTwsCb(remoteAddr); /* 发送echo5,D */

    uint8_t cap = DEFAULT_FIND_DEVICE_CAPACITY;
    HwHiechoSendPerServiceCapReqData(cap, remoteAddr); /* 发送echo9,1 */
}

void HwReportA2dpAbnormalStatusCb(const BLUEDROID::RawAddress &remoteAddr, int32_t errCode)
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService != nullptr, "a2dpService nullptr");
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(remoteAddr);
    a2dpService->ConnectManager().ReceiveA2dpAbnormalError(rawAddr, errCode);
}

void HwHiechoDtsCap(uint8_t echoType, uint8_t *args)
{
    if (args == nullptr) {
        return;
    }
    if (echoType != HIECHO_REQ) {
        return;
    }
    const uint8_t dtsSeviceVoice = 0x01;
    const uint8_t dtsSeviceWear = 0x02;
    const uint8_t dtsSeviceFoat = 0x03;
    const uint8_t dtsSeviceChr = 0x04;
    const uint8_t dtsSeviceBattary = 0x05;
    uint8_t dtsCaps[] = {dtsSeviceVoice, dtsSeviceWear, dtsSeviceFoat,
        dtsSeviceChr, dtsSeviceBattary};
    HwHiechoCombRspData(DTS_CAP, dtsCaps, sizeof(dtsCaps), args);
}

void HwHiechoAudioCapReq(const std::vector<uint8_t> &payload, uint16_t len, uint8_t *args)
{
    if (len != AUDIO_CAP_QUERY_LEN || payload.size() != AUDIO_CAP_QUERY_LEN) {
        HILOGE("invalid length %{public}u or payload size %{public}zu", len, payload.size());
        return;
    }
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    int32_t avLatency = (payload[AUDIO_CAP_QUERY_JOIN_INTERVAL_LOW_BYTE] & 0xFF)
        | ((payload[AUDIO_CAP_QUERY_JOIN_INTERVAL_HIGH_BYTE] & 0xFF) << MOVE_EIGHTI_BIT);
    BluetoothAudioManager::GetInstance().SaveHiechoAvLatency(RawAddress::ConvertToString(addr.data()), avLatency);
}

void HwHiechoWearState(const RawAddress &addr, const std::vector<uint8_t> &payload,
    uint16_t len, uint8_t *args)
{
    if (payload.size() != WEAR_STATUS_LEN || len != WEAR_STATUS_LEN) {
        HILOGE("invalid length %{public}u or payload size %{public}zu", len, payload.size());
        return;
    }
    uint8_t rspData = HIECHO_SUCCESS;
    HwHiechoCombRspData(WEAR_STATUS, &rspData, 1, args);
    uint8_t leftState = payload[0];
    uint8_t rightState = payload[1];
    BluetoothAudioManager::GetInstance().WearStateHiechoProcess(addr, leftState, rightState);
}

void HwHiechoDtsReport(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args, uint16_t argsLen)
{
    if (echoType == HIECHO_RSP) {
        HILOGE("wrong echoType = %{public}u", echoType);
        return;
    }
    if (len <= DTS_HEADER_LEN) {
        HILOGE("wrong length = %{public}u", len);
        return;
    }
    HwHiechoCombRspData(DTS_REPORT, payload, len, args);

    if (argsLen < HIECHO_SERVICE_ADDR_LEN) {
        HILOGE("wrong argsLen = %{public}u", argsLen);
        return;
    }
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();
    std::string atCommand(payload + DTS_HEADER_LEN, payload + len);

    std::string formatCommand = atCommand;
    size_t pos = atCommand.find("\r");
    if (pos != std::string::npos) {
        formatCommand = atCommand.substr(0, pos);
    }
    HILOGI("HwHiechoDtsReport len = %{public}d, formatCommand = %{public}s, addr = %{public}s",
        len, formatCommand.c_str(), GetEncryptAddr(address).c_str());
    if (BluetoothDeviceBatteryManager::GetInstance()->ProcessHwBatteryCommond(address, formatCommand,
        BATTERY_OBTAIN_TYPE_ECHO)) {
        return;
    }

    if (StartWith(formatCommand, SET_HW_DEVICE_FEATURE)) {
        HfpAgService *hfpAgService = HfpAgService::GetService();
        if (hfpAgService != nullptr) {
            hfpAgService->ProcessHwDeviceInfoCmd(address, formatCommand);
        }
    }
    return;
}

void HwHiechoQueryConn(uint8_t echoType, const std::vector<uint8_t> &payload, int length, uint8_t *args)
{
    CHECK_AND_RETURN_LOG(echoType != HIECHO_REQ, "Not find source service");
    CHECK_AND_RETURN_LOG(length == QUERY_BUSINESS_LEN, "data len wrong:%{public}d", length);
    uint8_t type = payload[0];
    uint8_t typeValue = payload[1];
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();
    HILOGI("HwHiechoQueryConn type:%{public}d, typeValue:%{public}d", type, typeValue);
    BluetoothConnectionManager::GetInstance()->OnGetConnReasonEchoRsp(address, type, typeValue);
}

void HwHiechoAutoConnSwitch(uint8_t echoType, const std::vector<uint8_t> &payload, uint16_t len, uint8_t *args)
{
    CHECK_AND_RETURN_LOG(echoType != HIECHO_RSP, "Not find source service");
    CHECK_AND_RETURN_LOG(len == AUTO_CONN_SWITCH_DATA_LEN, "data len wrong:%{public}d", len);
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    uint8_t rspData = HIECHO_SUCCESS;
    HwHiechoCombRspData(WEAR_STATUS, &rspData, 1, args);
    bool autoConnSwitch = payload[0];
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(address, autoConnSwitch);
}

void HwHiechoParseDescriptionRsp(uint8_t echoType, const std::vector<uint8_t> &payload, uint16_t length, uint8_t *args)
{
    CHECK_AND_RETURN_LOG(echoType == HIECHO_RSP, "Not find source service");
    CHECK_AND_RETURN_LOG(length == DEVICE_FOUND_DESCRIPTION_LEN, "data length wrong: %{public}d", length);
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();

    BluetoothDeviceBatteryManager *bluetoothDeviceBatteryManager = BluetoothDeviceBatteryManager::GetInstance();
    if (bluetoothDeviceBatteryManager != nullptr) {
        bluetoothDeviceBatteryManager->ProcessFindDeviceCommond(address, payload);
    }
}

void HwHiechoDeviceFoundConn(uint8_t echoType, const std::vector<uint8_t> &payload, int length, uint8_t *args)
{
    CHECK_AND_RETURN_LOG(echoType == HIECHO_REQ, "Not find source service");
    CHECK_AND_RETURN_LOG(length == DEVICE_FOUND_CONN_LEN, "data length wrong:%{public}d", length);
    uint8_t properties = payload[0];
    uint8_t state = payload[1];
    uint8_t rspData = HIECHO_SUCCESS;
    uint16_t dataLen = HIECHO_RSP_DATA_LEN;
    HwHiechoCombRspData(DEVICE_CONN, &rspData, dataLen, args);
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();

    BluetoothDeviceBatteryManager *bluetoothDeviceBatteryManager = BluetoothDeviceBatteryManager::GetInstance();
    if (bluetoothDeviceBatteryManager != nullptr) {
        bluetoothDeviceBatteryManager->DeviceConnUpdate(address, properties, state);
    }
}

void HwHiechoDeviceFoundState(uint8_t echoType, const std::vector<uint8_t> &payload, int length, uint8_t *args)
{
    CHECK_AND_RETURN_LOG(echoType == HIECHO_REQ, "Not find source service");
    CHECK_AND_RETURN_LOG(length == DEVICE_FOUND_STATE_LEN, "data length wrong:%{public}d", length);
    uint8_t leftVal = payload[0];
    uint8_t rightVal = payload[1];
    uint8_t rspData = HIECHO_SUCCESS;
    uint16_t dataLen = HIECHO_RSP_DATA_LEN;
    HwHiechoCombRspData(DEVICE_STATE, &rspData, dataLen, args);
    std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
    std::reverse(std::begin(addr), std::end(addr));
    std::string address = RawAddress::ConvertToString(addr.data()).GetAddress();

    BluetoothDeviceBatteryManager *bluetoothDeviceBatteryManager = BluetoothDeviceBatteryManager::GetInstance();
    if (bluetoothDeviceBatteryManager != nullptr) {
        bluetoothDeviceBatteryManager->DeviceControlStatus(address, leftVal, rightVal);
    }
}

void HwHiechoIndCallBack(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args,
                         uint16_t argsLen)
{
    if (argsLen != HIECHO_ARGS_LEN || len <= HIECHO_SEND_HEADER_LEN) {
        HILOGE("wrong length and argsLen");
        return;
    }

    uint8_t msgType;
    STREAM_TO_UINT8(msgType, payload);
    uint16_t dataLength;
    STREAM_TO_UINT16(dataLength, payload);

    if (dataLength > (len - HIECHO_SEND_HEADER_LEN)) {
        HILOGE("wrong dataLength datalen:%{public}d payloadlen:%{public}d", dataLength, len);
        return;
    }

    HILOGD("echoType(%{public}u) msgType(%{public}u)", echoType, msgType);
    std::vector<uint8_t> dataPayLoad(payload, payload + dataLength);
    switch (msgType) {
        case DTS_CAP:
            HwHiechoDtsCap(echoType, args);
            break;
        case AUDIO_CAP_QUERY:
            HwHiechoAudioCapReq(dataPayLoad, dataLength, args);
            break;
        case WEAR_STATUS: {
            std::vector<uint8_t> addr(args, args + HIECHO_SERVICE_ADDR_LEN);
            std::reverse(std::begin(addr), std::end(addr));
            HwHiechoWearState(RawAddress::ConvertToString(addr.data()), dataPayLoad, dataLength, args);
            break;
        }
        case DTS_REPORT:
            HwHiechoDtsReport(echoType, dataPayLoad.data(), dataLength, args, argsLen);
            break;
        case QUERY_BUSINESS:
            HwHiechoQueryConn(echoType, dataPayLoad, dataLength, args);
            break;
        case AUTO_CONN_SWITCH:
            HwHiechoAutoConnSwitch(echoType, dataPayLoad, dataLength, args);
            break;
        case DEVICE_DESCRIP: // echo9,1
            HwHiechoParseDescriptionRsp(echoType, dataPayLoad, dataLength, args);
            break;
        case DEVICE_CONN: // echo9,2
            HwHiechoDeviceFoundConn(echoType, dataPayLoad, dataLength, args);
            break;
        case DEVICE_STATE: // echo9,4
            HwHiechoDeviceFoundState(echoType, dataPayLoad, dataLength, args);
            break;
        default:
            break;
    }
}

void HwHiechoIndCallCb(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args,
                       uint16_t argsLen)
{
    if (payload == nullptr || args == nullptr) {
        HILOGI("args is null");
        return;
    }
    HwHiechoIndCallBack(echoType, payload, len, args, argsLen);
}

void HwBluetoothConnCb(const StackCallbackParam &param)
{
    g_socketObservers.ForEach([param](std::weak_ptr<SocketConnectionObserver> observer) {
        WPTR_CBACK_SOCKET_OBSERVER(observer, ConnectCallback, param);
    });
}

void StackErrnoCallback(bt_status_t status, BLUEDROID::RawAddress *addr, BtStackErrno state)
{
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*addr);
    RemoteDeviceProperties::GetInstance()->StackErrnoCallback(rawAddr, status, state);
}

void HwAclDisconnReasonCb(BLUEDROID::RawAddress *addr, int reason)
{
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(*addr);
    OnAclDisconnectReasonHook(rawAddr, reason);
}

static BthwifCallbacks sBluetoothHWCallbacks = {
    sizeof(sBluetoothHWCallbacks),
    NULL,
    StackErrnoCallback,
    NULL,
    HwHdapConnectCallback,
    HwProfileStateCallback,
    NULL,
    HwAclDisconnReasonCb,
    HwGetSensorhubDevInfoCb,
    HwSensorhubResetCb,
    HwHiechoIndCallCb,
    HwBluetoothConnCb,
    NULL,
    NULL,
    HwDeviceCallback,
    NULL,
    HwA2dpOffloadStateCallback,
    HwA2dpOffloadCodecConfigCallback,
    HwHiechoDeviceAclEncryptionChangedCb,
    HwReportA2dpAbnormalStatusCb,
#ifdef CONTEXTHUB_BLE_V3
    HwSensorhubCollaborationCb,
#endif
    { NULL },
};

void BluetoothHwInterface::UpdateA2dpOffloadCodecStatus(const RawAddress &addr)
{
    CHECK_AND_RETURN_LOG(bthwInterface_ != nullptr, "bthwInterface_ nullptr");
    A2dpOffloadConfigCallback config;
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(addr);
    CHECK_AND_RETURN_LOG(bthwInterface_->getA2dpOffloadCodecConfig != nullptr, "getA2dpOffloadCodecConfig nullptr");
    bool ret = bthwInterface_->getA2dpOffloadCodecConfig(rawAddr, config);
    CHECK_AND_RETURN_LOG(ret, "get a2dp offload config fail");
    HwA2dpOffloadCodecConfigCallback(&rawAddr, config);
}

bool BluetoothHwInterface::InitBtHwInterface(const bthwif_interface_t *bthwif)
{
    if (!bthwif) {
        HILOGE("bthwif is nullptr");
        return false;
    }
    bt_status_t status = bthwif->Init(&sBluetoothHWCallbacks);
    if (status != BT_STATUS_SUCCESS && status != BT_STATUS_DONE) {
        HILOGE("Failed to init bthwif interface, (%{public}s)", bt_status_text(status).c_str());
        return false;
    }

    bthwInterface_ = bthwif;
    return true;
}

void BluetoothHwInterface::HwSetAdvKey(const RawAddress &addr, const char &irk, const char &hbk, const uint8_t version,
    const char &feature)
{
    CHECK_AND_RETURN_LOG(bthwInterface_ != nullptr, "bthwInterface_ nullptr");
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(addr);
    bthwInterface_->hwSetAdvKey(rawAddr, irk, hbk, version, feature);
}

void BluetoothHwInterface::HwRemoveAdvKey(const RawAddress &addr)
{
    CHECK_AND_RETURN_LOG(bthwInterface_ != nullptr, "bthwInterface_ nullptr");
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(addr);
    bthwInterface_->hwRemoveAdvKey(rawAddr);
}

void BluetoothHwInterface::HwGetAdvIrk(const RawAddress &addr, std::vector<uint8_t>& irk)
{
    CHECK_AND_RETURN_LOG(bthwInterface_ != nullptr, "bthwInterface_ nullptr");
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(addr);
    bthwInterface_->hwGetAdvIrk(rawAddr, irk);
}

bool BluetoothHwInterface::CheckDeviceBonded(const RawAddress &addr)
{
    CHECK_AND_RETURN_LOG_RET(bthwInterface_ != nullptr, false, "bthwInterface_ is null");
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(addr);
    return bthwInterface_->checkRemoteDeviceBonded(rawAddr);
}
}  // namespace bluetooth
}  // namespace OHOS
