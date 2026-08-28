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
#define LOG_TAG "bt_service_resource_manager"
#endif

#include "bluetooth_resource_manager.h"
#include "base_observer_list.h"
#include "bt_chr_ue_manager.h"
#include "raw_address.h"
#include "safe_map.h"
#include "log.h"
#include "log_utils.h"
#include "ble_defs.h"
#include "control_intercept_plugin.h"
#include "bluetooth_channel_rssi_manager.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_errorcode.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

const std::string SLE_BLUETOOTH_DUAL_MAC_ENABLE = "const.bluetooth.dual_mac.enable";

IBluetoothResourceManager *IBluetoothResourceManager::GetInstance()
{
    return BluetoothResourceManager::GetInstance();
}

struct BluetoothResourceManager::impl {
public:
    impl();
    ~impl();

    void InitiateBleScanEnhanceModeMapping();

    std::unique_ptr<BaseObserverList<IResourceManagerObserver>> resourceManagerObserver_ = nullptr;
    SafeMap<uint8_t, std::string> sensingEventUeCodes_;
    SafeMap<int, BleScanEnhanceModeAction> bleScanEnhanceModeMapping_;
    BleScanEnhanceModeImpl enhanceMode_;
    bool IsUnittestModeOn_ = false;
};

BluetoothResourceManager::impl::impl()
{
    resourceManagerObserver_ = std::make_unique<BaseObserverList<IResourceManagerObserver>>();
    InitiateBleScanEnhanceModeMapping();

    // mapping different sensing event to UE scene code
    sensingEventUeCodes_.EnsureInsert(INVALID_SENSING_EVENT, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SPP_CLIENT_CONNECT_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SPP_SERVER_CONNECT_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SPP_PORT_CLOSE, CHR_UE_ACL_DISCONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SPP_SERVER_CLOSE, CHR_UE_ACL_DISCONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SPP_SERVER_LISTEN_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_SERVER_REGISTER_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_SERVER_UNREGISTER_DONE, CHR_UE_ACL_DISCONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_CLIENT_CONNECT_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_CLIENT_CONNECT_CLOSE, CHR_UE_ACL_DISCONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_SERVER_CONNECT_DONE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_SERVER_CONNECT_CLOSE, CHR_UE_ACL_DISCONN_CMP);
    sensingEventUeCodes_.EnsureInsert(SOCKET_REGISTER_APPLICATION_PKGNAME, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(GATT_REGISTER_APPLICATION_PKGNAME, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(BLE_CONN_INTERVAL_UPDATE, CHR_UE_ACL_CONN_CMP);
    sensingEventUeCodes_.EnsureInsert(BLE_SCAN_STARTED_EVENT, CHR_UE_BLE_START_SCAN);
    sensingEventUeCodes_.EnsureInsert(BLE_SCAN_STOPPED_EVENT, CHR_UE_BLE_STOP_SCAN);
    sensingEventUeCodes_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_STARTED_EVENT, CHR_UE_BLE_START_SCAN);
    sensingEventUeCodes_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_STOPPED_EVENT, CHR_UE_BLE_STOP_SCAN);
}

BluetoothResourceManager::impl::~impl()
{
    resourceManagerObserver_ = nullptr;
    sensingEventUeCodes_.Clear();
    bleScanEnhanceModeMapping_.Clear();
}

