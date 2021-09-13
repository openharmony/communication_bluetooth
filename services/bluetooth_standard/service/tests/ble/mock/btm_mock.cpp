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
#include "ble_mock.h"

BleMock *g_btmmocker;

void registerBtmMock(BleMock *instance)
{
    g_btmmocker = instance;
}
BtmAclCallbacks g_btmAclCallbacks;
BtmAclCallbacks *GetBtmCallbacks()
{
    return &g_btmAclCallbacks;
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
    g_btmAclCallbacks = *callbacks;
    return g_btmmocker->BTM_RegisterAclCallbacks();
}

int BTM_DeregisterAclCallbacks(const BtmAclCallbacks *callbacks)
{
    return g_btmmocker->BTM_DeregisterAclCallbacks();
}

void BTM_SetOwnAddressType(uint32_t ownAddressType)
{}
int BTM_SetLeRandomAddress(const BtAddr *addr)
{
    return BT_NO_ERROR;
}

int BTM_ReadRssi(const BtAddr *addr)
{
    return BT_NO_ERROR;
}

bool BTM_IsControllerSupportBrEdr()
{
    return true;
}

void BTM_SetLePairedDevices(const BtmLePairedDevice *pairedDevices, uint16_t count)
{}
void BTM_AddLePairedDevice(const BtmLePairedDevice *device)
{}
void BTM_RemoveLePairedDevice(const BtAddr *addr)
{}

bool BTM_IsControllerSupportLe2MPhy()
{
    return false;
}

bool BTM_IsControllerSupportLeCodedPhy()
{
    return false;
}

void BTM_SetLocalIdentityResolvingKey(const BtmKey *key)
{}

bool BTM_IsControllerSupportLeExtendedAdvertising()
{
    return g_btmmocker->BTM_IsControllerSupportLeExtendedAdvertising();
}

bool BTM_IsControllerSupportLePeriodicAdvertising()
{
    return false;
}

bool BTM_IsControllerSupportLlPrivacy()
{
    return false;
}

bool BTM_IsControllerSupportLeDataPacketLengthExtension()
{
    return g_btmmocker->BTM_IsControllerSupportLeDataPacketLengthExtension();
}

int BTM_AclDisconnect(uint16_t connectionHandle, uint8_t reason)
{
    return BT_NO_ERROR;
}