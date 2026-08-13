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
#define LOG_TAG "bt_service_connection"
#endif

#include "adapter_manager.h"
#include "bluetooth_common_event_helper.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_def.h"
#include "bt_chr_base.h"
#include "bt_chr_dft_exception.h"
#include "classic_adapter.h"
#include "gavdp/a2dp_service.h"
#include "hfp_ag/hfp_ag_service.h"
#include "interface_profile_manager.h"
#include "remote_device_properties.h"
#include "interface_profile_gatt_client.h"
#include "interface_profile_manager.h"
#include "permission_manager.h"
#include "gatt_client_service.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_hw_interface.h"
#include "hal_util.h"

#include "log.h"
#include "common_util.h"
#include "parameters.h"
#include "bluetooth_state_manager.h"

namespace OHOS {
namespace bluetooth {

const int32_t BT_CONNECT_BR_LINKS_ONE = 1;
const int32_t BT_CONNECT_BR_LINKS_ZERO = 0;
const int32_t MAX_DEFAULT_CONNECTIONS_NUM = BLUETOOTH_MAX_DEFAULT_CONNECTIONS_NUM;
constexpr int32_t MAX_HIGH_PRIORITY_LINK_NUM = 3;
constexpr int32_t RESET_NO_REFUSE_LINK_TIMEOUT_500MS = 500;

constexpr const int32_t GET_DEVICE_STATE_CATEGORY = 0x02;
constexpr const int32_t DEVICE_ACCEPT_CONN = 0x01;
constexpr const int32_t DEVICE_REJECT_CONN_FOR_EARBUD_BUSINESS = 0x02;
constexpr const int32_t DEVICE_REJECT_CONN_FOR_MOBILE_HWACCOUNT_NOT_RIGHT = 0x03;
constexpr const int32_t DEVICE_REJECT_CONN_FOR_AUTO_CONN_SWITCH_CLOSED = 0x04;
constexpr const int32_t DEVICE_REJECT_CONN_FOR_PROHIBIT_BUSINESS_PREEMPT = 0x05;
constexpr const uint8_t CONN_REASON_ECHO_ELN = 33;
constexpr const uint8_t CONN_REASON_ECHO_RESTRICT_LEN = 16;

static const std::string BLUETOOTH_BROKER_CALLING_NAME = "broker";

BluetoothConnectionManager::BluetoothConnectionManager()
{}

BluetoothConnectionManager::~BluetoothConnectionManager()
{}

BluetoothConnectionManager *BluetoothConnectionManager::GetInstance()
{
    static BluetoothConnectionManager instance;
    return &instance;
}

/**
 * Is Allowed the br link to connect
*/
bool BluetoothConnectionManager::IsBrLinkAllowed(std::string address)
{
    if (!ReachingMaxBrLinks(address)) {
        return true;
    }
    std::string unPairedAddr = GetFirstUnPairedBrLink();
    if (unPairedAddr.empty()) {
        HILOGE("unPairedAddr is nullptr.");
        return false;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (!classicAdapter) {
        HILOGE("classicAdapter is nullptr.");
        return false;
    }
    classicAdapter->DisconnectAcl(unPairedAddr);
    return true;
}

/**
 * Allowed the br link connect or disconnect address/unPairedAddr
*/
bool BluetoothConnectionManager::IsBrLinkAllowed(std::string address, std::string &unPairedAddr)
{
    if (!ReachingMaxBrLinks(address)) {
        return true;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (!classicAdapter) {
        HILOGE("classicAdapter is nullptr.");
        return false;
    }
    unPairedAddr = GetFirstUnPairedBrLink();
    if (!unPairedAddr.empty()) {
        HILOGE("Reaching Max Disconnect Acl address: %{public}s", GetEncryptAddr(unPairedAddr).c_str());
        classicAdapter->DisconnectAcl(unPairedAddr);
        return true;
    } else {
        HILOGE("Reaching Max Disconnect Acl address: %{public}s", GetEncryptAddr(address).c_str());
        classicAdapter->DisconnectAcl(address);
        return false;
    }
}

bool BluetoothConnectionManager::DisconnectForDeviceLimit()
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr, false, "serviceMgr null");
    A2dpService* a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    HfpAgService* hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    if (a2dpService == nullptr || hfpAgService == nullptr) {
        HILOGE("a2dpService/hfpAgService is nullptr");
        return false;
    }
    std::list<RawAddress> a2dpConnDevList = a2dpService->GetConnectingOrConnectionDevices();
    std::list<RawAddress> hfpConnDevList = hfpAgService->GetConnectingOrConnectionDevices();
    if (hfpAgService->GetConnectedDeviceNum() < MAX_DEFAULT_CONNECTIONS_NUM) {
        HILOGE("The number of profile connections does not reach the maximum.");
        return true;
    }
    HfpAgSystemInterface& mSystemInterface = HfpAgSystemInterface::GetInstance();
    std::string activeDeviceAddr = hfpAgService->GetActiveDevice();
    std::vector<std::string> brLinks = brLinks_.GetVector();
    for (std::string address : brLinks) {
        if (activeDeviceAddr == address && mSystemInterface.IsInCall() && hfpAgService->IsAudioConnected(address)) {
            HILOGE("disconnectForDeviceLimit: an active SCO device : %{public}s", GetEncryptAddr(address).c_str());
            continue;
        }
        auto a2dpIt = std::find(a2dpConnDevList.begin(), a2dpConnDevList.end(), RawAddress(address));
        auto hfpIt = std::find(hfpConnDevList.begin(), hfpConnDevList.end(), RawAddress(address));
        if (a2dpIt == a2dpConnDevList.end() && hfpIt == hfpConnDevList.end()) {
            HILOGE("hfp and a2dp are disconnected : %{public}s", GetEncryptAddr(address).c_str());
            continue;
        }
        if (a2dpIt != a2dpConnDevList.end()) {
            HILOGE("disconnectForDeviceLimit: A2DP : %{public}s", GetEncryptAddr(address).c_str());
            a2dpService->Disconnect(RawAddress(address));
        }
        if (hfpIt != hfpConnDevList.end()) {
            HILOGE("disconnectForDeviceLimit: HFP : %{public}s", GetEncryptAddr(address).c_str());
            hfpAgService->Disconnect(RawAddress(address));
        }
        return true;
    }
    return false;
}

void BluetoothConnectionManager::RecordAclConnect(std::string address,
    int linkType, int newConnectState, std::string unPairedAddr)
{
    if (newConnectState == CONNECTION_STATE_CONNECTED) {
        AddAclLinks(address, linkType, unPairedAddr);
        BluetoothHelper::BluetoothCommonEventHelper::PublishAclConnectedEvent(address, linkType);
    } else if (newConnectState == CONNECTION_STATE_DISCONNECTED) {
        RemoveAclLinks(address, linkType);
        BluetoothHelper::BluetoothCommonEventHelper::PublishAclDisconnectedEvent(address, linkType);
        BluetoothStateManager::GetInstance()->RemoveDeviceConnectState(address);
    }
}
// Record Ble gatt connection,when acl is established, add ble link to blelinls_
void BluetoothConnectionManager::RecordBleAclConnect(RawAddress address, bool isAllowed)
{
    // native app and system hap record true;
    bleTempRecordLinks_.EnsureInsert(address.GetAddress(), isAllowed);
    std::string callingname = Bluetooth::PermissionManager::GetCallingName();
    HILOGI("callingName(%{public}s); IsNativeApp : %{public}d", callingname.c_str(),
        IsNativeAppOrSystemHap());
}

void BluetoothConnectionManager::AddBleConnectionMaps(std::string address)
{
    bool isAllowed = false;
    bleTempRecordLinks_.Find(address, isAllowed);
    HILOGD("AddBleConnectionMaps isAllowed = %{public}d", isAllowed);
    bleConnection_.EnsureInsert(address, isAllowed);
}

bool BluetoothConnectionManager::isBleAclAllowed(RawAddress address)
{
    bool isAllowed = false;
    bleConnection_.Find(address.GetAddress(), isAllowed);
    return isAllowed;
}

void BluetoothConnectionManager::UpdateVirtualAutoConnCap(const std::string &address, int autoConnRejectReason)
{
    HILOGI("updateVirtualAutoConnCap, autoConnRejectReason=%{public}d", autoConnRejectReason);
    if (autoConnRejectReason != DEVICE_REJECT_CONN_FOR_AUTO_CONN_SWITCH_CLOSED) {
        return;
    }
    BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(address, false);
}

void BluetoothConnectionManager::ClearBleConnect()
{
    bleConnection_.Clear();
    bleTempRecordLinks_.Clear();
}

bool BluetoothConnectionManager::IsNativeAppOrSystemHap()
{
    return Bluetooth::PermissionManager::IsNativeCaller();
}

void BluetoothConnectionManager::SendAntennaStatusMsg(bool isFixed)
{
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("Failed to open the Bluetooth module");
        return;
    }
    btInterface->send_antenna_status_msg(isFixed);
}

void BluetoothConnectionManager::AddAclLinks(std::string address, int linkType, std::string unPairedAddr)
{
    // reach max acl num links, disconnect it, when recive acl connect, update mBrLinks/mBLELinks
    HILOGI("AddAclLinks address: %{public}s  linkeType is %{public}d", GetEncryptAddr(address).c_str(), linkType);
    if (linkType == LINK_TYPE_BREDR) {
        if (ReachingMaxBrLinks(address)) {
            if (!unPairedAddr.empty()) {
                brLinks_.Erase(unPairedAddr);
                brLinks_.PushNoDuplicate(address);
            }
        } else {
            // no reaching max or left 1 all need record address, ensure the order
            brLinks_.PushNoDuplicate(address);
        }
    } else if (linkType == LINK_TYPE_LE) {
        bleLinks_.PushNoDuplicate(address);
        AddBleConnectionMaps(address);
        SendAntennaStatusMsg(true);
        std::string btChipInfoJsonStr = BtChrGetChipInfoJsonStr(address, CHR_BLE_DISCONNECT);
        BtChrEventWriteStr(CHR_BLE_DISCONNECT, address, "CONNECTBTCHIPINFO", btChipInfoJsonStr);
    }
    if (AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        return;
    }
    if ((brLinks_.Size() + bleLinks_.Size()) == BT_CONNECT_BR_LINKS_ONE) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishDeviceConnectionStateEvent(address,
            static_cast<int32_t>(BTConnectState::CONNECTED));
    }
}

