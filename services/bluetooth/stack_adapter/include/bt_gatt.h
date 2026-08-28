/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer GATT profile glue header (bt_gatt.h).
 */

#ifndef BT_GATT_H
#define BT_GATT_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "bt_types.h"
#include "btm_ble_api_types.h"
#include "bt_uuid.h"
#include "gatt_api.h"

#define BLE_GATT_MAX_ATTR_NUM 1000
#define BLE_GATT_ATTR_HANDLE_START 0x0001
#define BLE_GATT_ATTR_HANDLE_END 0xFFFF

/* Max length of a GATT attribute value (bluedroid GATT_MAX_ATTR_LEN). */
#define BTGATT_MAX_ATTR_LEN 512

/* Timeouts used by the gatt services; values match config/bt_config.h which
 * may be included first by the same translation unit. */
#ifndef BLUETOOTH_GATT_CLIENT_TIMEOUT
#define BLUETOOTH_GATT_CLIENT_TIMEOUT 1 // 1s
#endif  // BLUETOOTH_GATT_CLIENT_TIMEOUT
#ifndef BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT
#define BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT 2 // 2s
#endif  // BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT

enum BleGattConnect {
    BLE_GATT_CONNECT_OK = 0,
    BLE_GATT_CONNECT_FAIL,
    BLE_GATT_CONNECT_CANCEL,
};

enum BleGattDisconnectReason {
    BLE_GATT_DISCONNECT_REASON_NORMAL = 0,
    BLE_GATT_DISCONNECT_REASON_REMOTE,
};

enum BleGattServiceType {
    BLE_GATT_SERVICE_PRIMARY = 0x01,
    BLE_GATT_SERVICE_SECONDARY = 0x02,
};

enum BleGattAttrType {
    BLE_GATT_ATTR_PRIMARY_SERVICE = 0,
    BLE_GATT_ATTR_SECONDARY_SERVICE,
    BLE_GATT_ATTR_INCLUDE_SERVICE,
    BLE_GATT_ATTR_CHARACTERISTIC,
    BLE_GATT_ATTR_CHARACTERISTIC_DECLARATION,
    BLE_GATT_ATTR_DESCRIPTOR,
};

enum BleGattPerm {
    BLE_GATT_PERM_READ = 0x0001,
    BLE_GATT_PERM_READ_ENCRYPTED = 0x0002,
    BLE_GATT_PERM_READ_ENCRYPTED_MITM = 0x0004,
    BLE_GATT_PERM_WRITE = 0x0010,
    BLE_GATT_PERM_WRITE_ENCRYPTED = 0x0020,
    BLE_GATT_PERM_WRITE_ENCRYPTED_MITM = 0x0040,
    BLE_GATT_PERM_WRITE_SIGNED = 0x0080,
    BLE_GATT_PERM_WRITE_SIGNED_MITM = 0x0100,
};

enum BleGattCharProp {
    BLE_GATT_CHAR_PROP_BROADCAST = 0x01,
    BLE_GATT_CHAR_PROP_READ = 0x02,
    BLE_GATT_CHAR_PROP_WRITE_NO_RSP = 0x04,
    BLE_GATT_CHAR_PROP_WRITE = 0x08,
    BLE_GATT_CHAR_PROP_NOTIFY = 0x10,
    BLE_GATT_CHAR_PROP_INDICATE = 0x20,
    BLE_GATT_CHAR_PROP_SIGNED_WRITE = 0x40,
};

struct BleGattUuid {
    uint8_t uuid[16];
};

/* Service database element types reported by the stack. */
#define BTGATT_DB_PRIMARY_SERVICE 0
#define BTGATT_DB_SECONDARY_SERVICE 1
#define BTGATT_DB_INCLUDED_SERVICE 2
#define BTGATT_DB_CHARACTERISTIC 3
#define BTGATT_DB_DESCRIPTOR 4

/* Gatt attribute value, see BtgattReadParams. */
struct BtGattValue {
    uint16_t len;
    uint8_t *value;
};

/* GATT value filled in the response to a remote read request, and the
 * response union (bluedroid system/include/hardware/bt_gatt_server.h). */
struct BtgattValue {
    uint8_t value[BTGATT_MAX_ATTR_LEN];
    uint16_t handle;
    uint16_t offset;
    uint16_t len;
    uint8_t authReq;
};

union BtgattResponse {
    BtgattValue attrValue;
    uint16_t handle;
};

struct BtgattDbElement {
    OHOS::bluetooth::Uuid uuid;
    uint8_t type;
    uint16_t attributeHandle;
    uint16_t startHandle;
    uint16_t endHandle;
    uint8_t properties;
    uint32_t permissions;
};

struct BtgattNotifyParams {
    RawAddress bda;
    uint16_t handle;
    uint16_t len;
    uint8_t *value;
    bool isConnected;
};

struct BtgattReadParams {
    uint16_t handle;
    uint8_t valueType;
    uint16_t status;
    BtGattValue value;
};

