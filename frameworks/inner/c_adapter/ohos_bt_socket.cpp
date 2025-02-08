/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_c_adapter_socket"
#endif

#include "ohos_bt_socket.h"

#include <iostream>
#include <cstring>
#include <vector>

#include "ohos_bt_adapter_utils.h"
#include "bluetooth_def.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_socket.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_object_map.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {

const int MAX_OBJECT_NUM = 10000;

static BluetoothObjectMap<std::shared_ptr<ServerSocket>, MAX_OBJECT_NUM> g_serverMap;
static BluetoothObjectMap<std::shared_ptr<ClientSocket>, MAX_OBJECT_NUM> g_clientMap;

class BluetoothConnectionObserverWapper : public BluetoothConnectionObserver {
public:
    explicit BluetoothConnectionObserverWapper(BtSocketConnectionCallback &callback)
    {
        socektConnectCallback = callback;
    }

    __attribute__((no_sanitize("cfi")))
    void OnConnectionStateChanged(const CallbackConnectParam &param) override
    {
        if (socektConnectCallback.connStateCb == nullptr && socektConnectCallback.bleConnStateCb == nullptr) {
            HILOGE("callback is null");
            return;
        }
        BdAddr addr;
        GetAddrFromString(param.addr.GetDeviceAddr(), addr.addr);
        BtUuid btUuid;
        string strUuid = param.uuid.ToString();
        btUuid.uuid = (char *)strUuid.c_str();
        btUuid.uuidLen = strUuid.size();
        if (param.type == OHOS_SOCKET_SPP_RFCOMM && socektConnectCallback.connStateCb != nullptr) {
            socektConnectCallback.connStateCb(&addr, btUuid, param.status, param.result);
        }
        if (param.type == OHOS_SOCKET_L2CAP_LE && socektConnectCallback.bleConnStateCb != nullptr) {
            socektConnectCallback.bleConnStateCb(&addr, param.psm, param.status, param.result);
        }
    }

    BtSocketConnectionCallback socektConnectCallback;
};

using ClientCbIterator = std::map<int, std::shared_ptr<BluetoothConnectionObserverWapper>>::iterator;

static bool GetSocketUuidPara(const BluetoothCreateSocketPara *socketPara, UUID &serverUuid)
{
    if (socketPara->socketType == OHOS_SOCKET_SPP_RFCOMM) {
        if (socketPara->uuid.uuid == nullptr || strlen(socketPara->uuid.uuid) != socketPara->uuid.uuidLen) {
            HILOGE("param uuid invalid!");
            return false;
        }
        string tmpUuid(socketPara->uuid.uuid);
        if (!IsValidUuid(tmpUuid)) {
            HILOGE("match the UUID faild.");
            return false;
        }
        serverUuid = UUID::FromString(tmpUuid);
    } else if (socketPara->socketType == OHOS_SOCKET_L2CAP_LE) {
        serverUuid = UUID::RandomUUID();
    } else {
        HILOGE("param socketType invalid. socketType: %{public}d", socketPara->socketType);
        return false;
    }
    return true;
}

