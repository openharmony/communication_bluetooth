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

#include "att_connect_mock.h"
#include <string.h>
#include "log.h"
#include "src/att/att_receive.h"
static AttConnectedCallback g_attConnect;
static AttConnectedCallback *AttGetATTConnectCallback_mock();

void ATT_ConnectDeregister()
{
    LOG_INFO("%s enter", __FUNCTION__);

    AttConnectedCallback *attConnectCallback = AttGetATTConnectCallback_mock();

    attConnectCallback->attConnect.attLEConnectCompleted = NULL;
    attConnectCallback->attConnect.attLEDisconnectCompleted = NULL;
    attConnectCallback->attConnect.attBREDRConnectCompleted = NULL;
    attConnectCallback->attConnect.attBREDRDisconnectCompleted = NULL;
    attConnectCallback->attConnect.attBREDRConnectInd = NULL;
    attConnectCallback->context = NULL;

    return;
}
void ATT_ConnectRegister(AttConnectCallback connectBack, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);
    g_attConnect.attConnect = connectBack;
    g_attConnect.context = context;
    return;
}
void ATT_ConnectRsp(uint16_t connectHandle, uint16_t result, uint16_t status, const AttConnect *cfg)
{}
void ATT_ConnectReq(
    uint8_t transportType, const AttConnect *connParaPtr, const BtAddr *btAddress, const uint16_t *bredrconnectHandle)
{}
void ATT_DisconnectReq(uint16_t connectHandle)
{}

void AttConnectCompletedCallback(AttConnectingInfo *connecting, uint16_t result)
{
    AttBredrConnectCallback bredrConnectCallback{
        .addr{.addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .type = 220}, .status = 0x49};
    AttConnectedCallback *attConnectCallback = AttGetATTConnectCallback_mock();
    connecting->connectHandle = 0;

    attConnectCallback->attConnect.attBREDRConnectCompleted(
        connecting->connectHandle, &bredrConnectCallback, attConnectCallback->context);
}

void AttLeConnected(const BtAddr *addr, uint16_t aclHandle, uint8_t role, uint8_t status)
{
    AttLeConnectCallback leConnectCallbackParaObj;
    AttConnectInfo *connect = new AttConnectInfo();
    AttConnectedCallback *attConnectCallback = AttGetATTConnectCallback_mock();
    connect->retGattConnectHandle = 0;

    attConnectCallback->attConnect.attLEConnectCompleted(
        connect->retGattConnectHandle, &leConnectCallbackParaObj, attConnectCallback->context);
    delete connect;
}

static AttConnectedCallback *AttGetATTConnectCallback_mock()
{
    return &g_attConnect;
}

void GAPSecCallback(uint16_t result, GapServiceSecurityInfo serviceInfo, void *context)
{
    AttConnectingInfo *connecting = NULL;
    AttConnectedCallback *attConnectCallback = NULL;
    AttBredrConnectCallback bredrConnectCallback;

    connecting = (AttConnectingInfo *)context;
    attConnectCallback = AttGetATTConnectCallback_mock();

    attConnectCallback->attConnect.attBREDRConnectInd(connecting->connectHandle, attConnectCallback->context);

    return;
}
void AttLeDisconnected(uint16_t aclHandle, uint8_t status, uint8_t reason)
{
    AttLeDisconnectCallback leDisConnCallbackObj;
    AttConnectInfo *connect = new AttConnectInfo();
    AttConnectedCallback *attConnectCallback = AttGetATTConnectCallback_mock();
    connect->retGattConnectHandle = 0;

    leDisConnCallbackObj.reason = reason;
    attConnectCallback->attConnect.attLEDisconnectCompleted(
        connect->retGattConnectHandle, &leDisConnCallbackObj, attConnectCallback->context);
    delete connect;
}

void AttReceiveConnectionRsp(
    uint16_t lcid, const L2capConnectionInfo *info, uint16_t result, uint16_t status, void *ctx)
{
    AttConnectingInfo *connecting = new AttConnectingInfo();

    result = BREDR_CONNECT_FAIL;
    AttConnectCompletedCallback(connecting, result);
    delete connecting;
}

void AttRecvDisconnectionRsp(uint16_t lcid, void *ctx)
{
    AttBredrDisconnectCallback bredrDisConnObj{.reason = 0x55};
    AttConnectedCallback *attConnectCallback = AttGetATTConnectCallback_mock();
    AttConnectInfo *connect = new AttConnectInfo();
    connect->retGattConnectHandle = 0;

    attConnectCallback->attConnect.attBREDRDisconnectCompleted(
        connect->retGattConnectHandle, &bredrDisConnObj, attConnectCallback->context);
    delete connect;
}

int ATT_LeConnectCancel(const BtAddr *addr)
{
    return BT_NO_ERROR;
}