struct BtgattRspParams {
    uint64_t timeStamp;
};

struct BtgattTrackAdvInfo {
    RawAddress bdAddr;
    int clientIf;
    int advertiserState;
    int addrType;
    int rssiValue;
    uint8_t advPktLen;
    uint8_t *pAdvPktData;
    uint8_t scanRspLen;
    uint8_t *pScanRspData;
};

struct BtgattClientCallbacks;
struct BtgattServerCallbacks;
struct BtgattScannerCallbacks;

/* Callbacks delivered to the stack through BtgattInterface::init. */
struct BtgattCallbacks {
    size_t size;
    const BtgattClientCallbacks *clientCb;
    const BtgattServerCallbacks *serverCb;
    const BtgattScannerCallbacks *scannerCb;
};

struct BtgattScannerCallbacks {
    void (*scanResultCb)(uint16_t bleEvtType, uint8_t addrType, RawAddress *bda, uint8_t blePrimaryPhy,
        uint8_t bleSecondaryPhy, uint8_t bleAdvertisingSid, int8_t bleTxPower, int8_t rssi,
        uint16_t blePeriodicAdvInt, std::vector<uint8_t> advData);
    void (*batchscanReportsCb)(int clientIf, int status, int reportFormat, int numRecords,
        std::vector<uint8_t> data);
    void (*batchscanThresholdCb)(int clientIf);
    void (*trackAdvEventCb)(BtgattTrackAdvInfo *advTrackInfo);
};

struct BtgattServerCallbacks {
    void (*registerServerCb)(int status, int serverIf, const OHOS::bluetooth::Uuid &appUuid);
    void (*connectionCb)(int connId, int serverIf, int connected, const RawAddress &bda, int reason);
    void (*serviceAddedCb)(int status, int serverIf, std::vector<BtgattDbElement> service);
    void (*serviceStoppedCb)(int status, int serverIf, int serviceHandle);
    void (*serviceDeletedCb)(int status, int serverIf, int serviceHandle);
    void (*requestReadCharacteristicCb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool isLong);
    void (*requestReadDescriptorCb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool isLong);
    void (*requestWriteCharacteristicCb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value);
    void (*requestWriteDescriptorCb)(int connId, int transId, const RawAddress &bda, int attrHandle,
        int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value);
    void (*requestExecWriteCb)(int connId, int transId, const RawAddress &bda, int execWrite);
    void (*responseConfirmationCb)(int status, int handle);
    void (*indicationSentCb)(int connId, int status);
    void (*congestionCb)(int connId, bool congested);
    void (*mtuChangedCb)(int connId, int mtu);
    void (*phyUpdatedCb)(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status);
    void (*connUpdatedCb)(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status);
};

struct BtgattClientCallbacks {
    void (*registerClientCb)(int status, int clientIf, const OHOS::bluetooth::Uuid &appUuid);
    void (*connectCb)(int connId, int status, int clientIf, const RawAddress &bda);
    void (*disconnectCb)(int connId, int status, int clientIf, const RawAddress &bda, int reason);
    void (*cancelOpenCb)(int connId, int status, int clientIf, const RawAddress &bda);
    void (*searchCompleteCb)(int connId, int status);
    void (*registerForNotificationCb)(int connId, int registered, int status, uint16_t handle);
    void (*notifyCb)(int connId, const BtgattNotifyParams &data);
    void (*readCharacteristicCb)(int connId, int status, BtgattReadParams *pData);
    void (*writeCharacteristicCb)(int connId, int status, uint16_t handle, const BtgattRspParams &rspContext);
    void (*readDescriptorCb)(int connId, int status, const BtgattReadParams &data);
    void (*writeDescriptorCb)(int connId, int status, uint16_t handle);
    void (*executeWriteCb)(int connId, int status);
    void (*readRemoteRssiCb)(int clientIf, const RawAddress &bda, int rssi, int status);
    void (*configureMtuCb)(int connId, int status, int mtu);
    void (*congestionCb)(int connId, bool congested);
    void (*getGattDbCb)(int connId, const BtgattDbElement *db, int count);
    void (*servicesRemovedCb)(int connId, uint16_t startHandle, uint16_t endHandle);
    void (*servicesAddedCb)(int connId, const BtgattDbElement &added, int addedCount);
    void (*phyUpdatedCb)(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status);
    void (*connUpdatedCb)(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status);
    void (*serviceChangedCb)(int connId);
};

/* Stack gatt interface, obtained via GetProfileInterface(BT_PROFILE_GATT_ID). */
struct BtgattInterface {
    size_t size;
    BtStackStatus (*init)(const BtgattCallbacks *callbacks);
};

/* GATT client interface consumed by the service layer (gatt_client_application.cpp);
 * signatures follow the call sites, bluedroid reference is
 * system/include/hardware/bt_gatt_client.h. */
