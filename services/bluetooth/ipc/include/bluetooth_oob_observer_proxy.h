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

#ifndef OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_PROXY_H

#include "i_bluetooth_oob_observer.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothOobObserverProxy : public IRemoteProxy<IBluetoothOobObserver> {
public:
    explicit BluetoothOobObserverProxy(const sptr<IRemoteObject> &impl) :
        IRemoteProxy<IBluetoothOobObserver>(impl) {}
    ~BluetoothOobObserverProxy()
    {}

    void OnGenerateLocalOobData(int32_t ret, const BluetoothOobData &oobData) override;
private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothOobObserverProxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_OOB_OBSERVER_PROXY_H