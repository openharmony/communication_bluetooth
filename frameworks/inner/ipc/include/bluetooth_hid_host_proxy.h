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
#ifndef OHOS_BLUETOOTH_BLUETOOTHHIDHOSTPROXY_H
#define OHOS_BLUETOOTH_BLUETOOTHHIDHOSTPROXY_H

#include <iremote_proxy.h>
#include "i_bluetooth_hid_host.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHidHostProxy : public IRemoteProxy<IBluetoothHidHost> {
public:
    explicit BluetoothHidHostProxy(const sptr<IRemoteObject>& remote) : IRemoteProxy<IBluetoothHidHost>(remote)
    {}
    virtual ~BluetoothHidHostProxy()
    {}

    int32_t Connect(const BluetoothRawAddress &device) override;
    int32_t Disconnect(const BluetoothRawAddress &device) override;
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override;
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRawAddress>& result) override;
    ErrCode RegisterObserver(const sptr<IBluetoothHidHostObserver> observer) override;
    ErrCode DeregisterObserver(const sptr<IBluetoothHidHostObserver> observer) override;
    ErrCode HidHostVCUnplug(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) override;
    ErrCode HidHostSendData(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) override;
    ErrCode HidHostSetReport(std::string &device,
        uint8_t &type, std::string &report, int& reuslt) override;
    ErrCode HidHostGetReport(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) override;
    int32_t SetConnectStrategy(const BluetoothRawAddress &device, int strategy) override;
    int32_t GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) override;
private:
    static inline BrokerDelegator<BluetoothHidHostProxy> delegator_;
    bool WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply);
};
} // Bluetooth
} // OHOS

#endif // OHOS_BLUETOOTH_BLUETOOTHHIDHOSTPROXY_H

