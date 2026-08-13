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

#ifndef BLUETOOTH_BLE_FILTER_MATCHER_H
#define BLUETOOTH_BLE_FILTER_MATCHER_H

#include "ble_service_data.h"
#include "bluetooth_ble_scan_result.h"
#include "log.h"

namespace OHOS {
namespace Bluetooth {

/**
 * @brief ble fliter match state define
 * use to MatchesScanFilters
 */
enum class FilterCheckState {
    FILTER_CHECK_PASS,
    FILTER_CHECK_FAIL,
};

class BluetoothBleFilterMatcher {
public:
    BluetoothBleFilterMatcher() = default;
    ~BluetoothBleFilterMatcher() = default;
    static FilterCheckState MatchesScanFilters(const std::vector<bluetooth::BleScanFilterImpl> &bleScanFilters,
        const BluetoothBleScanResult &result, bluetooth::BleScanFilterImpl &matchedFilter);
    static FilterCheckState MatchesScanFilter(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesAddress(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesName(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesServiceUuids(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesManufacturerDatas(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesServiceDatas(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);

    static FilterCheckState MatchesRssiThreshold(const bluetooth::BleScanFilterImpl &filter,
        const BluetoothBleScanResult &result);
    static FilterCheckState MatchesLpDeviceFilter(const bluetooth::BleScanFilterImpl &filter);
    static bool MatchesUuidWithMask(bluetooth::Uuid filterUuid, bluetooth::Uuid uuid, bluetooth::Uuid uuidMask);
    static std::string ParseServiceDataUUidToString(bluetooth::Uuid uuid, std::string data);
    static bool MatchesData(std::vector<uint8_t> fData, std::string rData, std::vector<uint8_t> dataMask);
    static FilterCheckState MatchesAdvIndReportFlag(uint16_t eventType, bool advIndReportFlag);
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_BLE_FILTER_MATCHER_H