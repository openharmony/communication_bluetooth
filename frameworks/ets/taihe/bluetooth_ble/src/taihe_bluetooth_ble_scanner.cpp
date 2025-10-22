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
// 日志打印，方便调试，不需要提交
// void PrintParseGattServiceData(const TaiheGattService &serviceData)
// {
//     static int count = 0;

//     HILOGI("PrintParseGattServiceData serviceData.serviceUuid: %{public}s, serviceData.isPrimary: %{public}d",
//         serviceData.serviceUuid.ToString().c_str(), serviceData.isPrimary);
//     if (serviceData.characteristics.size() > 0) {
//         for (const auto &c: serviceData.characteristics) {
//             std::string serviceUuid = c.serviceUuid.ToString();
//             std::string characteristicUuid = c.characteristicUuid.ToString();
//             int properties = c.properties;
//             uint16_t characteristicValueHandle = c.characteristicValueHandle;
//             int permissions = c.permissions;
//             HILOGI("PrintParseGattServiceData characteristics.serviceUuid: %{public}s, "
//                    "characteristics.characteristicUuid: %{public}s, characteristics.properties: %{public}d, "
//                    "characteristics.characteristicValueHandle: %{public}d, characteristics.permissions: %{public}d",
//                    serviceUuid.c_str(), characteristicUuid.c_str(), properties,
//                    characteristicValueHandle, permissions);
//             auto u = c.characteristicValue;
//             HILOGI("PrintParseGattServiceData characteristics.characteristicValue: %{public}d, %{public}d, %{public}d, %{public}d, "
//                    "%{public}d, %{public}d, %{public}d, %{public}d",
//                    u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7]);
//             for (const auto &d: c.descriptors) {
//                 std::string serviceUuid = d.serviceUuid.ToString();
//                 std::string characteristicUuid = d.characteristicUuid.ToString();
//                 std::string descriptorUuid = d.descriptorUuid.ToString();
//                 uint16_t descriptorHandle = d.descriptorHandle;
//                 int permissions = d.permissions;
//                 HILOGI("PrintParseGattServiceData descriptors.serviceUuid: %{public}s, descriptors.characteristicUuid: %{public}s, "
//                     "descriptors.descriptorUuid: %{public}s, descriptors.descriptorHandle: %{public}d, descriptors.permissions: %{public}d",
//                     serviceUuid.c_str(), characteristicUuid.c_str(), descriptorUuid.c_str(), descriptorHandle, permissions);
//                 auto dv = d.descriptorValue;
//                 HILOGI("PrintParseGattServiceData descriptors.descriptorValue: %{public}d, %{public}d, %{public}d, %{public}d, "
//                        "%{public}d, %{public}d, %{public}d, %{public}d",
//                        dv[0], dv[1], dv[2], dv[3], dv[4], dv[5], dv[6], dv[7]);
//             }
//         }
//     } else {
//         HILOGI("PrintParseGattServiceData characteristics is empty");
//     }

//     if (serviceData.includeServices.size() > 0) {
//         for (const auto &i: serviceData.includeServices) {
//             HILOGI("PrintParseGattServiceData includeServices --- [%{public}d] ----------", count++);
//             PrintParseGattServiceData(i);
//         }
//     } else {
//         HILOGI("PrintParseGattServiceData includeServices is empty");
//     }
// }

uintptr_t BleScannerImpl::StartScanPromise(const ohos::bluetooth::ble::ScanFilterNullValue &filters,
    taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options)
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    if (env == nullptr) {
        HILOGE("StartScanPromise get_env failed");
        TAIHE_BT_ASSERT_RETURN(env != nullptr, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    }
    HILOGE("StartScanPromise 1");
    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.BleScanner.StartScan");
    std::vector<BleScanFilter> scanFilters;
    BleScanSettings settings;
    auto status = CheckBleScanParams(filters, options, scanFilters, settings);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    HILOGE("StartScanPromise 2");

    TAIHE_BT_ASSERT_RETURN(this->GetBleCentralManager() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    TAIHE_BT_ASSERT_RETURN(this->GetCallback() != nullptr,
        BT_ERR_INVALID_PARAM, reinterpret_cast<uintptr_t>(nullptr));
    HILOGE("StartScanPromise 3");

    auto func = [this, settings, scanFilters]() {
        HILOGE("StartScanPromise 3.1");
        this->GetBleCentralManager()->SetNewApiFlag();
        // When apps like aibase are in frozen state, enabling flight mode and then turning on Bluetooth can cause the
        // scannerId to become invalid. The C++ interface's scannerId is reset every time scanning is turned off, so
        // the JS interface should check the scannerId's validity before each scan.
        HILOGE("StartScanPromise 3.2");
        this->GetBleCentralManager()->CheckValidScannerId();
        HILOGE("StartScanPromise 3.3");
        int ret = this->GetBleCentralManager()->StartScan(settings, scanFilters);
        HILOGE("StartScanPromise 3.4");
        return TaiheAsyncWorkRet(ret);
    };
    HILOGE("StartScanPromise 4");

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(env, nullptr, func, haUtils);
    TAIHE_BT_ASSERT_RETURN(asyncWork != nullptr, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));
    HILOGE("StartScanPromise 5");
    bool success = this->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::BLE_START_SCAN, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, reinterpret_cast<uintptr_t>(nullptr));
    HILOGE("StartScanPromise 6");

    asyncWork->Run();
    HILOGE("StartScanPromise 7");
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}

uintptr_t BleScannerImpl::StopScanPromise()
{
    HILOGI("enter");
    ani_env *env = taihe::get_env();
    if (env == nullptr) {
        HILOGE("StopScanPromise get_env failed");
        return reinterpret_cast<uintptr_t>(nullptr);
    }
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
    HILOGI("StopScanReturnsPromise success: %{public}d", success);

    asyncWork->Run();
    return reinterpret_cast<uintptr_t>(asyncWork->GetRet());
}
} // namespace Bluetooth
} // namespace OHOS
