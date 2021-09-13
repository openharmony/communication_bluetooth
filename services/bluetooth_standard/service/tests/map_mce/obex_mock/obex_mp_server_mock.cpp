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
#include "../mce_mock/mce_mock.h"
#include "obex_server.h"
#include "obex_mp_server.h"
#include "log.h"
#include <memory>
#include "obex_mp_client.h"

bluetooth::ObexServerObserver *obexServerObserver_ = nullptr;
namespace bluetooth {
// create obex server
ObexMpServer::ObexMpServer(const std::string &serviceName, const ObexServerConfig &config, ObexServerObserver &observer,
    utility::Dispatcher &dispatcher)
{
    LOG_INFO("Call ObexMpServer %s", __PRETTY_FUNCTION__);
    obexServerObserver_ = &observer;
}

ObexPrivateMpServer::ObexPrivateMpServer(
    const ObexPrivateServerConfig &config, ObexServerObserver &observer, utility::Dispatcher &dispatcher)
    : ObexPrivateServer(config, observer, dispatcher)
{}

void ObexPrivateMpServer::HandlePutRequest(ObexServerSession &session, ObexHeader &req) const
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateMpServer::HandleTransportDataBusy(ObexServerSession &session, uint8_t isBusy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateMpServer::HandleGetRequest(ObexServerSession &session, ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexPrivateMpServer::HandleAbortRequest(ObexServerSession &session, ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}
}  // namespace bluetooth