void BluetoothResourceManager::impl::InitiateBleScanEnhanceModeMapping()
{
    bleScanEnhanceModeMapping_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_BALANCED, BleScanEnhanceModeAction(true, false,
        BrTransControlSetting(BR_TRANS_CONTROL_P10_10_100_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_P10_10_100_PENDING_INTERVAL_MS,
        BR_TRANS_CONTROL_P10_10_100_SLEEPING_INTERVAL_MS)));
    bleScanEnhanceModeMapping_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_MEDIUM, BleScanEnhanceModeAction(true, false,
        BrTransControlSetting(BR_TRANS_CONTROL_P25_25_100_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_P25_25_100_PENDING_INTERVAL_MS,
        BR_TRANS_CONTROL_P25_25_100_SLEEPING_INTERVAL_MS)));
    bleScanEnhanceModeMapping_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_FAST, BleScanEnhanceModeAction(true, false,
        BrTransControlSetting(BR_TRANS_CONTROL_P50_50_100_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_P50_50_100_PENDING_INTERVAL_MS,
        BR_TRANS_CONTROL_P50_50_100_SLEEPING_INTERVAL_MS)));
    bleScanEnhanceModeMapping_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_ULTRA_FAST, BleScanEnhanceModeAction(true, true,
        BrTransControlSetting(BR_TRANS_CONTROL_P50_50_100_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_P50_50_100_PENDING_INTERVAL_MS,
        BR_TRANS_CONTROL_P50_50_100_SLEEPING_INTERVAL_MS)));
    bleScanEnhanceModeMapping_.EnsureInsert(BLE_SCAN_ENHANCE_MODE_INVALID, BleScanEnhanceModeAction(false, false,
        BrTransControlSetting(BR_TRANS_CONTROL_DEFAULT_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_DEFAULT_PENDING_INTERVAL_MS,
        BR_TRANS_CONTROL_DEFAULT_SLEEPING_INTERVAL_MS)));
}

BluetoothResourceManager::BluetoothResourceManager() : pimpl(std::make_unique<BluetoothResourceManager::impl>())
{
    HILOGI("BluetoothResourceManager:Create");
    enhanceModeTimer_ = std::make_shared<utility::Timer>([this]() {
        this->StopBleScanEnhanceMode();
    });
}

BluetoothResourceManager::~BluetoothResourceManager()
{
    if (enhanceModeTimer_ != nullptr) {
        enhanceModeTimer_->Stop();
        enhanceModeTimer_ = nullptr;
    }
}

const BthwifInterface* BluetoothResourceManager::GetBtHwInterface()
{
    if (bthwInterface_ != nullptr) {
        return bthwInterface_;
    }
    bthwInterface_ = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    return bthwInterface_;
}

void BluetoothResourceManager::RegisterObserver(std::shared_ptr<IResourceManagerObserver> observer)
{
    if (pimpl != nullptr && pimpl->resourceManagerObserver_ != nullptr) {
        pimpl->resourceManagerObserver_->Register(*observer);
    }
}

void BluetoothResourceManager::DeregisterObserver(std::shared_ptr<IResourceManagerObserver> observer)
{
    if (pimpl != nullptr && pimpl->resourceManagerObserver_ != nullptr) {
        pimpl->resourceManagerObserver_->Deregister(*observer);
    }
}

bool BluetoothResourceManager::IsBleScanEnhanceModeSupported()
{
    if (pimpl != nullptr && pimpl->IsUnittestModeOn_) {
        return true;
    }
    uint32_t dualMacCfg = static_cast<uint32_t>(
        OHOS::system::GetIntParameter(SLE_BLUETOOTH_DUAL_MAC_ENABLE, DUAL_MAC_DISABLE)
    );
    if (dualMacCfg & DUAL_MAC_ALL_SWITCH_BLE_ADV_SCAN) {
        HILOGI("ble scan switch to mac1, no bluetooth coexist scene, no need to enable ble scan enhance mode");
        return false;
    }
    return true;
}

bool BluetoothResourceManager::IsValidBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode)
{
    if (enhanceMode.timeout < 0 || enhanceMode.timeout > BLE_SCAN_ENHANCE_MODE_MAX_TIMEOUT_MS) {
        HILOGE("Invalid enhanceMode timeout: %{public}ld", enhanceMode.timeout);
        return false;
    }
    if (enhanceMode.mode < 0 || enhanceMode.mode > BLE_SCAN_ENHANCE_MODE_INVALID) {
        HILOGE("Invalid mode: %{public}d", enhanceMode.mode);
        return false;
    }
    if (enhanceMode.isEnable && (enhanceMode.timeout == 0 || enhanceMode.mode == BLE_SCAN_ENHANCE_MODE_INVALID)) {
        HILOGE("Invalid enhanceMode timeout: %{public}ld, mode: %{public}d", enhanceMode.timeout, enhanceMode.mode);
        return false;
    }
    return true;
}