void BluetoothConnectionManager::RemoveAclLinks(std::string address, int linkType)
{
    HILOGI("RemoveAclLinks address: %{public}s  linkeType is %{public}d", GetEncryptAddr(address).c_str(), linkType);
    // when recive acl disconnect, update mBrLinks/mBleLinks
    if (linkType == LINK_TYPE_BREDR) {
        brLinks_.Erase(address);
    } else if (linkType == LINK_TYPE_LE) {
        bleLinks_.Erase(address);
        if (bleLinks_.Size() == 0) {
            SendAntennaStatusMsg(false);
        }
        std::string btChipInfoJsonStr = BtChrGetChipInfoJsonStr(address, CHR_BLE_DISCONNECT);
        BtChrEventWriteStr(CHR_BLE_DISCONNECT, address, "DISCONNECTBTCHIPINFO", btChipInfoJsonStr);
    } else {
        HILOGE("unknow linkType.");
    }
    highPriorityLinks_.Erase(address);
    if (AdapterManager::GetInstance()->IsBluetoothRestricted()) {
        return;
    }
    if ((brLinks_.Size() + bleLinks_.Size()) == BT_CONNECT_BR_LINKS_ZERO) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishDeviceConnectionStateEvent(address,
            static_cast<int32_t>(BTConnectState::DISCONNECTED));
    }
}

