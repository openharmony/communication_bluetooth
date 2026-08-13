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

#ifndef OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_STUB_H
#define OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_gatt_client.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattClientStub : public IRemoteStub<IBluetoothGattClient> {
public:
    BluetoothGattClientStub();
    virtual ~BluetoothGattClientStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using GattClientStubFunc = std::function<int32_t(BluetoothGattClientStub *, MessageParcel &, MessageParcel &)>;
    using GattClientStubFuncPerm = std::pair<GattClientStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t RegisterApplicationInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterApplicationInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DiscoveryServicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadCharacteristicInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadCharacteristicByUuidInner(MessageParcel &data, MessageParcel &reply);
    int32_t WriteCharacteristicInner(MessageParcel &data, MessageParcel &reply);
    int32_t SignedWriteCharacteristicInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadDescriptorInner(MessageParcel &data, MessageParcel &reply);
    int32_t WriteDescriptorInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestExchangeMtuInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAllDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestConnectionPriorityInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetServicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestFastestConnInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadRemoteRssiValueInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestNotificationInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetPhyInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadPhyInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, GattClientStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothGattClientStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_STUB_H