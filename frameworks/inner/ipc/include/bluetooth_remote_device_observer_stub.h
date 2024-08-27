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

#ifndef OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_bluetooth_host.h"
#include "i_bluetooth_remote_device_observer.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothRemoteDeviceObserverstub : public IRemoteStub<IBluetoothRemoteDeviceObserver> {
public:
    explicit BluetoothRemoteDeviceObserverstub();
    ~BluetoothRemoteDeviceObserverstub() override;

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnAclStateChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnPairStatusChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteNameUuidChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteNameChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteAliasChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteCodChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteBatteryChangedInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnRemoteDeviceCommonInfoReportInner(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);

    using BluetoothRemoteDeviceObserverFunc = int32_t (*)(BluetoothRemoteDeviceObserverstub *stub,
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothRemoteDeviceObserverFunc> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothRemoteDeviceObserverstub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H