bool BluetoothConnectionManager::RemoveAllBrAclLinks()
{
    HILOGE("[ConnectionManager] RemoveAllBrAclLinks, brlinks size : %{public}zu; blelinks size : %{public}zu",
        brLinks_.Size(), bleLinks_.Size());
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (!classicAdapter) {
        HILOGE("classicAdapter is nullptr.");
        return false;
    }
    std::vector<std::string> brLinks = brLinks_.GetVector();
    for (const std::string &address : brLinks) {
        classicAdapter->DisconnectAcl(address);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, address, "DISCONNECTREASON",
            RESTRICIT_BLUETOOTH_DISCONNECT_BR_LINKS);
#ifdef BLUETOOTH_WATCH_ENABLE
        BtChrDftEventWriteInt(CHR_BT_WATCH_CONNECT, address, "DISCONNECTREASON",
            RESTRICIT_BLUETOOTH_DISCONNECT_BR_LINKS);
#endif
    }
    brLinks_.Clear();
    highPriorityLinks_.Clear();
    return true;
}

void BluetoothConnectionManager::RemoveAllAclLinks()
{
    HILOGI("[ConnectionManager] RemoveAllAclLinks, brlinks size : %{public}zu; blelinks size : %{public}zu",
        brLinks_.Size(), bleLinks_.Size());
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG(classicAdapter != nullptr, "classicAdapter is nullptr.");
    std::vector<std::string> brLinks = brLinks_.GetVector();
    for (const std::string &address : brLinks) {
        classicAdapter->DisconnectAcl(address);
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, address, "DISCONNECTREASON",
            DISABLE_BLUETOOTH_DISCONNECT_BR_LINKS);
        BtChrEventWriteInt(CHR_BT_WATCH_SPP_DISCONNECT, address, "DISCONNECTSCENE", CHR_DISABLE_BT);
#ifdef BLUETOOTH_WATCH_ENABLE
        BtChrDftEventWriteInt(CHR_BT_WATCH_CONNECT, address, "DISCONNECTREASON",
            DISABLE_BLUETOOTH_DISCONNECT_BR_LINKS);
#endif
    }
    brLinks_.Clear();
    std::vector<std::string> bleLinks = bleLinks_.GetVector();
    for (const std::string &address : bleLinks) {
        classicAdapter->DisconnectAcl(address);
    }
    bleLinks_.Clear();
    highPriorityLinks_.Clear();
}