void BluetoothResourceManager::UpdateBleScanEnhanceModeTimer(int64_t timeout)
{
    if (enhanceModeTimer_ == nullptr) {
        enhanceModeTimer_ = std::make_shared<utility::Timer>([this]() {
            this->StopBleScanEnhanceMode();
        });
    }
    enhanceModeTimer_->Stop();
    enhanceModeTimer_->Start(timeout);
}

int BluetoothResourceManager::StartBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode)
{
    const BthwifInterface* bthwifInterface = GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(pimpl != nullptr, BT_ERR_INTERNAL_ERROR, "pimpl is nullptr");
    if (!IsValidBleScanEnhanceMode(enhanceMode)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    BleScanEnhanceModeAction action;
    if (!pimpl->bleScanEnhanceModeMapping_.Find(enhanceMode.mode, action)) {
        HILOGE("error, fail to find enhance mode:%{public}d", enhanceMode.mode);
        return BT_ERR_INTERNAL_ERROR;
    }

    HILOGI("Start ble scan enhance mode, mode: %{public}d, timeout: %{public}ld", enhanceMode.mode,
        enhanceMode.timeout);
    pimpl->enhanceMode_ = enhanceMode;
    UpdateBleScanEnhanceModeTimer(enhanceMode.timeout);
    CHECK_AND_RETURN_LOG_RET(bthwifInterface != nullptr, BT_ERR_INTERNAL_ERROR, "bthwifInterface is nullptr");
    if (action.isAudioRateSpeedDown) {
        bthwifInterface->hwAdjustTwsBitRateInBleScanEnhanceMode(true);
    }
    if (action.isBrLinkSniffMode) {
        std::vector<std::string> brLinks = BluetoothConnectionManager::GetInstance()->brLinks_.GetVector();
        for (auto it = brLinks.begin(); it != brLinks.end(); it++) {
            bthwifInterface->hwSetBrLinkSniffMode(true, ServiceUtil::AddrToStack(RawAddress(*it)),
                BR_SNIFF_MODE_250_MS);
        }
    }
    bthwifInterface->hwSetBrTransControl(true, action.brTransControlSetting.workingInterval,
        action.brTransControlSetting.pendingInterval, action.brTransControlSetting.sleepingInterval);
    return BT_NO_ERROR;
}

int BluetoothResourceManager::StopBleScanEnhanceMode()
{
    const BthwifInterface* bthwifInterface = GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(pimpl != nullptr, BT_ERR_INTERNAL_ERROR, "pimpl is nullptr");
    CHECK_AND_RETURN_LOG_RET(bthwifInterface != nullptr, BT_ERR_INTERNAL_ERROR, "bthwifInterface is nullptr");
    HILOGI("Disable ble scan enahnce mode.");
    enhanceModeTimer_->Stop();
    pimpl->enhanceMode_ = BleScanEnhanceModeImpl();
    bthwifInterface->hwAdjustTwsBitRateInBleScanEnhanceMode(false);
    bthwifInterface->hwSetBrTransControl(false, BR_TRANS_CONTROL_DEFAULT_WORKING_INTERVAL_MS,
        BR_TRANS_CONTROL_DEFAULT_PENDING_INTERVAL_MS, BR_TRANS_CONTROL_DEFAULT_SLEEPING_INTERVAL_MS);
    std::vector<std::string> brLinks = BluetoothConnectionManager::GetInstance()->brLinks_.GetVector();
    for (auto it = brLinks.begin(); it != brLinks.end(); it++) {
        bthwifInterface->hwSetBrLinkSniffMode(false, ServiceUtil::AddrToStack(RawAddress(*it)),
            BR_SNIFF_MODE_INVALID);
    }
    return BT_NO_ERROR;
}

int BluetoothResourceManager::SetBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode)
{
    const BthwifInterface* bthwifInterface = GetBtHwInterface();
    CHECK_AND_RETURN_LOG_RET(pimpl != nullptr, BT_ERR_INTERNAL_ERROR, "pimpl is nullptr");
    if (!IsValidBleScanEnhanceMode(enhanceMode)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    SendBleScanEnhanceModeEvent(enhanceMode);
    if (!IsBleScanEnhanceModeSupported()) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(bthwifInterface != nullptr, BT_ERR_INTERNAL_ERROR, "bthwifInterface is nullptr");
    GetBtHwInterface()->hwSetBleScanEnhanceModeStatus(enhanceMode.isEnable);
    if (!enhanceMode.isEnable) {
        return StopBleScanEnhanceMode();
    }
    // Enable ble scan enhance mode
    if (pimpl->enhanceMode_.isEnable) {
        HILOGI("Repeat starting ble scan enhance mode, current mode: %{public}d, incoming mode: %{public}d",
            pimpl->enhanceMode_.mode, enhanceMode.mode);
        if (pimpl->enhanceMode_.mode == enhanceMode.mode) {
            pimpl->enhanceMode_ = enhanceMode;
            UpdateBleScanEnhanceModeTimer(enhanceMode.timeout);
            return BT_NO_ERROR;
        }
        StopBleScanEnhanceMode();
    }
    return StartBleScanEnhanceMode(enhanceMode);
}

void BluetoothResourceManager::SendSensingStateChanged(uint8_t eventId, const SensingInfo &info)
{
    HILOGD("eventId: %{public}d", eventId);
    if (pimpl == nullptr || pimpl->resourceManagerObserver_ == nullptr) {
        HILOGE("pimpl or resourceManagerObserver_ is nullptr");
        return;
    }

    SensingInfo tempInfo = info;
    if (eventId == SPP_PORT_CLOSE) {
        GetChannelRssiSensingData(tempInfo);
    }
    GetTransRateData(tempInfo);
    std::string ueSceneCode;
    if (pimpl->sensingEventUeCodes_.Find(eventId, ueSceneCode)) {
        RawAddress addr;
        addr.SetAddress(tempInfo.addr_);
        BtChrUeManager::GetInstance()->WriteCommonUe(ueSceneCode, addr, UE_COMMON_SCENE_CASE4, eventId);
    }
    pimpl->resourceManagerObserver_->ForEach([eventId, tempInfo](IResourceManagerObserver &observer) {
        observer.OnSensingStateChanged(eventId, tempInfo);
    });
}

void BluetoothResourceManager::SendBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result)
{
    HILOGD("eventId: %{public}d, result: %{public}d", eventId, result);
    if (pimpl == nullptr || pimpl->resourceManagerObserver_ == nullptr) {
        HILOGE("pimpl or resourceManagerObserver_ is nullptr");
        return;
    }
    pimpl->resourceManagerObserver_->ForEach([eventId, info, &result](IResourceManagerObserver &observer) {
        observer.OnBluetoothResourceDecision(eventId, info, result);
    });
}

static bool StreamToUint8(const std::string &advPayload, uint8_t &data, int &index)
{
    if (index >= static_cast<int>(advPayload.length()) || index < 0) {
        return false;
    }
    data = static_cast<uint8_t>(advPayload[index]);
    ++index;
    return true;
}

static bool StreamToUint16(const std::string &advPayload, uint16_t &data, int &index)
{
    if (index >= static_cast<int>(advPayload.length()) - 1 || index < 0) {
        return false;
    }
    data = static_cast<uint16_t>(advPayload[index]) + (static_cast<uint16_t>(advPayload[index + 1]) << ONE_OCTET_BITS);
    index += ADV_2_OCTS_LENGTH;
    return true;
}

static bool GetBusinessType(const std::string &advPayload, uint16_t uuid, std::string &businessType, int &index)
{
    if (uuid == ADV_UUID_FDEE) {
        uint8_t version = 0;
        CHECK_AND_RETURN_LOG_RET(StreamToUint8(advPayload, version, index), false, "failed to parse version");
        CHECK_AND_RETURN_LOG_RET(index < static_cast<int>(advPayload.length()) - 1, false,
            "failed to parse businesstype");
        businessType = advPayload.substr(index, ADV_2_OCTS_LENGTH);
        return true;
    }
    if (uuid == ADV_UUID_FE35 || uuid == ADV_MANUFACTUREID) {
        CHECK_AND_RETURN_LOG_RET(index < static_cast<int>(advPayload.length()), false,
            "failed to parse businesstype");
        businessType = advPayload.substr(index, 1);
        return true;
    }
    HILOGE("not supported advertiser");
    return false;
}

