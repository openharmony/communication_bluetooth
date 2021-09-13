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
#include "obex_server.h"
#include "dispatcher.h"
#include "log.h"
#include "obex_utils.h"
#include "string.h"

namespace bluetooth {
ObexServer::ObexServer(const std::string &serviceName, const ObexServerConfig &config, ObexServerObserver &observer,
    utility::Dispatcher &dispatcher)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

int ObexServer::Startup() const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return 0;
}

void ObexServer::Shutdown() const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnTransportConnect(ObexIncomingConnect &incomingConnect)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnTransportConnected(const std::string &btAddr)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnTransportDisconnected(const std::string &btAddr)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnTransportError(const std::string &btAddr, int errCd, const std::string &msg)
{
    LOG_ERROR("Call %s, ERROR:%d,%s", __PRETTY_FUNCTION__, errCd, msg.c_str());
}

void ObexServerObserver::OnError(const int errCd, const std::string &msg)
{
    LOG_ERROR("Call %s, ERROR:%d,%s", __PRETTY_FUNCTION__, errCd, msg.c_str());
}

void ObexServerObserver::OnPut(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnGet(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::NOT_IMPLEMENTED, false));
}

void ObexServerObserver::OnAbort(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnSetPath(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    session.SendResponse(*ObexHeader::CreateResponse(ObexRspCode::NOT_IMPLEMENTED, false));
}

void ObexServerObserver::OnAction(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexServerObserver::OnSession(ObexServerSession &session, const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportConnectIncoming(ObexIncomingConnect &incomingConnect)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportIncomingDisconnected(const std::string &btAddr)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportConnected(ObexTransport &transport)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportDisconnected(ObexTransport &transport)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportDataAvailable(
    ObexTransport &transport, ObexPacket &obexPacket)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

std::unique_ptr<bluetooth::ObexHeader> ObexPrivateServer::ObexServerTransportObserver::GetObexHeaderFromPacket(
    ObexPacket &obexPacket) const
{
    std::unique_ptr<bluetooth::ObexHeader> testptr = nullptr;
    return testptr;
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportError(ObexTransport &transport, int errCd)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::ObexServerTransportObserver::OnTransportDataBusy(ObexTransport &transport, uint8_t isBusy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

ObexPrivateServer::ObexPrivateServer(
    const ObexPrivateServerConfig &config, ObexServerObserver &observer, utility::Dispatcher &dispatcher)
    : observer_(observer), dispatcher_(dispatcher)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

int ObexPrivateServer::Startup() const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

void ObexPrivateServer::Shutdown() const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

int ObexPrivateServer::RemoveSession(ObexServerSession &session) const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

void ObexPrivateServer::SetBusy(ObexServerSession &session, bool isBusy) const
{
    if (session.IsBusy() != isBusy) {
        session.SetBusy(isBusy);
        observer_.OnBusy(session, isBusy);
    }
}

void ObexServerObserver::OnBusy(ObexServerSession &session, bool isBusy) const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::HandlePutRequest(ObexServerSession &session, ObexHeader &req) const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::HandleGetRequest(ObexServerSession &session, ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::HandleSetPathRequest(ObexServerSession &session, ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::HandleAbortRequest(ObexServerSession &session, ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateServer::HandleTransportDataBusy(ObexServerSession &serverSession, uint8_t isBusy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}
}  // namespace bluetooth