void BluetoothConnectionManager::OnGetConnReasonEchoRsp(const std::string &address, int type, int typeval)
{
    if (type == GET_DEVICE_STATE_CATEGORY) {
        if (typeval == DEVICE_ACCEPT_CONN) {
            HILOGI("onGetConnReasonEchoRsp: remote device accept the connection request");
        } else {
            UpdateVirtualAutoConnCap(address, typeval);
            BtChrBtExcpEvent(address, BTOPT_MULTCONNECT_AUTOCONNECT_FAIL, typeval);
            const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
            CHECK_AND_RETURN_LOG(bluetoothHwSrcInterface != nullptr, "interface nullptr");
            BLUEDROID::RawAddress dstAddr;
            BLUEDROID::RawAddress::FromString(address, dstAddr);
            bluetoothHwSrcInterface->hwBtifHfpQueueRemove(dstAddr);
            bluetoothHwSrcInterface->hwBtifA2dpQueueRemove(dstAddr);
            bluetoothHwSrcInterface->hwBtsndHcicDisconnect(dstAddr);
        }
    } else {
        HILOGI("onGetConnReasonEchoRsp: continue authentication encryption and profile connection");
    }
}

void BluetoothConnectionManager::SetVirtualAutoConnectType(const RawAddress &address, int connType, int businessType)
{
    auto *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bthwif != nullptr, "bthwif nullptr");
    std::vector<uint8_t> connReasonData(CONN_REASON_ECHO_ELN, 0);
#ifdef BLUETOOTH_OS_ACCOUNT
    uint8_t hashAccount[HW_ACCOUNT_HASH_LEN] = {0};
    bool ret = BluetoothHwInterface::GetInstance()->GetSha256EncryptHwHashAccount(hashAccount, HW_ACCOUNT_HASH_LEN);
    if (ret) {
        std::copy(std::begin(hashAccount) + CONN_REASON_ECHO_RESTRICT_LEN, std::end(hashAccount),
            connReasonData.begin() + CONN_REASON_ECHO_RESTRICT_LEN);
    } else {
        HILOGW("Get hash account failed");
    }
#endif
    connReasonData[CONN_REASON_ECHO_ELN - 1] = businessType;
    HILOGI("SetVirtualAutoConnectType, connType:%{public}x, businessType:%{public}x", connType, businessType);

    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpService, "a2dpService is nullptr.");
    a2dpService->SendDeviceSelection(address, USE_A2DP, NO_CHANGE, USER_SELECT);

    bthwif->hwSetConnReasonFlag(ServiceUtil::AddrToBluedroid(address), connType, connReasonData.data(),
        connReasonData.size());
}

