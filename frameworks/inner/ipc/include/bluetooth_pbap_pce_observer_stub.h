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

#ifndef OHOS_BLUETOOTH_STANDARD_PBAP_PSE_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_PBAP_PSE_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_pbap_pce_observer.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPbapPceObserverStub : public IRemoteStub<IBluetoothPbapPceObserver> {
public:
    BluetoothPbapPceObserverStub();
    virtual ~BluetoothPbapPceObserverStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnConnectionStateChangedInner(
        BluetoothPbapPceObserverStub *stub, MessageParcel &data, MessageParcel &reply);

    static int32_t OnSyncStateChangeInner(
        BluetoothPbapPceObserverStub *stub, MessageParcel &data, MessageParcel &reply);

    using BluetoothPbapPceObserverFunc = int32_t (*)(
        BluetoothPbapPceObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothPbapPceObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(BluetoothPbapPceObserverStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_PBAP_PSE_OBSERVER_STUB_H