static bool ParseAdvPayload(const std::string &advPayload, std::string &uuid, std::string &businessType)
{
    int index = 0;
    uint8_t flagDataLen = 0;
    CHECK_AND_RETURN_LOG_RET(StreamToUint8(advPayload, flagDataLen, index), false, "failed to parse flagDataLen");
    uint8_t flagAdType = 0;
    CHECK_AND_RETURN_LOG_RET(StreamToUint8(advPayload, flagAdType, index), false, "failed to parse flagAdType");
    if (flagAdType == FLAG_ADTYPE) {
        index += flagDataLen - 1;
        uint8_t dataLen = 0;
        CHECK_AND_RETURN_LOG_RET(StreamToUint8(advPayload, dataLen, index), false, "failed to parse dataLen");
        uint8_t adType = 0;
        CHECK_AND_RETURN_LOG_RET(StreamToUint8(advPayload, adType, index), false, "failed to parse adType");
        CHECK_AND_RETURN_LOG_RET(adType == ADV_UUID_ADTYPE || adType == ADV_MANUFACTURE_ADTYPE, false,
            "not supported advertiser");
    }
    CHECK_AND_RETURN_LOG_RET(index < static_cast<int>(advPayload.length()) - 1, false, "failed to parse uuid");
    uuid = advPayload.substr(index, ADV_2_OCTS_LENGTH);
    uint16_t uintUuid = 0;
    StreamToUint16(advPayload, uintUuid, index);
    CHECK_AND_RETURN_LOG_RET(GetBusinessType(advPayload, uintUuid, businessType, index), false,
        "GetBusinessType fail");
    return true;
}

static std::vector<std::string> ConvertChannelRssiSensingInfo(const std::vector<uint32_t> &idxInfo,
    const std::vector<int64_t> &channelRssiInfo, int64_t timestamp)
{
    std::vector<std::string> channelRssiSensingInfo = {};
    if (channelRssiInfo.size() < static_cast<size_t>(BT_ACL_RSSI_REP_SIZE)) {
        HILOGI_TIME_LIMIT(std::to_string(channelRssiInfo.size()),
            "(ConvertChannelRssiSensingInfo)invalid channelRssiInfo, length: %{public}lu", channelRssiInfo.size());
        return channelRssiSensingInfo;
    }
    channelRssiSensingInfo.emplace_back(std::to_string(CHANNEL_RSSI_REPORT));
    channelRssiSensingInfo.emplace_back(std::to_string(BT_ACL_RSSI_REP_SIZE * RSSI_UNIT_SIZE + 1));
    channelRssiSensingInfo.emplace_back(std::to_string(timestamp));
    for (int i = 0; i < BT_ACL_RSSI_REP_SIZE; i++) {
        channelRssiSensingInfo.emplace_back(std::to_string(idxInfo[i]));
        channelRssiSensingInfo.emplace_back(std::to_string(channelRssiInfo[i]));
    }
    return channelRssiSensingInfo;
}

static std::vector<std::string> ConvertTransRateSensingInfo(const std::pair<uint32_t, uint32_t> &transRate,
    const BTTransport transport)
{
    std::vector<std::string> sensingArray = {};
    if (transRate.first == 0 && transRate.second == 0) {
        HILOGD("Invalid trans rate.");
        return sensingArray;
    }
    sensingArray.emplace_back(std::to_string(BT_TRANS_RATE_REPORT));
    sensingArray.emplace_back(std::to_string(BT_TRANS_RATE_REP_SIZE));
    sensingArray.emplace_back(std::to_string(transport));
    sensingArray.emplace_back(std::to_string(transRate.first));
    sensingArray.emplace_back(std::to_string(transRate.second));
    return sensingArray;
}

