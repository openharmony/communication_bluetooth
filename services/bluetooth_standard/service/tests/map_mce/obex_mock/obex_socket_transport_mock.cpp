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
#include "obex_socket_transport.h"
#include "log.h"
#include "obex_utils.h"
#include "packet.h"
#include "transport/transport_factory.h"
#include <iomanip>
#include <memory>
#include <sstream>
#include <string.h>

namespace bluetooth {
ObexSocketTransport::ObexSocketTransport(
    DataTransport *sockTransport, uint16_t sendMtu, uint16_t recvMtu, const std::string &parentTranKey)
{
}

ObexSocketTransport::~ObexSocketTransport()
{
    OBEX_LOG_INFO("Delete ~ObexSocketTransport: %p", this);
}

// write packet
bool ObexSocketTransport::Write(Packet &pkt)
{
    return false;
}

// the maximum allowed OBEX packet that can be send over the transport
int ObexSocketTransport::GetMaxSendPacketSize()
{
    return this->sendMtu_;
}

// the maximum allowed OBEX packet that can be received over the transport
int ObexSocketTransport::GetMaxReceivePacketSize()
{
    return this->recvMtu_;
}

/// This function is used to get the Bluetooth address of the peer of the connected channel
const RawAddress &ObexSocketTransport::GetRemoteAddress()
{
    return this->remoteBtAddr_;
}

/// is transport Connected
bool ObexSocketTransport::IsConnected()
{
    return this->isConnected_;
}

/// get transport key
const std::string &ObexSocketTransport::GetTransportKey()
{
    return this->tranKey_;
}

/// ObexClientSocketTransport
ObexClientSocketTransport::ObexClientSocketTransport(
    const Option &option, ObexTransportObserver &observer, utility::Dispatcher &dispatcher)
    : ObexClientTransport(observer), dispatcher_(dispatcher)
{
}

ObexClientSocketTransport::~ObexClientSocketTransport()
{}

std::string ObexClientSocketTransport::MakeTransportKey(bool isGoepL2capPSM, uint16_t scn)
{
    return "test";
}

int ObexClientSocketTransport::Connect()
{
    return -1;
}

int ObexClientSocketTransport::Disconnect()
{
    return -1;
}

bool ObexClientSocketTransport::Write(Packet &pkt)
{
    return false;
}

// the maximum allowed OBEX packet that can be send over the transport
int ObexClientSocketTransport::GetMaxSendPacketSize()
{
    return this->sendMtu_;
}

// the maximum allowed OBEX packet that can be received over the transport
int ObexClientSocketTransport::GetMaxReceivePacketSize()
{
    return this->recvMtu_;
}

/// This function is used to get the Bluetooth address of the peer of the connected channel
const RawAddress &ObexClientSocketTransport::GetRemoteAddress()
{
    return this->remoteBtAddr_;
}

/// is transport Connected
bool ObexClientSocketTransport::IsConnected()
{
    return this->isConnected_;
}

/// get transport key
const std::string &ObexClientSocketTransport::GetTransportKey()
{
    return this->tranKey_;
}

ObexClientSocketTransport::TransportObserver::TransportObserver(ObexClientSocketTransport &obexTran)
    : obexTran_(obexTran)
{}

/// connection success
void ObexClientSocketTransport::TransportObserver::OnConnected(
    DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU)
{
}

/// The event is triggered when a disconnect request is received.
void ObexClientSocketTransport::TransportObserver::OnDisconnected(DataTransport *transport)
{
}

/// The event is triggered when the disconnection process is successful.
void ObexClientSocketTransport::TransportObserver::OnDisconnectSuccess(DataTransport *transport)
{
}

/// when data is received from RFCOMM
void ObexClientSocketTransport::TransportObserver::OnDataAvailable(DataTransport *transport)
{
}

/// when data is received from L2CAP
void ObexClientSocketTransport::TransportObserver::OnDataAvailable(DataTransport *transport, Packet *pkt)
{
}

// The event is triggered when peer or RFCOMM/L2CAP is not avaliable to received data.
void ObexClientSocketTransport::TransportObserver::OnDataBusy(DataTransport *transport, uint8_t isBusy)
{
}

/// when peer is not avaliable to received data, or RFCOMM's send queue is full
void ObexClientSocketTransport::TransportObserver::OnTransportError(DataTransport *transport, int errType)
{
}

/// The event is triggered when connection complete successfully.
void ObexClientSocketTransport::TransportObserver::ProcessOnConnected(
    DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU)
{
}

/// The event is triggered when a disconnect request is received.
void ObexClientSocketTransport::TransportObserver::ProcessOnDisconnected()
{
}

/// The event is triggered when the disconnection process is successful.
void ObexClientSocketTransport::TransportObserver::ProcessOnDisconnectSuccess()
{
}

void ObexClientSocketTransport::TransportObserver::ProcessOnDataBusy(uint8_t isBusy)
{
}

/// The event is triggered when data is received from stack.
void ObexClientSocketTransport::TransportObserver::ProcessOnDataAvailable(DataTransport *transport, Packet *pkt)
{
}

/// The event is triggered when process is failed.
void ObexClientSocketTransport::TransportObserver::ProcessOnTransportError(int errType)
{
}

// ObexServerIncomingConnect
ObexServerIncomingConnect::ObexServerIncomingConnect(
    ObexServerSocketTransport &obexTransport, const RawAddress &btAddr, int port)
    : obexTransport_(obexTransport), port_(port)
{
}

ObexServerIncomingConnect::~ObexServerIncomingConnect()
{
    OBEX_LOG_INFO("Delete ~ObexServerIncomingConnect: %p", this);
}

///  accept the connection request
int ObexServerIncomingConnect::AcceptConnection()
{
    return -1;
}

///  reject the connection request
int ObexServerIncomingConnect::RejectConnection()
{
    return -1;
}

const RawAddress &ObexServerIncomingConnect::GetRemoteAddress()
{
    return this->btAddr_;
}

int ObexServerIncomingConnect::GetPort()
{
    return -1;
}

ObexServerSocketTransport::ObexServerSocketTransport(
    const Option &option, ObexTransportObserver &observer, utility::Dispatcher &dispatcher)
    : ObexServerTransport(observer), dispatcher_(dispatcher)
{
}

std::string ObexServerSocketTransport::MakeTransportKey(bool isGoepL2capPSM, uint16_t scn)
{
    return "test";
}

int ObexServerSocketTransport::Listen()
{
    return -1;
}

int ObexServerSocketTransport::Disconnect()
{
    return -1;
}

int ObexServerSocketTransport::Disconnect(ObexTransport &subTransport)
{
    return -1;
}

/// Server accept the connection request
int ObexServerSocketTransport::AcceptConnection(ObexIncomingConnect &incomingConnect)
{
    return -1;
}

/// Server reject the connection request
int ObexServerSocketTransport::RejectConnection(ObexIncomingConnect &incomingConnect)
{
    return -1;
}

/// get transport key
const std::string &ObexServerSocketTransport::GetTransportKey()
{
    std::string test = "test";
    std::string &test1 = test;
    return test1;
}

ObexServerSocketTransport::TransportObserver::TransportObserver(ObexServerSocketTransport &sockTransport)
    : mainTran_(sockTransport)
{}
/// connection success
void ObexServerSocketTransport::TransportObserver::OnConnectIncoming(const RawAddress &addr, uint16_t port)
{
}

void ObexServerSocketTransport::TransportObserver::OnIncomingDisconnected(const RawAddress &addr)
{
}

/// connection success
void ObexServerSocketTransport::TransportObserver::OnConnected(
    DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU)
{
}

/// connection failed
void ObexServerSocketTransport::TransportObserver::OnDisconnected(DataTransport *transport)
{
}

void ObexServerSocketTransport::TransportObserver::OnDisconnectSuccess(DataTransport *transport)
{
}

/// when data is received from RFCOMM
void ObexServerSocketTransport::TransportObserver::OnDataAvailable(DataTransport *transport)
{
}

/// when data is received from L2CAP
void ObexServerSocketTransport::TransportObserver::OnDataAvailable(DataTransport *transport, Packet *pkt)
{
}

/// when peer is not avaliable to received data, or RFCOMM's send queue is full
void ObexServerSocketTransport::TransportObserver::OnTransportError(DataTransport *transport, int rfcError)
{
}

// The event is triggered when peer or RFCOMM/L2CAP is not avaliable to received data.
void ObexServerSocketTransport::TransportObserver::OnDataBusy(DataTransport *transport, uint8_t isBusy)
{
}

/// The event is triggered when server accept a new connection.
void ObexServerSocketTransport::TransportObserver::ProcessOnConnectIncoming(RawAddress btAddr, uint16_t port)
{
}

/// The event is triggered when new connection disconnect before accept.
void ObexServerSocketTransport::TransportObserver::ProcessOnIncomingDisconnected(RawAddress btAddr)
{
}

/// The event is triggered when connection complete successfully.
void ObexServerSocketTransport::TransportObserver::ProcessOnConnected(
    DataTransport *transport, uint16_t sendMTU, uint16_t recvMTU)
{
}

/// The event is triggered when a disconnect request is received.
void ObexServerSocketTransport::TransportObserver::ProcessOnDisconnected(DataTransport *transport)
{
}

/// The event is triggered when the disconnection process is successful.
void ObexServerSocketTransport::TransportObserver::ProcessOnDisconnectSuccess(DataTransport *transport)
{
}

/// The event is triggered when data is received from stack.
void ObexServerSocketTransport::TransportObserver::ProcessOnDataAvailable(DataTransport *transport, Packet *pkg)
{
}

void ObexServerSocketTransport::TransportObserver::ProcessOnDataBusy(DataTransport *transport, uint8_t isBusy)
{
}

/// The event is triggered when process is failed.
void ObexServerSocketTransport::TransportObserver::ProcessOnTransportError(DataTransport *transport, int errType)
{
}
}  // namespace bluetooth
