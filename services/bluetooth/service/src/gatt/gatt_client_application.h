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

#ifndef GATT_CLIENT_APPLICATION
#define GATT_CLIENT_APPLICATION

#include <map>
#include <memory>
#include <vector>

#include "bluetooth_gatt_interface.h"
#include "context.h"
#include "gatt_api.h"
#include "gatt_data.h"
#include "gatt_service_base.h"
#include "interface_profile_gatt_client.h"
#include "semaphore_utils.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {

// Authentication Status
const int AUTH_RETRY_STATE_IDLE = 0;
const int AUTH_RETRY_STATE_NO_MITM = 1;
const int AUTH_RETRY_STATE_MITM = 2;

// Authentication requested; no person-in-the-middle protection required.
const int AUTHENTICATION_NO_MITM = 1;
// Authentication with person-in-the-middle protection requested.
const int AUTHENTICATION_MITM = 2;

const int OHOS_GATT_INSUFFICIENT_AUTHENTICATION = 0x05;
const int OHOS_GATT_INSUFFICIENT_ENCRYPTION = 0x0F;

enum GattType {
  GATT_TYPE_READ_CHARACTERISTIC = 1, /* Char read */
  GATT_TYPE_READ_DESCRIPTOR,         /* Desc read */
  GATT_TYPE_WRITE_CHARACTERISTIC,    /* Char write */
  GATT_TYPE_WRITE_DESCRIPTOR        /* Desc write */
};

class GattClientApplication : public BluetoothGattInterface::GattClientObserver,
    public std::enable_shared_from_this<GattClientApplication> {
public:
    explicit GattClientApplication(int clientIf, const btgatt_client_interface_t *interface, const GattDevice &device,
        std::weak_ptr<IGattClientCallback> callback);
    ~GattClientApplication() override;

    void Init();
    void KeepBleScan(const std::string &pkgName, int uid);
    void Connect(bool autoConnect);
    void Disconnect(void);
    void DiscoveryServices(void);
    void RequestConnectionPriority(int connPriority);
    std::vector<Service> GetServices(void);

    void ReadCharacteristic(uint16_t handle);
    void ReadCharacteristicByUuid(const Uuid &uuid, int32_t startHandle = 0x0001, int32_t endHandle = 0xFFFF);
    void WriteCharacteristic(uint16_t handle, std::vector<uint8_t> value);
    void WriteCharacteristicNoRespond(uint16_t handle, std::vector<uint8_t> value,
        std::shared_ptr<utility::Semaphore> semaphore);
    void ReadDescriptor(uint16_t handle);
    void WriteDescriptor(uint16_t handle, const std::vector<uint8_t> &value);
    void RequestExchangeMtu(int mtu);
    void ReadRemoteRssiValue(int appId);
    void SetPhy(int32_t txPhy, int32_t rxPhy, int32_t phyOptions);
    void ReadPhy(void);
    int RequestNotification(uint16_t characteristicHandle, bool enable);

    inline int GetAppId(void) const
    {
        return clientIf_;
    }

    inline RawAddress GetAddress(void) const
    {
        return addr_;
    }

    inline int GetTransport(void) const
    {
        return transport_;
    }

    inline int GetConnState(void) const
    {
        return connState_;
    }

    void ConnectCallback(
        int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda) override;
    void DisconnectCallback(
        int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda, int reason) override;
    void CancelOpenCallback(int connId, int status, int clientIf, const BLUEDROID::RawAddress &bda) override;
    void SearchCompleteCallback(int connId, int status) override;
    void RegisterForNotificationCallback(int connId, int registered, int status, uint16_t handle) override;
    void NotifyCallback(int connId, const btgatt_notify_params_t &data) override;
    void ReadCharacteristicCallback(int connId, int status, btgatt_read_params_t *pData) override;
    void WriteCharacteristicCallback(int connId, int status, uint16_t handle,
        const btgatt_rsp_params_t &rspContext) override;
    void ReadDescriptorCallback(int connId, int status, const btgatt_read_params_t &data) override;
    void WriteDescriptorCallback(int connId, int status, uint16_t handle) override;
    void ConfigureMtuCallback(int connId, int status, int mtu) override;
    void GetGattDbCallback(int connId, const btgatt_db_element_t *db, int count) override;
    void ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
        uint8_t status) override;
    void ServicesChangedCallback(int connId) override;
    void ReadRemoteRssiValueCallback(int clientIf, const BLUEDROID::RawAddress &bda, int rssi, int status) override;
    void PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status) override;
    void ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status) override;

private:
    int PermissionFromBluedroid(uint16_t perm);
    int GetBleMinConnectionInterval(int connPriority);
    int GetBleMaxConnectionInterval(int connPriority);
    int GetBleConnectionLatency(int connPriority);
    int GetBleConnectionSupervisionTimeout(int connPriority);
    void GetGattDatabase(int connId);
    bool HandleWriteCharacteristic(int ret, int connId, uint16_t handle, int authReq);
    bool CheckAuthentication(int connId, int status, uint16_t handle, int gattType);

    bool WriteCharacteristicInner(uint16_t handle, int writeType, int authReq, std::vector<uint8_t> value);

    inline bool IsConnected(void)
    {
        return connState_ == static_cast<int>(BTConnectState::CONNECTED);
    }

    int clientIf_ = 0;
    // In the current version, an application connects to only one device.
    int connState_;
    int connId_ = GATT_INVALID_CONN_ID;
    RawAddress addr_;
    int transport_ = GATT_TRANSPORT_INVALID;
    std::vector<Service> gattDb_ {};

    const btgatt_client_interface_t *btIfGattClient_ = nullptr;
    std::weak_ptr<IGattClientCallback> callback_;
    int authRetryState_ = AUTH_RETRY_STATE_IDLE;
    // Caches the value transferred in WriteDescriptor and WriteCharacteristic.
    std::vector<uint8_t> cacheValue_;

    std::shared_ptr<utility::Semaphore> writeNoRespondSem_ {nullptr};
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // GATT_CLIENT_APPLICATION
