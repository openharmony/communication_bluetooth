/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_ble_range"
#endif

#include "ble_range_impl.h"
#include "log.h"
#include "bt_types.h"
#include "stack_adapter.h"
#include "bluetooth_errorcode.h"
#include <parameters.h>
#include <parameter.h>
#include "json_utils.h"
#include "cJSON.h"

namespace OHOS {
namespace bluetooth {

static const uint8_t DEFAULT_MAC_ID_BIT = 5; //bit5代表mac所在bit位
static const uint8_t MAC_ID_ONE = 1;
static const uint8_t MAC_ID_ZERO = 0;
static const uint8_t MAC_SHIFT_BITS = 1;
static const uint8_t MAC_MASK = 0x3;
static const uint8_t BLE_MAC_SWITCH_TYPE = 0xA0;
static const uint8_t BLE_MAC_SWITCH_TYPE_SHIFT = 8;

constexpr const char* BLE_RANGE_CCM_CONFIG_FILENAME = "sys_prod/etc/msdp/scenario_cfg.json";
constexpr size_t MAX_BLE_RANGE_MESSAGE_LEN = 1024 * 1024;
constexpr int32_t DEFAULT_BLE_RANGE_ADVPOWER = -10;
constexpr const char* RANGING_ANT_INFO_PARAM = "persist.bluetooth.ranging_ant_info";
constexpr const char* RANGING_ANT_INFO_DEFAULT = "0:0";
constexpr const char *FOLD_STATE_KEY = "vendor.cust.stablefoldstate";
constexpr const char *FOLD_STATE = "2";

static const uint8_t MAC_MASK_BIT = 0x1;
std::weak_ptr<BleRangeImpl> BleRangeImpl::s_currentInstance;
std::mutex BleRangeImpl::s_instanceMutex;

BleRangeImpl::BleRangeImpl()
    : configMac_(0),
      configAnt_(0),
      defaultMac_(0),
      btInterface_(nullptr),
      advPower_(DEFAULT_BLE_RANGE_ADVPOWER),
      currentAdvPower_(INNER_BLE_RANGING_INVALID_ADVPOWER),
      hasSwitchedMac_(false)
{
    configMac_ = static_cast<uint32_t>(OHOS::system::GetIntParameter("const.bluetooth.ranging.multi_mac", 0));
    configAnt_ = static_cast<uint32_t>(OHOS::system::GetIntParameter("const.bluetooth.ranging.multi_ant", 0));
    defaultMac_ = static_cast<uint32_t>(OHOS::system::GetIntParameter("const.bluetooth.dual_mac.enable", 0));

    cachedMacID_ = (defaultMac_ >> DEFAULT_MAC_ID_BIT) & MAC_MASK_BIT;
    cachedAntID_ = 0; // 初始化，缓存ant为0，代表天线默认态
    int status = hal_util_load_bt_library(&btInterface_);
    if (status != 0 || btInterface_ == nullptr) {
        HILOGE("Failed to load bt library for antenna switch");
    }

    satelliteManager_ = std::make_shared<SatelliteManager>();

    int32_t ret = InitGetBleRangeParam();
    if (ret != 0) {
        HILOGE("InitGetBleRangeParam failed, ret=%{public}d", ret);
    }
}

void BleRangeImpl::Init()
{
    {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        s_currentInstance = weak_from_this();
    }

    RegisterAntSwitchCallback();
    SendAntennaInfoQueryMsg();
}

BleRangeImpl::~BleRangeImpl()
{
    std::lock_guard<std::mutex> lock(rangingMutex_);
    rangingPromise_ = nullptr;

    if (btInterface_ != nullptr && btInterface_->unregisterRangingAntSwitchCallback != nullptr) {
        btInterface_->unregisterRangingAntSwitchCallback();
    }

    {
        std::lock_guard<std::mutex> lockGuard(s_instanceMutex);
        s_currentInstance.reset();
    }
}

bool BleRangeImpl::SwitchMacIfNeeded(uint8_t currentMacId, uint8_t targetMacId)
{
    HILOGI("SwitchMacIfNeeded - currentMacId: %{public}u, targetMacId: %{public}u", currentMacId, targetMacId);

    if (currentMacId == targetMacId) {
        HILOGI("Current MAC matches target, no switch needed");
        return true;
    }

    if (satelliteManager_ == nullptr) {
        HILOGE("satelliteManager_ is null, cannot switch MAC");
        return false;
    }

    // 切换mac参数格式 type|macid
    int32_t combinedParam = (static_cast<int32_t>(BLE_MAC_SWITCH_TYPE) << BLE_MAC_SWITCH_TYPE_SHIFT) | targetMacId;

    int32_t result = satelliteManager_->SatelliteControl(
        static_cast<int>(SATELLITE_CONTROL_MODE::SIGNALHUB_MAC_SWITCH), combinedParam);

    if (result == Bluetooth::BT_NO_ERROR) {
        HILOGI("MAC switch to %{public}u success", targetMacId);
        return true;
    } else {
        HILOGE("MAC switch failed, result: %{public}d", result);
        return false;
    }
}

void BleRangeImpl::OnRangingAntSwitchCallbackStatic(uint8_t macID, uint8_t antID)
{
    std::shared_ptr<BleRangeImpl> instance;
    {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        instance = s_currentInstance.lock();
    }
    if (!instance) {
        HILOGW("s_currentInstance is null, callback ignored");
        return;
    }
    instance->OnRangingAntSwitchCallbackInternal(macID, antID);
}

void BleRangeImpl::OnRangingAntSwitchCallbackInternal(uint8_t macID, uint8_t antID)
{
    uint8_t targetMacId = (configMac_ >> MAC_SHIFT_BITS) & MAC_MASK;
    uint8_t expectedMac = 0;

    bool hasSwitched = false;
    {
        std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
        hasSwitched = hasSwitchedMac_;
        if (hasSwitched) {
            expectedMac = targetMacId;
        } else {
            expectedMac = cachedMacID_;
        }
    }

    HILOGI("Ranging ant switch callback received, macID: %{public}u, antID: %{public}u, expectedMac: %{public}u",
           macID, antID, expectedMac);

    if (macID != expectedMac) {
        HILOGW("MAC mismatch, expected: %{public}u, got: %{public}u, ignoring", expectedMac, macID);
        return;
    }

    HILOGI("MAC validated, updating antenna info");
    StoreRangingAntSwitchInfo(macID, antID);

    std::lock_guard<std::mutex> lock(rangingMutex_);
    if (rangingPromise_ != nullptr) {
        rangingPromise_->set_value(true);
        rangingPromise_ = nullptr;
    }
}

int BleRangeImpl::GetCurrentAdvPower() const
{
    return currentAdvPower_.load();
}

void BleRangeImpl::SetCurrentAdvPower(int advPower)
{
    currentAdvPower_.store(advPower);
    HILOGI("SetCurrentAdvPower: %{public}d", advPower);
}

int32_t BleRangeImpl::GetAdvPower() const
{
    return advPower_;
}

int32_t BleRangeImpl::GetAdvPowerByAntInfo()
{
    uint8_t macID = 0;
    uint8_t antID = 0;
    {
        std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
        macID = cachedMacID_;
        antID = cachedAntID_;
    }

    std::string foldStateVal = OHOS::system::GetParameter(FOLD_STATE_KEY, "");
    if (foldStateVal.empty()) {
        HILOGW("GetAdvPowerByAntInfo: fold state param not found, key=%{public}s", FOLD_STATE_KEY);
    }
    bool isFolded = (foldStateVal == FOLD_STATE);

    std::vector<int32_t>& antArr = (macID == MAC_ID_ZERO) ? advPowerMac0Ant_ : advPowerMac1Ant_;
    std::vector<int32_t>& foldedAntArr =
        (macID == MAC_ID_ZERO) ? advPowerFoldedMac0Ant_ : advPowerFoldedMac1Ant_;

    if (isFolded && !foldedAntArr.empty() && antID < foldedAntArr.size()) {
        int32_t advpower = foldedAntArr[antID];
        HILOGI("GetAdvPowerByAntInfo: folded, macID=%{public}u, antID=%{public}u, advpower=%{public}d",
            macID, antID, advpower);
        return advpower;
    }

    if (!antArr.empty() && antID < antArr.size()) {
        int32_t advpower = antArr[antID];
        HILOGI("GetAdvPowerByAntInfo: macID=%{public}u, antID=%{public}u, advpower=%{public}d",
            macID, antID, advpower);
        return advpower;
    }

    HILOGI("GetAdvPowerByAntInfo: no match for macID=%{public}u, antID=%{public}u, fallback to default=%{public}d",
        macID, antID, advPower_);
    return advPower_;
}

int BleRangeImpl::GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType)
{
    HILOGI("GetAdvPowerForRangingBusiness appType: %{public}u", appType);
    HILOGI("get config mac_config: 0x%{public}x, ant_config : 0x%{public}x", configMac_, configAnt_);
    int advPower = INNER_BLE_RANGING_INVALID_ADVPOWER;

    int nonRangingResult = HandleNonRangingAppType(appType, advPower);
    if (nonRangingResult != -1) {
        return nonRangingResult;
    }

    HandleRangingSwitching(appType, advPower);
    return GetCurrentAdvPower();
}

void BleRangeImpl::HandleRangingSwitching(bluetooth::BleAppType appType, int advPower)
{
    HILOGI("HandleRangingSwitching appType: %{public}u", appType);
    bool needMacSwitch = (configMac_ & MAC_MASK_BIT) != 0;
    bool needAntSwitch = (configAnt_ & MAC_MASK_BIT) != 0;

    if (!needMacSwitch && !needAntSwitch) {
        HILOGI("No switch needed, config not support");
        advPower = GetAdvPowerByAntInfo();
        SetCurrentAdvPower(advPower);
        return;
    }

    uint8_t defaultMacId = (defaultMac_ >> DEFAULT_MAC_ID_BIT) & MAC_MASK_BIT;
    uint8_t targetMacId = (configMac_ >> MAC_SHIFT_BITS) & MAC_MASK;
    if (needMacSwitch) {
        uint8_t currentMacId = GetRangingAntSwitchInfo().macID;
        if (!SwitchMacIfNeeded(currentMacId, targetMacId)) {
            HILOGW("MAC switch from %{public}u to %{public}u failed", currentMacId, targetMacId);
            advPower = GetAdvPowerByAntInfo();
            SetCurrentAdvPower(advPower);
            return;
        }
        {
            std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
            hasSwitchedMac_ = true;
        }
        uint8_t antID = 0;
        StoreRangingAntSwitchInfo(targetMacId, antID);
    }

    advPower = GetAdvPowerByAntInfo();

    if (needAntSwitch) {
        if (!PrepareAndSendBleAdv(advPower)) {
            HILOGW("PrepareAndSendBleAdv failed");
            SetCurrentAdvPower(advPower);
            return;
        }
        WaitForAntSwitchCallbackAndHandleResult(defaultMacId, targetMacId, advPower);
        return;
    }

    SetCurrentAdvPower(advPower);
}

bool BleRangeImpl::PrepareAndSendBleAdv(int advPower)
{
    {
        std::lock_guard<std::mutex> lock(rangingMutex_);
        rangingPromise_ = std::make_shared<std::promise<bool>>();
    }

    if (!SendBleAdvStartMsg()) {
        SetCurrentAdvPower(advPower);
        std::lock_guard<std::mutex> lock(rangingMutex_);
        if (rangingPromise_ != nullptr) {
            rangingPromise_->set_value(false);
            rangingPromise_ = nullptr;
        }
        return false;
    }
    return true;
}

int BleRangeImpl::HandleNonRangingAppType(bluetooth::BleAppType appType, int advPower)
{
    if (appType != bluetooth::BleAppType::INNER_BLE_RANGING_TOUCH) {
        advPower = GetAdvPowerByAntInfo();
        HILOGI("No ant switch needed for this appType, advpower: %{public}d", advPower);
        SetCurrentAdvPower(advPower);
        return GetCurrentAdvPower();
    }
    return -1;
}

void BleRangeImpl::RegisterAntSwitchCallback()
{
    if (btInterface_ == nullptr || btInterface_->registerRangingAntSwitchCallback == nullptr) {
        HILOGE("btInterface_ or register_ranging_ant_switch_callback is null");
        return;
    }
    int result = btInterface_->registerRangingAntSwitchCallback(OnRangingAntSwitchCallbackStatic);

    if (result != 0) {
        HILOGE("Failed to register ant switch callback, result=%{public}d", result);
        return;
    }
}

bool BleRangeImpl::SendBleAdvStartMsg()
{
    if (btInterface_ != nullptr && btInterface_->sendBleAdvStartMsg != nullptr) {
        btInterface_->sendBleAdvStartMsg(configMac_, configAnt_);
        return true;
    }
    HILOGE("btInterface_ or sendbleadvstartmsg is null");
    return false;
}

void BleRangeImpl::SendAntennaInfoQueryMsg()
{
    if (btInterface_ != nullptr && btInterface_->sendAntennaInfoQueryMsg != nullptr) {
        btInterface_->sendAntennaInfoQueryMsg();
        return;
    }
    HILOGE("btInterface_ or send_antenna_info_query_msg is null");
}

void BleRangeImpl::WaitForAntSwitchCallbackAndHandleResult(uint8_t defaultMacId, uint8_t targetMacId, int advPower)
{
    using namespace std::chrono_literals;

    std::future<bool> future;
    {
        std::lock_guard<std::mutex> lock(rangingMutex_);
        if (rangingPromise_ != nullptr) {
            future = rangingPromise_->get_future();
        }
    }

    bool callbackReceived = false;
    if (future.valid()) {
        auto waitResult = future.wait_for(std::chrono::milliseconds(RANGING_ANT_SWITCH_TIMEOUT_MS));
        if (waitResult == std::future_status::ready) {
            callbackReceived = future.get();
        } else {
            HILOGI("wait timeout, consumption promise");
            std::lock_guard<std::mutex> lock(rangingMutex_);
            if (rangingPromise_ != nullptr) {
                rangingPromise_->set_value(false);
                rangingPromise_ = nullptr;
            }
        }
    } else {
        HILOGI("callback arrived before get_future, treating as success");
        callbackReceived = true;
    }

    advPower = GetAdvPowerByAntInfo();
    if (callbackReceived) {
        HILOGI("Ant switch callback received in time, advpower: %{public}d", advPower);
        SetCurrentAdvPower(advPower);
    } else {
        HILOGW("Ant switch callback timeout: %{public}d", advPower);
        uint8_t currentMacId = GetRangingAntSwitchInfo().macID;
        if (!SwitchMacIfNeeded(currentMacId, defaultMacId)) {
            HILOGW("MAC switch back from %{public}u to %{public}u failed", currentMacId, defaultMacId);
        } else {
            uint8_t antID = 0;
            StoreRangingAntSwitchInfo(defaultMacId, antID);
            advPower = GetAdvPowerByAntInfo();
        }
        SetCurrentAdvPower(advPower);
    }

    {
        std::lock_guard<std::mutex> lock(rangingMutex_);
        rangingPromise_ = nullptr;
    }

    HILOGI("HandleRangingSwitching advPower: %{public}d", GetCurrentAdvPower());
}

int BleRangeImpl::BleRestoreRangingAntSwitch(bluetooth::BleAppType appType)
{
    uint8_t defaultMacId = (defaultMac_ >> DEFAULT_MAC_ID_BIT) & MAC_MASK_BIT;

    uint8_t currentMacId = GetRangingAntSwitchInfo().macID;
    if (!SwitchMacIfNeeded(currentMacId, defaultMacId)) {
        HILOGW("ranging stop reset mac from %{public}u to %{public}u failed", currentMacId, defaultMacId);
    }

    {
        std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
        hasSwitchedMac_ = false;
    }
    uint8_t antID = 0;
    StoreRangingAntSwitchInfo(defaultMacId, antID);

    if (btInterface_ != nullptr && btInterface_->sendBleAdvStopMsg != nullptr) {
        btInterface_->sendBleAdvStopMsg();
    } else {
        HILOGE("btInterface_ or sendbleadvstopmsg is null");
        return -1;
    }

    return 0;
}

int32_t BleRangeImpl::InitGetBleRangeParam()
{
    HILOGI("Enter");
    cJSON* root = JsonUtil::ReadJsonFile(BLE_RANGE_CCM_CONFIG_FILENAME);
    CHECK_AND_RETURN_LOG_RET(root, -1, "cJSON parse failed");

    if (!cJSON_IsArray(root) || cJSON_GetArraySize(root) == 0) {
        HILOGE("Data is invalid, root is not array or array is empty");
        cJSON_Delete(root);
        return -1;
    }
    cJSON* configItem = cJSON_GetArrayItem(root, 0);
    if (configItem == nullptr || !cJSON_IsObject(configItem)) {
        HILOGE("ConfigItem is null or not an object");
        cJSON_Delete(root);
        return -1;
    }

    constexpr const char* key = "advPower";
    cJSON* jsonItem = cJSON_GetObjectItem(configItem, key);
    if (jsonItem != nullptr && cJSON_IsNumber(jsonItem)) {
        advPower_ = jsonItem->valueint;
    }

    HILOGI("advPower is %{public}d", advPower_);

    ParseAdvPowerAntFromConfig(configItem);

    cJSON_Delete(root);
    return 0;
}

void BleRangeImpl::ParseAdvPowerAntFromConfig(cJSON* configItem)
{
    auto parseArr = [configItem](const char* key, std::vector<int32_t>& outArr) {
        cJSON* jsonArr = cJSON_GetObjectItem(configItem, key);
        if (jsonArr == nullptr || !cJSON_IsArray(jsonArr)) {
            return;
        }
        for (int i = 0; i < cJSON_GetArraySize(jsonArr); i++) {
            cJSON* item = cJSON_GetArrayItem(jsonArr, i);
            if (item != nullptr && cJSON_IsNumber(item)) {
                outArr.push_back(item->valueint);
            }
        }
        HILOGI("%{public}s size=%{public}zu", key, outArr.size());
    };
    parseArr("advpower_mac0_ant", advPowerMac0Ant_);
    parseArr("advpower_mac1_ant", advPowerMac1Ant_);
    parseArr("advpower_folded_mac0_ant", advPowerFoldedMac0Ant_);
    parseArr("advpower_folded_mac1_ant", advPowerFoldedMac1Ant_);
}

void BleRangeImpl::StoreRangingAntSwitchInfo(uint8_t macID, uint8_t antID)
{
    {
        std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
        cachedMacID_ = macID;
        cachedAntID_ = antID;
    }

    std::string paramValue = std::to_string(macID) + ":" + std::to_string(antID);
    int ret = SetParameter(RANGING_ANT_INFO_PARAM, paramValue.c_str());
    if (ret != 0) {
        HILOGE("SetParameter failed, ret=%{public}d", ret);
    } else {
        HILOGI("SetParameter success, macID=%{public}u, antID=%{public}u", macID, antID);
    }
}

BleRangeImpl::AntSwitchInfo BleRangeImpl::GetRangingAntSwitchInfo() const
{
    std::lock_guard<std::mutex> lock(antSwitchInfoMutex_);
    AntSwitchInfo info;
    info.macID = cachedMacID_;
    info.antID = cachedAntID_;
    return info;
}

}  // namespace bluetooth
}  // namespace OHOS
