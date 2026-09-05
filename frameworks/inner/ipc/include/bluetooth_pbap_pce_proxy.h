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

#ifndef OHOS_BLUETOOTH_STANDARD_PBAP_PCE_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_PBAP_PCE_PROXY_H

#include "iremote_proxy.h"
#include "i_bluetooth_pbap_pce.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPbapPceProxy : public IRemoteProxy<IBluetoothPbapPce> {
public:
    explicit BluetoothPbapPceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBluetoothPbapPce>(impl) {};
    ~BluetoothPbapPceProxy() {};

    int32_t GetPhoneBookSyncState(const BluetoothRawAddress &device) override;
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override;
    int32_t GetDevicesByStates(const std::vector<int32_t> &states,
        std::vector<BluetoothRawAddress> &rawDevices) override;
    int32_t Connect(const BluetoothRawAddress &device) override;
    int32_t Disconnect(const BluetoothRawAddress &device) override;
    int32_t SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy) override;
    int32_t GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy) override;
    void RegisterObserver(const sptr<IBluetoothPbapPceObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothPbapPceObserver> &observer) override;

private:
    static inline BrokerDelegator<BluetoothPbapPceProxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_PBAP_PCE_PROXY_H