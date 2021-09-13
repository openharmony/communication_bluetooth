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

#include "socket_gap_mock.h"
#include "transport_def.h"

namespace bluetooth {
// Require the connection to be encrypted.
static const int SOCK_FLAG_ENCRYPTION = 1;
// Require the connection to be authenticated.
static const int SOCK_FLAG_AUTHENTICATION = 1 << 1;
int SocketGapMock::RegisterServiceSecurity(const BtAddr *addr, GAP_SecMultiplexingProtocol protocolId, uint16_t scn,
    bool isServer, int securityFlag, GAP_Service serviceId)
{
    GapSecChannel secChan{scn};
    GAP_ServiceConnectDirection connectDirection;
    int securityLevel = 0;

    connectDirection = isServer ? INCOMING : OUTGOING;

    if (securityFlag & SOCK_FLAG_ENCRYPTION) {
        securityLevel |= isServer ? GAP_SEC_IN_ENCRYPTION : GAP_SEC_OUT_ENCRYPTION;
    }
    if (securityFlag & SOCK_FLAG_AUTHENTICATION) {
        securityLevel |= isServer ? GAP_SEC_IN_AUTHENTICATION : GAP_SEC_OUT_AUTHENTICATION;
    }
    GapServiceSecurityInfo serviceInfo;
    serviceInfo.channelId = secChan;
    serviceInfo.direction = connectDirection;
    serviceInfo.protocolId = protocolId;
    serviceInfo.serviceId = serviceId;
    return GAPIF_RegisterServiceSecurity(addr, &serviceInfo, securityLevel);
}

int SocketGapMock::UnregisterSecurity(const BtAddr *addr, uint16_t scn, GAP_Service serviceId, bool isServer)
{
    GAP_ServiceConnectDirection connectDirection;
    GapSecChannel secChan{scn};
    connectDirection = isServer ? INCOMING : OUTGOING;
    GapServiceSecurityInfo serviceInfo;
    serviceInfo.channelId = secChan;
    serviceInfo.direction = connectDirection;
    serviceInfo.protocolId = SEC_PROTOCOL_RFCOMM;
    serviceInfo.serviceId = serviceId;

    return GAPIF_DeregisterServiceSecurity(addr, &serviceInfo);
}
}  // namespace bluetooth