/**
 * @brief Creates an server listening socket based on the service record.
 *
 * @param socketPara The parameters to create a server socket.
 * @param name The service's name.
 * @return Returns a server ID, if create fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketServerCreate(const BluetoothCreateSocketPara *socketPara, const char *name)
{
    HILOGD("SocketServerCreate start!");
    if (socketPara == nullptr || name == nullptr) {
        HILOGE("socketPara or name is null.");
        return BT_SOCKET_INVALID_ID;
    }

    if (socketPara->socketType == OHOS_SOCKET_L2CAP_LE) {
        CHECK_AND_RETURN_LOG_RET(IS_BLE_ENABLED(), BT_SOCKET_INVALID_ID, "BLE is not TURN_ON");
    } else {
        CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_SOCKET_INVALID_ID, "BR is not TURN_ON");
    }

    UUID serverUuid;
    if (!GetSocketUuidPara(socketPara, serverUuid)) {
        return BT_SOCKET_INVALID_ID;
    }

    string serverName(name);
    std::shared_ptr<ServerSocket> server = std::make_shared<ServerSocket>(serverName, serverUuid,
        BtSocketType(socketPara->socketType), socketPara->isEncrypt);
    int result = server->Listen();
    if (result != BT_NO_ERROR) {
        HILOGE("SocketServerCreate fail, result: %{public}d", result);
        server->Close();
        HILOGE("SocketServerCreate closed.");
        return BT_SOCKET_INVALID_ID;
    }
    int serverId = g_serverMap.AddObject(server);
    HILOGI("success, serverId: %{public}d, socketType: %{public}d, isEncrypt: %{public}d", serverId,
        socketPara->socketType, socketPara->isEncrypt);
    return serverId;
}

/**
 * @brief Waits for a remote device to connect to this server socket.
 *
 * This method return a client ID indicates a client socket
 * can be used to read data from and write data to remote device.
 * This method will block until a connection is established.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns a client ID, if accept fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketServerAccept(int serverId)
{
    HILOGI("SocketServerAccept start, serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    if (server == nullptr) {
        HILOGD("server is null!");
        return BT_SOCKET_INVALID_ID;
    }

    std::shared_ptr<ClientSocket> client = server->Accept(0);
    if (client == nullptr) {
        HILOGE("client is null!");
        return BT_SOCKET_INVALID_ID;
    }
    int clientId = g_clientMap.AddObject(client);
    HILOGI("success, clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Disables an socket server socket and releases related resources.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketServerClose(int serverId)
{
    HILOGI("SocketServerClose start, serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    if (server == nullptr) {
        HILOGE("server is null!");
        return OHOS_BT_STATUS_FAIL;
    }
    server->Close();
    g_serverMap.RemoveObject(serverId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Set fast connection flag
 *
 * @param bdAddr The remote device address to connect.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketSetFastConnection(const BdAddr *bdAddr)
{
    string strAddress;
    int leType = 1;
    if (bdAddr == nullptr) {
        HILOGE("bdAddr is nullptr.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    ConvertAddr(bdAddr->addr, strAddress);
    // create a client to reuse requestfastestconn.
    std::shared_ptr<GattClient> client = nullptr;
    BluetoothRemoteDevice device(strAddress, leType);
    client = std::make_shared<GattClient>(device);
    client->Init();
    int result = client->RequestFastestConn();
    if (result != OHOS_BT_STATUS_SUCCESS) {
        HILOGE("request fastest connect fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Connects to a remote device over the socket.
 * This method will block until a connection is made or the connection fails.
 *
 * @param socketPara The param to create a client socket and connect to a remote device.
 * @param bdAddr The remote device address to connect.
 * @param psm BluetoothSocketType is {@link OHOS_SOCKET_L2CAP_LE} use dynamic PSM value from remote device.
 * BluetoothSocketType is {@link OHOS_SOCKET_SPP_RFCOMM} use -1.
 * @return Returns a client ID, if connect fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketConnect(const BluetoothCreateSocketPara *socketPara, const BdAddr *bdAddr, int psm)
{
    HILOGI("SocketConnect start.");
    if (socketPara == nullptr || bdAddr == nullptr) {
        HILOGE("socketPara is nullptr, or bdAddr is nullptr");
        return BT_SOCKET_INVALID_ID;
    }

    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    std::shared_ptr<BluetoothRemoteDevice> device = std::make_shared<BluetoothRemoteDevice>(strAddress, 0);

    UUID serverUuid;
    if (!GetSocketUuidPara(socketPara, serverUuid)) {
        return BT_SOCKET_INVALID_ID;
    }

    std::shared_ptr<ClientSocket> client = std::make_shared<ClientSocket>(*device, serverUuid,
        BtSocketType(socketPara->socketType), socketPara->isEncrypt);
    HILOGI("socketType: %{public}d, isEncrypt: %{public}d", socketPara->socketType, socketPara->isEncrypt);
    if (socketPara->socketType != OHOS_SOCKET_L2CAP_LE && !client->IsAllowSocketConnect(socketPara->socketType)) {
        HILOGE("SocketConnect fail due to limited resources, addr: %{public}s", GetEncryptAddr(strAddress).c_str());
        return BT_SOCKET_LIMITED_RESOURCES;
    }
    client->Init();
    int result = client->Connect(psm);
    if (result != OHOS_BT_STATUS_SUCCESS) {
        HILOGE("SocketConnect fail, result: %{public}d", result);
        client->Close();
        HILOGE("SocketConnect closed.");
        return BT_SOCKET_INVALID_ID;
    }
    int clientId = g_clientMap.AddObject(client);
    HILOGI("SocketConnect success, clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Connects to a remote device over the socket.
 * This method will block until a connection is made or the connection fails.
 * @param socketPara The param to create a client socket and connect to a remote device.
 * @param bdAddr The remote device address to connect.
 * @param psm BluetoothSocketType is {@link OHOS_SOCKET_L2CAP_LE} use dynamic PSM value from remote device.
 * BluetoothSocketType is {@link OHOS_SOCKET_SPP_RFCOMM} use -1.
 * @param callback Reference to the socket state observer
 * @return Returns a client ID, if connect fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketConnectEx(const BluetoothCreateSocketPara *socketPara, const BdAddr *bdAddr, int psm,
    BtSocketConnectionCallback *callback)
{
    HILOGI("SocketConnect start.");
    if (socketPara == nullptr || bdAddr == nullptr || callback == nullptr) {
        HILOGE("socketPara is nullptr, or bdAddr is nullptr, or callback is nullptr");
        return BT_SOCKET_INVALID_ID;
    }

    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    std::shared_ptr<BluetoothRemoteDevice> device = std::make_shared<BluetoothRemoteDevice>(strAddress, 0);

    UUID serverUuid;
    if (!GetSocketUuidPara(socketPara, serverUuid)) {
        return BT_SOCKET_INVALID_ID;
    }

    if (socketPara->socketType != OHOS_SOCKET_SPP_RFCOMM && socketPara->socketType != OHOS_SOCKET_L2CAP_LE) {
        HILOGE("SocketType is not support");
        return BT_SOCKET_INVALID_TYPE;
    }

    std::shared_ptr<BluetoothConnectionObserverWapper> connWrapper =
        std::make_shared<BluetoothConnectionObserverWapper>(*callback);
    std::shared_ptr<ClientSocket> client = std::make_shared<ClientSocket>(*device, serverUuid,
        BtSocketType(socketPara->socketType), socketPara->isEncrypt, connWrapper);
    HILOGI("socketType: %{public}d, isEncrypt: %{public}d", socketPara->socketType, socketPara->isEncrypt);
    if (socketPara->socketType != OHOS_SOCKET_L2CAP_LE && !client->IsAllowSocketConnect(socketPara->socketType)) {
        HILOGE("SocketConnect fail due to limited resources, addr: %{public}s", GetEncryptAddr(strAddress).c_str());
        return BT_SOCKET_LIMITED_RESOURCES;
    }
    client->Init();
    int result = client->Connect(psm);
    if (result != OHOS_BT_STATUS_SUCCESS) {
        HILOGE("SocketConnect fail, result: %{public}d", result);
        client->Close();
        HILOGE("SocketConnect closed.");
        return BT_SOCKET_INVALID_ID;
    }
    int clientId = g_clientMap.AddObject(client);
    HILOGI("SocketConnect success, clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Disables a connection and releases related resources.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketDisconnect(int clientId)
{
    HILOGI("SocketDisconnect start, clientId: %{public}d", clientId);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    client->Close();
    g_clientMap.RemoveObject(clientId);
    HILOGI("SocketDisConnect success, clientId: %{public}d", clientId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Socket get remote device's address.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param remoteAddr Remote device's address, memory allocated by caller.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketGetRemoteAddr(int clientId, BdAddr *remoteAddr)
{
    HILOGI("SocketGetRemoteAddr clientId: %{public}d", clientId);
    if (remoteAddr == nullptr) {
        HILOGE("remoteAddr is null, clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    string tmpAddr = client->GetRemoteDevice().GetDeviceAddr();
    GetAddrFromString(tmpAddr, remoteAddr->addr);
    HILOGI("device: %{public}s", GetEncryptAddr(tmpAddr).c_str());
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Get the connection status of this socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns true is connected or false is not connected.
 */
