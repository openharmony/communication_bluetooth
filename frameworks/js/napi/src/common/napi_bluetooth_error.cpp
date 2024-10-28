/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_error"
#endif

#include "napi_bluetooth_error.h"

#include "bluetooth_errorcode.h"
#include "napi_bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"

static const int SDK_VERSION_13 = 13;

namespace OHOS {
namespace Bluetooth {

static std::map<int32_t, std::string> napiErrMsgMap {
    { BtErrCode::BT_ERR_SERVICE_DISCONNECTED, "Service stoped." },
    { BtErrCode::BT_ERR_UNBONDED_DEVICE, "Device is not bonded." },
    { BtErrCode::BT_ERR_INVALID_STATE, "Bluetooth disabled." },
    { BtErrCode::BT_ERR_PROFILE_DISABLED, "Profile not supported." },
    { BtErrCode::BT_ERR_DEVICE_DISCONNECTED, "Device not connected." },
    { BtErrCode::BT_ERR_MAX_CONNECTION, "The maximum number of connections has been reached." },
    { BtErrCode::BT_ERR_INTERNAL_ERROR, "Operation failed" },
    { BtErrCode::BT_ERR_IPC_TRANS_FAILED, "IPC failed." },
    { BtErrCode::BT_ERR_UNAVAILABLE_PROXY, "The value of proxy is a null pointer." },
    { BtErrCode::BT_ERR_PERMISSION_FAILED, "Permission denied." },
    { BtErrCode::BT_ERR_SYSTEM_PERMISSION_FAILED, "Non-system applications are not allowed to use system APIs."},
    { BtErrCode::BT_ERR_PROHIBITED_BY_EDM, "Bluetooth is prohibited by EDM."},
    { BtErrCode::BT_ERR_INVALID_PARAM, "Invalid parameter." },
    { BtErrCode::BT_ERR_API_NOT_SUPPORT, "Capability is not supported." },
    { BtErrCode::BT_ERR_GATT_READ_NOT_PERMITTED, "Gatt read forbiden." },
    { BtErrCode::BT_ERR_GATT_WRITE_NOT_PERMITTED, "Gatt write forbiden." },
    { BtErrCode::BT_ERR_GATT_MAX_SERVER, "Max gatt server." },
    { BtErrCode::BT_ERR_SPP_SERVER_STATE, "SPP server not running." },
    { BtErrCode::BT_ERR_SPP_BUSY, "SPP translate busy." },
    { BtErrCode::BT_ERR_SPP_DEVICE_NOT_FOUND, "Device is not inquired." },
    { BtErrCode::BT_ERR_SPP_IO, "SPP IO error." },
    { BtErrCode::BT_ERR_NO_ACTIVE_HFP_DEVICE, "Active hfp device is not exist." },
    { BtErrCode::BT_ERR_NULL_HFP_STATE_MACHINE, "Hfp state machine is not null." },
    { BtErrCode::BT_ERR_HFP_NOT_CONNECT, "Hfp is not connected." },
    { BtErrCode::BT_ERR_SCO_HAS_BEEN_CONNECTED, "Sco has been connected." },
    { BtErrCode::BT_ERR_VR_HAS_BEEN_STARTED, "Voice recognition has been started." },
    { BtErrCode::BT_ERR_AUDIO_NOT_IDLE, "Audio is not idle." },
    { BtErrCode::BT_ERR_VIRTUAL_CALL_NOT_STARTED, "Virtual call is not started." },
    { BtErrCode::BT_ERR_DISCONNECT_SCO_FAILED, "Disconnect sco failed." },
};

static int GetSdkVersion(void)
{
    int version = SDK_VERSION_13;  // default sdk version is api 13

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr.");
        return version;
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy.");
        return version;
    }
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        HILOGE("Failed to get bundle manager proxy.");
        return version;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        HILOGE("GetBundleInfoForSelf: get fail.");
        return version;
    }

    version = bundleInfo.targetVersion % 100; // %100 to get the real version
    return version;
}

static napi_value GenerateBusinessError(napi_env env, int32_t errCode, const std::string &errMsg)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &businessError);
    napi_set_named_property(env, businessError, "code", code);
    return businessError;
}

std::string GetNapiErrMsg(napi_env env, int32_t errCode)
{
    auto iter = napiErrMsgMap.find(errCode);
    if (iter != napiErrMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        errMessage.append(std::to_string(errCode)).append(": ").append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

void HandleSyncErr(napi_env env, int32_t errCode)
{
    if (errCode == BtErrCode::BT_NO_ERROR) {
        return;
    }

    int ret = -1;
    std::string errMsg = GetNapiErrMsg(env, errCode);
    // In API13 version, the error code type is changed from string to number.
    if (GetSdkVersion() >= SDK_VERSION_13) {
        ret = napi_throw(env, GenerateBusinessError(env, errCode, errMsg));
    } else {
        ret = napi_throw_error(env, std::to_string(errCode).c_str(), errMsg.c_str());
    }
    CHECK_AND_RETURN_LOG(ret == napi_ok, "napi_throw failed, ret: %{public}d", ret);
}
}
}