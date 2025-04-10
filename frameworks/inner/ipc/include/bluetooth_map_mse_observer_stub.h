/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_MAP_MSE_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_MAP_MSE_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_map_mse_observer.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothMapMseObserverStub : public IRemoteStub<IBluetoothMapMseObserver> {
public:
    BluetoothMapMseObserverStub();
    virtual ~BluetoothMapMseObserverStub();

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnConnectionStateChangedInner(
        BluetoothMapMseObserverStub *stub, MessageParcel &data, MessageParcel &reply);

    using BluetoothMapMseObserverFunc = int32_t (*)(
        BluetoothMapMseObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothMapMseObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(BluetoothMapMseObserverStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_MAP_MSE_OBSERVER_STUB_H