bool IsSocketConnected(int clientId)
{
    HILOGI("IsSocketConnected clientId: %{public}d", clientId);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return false;
    }
    bool isConnected = client->IsConnected();
    HILOGI("clientId: %{public}d, isConnected: %{public}d", clientId, isConnected);
    return isConnected;
}

/**
 * @brief Read data from socket.
 * This method blocks until input data is available.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param buf Indicate the buffer which read in, memory allocated by caller.
 * @param bufLen Indicate the buffer length.
 * @return Returns the length greater than 0 as read the actual length.
 * Returns {@link BT_SOCKET_READ_SOCKET_CLOSED} if the socket is closed.
 * Returns {@link BT_SOCKET_READ_FAILED} if the operation failed.
 */
int SocketRead(int clientId, uint8_t *buf, uint32_t bufLen)
{
    HILOGD("SocketRead start, clientId: %{public}d, bufLen: %{public}d", clientId, bufLen);
    if (buf == nullptr || bufLen == 0) {
        HILOGE("buf is null or bufLen is 0! clientId: %{public}d", clientId);
        return BT_SOCKET_READ_FAILED;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null, clientId: %{public}d", clientId);
        return BT_SOCKET_READ_FAILED;
    }
    if (client->GetInputStream() == nullptr) {
        HILOGE("inputStream is null, clientId: %{public}d", clientId);
        return BT_SOCKET_READ_FAILED;
    }
    int readLen = client->GetInputStream()->Read(buf, bufLen);
    HILOGD("SocketRead ret, clientId: %{public}d, readLen: %{public}d", clientId, readLen);
    return readLen;
}

