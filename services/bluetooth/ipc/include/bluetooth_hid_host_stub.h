/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_BLUETOOTHHIDHOSTSTUB_H
#define OHOS_BLUETOOTH_BLUETOOTHHIDHOSTSTUB_H

#include <map>
#include <iremote_stub.h>
#include "i_bluetooth_hid_host.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHidHostStub : public IRemoteStub<IBluetoothHidHost> {
public:

    BluetoothHidHostStub();
    ~BluetoothHidHostStub();

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    using HidHostStubFunc = std::function<int32_t(BluetoothHidHostStub *, MessageParcel &, MessageParcel &)>;
    using HidHostStubFuncPerm = std::pair<HidHostStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostVCUnplugInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostSendDataInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostSetReportInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostGetReportInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostSetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t HidHostGetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, HidHostStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothHidHostStub);
};
} // Bluetooth
} // OHOS

#endif // OHOS_BLUETOOTH_BLUETOOTHHIDHOSTSTUB_H

