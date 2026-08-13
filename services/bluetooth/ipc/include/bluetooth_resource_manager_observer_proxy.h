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

#ifndef OHOS_BLUETOOTH_STANDARD_RESOURCE_MANAGER_OBSERVER_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_RESOURCE_MANAGER_OBSERVER_PROXY_H

#include <map>

#include "i_bluetooth_resource_manager_observer.h"
#include "iremote_proxy.h"
#include "bluetooth_sensing_info.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothResourceManagerObserverproxy : public IRemoteProxy<IBluetoothResourceManagerObserver> {
public:
    explicit BluetoothResourceManagerObserverproxy(const sptr<IRemoteObject> &impl);
    ~BluetoothResourceManagerObserverproxy();

    void OnSensingStateChanged(uint8_t eventId, const BluetoothSensingInfo &info) override;
    void OnBluetoothResourceDecision(uint8_t eventId, const BluetoothSensingInfo &info, uint32_t &result) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothResourceManagerObserverproxy> delegator_;
};
}
}

#endif