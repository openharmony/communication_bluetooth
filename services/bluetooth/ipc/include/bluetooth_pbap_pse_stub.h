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

#ifndef OHOS_BLUETOOTH_STANDARD_PBAP_PSE_STUB_H
#define OHOS_BLUETOOTH_STANDARD_PBAP_PSE_STUB_H
#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_host.h"
#include "i_bluetooth_pbap_pse.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPbapPseStub : public IRemoteStub<IBluetoothPbapPse> {
public:
    BluetoothPbapPseStub();
    virtual ~BluetoothPbapPseStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using PbapPseStubFunc = std::function<int32_t(BluetoothPbapPseStub *, MessageParcel &, MessageParcel &)>;
    using PbapPseStubFuncPerm = std::pair<PbapPseStubFunc, std::shared_ptr<PermissionItem>>;

private:

    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectionStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetShareTypeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetShareTypeInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetPhoneBookAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPhoneBookAccessAuthorizationInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, PbapPseStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothPbapPseStub);
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_PBAP_PSE_STUB_H