bool BluetoothConnectionManager::ReachingMaxBrLinks(std::string address)
{
    if (address.empty() || brLinks_.IsExist(address)) {
        HILOGW("the device %{public}s acl exists.", GetEncryptAddr(address).c_str());
        return false;
    }
    if (brLinks_.Size() >= BLUETOOTH_MAX_BR_LINKS) {
        HILOGI("the device %{public}s ReachingMaxBrLinks.", GetEncryptAddr(address).c_str());
        std::vector<std::string> brLinks = brLinks_.GetVector();
        int brLinkId = 0;
        for (auto it = brLinks.begin(); it != brLinks.end(); it++, brLinkId++) {
            HILOGI("brLinks[%{public}d]: %{public}s", brLinkId, GetEncryptAddr(*it).c_str());
        }
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, address, "DISCONNECTREASON", MAX_BR_LINKS_CHR);
#ifdef BLUETOOTH_WATCH_ENABLE
        BtChrDftEventWriteInt(CHR_BT_WATCH_CONNECT, address, "DISCONNECTREASON", MAX_BR_LINKS_CHR);
#endif
        return true;
    }
    return false;
}

bool BluetoothConnectionManager::IsConnectTooFrequently(ConnectRecord connectRecord)
{
    int64_t firstConnectTime = connectRecord.firstConnectTime;
    int64_t currentTime = connectRecord.timestamp;
    int64_t lastConnectedTime = connectRecord.lastConnectedTime;
    std::uint32_t tokenId = connectRecord.tokenId;
    if ((currentTime - firstConnectTime) <= EXCESSIVE_CONNECTING_NO_CONTROL_TIMER) {
        // No control
        return false;
    } else {
        if (!connectTokenList_.IsExist(tokenId)) {
            // ALLOW_CONNECT_TIMER first connect
            HILOGI("allow first time tokenId = %{public}d", tokenId);
            connectTokenList_.Push(tokenId);
            return false;
        }
        if ((currentTime - lastConnectedTime) > EXCESSIVE_CONNECTING_ALLOW_CONNECT_TIMER) {
            HILOGI("allow one time per 60s currentTime:%{public}d lastConnectedTime:%{public}d",
                currentTime, lastConnectedTime);
            return false;
        }
        //Be control
        HILOGI("be control currentTime:%{public}d firstConnectTime:%{public}d lastConnectedTime:%{public}d ",
            currentTime, firstConnectTime, lastConnectedTime);
        return true;
    }
    return false;
}

bool IsNearbyDebug()
{
    if (system::GetParameter("ro.bluetooth.connectioncontrol.debug.on", "0") != "0") {
        HILOGI("[SocketService::Connect] nearby debug mode.");
        return true;
    }
    return false;
}

bool IsProfileConnecting(std::string address)
{
    RawAddress device = RawAddress(address);
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG_RET(serviceMgr != nullptr, false, "serviceMgr null");
    HfpAgService* hfpAgService = static_cast<HfpAgService *>(serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG));
    if (hfpAgService && hfpAgService->GetDeviceState(device) ==
        static_cast<int>(BTConnectState::CONNECTING)) {
        return true;
    }
    A2dpService* a2dpService = static_cast<A2dpService *>(serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC));
    if (a2dpService && a2dpService->GetDeviceState(device) ==
        static_cast<int>(BTConnectState::CONNECTING)) {
        return true;
    }
    return false;
}

bool BluetoothConnectionManager::ShouldRefuseSocketConnectEX(const std::string address, const Uuid &uuid,
    std::string bundle)
{
    if (brLinks_.IsExist(address)) {
        return false;
    }
    if (ReachingMaxBrLinks(address)) {
        HILOGE("[SocketService::Connect] Failed connect acl link because of MAX links.");
        BtChrSocketExcpEvent(address, uuid.ToString(), CLIENT_CONNECT_REFUSE, SOCKET_SUBERROR_LINK_LIMIT, bundle);
        return true;
    }
    if (IsInCloudBondingState(address)) {
        HILOGI("[SocketService::Connect] in cloud bonding state.");
        return false;
    }
    if (bundle == BLUETOOTH_BROKER_CALLING_NAME) {
        HILOGI("No restrictions in the broker scenario.");
        return false;
    }
    if (IsProfileConnecting(address)) {
        HILOGE("[SocketService::Connect] Failed connect acl link because of profile is connecting.");
        BtChrSocketExcpEvent(address, uuid.ToString(),
            CLIENT_CONNECT_REFUSE, SOCKET_SUBERROR_PROFILE_CONNECTING, bundle);
        return true;
    }
    return false;
}

