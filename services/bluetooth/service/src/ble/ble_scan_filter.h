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

#ifndef BLE_SCAN_FILTER_H
#define BLE_SCAN_FILTER_H

#include <mutex>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include "safe_map.h"
#include "base_def.h"
#include "ble_defs.h"
#include "ble_service_data.h"
#include "btif_gatt.h"
#include "bluetooth_hw_interface.h"

namespace OHOS {
namespace bluetooth {

// Get max scan filters, by LE_Get_Vendor_Capabilities_Comman
class BleScanFilter {
public:
    static constexpr int WAIT_TIMEOUT = 2000;  // 2000ms

    class FilterIndex {
    public:
        static FilterIndex *GetInstance(void);
        size_t GetAvailableNumber(void);
        std::pair<int, bool> GetOne(void);
        void Release(int index);
        void Release(const std::vector<int> &indexs);

    private:
        static constexpr uint8_t MAX_ADV_FILTERS = 32;

        std::mutex mutex_ {};
        std::queue<uint8_t> releasedIndexs_ {};
        uint8_t currentIndex_ = 3; // Index 0~2 is used in stack.
    };

    static constexpr uint8_t FILTER_ACTION_ADD      = 0x00;
    static constexpr uint8_t FILTER_ACTION_DELETE   = 0x01;
    static constexpr uint8_t FILTER_ACTION_CLEAR    = 0x02;

    static constexpr uint16_t LIST_LOGIC_TYPE = 0x1111;
    static constexpr uint8_t LOGIC_OR  = 0x00;
    static constexpr uint8_t LOGIC_AND = 0x01;

    static constexpr uint8_t DELIVERY_MODE_IMMEDIATE    = 0x00;
    static constexpr uint8_t DELIVERY_MODE_ON_FOUND     = 0x01;
    static constexpr uint8_t DELIVERY_MODE_BATCHED      = 0x02;

    static constexpr int8_t RSSI_HIGH_THRESH = -128; // -128 dBm
    static constexpr int8_t WALLET_RSSI_HIGH_THRESH = -90; // -90 dBm
    static constexpr int8_t RSSI_LOW_THRESH = -80; // -80 dBm

    static constexpr uint16_t ON_FOUND_OR_LOST_BASE_TIMEOUT_MS = 500; // ms
    static constexpr uint16_t ON_LOST_IMEOUT_MS = 10000; // ms
    static constexpr uint8_t SENSITIVITY_MODE_HIGH_TIMEOUT_FACTOR = 1;
    static constexpr uint8_t SENSITIVITY_MODE_LOW_TIMEOUT_FACTOR = 3;
    static constexpr uint8_t ON_LOST_FACTOR = 2;
    static constexpr uint8_t SENSITIVITY_MODE_HIGH_COUNT = 1;
    static constexpr uint8_t SENSITIVITY_MODE_LOW_COUNT = 4;

    static constexpr uint16_t NUM_OF_ONE_MATCH_TRACK_ADV_TYPE = 1;
    static constexpr uint16_t NUM_OF_FEW_MATCH_TRACK_ADV_TYPE = 2;

    static constexpr uint16_t ENABLE_BROADCAST_ADDRESS          = 1 << 0;
    static constexpr uint16_t ENABLE_SERVICE_DATA_CHANGE        = 1 << 1;
    static constexpr uint16_t ENABLE_SERVICE_UUID               = 1 << 2;
    static constexpr uint16_t ENABLE_SERVICE_SOLICITATION_UUID  = 1 << 3;
    static constexpr uint16_t ENABLE_LOCAL_NAME                 = 1 << 4;
    static constexpr uint16_t ENABLE_MANUFACTURER_DATA          = 1 << 5;
    static constexpr uint16_t ENABLE_SERVICE_DATA               = 1 << 6;
    static constexpr uint16_t ENABLE_IRK                        = 1 << 7;

    explicit BleScanFilter(int scannerId, BleScannerInterface *scanner);
    ~BleScanFilter();

    int AddScanFilters(const std::vector<BleScanFilterImpl> &filters,
        uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvs);
    void DeleteScanFilters(void);
    int GetScannerId(void) const;

    bool operator < (const BleScanFilter &rhs) const
    {
        return scannerId_ < rhs.GetScannerId();
    }

    std::set<int> GetFilterIndexs(void) const;
    int RemoveScanFilters(const std::vector<BleScanFilterImpl> &filters);
    int AppendScanFilters(const std::vector<BleScanFilterImpl> &filters,
        uint8_t deliveryMode, uint8_t matchMode, uint16_t trackAdvs);

private:
    bool AddScanFilter(int filterIndex, const BleScanFilterImpl &filter,
        uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvNum);
    void CheckFilter(BleScanFilterImpl &filter);
    uint16_t GetFeatureSelection(const BleScanFilterImpl &filter) const;
    void SetApcfCommand(const BleScanFilterImpl &filter, std::vector<ApcfCommand> &outCmds) const;
    bool SetScanFilterParameters(int filterIndex, const BleScanFilterImpl &filter,
        uint8_t deliveryMode, uint8_t sensitivityMode, uint16_t trackAdvNum) const;
    ApcfCommand GetCommand(uint8_t cmdType, const BleScanFilterImpl &filter) const;
    void ApcfCommandLog(const std::vector<ApcfCommand> &cmds) const;

    bool SendScanFilterParam(uint8_t action, int filterIndex,
        std::unique_ptr<BtgattFiltParamSetup> filtParam) const;
    bool SendScanFilter(int filterIndex, std::vector<ApcfCommand> cmds) const;
    std::pair<int8_t, int8_t> GetRssiThres(uint8_t sensitivityMode, bool onFound) const;
    uint16_t GetOnFoundOrLostTimeout(uint8_t sensitivityMode, bool onFound) const;
    uint16_t GetOnFoundCount(uint8_t sensitivityMode) const;

    int scannerId_ = 0;
    BleScannerInterface *btifBleScanner_ = nullptr;
    std::mutex filterIndexsMutex_ {};
    std::set<int> filterIndexSet_ {};
    SafeMap<int, BleScanFilterImpl> filterImpl_;
};


}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLE_SCAN_FILTER_H