/**
 * @brief Client write data to socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param data Indicate the data to be written.
 * @return Returns the actual write length.
 * Returns {@link BT_SOCKET_WRITE_FAILED} if the operation failed.
 */
int SocketWrite(int clientId, const uint8_t *data, uint32_t len)
{
    HILOGD("SocketWrite start, clientId: %{public}d, len: %{public}d", clientId, len);
    if (data == nullptr || len == 0) {
        HILOGE("data is null or len is 0! clientId: %{public}d", clientId);
        return BT_SOCKET_WRITE_FAILED;
    }
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null! clientId: %{public}d", clientId);
        return BT_SOCKET_WRITE_FAILED;
    }
    if (client->GetOutputStream() == nullptr) {
        HILOGE("outputStream is null, clientId: %{public}d", clientId);
        return BT_SOCKET_READ_FAILED;
    }
    int writeLen = client->GetOutputStream()->Write(data, len);
    HILOGD("end, writeLen: %{public}d", writeLen);
    return writeLen;
}

/**
 * @brief Get dynamic PSM value for OHOS_SOCKET_L2CAP.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the PSM value.
 * Returns {@link BT_SOCKET_INVALID_PSM} if the operation failed.
 */
int SocketGetPsm(int serverId)
{
    HILOGI("serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    CHECK_AND_RETURN_LOG_RET(server, BT_SOCKET_INVALID_PSM, "server is null!");
    return server->GetL2capPsm();
}

/**
 * @brief Get server channel number for OHOS_SOCKET_RFCOMM.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the scn.
 * Returns {@link BT_SOCKET_INVALID_PSM} if the operation failed.
 */
int SocketGetScn(int serverId)
{
    HILOGI("serverId: %{public}d", serverId);
    std::shared_ptr<ServerSocket> server = g_serverMap.GetObject(serverId);
    CHECK_AND_RETURN_LOG_RET(server, BT_SOCKET_INVALID_SCN, "server is null!");
    return server->GetRfcommScn();
}

/**
 * @brief Adjust the socket send and recv buffer size, limit range is 4KB to 50KB
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param bufferSize The buffer size want to set, unit is byte.
 * @return  Returns the operation result status {@link BtStatus}.
 */
int SetSocketBufferSize(int clientId, uint32_t bufferSize)
{
    HILOGI("start, clientId: %{public}d, bufferSize: %{public}d", clientId, bufferSize);
    std::shared_ptr<ClientSocket> client = g_clientMap.GetObject(clientId);
    if (client == nullptr) {
        HILOGE("client is null! clientId: %{public}d", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = client->SetBufferSize(bufferSize);
    if (ret == RET_BAD_PARAM) {
        return OHOS_BT_STATUS_PARM_INVALID;
    } else if (ret == RET_BAD_STATUS) {
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}
/**
 * @brief Update the coc connection params
 *
 * @param param CocUpdateSocketParam instance for carry params.
 * @param bdAddr The remote device address to connect.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketUpdateCocConnectionParams(BluetoothCocUpdateSocketParam* param, const BdAddr *bdAddr)
{
    CocUpdateSocketParam params;

    HILOGI("Socket update coc params start");
    CHECK_AND_RETURN_LOG_RET(param, OHOS_BT_STATUS_FAIL, "param is null");
    CHECK_AND_RETURN_LOG_RET(bdAddr, OHOS_BT_STATUS_FAIL, "bdAddr is null");
    ConvertAddr(bdAddr->addr, params.addr);
    params.minInterval = param->minInterval;
    params.maxInterval = param->maxInterval;
    params.peripheralLatency = param->peripheralLatency;
    params.supervisionTimeout = param->supervisionTimeout;
    params.minConnEventLen = param->minConnEventLen;
    params.maxConnEventLen = param->maxConnEventLen;

    std::shared_ptr<BluetoothRemoteDevice> device = std::make_shared<BluetoothRemoteDevice>(params.addr,
        OHOS_SOCKET_SPP_RFCOMM);
    std::shared_ptr<ClientSocket> client = std::make_shared<ClientSocket>(*device, UUID::RandomUUID(),
        TYPE_L2CAP_LE, false);
    CHECK_AND_RETURN_LOG_RET(client, OHOS_BT_STATUS_FAIL, "client is null");
    return client->UpdateCocConnectionParams(params);
}

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif