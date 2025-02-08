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

#ifndef OHOS_BT_SOCKET_H
#define OHOS_BT_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#include "ohos_bt_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BT_SOCKET_READ_SOCKET_CLOSED (0)
#define BT_SOCKET_READ_FAILED (-1)
#define BT_SOCKET_WRITE_FAILED (-1)
#define BT_SOCKET_INVALID_ID (-1)
#define BT_SOCKET_INVALID_PSM (-1)
#define BT_SOCKET_INVALID_SCN (-1)
#define BT_SOCKET_INVALID_TYPE (-2)
#define BT_SOCKET_LIMITED_RESOURCES (-3)

typedef enum {
    OHOS_SOCKET_SPP_RFCOMM = 0x0,
    OHOS_SOCKET_L2CAP_LE = 0x02,
} BluetoothSocketType;

typedef struct {
    /** uuid This parameter is required when type is {@link OHOS_SOCKET_SPP_RFCOMM} */
    BtUuid uuid;
    /** type type of socket */
    BluetoothSocketType socketType;
    /** encrypt require the connection to be encrypted */
    bool isEncrypt;
} BluetoothCreateSocketPara;

typedef struct {
    int minInterval;
    int maxInterval;
    int peripheralLatency;
    int supervisionTimeout;
    int minConnEventLen;
    int maxConnEventLen;
} BluetoothCocUpdateSocketParam;

/**
 * @brief callback invoked when the socket connection state changed.
 * @param bdAddr Indicates the ID of the GATT client.
 * @param uuid This parameter is required when type is {@link OHOS_SOCKET_SPP_RFCOMM}.
 * @param status Indicates the connection status {@link ConnStatus}.
 * @param result Indicates the operation result.
 */
typedef void (*SocketConnectionStateChangedCallback)(const BdAddr *bdAddr, BtUuid uuid, int status, int result);
typedef void (*SocketBleConnectionStateChangedCallback)(const BdAddr *bdAddr, int psm, int status, int result);
typedef struct {
    SocketConnectionStateChangedCallback connStateCb;
    SocketBleConnectionStateChangedCallback bleConnStateCb;
} BtSocketConnectionCallback;

/**
 * @brief Creates an server listening socket based on the service record.
 *
 * @param socketPara The parameters to create a server socket.
 * @param name The service's name.
 * @return Returns a server ID, if create fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketServerCreate(const BluetoothCreateSocketPara *socketPara, const char *name);

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
int SocketServerAccept(int serverId);

/**
 * @brief Disables an socket server socket and releases related resources.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketServerClose(int serverId);

/**
 * @brief Set fast connection flag
 *
 * @param bdAddr The remote device address to connect.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketSetFastConnection(const BdAddr *bdAddr);

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
int SocketConnect(const BluetoothCreateSocketPara *socketPara, const BdAddr *bdAddr, int psm);

/**
 * @brief Connects to a remote device over the socket.
 * This method will block until a connection is made or the connection fails.
 *
 * @param socketPara The param to create a client socket and connect to a remote device.
 * @param bdAddr The remote device address to connect.
 * @param psm BluetoothSocketType is {@link OHOS_SOCKET_L2CAP_LE} use dynamic PSM value from remote device.
 * BluetoothSocketType is {@link OHOS_SOCKET_SPP_RFCOMM} use -1.
 * @param callback Reference to the socket state observer.
 * @return Returns a client ID, if connect fail return {@link BT_SOCKET_INVALID_ID}.
 */
int SocketConnectEx(const BluetoothCreateSocketPara *socketPara, const BdAddr *bdAddr, int psm,
    BtSocketConnectionCallback *callback);

/**
 * @brief Disables a connection and releases related resources.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketDisconnect(int clientId);

/**
 * @brief Get the connection status of this socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @return Returns true is connected or false is not connected.
 */
bool IsSocketConnected(int clientId);

/**
 * @brief Socket get remote device's address.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param remoteAddr Remote device's address, memory allocated by caller.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketGetRemoteAddr(int clientId, BdAddr *remoteAddr);

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
int SocketRead(int clientId, uint8_t *buf, uint32_t bufLen);

/**
 * @brief Client write data to socket.
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param data Indicate the data to be written.
 * @param len Indicates the length of the data to be written.
 * @return Returns the actual write length.
 * Returns {@link BT_SOCKET_WRITE_FAILED} if the operation failed.
 */
int SocketWrite(int clientId, const uint8_t *data, uint32_t len);

/**
 * @brief Get dynamic PSM value for OHOS_SOCKET_L2CAP.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the PSM value.
 * Returns {@link BT_SOCKET_INVALID_PSM} if the operation failed.
 */
int SocketGetPsm(int serverId);

/**
 * @brief Get server scm number for OHOS_SOCKET_RFCOMM.
 *
 * @param serverId The relative ID used to identify the current server socket, obtain the value by calling
 * {@link SocketServerCreate}.
 * @return Returns the scn number.
 * Returns {@link BT_SOCKET_INVALID_SCN} if the operation failed.
 */
int SocketGetScn(int serverId);

/**
 * @brief Adjust the socket send and recv buffer size, limit range is 4KB to 50KB
 *
 * @param clientId The relative ID used to identify the current client socket.
 * @param bufferSize The buffer size want to set, unit is byte.
 * @return  Returns the operation result status {@link BtStatus}.
 */
int SetSocketBufferSize(int clientId, uint32_t bufferSize);

/**
 * @brief Update the coc connection params.
 *
 * @param param CocUpdateSocketParam instance for carry params.
 * @return Returns the operation result status {@link BtStatus}.
 */
int SocketUpdateCocConnectionParams(BluetoothCocUpdateSocketParam* param, const BdAddr *bdAddr);

#ifdef __cplusplus
}
#endif
#endif