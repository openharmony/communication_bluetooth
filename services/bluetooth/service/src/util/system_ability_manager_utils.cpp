/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_util_system_ability"
#endif

#include "log.h"

#include "iservice_registry.h"
#include "bluetooth_errorcode.h"
#include "system_ability_definition.h"
#include "system_ability_manager_utils.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

namespace {
    const std::u16string ABILITY_MGR_DESCRIPTOR = u"ohos.aafwk.AbilityManager";
    constexpr int DEFAULT_INVAL_VALUE = -1;
}

int32_t SystemAbilityManagerUtils::StartAbility(AAFwk::Want& want)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
         SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET(systemAbilityManager != nullptr, BT_ERR_INTERNAL_ERROR, "SystemAbilityManager is nullptr");

    sptr<IRemoteObject> remote = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    CHECK_AND_RETURN_LOG_RET(remote != nullptr, BT_ERR_INTERNAL_ERROR, "Remote is nullptr");
 
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR), BT_ERR_INTERNAL_ERROR,
        "Write interface token error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&want), BT_ERR_INTERNAL_ERROR, "Write Want error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(DEFAULT_INVAL_VALUE), BT_ERR_INTERNAL_ERROR, "Write UserId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(DEFAULT_INVAL_VALUE), BT_ERR_INTERNAL_ERROR, "Write RequestCode error");

    MessageParcel reply;
    MessageOption option;
    uint32_t task =  static_cast<uint32_t>(AAFwk::AbilityManagerInterfaceCode::START_ABILITY);
    int ret = remote->SendRequest(task, data, reply, option);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "Send request error");
    return reply.ReadInt32();
}
}  // namespace bluetooth
}