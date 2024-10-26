/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_proxy_manager"
#endif

#include "bluetooth_proxy_manager.h"

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"
#include "i_bluetooth_ble_central_manager.h"

namespace OHOS {
namespace Bluetooth {
BluetoothProxyManager& BluetoothProxyManager::GetInstance()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<BluetoothProxyManager> instance;
    return *instance;
#else
    static BluetoothProxyManager instance;
    return instance;
#endif
}

bool BluetoothProxyManager::FreezeByRss(std::set<int> pidSet, bool isProxy)
{
    if (!IS_BLE_ENABLED()) {
        HILOGD("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy, false, "failed: no proxy");
    return proxy->FreezeByRss(pidSet, isProxy);
}

bool BluetoothProxyManager::ResetAllProxy()
{
    HILOGI("ResetAllProxy start.");
    if (!IS_BLE_ENABLED()) {
        HILOGD("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy, false, "failed: no proxy");
    return proxy->ResetAllProxy();
}
} // namespace Bluetooth
} // namespace OHOS