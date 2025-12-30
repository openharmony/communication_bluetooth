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
#ifndef OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICEOBSERVER_H
#define OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICEOBSERVER_H
#include <string_ex.h>
#include <iremote_broker.h>
#include "bluetooth_raw_address.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "ipc_types.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothHidDeviceObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Bluetooth.IBluetoothHidDeviceObserver");

    virtual ErrCode OnAppStatusChanged(int state) = 0;
    virtual ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int state) = 0;
    virtual ErrCode OnGetReport(int type, int id, uint16_t bufferSize) = 0;
    virtual ErrCode OnInterruptDataReceived(int id, std::vector<uint8_t> data) = 0;
    virtual ErrCode OnSetProtocol(int protocol) = 0;
    virtual ErrCode OnSetReport(int type, int id, std::vector<uint8_t> data) = 0;
    virtual ErrCode OnVirtualCableUnplug() = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICEOBSERVER_H
