/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_BLUETOOTHBASSTUB_H
#define OHOS_BLUETOOTH_BLUETOOTHBASSTUB_H

#include <map>
#include <iremote_stub.h>
#include "i_bluetooth_bas.h"
#include "permission_item.h"
#include "bluetooth_service_ipc_interface_code.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBasStub : public IRemoteStub<IBluetoothBas> {
public:
    BluetoothBasStub();
    ~BluetoothBasStub();

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    using BasStubFunc = std::function<int32_t(BluetoothBasStub*, MessageParcel&, MessageParcel&)>;
    using BasStubFuncPerm = std::pair<BasStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t RegisterDeviceBatteryObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterDeviceBatteryObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBatteryLevelInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedDeviceBatteryInfosInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, BasStubFuncPerm> memberFuncMap_;
    static bool IsValidAddress(const std::string &addr);
    DISALLOW_COPY_AND_MOVE(BluetoothBasStub);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_BLUETOOTHBASSTUB_H