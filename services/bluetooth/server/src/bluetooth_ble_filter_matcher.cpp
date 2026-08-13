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
#define LOG_TAG "bt_server_ble_filter"
#endif

#include "bluetooth_ble_filter_matcher.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

constexpr uint8_t BLE_EVT_SCANNABLE_BIT = 1;
constexpr uint8_t BLE_EVT_SCAN_RESPONSE_BIT = 3;

FilterCheckState BluetoothBleFilterMatcher::MatchesScanFilters(
    const std::vector<bluetooth::BleScanFilterImpl> &bleScanFilters, const BluetoothBleScanResult &result,
    bluetooth::BleScanFilterImpl &matchedFilter)
{
    //no Filter equals all result pass
    if (bleScanFilters.empty()) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }

    for (const auto &filter : bleScanFilters) {
        if (MatchesScanFilter(filter, result) == FilterCheckState::FILTER_CHECK_PASS) {
            matchedFilter = filter;
            return MatchesAdvIndReportFlag(result.GetEventType(), filter.GetAdvIndReportFlag());
        }
    }
    return FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesScanFilter(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    if (MatchesAddress(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesName(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesServiceUuids(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesManufacturerDatas(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesServiceDatas(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesRssiThreshold(filter, result) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    if (MatchesLpDeviceFilter(filter) == FilterCheckState::FILTER_CHECK_FAIL) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    return FilterCheckState::FILTER_CHECK_PASS;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesAddress(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    std::string filterAddress = filter.GetDeviceId();
    //no Filter equals all result pass
    if (filterAddress.empty()) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }

    std::string resultAddress = result.GetPeripheralDevice().GetAddress();
    if (resultAddress.empty()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    return filterAddress == resultAddress ? FilterCheckState::FILTER_CHECK_PASS :  FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesName(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    std::string filterName = filter.GetName();
    //no Filter equals all result pass
    if (filterName.empty()) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }

    std::string resultName = result.GetName();
    //if filter needs this but result is empty means fail.
    if (resultName.empty()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    return resultName == filterName ? FilterCheckState::FILTER_CHECK_PASS :  FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesRssiThreshold(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    // no Filter equals all result pass
    if (!filter.HasRssiThreshold()) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }
    int32_t rssiThreshold = filter.GetRssiThreshold();
    int32_t resultRssi = result.GetRssi();
    return resultRssi >= rssiThreshold ? FilterCheckState::FILTER_CHECK_PASS :  FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesLpDeviceFilter(const bluetooth::BleScanFilterImpl &filter)
{
    if (filter.GetLpDeviceFlag()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }
    return FilterCheckState::FILTER_CHECK_PASS;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesServiceUuids(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    //no Filter equals all result pass
    if (!filter.HasServiceUuid()) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }
    bluetooth::Uuid filterUuid = filter.GetServiceUuid();

    std::vector<bluetooth::Uuid> resultUuids = result.GetServiceUuids();
    //if filter needs this but result is empty means fail.
    if (resultUuids.empty()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    for (auto &uuid : resultUuids) {
        //no mask means filter = result
        if (!filter.HasServiceUuidMask()) {
            if (filterUuid.operator == (uuid)) {
                return FilterCheckState::FILTER_CHECK_PASS;
            }
        }
        //mask means filter&&mask = result&&mask
        if (filter.HasServiceUuidMask()) {
            bluetooth::Uuid uuidMask = filter.GetServiceUuidMask();
            if (MatchesUuidWithMask(filterUuid, uuid, uuidMask)) {
                return FilterCheckState::FILTER_CHECK_PASS;
            }
        }
    }
    return FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesManufacturerDatas(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    uint16_t filterManufacturerId = filter.GetManufacturerId();
    std::vector<uint8_t> filterData = filter.GetManufactureData();
    std::vector<uint8_t> filterDataMask = filter.GetManufactureDataMask();
    //no Filter equals all result pass
    if (filterData.size() == 0) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }
    //if filter needs this but result is empty means fail.
    if (result.GetManufacturerData().empty()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }

    for (auto &resultManufacturerData : result.GetManufacturerData()) {
        //if ManufacturerId same, then check data
        if (filterManufacturerId == resultManufacturerData.first) {
            bool result = MatchesData(filterData, resultManufacturerData.second, filterDataMask);
            return result ? FilterCheckState::FILTER_CHECK_PASS : FilterCheckState::FILTER_CHECK_FAIL;
        }
    }
    return FilterCheckState::FILTER_CHECK_FAIL;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesServiceDatas(const bluetooth::BleScanFilterImpl &filter,
    const BluetoothBleScanResult &result)
{
    std::vector<uint8_t> filterData = filter.GetServiceData();
    std::vector<uint8_t> dataMask = filter.GetServiceDataMask();
    //no Filter equals all result pass
    if (filterData.size() == 0) {
        return FilterCheckState::FILTER_CHECK_PASS;
    }
    //if filter needs this but result is empty means fail.
    if (result.GetServiceData().empty()) {
        return FilterCheckState::FILTER_CHECK_FAIL;
    }
    for (auto &serviceData : result.GetServiceData()) {
        std::string resultData = ParseServiceDataUUidToString(serviceData.first, serviceData.second);
        if (MatchesData(filterData, resultData, dataMask)) {
            return FilterCheckState::FILTER_CHECK_PASS;
        }
    }
    return FilterCheckState::FILTER_CHECK_FAIL;
}

bool BluetoothBleFilterMatcher::MatchesUuidWithMask(bluetooth::Uuid filterUuid, bluetooth::Uuid uuid,
    bluetooth::Uuid uuidMask)
{
    uint8_t uuid128[bluetooth::Uuid::UUID128_BYTES_TYPE];
    uint8_t uuidMask128[bluetooth::Uuid::UUID128_BYTES_TYPE];
    uint8_t resultUuid128[bluetooth::Uuid::UUID128_BYTES_TYPE];
    if (!filterUuid.ConvertToBytesLE(uuid128, bluetooth::Uuid::UUID128_BYTES_TYPE)) {
        HILOGE("Convert filter uuid faild.");
        return false;
    }
    if (!uuidMask.ConvertToBytesLE(uuidMask128, bluetooth::Uuid::UUID128_BYTES_TYPE)) {
        HILOGE("Convert uuid mask faild.");
        return false;
    }
    if (!uuid.ConvertToBytesLE(resultUuid128, bluetooth::Uuid::UUID128_BYTES_TYPE)) {
        HILOGE("Convert result uuid faild.");
        return false;
    }
    size_t maskLength = sizeof(uuidMask128);

    for (size_t i = 0; i < maskLength; i++) {
        if ((uuid128[i] & uuidMask128[i]) != (resultUuid128[i] & uuidMask128[i])) {
            return false;
        }
    }

    return true;
}

std::string BluetoothBleFilterMatcher::ParseServiceDataUUidToString(bluetooth::Uuid uuid, std::string data)
{
    std::string tmpServcieData;
    int uuidType = uuid.GetUuidType();
    switch (uuidType) {
        case bluetooth::Uuid::UUID16_BYTES_TYPE: {
            uint16_t uuid16 = uuid.ConvertTo16Bits();
            tmpServcieData = std::string(reinterpret_cast<char *>(&uuid16), BLE_UUID_LEN_16);
            break;
        }
        case bluetooth::Uuid::UUID32_BYTES_TYPE: {
            uint32_t uuid32 = uuid.ConvertTo32Bits();
            tmpServcieData = std::string(reinterpret_cast<char *>(&uuid32), BLE_UUID_LEN_32);
            break;
        }
        case bluetooth::Uuid::UUID128_BYTES_TYPE: {
            uint8_t uuid128[bluetooth::Uuid::UUID128_BYTES_TYPE];
            if (!uuid.ConvertToBytesLE(uuid128, bluetooth::Uuid::UUID128_BYTES_TYPE)) {
                HILOGE("Convert filter uuid faild.");
            }
            tmpServcieData = std::string(reinterpret_cast<char *>(&uuid128), BLE_UUID_LEN_128);
            break;
        }
        default:
            break;
    }
    return tmpServcieData + data;
}

bool BluetoothBleFilterMatcher::MatchesData(std::vector<uint8_t> filterData, std::string resultData,
    std::vector<uint8_t> dataMask)
{
    if (resultData.empty()) {
        return false;
    }
    size_t length = filterData.size();
    std::vector<uint8_t> resultVec(resultData.begin(), resultData.end());
    if (resultVec.size() < length) {
        return false;
    }
    if (dataMask.empty() || dataMask.size() != length) {
        for (size_t i = 0; i < length; i++) {
            if (filterData[i] != resultVec[i]) {
                return false;
            }
        }
        return true;
    }
    for (size_t i = 0; i < length; i++) {
        if ((filterData[i] & dataMask[i]) != (resultVec[i] & dataMask[i])) {
            return false;
        }
    }
    return true;
}

FilterCheckState BluetoothBleFilterMatcher::MatchesAdvIndReportFlag(uint16_t eventType, bool advIndReportFlag)
{
    // ADV_IND + flag true --> PASS
    // ADV_SCAN_IND + flag true --> PASS
    // ADV_IND + flag false --> NO PASS
    // ADV_SCAN_IND + flag false --> NO PASS
    // ADV_NONCONN_IND + flag true/false --> PASS
    // ADV_DIRECT_IND + flag true/false --> PASS
    // SCAN_RSP + flag true/false --> PASS
    // SCAN_RSP to ADV_SCAN_IND + flag true/false --> PASS
    bool isScannable = eventType & (1 << BLE_EVT_SCANNABLE_BIT);
    bool isScanRsp = eventType & (1 << BLE_EVT_SCAN_RESPONSE_BIT);
    if (isScannable && (!isScanRsp)) {
        if (advIndReportFlag == true) {
            HILOGD("Matches Adv_Ind Flag, need to report");
            return FilterCheckState::FILTER_CHECK_PASS;
        } else {
            return FilterCheckState::FILTER_CHECK_FAIL;
        }
    }
    return FilterCheckState::FILTER_CHECK_PASS;
}

}  // namespace Bluetooth
}  // namespace OHOS