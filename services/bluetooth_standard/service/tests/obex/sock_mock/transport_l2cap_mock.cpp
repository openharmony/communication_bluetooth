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

#include "log.h"
#include "raw_address.h"
#include "transport_l2cap.h"

namespace bluetooth {
std::map<uint16_t, L2capTransport *> L2capTransport::g_clientTransportMap{};

L2capTransport::L2capTransport(L2capTransportInfo &createInfo)
    : lpsm_(createInfo.lpsm),
      rpsm_(createInfo.rpsm),
      localMtu_(createInfo.mtu),
      observer_(createInfo.observer),
      dispatcher_(createInfo.dispatcher)
{
    if (createInfo.addr) {
        remoteAddr_ = *createInfo.addr;
    }
}

L2capTransport::~L2capTransport()
{}

int L2capTransport::Connect()
{
    LOG_INFO("[L2capTransport]%s lpsm:0x%04X, rpsm:0x%04X", __func__, lpsm_, rpsm_);
    return 0;
}

int L2capTransport::Disconnect()
{
    LOG_INFO("[L2capTransport]%s lcid:%d", __func__, l2capHandle_);

    return 0;
}

int L2capTransport::RegisterServer()
{
    LOG_INFO("[L2capTransport]%s", __func__);

    isServer_ = true;
    return 0;
}

int L2capTransport::RemoveServer(bool isDisable)
{
    LOG_INFO("[L2capTransport]%s", __func__);

    return 0;
}

int L2capTransport::AcceptConnection(const RawAddress &addr, uint16_t psm)
{
    LOG_INFO("[L2capTransport]%s ", __func__);
    return 0;
}

int L2capTransport::RejectConnection(const RawAddress &addr, uint16_t psm)
{
    LOG_INFO("[L2capTransport]%s", __func__);
    return 0;
}

int L2capTransport::Read(Packet **pkt)
{
    LOG_INFO("[L2capTransport]%s", __func__);
    return 0;
}

int L2capTransport::Write(Packet *pkt)
{
    LOG_INFO("[L2capTransport]%s", __func__);
    return 0;
}

RawAddress L2capTransport::GetRemoteAddress()
{
    LOG_INFO("[L2capTransport]%s", __func__);
    RawAddress rawAddr;
    return rawAddr;
}

int L2capTransport::RegisterClientPsm(uint16_t lpsm)
{
    LOG_INFO("[L2capTransport]%s lpsm:0x%04X", __func__, lpsm);
    return 0;
}

void L2capTransport::DeregisterClientPsm(uint16_t lpsm)
{
    LOG_INFO("[L2capTransport]%s lpsm:0x%04X", __func__, lpsm);
}
}  // namespace bluetooth