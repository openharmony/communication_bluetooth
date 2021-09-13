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

#ifndef GATT_SERVER_SERVICE_H
#define GATT_SERVER_SERVICE_H

#include <list>
#include <string>
#include "gatt_data.h"
#include "interface_profile_gatt_server.h"
#include "raw_address.h"
#include "context.h"

namespace bluetooth {
class GattServerService : public IProfileGattServer {
public:
    int RegisterApplication(IGattServerCallback &callback) override;
    int DeregisterApplication(int appId) override {return 0;}
    int AddService(int appId, Service &service) override {return 0;}
    int RemoveService(int appId, const Service &service) override {return 0;}
    int ClearServices(int appId) override {return 0;}
    int NotifyClient(const GattDevice &device, Characteristic &characteristic, bool needConfirm = false) override {return 0;}
    int RespondCharacteristicRead(const GattDevice &device, Characteristic &characteristic, int ret) override {return 0;}
    int RespondCharacteristicReadByUuid(const GattDevice &device, Characteristic &characteristic, int ret) override {return 0;}
    int RespondCharacteristicWrite(const GattDevice &device, const Characteristic &characteristic, int ret) override {return 0;}
    int RespondDescriptorRead(const GattDevice &device, Descriptor &descriptor, int ret) override {return 0;}
    int RespondDescriptorWrite(const GattDevice &device, const Descriptor &descriptor, int ret) override {return 0;}
    int CancelConnection(const GattDevice &device) override {return 0;}
   
    int Connect(const RawAddress &device) override {return 0;};
    int Disconnect(const RawAddress &device) override {return 0;};
    std::list<RawAddress> GetConnectDevices() override {std::list<RawAddress> addr; return addr;};
    int GetConnectState() override {return 0;};
    int GetMaxConnectNum() override {return 0;};
    utility::Context *GetContext() override {return nullptr;};

    GattServerService();
    ~GattServerService();


private:

    IGattServerCallback* callback_;
};
}  // namespace bluetooth

#endif  // !GATT_SERVER_SERVICE_H
