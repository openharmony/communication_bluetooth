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

/**
 * @file att_connect.h
 *
 * @brief declare connect function to be called.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef ATT_CONNECT__H
#define ATT_CONNECT__H

#include <stdint.h>
#include "l2cap_le_if.h"
#include "l2cap_if.h"
#include "btstack.h"
#include "list.h"
#include "packet.h"
#include "att/att_common.h"
#include "gap_if.h"

#ifdef __cplusplus
extern "C" {
#endif

// connect and disconnect flag
#define CONNECTFLAG 1
#define DISCONNECTFLAG 2

// connect status
#define DISCONNECTED 0
#define CONNECTING 1
#define CONFIGING 2
#define CONFIGED 3
#define CONNECTED 4
#define CONNECTIND 5

// connect result
#define CONNECTIONSUCCESSFUL 0
#define CONNECTIONPENDING 1

// pending status
#define AUTHENTICATIONPENDING 1
#define AUTHORIZATIONPENDING 2

// le connect role
#define LEROLEMASTER 0
#define LEROLESLAVE 1

enum { BR_EDR_BASIC_MODE, BR_EDR_ENHANCED_RETRANSMISSION_MODE = 3, BR_EDR_STREAMING_MODE };

/**
 * @brief received Le connect response.
 *
 * @param1 addr Indicates the pointer to BtAddr.
 * @param2 aclHandle Indicates the aclHandle.
 * @param3 role Indicates the role.
 * @param4 status Indicates the status.
 * @since 1.0
 * @version 1.0
 */
void AttLeConnected(const BtAddr *addr, uint16_t aclHandle, uint8_t role, uint8_t status);

/**
 * @brief received Le disconnect response.
 *
 * @param1 aclHandle Indicates the aclHandle.
 * @param2 status Indicates the status.
 * @param3 reason Indicates the reason.
 * @since 1.0
 * @version 1.0
 */
void AttLeDisconnected(uint16_t aclHandle, uint8_t status, uint8_t reason);

/**
 * @brief received bredr connect request.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 id Indicates the id.
 * @param3 info Indicates the pointer to L2capConnectionInfo.
 * @param4 lpsm Indicates the lpsm.
 * @param5 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttReceiveConnectionReq(uint16_t lcid, uint8_t id, const L2capConnectionInfo *info, uint16_t lpsm, void *ctx);

/**
 * @brief received bredr connect response.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 info Indicates the pointer to L2capConnectionInfo.
 * @param3 result Indicates the result.
 * @param4 status Indicates the status.
 * @param5 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttReceiveConnectionRsp(
    uint16_t lcid, const L2capConnectionInfo *info, uint16_t result, uint16_t status, void *ctx);

/**
 * @brief received bredr config request.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 id Indicates the id.
 * @param3 cfg Indicates the pointer to L2capConfigInfo.
 * @param4 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttReceiveConfigReq(uint16_t lcid, uint8_t id, const L2capConfigInfo *cfg, void *ctx);

/**
 * @brief received bredr config response.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 cfg Indicates the pointer to L2capConfigInfo.
 * @param3 result Indicates the result.
 * @param4 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttReceiveConfigRsp(uint16_t lcid, const L2capConfigInfo *cfg, uint16_t result, void *ctx);

/**
 * @brief received bredr disconnect request.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 id Indicates the id.
 * @param3 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttReceiveDisconnectionReq(uint16_t lcid, uint8_t id, void *ctx);

/**
 * @brief received bredr disconnect abnormal.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 reason Indicates the reason.
 * @param3 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttDisconnectAbnormal(uint16_t lcid, uint8_t reason, void *ctx);

/**
 * @brief received bredr disconnect response.
 *
 * @param1 lcid Indicates the lcid.
 * @param2 ctx Indicates the pointer to context.
 * @since 1.0
 * @version 1.0
 */
void AttRecvDisconnectionRsp(uint16_t lcid, void *ctx);

/**
 * @brief GAPSecCallback.
 *
 * @param1 result.
 * @param2 serviceInfo.
 * @param2 context.
 * @since 1.0
 * @version 1.0
 */
void GAPSecCallback(uint16_t result, GapServiceSecurityInfo serviceInfo, void *context);

/**
 * @brief Initiatiove disconnect.
 *
 * @param1 connect Handle.
 * @since 1.0
 * @version 1.0
 */
void InitiativeDisconnect(uint16_t connectHandle);

#ifdef __cplusplus
}
#endif

#endif
