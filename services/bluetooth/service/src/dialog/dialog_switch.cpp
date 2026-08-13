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
#define LOG_TAG "bt_service_dialog_switch"
#endif

#include <algorithm>
#include <string>

#include "ability_connection.h"
#include "ability_connect_callback.h"
#include "bluetooth_dialog.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "extension_manager_client.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "log.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "refbase.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "dialog_switch.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
bool DialogSwitch::RequestBluetoothSwitchDialog(DialogSwitchType type)
{
    const std::string abilityName = "BluetoothSwitchDialog";
    std::string thirdlyBundleName = PermissionManager::GetCallingName();
    std::string connectStr = DialogSwitch::BuildStartCommand(type, thirdlyBundleName);
    HILOGI("The bundlename is %{public}s.", thirdlyBundleName.c_str());
    if (!BluetoothDialog::DialogConnectExtension(connectStr, bundleName, abilityName)) {
        HILOGE("failed to build switch dialog.");
        return false;
    }
    return true;
}

std::string DialogSwitch::BuildStartCommand(DialogSwitchType type, std::string thirdlyBundleName)
{
    std::string types;
    if (type == ENABLE_BLUETOOTH) {
        types = "enable";
    } else {
        types = "disable";
    }
    cJSON* root = cJSON_CreateObject();
    std::string uiType = "sysDialog/common";
    cJSON_AddStringToObject(root, "ability.want.params.uiExtensionType", uiType.c_str());
    cJSON_AddStringToObject(root, "bundleName", thirdlyBundleName.c_str());
    cJSON_AddStringToObject(root, "type", types.c_str());

    std::string result = JsonUtil::GetSimpleFormatString(root);
    if (result.empty()) {
        HILOGE("cJSON_Print error.");
    }
    cJSON_Delete(root);
    HILOGI("cmdData is: %{public}s.", result.c_str());
    return result;
}

} // namespace bluetooth
} // namespace OHOS