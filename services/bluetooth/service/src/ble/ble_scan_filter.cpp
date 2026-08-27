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
#define LOG_TAG "bt_service_ble_scan_filter"
#endif

#include "ble_scan_filter.h"

#include <future>
#include <sstream>

#include "bt_gatt.h"
#include "btm_ble_api_types.h"
#include "log.h"
#include "securec.h"
#include "service_util.h"
#include "log_utils.h"
#include "hitrace_meter.h"
#include "permission_manager.h"
#include "parameter.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_dft_exception.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
constexpr int PROPERTY_VALUE_MAX = 128;
constexpr int IRK_ENCRYPT_START_POS = 8;
constexpr int IRK_ENCRYPT_END_POS = 30;
constexpr const char *CONNECTIVITY_BT_CHIP_TYPE = "ohos.boot.odm.conn.schiptype";
constexpr const char *WALLET_SET_HIGH_RSSI_CHIP_TYPE = "mp17c";

// ble扫描地址过滤器中的地址类型设置为"NA"，则BTC只过滤地址，不限制地址类型
// 0x00(BLE_ADDR_PUBLIC)、0x01(BLE_ADDR_RANDOM)见ble_address_with_type.h
constexpr int BLE_ADDR_ALL_TYPE = 0x02;

BleScanFilter::BleScanFilter(int scannerId, BleScannerInterface *scanner)
    : scannerId_(scannerId), btifBleScanner_(scanner) {}

BleScanFilter::~BleScanFilter()
{
}

bool BleScanFilter::SendScanFilterParam(uint8_t action, int filterIndex,
    std::unique_ptr<btgatt_filt_param_setup_t> filtParam) const
{
    if (!btifBleScanner_) {
        return false;
    }

    HITRACE_METER(BT_TRACE_TAG);
    auto promise = std::make_shared<std::promise<uint8_t>>();
    auto future = promise->get_future();

    btifBleScanner_->ScanFilterParamSetup(scannerId_, action, filterIndex, std::move(filtParam),
        [promise](uint8_t availableSpace, uint8_t actionType, uint8_t btmStatus) {
            HITRACE_METER_NAME(BT_TRACE_TAG, "ScanFilterParamCb");
            if (btmStatus != BTM_SUCCESS) {
                HILOGI("availableSpace: %{public}u, actionType: %{public}u, btmStatus: %{public}u",
                    availableSpace, actionType, btmStatus);
                BtChrBtExcpEvent("", BTOPT_BLE_SCAN_FILTER_FAIL, btmStatus);
            } else {
                HILOGD("availableSpace: %{public}u, actionType: %{public}u, btmStatus: %{public}u",
                    availableSpace, actionType, btmStatus);
            }
            if (promise) {
                promise->set_value(btmStatus);
            }
        });

    if (future.wait_for(std::chrono::milliseconds(WAIT_TIMEOUT)) != std::future_status::ready) {
        HILOGE("ScanFilterParamSetup timeout");
        return false;
    }

    uint8_t result = future.get();
    if (result == HCI_ERR_REPEATED_ATTEMPTS && action == FILTER_ACTION_ADD) {
        SendScanFilterParam(FILTER_ACTION_DELETE, filterIndex, nullptr);
    }

    return result == BTM_SUCCESS;
}

bool BleScanFilter::SendScanFilter(int filterIndex, std::vector<ApcfCommand> cmds) const
{
    if (!btifBleScanner_) {
        return false;
    }

    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();

    btifBleScanner_->ScanFilterAdd(filterIndex, std::move(cmds),
        [promise](uint8_t filterType, uint8_t availableSpace, uint8_t action, uint8_t btmStatus) {
            HILOGD("ScanFilterAdd: filterType: %{public}u, availableSpace: %{public}u, action: %{public}u,"
                "btmStatus: %{public}u", filterType, availableSpace, action, btmStatus);
            if (btmStatus != BTM_SUCCESS) {
                BtChrBtExcpEvent("", BTOPT_BLE_SCAN_FILTER_FAIL, btmStatus);
            }
            if (promise) {
                bool ok = (btmStatus == BTM_SUCCESS);
                promise->set_value(ok);
            }
        });

    if (future.wait_for(std::chrono::milliseconds(WAIT_TIMEOUT)) != std::future_status::ready) {
        HILOGE("ScanFilterAdd timeout");
        return false;
    }

    return future.get();
}

