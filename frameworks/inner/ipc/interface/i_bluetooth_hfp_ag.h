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
#ifndef OHOS_BLUETOOTH_STANDARD_HFP_AG_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_HFP_AG_INTERFACE_H

#include <string>
#include <vector>
#include "bluetooth_service_ipc_interface_code.h"
#include "bluetooth_phone_state.h"
#include "i_bluetooth_hfp_ag_observer.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothHfpAg : public OHOS::IRemoteBroker {
public:

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothHfpAg");

    virtual int32_t GetConnectDevices(std::vector<BluetoothRawAddress> &devices) = 0;
    virtual int GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices) = 0;
    virtual int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) = 0;
    virtual int32_t Connect(const BluetoothRawAddress &device) = 0;
    virtual int32_t Disconnect(const BluetoothRawAddress &device) = 0;
    virtual int GetScoState(const BluetoothRawAddress &device) = 0;
    virtual int32_t ConnectSco(uint8_t callType) = 0;
    virtual int32_t DisconnectSco(uint8_t callType) = 0;
    virtual bool ConnectSco() = 0;
    virtual bool DisconnectSco() = 0;
    virtual void PhoneStateChanged(BluetoothPhoneState &phoneState) = 0;
    virtual void ClccResponse(int index, int direction, int status, int mode, bool mpty, const std::string &number,
        int type) = 0;
    virtual bool OpenVoiceRecognition(const BluetoothRawAddress &device) = 0;
    virtual bool CloseVoiceRecognition(const BluetoothRawAddress &device) = 0;
    virtual bool SetActiveDevice(const BluetoothRawAddress &device) = 0;
    virtual std::string GetActiveDevice() = 0;
    virtual bool IntoMock(const BluetoothRawAddress &device, int state) = 0;
    virtual bool SendNoCarrier(const BluetoothRawAddress &device) = 0;
    virtual void RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) = 0;
    virtual void DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) = 0;
    virtual int SetConnectStrategy(const BluetoothRawAddress &device, int strategy) = 0;
    virtual int GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) = 0;
    virtual int IsInbandRingingEnabled(bool &isEnabled) = 0;
    virtual void CallDetailsChanged(int callId, int callState) = 0;
    virtual int IsVgsSupported(const BluetoothRawAddress &device, bool &isSupported) = 0;
    virtual void EnableBtCallLog(bool state) = 0;
    virtual void GetVirtualDeviceList(std::vector<std::string> &devices) = 0;
    virtual void UpdateVirtualDevice(int32_t action, const std::string &address) = 0;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_HFP_AG_INTERFACE_H