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

#include "ohos.bluetooth.access.proj.hpp"
#include "ohos.bluetooth.access.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

using namespace taihe;
using namespace ohos::bluetooth::access;

namespace {
// To be implemented.

void RestrictBluetoothSync()
{
    int32_t ret = OHOS::Bluetooth::BluetoothHost::GetDefaultHost().RestrictBluetooth();
    if (ret == OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR) {
        set_business_error(ret, "RestrictBluetoothSync return error");
    }
}

BluetoothState GetState()
{
    int32_t state = static_cast<int>(OHOS::Bluetooth::BluetoothHost::GetDefaultHost().GetBluetoothState());
    return static_cast<BluetoothState::key_t>(state);
}

void EnableBluetooth()
{
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    if (ret != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(ret, "EnableBluetooth return error");
    }
}

void DisableBluetooth()
{
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int32_t ret = host->DisableBt();
    if (ret != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(ret, "DisableBluetooth return error");
    }
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_RestrictBluetoothSync(RestrictBluetoothSync);
TH_EXPORT_CPP_API_GetState(GetState);
TH_EXPORT_CPP_API_EnableBluetooth(EnableBluetooth);
TH_EXPORT_CPP_API_DisableBluetooth(DisableBluetooth);
// NOLINTEND