bool BluetoothConnectionManager::ShouldRefuseSocketConnectTooFrequently(std::uint32_t tokenId,
    int64_t currentTime, std::string address)
{
    if (brLinks_.IsExist(address) || IsNearbyDebug()) {
        return false;
    }
    // if current connection is set no refuse, it should not be controlled
    if (ShouldPermitWhitelistFrequentConnect(address)) {
        return false;
    }
    // find exist connectRecord or create new connectRecord
    bool isTokenConnectRecordExist = false;
    int64_t lastConnectTime = 0;
    int64_t firstConnectTime = 0;
    UpdateConnectRecord(isTokenConnectRecordExist, lastConnectTime, firstConnectTime, tokenId, currentTime);
    HILOGI("ShouldRefuseSocketConnectTooFrequently tokenId %{public}d currentTime %{public}d address:%{public}s"
        "isTokenConnectRecordExist %{public}d firstConnectTime:%{public}d lastConnectTime:%{public}d ", tokenId,
        currentTime, GetEncryptAddr(address).c_str(), isTokenConnectRecordExist, firstConnectTime, lastConnectTime);
    // in this interval clear control
    if ((currentTime - firstConnectTime) > EXCESSIVE_CONNECTING_NO_CONTROL_TIMER &&
        (currentTime - lastConnectTime) > EXCESSIVE_CONNECTING_EXIST_CONTROL_TIMER) {
        // reset control
        HILOGI("more 300s than firsttime and more 120s than lasttime , reset control.");
        RemoveConnectRecords(tokenId);
        return false;
    }
    // if it needs be control
    {
        std::lock_guard<std::mutex> lock(connectRecordListMutex_);
        for (auto it = connectRecordList_.begin(); it != connectRecordList_.end(); ++it) {
            if (tokenId != it->tokenId) {
                continue;
            }
            it->timestamp = currentTime;
            if (IsConnectTooFrequently(*it)) {
                it->lastConnectTime = currentTime;
                return true;
            } else {
                it->lastConnectTime = currentTime;
                it->lastConnectedTime = currentTime;
                return false;
            }
        }
    }
    return false;
}

void BluetoothConnectionManager::UpdateConnectRecord(bool &isRecordExist,
    int64_t &lastConnectTime, int64_t &firstConnectTime, std::uint32_t tokenId, int64_t currentTime)
{
    std::lock_guard<std::mutex> lock(connectRecordListMutex_);
    for (auto it = connectRecordList_.begin(); it != connectRecordList_.end(); ++it) {
        if (tokenId == it->tokenId) {
            isRecordExist = true;
            lastConnectTime = it->lastConnectTime;
            firstConnectTime = it->firstConnectTime;
            break;
        }
    }
    if (!isRecordExist) {
        ConnectRecord connectRecord(tokenId, currentTime);
        lastConnectTime = currentTime;
        firstConnectTime = currentTime;
        connectRecordList_.emplace_back(connectRecord);
    }
}

void BluetoothConnectionManager::RemoveConnectRecords(std::uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(connectRecordListMutex_);
    auto removeConnectRecord = connectRecordList_.end();
    for (auto it = connectRecordList_.begin(); it != connectRecordList_.end(); ++it) {
        if (tokenId == it->tokenId) {
            removeConnectRecord = it;
            break;
        }
    }
    if (removeConnectRecord != connectRecordList_.end()) {
        connectRecordList_.erase(removeConnectRecord);
    }
    connectTokenList_.Erase(tokenId);
}

