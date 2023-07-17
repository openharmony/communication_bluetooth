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

#ifndef OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_INTERFACE_H

#include "iremote_broker.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_gatt_client_callback.h"
#include "../parcel/bluetooth_gatt_characteristic_parcel.h"
#include "../parcel/bluetooth_gatt_descriptor_parcel.h"
#include "../parcel/bluetooth_gatt_device.h"
#include "../parcel/bluetooth_gatt_service_parcel.h"
#include "../parcel/bluetooth_raw_address.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothGattClient : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothGattClient");

    virtual int RegisterApplication(
        const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport) = 0;
    virtual int RegisterApplication(const sptr<IBluetoothGattClientCallback> &callback,
        const BluetoothRawAddress &addr, int32_t transport, int &appId) = 0;
    virtual int DeregisterApplication(int32_t appId) = 0;
    virtual int Connect(int32_t appId, bool autoConnect) = 0;
    virtual int Disconnect(int32_t appId) = 0;
    virtual int DiscoveryServices(int32_t appId) = 0;
    virtual int ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic &characteristic) = 0;
    virtual int WriteCharacteristic(
        int32_t appId, BluetoothGattCharacteristic *characteristic, bool withoutRespond) = 0;
    virtual int SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic *characteristic) = 0;
    virtual int ReadDescriptor(int32_t appId, const BluetoothGattDescriptor &descriptor) = 0;
    virtual int WriteDescriptor(int32_t appId, BluetoothGattDescriptor *descriptor) = 0;
    virtual int RequestExchangeMtu(int32_t appId, int32_t mtu) = 0;
    virtual void GetAllDevice(std::vector<BluetoothGattDevice> &device) = 0;
    virtual int RequestConnectionPriority(int32_t appId, int32_t connPriority) = 0;
    virtual int GetServices(int32_t appId, std::vector<BluetoothGattService> &service) = 0;
    virtual int RequestFastestConn(const BluetoothRawAddress &addr) = 0;
    virtual int ReadRemoteRssiValue(int32_t appId) = 0;
    virtual int RequestNotification(int32_t appId, uint16_t characterHandle, bool enable) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_INTERFACE_H