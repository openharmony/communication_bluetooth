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

#include "gap_if.h"
#include "src/gap/gap_def.h"

#include <string.h>

#include "allocator.h"
#include "mutex.h"
#include "list.h"
#include "alarm.h"
#include "module.h"
#include "log.h"

#include "btstack.h"
#include "btm.h"
#include "btm/btm_controller.h"
#include "btm/btm_thread.h"
#include "btm/btm_inq_db.h"
#include "btm/btm_le_sec.h"
#include "hci/hci.h"
#include "hci/hci_error.h"
#include "smp/smp.h"
#include "smp/smp_def.h"

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s enter", __FUNCTION__);

    return 0;
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}


int GAPIF_LeGetSecurityStatus(const BtAddr *addr, GAP_LeSecurityStatus *status, uint8_t *encKeySize)
{
    *status = GAP_LE_UNAUTHENTICATED_ENCRYPTION;
    return GAP_SUCCESS;
}

int GAPIF_RegisterLeConnCallback(const GapLeConnCallback *callback, void *context)
{
    uint8_t status = 1;
    BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    uint16_t connInterval = 0x01;
    uint16_t connLatency = 0x01;
    uint16_t timeout = 0x0C81;
    uint16_t connIntervalMin = 0x07;
    uint16_t connIntervalMax = 0x07;
    if (callback->leConnectionUpdateComplete) {
        callback->leConnectionUpdateComplete(status, &btAddr, connInterval, connLatency, timeout, 0);
    }
    if (callback->leConnectionParameterReq) {
        callback->leConnectionParameterReq(&btAddr, connIntervalMin, connIntervalMax, connLatency, timeout, 0);
    }
    return 0;
}

int GAPIF_LeConnParamUpdate(const BtAddr *addr, const GapLeConnectionParameter *connParam)
{
    return 0;
}

int GAPIF_LeConnectionParameterRsp(const BtAddr *addr, uint8_t accept, const GapLeConnectionParameter *connParam)
{
    return 0;
}

int GAPIF_DeregisterLeConnCallback(void)
{
    return 0;
}