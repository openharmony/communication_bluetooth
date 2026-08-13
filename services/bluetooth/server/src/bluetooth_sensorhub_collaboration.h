/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SENSORHUB_COLLABORATION_H
#define BLUETOOTH_SENSORHUB_COLLABORATION_H

#include "bluetooth_types.h"
#include "ble_scanner_state_machine.h"

namespace OHOS {
namespace Bluetooth {

void SendScanCollaborationMsg(int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos);
bool IsScanCollborationEnabled(void);

class SensorhubCollaboration {
public:
    using ScannerMachineNotifyCallback = std::function<void(const BleScannerMessage &)>;
    explicit SensorhubCollaboration(ScannerMachineNotifyCallback smNotifyCb);
    ~SensorhubCollaboration();

    void QueryCapability();
    void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable);

private:
    BLUETOOTH_DECLARE_IMPL();
};

}
}

#endif
