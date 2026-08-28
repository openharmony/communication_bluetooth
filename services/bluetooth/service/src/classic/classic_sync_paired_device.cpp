/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_classic_paired"
#endif

#include "classic_sync_paired_device.h"
#include "adapter_manager.h"
#include "hw_bt_hwif.h"

namespace OHOS {
namespace bluetooth {

BluetoothSyncPairedDev& BluetoothSyncPairedDev::GetInstance()
{
    static BluetoothSyncPairedDev instance;
    return instance;
}

bool BluetoothSyncPairedDev::SyncPairedDev(std::vector<std::string> &pairedAddr)
{
    BtInterface *btIf = AdapterManager::GetInstance()->getBluetoothInterface();
    if (btIf == nullptr) {
        HILOGE("Failed to get bluetooth interface");
        return false;
    }

    const BthwifInterface *bthwif =
        reinterpret_cast<const BthwifInterface*>(btIf->getProfileInterface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("Failed to get hw bluetooth interface");
        return false;
    }

    return bthwif->needSyncPairedInfo(pairedAddr);
}

}
}
