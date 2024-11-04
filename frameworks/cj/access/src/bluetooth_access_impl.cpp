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
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_access_impl"
#endif

#include "bluetooth_access_impl.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothAccess {
using Bluetooth::BluetoothHost;
using Bluetooth::BTStateID;
using Bluetooth::BluetoothState;

void AccessImpl::EnableBluetooth(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->EnableBle();
    return;
}

void AccessImpl::DisableBluetooth(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->DisableBt();
    return;
}

int32_t AccessImpl::GetState(int32_t* errCode)
{
    int32_t state = static_cast<int>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    *errCode = Bluetooth::BT_NO_ERROR;
    return state;
}
} // namespace CJBluetoothAccess
} // namespace CJSystemapi
} // namespace OHOS