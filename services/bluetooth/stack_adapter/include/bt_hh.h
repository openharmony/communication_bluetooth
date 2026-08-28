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
 * Stub of the removed stack layer HID host profile interface (bt_hh.h),
 * mirroring bluedroid system/include/hardware/bt_hh.h.
 */

#ifndef BT_HH_H
#define BT_HH_H

#include <cstddef>
#include <cstdint>

#include "bt_types.h"

#define BTHH_MAX_DSC_LEN 884

enum BthhConnectionState {
    BTHH_CONN_STATE_DISCONNECTED = 0,
    BTHH_CONN_STATE_CONNECTING,
    BTHH_CONN_STATE_CONNECTED,
    BTHH_CONN_STATE_DISCONNECTING,
};

enum BthhStatus {
    BTHH_ERR_NONE = 0,
    BTHH_ERR_DEV_NOT_CONNECTED,
};

enum BthhProtocolMode {
    BTHH_REPORT_MODE = 0x00,
    BTHH_BOOT_MODE = 0x01,
    BTHH_UNSUPPORTED_MODE = 0xff
};

/* Report types */
enum BthhReportType { BTHH_INPUT_REPORT = 1, BTHH_OUTPUT_REPORT, BTHH_FEATURE_REPORT };

struct BthhHidInfo {
    int attrMask;
    uint8_t subClass;
    uint8_t appId;
    int vendorId;
    int productId;
    int version;
    uint8_t ctryCode;
    int dlLen;
    uint8_t dscList[BTHH_MAX_DSC_LEN];
};

typedef void (*BthhConnectionStateCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                               BtTransport transport,
                                               BthhConnectionState state);

typedef void (*BthhVirtualUnplugCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                             BtTransport transport, BthhStatus hhStatus);

typedef void (*BthhHidInfoCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                       BtTransport transport, BthhHidInfo hidInfo);

typedef void (*BthhProtocolModeCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                            BtTransport transport, BthhStatus hhStatus,
                                            BthhProtocolMode mode);

typedef void (*BthhIdleTimeCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                        BtTransport transport, BthhStatus hhStatus,
                                        int idleRate);

typedef void (*BthhGetReportCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                         BtTransport transport, BthhStatus hhStatus,
                                         uint8_t *rptData, int rptSize);

typedef void (*BthhHandshakeCallback)(RawAddress *bdAddr, BleAddrType addrType,
                                        BtTransport transport, BthhStatus hhStatus);

/* BT-HH callback structure. */
struct BthhCallbacks {
    size_t size;
    BthhConnectionStateCallback connectionStateCb;
    BthhHidInfoCallback hidInfoCb;
    BthhProtocolModeCallback protocolModeCb;
    BthhIdleTimeCallback idleTimeCb;
    BthhGetReportCallback getReportCb;
    BthhVirtualUnplugCallback virtualUnplugCb;
    BthhHandshakeCallback handshakeCb;
};

/* Represents the standard BT-HH interface. */
struct BthhInterface {
    size_t size;
    BtStackStatus (*init)(BthhCallbacks *callbacks);
    BtStackStatus (*connect)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport);
    BtStackStatus (*disconnect)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport,
                              bool reconnectAllowed);
    BtStackStatus (*virtualUnplug)(RawAddress *bdAddr, BleAddrType addrType,
                                  BtTransport transport);
    BtStackStatus (*setInfo)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport,
                            BthhHidInfo hidInfo);
    BtStackStatus (*getProtocol)(RawAddress *bdAddr, BleAddrType addrType,
                                BtTransport transport, BthhProtocolMode protocolMode);
    BtStackStatus (*setProtocol)(RawAddress *bdAddr, BleAddrType addrType,
                                BtTransport transport, BthhProtocolMode protocolMode);
    BtStackStatus (*getIdleTime)(RawAddress *bdAddr, BleAddrType addrType,
                                 BtTransport transport);
    BtStackStatus (*setIdleTime)(RawAddress *bdAddr, BleAddrType addrType,
                                 BtTransport transport, uint8_t idleTime);
    BtStackStatus (*getReport)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport,
                              BthhReportType reportType, uint8_t reportId, int bufferSize);
    BtStackStatus (*getReportReply)(RawAddress *bdAddr, BleAddrType addrType,
                                    BtTransport transport, BthhStatus status, char *report,
                                    uint16_t size);
    BtStackStatus (*setReport)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport,
                              BthhReportType reportType, char *report);
    BtStackStatus (*sendData)(RawAddress *bdAddr, BleAddrType addrType, BtTransport transport,
                             char *data);
    void (*cleanup)(void);
    void (*configureEnabledProfiles)(bool enableHidp, bool enableHogp);
};

#endif  // BT_HH_H
