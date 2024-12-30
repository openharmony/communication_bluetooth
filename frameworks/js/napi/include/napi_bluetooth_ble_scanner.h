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
#ifndef NAPI_BLUETOOTH_BLE_SCANNER_H_
#define NAPI_BLUETOOTH_BLE_SCANNER_H_

#include "bluetooth_ble_central_manager.h"
#include "napi_bluetooth_ble_central_manager_callback.h"

namespace OHOS {
namespace Bluetooth {

class NapiBleScanner {
public:
    static napi_value CreateBleScanner(napi_env env, napi_callback_info info);
    static void DefineBleScannerJSClass(napi_env env);
    static napi_value BleScannerConstructor(napi_env env, napi_callback_info info);

    static napi_value StartScan(napi_env env, napi_callback_info info);
    static napi_value StopScan(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    std::shared_ptr<BleCentralManager> &GetBleCentralManager()
    {
        return bleCentralManager_;
    }

    std::shared_ptr<NapiBluetoothBleCentralManagerCallback> GetCallback()
    {
        return callback_;
    }

    NapiBleScanner()
    {
        callback_ = std::make_shared<NapiBluetoothBleCentralManagerCallback>(true);
        bleCentralManager_ = std::make_shared<BleCentralManager>(callback_);
    }
    ~NapiBleScanner() = default;

    static thread_local napi_ref consRef_;
private:
    std::shared_ptr<BleCentralManager> bleCentralManager_ = nullptr;
    std::shared_ptr<NapiBluetoothBleCentralManagerCallback> callback_ = nullptr;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_BLUETOOTH_BLE_SCANNER_H_ */