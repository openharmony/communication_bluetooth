/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_BLUETOOTHHIDDEVICEOBSERVERPROXY_H
#define OHOS_BLUETOOTH_BLUETOOTHHIDDEVICEOBSERVERPROXY_H

#include <iremote_proxy.h>
#include "i_bluetooth_hid_device_observer.h"

namespace OHOS {
namespace Bluetooth {

class BluetoothHidDeviceObserverProxy : public IRemoteProxy<IBluetoothHidDeviceObserver> {
public:
    explicit BluetoothHidDeviceObserverProxy(
        const sptr<IRemoteObject>& remote) : IRemoteProxy<IBluetoothHidDeviceObserver>(remote)
    {}
    ~BluetoothHidDeviceObserverProxy()
    {}
    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int state) override;
    ErrCode OnAppStatusChanged(int state) override;
    ErrCode OnGetReport(int type, int id, uint16_t bufferSize) override;
    ErrCode OnInterruptDataReceived(int reportId, std::vector<uint8_t> data) override;
    ErrCode OnSetProtocol(int protocol) override;
    ErrCode OnSetReport(int type, int id, std::vector<uint8_t> data) override;
    ErrCode OnVirtualCableUnplug()  override;

private:
    static inline BrokerDelegator<BluetoothHidDeviceObserverProxy> delegator_;
};
} // Bluetooth
} // OHOS

#endif // OHOS_BLUETOOTH_BLUETOOTHHIDDEVICEOBSERVERPROXY_H

