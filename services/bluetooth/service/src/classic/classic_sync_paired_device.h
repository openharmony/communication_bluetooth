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
 
/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines classic bond info synchronous functions.
 *
 */
 
/**
 * @file classic_bondinfo_sync.h
 *
 * @brief Classic bond info synchronous.
 *
 */
 
#ifndef BLUETOOTH_SYNC_PAIRED_DEVICE_H
#define BLUETOOTH_SYNC_PAIRED_DEVICE_H

#include <string>
#include <vector>

namespace OHOS {
namespace bluetooth {

class BluetoothSyncPairedDev {
public:
    /**
     * @brief Get BluetoothSyncPairedDev singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     */
    static BluetoothSyncPairedDev& GetInstance();

    /**
     * @brief sync paired device info.
     *
     * @param pairedAddr , vector of paired device address.
     * @return Returns <b>true</b> if need sync;
     *         returns <b>false</b> if no need sync.
     */
    bool SyncPairedDev(std::vector<std::string> &pairedAddr);
private:
    /**
     * @brief A constructor used to create a <b>BluetoothSyncPairedDev</b> instance.
     *
     */
    BluetoothSyncPairedDev() = default;
};
}
}

#endif