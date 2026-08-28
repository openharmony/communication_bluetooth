/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_RESOURCE_MANAGER_H
#define BLUETOOTH_RESOURCE_MANAGER_H

#include "interface_profile_resource_manager.h"
#include "base_def.h"
#include "safe_map.h"
#include "ble_service_data.h"
#include "bluetooth_resource_manager_def.h"
#include "btcommon/timer_manager.h"
#include "hw_bt_hwif.h"

namespace OHOS {
namespace bluetooth {
class BluetoothResourceManager : public IBluetoothResourceManager {
public:
    BluetoothResourceManager();
    ~BluetoothResourceManager();

    // if comm_protocol_resource_manager is not enable, instance will be nullptr
    static BluetoothResourceManager *GetInstance()
    {
#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
        static BluetoothResourceManager resourceManager;
        return &resourceManager;
#else
        return nullptr;
#endif
    }

    void RegisterObserver(std::shared_ptr<IResourceManagerObserver> observer) override;
    void DeregisterObserver(std::shared_ptr<IResourceManagerObserver> observer) override;
    int SetBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode) override;

    void SendSensingStateChanged(uint8_t eventId, const SensingInfo &info);
    void SendBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result);
    void AddAdvRecord(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
        uint8_t advHandle);
    std::shared_ptr<SensingInfo> FindAdvRecord(uint8_t advHandle);
    void SendAdvStartedEvent(int result, uint8_t advHandle);
    void SendAdvStoppedEvent(int result, uint8_t advHandle);
    void GetChannelRssiSensingData(SensingInfo &sensingInfo);
    void GetTransRateData(SensingInfo &sensingInfo);
    void RemoveAdvRecord(uint8_t advHandle);
    void UpdateScanRecord(int scanMode);
    void SendScanStartEvent(int status);
    void SendScanStopEvent(int status);
    int StartBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode);
    int StopBleScanEnhanceMode();

private:
    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothResourceManager);
    DECLARE_IMPL();
    const BthwifInterface* GetBtHwInterface();
    bool IsValidBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode);
    bool IsBleScanEnhanceModeSupported();
    void UpdateBleScanEnhanceModeTimer(int64_t timeout);
    void SendBleScanEnhanceModeEvent(const BleScanEnhanceModeImpl &enhanceMode);

    const BthwifInterface *bthwInterface_ = nullptr;
    SafeMap<uint8_t, std::shared_ptr<SensingInfo>> advMap_;
    ScanMsg scanMsg_;
    std::shared_ptr<utility::Timer> enhanceModeTimer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif