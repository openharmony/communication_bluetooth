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

#include "ble_mock.h"
#include "gap_if.h"

BleMock *g_gapmocker;

void registerGapMock(BleMock *instance)
{
    g_gapmocker = instance;
}
GapAdvCallback g_gapAdvCallbacks;
GapScanCallback g_gapScanCallbacks;
GapLeSecurityCallback g_gapLeSecurityCallbacks;
GapLePairCallback g_gapLePairCallbacks;
GapExScanCallback g_gapExScanCallbacks;
GapExAdvCallback g_gapExAdvcallbacks;
GenResPriAddrResult g_genRescallback;
BleCentralManagerImpl *g_bleCentralManager = nullptr;
BleAdvertiserImpl *g_bleAdvertiser = nullptr;
BleSecurity *g_bleSecurity = nullptr;

GapAdvCallback *GetAdvCallback()
{
    return &g_gapAdvCallbacks;
}
GapScanCallback *GetScanCallback()
{
    return &g_gapScanCallbacks;
}
GapLeSecurityCallback *GetLeSecurityCallback()
{
    return &g_gapLeSecurityCallbacks;
}
GapLePairCallback *GetLePairCallback()
{
    return &g_gapLePairCallbacks;
}
GapExScanCallback *GetExScanCallback()
{
    return &g_gapExScanCallbacks;
}
GapExAdvCallback *GetExAdvCallback()
{
    return &g_gapExAdvcallbacks;
}
GenResPriAddrResult *GetGenRPACallback()
{
    return &g_genRescallback;
}
BleCentralManagerImpl *GetBleCentraManager()
{
    return g_bleCentralManager;
}
BleAdvertiserImpl *GetBleAdvertiser()
{
    return g_bleAdvertiser;
}
BleSecurity *GetBleSecurity()
{
    return g_bleSecurity;
}

int GAPIF_GetLocalAddr(BtAddr *addr)
{
    return BT_NO_ERROR;
}

int GAPIF_LeSetRole(uint8_t role)
{
    return g_gapmocker->GAPIF_LeSetRole();
}

int GAPIF_RegisterAdvCallback(const GapAdvCallback *cb, void *context)
{
    g_gapAdvCallbacks = *cb;
    if (context == nullptr) {
        return BT_NO_ERROR;
    } else {
        g_bleAdvertiser = static_cast<BleAdvertiserImpl *>(context);
    }

    return g_gapmocker->GAPIF_RegisterAdvCallback();
}

int GAPIF_LeAdvSetParam(uint8_t advType, GapLeAdvParam advParam)
{
    return g_gapmocker->GAPIF_LeAdvSetParam();
}

int GAPIF_LeAdvReadTxPower(void)
{
    return g_gapmocker->GAPIF_LeAdvReadTxPower();
}

int GAPIF_LeAdvSetData(uint8_t advDataLength, const uint8_t *advData)
{
    return g_gapmocker->GAPIF_LeAdvSetData();
}

int GAPIF_LeAdvSetScanRspData(uint8_t advDataLength, const uint8_t *advData)
{
    return g_gapmocker->GAPIF_LeAdvSetScanRspData();
}

int GAPIF_LeAdvSetEnable(uint8_t enable)
{
    return g_gapmocker->GAPIF_LeAdvSetEnable();
}

int GAPIF_LeScanSetParam(GapLeScanParam param, uint8_t scanFilterPolity)
{
    return g_gapmocker->GAPIF_LeScanSetParam();
}
int GAPIF_LeScanSetEnable(uint8_t scanEnable, uint8_t filterDuplicates)
{
    return g_gapmocker->GAPIF_LeScanSetEnable();
}

int GAPIF_LePair(const BtAddr *addr)
{
    return BT_NO_ERROR;
}

int GAPIF_RegisterLePairCallback(const GapLePairCallback *callback, void *context)
{
    if (context != nullptr) {
        g_bleSecurity = static_cast<BleSecurity *>(context);
    }
    g_gapLePairCallbacks = *callback;
    return 0;
}

int GAPIF_RegisterLeSecurityCallback(const GapLeSecurityCallback *callback, void *context)
{
    if (context != nullptr) {
        g_bleSecurity = static_cast<BleSecurity *>(context);
    }
    g_gapLeSecurityCallbacks = *callback;
    return 0;
}

int GAPIF_RegisterScanCallback(const GapScanCallback *callback, void *context)
{
    if (context != nullptr) {
        g_bleCentralManager = static_cast<BleCentralManagerImpl *>(context);
    }
    g_gapScanCallbacks = *callback;
    return 0;
}

int GAPIF_LeSetSecurityMode(GAP_LeSecMode1Level mode1Level, GAP_LeSecMode2Level mode2Level)
{
    return g_gapmocker->GAPIF_LeSetSecurityMode();
}

int GAPIF_LeSetMinEncKeySize(uint8_t minSize)
{
    return g_gapmocker->GAPIF_LeSetMinEncKeySize();
}

int GAPIF_LeSetBondMode(uint8_t mode)
{
    return g_gapmocker->GAPIF_LeSetBondMode();
}

int GAPIF_LeExAdvGetMaxDataLen(uint16_t *len)
{
    *len = 0x0200;
    return g_gapmocker->GAPIF_LeExAdvGetMaxDataLen();
}

