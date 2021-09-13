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
#include "obex_client.h"
#include "buffer.h"
#include "log.h"
#include "obex_socket_transport.h"
#include "obex_session.h"
#include "obex_utils.h"
#include <iostream>
#include <string.h>
#include "../mce_mock/mce_mock.h"

int obexConnectCtrl = MCE_TEST_CTRL_CONNECT_OK;
int obexDisconnectCtrl = MCE_TEST_CTRL_DISCONNECT_OK;
bluetooth::ObexClient *obexClient = nullptr;
int obexPutGetCtrl = MCE_TEST_CTRL_NO_RESPONES;

namespace bluetooth {

void ObexClient::ObexClientTransportObserver::OnTransportConnected(ObexTransport &transport)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexClient::ObexClientTransportObserver::OnTransportDisconnected(ObexTransport &transport)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexClient::ObexClientTransportObserver::OnTransportDataBusy(ObexTransport &transport, uint8_t isBusy)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexClient::ObexClientTransportObserver::OnTransportDataAvailable(ObexTransport &transport, ObexPacket &obexPacket)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexClient::ObexClientTransportObserver::OnTransportError(ObexTransport &transport, int errCd)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

ObexClient::ObexClient(const ObexClientConfig &config, ObexClientObserver &observer, utility::Dispatcher &dispatcher)
    : clientObserver_(observer), dispatcher_(dispatcher)
{
    obexClient = this;
    this->clientObserver_ = observer;
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    isObexConnected_ = false;
    clientSession_ = std::make_unique<ObexClientSession>(RawAddress::ConvertToString(config.addr_.addr));
    clientSession_->SetMaxPacketLength(config.mtu_);
    clientSession_->SetServiceUUID(config.serviceUUID_);
    clientSession_->FreeSendObject();
    connectReqHeader_ = nullptr;
    clientState_ = ObexClientState::INIT;
}

/// send obex request
int ObexClient::SendRequest(const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return 0;
}

int ObexClient::Connect()
{
    ObexConnectParams connectParams;
    return Connect(connectParams);
}

// int ObexClient::Connect(ObexTlvParamters* params)
int ObexClient::Connect(ObexConnectParams &connectParams)

{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    int errCd;

    if ((connectParams.appParams_ == nullptr) && (obexConnectCtrl == MCE_TEST_CTRL_TRANSPORT_ERRO)) {
        errCd = 0xaa;
        clientObserver_.OnTransportFailed(*this, errCd);
        return BT_NO_ERROR;
    }

    if ((connectParams.appParams_ == nullptr) && (obexConnectCtrl == MCE_TEST_CTRL_ACTION_COMPLETE_OK)) {
        clientObserver_.OnActionCompleted(*this, *header0);
        return BT_NO_ERROR;
    }

    if ((connectParams.appParams_ == nullptr) && (obexConnectCtrl == MCE_TEST_CTRL_DISCONNECT_OK)) {
        clientObserver_.OnDisconnected(*this);
        return BT_NO_ERROR;
    }

    switch (obexConnectCtrl) {
        case MCE_TEST_CTRL_CONNECT_OK:
            clientObserver_.OnConnected(*this, *header0);
            break;
        case MCE_TEST_CTRL_CONNECT_FAILED:
            clientObserver_.OnConnectFailed(*this, *header1);
            break;
        case MCE_TEST_CTRL_TRANSPORT_ERRO:
            errCd = 0xee;
            clientObserver_.OnTransportFailed(*this, errCd);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

// send obex disconnect request
int ObexClient::Disconnect(bool withObexReq)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexDisconnectCtrl) {
        case MCE_TEST_CTRL_DISCONNECT_OK:
            clientObserver_.OnDisconnected(*this);
            break;
        case 1:
            break;
        case 2:
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

/// send obex abort request
int ObexClient::Abort()
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// send obex put request
int ObexClient::Put(const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

// send obex get request
int ObexClient::Get(const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

// send obex set_path request
int ObexClient::SetPath(uint8_t flag, const std::u16string &path)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

/// send obex action request
int ObexClient::Copy(const std::u16string &srcName, const std::u16string &destName)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexClient::Move(const std::u16string &srcName, const std::u16string &destName)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexClient::SetPermissions(const std::u16string &name, const uint32_t permissions)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// send obex session request
/// This command must include a Session-Parameters header containing the Session Opcode, Device Address and Nonce
/// fields. Optionally, a Timeout field can be included.
int ObexClient::CreateSession()
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return CreateSession(OBEX_SESSION_MAX_TIMEOUT_SEC);  // TODO
}

int ObexClient::CreateSession(uint32_t timeoutSec)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// This command must include a Session-Parameters header containing the Session Opcode field.
/// Optionally, a Timeout field can be included.
int ObexClient::SuspendSession()
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// This command must include a Session-Parameters header containing the Session Opcode, Device Address, Nonce,
/// and Session ID and fields. Optionally, a Timeout field can be included.
int ObexClient::ResumeSession()
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// This command must include a Session-Parameters header containing the Session Opcode and Session ID fields.
int ObexClient::CloseSession()
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

/// This command must include a Session-Parameters header containing the Session Opcode field.
/// Optionally, a Timeout field can be included.
int ObexClient::SetSessionTimeout(uint32_t timeoutSec /*default OBEX_SESSION_MAX_TIMEOUT_SEC*/)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

bool ObexClient::CheckBeforeSession(uint8_t sessionOpcode)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return true;
}

bool ObexClient::CheckBeforeRequest(uint8_t opeId) const
{
    return true;
}

void ObexClient::PutDataAvailable(const ObexHeader &resp)
{}
void ObexClient::GetDataAvailable(const ObexHeader &resp)
{}
void ObexClient::SetPathDataAvailable(const ObexHeader &resp)
{}
int ObexClient::SendConnectRequest(ObexHeader &header)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    return BT_NO_ERROR;
}

const ObexClientSession &ObexClient::GetClientSession() const
{
    return *this->clientSession_;
}

const std::string &ObexClient::GetClientId()
{
    static std::string temp = "temp";
    std::string &tp = temp;
    return tp;
}

void ObexClient::HandleTransportDataBusy(uint8_t isBusy)
{
    LOG_INFO("Call %s, isBusy %d", __PRETTY_FUNCTION__, isBusy);
}

void ObexClient::AbortDataAvailable(const ObexHeader &resp)
{
    isWaitingSendAbort_ = false;
    isAbortSended_ = false;
    this->clientObserver_.OnActionCompleted(*this, resp);
}

int ObexClient::RegisterL2capLPsm(uint16_t lpsm)
{
    return RET_NO_ERROR;
}

void ObexClient::DeregisterL2capLPsm(uint16_t lpsm)
{}
void ObexClient::SetBusy(bool isBusy)
{

}

void ObexClientObserver::OnBusy(ObexClient &client, bool isBusy)
{
}
}  // namespace bluetooth
