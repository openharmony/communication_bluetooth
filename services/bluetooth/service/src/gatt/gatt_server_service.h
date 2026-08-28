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

#ifndef GATT_SERVER_SERVICE_H
#define GATT_SERVER_SERVICE_H

#include <list>
#include <string>

#include "base_def.h"
#include "btif_gatt.h"
#include "class_creator.h"
#include "context.h"
#include "gatt_data.h"
#include "interface_profile_gatt_server.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
class GattServerService : public IProfileGattServer, public utility::Context {
public:
    int RegisterApplication(std::weak_ptr<IGattServerCallback> callback) override;
    int DeregisterApplication(int appId) override;
    int AddService(int appId, Service &service) override;
    int RemoveService(int appId, const Service &service) override;
    int ClearServices(int appId) override;
    int NotifyClient(const GattDevice &device, Characteristic &characteristic, bool needConfirm = false) override;
    int RespondCharacteristicRead(const GattDevice &device, Characteristic &characteristic, int ret) override;
    int RespondCharacteristicReadByUuid(const GattDevice &device, Characteristic &characteristic, int ret) override;
    int RespondCharacteristicWrite(const GattDevice &device, const Characteristic &characteristic, int ret) override;
    int RespondDescriptorRead(const GattDevice &device, Descriptor &descriptor, int ret) override;
    int RespondDescriptorWrite(const GattDevice &device, const Descriptor &descriptor, int ret) override;
    int GetConnectedState(const RawAddress &address) override;
    int SetPhy(int32_t appId, const RawAddress &address, int32_t txPhy, int32_t rxPhy, int32_t phyOptions) override;
    int ReadPhy(int32_t appId, const RawAddress &address) override;

    /**
     * @brief: The gatt server initiates a connection to the client.
     *
     * @param appId The gatt server application's identifier.
     * @param device The remote device.
     * @param isDirect Whether directed connection is required.
     * @return int
     */
    int Connect(int appId, const GattDevice &device, bool isDirect) override;

    /**
     * @brief: Disconnects an established connection, or cancels a connection attempt currently in progress.
     *
     * @param appId The gatt server application's identifier.
     * @param device The remote device.
     * @return int
     */
    int CancelConnection(int appId, const GattDevice &device) override;

    GattServerService();
    ~GattServerService() override;

    /// Inherited from the parent class
    void Enable(void) override;
    void Disable(void) override;
    utility::Context *GetContext(void) override;

    std::list<RawAddress> GetConnectDevices(void) override;
    int GetConnectState(void) override;
    int GetMaxConnectNum(void) override;

    // Only used in test
    void SetBtifInterface(const BtgattServerInterface *interface);

    BT_DISALLOW_COPY_AND_ASSIGN(GattServerService);

private:
    class GattServerServiceRegisterObserver;

    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;

    const BtgattServerInterface *btIfGattServer_ = nullptr;

    DECLARE_IMPL();
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // !GATT_SERVER_SERVICE_H
