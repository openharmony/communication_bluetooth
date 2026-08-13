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
#define LOG_TAG "bt_service_dialog_pair"
#endif

#include <algorithm>
#include <string>

#include "ability_connection.h"
#include "ability_connect_callback.h"
#include "bluetooth_dialog.h"
#include "bool_wrapper.h"
#include "common_util.h"
#include "double_wrapper.h"
#include "extension_manager_client.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "log.h"
#include "raw_address.h"
#include "refbase.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "dialog_pair.h"
#include "bt_func_hook.h"

namespace OHOS {
namespace bluetooth {
    constexpr int32_t PINCODE_SIZE = 6;
bool DialogPair::RequestBluetoothPairDialog(const RawAddress &device, int reqType, int number)
{
    HILOG_COMM_INFO("Pair dialog");
    if (!IsAllowPairDialogHook(device)) {
        return false;
    }
    return true;
}

std::string DialogPair::BuildStartCommand(const RawAddress &device, int reqType, int number)
{
    cJSON* root = cJSON_CreateObject();
    std::string uiType = "sysDialog/common";
    std::string pinNum = std::to_string(number);
    while (pinNum.size() < PINCODE_SIZE) {
        pinNum = "0" + pinNum;
    }
    cJSON_AddStringToObject(root, "ability.want.params.uiExtensionType", uiType.c_str());
    cJSON_AddStringToObject(root, "deviceId", device.GetAddress().c_str());
    cJSON_AddStringToObject(root, "pinCode", pinNum.c_str());
    cJSON_AddNumberToObject(root, "pinType", reqType);
    HILOGI("address: %{public}s, pinCode: %{public}s, pinType: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), GetEncryptPinCode(pinNum).c_str(), reqType);
    std::string result = JsonUtil::GetSimpleFormatString(root);
    if (result.empty()) {
        HILOGE("cJSON_Print error.");
    }
    cJSON_Delete(root);
    return result;
}
} // namespace bluetooth
} // namespace OHOS
