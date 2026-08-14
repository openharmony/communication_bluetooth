/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_dialog"
#endif

#include <algorithm>
#include <string>

#include "ability_connection.h"
#include "ability_connect_callback.h"
#include "bluetooth_dialog.h"
#include "remote_device_properties.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "extension_manager_client.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "log.h"
#include "raw_address.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
namespace {
    constexpr int32_t DEFAULT_VALUE = -1;
    constexpr const char *CALLING_NAME = "com.ohos.settings";
    constexpr const char *UI_TYPE_KEY = "ability.want.params.uiExtensionType";
    constexpr const char *UI_TYPE_VAL = "sysDialog/common";
    constexpr const char *PARAM_DEVICE_ID = "deviceId";
    constexpr const char *PARAM_DEVICE_NAME = "deviceName";

    const std::map<DialogType, std::string> ABILITY_NAME_MAP = {
        {PBAP_AUTH_DIALOG, "BluetoothPbapPseDialog"},
        {MAP_AUTH_DIALOG, "BluetoothMapAuthDialog"},
        {AUTO_PLAY_AUTH_DIALOG, "BluetoothAutoPlayAuthDialog"},
    };

    std::queue<DialogInfo> g_dialogQueue;
    std::mutex g_queueMutex;
}

bool BluetoothDialog::RequestAuthDialog(DialogInfo &dialog)
{
    HILOGI("device: %{public}s request dialog, type: %{public}d",
        GET_ENCRYPT_STR_ADDR(dialog.address), dialog.type);
    std::lock_guard<std::mutex> lock(g_queueMutex);
    if (g_dialogQueue.empty()) {
        DisplayDialog(dialog);
    }
    g_dialogQueue.push(dialog);
    HILOGI("dialog queue size: %{public}d", g_dialogQueue.size());
    return true;
}

bool BluetoothDialog::DisplayDialog(DialogInfo &dialog)
{
    HILOGI("start display dialog, device: %{public}s, type: %{public}d", GET_ENCRYPT_STR_ADDR(dialog.address),
        dialog.type);
    HITRACE_METER(BT_TRACE_TAG);
    auto it = ABILITY_NAME_MAP.find(dialog.type);
    if (it == ABILITY_NAME_MAP.end()) {
        HILOGE("dialog ability name not exist.");
        return false;
    }
    std::string abilityName = it->second;
    std::string connectStr = BuildStartCommand(dialog.address);
    if (!DialogConnectExtension(connectStr, CALLING_NAME, abilityName)) {
        HILOGE("failed to connect dialog.");
        return false;
    }

    HILOGI("success display dialog");
    // start pbap and map auth timer, dismiss dialog while time out
    // refuse play dialog not need timer
    if (dialog.timer != nullptr) {
        HILOGI("start timer");
        dialog.timer->Start(dialog.timeOutMs);
    }
    return true;
}

bool BluetoothDialog::DismissCurAndShowNext()
{
    std::lock_guard<std::mutex> lock(g_queueMutex);
    if (g_dialogQueue.empty()) {
        return false;
    }

    // remove displayed dialog
    g_dialogQueue.pop();

    if (g_dialogQueue.empty()) {
        HILOGI("g_dialogQueue is empty, not continue");
        return false;
    }

    // go on display the next dialog
    return DisplayDialog(g_dialogQueue.front());
}

std::string BluetoothDialog::BuildStartCommand(const std::string &address)
{
    cJSON* root = cJSON_CreateObject();
    RawAddress rawAddr(address);
    std::string deviceName = RemoteDeviceProperties::GetInstance()->GetDeviceName(rawAddr);
    
    cJSON_AddStringToObject(root, UI_TYPE_KEY, UI_TYPE_VAL);
    cJSON_AddStringToObject(root, PARAM_DEVICE_ID, address.c_str());
    if (!deviceName.empty()) {
        cJSON_AddStringToObject(root, PARAM_DEVICE_NAME, deviceName.c_str());
    } else {
        cJSON_AddStringToObject(root, PARAM_DEVICE_NAME, address.c_str());
    }
    std::string result = JsonUtil::GetSimpleFormatString(root);
    if (result.empty()) {
        HILOGE("cJSON_Print error.");
    }
    cJSON_Delete(root);
    return result;
}

bool BluetoothDialog::DialogConnectExtensionAbility(const AAFwk::Want &want, const std::string commandStr,
    const std::string bundleName, const std::string abilityName)
{
    HITRACE_METER(BT_TRACE_TAG);
    sptr<BluetoothAbilityConnection> connection_ = sptr<BluetoothAbilityConnection> (new (std::nothrow)
        BluetoothAbilityConnection(commandStr, bundleName, abilityName));
    if (connection_ == nullptr) {
        HILOGE("connection_ is nullptr.");
        return false;
    }
    HILOGI("calling pid is : %{public}d, calling uid is: %{public}d, fullTokenId:0x%{public}lx.",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingFullTokenID());
    // reset current callingIdentify to bluetooth. (xxxx -> 1002)
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto ret = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want, connection_, nullptr,
        DEFAULT_VALUE);
    HILOGI("ret is: %{public}d.", ret);
    // set current callingIdentify back. (1002 -> xxxx)
    IPCSkeleton::SetCallingIdentity(identity);
    if (ret != ERR_OK) {
        HILOGE("ret isn't ERR_OK");
        return false;
    }
    return true;
}

bool BluetoothDialog::DialogConnectExtension(const std::string commandStr,
    const std::string bundleName, const std::string abilityName)
{
    HITRACE_METER(BT_TRACE_TAG);
    AAFwk::Want want;
    std::string sceneboardName = "com.ohos.sceneboard";
    std::string abilityNames = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(sceneboardName, abilityNames);
    bool ret = DialogConnectExtensionAbility(want, commandStr, bundleName, abilityName);
    if (!ret) {
        HILOGE("ConnectExtensionAbility failed.");
        return false;
    }
    HILOGI("ConnectExtensionAbility successed.");
    return true;
}
} // namespace bluetooth
} // namespace OHOS