std::string BluetoothConnectionManager::GetFirstUnPairedBrLink()
{
    // disconnect unpaired acl address eg. socket unsafe connect
    std::string unPairedAddr;
    std::vector<std::string> brLinks = brLinks_.GetVector();
    for (const std::string &address : brLinks) {
        std::shared_ptr<BluetoothDevice> device = RemoteDeviceProperties::GetInstance()->
            FindRemoteDevice(RawAddress(address));
        if (device == nullptr || highPriorityLinks_.IsExist(address)) {
            continue;
        }
        if (device->GetPairedStatus() == PAIR_NONE) {
            unPairedAddr = address;
            break;
        }
    }
    if (!unPairedAddr.empty()) {
        HILOGW("GetFirstUnPairedBrLink, address = %{public}s", GetEncryptAddr(unPairedAddr).c_str());
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT,
            unPairedAddr, "DISCONNECTREASON", FIRST_UNPAIRED_BR_LINK_CHR);
    } else {
        HILOGW("No unpaired br connections");
    }
    return unPairedAddr;
}

void BluetoothConnectionManager::CheckNeedReportConnectedDevice()
{
    std::string address;
    if (bleLinks_.Front(address)) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishDeviceConnectionStateEvent(address,
            static_cast<int32_t>(BTConnectState::CONNECTED));
    }
}

bool BluetoothConnectionManager::IsInCloudBondingState(const std::string &address)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    return (classicAdapter->GetCloudBondState(RawAddress(address)) == CLOUD_BOND_BONDING);
}

bool BluetoothConnectionManager::IsBrConnected(std::string address)
{
    return brLinks_.IsExist(address);
}

bool BluetoothConnectionManager::IsBleConnected(std::string address)
{
    return bleLinks_.IsExist(address);
}

int32_t BluetoothConnectionManager::SetConnectionPriority(const std::string &address, int priority)
{
    HILOGI("Set connection priority, addr: %{public}s, priority: %{public}d", GET_ENCRYPT_STR_ADDR(address), priority);
    switch (priority) {
        case Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_DEFAULT:
            highPriorityLinks_.Erase(address);
            return Bluetooth::BT_NO_ERROR;
        case Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NON_PREEMPTIBLE:
        {
            if (highPriorityLinks_.IsExist(address)) {
                return Bluetooth::BT_NO_ERROR;
            }
            if (highPriorityLinks_.Size() < MAX_HIGH_PRIORITY_LINK_NUM) {
                highPriorityLinks_.Push(address);
            } else {
                std::string firstLink;
                highPriorityLinks_.Front(firstLink);
                highPriorityLinks_.Erase(firstLink);
                highPriorityLinks_.Push(address);
            }
            return Bluetooth::BT_NO_ERROR;
        }
        case Bluetooth::BtLinkPriority::BT_LINK_PRIORITY_NO_REFUSE_FREQUENT_CONNECT:
        {
            std::lock_guard<std::mutex> lock(noRefuseLinkMutex_);
            noRefuseLinkAddr_ = address;
            noRefuseLinkTimer_ = std::make_shared<utility::Timer>([this] {
                {
                    std::lock_guard<std::mutex> lock(noRefuseLinkMutex_);
                    HILOGI("Reset noRefuseLinkAddr_: %{public}s", GET_ENCRYPT_STR_ADDR(this->noRefuseLinkAddr_));
                    this->noRefuseLinkAddr_ = "";
                }
            });
            this->noRefuseLinkTimer_->Start(RESET_NO_REFUSE_LINK_TIMEOUT_500MS);
            return Bluetooth::BT_NO_ERROR;
        }
        default:
            HILOGE("Invalid connection priority, addr: %{public}s, priority: %{public}d",
                GET_ENCRYPT_STR_ADDR(address), priority);
            break;
    }
    return Bluetooth::BT_ERR_INVALID_PARAM;
}

bool BluetoothConnectionManager::ShouldPermitWhitelistFrequentConnect(const std::string &address)
{
    std::lock_guard<std::mutex> lock(noRefuseLinkMutex_);
    if (noRefuseLinkAddr_ == address) {
        HILOGI("should permit whitelist link: %{public}s", GET_ENCRYPT_STR_ADDR(address));
        noRefuseLinkAddr_ = "";
        if (noRefuseLinkTimer_ && noRefuseLinkTimer_->IsStarted()) {
            noRefuseLinkTimer_->Stop();
        }
        return true;
    }
    return false;
}
} //namespace bluetooth
} //namespace OHOS
