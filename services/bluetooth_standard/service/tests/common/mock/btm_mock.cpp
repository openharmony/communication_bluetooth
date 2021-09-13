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

#include "common_mock.h"
#include "log.h"

Mock_BTM *btmMock_ = NULL;
BtmPmCallbackTest *g_block;

void registerPmCallback(BtmPmCallbackTest *newBlock)
{
    g_block = newBlock;
}

BtmPmCallbackTest* getPmCallback()
{
    return g_block;
}

void registerNewMockBTM(Mock_BTM *newInstance)
{
    btmMock_ = newInstance;
}

int BTM_Initialize()
{
    LOG_INFO("[%s]", __func__);
    return btmMock_->BTM_Initialize();
}

int BTM_Close()
{
    return btmMock_->BTM_Close();
}

int BTM_Enable(int controller)
{
    return btmMock_->BTM_Enable(controller);
}

int BTM_Disable(int controller)
{
    return btmMock_->BTM_Disable(controller);
}

int BTM_RegisterCallbacks(const BtmCallbacks *callbacks, void *context)
{
    return BT_NO_ERROR;
}

int BTM_DeregisterCallbacks(const BtmCallbacks *callbacks)
{
    return BT_NO_ERROR;
}

int BTM_SetSnoopFilePath(const char *path, uint16_t length)
{
    return btmMock_->BTM_SetSnoopFilePath();
}

int BTM_EnableSnoopFileOutput(bool filter)
{
    return btmMock_->BTM_EnableSnoopFileOutput();
}

int BTM_DisableSnoopFileOutput()
{
    return BT_NO_ERROR;
}

int BTM_RegisterPmCallbacks(const BtmPmCallbacks *callbacks, void *context)
{
    g_block->callbacks = callbacks;
    g_block->context = context;

    return BT_NO_ERROR;
}

int BTM_DeregisterPmCallbacks(const BtmPmCallbacks *callbacks)
{
    return BT_NO_ERROR;
}

int BTM_RegisterAclCallbacks(const BtmAclCallbacks *callbacks, void *context)
{
    g_block->aclcallbacks = callbacks;
    return BT_NO_ERROR;
}

int BTM_DeregisterAclCallbacks(const BtmAclCallbacks *callbacks)
{
    return BT_NO_ERROR;
}

int BTM_SetSniffSubrating(const BtAddr *addr, const BtmSniffSubrating *subRating)
{
    return BT_NO_ERROR;
}

int BTM_ExitSniffMode(const BtAddr *addr)
{
    return BT_NO_ERROR;
}

int BTM_EnterSniffMode(const BtAddr *addr, const BtmSniffParam *param)
{
    return BT_NO_ERROR;
}

int BTM_SetDefaultLinkPolicySettings(uint16_t linkPolicySettings)
{
    return BT_NO_ERROR;
}

int BTM_EnableHciLogOutput(bool filter)
{
    return BT_NO_ERROR;
}

int BTM_DisableHciLogOutput()
{
    return BT_NO_ERROR;
}

int MockProfileEnable(int controller)
{
    return btmMock_->MockProfileEnable(controller);
}

int MockProfileDisable(int controller)
{
    return btmMock_->MockProfileDisable(controller);
}

int MockGetConnectState(int controller)
{
    return btmMock_->MockGetConnectState(controller);
}