int GAPIF_LeCancelPair(const BtAddr *addr)
{
    return g_gapmocker->GAPIF_LeCancelPair();
}

int GAPIF_DeregisterAdvCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_DeregisterScanCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_LeLocalEncryptionKeyRsp(const BtAddr *addr, uint8_t accept, LeEncKey localEncKey, uint8_t keyType)
{
    return BT_NO_ERROR;
}

int GAPIF_LeRemoteEncryptionKeyRsp(const BtAddr *addr, uint8_t accept, LeEncKey remoteEncKey, uint8_t keyType)
{
    return BT_NO_ERROR;
}

int GAPIF_RequestSigningAlgorithmInfoRsp(const BtAddr *addr, uint8_t accept, GapSigningAlgorithmInfo info)
{
    return BT_NO_ERROR;
}

int GAPIF_LePairFeatureRsp(const BtAddr *addr, GapLePairFeature localFrature)
{
    return 1;
}

int GAPIF_LeRequestSecurity(
    const BtAddr *addr, GAP_LeSecurityStatus status, GapLeRequestSecurityResult callback, void *context)
{
    callback(addr, 0x00, status, context);
    return BT_NO_ERROR;
}

int GAPIF_DeregisterLeSecurityCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_DeregisterLePairCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_LePairPassKeyRsp(const BtAddr *addr, uint8_t accept, uint32_t number)
{
    return BT_NO_ERROR;
}

int GAPIF_LePairOobRsp(const BtAddr *addr, uint8_t accept, uint8_t oobData[GAP_OOB_DATA_SIZE])
{
    return BT_NO_ERROR;
}

int GAPIF_LePairScOobRsp(const BtAddr *addr, uint8_t accept, const uint8_t oobDataC[GAP_OOB_DATA_CONFIRM_SIZE],
    const uint8_t oobDataR[GAP_OOB_DATA_RANDOM_SIZE])
{
    return BT_NO_ERROR;
}

int GAPIF_LePairScUserConfirmRsp(const BtAddr *addr, uint8_t accept)
{
    return 1;
}

int GAPIF_RegisterExAdvCallback(const GapExAdvCallback *callback, void *context)
{
    if (context == nullptr) {
        return BT_NO_ERROR;
    } else {
        g_bleAdvertiser = static_cast<BleAdvertiserImpl *>(context);
        g_gapExAdvcallbacks = *callback;
    }
    return g_gapmocker->GAPIF_RegisterExAdvCallback();
}

int GAPIF_LeExAdvSetRandAddr(uint8_t advHandle, const uint8_t addr[BT_ADDRESS_SIZE])
{
    return g_gapmocker->GAPIF_LeExAdvSetRandAddr();
}

int GAPIF_LeExAdvSetEnable(uint8_t enable, uint8_t numberOfSet, const GapExAdvSet *advSet)
{
    return g_gapmocker->GAPIF_LeExAdvSetEnable();
}

int GAPIF_LeExAdvSetParam(uint8_t advHandle, uint8_t properties, int8_t tx_power, GapLeExAdvParam advExParam)
{
    return g_gapmocker->GAPIF_LeExAdvSetParam();
}

int GAPIF_LeExAdvSetData(
    uint8_t advHandle, uint8_t operation, uint8_t fragmentPreference, uint8_t advDataLength, const uint8_t *advData)
{
    return g_gapmocker->GAPIF_LeExAdvSetData();
}

int GAPIF_LeExAdvSetScanRspData(uint8_t advHandle, uint8_t operation, uint8_t fragmentPreference,
    uint8_t scanResponseDataLength, const uint8_t *scanResponseData)
{
    return g_gapmocker->GAPIF_LeExAdvSetScanRspData();
}

int GAPIF_RegisterExScanCallback(const GapExScanCallback *callback, void *context)
{
    if (context != nullptr) {
        g_bleCentralManager = static_cast<BleCentralManagerImpl *>(context);
    }
    g_gapExScanCallbacks = *callback;
    return g_gapmocker->GAPIF_RegisterExScanCallback();
}

int GAPIF_LeExScanSetParam(uint8_t scanFilterPolity, uint8_t scanPhys, const GapLeScanParam param[])
{
    return g_gapmocker->GAPIF_LeExScanSetParam();
}

int GAPIF_LeExScanSetEnable(uint8_t scanEnable, uint8_t filterDuplicates, uint16_t duration, uint16_t period)
{
    return g_gapmocker->GAPIF_LeExScanSetEnable();
}

int GAPIF_LeSetStaticIdentityAddr(uint8_t addr[BT_ADDRESS_SIZE])
{
    return 0;
}

int GAPIF_LeGenResPriAddr(GenResPriAddrResult callback, void *context)
{
    g_genRescallback = callback;
    return 1;
}

int GAPIF_LeExAdvGetMaxHandleNum(uint8_t *num)
{
    *num = 0x07;
    return *num;
}

int GAPIF_DeregisterExAdvCallback(void)
{
    return BT_NO_ERROR;
}

int GAPIF_LeExAdvClearHandle(void)
{
    return 1;
}

int GAPIF_DeregisterExScanCallback(void)
{
    return BT_NO_ERROR;
}