void BluetoothResourceManager::AddAdvRecord(const BleAdvertiserSettingsImpl &settings,
    const BleAdvertiserDataImpl &advData, uint8_t advHandle)
{
    std::string advPayload = advData.GetPayload();
    std::string uuid = "";
    std::string businessType = "";
    bool ret = ParseAdvPayload(advPayload, uuid, businessType);
    uint16_t uintUuid = 0;
    uint8_t mainBusinessType = 0;
    uint8_t extendBusinessType = 0;
    int uuidIndex = 0;
    StreamToUint16(uuid, uintUuid, uuidIndex);
    int businessTypeIndex  = 0;
    StreamToUint8(businessType, mainBusinessType, businessTypeIndex);
    StreamToUint8(businessType, extendBusinessType, businessTypeIndex);
    HILOGI("advHandle: %{public}d, uuid: %{public}04x, connectable: %{public}d,"
        "businesstype: %{public}02x%{public}02x, interval: %{public}d, result: %{public}d",
        advHandle, uintUuid, settings.IsConnectable(), mainBusinessType, extendBusinessType,
        settings.GetInterval(), ret);
    advMap_.EnsureInsert(advHandle, std::make_shared<SensingInfo>(
        BleAdvData(uuid, settings.IsConnectable(), advPayload.length(), businessType),
        advHandle, settings.GetInterval()));
}

std::shared_ptr<SensingInfo> BluetoothResourceManager::FindAdvRecord(uint8_t advHandle)
{
    std::shared_ptr<SensingInfo> info = nullptr;
    bool ret = advMap_.Find(advHandle, info);
    if (!ret || info == nullptr) {
        return nullptr;
    }
    return info;
}

void BluetoothResourceManager::SendAdvStartedEvent(int result, uint8_t advHandle)
{
    if (result != ADVERTISE_SUCCESS) {
        HILOGE("Adv failed to start, result: %{public}d, advHandle: %{public}d", result, advHandle);
        RemoveAdvRecord(advHandle);
        return;
    }
    std::shared_ptr<SensingInfo> info = FindAdvRecord(advHandle);
    if (info == nullptr) {
        HILOGE("AdvRecord not found, result: %{public}d, advHandle: %{public}d", result, advHandle);
        return;
    }
    HILOGD("SendAdvStartedEvent success, result: %{public}d, advHandle: %{public}d", result, advHandle);
    SendSensingStateChanged(BLE_ADV_STARTED, *info);
}

void BluetoothResourceManager::SendAdvStoppedEvent(int result, uint8_t advHandle)
{
    if (result != ADVERTISE_SUCCESS) {
        HILOGE("Adv failed to stop, result: %{public}d, advHandle: %{public}d", result, advHandle);
        return;
    }
    HILOGD("SendAdvStoppedEvent success, result: %{public}d, advHandle: %{public}d", result, advHandle);
    SensingInfo sensingInfo(advHandle);
    GetChannelRssiSensingData(sensingInfo);
    SendSensingStateChanged(BLE_ADV_STOPPED, sensingInfo);
    RemoveAdvRecord(advHandle);
}

void BluetoothResourceManager::GetChannelRssiSensingData(SensingInfo &sensingInfo)
{
    std::vector<uint32_t> idxInfo = {};
    std::vector<int64_t> channelRssiInfo = {};
    int64_t timestamp = 0;
    BluetoothChannelRssiManager::GetInstance()->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    std::vector<std::string> channelRssiSensingInfo = ConvertChannelRssiSensingInfo(idxInfo, channelRssiInfo,
        timestamp);
    sensingInfo.sensingArray_ = channelRssiSensingInfo;
}

void BluetoothResourceManager::GetTransRateData(SensingInfo &sensingInfo)
{
    const BthwifInterface* bthwifInterface = GetBtHwInterface();
    if (bthwifInterface == nullptr) {
        HILOGE("bthwifInterface is nullptr");
        return;
    }
    std::pair<uint32_t, uint32_t> brTransRate = bthwifInterface->hwGetBrTransTxRxRate();
    std::vector<std::string> brTransRateSensingInfo =
        ConvertTransRateSensingInfo(brTransRate, BTTransport::ADAPTER_BREDR);
    sensingInfo.sensingArray_.insert(sensingInfo.sensingArray_.end(), brTransRateSensingInfo.begin(),
        brTransRateSensingInfo.end());
}

