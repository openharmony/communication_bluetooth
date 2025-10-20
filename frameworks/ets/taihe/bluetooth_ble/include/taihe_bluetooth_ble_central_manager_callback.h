/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_BLUETOOTH_BLE_CENTRAL_MANAGER_H
#define TAIHE_BLUETOOTH_BLE_CENTRAL_MANAGER_H

#include "bluetooth_ble_central_manager.h"
#include "taihe_async_callback.h"
#include "taihe_bluetooth_ble_utils.h"

namespace OHOS {
namespace Bluetooth {
class TaiheBluetoothBleCentralManagerCallback : public BleCentralManagerCallback {
public:
    TaiheBluetoothBleCentralManagerCallback(bool isLatestNapiBleScannerObj);
    ~TaiheBluetoothBleCentralManagerCallback() override = default;

    static TaiheBluetoothBleCentralManagerCallback &GetInstance(void);

    void OnScanCallback(const BleScanResult &result) override;
    void OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType) override;
    void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) override;
    void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override;
    void OnNotifyMsgReportFromLpDevice(const UUID &uuid, int msgType, const std::vector<uint8_t> &value) override {};

    void ConvertScanReportType(ScanReportType &scanReportType, uint8_t callbackType);
    TaiheAsyncWorkMap asyncWorkMap_ {};
    bool isLatestNapiBleScannerObj_ = false;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_BLE_CENTRAL_MANAGER_H