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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_ble_scanner"
#endif

#include "taihe_bluetooth_ble_scanner.h"

#include "bluetooth_utils.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
uintptr_t BleScannerImpl::StartScanPromise(const ohos::bluetooth::ble::ScanFilterNullValue &filters,
    taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    TAIHE_BT_ASSERT_RETURN(env != nullptr, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.BleScanner.StartScan");
    std::vector<BleScanFilter> scanFilters;
    BleScanSettings settings;
    auto status = CheckBleScanParams(filters, options, scanFilters, settings);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    TAIHE_BT_ASSERT_RETURN(this->GetBleCentralManager() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_ASSERT_RETURN(this->GetCallback() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    auto func = [this, settings, scanFilters]() {
        this->GetBleCentralManager()->SetNewApiFlag();
        // When apps like aibase are in frozen state, enabling flight mode and then turning on Bluetooth can cause the
        // scannerId to become invalid. The C++ interface's scannerId is reset every time scanning is turned off, so
        // the JS interface should check the scannerId's validity before each scan.
        this->GetBleCentralManager()->CheckValidScannerId();
        int ret = this->GetBleCentralManager()->StartScan(settings, scanFilters);
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, haUtils);
    TAIHE_BT_ASSERT_RETURN(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));
    bool success = this->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::BLE_START_SCAN, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

uintptr_t BleScannerImpl::StopScanPromise()
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    TAIHE_BT_ASSERT_RETURN(env != nullptr, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.BleScanner.StopScan");

    TAIHE_BT_ASSERT_RETURN(this->GetBleCentralManager() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_ASSERT_RETURN(this->GetCallback() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));

    auto func = [this]() {
        // When apps like aibase are in frozen state, enabling flight mode and then turning on Bluetooth can cause the
        // scannerId to become invalid. The C++ interface's scannerId is reset every time scanning is turned off, so
        // the JS interface should check the scannerId's validity before each scan.
        this->GetBleCentralManager()->CheckValidScannerId();
        int ret = this->GetBleCentralManager()->StopScan();
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, haUtils);
    TAIHE_BT_ASSERT_RETURN(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));
    bool success = this->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::BLE_STOP_SCAN, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));

    asyncWork->Run();
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}
} // namespace Bluetooth
} // namespace OHOS
