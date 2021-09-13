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

#include "ag_mock.h"

using namespace bluetooth;
AgRfcommMock *gRfcommMocker = nullptr;

void registerRfcommMocker(AgRfcommMock *mocker)
{
    gRfcommMocker = mocker;
}

AgRfcommMock& GetRfcommMocker()
{
    return *gRfcommMocker;
}

int RFCOMM_RegisterServer(uint8_t scn, uint16_t mtu, uint32_t eventMask, RFCOMM_EventCallback callback, void *context)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    gRfcommMocker->rfcommCb = callback;
    return gRfcommMocker->RFCOMM_RegisterServer();
}

int RFCOMM_DeregisterServer(uint8_t scn, bool isRemoveCallback)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_DeregisterServer();
}

int RFCOMM_FreeServerNum(uint8_t scn)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_FreeServerNum();
}

int RFCOMM_AcceptConnection(uint16_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_AcceptConnection();
}

int RFCOMM_RejectConnection(uint16_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_RejectConnection();
}

uint8_t RFCOMM_AssignServerNum()
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_AssignServerNum();
}

int RFCOMM_ConnectChannel(const RfcommConnectReqInfo *reqInfo, uint16_t *handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_ConnectChannel();
}

int RFCOMM_DisconnectChannel(uint16_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_DisconnectChannel();
}

int RFCOMM_Read(uint16_t handle, Packet **pkt)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_Read(handle, pkt);
}

int RFCOMM_Write(uint16_t handle, Packet *pkt)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_Write();
}

BtAddr RFCOMM_GetPeerAddress(uint16_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gRfcommMocker->RFCOMM_GetPeerAddress();
}