struct BtgattClientInterface {
    size_t size;
    BtStackStatus (*registerClient)(const OHOS::bluetooth::Uuid &appUuid, bool eattSupport);
    BtStackStatus (*unregisterClient)(int clientIf);
    BtStackStatus (*connect)(int clientIf, const RawAddress &bdAddr, bool isDirect, int transport,
        bool opportunistic, int phy);
    BtStackStatus (*disconnect)(int clientIf, const RawAddress &bdAddr, int connId);
    BtStackStatus (*searchService)(int connId, const OHOS::bluetooth::Uuid *filterUuid);
    BtStackStatus (*getGattDb)(int connId);
    BtStackStatus (*readCharacteristic)(int connId, int handle, int authReq);
    BtStackStatus (*readUsingCharacteristicUuid)(int connId, const OHOS::bluetooth::Uuid &uuid,
        int startHandle, int endHandle, int authReq);
    BtStackStatus (*writeCharacteristic)(int connId, int handle, int writeType, int authReq,
        std::vector<uint8_t> value);
    BtStackStatus (*readDescriptor)(int connId, int handle, int authReq);
    BtStackStatus (*writeDescriptor)(int connId, int handle, int authReq,
        std::vector<uint8_t> value);
    BtStackStatus (*registerForNotification)(int clientIf, const RawAddress &bdAddr, int handle);
    BtStackStatus (*deregisterForNotification)(int clientIf, const RawAddress &bdAddr, int handle);
    BtStackStatus (*configureMtu)(int connId, int mtu);
    BtStackStatus (*connParameterUpdate)(const RawAddress &bdAddr, int minInterval,
        int maxInterval, int latency, int timeout, int minCeLen, int maxCeLen);
    BtStackStatus (*readRemoteRssi)(int clientIf, const RawAddress &bdAddr);
    BtStackStatus (*setPreferredPhy)(const RawAddress &bdAddr, uint8_t txPhy, uint8_t rxPhy,
        uint16_t phyOptions);
    BtStackStatus (*readPhy)(const RawAddress &bdAddr,
        std::function<void(uint8_t txPhy, uint8_t rxPhy, uint8_t status)> cb);
};

/* Null-initialized instance: the service layer detects the empty table and
 * takes its "stack unavailable" path (RegisterClient == nullptr checks). */
inline BtgattClientInterface btgattClientInterface {};

/* GATT server interface consumed by gatt_server_application.cpp; bluedroid
 * reference is system/include/hardware/bt_gatt_server.h. */
struct BtgattServerInterface {
    size_t size;
    BtStackStatus (*registerServer)(const OHOS::bluetooth::Uuid &uuid, bool eattSupport);
    BtStackStatus (*unregisterServer)(int serverIf);
    BtStackStatus (*connect)(int serverIf, const RawAddress &bdAddr, bool isDirect, int transport);
    BtStackStatus (*disconnect)(int serverIf, const RawAddress &bdAddr, int connId);
    BtStackStatus (*addService)(int serverIf, std::vector<BtgattDbElement> service);
    BtStackStatus (*deleteService)(int serverIf, int serviceHandle);
    BtStackStatus (*sendIndication)(int serverIf, int attributeHandle, int connId, int confirm,
        std::vector<uint8_t> value);
    BtStackStatus (*sendResponse)(int connId, int transId, int status,
        const BtgattResponse &response);
    BtStackStatus (*setPreferredPhy)(const RawAddress &bdAddr, uint8_t txPhy, uint8_t rxPhy,
        uint16_t phyOptions);
    BtStackStatus (*readPhy)(const RawAddress &bdAddr,
        std::function<void(uint8_t txPhy, uint8_t rxPhy, uint8_t status)> cb);
};

/* Null-initialized instance; gatt_server_service.cpp checks RegisterServer
 * before calling into the table. */
inline BtgattServerInterface btgattServerInterface {};

/* Scan filter parameter setup, see BleScanFilter::SetScanFilterParameters. */
struct BtgattFiltParamSetup {
    uint16_t featSeln;
    uint16_t listLogicType;
    uint8_t filtLogicType;
    int8_t rssiHighThres;
    int8_t rssiLowThres;
    uint8_t delyMode;
    uint16_t foundTimeout;
    uint16_t lostTimeout;
    uint16_t foundTimeoutCnt;
    uint16_t numOfTrackingEntries;
};

/* One scan filter command in APCF format, see BleScanFilter::GetCommand. */
struct ApcfCommand {
    uint8_t type;
    RawAddress address;
    uint8_t addrType;
    std::array<uint8_t, 16> irk;
    OHOS::bluetooth::Uuid uuid;
    OHOS::bluetooth::Uuid uuidMask;
    std::vector<uint8_t> name;
    uint16_t company;
    uint16_t companyMask;
    std::vector<uint8_t> data;
    std::vector<uint8_t> dataMask;
};

#endif  // BT_GATT_H
