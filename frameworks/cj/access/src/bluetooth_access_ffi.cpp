/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
#define LOG_TAG "bt_cj_access_ffi"
#endif

#include "bluetooth_access_ffi.h"
#include "bluetooth_access_impl.h"
#include "bluetooth_log.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothAccess {

extern "C" {
void FfiBluetoothAccEnableBluetooth(int32_t* errCode)
{
    return AccessImpl::EnableBluetooth(errCode);
}

void FfiBluetoothAccDisableBluetooth(int32_t* errCode)
{
    return AccessImpl::DisableBluetooth(errCode);
}

int32_t FfiBluetoothAccGetState(int32_t* errCode)
{
    return AccessImpl::GetState(errCode);
}

void FfiBluetoothAccOn(int32_t callbackType, void (*callback)(), int32_t* errCode)
{
    return AccessImpl::RegisterAccessObserver(callbackType, callback, errCode);
}
}
} // namespace BluetoothAccess
} // namespace CJSystemapi
} // namespace OHOS