void BluetoothResourceManager::RemoveAdvRecord(uint8_t advHandle)
{
    std::shared_ptr<SensingInfo> info = FindAdvRecord(advHandle);
    if (info == nullptr) {
        HILOGE("AdvRecord not found, advHandle: %{public}d", advHandle);
        return;
    }
    HILOGD("RemoveAdvRecord success, advHandle: %{public}d", advHandle);
    advMap_.Erase(advHandle);
}

void BluetoothResourceManager::UpdateScanRecord(int scanMode)
{
    scanMsg_.scanMode_ = scanMode;
}

void BluetoothResourceManager::SendScanStartEvent(int status)
{
    if (status != SCAN_SUCCESS) {
        HILOGE("Scanning failed to start, status: %{public}d", status);
        return;
    }
    HILOGD("SendScanStartEvent success");
    scanMsg_.scanStatus_ = STARTED;
    SendSensingStateChanged(BLE_SCAN_STARTED_EVENT, SensingInfo(scanMsg_.scanMode_, scanMsg_.scanStatus_));
}

void BluetoothResourceManager::SendScanStopEvent(int status)
{
    if (status != SCAN_SUCCESS) {
        HILOGE("Scanning failed to stop, status: %{public}d", status);
        return;
    }
    HILOGD("SendScanStoppedEvent success");
    scanMsg_.scanStatus_ = STOPPED;
    SensingInfo sensingInfo(scanMsg_.scanMode_, scanMsg_.scanStatus_);
    std::vector<uint32_t> idxInfo = {};
    std::vector<int64_t> channelRssiInfo = {};
    int64_t timestamp = 0;
    BluetoothChannelRssiManager::GetInstance()->GetChannelRssiInfo(idxInfo, channelRssiInfo, timestamp);
    std::vector<std::string> channelRssiSensingInfo = ConvertChannelRssiSensingInfo(idxInfo, channelRssiInfo,
        timestamp);
    sensingInfo.sensingArray_ = channelRssiSensingInfo;
    SendSensingStateChanged(BLE_SCAN_STOPPED_EVENT, sensingInfo);
}

void BluetoothResourceManager::SendBleScanEnhanceModeEvent(const BleScanEnhanceModeImpl &enhanceMode)
{
    uint8_t sensingEvent = enhanceMode.isEnable ? BLE_SCAN_ENHANCE_MODE_STARTED_EVENT :
        BLE_SCAN_ENHANCE_MODE_STOPPED_EVENT;
    SensingInfo sensingInfo(enhanceMode.mode, scanMsg_.scanStatus_);
    sensingInfo.sensingArray_.emplace_back(std::to_string(enhanceMode.mode));
    sensingInfo.sensingArray_.emplace_back(std::to_string(enhanceMode.timeout));
    SendSensingStateChanged(sensingEvent, sensingInfo);
}

/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
static bool IsAllowedFastestGattConn(const ControlInterceptMessage &msg)
{
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    uint32_t decisionResult = CONNECTION_ACCEPT;
    if (resourceMgr) {
        resourceMgr->SendBluetoothResourceDecision(
            BLE_FASTEST_CONN_DECISION, SensingInfo(msg.addr), decisionResult);
    }
    if (decisionResult == CONNECTION_REJECT) {
        HILOGE("FastestConn is rejected while audio or hfp is playing");
        BtChrUeManager::GetInstance()->WriteCommonUe(
            CHR_UE_BLE_DISCONN_CMP, RawAddress(msg.addr), UE_COMMON_SCENE_CASE4, BLE_FASTEST_CONN_DECISION);
        return false;
    }
    return true;
}

static ControlInterceptPlugin g_resourceManagerPlugin = {
    .fastestGattConn = IsAllowedFastestGattConn,
};

REGISTER_CONTROL_INTERCEPT_PLUGIN(g_resourceManagerPlugin);
/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
}
}