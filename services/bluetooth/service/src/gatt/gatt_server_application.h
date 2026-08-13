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

#ifndef GATT_SERVER_APPLICATION
#define GATT_SERVER_APPLICATION

#include <algorithm>
#include <functional>
#include <map>
#include <memory>

#include "bluetooth_gatt_interface.h"
#include "context.h"
#include "gatt_api.h"
#include "gatt_data.h"
#include "gatt_service_base.h"
#include "interface_profile_gatt_server.h"
#include "semaphore_utils.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
class GattServerApplication : public BluetoothGattInterface::GattServerObserver {
public:
    enum class GattElement {
        SERVICE,
        CHARACTERISTIC,
        DESCRIPTOR,
    };
    using NotifyServiceChangedFunc = std::function<void(int appId, const Service &service)>;

    static constexpr int LE_DEFAULT_ATT_MTU = 22;

    GattServerApplication(int appId, const btgatt_server_interface_t *interface, uint32_t tokenId,
        std::weak_ptr<IGattServerCallback> callback, NotifyServiceChangedFunc func);
    ~GattServerApplication() override;

    void AddService(const Service &service);
    void RemoveService(uint16_t handle);
    void ClearServices(void);
    void NotifyClient(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value);
    void NotifyClientNoCfm(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value,
        const std::shared_ptr<utility::Semaphore> &semaphore);
    void RespondCharacteristicRead(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value, int ret);
    void RespondCharacteristicWrite(const RawAddress &addr, uint16_t handle, int ret);
    void RespondDescriptorRead(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value, int ret);
    void RespondDescriptorWrite(const RawAddress &addr, uint16_t handle, int ret);
    void SetPhy(const RawAddress &address, int32_t txPhy, int32_t rxPhy, int32_t phyOptions);
    void ReadPhy(const RawAddress &address);

    /**
     * @brief: The gatt server initiates a connection to the client.
     *
     * @param addr The remote address.
     * @param isDirect Whether directed connection is required.
     * @return void
     */
    void Connect(const RawAddress &addr, bool isDirect);

    /**
     * @brief: Disconnects an established connection, or cancels a connection attempt currently in progress.
     *
     * @param addr The remote address.
     * @return void
     */
    void CancelConnection(const RawAddress &addr);

    inline int GetAppId(void) const
    {
        return serverIf_;
    }
    inline void ReportServiceChanged(const Service &service) const
    {
        WPTR_CBACK(callback_, OnServiceChanged, service);
    }
    inline bool IsValidHandle(uint16_t handle) const
    {
        for (auto iter = serviceHandles_.begin(); iter != serviceHandles_.end(); iter++) {
            if ((iter->first == handle) || (iter->second.find(handle) != iter->second.end())) {
                return true;
            }
        }
        return false;
    }

    void ConnectionCallback(
        int connId, int serverIf, int connected, const BLUEDROID::RawAddress &bda, int reason) override;
    void ServiceAddedCallback(int status, int serverIf, std::vector<btgatt_db_element_t> service) override;
    void ServiceDeletedCallback(int status, int serverIf, int serviceHandle) override;
    void RequestReadCharacteristicCallback(int connId, int transId, const BLUEDROID::RawAddress &bda, int attrHandle,
        int offset, bool isLong) override;
    void RequestReadDescriptorCallback(int connId, int transId, const BLUEDROID::RawAddress &bda, int attrHandle,
        int offset, bool isLong) override;
    void RequestWriteCharacteristicCallback(int connId, int transId, const BLUEDROID::RawAddress &bda,
        int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value) override;
    void RequestWriteDescriptorCallback(int connId, int transId, const BLUEDROID::RawAddress &bda, int attrHandle,
        int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value) override;
    void RequestExecWriteCallback(int connId, int transId, const BLUEDROID::RawAddress &bda, int execWrite) override;
    void IndicationSentCallback(int connId, int status) override;
    void MtuChangedCallback(int connId, int mtu) override;
    void ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
        uint8_t status) override;
    void PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status) override;
    void ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status) override;

private:
    struct SendResponseContext {
        int connId;
        int transId;
        uint16_t handle;
        int ret;
    };

    struct RequestReadCallbackContext {
        int connId;
        int transId;
        RawAddress addr;
        int handle;
        int offset;
        bool isLong;
    };

    struct RequestWriteCallbackContext {
        int connId;
        int transId;
        RawAddress addr;
        int handle;
        int offset;
        bool needRsp;
        bool isPrep;
        std::vector<uint8_t> value;
    };

    struct GattLongValue {
        std::vector<uint8_t> value;
        struct {
            std::vector<uint8_t> value;
            GattElement type;
            uint16_t handle;
        } prep;
    };

    struct GattConnection {
        explicit GattConnection(GattDevice device)
            : transId(0), mtu(LE_DEFAULT_ATT_MTU), notifyHandle(0), device(device), longValue() {}

        int transId;
        size_t mtu;
        uint16_t notifyHandle;
        GattDevice device;
        GattLongValue longValue;
        // Used for notify no confirm sync
        std::shared_ptr<utility::Semaphore> notifyNoCfmSem {nullptr};
    };

    inline bool IsValidConnId(int connId) const
    {
        return connIdMap_.find(connId) != connIdMap_.end();
    }
    inline std::map<int, GattConnection>::iterator GetConnectionIter(const RawAddress &addr)
    {
        return std::find_if(connIdMap_.begin(), connIdMap_.end(),
            [addr](auto &pair) { return pair.second.device.addr_ == addr; });
    }

    void BuildGattService(const std::vector<btgatt_db_element_t> &svc, Service &service);
    void ReportAddService(int ret, const Service &service);
    uint16_t PermissionToBluedroid(int perm);

    void RequestReadCallback(const RequestReadCallbackContext &ctx, GattElement type);
    void RequestWriteCallback(const RequestWriteCallbackContext &ctx, GattElement type);
    void ProcessLongValueRead(const RequestReadCallbackContext &ctx);
    void ProcessLongValueWrite(const RequestWriteCallbackContext &ctx, GattElement type);
    void SendResponse(
        int connId, int transId, uint16_t handle, int ret, const std::vector<uint8_t> &value = {});
    void SendResponse(const SendResponseContext &ctx, const std::vector<uint8_t> &value, int offset);

    void ReportWriteRequest(const GattDevice &device, uint16_t handle, const std::vector<uint8_t> &value, bool needRsp,
        GattElement type);
    bool NotifyClientInner(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value, bool needConfirm);

    int serverIf_ = 0;
    // connId <-> GattConnection
    std::map<int, GattConnection> connIdMap_ {};
    // service handle <-> characteristic and descriptor handles
    std::map<uint16_t, std::set<uint16_t>> serviceHandles_ {};

    const btgatt_server_interface_t *btIfGattServer_ = nullptr;
    std::weak_ptr<IGattServerCallback> callback_;
    // Notify other application service has changed.
    NotifyServiceChangedFunc notifyServiceChanged_ {};
    // Save caller token id for permission check in callback
    uint32_t tokenId_ = 0;
    bool isSystemHap_ = false;
    bool isNativeCaller_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // GATT_SERVER_APPLICATION
