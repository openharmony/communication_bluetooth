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

#ifndef OHOS_BLUETOOTH_STANDARD_BAS_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_BAS_PROXY_H

#include "i_bluetooth_bas.h"
#include "iremote_proxy.h"
#include "bt_def.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBasProxy : public IRemoteProxy<IBluetoothBas> {
public:
    explicit BluetoothBasProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBluetoothBas>(impl)
    {}
    ~BluetoothBasProxy()
    {}

    int32_t IsBasSupported(bool &isSupported) override;
    int32_t RegisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) override;
    int32_t DeregisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) override;
    int32_t GetBatteryLevel(const std::string &address) override;
    int32_t GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos) override;

private:
    int32_t InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothBasProxy> delegator_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BAS_PROXY_H