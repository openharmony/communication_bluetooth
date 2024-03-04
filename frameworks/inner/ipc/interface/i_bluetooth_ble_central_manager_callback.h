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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_CALLBACK_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_CALLBACK_INTERFACE_H

#include "../parcel/bluetooth_ble_scan_result.h"
#include "../common/ble_service_data.h"
#include "../common/bt_def.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothBleCentralManagerCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothBleCentralManagerCallback");

    virtual void OnScanCallback(const BluetoothBleScanResult &result,
        uint8_t callbackType = bluetooth::BLE_SCAN_CALLBACK_TYPE_ALL_MATCH) = 0;
    virtual void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) = 0;
    virtual void OnStartOrStopScanEvent(int resultCode, bool isStartScan) = 0;
    virtual void OnNotifyMsgReportFromLpDevice(const bluetooth::Uuid &uuid, int msgType,
        const std::vector<uint8_t> &notifyValue) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_CALLBACK_INTERFACE_H