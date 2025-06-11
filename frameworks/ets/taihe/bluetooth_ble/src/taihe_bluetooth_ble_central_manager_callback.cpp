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

#include "taihe_bluetooth_ble_central_manager_callback.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

TaiheBluetoothBleCentralManagerCallback::TaiheBluetoothBleCentralManagerCallback(bool isLatestNapiBleScannerObj)
{
    isLatestNapiBleScannerObj_ = isLatestNapiBleScannerObj;
}

TaiheBluetoothBleCentralManagerCallback &TaiheBluetoothBleCentralManagerCallback::GetInstance(void)
{
    static TaiheBluetoothBleCentralManagerCallback instance(false);
    return instance;
}

void TaiheBluetoothBleCentralManagerCallback::OnScanCallback(const BleScanResult &result)
{
    HILOGD("enter, remote device address: %{public}s", GET_ENCRYPT_ADDR(result.GetPeripheralDevice()));
}

void TaiheBluetoothBleCentralManagerCallback::OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType)
{
    HILOGD("enter, remote device address: %{public}s", GET_ENCRYPT_ADDR(result.GetPeripheralDevice()));
}

void TaiheBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results)
{
    HILOGI("enter, scan result size: %{public}zu", results.size());
}

void TaiheBluetoothBleCentralManagerCallback::OnStartOrStopScanEvent(int resultCode, bool isStartScan)
{
    HILOGI("resultCode: %{public}d, isStartScan: %{public}d", resultCode, isStartScan);
}

void TaiheBluetoothBleCentralManagerCallback::ConvertScanReportType(
    ScanReportType &scanReportType, uint8_t callbackType)
{
}
}  // namespace Bluetooth
}  // namespace OHOS