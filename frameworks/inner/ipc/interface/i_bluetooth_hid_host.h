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
#ifndef OHOS_BLUETOOTH_IBLUETOOTHHIDHOST_H
#define OHOS_BLUETOOTH_IBLUETOOTHHIDHOST_H

#include <vector>
#include <string_ex.h>
#include <iremote_broker.h>
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_hid_host_observer.h"

using OHOS::Bluetooth::IBluetoothHidHostObserver;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

class IBluetoothHidHost : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Bluetooth.IBluetoothHidHost");

    virtual int32_t Connect(const BluetoothRawAddress &device) = 0;
    virtual int32_t Disconnect(const BluetoothRawAddress &device) = 0;
    virtual int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) = 0;
    virtual int32_t GetDevicesByStates(const std::vector<int32_t> &states,
        std::vector<BluetoothRawAddress>& result) = 0;
    virtual ErrCode RegisterObserver(const sptr<IBluetoothHidHostObserver> observer) = 0;
    virtual ErrCode DeregisterObserver(const sptr<IBluetoothHidHostObserver> observer) = 0;
    virtual ErrCode HidHostVCUnplug(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) = 0;
    virtual ErrCode HidHostSendData(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) = 0;
    virtual ErrCode HidHostSetReport(std::string &device,
        uint8_t &type, std::string &report, int& reuslt) = 0;
    virtual ErrCode HidHostGetReport(std::string &device,
        uint8_t &id, uint16_t &size, uint8_t &type, int& result) = 0;
    virtual int32_t SetConnectStrategy(const BluetoothRawAddress &device, int strategy) = 0;
    virtual int32_t GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) = 0;
};
} // Bluetooth
} // OHOS

#endif // OHOS_BLUETOOTH_IBLUETOOTHHIDHOST_H

