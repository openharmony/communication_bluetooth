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

#ifndef OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_CALLBACK_STUB_H
#define OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_gatt_client_callback.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattClientCallbackStub : public IRemoteStub<IBluetoothGattClientCallback> {
public:
    BluetoothGattClientCallbackStub();
    ~BluetoothGattClientCallbackStub() override;

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static ErrCode OnConnectionStateChangedInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnCharacteristicChangedInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnCharacteristicReadInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnCharacteristicWriteInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnDescriptorReadInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnDescriptorWriteInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnMtuChangedInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnServicesDiscoveredInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnConnectionParameterChangedInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnServicesChangedInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnReadRemoteRssiValueInner(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    using BluetoothHostFunc = ErrCode (*)(
        BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothHostFunc> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothGattClientCallbackStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_CALLBACK_STUB_H