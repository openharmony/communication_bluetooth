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
#ifndef OHOS_BLUETOOTH_BLUETOOTHOPPOBSERVERSTUB_H
#define OHOS_BLUETOOTH_BLUETOOTHOPPOBSERVERSTUB_H

#include <map>

#include <iremote_stub.h>
#include "i_bluetooth_opp_observer.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothOppObserverStub : public IRemoteStub<IBluetoothOppObserver> {
public:
    BluetoothOppObserverStub();
    virtual ~BluetoothOppObserverStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnReceiveIncomingFileChangedInner(BluetoothOppObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    static int32_t OnTransferStateChangedInner(BluetoothOppObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);

    using BluetoothOppObserverFunc = int32_t (*)(BluetoothOppObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothOppObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(BluetoothOppObserverStub);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_BLUETOOTHOPPOBSERVERSTUB_H