void BleScanFilter::DeleteScanFilters(void)
{
    std::lock_guard<std::mutex> lock(filterIndexsMutex_);
    for (int index : filterIndexSet_) {
        SendScanFilterParam(FILTER_ACTION_DELETE, index, nullptr);
        const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
        if (bluetoothHwSrcInterface == nullptr) {
            HILOGE("interface nullptr");
            return;
        }
        bluetoothHwSrcInterface->hwDeletAdvIndReportFlag(index);
    }

    FilterIndex::GetInstance()->Release(std::vector<int>(filterIndexSet_.begin(), filterIndexSet_.end()));
    filterIndexSet_.clear();
    filterImpl_.Clear();
}

uint16_t BleScanFilter::GetFeatureSelection(const BleScanFilterImpl &filter) const
{
    uint16_t ret = 0;
    if (!filter.GetDeviceId().empty()) {
        ret |= ENABLE_BROADCAST_ADDRESS;
    }
    if (filter.HasServiceUuid()) {
        ret |= ENABLE_SERVICE_UUID;
    }
    if (filter.HasSolicitationUuid()) {
        ret |= ENABLE_SERVICE_SOLICITATION_UUID;
    }
    if (!filter.GetName().empty()) {
        ret |= ENABLE_LOCAL_NAME;
    }
    if (!filter.GetManufactureData().empty()) {
        ret |= ENABLE_MANUFACTURER_DATA;
    }
    if (!filter.GetServiceData().empty()) {
        ret |= ENABLE_SERVICE_DATA;
    }
    if (!filter.GetIrk().empty()) {
        ret |= ENABLE_IRK;
    }
    return ret;
}

bool BleScanFilter::SetScanFilterParameters(int filterIndex, const BleScanFilterImpl &filter,
    uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvNum) const
{
    bool onFound = (deliveryMode == DELIVERY_MODE_ON_FOUND);
    auto param = std::make_unique<btgatt_filt_param_setup_t>();
    auto [highRssi, lowRssi] = GetRssiThres(sensitivityMode, onFound);
    param->feat_seln = GetFeatureSelection(filter);
    param->list_logic_type = LIST_LOGIC_TYPE;
    param->filt_logic_type = LOGIC_AND;
    int32_t rssiThreshold = filter.HasRssiThreshold() ? filter.GetRssiThreshold() : BLE_SCAN_MIN_RSSI_THRESHOLD;
    // Set a value with a smaller absolute value
    param->rssi_high_thres = static_cast<uint8_t>((rssiThreshold > highRssi) ? rssiThreshold : highRssi);
    param->dely_mode = deliveryMode;
    param->found_timeout = GetOnFoundOrLostTimeout(sensitivityMode, true);
    param->lost_timeout = ON_LOST_IMEOUT_MS;
    param->found_timeout_cnt = GetOnFoundCount(sensitivityMode);
    param->rssi_low_thres = static_cast<uint8_t>(lowRssi);
    param->num_of_tracking_entries = trackAdvNum;

    return SendScanFilterParam(FILTER_ACTION_ADD, filterIndex, std::move(param));
}

static std::string IntToHexString(uint8_t value)
{
    std::stringstream strStream;
    char token[3] = {0};
    sprintf_s(token, sizeof(token), "%02X", value);
    strStream << token;
    return strStream.str();
}

static std::string ToHexString(const std::vector<uint8_t> &v)
{
    std::string strs ("0x");
    for (auto iter = v.begin(); iter != v.end(); iter++) {
        uint8_t temp = *iter;
        strs += IntToHexString(temp);
    }
    return strs;
}

