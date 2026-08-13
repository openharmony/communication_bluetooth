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

#ifndef OHOS_BLUETOOTH_STANDARD_SOCKET_STUB_H
#define OHOS_BLUETOOTH_STANDARD_SOCKET_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_socket.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothSocketStub : public IRemoteStub<IBluetoothSocket> {
public:
    BluetoothSocketStub();
    ~BluetoothSocketStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using SocketStubFunc = std::function<int32_t(BluetoothSocketStub *, MessageParcel &, MessageParcel &)>;
    using SocketStubFuncPerm = std::pair<SocketStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t ListenInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterServerObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateCocConnectionParamsInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterClientObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterClientObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsAllowSocketConnectInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, SocketStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothSocketStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_SOCKET_STUB_H