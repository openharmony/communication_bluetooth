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

#include "bluetooth_sensorhub_collaboration.h"
#include <set>
#include "ble_defs.h"
#include "bluetooth_log.h"
#include "ble_scanner_state_machine.h"
#include "interface_adapter_manager.h"
#include "interface_profile_gatt_client.h"
#include "interface_sensorhub_collaboration.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

void SendScanCollaborationMsg(int type, const std::vector<BleScannerStateMachine::ScannerAppInfo> &appInfos)
{
    std::vector<SensorhubScannerInfo> shScannerInfo;
    for (const BleScannerStateMachine::ScannerAppInfo &info : appInfos) {
        shScannerInfo.emplace_back(info.scannerId, 0, info.params.scanInterval, info.params.scanWindow);
    }
    ISensorhubCollaboration::GetInstance()->SendScanCollaborationMsg(type, shScannerInfo);
}

bool IsScanCollborationEnabled(void)
{
    return ISensorhubCollaboration::GetInstance()->IsScanCollborationEnabled();
}

namespace {
int ConvertScanParamToScanMode(uint16_t interval, uint16_t window)
{
    for (int scanMode = SCAN_MODE_LOW_POWER; scanMode < SCAN_MODE_OP_MAX; scanMode++) {
        uint16_t tmpInterval = 0;
        uint16_t tmpWindow = 0;
        ConvertBleScanMode(scanMode, tmpWindow, tmpInterval);
        if (tmpInterval == interval && tmpWindow == window) {
            return scanMode;
        }
    }

    return -1;
}
}

struct SensorhubCollaboration::impl {
    impl();
    ~impl();

    /// power mgr state observer
    class SensorhubCollaborationObserver;
    std::unique_ptr<SensorhubCollaborationObserver> shCollaboraionObserver_ = nullptr;

    ScannerMachineNotifyCallback smNotifyCb_ = {};
};

class SensorhubCollaboration::impl::SensorhubCollaborationObserver : public ISensorhubCollaborationObserver {
public:
    explicit SensorhubCollaborationObserver(SensorhubCollaboration::impl *pimpl) : pimpl_(pimpl){};

    void OnSensorhubStateChanged(const SensorhubState state) override
    {
        if (state == SensorhubState::SENSORHUB_SCAN_REVMOED) {
            HILOGE("notify sensorhub scan removed");
            pimpl_->smNotifyCb_(BleScannerMessage(
                BleScannerStateMachine::SH_COLLABORATION_EVENT,
                BleScannerStateMachine::SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN));
        } else {
            HILOGE("unsupported state %{public}d", state);
        }
    }

    void OnSensorhubScanCollaborationMsgRecved(const SensorhubScannerInfo &shScannerInfo) override
    {
        uint16_t scanInterval = shScannerInfo.scanInterval;
        uint16_t scanWindow = shScannerInfo.scanWindow;
        if (scanInterval == 0 || scanWindow == 0) {
            HILOGI("scan in lp device is not started");
            pimpl_->smNotifyCb_(BleScannerMessage(
                BleScannerStateMachine::SH_COLLABORATION_EVENT,
                BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS));
            return;
        }

        int scanMode = ConvertScanParamToScanMode(scanInterval, scanWindow);
        if (scanMode == -1) {
            HILOGE("can't find scan mode for scanInterval: %{public}hu, scanWindow: %{public}hu",
                scanInterval, scanWindow);
            return;
        }

        HILOGI("scanInterval: %{public}hu, scanWindow: %{public}hu, scanMode: %{public}d",
            scanInterval, scanWindow, scanMode);

        BleScannerStateMachine::ScannerParameters param(0, scanMode, true, 0);
        pimpl_->smNotifyCb_(BleScannerMessage(
            BleScannerStateMachine::SH_COLLABORATION_EVENT,
            BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
            BleScannerStateMachine::ScannerAppInfo(0, 0, 0, param)));
    }

    void OnSensorhubConnCollaborationCompleted(void) override {}

private:
    SensorhubCollaboration::impl *pimpl_ = nullptr;
};

SensorhubCollaboration::impl::impl()
{
}

SensorhubCollaboration::impl::~impl()
{
}

SensorhubCollaboration::SensorhubCollaboration(ScannerMachineNotifyCallback smNotifyCb)
{
    pimpl = std::make_unique<impl>();
    pimpl->smNotifyCb_ = smNotifyCb;
    pimpl->shCollaboraionObserver_ = std::make_unique<impl::SensorhubCollaborationObserver>(pimpl.get());
    ISensorhubCollaboration::GetInstance()->RegisterShCollaborationObserver(*(pimpl->shCollaboraionObserver_));
}

// donot DeregisterShCollaborationObserver here, avoid the deconstructor called misorder and lead to coredump
SensorhubCollaboration::~SensorhubCollaboration()
{
}

void SensorhubCollaboration::QueryCapability()
{
    ISensorhubCollaboration::GetInstance()->QueryCapability();
}

void SensorhubCollaboration::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    ISensorhubCollaboration::GetInstance()->SetScanReportChannelToLpDevice(scannerId, enable);
}

}
}