static std::string GetEncryptIrk(const std::array<uint8_t, LEN_OF_IRK> &arrayIrk)
{
    std::vector<uint8_t> vecIrk(arrayIrk.begin(), arrayIrk.end());
    std::string strIrk = ToHexString(vecIrk);
    std::fill(strIrk.begin() + IRK_ENCRYPT_START_POS, strIrk.begin() + IRK_ENCRYPT_END_POS, '*');
    return strIrk;
}

void BleScanFilter::ApcfCommandLog(const std::vector<ApcfCommand> &cmds) const
{
    for (const auto &cmd : cmds) {
        switch (cmd.type) {
            case BTM_BLE_PF_ADDR_FILTER:
                HILOGI("Address: %{public}s, AddrType: %{public}d, Irk: %{public}s", cmd.address.ToStringForLogging().c_str(),
                    cmd.addr_type, GetEncryptIrk(cmd.irk).c_str());
                break;
            case BTM_BLE_PF_SRVC_UUID:
                HILOGI("ServiceUuid: %{public}s, mask: %{public}s",
                    cmd.uuid.ToString().c_str(), cmd.uuid_mask.ToString().c_str());
                break;
            case BTM_BLE_PF_SRVC_SOL_UUID:
                HILOGI("ServiceSoliUuid: %{public}s, mask: %{public}s",
                    cmd.uuid.ToString().c_str(), cmd.uuid_mask.ToString().c_str());
                break;
            case BTM_BLE_PF_LOCAL_NAME:
                HILOGI("Name: %{public}s", std::string(cmd.name.begin(), cmd.name.end()).c_str());
                break;
            case BTM_BLE_PF_MANU_DATA:
                HILOGI_TIME_LIMIT(__func__,
                    "CompanyId:%{public}#x, CompanyMask:%{public}#x, ManuData:%{public}s, ManuMask:%{public}s",
                    cmd.company, cmd.company_mask, ToHexString(cmd.data).c_str(), ToHexString(cmd.data_mask).c_str());
                break;
            case BTM_BLE_PF_SRVC_DATA_PATTERN:
                HILOGD("ServiceData: %{public}s, mask: %{public}s",
                    ToHexString(cmd.data).c_str(), ToHexString(cmd.data_mask).c_str());
                break;
            default:
                break;
        }
    }
}

ApcfCommand BleScanFilter::GetCommand(uint8_t cmdType, const BleScanFilterImpl &filter) const
{
    ApcfCommand cmd {};
    cmd.type = cmdType;

    switch (cmdType) {
        case BTM_BLE_PF_ADDR_FILTER:
            cmd.address = ServiceUtil::AddrToStack(RawAddress(filter.GetDeviceId()));
            if (filter.GetRawAddressType() == RawAddressType::RANDOM_ADDRESS) {
                cmd.addr_type = BLE_ADDR_RANDOM;
            } else if (filter.GetRawAddressType() == RawAddressType::PUBLIC_ADDRESS) {
                cmd.addr_type = BLE_ADDR_PUBLIC;
            } else {
                // 若应用没有设置ble扫描地址过滤器中的地址类型，BTC只过滤地址，不限制地址类型
                cmd.addr_type = BLE_ADDR_ALL_TYPE;
            }
            break;
        case BTM_BLE_PF_SRVC_UUID:
            cmd.uuid = ServiceUtil::UuidToStack(filter.GetServiceUuid());
            cmd.uuid_mask = ServiceUtil::UuidToStack(filter.GetServiceUuidMask());
            break;
        case BTM_BLE_PF_SRVC_SOL_UUID:
            cmd.uuid = ServiceUtil::UuidToStack(filter.GetServiceSolicitationUuid());
            cmd.uuid_mask = ServiceUtil::UuidToStack(filter.GetServiceSolicitationUuidMask());
            break;
        case BTM_BLE_PF_LOCAL_NAME: {
            std::string name = filter.GetName();
            cmd.name = std::vector<uint8_t>(name.begin(), name.end());
            break;
        }
        case BTM_BLE_PF_MANU_DATA:
            cmd.company = filter.GetManufacturerId();
            cmd.company_mask = 0xFFFF;  // default mask all
            cmd.data = filter.GetManufactureData();
            cmd.data_mask = filter.GetManufactureDataMask();
            break;
        case BTM_BLE_PF_SRVC_DATA_PATTERN:
            cmd.data = filter.GetServiceData();
            cmd.data_mask = filter.GetServiceDataMask();
            break;
        default:
            HILOGE("Unknown command: %{public}u", cmdType);
            break;
    }

    return cmd;
}

