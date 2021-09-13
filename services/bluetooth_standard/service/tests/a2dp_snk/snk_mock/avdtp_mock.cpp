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

#include "a2dp_snk_mock.h"

MockSnkAvdtp *g_mocker = nullptr;

void registerSnkAvdtpMock(MockSnkAvdtp *instance)
{
    g_mocker = instance;
}

MockSnkAvdtp &GetSnkAvdtpMocker()
{
    return *g_mocker;
}

void AVDT_Register(const AvdtRegisterParam *reg)
{
    return;
}

void AVDT_Deregister(void)
{
    return;
}

uint16_t AVDT_AbortReq(uint16_t handle, uint8_t *tranLabel)
{
    return g_mocker->AVDT_AbortReq();
}

uint16_t AVDT_AbortRsp(uint16_t handle, uint8_t tranLabel, uint8_t errCode)
{
    return g_mocker->AVDT_AbortRsp();
}

uint16_t AVDT_CreateStream(const BtAddr *bdAddr, uint16_t *handle, uint16_t codecIndex)
{
    return g_mocker->AVDT_CreateStream();
}

uint16_t AVDT_RemoveStream(uint16_t handle)
{
    return g_mocker->AVDT_RemoveStream();
}

uint16_t AVDT_DiscoverReq(const BtAddr *bdAddr, uint8_t maxSeps, uint8_t *tranLabel)
{
    return g_mocker->AVDT_DiscoverReq();
}

uint16_t AVDT_DiscoverRsp(BtAddr bdAddr, uint8_t tranLabel, uint8_t errCode, AvdtSepInfo sepInfo[], uint8_t numSeps)
{
    return g_mocker->AVDT_DiscoverRsp();
}

uint16_t AVDT_GetCapReq(const BtAddr *bdAddr, uint8_t acpSeid, uint8_t *tranLabel)
{
    return g_mocker->AVDT_GetCapReq();
}

uint16_t AVDT_GetCapRsp(BtAddr bdAddr, uint8_t tranLabel, uint8_t errCode, AvdtSepConfig sepCap[])
{
    return g_mocker->AVDT_GetCapRsp();
}

uint16_t AVDT_GetAllCapReq(const BtAddr *bdAddr, uint8_t acpSeid, uint8_t *tranLabel)
{
    return g_mocker->AVDT_GetAllCapReq();
}

uint16_t AVDT_GetAllCapRsp(BtAddr bdAddr, uint8_t tranLabel, uint8_t errCode, AvdtSepConfig sepCap[])
{
    return g_mocker->AVDT_GetAllCapRsp();
}

uint16_t AVDT_DelayReq(uint16_t handle, uint8_t *tranLabel, uint16_t delayValue)
{
    return g_mocker->AVDT_DelayReq();
}

uint16_t AVDT_DelayRsp(uint16_t handle, uint8_t tranLabel, uint8_t errCode)
{
    return g_mocker->AVDT_DelayRsp();
}

uint16_t AVDT_OpenReq(uint16_t handle, uint8_t *tranLabel)
{
    return g_mocker->AVDT_OpenReq();
}

uint16_t AVDT_OpenRsp(uint16_t handle, uint8_t tranLabel, uint8_t errCode)
{
    return g_mocker->AVDT_OpenRsp();
}

uint16_t AVDT_SetConfigReq(uint16_t handle, uint8_t seid, AvdtSepConfig *p_sepConfig, uint8_t *tranLabel)
{
    return g_mocker->AVDT_SetConfigReq();
}

uint16_t AVDT_SetConfigRsp(uint16_t handle, uint8_t tranLabel, AvdtCatetory category)
{
    return g_mocker->AVDT_SetConfigRsp();
}

uint16_t AVDT_GetConfigReq(uint16_t handle, uint8_t *tranLabel)
{
    return g_mocker->AVDT_GetConfigReq();
}

uint16_t AVDT_GetConfigRsp(uint16_t handle, uint8_t tranLabel, uint8_t errCode, AvdtSepConfig *cfg)
{
    return g_mocker->AVDT_GetConfigRsp();
}

uint16_t AVDT_StartReq(const uint16_t handles[], uint8_t numHandles, uint8_t *tranLabel)
{
    return g_mocker->AVDT_StartReq();
}

uint16_t AVDT_StartRsp(uint16_t firstHandle, uint8_t tranLabel, uint16_t firstFailHandle, uint8_t errCode)
{
    return g_mocker->AVDT_StartRsp();
}

uint16_t AVDT_SuspendReq(const uint16_t handles[], uint8_t numHandles, uint8_t *tranLabel)
{
    return g_mocker->AVDT_SuspendReq();
}

uint16_t AVDT_SuspendRsp(uint16_t firstHandle, uint8_t tranLabel, uint16_t firstFailHandle, uint8_t errCode)
{
    return g_mocker->AVDT_SuspendRsp();
}

uint16_t AVDT_CloseReq(uint16_t handle)
{
    return g_mocker->AVDT_CloseReq();
}

uint16_t AVDT_CloseRsp(uint16_t handle, uint8_t tranLabel, uint8_t errCode)
{
    return g_mocker->AVDT_CloseRsp();
}

uint16_t AVDT_ReconfigReq(uint16_t handle, AvdtSepConfig *cfg, uint8_t *tranLabel)
{
    return g_mocker->AVDT_ReconfigReq();
}

uint16_t AVDT_ReconfigRsp(uint16_t handle, uint8_t tranLabel, AvdtCatetory category)
{
    return g_mocker->AVDT_ReconfigRsp();
}

uint16_t AVDT_WriteReq(uint16_t handle, const Packet *pkt, uint32_t timeStamp, uint8_t payloadType, uint16_t marker)
{
    return g_mocker->AVDT_WriteReq();
}

uint16_t AVDT_ConnectRsp(const BtAddr *bdAddr, uint8_t errCode)
{
    return g_mocker->AVDT_ConnectRsp();
}

uint16_t AVDT_ConnectReq(const BtAddr *bdAddr, uint8_t role)
{
    return g_mocker->AVDT_ConnectReq();
}

uint16_t AVDT_DisconnectReq(const BtAddr *bdAddr)
{
    return g_mocker->AVDT_DisconnectReq();
}

uint16_t AVDT_DisconnectRsp(const BtAddr *bdAddr, uint8_t errCode)
{
    return g_mocker->AVDT_DisconnectRsp();
}

uint16_t AVDT_GetL2capChannel(uint16_t handle)
{
    return g_mocker->AVDT_GetL2capChannel();
}

void AVDT_Rej(uint16_t handle, const BtAddr *bdAddr, uint8_t cmd, uint8_t tranLabel, AvdtRejErrInfo errInfo)
{
    return;
}

uint16_t AVDT_RegisterLocalSEP(AvdtStreamConfig avdtStreamConfig[], uint8_t number)
{
    return g_mocker->AVDT_RegisterLocalSEP();
}
