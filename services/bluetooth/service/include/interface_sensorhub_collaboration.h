/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACE_SENSORHUB_COLLABORATION_H
#define INTERFACE_SENSORHUB_COLLABORATION_H

#include <string>
#include <vector>

#include "bt_def.h"

namespace OHOS {
namespace bluetooth {

enum class SensorhubState : int {
    SENSORHUB_SCAN_REVMOED = 1,
};

struct SensorhubScannerInfo {
    SensorhubScannerInfo(int scannerIdIn, uint16_t scanModeIn,
        uint16_t scanIntervalIn, uint16_t scanWindowIn)
        : scannerId(scannerIdIn), scanMode(scanModeIn),
        scanInterval(scanIntervalIn), scanWindow(scanWindowIn) {}
    SensorhubScannerInfo(uint16_t scanIntervalIn, uint16_t scanWindowIn)
        : scanInterval(scanIntervalIn), scanWindow(scanWindowIn) {}
    int scannerId = 0;
    uint16_t scanMode = 0;
    uint16_t scanInterval = 0;
    uint16_t scanWindow = 0;
};

/**
 * @brief Represents power state change observer during CPU sleeping or awake.
 *
 * @since 6
 */
class ISensorhubCollaborationObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IShCollaborationObserver</b> instance.
     *
     * @since 6
     */
    virtual ~ISensorhubCollaborationObserver() = default;

    /**
     * @brief ISensorhubCollaborationObserver sensorhub state change function.
     *
     * @param state Change to the new state.
     * @since 6
     */
    virtual void OnSensorhubStateChanged(const SensorhubState state) = 0;

    /**
     * @brief ISensorhubCollaborationObserver recv sensorhub scan collboration msg function.
     *
     * @param shScannerInfo Sensorhub scanner info.
     * @since 6
     */
    virtual void OnSensorhubScanCollaborationMsgRecved(const SensorhubScannerInfo &shScannerInfo) = 0;

    /**
     * @brief ISensorhubCollaborationObserver recv sensorhub gatt collboration completed event function.
     *
     * @since 6
     */
    virtual void OnSensorhubConnCollaborationCompleted(void) = 0;
};

/**
 * @brief Represents interfaces for sensorhub collaboration.
 *
 * @since 6
 */
class BLUETOOTH_API ISensorhubCollaboration {
public:
    /**
     * @brief A destructor used to delete the <b>ISensorhubCollaboration</b> instance.
     *
     * @since 6
     */
    virtual ~ISensorhubCollaboration() = default;

    /**
     * @brief Get sensorhub collaboration singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static ISensorhubCollaboration *GetInstance();

    /**
     * @brief Register sensorhub collaboration state observer.
     *
     * @param observer Class ISensorhubCollaborationObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterShCollaborationObserver(ISensorhubCollaborationObserver &observer) const = 0;

    /**
     * @brief Deregister sensorhub collaboration observer.
     *
     * @param observer Class ISensorhubCollaborationObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterShCollaborationObserver(ISensorhubCollaborationObserver &observer) const = 0;

    virtual bool IsScanCollborationEnabled() = 0;

    /**
     * @brief Deregister sensorhub collaboration observer.
     *
     * @param type scan collaboration msg type
     * @param observer Class ISensorhubCollaborationObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual void SendScanCollaborationMsg(uint8_t type,
        const std::vector<SensorhubScannerInfo> &scannerInfo) const = 0;

    /**
    * @brief Notifyt scan result report channel.
    *
    * @param scannerId scanner id.
    * @param isToAp the switch of report.(true:report msg to low power device; false: not report;).
    */
    virtual void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) = 0;

    virtual void QueryCapability() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_SENSORHUB_COLLABORATION_H