void BleScanFilter::SetApcfCommand(const BleScanFilterImpl &filter, std::vector<ApcfCommand> &outCmds) const
{
    uint16_t features = GetFeatureSelection(filter);
    if (features & ENABLE_BROADCAST_ADDRESS) {
        ApcfCommand addrCmd = GetCommand(BTM_BLE_PF_ADDR_FILTER, filter);
        if (features & ENABLE_IRK) {
            addrCmd.irk = filter.GetIrkArray();
        }
        outCmds.push_back(addrCmd);
    }
    if (features & ENABLE_SERVICE_UUID) {
        outCmds.push_back(GetCommand(BTM_BLE_PF_SRVC_UUID, filter));
    }
    if (features & ENABLE_SERVICE_SOLICITATION_UUID) {
        outCmds.push_back(GetCommand(BTM_BLE_PF_SRVC_SOL_UUID, filter));
    }
    if (features & ENABLE_LOCAL_NAME) {
        outCmds.push_back(GetCommand(BTM_BLE_PF_LOCAL_NAME, filter));
    }
    if (features & ENABLE_MANUFACTURER_DATA) {
        outCmds.push_back(GetCommand(BTM_BLE_PF_MANU_DATA, filter));
    }
    if (features & ENABLE_SERVICE_DATA) {
        outCmds.push_back(GetCommand(BTM_BLE_PF_SRVC_DATA_PATTERN, filter));
    }

    ApcfCommandLog(outCmds);
}

__attribute__((no_sanitize("cfi")))
bool BleScanFilter::AddScanFilter(int filterIndex, const BleScanFilterImpl &filter,
    uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvNum)
{
    bool ret = false;
    if (!SetScanFilterParameters(filterIndex, filter, deliveryMode, sensitivityMode, trackAdvNum)) {
        HILOGE("set param failed");
        return ret;
    }

    std::vector<ApcfCommand> cmds;
    SetApcfCommand(filter, cmds);

    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bluetoothHwSrcInterface == nullptr) {
        HILOGE("interface nullptr");
        return false;
    }
    if (filter.GetAdvIndReportFlag()) {
        HILOGI("record adv_ind report flag = %{public}d", filterIndex);
    }
    bluetoothHwSrcInterface->hwSetAdvIndReportFlag(filterIndex, filter.GetAdvIndReportFlag());

    ret = SendScanFilter(filterIndex, std::move(cmds));
    return ret;
}

void BleScanFilter::CheckFilter(BleScanFilterImpl &filter)
{
    if (filter.HasServiceUuid() && !filter.HasServiceUuidMask()) {
        Uuid::UUID128Bit mask {};
        mask.fill(0xFF);
        filter.SetServiceUuidMask(Uuid::ConvertFrom128Bits(mask));
    }

    if (filter.HasSolicitationUuid() && !filter.HasSolicitationUuidMask()) {
        Uuid::UUID128Bit mask {};
        mask.fill(0xFF);
        filter.SetServiceSolicitationUuidMask(Uuid::ConvertFrom128Bits(mask));
    }

    auto data = filter.GetManufactureData();
    auto dataMask = filter.GetManufactureDataMask();
    if (data.size() > 0 && data.size() != dataMask.size()) {
        std::vector<uint8_t> newMask(data.size(), 0xFF);
        filter.SetManufactureDataMask(std::move(newMask));
    }

    data = filter.GetServiceData();
    dataMask = filter.GetServiceDataMask();
    if (data.size() > 0 && data.size() != dataMask.size()) {
        std::vector<uint8_t> newMask(data.size(), 0xFF);
        filter.SetServiceDataMask(std::move(newMask));
    }
}

