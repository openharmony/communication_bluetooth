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

#include "classic_mock.h"

ClassicMock *g_gapmocker;
GapDiscoveryCallback g_discoveryCbs;
GapAuthenticationCallback g_authenticationCbs;
GapSecurityCallback g_securityCb;
GapSetScanModeResultCallback g_SetScanModeResultCb;
GapDiscoveryCallback *GetDiscoveryCallback()
{
    return &g_discoveryCbs;
}

GapAuthenticationCallback *GetAuthenticationCallback()
{
    return &g_authenticationCbs;
}
GapSecurityCallback *GetSecurityCallback()
{
    return &g_securityCb;
}
GapSetScanModeResultCallback GetSetScanModeResultCallback()
{
    return g_SetScanModeResultCb;
}

void registerGapMock(ClassicMock *instance)
{
    g_gapmocker = instance;
}

int GAPIF_SetScanMode(const GapDiscoverModeInfo *discoverInfo, const GapConnectableModeInfo *connectableInfo,
    GapSetScanModeResultCallback cb, void *context)
{
    g_SetScanModeResultCb = cb;
    return g_gapmocker->GAPIF_SetScanMode();
}
int GAPIF_SetBondableMode(uint8_t bondableMode)
{
    return BT_NO_ERROR;
}

int GAPIF_GetRemoteName(const BtAddr *addr)
{
    return g_gapmocker->GAPIF_GetRemoteName();
}

int GAPIF_RegisterDiscoveryCallback(const GapDiscoveryCallback *callback, void *context)
{
    g_discoveryCbs = *callback;
    return g_gapmocker->GAPIF_RegisterDiscoveryCallback();
}
int GAPIF_RegisterAuthenticationCallback(const GapAuthenticationCallback *callback, void *context)
{
    g_authenticationCbs = *callback;
    return g_gapmocker->GAPIF_RegisterAuthenticationCallback();
}
int GAPIF_RegisterSecurityCallback(const GapSecurityCallback *callback, void *context)
{
    g_securityCb = *callback;
    return g_gapmocker->GAPIF_RegisterSecurityCallback();
}
int GAPIF_GetLocalAddr(BtAddr *addr)
{
    return BT_NO_ERROR;
}
int GAPIF_SetExtendedInquiryResponse(const uint8_t eir[240])
{
    return BT_NO_ERROR;
}
int GAPIF_SetLocalName(const char *name, int length)
{
    return BT_NO_ERROR;
}
int GAPIF_SetClassOfDevice(uint32_t cod)
{
    return g_gapmocker->GAPIF_SetClassOfDevice();
}

int GAPIF_SetSecurityMode(GAP_SecurityMode mode)
{
    return BT_NO_ERROR;
}

int GAPIF_LinkKeyRsp(const BtAddr *addr, uint8_t accept, const uint8_t linkKey[GAP_LINKKEY_SIZE], uint8_t keyType)
{
    return BT_NO_ERROR;
}
int GAPIF_AuthenticationReq(const BtAddr *addr)
{
    return BT_NO_ERROR;
}

int GAPIF_UserConfirmRsp(const BtAddr *addr, uint8_t accept)
{
    return g_gapmocker->GAPIF_UserConfirmRsp();
}

int GAPIF_UserPasskeyRsp(const BtAddr *addr, uint8_t accept, uint32_t number)
{
    return g_gapmocker->GAPIF_UserPasskeyRsp();
}

int GAPIF_PinCodeRsp(const BtAddr *addr, uint8_t accept, const uint8_t *pinCode, uint8_t pinCodeLength)
{
    return g_gapmocker->GAPIF_PinCodeRsp();
}

int GAPIF_Inquiry(uint8_t mode, uint8_t inquiryLength)
{
    return g_gapmocker->GAPIF_Inquiry();
}

int GAPIF_InquiryCancel(void)
{
    return BT_NO_ERROR;
}

int GAPIF_IOCapabilityRsp(
    const BtAddr *addr, uint8_t accept, uint8_t ioCapability, uint8_t oobDataPresent, uint8_t authReq)
{
    return g_gapmocker->GAPIF_IOCapabilityRsp();
}

int GAPIF_AuthorizeRes(const BtAddr *addr, GAP_Service service, uint8_t accept)
{
    return BT_NO_ERROR;
}

int GAPIF_PairIsFromLocal(const BtAddr *addr, bool *isLocal)
{
    BtAddr btaddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x04}, 0x00};
    if (g_gapmocker->GAPIF_PairIsFromLocal()) {
        *isLocal = true;
    }
    return BT_NO_ERROR;
}

int GAPIF_GetRemoteNameCancel(const BtAddr *addr)
{
    return 0;
}

int GAPIF_DeregisterDiscoveryCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_DeregisterAuthenticationCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_DeregisterSecurityCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_CancelAuthenticationReq(const BtAddr *addr)
{
    return BT_NO_ERROR;
}