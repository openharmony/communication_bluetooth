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

#ifndef TAIHE_BLUETOOTH_BLE_SCANNER_H_
#define TAIHE_BLUETOOTH_BLE_SCANNER_H_

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_log.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe_bluetooth_ble_central_manager_callback.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
class BleScannerImpl {
public:
    uintptr_t StartScanPromise(const ohos::bluetooth::ble::ScanFilterNullValue &filters,
        taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options);
    uintptr_t StopScanPromise();

    std::shared_ptr<BleCentralManager> &GetBleCentralManager()
    {
        return bleCentralManager_;
    }

    std::shared_ptr<TaiheBluetoothBleCentralManagerCallback> GetCallback()
    {
        return callback_;
    }

    BleScannerImpl()
    {
        callback_ = std::make_shared<TaiheBluetoothBleCentralManagerCallback>(true);
        bleCentralManager_ = std::make_shared<BleCentralManager>(callback_);
    }
    ~BleScannerImpl() = default;
private:
    std::shared_ptr<BleCentralManager> bleCentralManager_ = nullptr;
    std::shared_ptr<TaiheBluetoothBleCentralManagerCallback> callback_ = nullptr;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_BLE_SCANNER_H_ */
