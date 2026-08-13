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

#ifndef BLUETOOTH_GATT_INTERFACE_H
#define BLUETOOTH_GATT_INTERFACE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base_def.h"
#include "base_observer_list.h"
#include "bt_gatt.h"
#include "service_util.h"
#include "hw_bt_hwif.h"
#include "ble_defs.h"

namespace OHOS {
namespace bluetooth {
class BluetoothGattInterface final {
public:
    class ScannerObserver {
    public:
        virtual ~ScannerObserver() = default;
        virtual void ScanResultCallback(uint16_t eventType, uint8_t addrType, const BLUEDROID::RawAddress *bda,
            int8_t rssi, std::vector<uint8_t> advData) {}
        virtual void BatchScanReportCallback(int clientIf, int status, int reportFormat, int numRecords,
            std::vector<uint8_t> data) {}
        virtual void BatchScanThresholdCallback(int clientIf) {}
        virtual void TrackAdvFoundLostCallback(const TrackAdvBaseInfo &info, const BLUEDROID::RawAddress &bda,
            std::vector<uint8_t> advData) {};
        virtual void SensorhubDevInfoCallback(uint8_t *buffer, int length) {};
        virtual void SensorhubResetCallback(uint32_t state) {};
    };

    class GattServerObserver {
    public:
        virtual ~GattServerObserver() = default;
        // status is tGATT_STATUS in gatt_api.h
        virtual void RegisterServerCallback(int status, int serverIf, const BLUEDROID::bluetooth::Uuid &appUuid) {}
        // connId is a logic connection id (index + serverIf), ervey application has it's own connId
        virtual void ConnectionCallback(
            int connId, int serverIf, int connected, const BLUEDROID::RawAddress &bda, int reason) {}
        virtual void ServiceAddedCallback(int status, int serverIf, std::vector<btgatt_db_element_t> service) {}
        virtual void ServiceStoppedCallback(int status, int serverIf, int serviceHandle) {}
        virtual void ServiceDeletedCallback(int status, int serverIf, int serviceHandle) {}
        virtual void RequestReadCharacteristicCallback(int connId, int transId, const BLUEDROID::RawAddress &bda,
            int attrHandle, int offset, bool isLong) {}
        virtual void RequestReadDescriptorCallback(int connId, int transId, const BLUEDROID::RawAddress &bda,
            int attrHandle, int offset, bool isLong) {}
        virtual void RequestWriteCharacteristicCallback(int connId, int transId, const BLUEDROID::RawAddress &bda,
            int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value) {}
        virtual void RequestWriteDescriptorCallback(int connId, int transId, const BLUEDROID::RawAddress &bda,
            int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value) {}
        virtual void RequestExecWriteCallback(int connId, int transId, const BLUEDROID::RawAddress &bda, int execWrite)
            {}
        virtual void ResponseConfirmationCallback(int status, int handle) {}
        virtual void IndicationSentCallback(int connId, int status) {}
        virtual void CongestionCallback(int connId, bool congested) {}
        virtual void MtuChangedCallback(int connId, int mtu) {}
        virtual void PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status) {}
        // call every connId
        virtual void ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
            uint8_t status) {}
        virtual void ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status) {}
    };

    class GattClientObserver {
    public:
        virtual ~GattClientObserver() = default;
        // status is tGATT_STATUS in gatt_api.h
        virtual void RegisterClientCallback(int status, int clientIf, const BLUEDROID::bluetooth::Uuid &appUuid) {}
        virtual void ConnectCallback(
            int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda) {}
        virtual void DisconnectCallback(
            int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda, int reason) {}
        virtual void CancelOpenCallback(int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda) {}
        virtual void SearchCompleteCallback(int connId, int status) {}
        virtual void RegisterForNotificationCallback(int connId, int registered, int status, uint16_t handle) {}
        virtual void NotifyCallback(int connId, const btgatt_notify_params_t &data) {}
        // "data->status" is equal "status", "data->value_type" is always 0, means GATTC_READ_VALUE_TYPE_VALUE
        virtual void ReadCharacteristicCallback(int connId, int status, btgatt_read_params_t *pData) {}
        virtual void WriteCharacteristicCallback(int connId, int status, uint16_t handle,
            const btgatt_rsp_params_t &rspContext) {}
        virtual void ReadDescriptorCallback(int connId, int status, const btgatt_read_params_t &data) {}
        virtual void WriteDescriptorCallback(int connId, int status, uint16_t handle) {}
        virtual void ExecuteWriteCallback(int connId, int status) {}
        virtual void ConfigureMtuCallback(int connId, int status, int mtu) {}
        virtual void GetGattDbCallback(int connId, const btgatt_db_element_t* db, int count) {}
        virtual void ServicesRemovedCallback(int connId, uint16_t startHandle, uint16_t endHandle) {}
        virtual void ServicesAddedCallback(int connId, const btgatt_db_element_t &added, int addedCount) {}
        virtual void ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
            uint8_t status) {}
        virtual void ServicesChangedCallback(int connId) {}
        virtual void ReadRemoteRssiValueCallback(int clientIf, const BLUEDROID::RawAddress &bda,
            int rssi, int status) {}
        virtual void PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status) {}
        virtual void ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status) {}
    };

    static BluetoothGattInterface *GetInstance(void);
    bool Initialize(const btgatt_interface_t *gattInterface);

    void AddScannerObserver(ScannerObserver &observer);
    void RemoveScannerObserver(ScannerObserver &observer);

    void AddGattServerObserver(std::shared_ptr<GattServerObserver> observer);
    void RemoveGattServerObserver(std::shared_ptr<GattServerObserver> observer);
    void ClearGattServerInValidObserver();

    void AddGattClientObserver(std::shared_ptr<GattClientObserver> observer);
    void RemoveGattClientObserver(std::shared_ptr<GattClientObserver> observer);
    void ClearGattClientInValidObserver();

private:
    BluetoothGattInterface();
    ~BluetoothGattInterface() = default;

    const btgatt_interface_t *gattInterface_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothGattInterface);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_GATT_INTERFACE_H
