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
#define LOG_TAG "bt_taihe_access"
#endif

#include "ohos.bluetooth.access.proj.hpp"
#include "ohos.bluetooth.access.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "taihe_bluetooth_access_observer.h"
#include "taihe_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<TaiheBluetoothAccessObserver> g_accessCallback = TaiheBluetoothAccessObserver::GetInstance();

void RestrictBluetooth()
{
    HILOGI("enter");
    int32_t ret = BluetoothHost::GetDefaultHost().RestrictBluetooth();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_ERR_INTERNAL_ERROR, ret);
}

::ohos::bluetooth::access::BluetoothState GetState()
{
    HILOGI("enter");
    int32_t state = static_cast<int>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    return ohos::bluetooth::access::BluetoothState::from_value(state);
}

void EnableBluetooth()
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    if (ret == BT_ERR_DIALOG_FOR_USER_CONFIRM) {
        ret = BT_NO_ERROR;
    }
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void DisableBluetooth()
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->DisableBt();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
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

void EnableBluetoothAsync()
{
    HILOGI("enter");
    bool isAsync = true;
    int32_t ret = BluetoothHost::GetDefaultHost().EnableBle("", isAsync);
    HILOGI("EnableBluetoothAsync ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void DisableBluetoothAsync()
{
    HILOGI("enter");
    bool isAsync = true;
    int32_t ret = BluetoothHost::GetDefaultHost().DisableBt("", isAsync);
    HILOGI("DisableBluetoothAsync ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

::taihe::string GetLocalAddress()
{
    HILOGI("enter");
    std::string localAddr = INVALID_MAC_ADDRESS;
    int32_t err = BluetoothHost::GetDefaultHost().GetLocalAddress(localAddr);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, localAddr);

    return localAddr;
}

bool IsValidRandomDeviceId(::taihe::string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, checkRet);

    bool isValid = false;
    std::vector<std::string> deviceIdVec = { remoteAddr };
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::IS_VALID), deviceIdVec, isValid);
    HILOGI("isValidRandomDeviceId ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, isValid);

    return isValid;
}

::taihe::array<::taihe::string> GetPersistentDeviceIds()
{
    bool isValid = false;
    std::vector<std::string> deviceIdVec;
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::GET), deviceIdVec, isValid);
    HILOGI("GetPersistentDeviceIds ret: %{public}d", ret);
    ::taihe::array<::taihe::string> result(taihe::copy_data_t{}, deviceIdVec.data(), deviceIdVec.size());
    TAIHE_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, result);

    return result;
}

void DeletePersistentDeviceId(::taihe::string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    bool isValid = false;
    std::vector<std::string> deviceIdVec = { static_cast<std::string>(deviceId) };
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::DELETE), deviceIdVec, isValid);
    HILOGI("DeletePersistentDeviceId ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void AddPersistentDeviceId(::taihe::string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    bool isValid = false;
    std::vector<std::string> deviceIdVec = { static_cast<std::string>(deviceId) };
    int32_t ret = BluetoothHost::GetDefaultHost().ProcessRandomDeviceIdCommand(
        static_cast<int32_t>(RandomDeviceIdCommand::ADD), deviceIdVec, isValid);
    HILOGI("AddPersistentDeviceId ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void NotifyDialogResult(::ohos::bluetooth::access::NotifyDialogResultParams notifyDialogResultParams)
{
    HILOGI("enter");
    uint32_t dialogType = 0;
    if (notifyDialogResultParams.dialogType.is_valid()) {
        dialogType = notifyDialogResultParams.dialogType.get_value();
    }
    bool dialogResult = notifyDialogResultParams.dialogResult;
    int32_t ret = BluetoothHost::GetDefaultHost().NotifyDialogResult(dialogType, dialogResult);
    HILOGI("NotifyDialogResult err: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void FactoryResetSync()
{
    HILOGD("enter");
    int32_t ret = BluetoothHost::GetDefaultHost().BluetoothFactoryReset();
    HILOGI("factoryReset ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}
}  // Bluetooth
}  // OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnStateChange(OHOS::Bluetooth::OnStateChange);
TH_EXPORT_CPP_API_OffStateChange(OHOS::Bluetooth::OffStateChange);
TH_EXPORT_CPP_API_RestrictBluetooth(OHOS::Bluetooth::RestrictBluetooth);
TH_EXPORT_CPP_API_GetState(OHOS::Bluetooth::GetState);
TH_EXPORT_CPP_API_EnableBluetooth(OHOS::Bluetooth::EnableBluetooth);
TH_EXPORT_CPP_API_DisableBluetooth(OHOS::Bluetooth::DisableBluetooth);
TH_EXPORT_CPP_API_EnableBluetoothAsync(OHOS::Bluetooth::EnableBluetoothAsync);
TH_EXPORT_CPP_API_DisableBluetoothAsync(OHOS::Bluetooth::DisableBluetoothAsync);
TH_EXPORT_CPP_API_GetLocalAddress(OHOS::Bluetooth::GetLocalAddress);
TH_EXPORT_CPP_API_IsValidRandomDeviceId(OHOS::Bluetooth::IsValidRandomDeviceId);
TH_EXPORT_CPP_API_GetPersistentDeviceIds(OHOS::Bluetooth::GetPersistentDeviceIds);
TH_EXPORT_CPP_API_DeletePersistentDeviceId(OHOS::Bluetooth::DeletePersistentDeviceId);
TH_EXPORT_CPP_API_AddPersistentDeviceId(OHOS::Bluetooth::AddPersistentDeviceId);
TH_EXPORT_CPP_API_NotifyDialogResult(OHOS::Bluetooth::NotifyDialogResult);
TH_EXPORT_CPP_API_FactoryResetSync(OHOS::Bluetooth::FactoryResetSync);
// NOLINTEND
