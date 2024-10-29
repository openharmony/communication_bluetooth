/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_common.h"
#include "bluetooth_ble_impl.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {

FfiBluetoothBleCentralManagerCallback::FfiBluetoothBleCentralManagerCallback() {}

FfiBluetoothBleCentralManagerCallback &FfiBluetoothBleCentralManagerCallback::GetInstance(void)
{
    static FfiBluetoothBleCentralManagerCallback instance;
    return instance;
}

void FfiBluetoothBleCentralManagerCallback::RegisterBLEDeviceFindFunc(std::function<void(CArrScanResult)> cjCallback)
{
    bleDeviceFindFunc = cjCallback;
}

void FfiBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results)
{
    if (bleDeviceFindFunc == nullptr) {
        return;
    }
    CArrScanResult outResults{};
    if (results.empty()) {
        return;
    }
    size_t size = results.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(NativeScanResult)) {
        return;
    }
    outResults.head = static_cast<NativeScanResult *>(malloc(sizeof(NativeScanResult) * results.size()));
    if (outResults.head == nullptr) {
        return;
    }
    size_t i = 0;
    for (; i < results.size(); i++) {
        BleScanResult bleScanResult = results[i];
        NativeScanResult result{};
        result.deviceId = MallocCString(bleScanResult.GetPeripheralDevice().GetDeviceAddr());
        result.rssi = bleScanResult.GetRssi();
        result.data = Convert2CArrUI8(bleScanResult.GetPayload());
        result.deviceName = MallocCString(bleScanResult.GetName());
        result.connectable = bleScanResult.IsConnectable();
        outResults.head[i] = result;
    }
    outResults.size = static_cast<int64_t>(i);
    bleDeviceFindFunc(outResults);
    return;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS