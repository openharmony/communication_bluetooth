/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_RESOURCE_MANAGER_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_RESOURCE_MANAGER_OBSERVER_STUB_H

#include "i_bluetooth_resource_manager_observer.h"

#include "iremote_stub.h"
#include <map>

namespace OHOS {
namespace Bluetooth {
class BluetoothResourceManagerObserverStub : public IRemoteStub<IBluetoothResourceManagerObserver> {
public:
    BluetoothResourceManagerObserverStub();
    ~BluetoothResourceManagerObserverStub() override;

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnSensingStateChangedInner(BluetoothResourceManagerObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnBluetoothResourceDecisionInner(BluetoothResourceManagerObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);

    using BluetoothResourceManagerObserverFunc = int32_t (*)(BluetoothResourceManagerObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothResourceManagerObserverFunc> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothResourceManagerObserverStub);
};
}
}

#endif