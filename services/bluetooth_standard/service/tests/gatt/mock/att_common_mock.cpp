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

#include "src/att/att_common.h"
#include <memory.h>
#include "l2cap/l2cap_le.h"
#include "l2cap/l2cap.h"
#include "alarm.h"
#include "log.h"
static AttConnectInfo g_connectInfo[MAXCONNECT] = {0};
void MOCK_AttCallback(uint16_t connectHandle, uint16_t event, void *eventData, Buffer *buffer, void *context)
{
    connectHandle = 0x13;
    (void)connectHandle;
};
int exAdvCb = 1000;
static AttServerDataCallback g_attServerCallback = {
    .attServerCallback = MOCK_AttCallback,
    .context = (void *)(&exAdvCb),
};
static AttClientDataCallback g_attClientCallback = {
    .attClientCallback = MOCK_AttCallback,
    .context = (void *)(&exAdvCb),
};

void ATT_ClientDataDeregister()
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attClientCallback.attClientCallback = NULL;
    g_attClientCallback.context = NULL;

    return;
}
void ATT_ClientDataRegister(attCallback dataCallback, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attClientCallback.attClientCallback = dataCallback;
    g_attClientCallback.context = context;

    return;
}
void AttGetConnectInfoIndexByConnectHandle(uint16_t connectHandle, uint16_t *index, AttConnectInfo **connect)
{
    LOG_INFO("%s enter, connectHandle = %d", __FUNCTION__, connectHandle);

    uint16_t inindex = 0;

    for (; inindex < MAXCONNECT; ++inindex) {
        if (g_connectInfo[inindex].retGattConnectHandle == connectHandle) {
            break;
        }
    }

    *index = inindex;

    if (inindex != MAXCONNECT) {
        *connect = &g_connectInfo[inindex];

        goto ATTGETCONNECTINFOINDEXBYCONNECTHANDLE_END;
    } else {
        *connect = NULL;
        *index = MAXCONNECT;
        goto ATTGETCONNECTINFOINDEXBYCONNECTHANDLE_END;
    }

ATTGETCONNECTINFOINDEXBYCONNECTHANDLE_END:
    return;
}

void ATT_ServerDataRegister(attCallback dataCallback, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attServerCallback.attServerCallback = dataCallback;
    g_attServerCallback.context = context;

    return;
}

void ATT_ServerDataDeregister()
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attServerCallback.attServerCallback = NULL;
    g_attServerCallback.context = NULL;

    return;
}
AttClientDataCallback *AttGetATTClientCallback()
{
    LOG_INFO("%s enter", __FUNCTION__);

    return &g_attClientCallback;
}
AttServerDataCallback *AttGetATTServerCallback()
{

    return &g_attServerCallback;
}

void AttGetConnectInfoIndexByCid(uint16_t Value, AttConnectInfo **connect)
{
    LOG_INFO("%s enter, cid = %d", __FUNCTION__, Value);

    uint16_t index = 0;

    for (; index < MAXCONNECT; ++index) {
        if (g_connectInfo[index].transportType == BT_TRANSPORT_BR_EDR) {
            if (g_connectInfo[index].AttConnectID.bredrcid == Value) {
                break;
            }
        } else if (g_connectInfo[index].transportType == BT_TRANSPORT_LE) {
            if (g_connectInfo[index].aclHandle == Value) {
                break;
            }
        }
    }

    if (index != MAXCONNECT) {
        *connect = &g_connectInfo[index];
        goto ATTGETCONNECTINFOINDEXBYCID_END;
    } else {
        *connect = NULL;
        goto ATTGETCONNECTINFOINDEXBYCID_END;
    }

ATTGETCONNECTINFOINDEXBYCID_END:
    return;
}

uint16_t ATT_MTUSize(uint16_t connectHandle)
{
    return 23;
}