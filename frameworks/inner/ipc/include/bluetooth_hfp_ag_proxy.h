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
#ifndef OHOS_BLUETOOTH_STANDARD_HFP_AG_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_HFP_AG_PROXY_H

#include "i_bluetooth_hfp_ag.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHfpAgProxy : public IRemoteProxy<IBluetoothHfpAg> {
public:
    explicit BluetoothHfpAgProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBluetoothHfpAg>(impl) {};
    ~BluetoothHfpAgProxy() {};

    int32_t GetConnectDevices(std::vector<BluetoothRawAddress> &devices) override;
    int GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices) override;
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override;
    int32_t Connect(const BluetoothRawAddress &device) override;
    int32_t Disconnect(const BluetoothRawAddress &device) override;
    int GetScoState(const BluetoothRawAddress &device) override;
    int32_t ConnectSco(uint8_t callType) override;
    int32_t DisconnectSco(uint8_t callType) override;
    bool ConnectSco() override;
    bool DisconnectSco() override;
    void PhoneStateChanged(BluetoothPhoneState &phoneState) override;
    void ClccResponse(int index, int direction, int status, int mode, bool mpty, const std::string &number,
        int type) override;
    bool OpenVoiceRecognition(const BluetoothRawAddress &device) override;
    bool CloseVoiceRecognition(const BluetoothRawAddress &device) override;
    bool SetActiveDevice(const BluetoothRawAddress &device) override;
    bool IntoMock(const BluetoothRawAddress &device, int state) override;
    bool SendNoCarrier(const BluetoothRawAddress &device) override;
    std::string GetActiveDevice() override;
    void RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) override;
    int SetConnectStrategy(const BluetoothRawAddress &device, int strategy) override;
    int GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) override;
    int IsInbandRingingEnabled(bool &isEnabled) override;
    void CallDetailsChanged(int callId, int callState) override;
    int IsVgsSupported(const BluetoothRawAddress &device, bool &isSupported) override;
    void EnableBtCallLog(bool state) override;
    void GetVirtualDeviceList(std::vector<std::string> &devices) override;
    void UpdateVirtualDevice(int32_t action, const std::string &address) override;

private:
    static inline BrokerDelegator<BluetoothHfpAgProxy> delegator_;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_HFP_AG_PROXY_H