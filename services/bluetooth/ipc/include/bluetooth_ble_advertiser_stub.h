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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_STUB_H

#include <map>

#include "i_bluetooth_ble_advertiser.h"
#include "iremote_stub.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleAdvertiserStub : public IRemoteStub<IBluetoothBleAdvertiser> {
public:
    BluetoothBleAdvertiserStub();
    ~BluetoothBleAdvertiserStub() override;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using BleAdvertiserStubFunc =
        std::function<int32_t(BluetoothBleAdvertiserStub *, MessageParcel &, MessageParcel &)>;
    using BleAdvertiserStubFuncPerm = std::pair<BleAdvertiserStubFunc, std::shared_ptr<PermissionItem>>;
    using BleAdvertiserStubFuncMap = std::map<uint32_t, BleAdvertiserStubFuncPerm>;

private:
    int32_t RegisterBleAdvertiserCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterBleAdvertiserCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartAdvertisingInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableAdvertisingInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableAdvertisingInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopAdvertisingInner(MessageParcel &data, MessageParcel &reply);
    int32_t CloseInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAdvertiserHandleInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAdvertisingDataInner(MessageParcel &data, MessageParcel &reply);
    int32_t ChangeAdvertisingParamsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAdvPowerForRangingBusinessInner(MessageParcel &data, MessageParcel &reply);
    int32_t RestoreRangingAntSwitchInner(MessageParcel &data, MessageParcel &reply);

private:
    static const BleAdvertiserStubFuncMap memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothBleAdvertiserStub);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif