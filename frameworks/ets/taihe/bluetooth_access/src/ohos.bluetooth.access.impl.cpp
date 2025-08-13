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
#include "taihe_bluetooth_access_observer.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::access;

std::shared_ptr<TaiheBluetoothAccessObserver> g_accessCallback = TaiheBluetoothAccessObserver::GetInstance();

void RestrictBluetoothSync()
{
    int32_t ret = BluetoothHost::GetDefaultHost().RestrictBluetooth();
    if (ret == BT_ERR_INTERNAL_ERROR) {
        set_business_error(ret, "RestrictBluetoothSync return error");
    }
}

::ohos::bluetooth::access::BluetoothState GetState()
{
    int32_t state = static_cast<int>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    return static_cast<ohos::bluetooth::access::BluetoothState::key_t>(state);
}

void EnableBluetooth()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    if (ret != BT_NO_ERROR) {
        set_business_error(ret, "EnableBluetooth return error");
    }
}

void DisableBluetooth()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->DisableBt();
    if (ret != BT_NO_ERROR) {
        set_business_error(ret, "DisableBluetooth return error");
    }
}

void OnStateChange(::taihe::callback_view<void(::ohos::bluetooth::access::BluetoothState data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_accessCallback->stateChangedObserverLock);
    auto stateChangeCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::access::BluetoothState data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_accessCallback->stateChangedObserverVec;
    // callback重复注册不处理
    if (std::find(callbackVec.begin(), callbackVec.end(), stateChangeCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(stateChangeCb);
}

void OffStateChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::access::BluetoothState data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_accessCallback->stateChangedObserverLock);
    auto& callbackVec = g_accessCallback->stateChangedObserverVec;
    // callback有值，则删除该callback，否则删除vec中注册的所有callback
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::access::BluetoothState data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}
}  // Bluetooth
}  // OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnStateChange(OHOS::Bluetooth::OnStateChange);
TH_EXPORT_CPP_API_OffStateChange(OHOS::Bluetooth::OffStateChange);
TH_EXPORT_CPP_API_RestrictBluetoothSync(OHOS::Bluetooth::RestrictBluetoothSync);
TH_EXPORT_CPP_API_GetState(OHOS::Bluetooth::GetState);
TH_EXPORT_CPP_API_EnableBluetooth(OHOS::Bluetooth::EnableBluetooth);
TH_EXPORT_CPP_API_DisableBluetooth(OHOS::Bluetooth::DisableBluetooth);
// NOLINTEND
