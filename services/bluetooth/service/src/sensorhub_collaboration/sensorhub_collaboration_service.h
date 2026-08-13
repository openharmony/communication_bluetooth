/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef SENSORHUB_COLLABORATION_SERVICE_H
#define SENSORHUB_COLLABORATION_SERVICE_H

#include "interface_sensorhub_collaboration.h"
#include "base_def.h"

namespace OHOS {
namespace bluetooth {

enum class ScreenState : int {
    SCREEN_OFF = 1,
    SCREEN_ON,
};

enum class SensorhubConnState : int {
    NONE,
    COLLABORATED,
    WAITING_FOR_RESULT,
};

class SensorhubCollaborationService : public ISensorhubCollaboration {
public:
    SensorhubCollaborationService();
    ~SensorhubCollaborationService();

    static SensorhubCollaborationService *GetInstance();

    bool RegisterShCollaborationObserver(ISensorhubCollaborationObserver &observer) const override;
    bool DeregisterShCollaborationObserver(ISensorhubCollaborationObserver &observer) const override;
    bool IsScanCollborationEnabled() override;
    void SendScanCollaborationMsg(uint8_t type,
        const std::vector<SensorhubScannerInfo> &scannerInfo) const override;
    void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) override;
    void QueryCapability() override;

    void Init();

    bool StartConnCollaboration(const std::string &connName);
    void AllConnectionComplete(const std::string &connName);

private:
    void OnScreenStateChanged(ScreenState state) const;
    void RegisterScreenStateCallback() const;

    struct Stats;

    BT_DISALLOW_COPY_AND_ASSIGN(SensorhubCollaborationService);
    DECLARE_IMPL();
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // !GATT_CLIENT_SERVICE_H
