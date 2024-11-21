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

FfiBluetoothBleCentralManagerCallback::FfiBluetoothBleCentralManagerCallback()
{
}

FfiBluetoothBleCentralManagerCallback &FfiBluetoothBleCentralManagerCallback::GetInstance(void)
{
    static FfiBluetoothBleCentralManagerCallback instance;
    return instance;
}

void FfiBluetoothBleCentralManagerCallback::RegisterBLEDeviceFindFunc(std::function<void(CArrScanResult)> cjCallback)
{
    bleDeviceFindFunc = cjCallback;
}

void FfiBluetoothBleCentralManagerCallback::OnScanCallback(const BleScanResult &result)
{
    if (bleDeviceFindFunc == nullptr) {
        return;
    }
    CArrScanResult outResults{};
    outResults.size = 1;
    NativeScanResult *resultValue = static_cast<NativeScanResult *>(malloc(sizeof(NativeScanResult) * outResults.size));
    if (resultValue == nullptr) {
        return;
    }
    for (int i = 0; i < outResults.size; i++) {
        BleScanResult bleScanResult = result;
        NativeScanResult nativeResult{};
        nativeResult.deviceId = MallocCString(bleScanResult.GetPeripheralDevice().GetDeviceAddr());
        nativeResult.rssi = bleScanResult.GetRssi();
        nativeResult.data = Convert2CArrUI8(bleScanResult.GetPayload());
        nativeResult.deviceName = MallocCString(bleScanResult.GetName());
        nativeResult.connectable = bleScanResult.IsConnectable();
        resultValue[i] = nativeResult;
    }
    outResults.head = resultValue;
    bleDeviceFindFunc(outResults);

    for (int i = 0; i < outResults.size; i++) {
        NativeScanResult nativeResult = outResults.head[i];
        free(nativeResult.deviceId);
        free(nativeResult.data.head);
        free(nativeResult.deviceName);
        nativeResult.deviceId = nullptr;
        nativeResult.data.head = nullptr;
        nativeResult.deviceName = nullptr;
    }
    free(resultValue);
    resultValue = nullptr;
    return;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS