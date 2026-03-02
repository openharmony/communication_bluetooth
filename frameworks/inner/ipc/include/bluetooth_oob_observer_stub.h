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

#ifndef OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_STUB_H

#include <map>
#include "i_bluetooth_oob_observer.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothOobObserverStub : public IRemoteStub<IBluetoothOobObserver> {
public:
    BluetoothOobObserverStub();
    ~BluetoothOobObserverStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static int32_t OnGenerateLocalOobDataInner(BluetoothOobObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    using BluetoothObserverOobFunc = int32_t (*)(BluetoothOobObserverStub *stub,
        MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothObserverOobFunc> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothOobObserverStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_STUB_H