int BleScanFilter::AddScanFilters(const std::vector<BleScanFilterImpl> &filters,
    uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvs)
{
    size_t num = FilterIndex::GetInstance()->GetAvailableNumber();
    if (filters.size() > num) {
        HILOGE("filter index number is not enough, need: %{public}zu, actual: %{public}zu", filters.size(), num);
        return BT_ERR_BLE_SCAN_NO_RESOURCE;
    }
    if (filters.size() == 0) {
        HILOGE("Error, filters is empty");
        return EMPTY_FILTER;
    }

    for (auto filter : filters) {
        CheckFilter(filter);

        auto [index, ok] = FilterIndex::GetInstance()->GetOne();
        if (!ok) {
            HILOGE("get index failed");
            return GATT_FAILURE;
        }

        if (!AddScanFilter(index, filter, deliveryMode, sensitivityMode, trackAdvs)) {
            HILOGE("add failed");
            FilterIndex::GetInstance()->Release(index);
            return GATT_FAILURE;
        }

        filterImpl_.Insert(index, filter);
        std::lock_guard<std::mutex> lock(filterIndexsMutex_);
        filterIndexSet_.insert(index);
    }

    return GATT_SUCCESS;
}

int BleScanFilter::GetScannerId(void) const
{
    return scannerId_;
}

std::set<int> BleScanFilter::GetFilterIndexs(void) const
{
    return filterIndexSet_;
}

int BleScanFilter::RemoveScanFilters(const std::vector<BleScanFilterImpl> &filters)
{
    int removeSize = 0;
    if (filters.size() == 0) {
        HILOGE("filters size 0");
        return removeSize;
    }
    const bthwif_interface_t *bluetoothHwSrcInterface = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bluetoothHwSrcInterface == nullptr) {
        HILOGE("interface nullptr");
        return removeSize;
    }
    for (auto filter : filters) {
        int index = -1;
        filterImpl_.Iterate([&filter, &index](int key, BleScanFilterImpl &tempFilter) {
            if (tempFilter.GetFiltIndex() == filter.GetFiltIndex()) {
                index = key;
            }
        });
        if (index == -1) {
            continue;
        }
        removeSize++;
        HILOGI("filterIdx:%{public}d", index);
        SendScanFilterParam(FILTER_ACTION_DELETE, index, nullptr);
        bluetoothHwSrcInterface->hwDeletAdvIndReportFlag(index);
        FilterIndex::GetInstance()->Release(index);
        filterImpl_.Erase(index);
        std::lock_guard<std::mutex> lock(filterIndexsMutex_);
        filterIndexSet_.erase(index);
    }
    HILOGD("removeSize:%{public}d", removeSize);
    return removeSize;
}

int BleScanFilter::AppendScanFilters(const std::vector<BleScanFilterImpl> &filters, uint8_t deliveryMode,
    uint8_t matchMode, uint16_t trackAdvs)
{
    int addSize = 0;
    size_t num = FilterIndex::GetInstance()->GetAvailableNumber();
    if (filters.size() > num) {
        HILOGE("filter index number is not enough, need: %{public}zu, actual: %{public}zu", filters.size(), num);
        return addSize;
    }
    if (filters.size() == 0) {
        HILOGE("Error, filters is empty");
        return addSize;
    }

    for (auto filter : filters) {
        int index = -1;
        filterImpl_.Iterate([&filter, &index](int key, BleScanFilterImpl &tempFilter) {
            if (tempFilter.GetFiltIndex() == filter.GetFiltIndex()) {
                index = key;
            }
        });
        if (index != -1) {
            HILOGW("filter already exist fitlerIndex:%{public}d", filter.GetFiltIndex());
            return addSize;
        }
    }
    int ret = AddScanFilters(filters, deliveryMode, matchMode, trackAdvs);
    if (ret != GATT_SUCCESS) {
        RemoveScanFilters(filters);
        return addSize;
    } else {
        return filters.size();
    }
}

