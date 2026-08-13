/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_BLUETOOTHHIDDEVICESTUB_H
#define OHOS_BLUETOOTH_BLUETOOTHHIDDEVICESTUB_H

#include <map>
#include <iremote_stub.h>
#include "i_bluetooth_hid_device.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHidDeviceStub : public IRemoteStub<IBluetoothHidDevice> {
public:

    BluetoothHidDeviceStub();
    ~BluetoothHidDeviceStub();

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    using HidDeviceStubFunc = std::function<int32_t(BluetoothHidDeviceStub *, MessageParcel &, MessageParcel &)>;
    using HidDeviceStubFuncPerm = std::pair<HidDeviceStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectionStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnRegisterAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendReportInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReplyReportInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReportErrorInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidDeviceSetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidDeviceGetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);

    static const std::map<uint32_t, HidDeviceStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothHidDeviceStub);
};
} // Bluetooth
} // OHOS

#endif // OHOS_BLUETOOTH_BLUETOOTHHIDDEVICESTUB_H

