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

#ifndef GATT_CLIENT_SERVICE_H
#define GATT_CLIENT_SERVICE_H

#include <cstdint>
#include <list>
#include <memory>
#include <vector>
#include "bt_uuid.h"
#include "gatt_data.h"
#include "interface_profile_gatt_client.h"
#include "raw_address.h"
#include "context.h"

namespace bluetooth {
class GattClientService : public IProfileGattClient {
public:
    int RegisterApplication(IGattClientCallback &callback, const RawAddress &addr, uint8_t transport) override;
    int RegisterSharedApplication(IGattClientCallback &callback, const RawAddress &addr, uint8_t transport) override {return 0;}
    int DeregisterApplication(int appId) override {return 0;}
    int Connect(int appId, bool autoConnect = false) override {return 0;}
    int Disconnect(int appId) override {return 0;}
    int DiscoveryServices(int appId) override {return 0;}
    int ReadCharacteristic(int appId, const Characteristic &characteristic) override {return 0;}
    int ReadCharacteristicByUuid(int appId, const Uuid &uuid) override {return 0;}
    int WriteCharacteristic(int appId, Characteristic &characteristic, bool withoutRespond = false) override {return 0;}
    int SignedWriteCharacteristic(int appId, Characteristic &characteristic) override {return 0;}
    int ReadDescriptor(int appId, const Descriptor &descriptor) override {return 0;}
    int WriteDescriptor(int appId, Descriptor &descriptor) override {return 0;}
    int RequestExchangeMtu(int appId, int mtu) override {return 0;}
    std::vector<GattDevice> GetAllDevice() override {std::vector<GattDevice> vec; return vec;}
    int RequestConnectionPriority(int appId, int connPriority) override {return 0;}
    std::vector<Service> GetServices(int appId) override {std::vector<Service> vec; return vec;}

    int Connect(const RawAddress &device) override {return 0;}
    //MOCK_METHOD1(Connect, int(RawAddress&));

    int Disconnect(const RawAddress &device) override {return 0;};
    std::list<RawAddress> GetConnectDevices() override {std::list<RawAddress> addr; return addr;};
    int GetConnectState() override {return 0;};
    int GetMaxConnectNum() override {return 0;};
    utility::Context *GetContext() override {return nullptr;};

    GattClientService();
    ~GattClientService();

    void OnWriteCharacteristicValueEvent(int reqId, uint16_t connectHandle, uint16_t handle, int ret);
    void OnConnectionStateChanged(int state, int newState);
private:

    IGattClientCallback* callback_;
};
}  // namespace bluetooth

#endif  // !GATT_CLIENT_SERVICE_H
