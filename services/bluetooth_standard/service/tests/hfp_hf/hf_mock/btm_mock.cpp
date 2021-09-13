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

#include "hf_mock.h"

using namespace bluetooth;
HfBtmMock *gBtmMocker = nullptr;

void registerBtmMocker(HfBtmMock *mocker)
{
    gBtmMocker = mocker;
}

HfBtmMock& GetBtmMocker()
{
    return *gBtmMocker;
}

int BTM_RegisterScoCallbacks(const BtmScoCallbacks *callbacks, void *context)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    gBtmMocker->callbacks = callbacks;
    return gBtmMocker->BTM_RegisterScoCallbacks();
}

int BTM_DeregisterScoCallbacks(const BtmScoCallbacks *callbacks)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_DeregisterScoCallbacks();
}

int BTM_WriteVoiceSetting(uint16_t voiceSetting)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_WriteVoiceSetting();
}

bool BTM_IsSecureConnection(const BtAddr *addr)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_IsSecureConnection();
}

int BTM_CreateEscoConnection(const BtmCreateEscoConnectionParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_CreateEscoConnection();
}

int BTM_DisconnectScoConnection(uint16_t connectionHandle, uint8_t reason)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_DisconnectScoConnection();
}

int BTM_AcceptEscoConnectionRequest(const BtmAcceptEscoConnectionRequestParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_AcceptEscoConnectionRequest();
}

int BTM_CreateScoConnection(const BtmCreateScoConnectionParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_CreateScoConnection();
}

int BTM_AcceptScoConnectionRequest(const BtmAcceptScoConnectionRequestParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_AcceptScoConnectionRequest();
}

int BTM_RejectScoConnectionRequest(const BtmRejectScoConnectionRequestParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_RejectScoConnectionRequest();
}

int BTM_GetLocalSupportedCodecs(BtmLocalSupportedCodecs **localSupportedCodes)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_GetLocalSupportedCodecs();
}

bool BTM_IsControllerSupportEsco()
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gBtmMocker->BTM_IsControllerSupportEsco();
}

void BTM_AddLocalRfcommScnForLogging(uint8_t module, uint8_t scn)
{
}

void BTM_AddRemoteRfcommScnForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{
}

void BTM_RemoveLocalRfcommScnChannelForLogging(uint8_t module, uint8_t scn)
{
}

void BTM_RemoveRemoteRfcommScnChannelForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{
}