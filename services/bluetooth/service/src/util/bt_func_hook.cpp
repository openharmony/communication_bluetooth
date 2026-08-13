/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bt_func_hook.h"
#include "adapter_config.h"
#ifdef __cplusplus
extern "C" {
#endif
/**********  Function hook  ***************/
BT_FUNC_HOOK void OnBluetoothOnHook(void)
{}

BT_FUNC_HOOK void OnBluetoothOffHook(void)
{}

BT_FUNC_HOOK void OnBluetoothInitHook(void)
{}

BT_FUNC_HOOK bool IsAutoConnectEnabledHook(void)
{
    return true;
}

BT_FUNC_HOOK void OnAclDisconnectReasonHook(const OHOS::bluetooth::RawAddress &device, int reason)
{
    (void)device;
    (void)reason;
}

BT_FUNC_HOOK bool IsAllowHfpAgCallHook(std::string address)
{
    return true;
}

BT_FUNC_HOOK bool IsAllowConnectHfpAgScoHook(void)
{
    return true;
}

BT_FUNC_HOOK const char* GetBleMinConnectionIntervalHook(void)
{
    return OHOS::bluetooth::PROPERTY_HIGH_PRIORITY_MIN_INTERVAL.c_str();
}

BT_FUNC_HOOK bool IsEnableBluetoothAfterExitStrModeHook(void)
{
    return false;
}

BT_FUNC_HOOK bool IsAllowPairDialogHook(const OHOS::bluetooth::RawAddress &device)
{
    (void)device;
    return true;
}

BT_FUNC_HOOK int StrModeAbnormalExitHandleHook(const OHOS::bluetooth::BTTransport transport)
{
    return 0;
}

BT_FUNC_HOOK void OnBluetoothDestroyHook(void)
{
    return;
}

BT_FUNC_HOOK bool QuickStopOffloadPlayHook(void)
{
    return false;
}

BT_FUNC_HOOK void BluetoothTurningOffHook(void)
{
    return;
}

BT_FUNC_HOOK bool StartDialHook(std::string address, const int hfpState, const std::string &number)
{
    return true;
}
/**********  Function hook  ***************/
#ifdef __cplusplus
}
#endif
