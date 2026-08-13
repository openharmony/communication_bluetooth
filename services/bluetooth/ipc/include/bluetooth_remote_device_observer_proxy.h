/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_PROXY_H

#include <map>

#include "i_bluetooth_host.h"
#include "i_bluetooth_remote_device_observer.h"
#include "iremote_proxy.h"
#include "bluetooth_battery_info.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothRemoteDeviceObserverproxy : public IRemoteProxy<IBluetoothRemoteDeviceObserver> {
public:
    explicit BluetoothRemoteDeviceObserverproxy(const sptr<IRemoteObject> &impl);
    ~BluetoothRemoteDeviceObserverproxy();

    void OnAclStateChanged(const BluetoothRawAddress &device, int32_t state, uint32_t reason) override;
    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device,
        int32_t status, int32_t cause, const std::string &causeMessage) override;
    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) override;
    void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) override;
    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) override;
    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) override;
    void OnRemoteBatteryChanged(const BluetoothRawAddress &device, const BluetoothBatteryInfo &batteryInfo) override;
    void OnRemoteDeviceCommonInfoReport(const BluetoothRawAddress &device, const std::vector<uint8_t> &value) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothRemoteDeviceObserverproxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_PROXY_H