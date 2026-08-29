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
 * Stub of the removed stack layer btif sdp glue (btif_sdp.h).
 */

#ifndef BTIF_SDP_H
#define BTIF_SDP_H

#include "bt_types.h"

/* SDP record types of the removed stack layer HAL (hardware/bt_sdp.h); only
 * the entries referenced by the service layer are kept. */
enum BluetoothSdpTypes {
    SDP_TYPE_RAW = 0,
    SDP_TYPE_MAP_MAS,
    SDP_TYPE_MAP_MNS,
    SDP_TYPE_PBAP_PSE,
    SDP_TYPE_PBAP_PCE,
    SDP_TYPE_OPP_SERVER,
    SDP_TYPE_SAP_SERVER,
    SDP_TYPE_DIP,
    SDP_TYPE_MPS,
};

struct BluetoothSdpHdrOverlay {
    BluetoothSdpTypes type;
    OHOS::bluetooth::Uuid uuid;
    uint32_t serviceNameLength;
    const char *serviceName;
    int32_t rfcommChannelNumber;
    int32_t l2capPsm;
    int32_t profileVersion;

    /* User pointers, only used for some signals (bluedroid bt_sdp.h). */
    int user1PtrLen;
    const uint8_t *user1Ptr;
    int user2PtrLen;
    const uint8_t *user2Ptr;
};

struct BluetoothSdpMasRecord {
    BluetoothSdpHdrOverlay hdr;
    uint32_t masInstanceId;
    uint32_t supportedFeatures;
    uint32_t supportedMessageTypes;
};

struct BluetoothSdpMnsRecord {
    BluetoothSdpHdrOverlay hdr;
    uint32_t supportedFeatures;
};

struct BluetoothSdpPseRecord {
    BluetoothSdpHdrOverlay hdr;
    uint32_t supportedFeatures;
    uint32_t supportedRepositories;
};

#define SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH 15

struct BluetoothSdpOpsRecord {
    BluetoothSdpHdrOverlay hdr;
    int supportedFormatsListLen;
    uint8_t supportedFormatsList[SDP_OPP_SUPPORTED_FORMATS_MAX_LENGTH];
};

struct BluetoothSdpSapRecord {
    BluetoothSdpHdrOverlay hdr;
};

struct BluetoothSdpDipRecord {
    BluetoothSdpHdrOverlay hdr;
    uint16_t specId;
    uint16_t vendor;
    uint16_t vendorIdSource;
    uint16_t product;
    uint16_t version;
    bool primaryRecord;
};

struct BluetoothSdpMpsRecord {
    BluetoothSdpHdrOverlay hdr;
    uint8_t supportedScenariosMpsd[8];
    uint8_t supportedScenariosMpmd[8];
    uint8_t supportedDependencies[2];
};

struct BluetoothSdpRecord {
    BluetoothSdpHdrOverlay hdr;
    union {
        BluetoothSdpMasRecord mas;
        BluetoothSdpMnsRecord mns;
        BluetoothSdpPseRecord pse;
        BluetoothSdpOpsRecord ops;
        BluetoothSdpSapRecord sap;
        BluetoothSdpDipRecord dip;
        BluetoothSdpMpsRecord mps;
    };
};

/* Callback invoked when an SDP search completes. */
typedef void (*BtsdpSearchCallback)(BtStackStatus status, const OHOS::bluetooth::RawAddress &bdAddr,
    const OHOS::bluetooth::Uuid &uuid, int numRecords, BluetoothSdpRecord *records);

struct BtsdpCallbacks {
    size_t size;
    BtsdpSearchCallback sdpSearchCb;
};

/* SDP interface of the removed stack layer HAL (hardware/bt_sdp.h); only the
 * members referenced by the service layer are kept. */
struct BtsdpInterface {
    size_t size;
    BtStackStatus (*init)(BtsdpCallbacks *callbacks);
    BtStackStatus (*deinit)();
    BtStackStatus (*sdpSearch)(OHOS::bluetooth::RawAddress *bdAddr, const OHOS::bluetooth::Uuid &uuid);
    BtStackStatus (*createSdpRecord)(BluetoothSdpRecord *record, int *recordHandle);
    BtStackStatus (*removeSdpRecord)(int sdpHandle);
};

#endif  // BTIF_SDP_H
