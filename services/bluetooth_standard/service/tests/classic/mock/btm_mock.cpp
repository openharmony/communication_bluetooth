/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "btm.h"
#include "classic_mock.h"

ClassicMock *g_btmmocker;
BtmAclCallbacks g_btmAclCbs;
BtmAclCallbacks *GetAclCallbacks()
{
    return &g_btmAclCbs;
}
void registerBtmMock(ClassicMock *instance)
{
    g_btmmocker = instance;
}

int BTM_Enable(int controller)
{
    return g_btmmocker->BTM_Enable();
}
int BTM_Disable(int controller)
{
    return g_btmmocker->BTM_Disable();
}
int BTM_RegisterAclCallbacks(const BtmAclCallbacks *callbacks, void *context)
{
    g_btmAclCbs = *callbacks;
    return g_btmmocker->BTM_RegisterACLCallbacks();
}
int BTM_DeregisterAclCallbacks(const BtmAclCallbacks *callbacks)
{
    return g_btmmocker->BTM_DeregisterACLCallbacks();
}

int BTM_AclDisconnect(uint16_t connectionHandle, uint8_t reason)
{
    return BT_NO_ERROR;
}