std::pair<int8_t, int8_t> BleScanFilter::GetRssiThres(uint8_t sensitivityMode, bool onFound) const
{
    // When the RSSI value is greater than the value of this parameter, the device is considered as found.
    int8_t highRssi = RSSI_HIGH_THRESH;
    // When the RSSI value is lower than the value of this parameter, the device is considered as lost.
    int8_t lowRssi = RSSI_HIGH_THRESH;
    // In NORMAL mode, highRssi and lowRssi are both -128.
    if (!onFound) {
        return std::make_pair(highRssi, lowRssi);
    }
    // In SENSITIVITY_MODE_HIGH mode, it is easier to find devices and detect missing devices in a timely manner.
    // highRssi = -128, lowRssi = -80,
    if (sensitivityMode == bluetooth::SENSITIVITY_MODE_HIGH) {
        lowRssi = RSSI_LOW_THRESH;
    } else {
        highRssi = RSSI_LOW_THRESH;
    }
    return std::make_pair(highRssi, lowRssi);
}

uint16_t BleScanFilter::GetOnFoundOrLostTimeout(uint8_t sensitivityMode, bool onFound) const
{
    uint8_t factor;
    uint16_t timeout = ON_FOUND_OR_LOST_BASE_TIMEOUT_MS;
    if (sensitivityMode == bluetooth::SENSITIVITY_MODE_HIGH) {
        factor = SENSITIVITY_MODE_HIGH_TIMEOUT_FACTOR;
    } else {
        factor = SENSITIVITY_MODE_LOW_TIMEOUT_FACTOR;
    }
    if (!onFound) {
        factor *= ON_LOST_FACTOR;
    }
    return (timeout * factor);
}

uint16_t BleScanFilter::GetOnFoundCount(uint8_t sensitivityMode) const
{
    if (sensitivityMode == bluetooth::SENSITIVITY_MODE_HIGH) {
        return SENSITIVITY_MODE_HIGH_COUNT;
    }
    return SENSITIVITY_MODE_LOW_COUNT;
}

BleScanFilter::FilterIndex *BleScanFilter::FilterIndex::GetInstance(void)
{
    static FilterIndex singleton;
    return &singleton;
}

size_t BleScanFilter::FilterIndex::GetAvailableNumber(void)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (currentIndex_ > MAX_ADV_FILTERS) {
        HILOGE("Unknown error");
        return 0;
    }
    return releasedIndexs_.size() + (MAX_ADV_FILTERS - currentIndex_);
}

std::pair<int, bool> BleScanFilter::FilterIndex::GetOne(void)
{
    std::lock_guard<std::mutex> lock(mutex_);

    int index = 0;
    bool ret = true;
    if (!releasedIndexs_.empty()) {
        index = releasedIndexs_.front();
        releasedIndexs_.pop();
    } else if (currentIndex_ < MAX_ADV_FILTERS) {
        index = currentIndex_++;
    } else {
        HILOGE("Not enough filter indexs");
        ret = false;
    }

    return {index, ret};
}

void BleScanFilter::FilterIndex::Release(int index)
{
    std::lock_guard<std::mutex> lock(mutex_);

    releasedIndexs_.push(index);
}

void BleScanFilter::FilterIndex::Release(const std::vector<int> &indexs)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (int index : indexs) {
        releasedIndexs_.push(index);
    }
}

}  // namespace bluetooth
}  // namespace OHOS
