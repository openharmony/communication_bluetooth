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
 * Stub of the removed stack layer HID device profile interface (bt_hd.h).
 */

#ifndef BT_HD_H
#define BT_HD_H

#include <cstddef>

#include "bt_types.h"

enum BthdConnectionState {
    BTHD_CONN_STATE_DISCONNECTED = 0,
    BTHD_CONN_STATE_CONNECTING,
    BTHD_CONN_STATE_CONNECTED,
    BTHD_CONN_STATE_DISCONNECTING,
};

/* Report types accepted by BthdInterface::SendReport. */
enum BthdReportType {
    BTHD_REPORT_TYPE_OTHER = 0,
    BTHD_REPORT_TYPE_INPUT,
    BTHD_REPORT_TYPE_OUTPUT,
    BTHD_REPORT_TYPE_FEATURE,
    /* Special value for reports sent on the interrupt channel (INPUT assumed). */
    BTHD_REPORT_TYPE_INTRDATA
};

/* Application registration state reported by the application state callback. */
enum BthdApplicationState {
    BTHD_APP_STATE_NOT_REGISTERED = 0,
    BTHD_APP_STATE_REGISTERED,
};

/* HID device application parameters of BthdInterface::RegisterApp. */
struct BthdAppParam {
    const char *name;
    const char *description;
    const char *provider;
    uint8_t subclass;
    uint8_t *descList;
    int descListLen;
};

/* HID device QoS parameters of BthdInterface::RegisterApp. */
struct BthdQosParam {
    uint8_t serviceType;
    uint32_t tokenRate;
    uint32_t tokenBucketSize;
    uint32_t peakBandwidth;
    uint32_t accessLatency;
    uint32_t delayVariation;
};

/* HID device callbacks (bluedroid system/include/hardware/bt_hd.h). */
typedef void (*BthdApplicationStateCallback)(RawAddress *bdAddr,
                                                BthdApplicationState state);
typedef void (*BthdConnectionStateCallback)(RawAddress *bdAddr,
                                               BthdConnectionState state);
typedef void (*BthdGetReportCallback)(uint8_t type, uint8_t id, uint16_t bufferSize);
typedef void (*BthdSetReportCallback)(uint8_t type, uint8_t id, uint16_t len,
                                         uint8_t *pData);
typedef void (*BthdSetProtocolCallback)(uint8_t protocol);
typedef void (*BthdIntrDataCallback)(uint8_t reportId, uint16_t len, uint8_t *pData);
typedef void (*BthdVcUnplugCallback)(void);

struct BthdCallbacks {
    size_t size;
    BthdApplicationStateCallback applicationStateCb;
    BthdConnectionStateCallback connectionStateCb;
    BthdGetReportCallback getReportCb;
    BthdSetReportCallback setReportCb;
    BthdSetProtocolCallback setProtocolCb;
    BthdIntrDataCallback intrDataCb;
    BthdVcUnplugCallback vcUnplugCb;
};

/* HID device interface consumed by hid_device and watch services; layout
 * mirrors bluedroid system/include/hardware/bt_hd.h. */
struct BthdInterface {
    size_t size;
    BtStackStatus (*init)(BthdCallbacks *callbacks);
    void (*cleanup)(void);
    BtStackStatus (*registerApp)(BthdAppParam *appParam, BthdQosParam *inQos,
                                BthdQosParam *outQos);
    BtStackStatus (*unregisterApp)(void);
    BtStackStatus (*connect)(RawAddress *bdAddr);
    BtStackStatus (*disconnect)(void);
    BtStackStatus (*sendReport)(BthdReportType type, uint8_t id, uint16_t len,
                               uint8_t *pData);
    BtStackStatus (*reportError)(uint8_t error);
    BtStackStatus (*virtualCableUnplug)(void);
};

#endif  // BT_HD_H
