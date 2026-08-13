/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_MAP_MSE_STUB_H
#define OHOS_BLUETOOTH_STANDARD_MAP_MSE_STUB_H
#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_host.h"
#include "i_bluetooth_map_mse.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothMapMseStub : public IRemoteStub<IBluetoothMapMse> {
public:
    BluetoothMapMseStub();
    virtual ~BluetoothMapMseStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using MapMseStubFunc = std::function<int32_t(BluetoothMapMseStub *, MessageParcel &, MessageParcel &)>;
    using MapMseStubFuncPerm = std::pair<MapMseStubFunc, std::shared_ptr<PermissionItem>>;

private:

    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetMessageAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetMessageAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, MapMseStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothMapMseStub);
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_MAP_MSE_STUB_H