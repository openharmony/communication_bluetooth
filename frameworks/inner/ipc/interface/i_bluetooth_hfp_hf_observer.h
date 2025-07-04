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
#ifndef OHOS_BLUETOOTH_STANDARD_HFP_HF_OBSERVER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_HFP_HF_OBSERVER_INTERFACE_H

#include <string>
#include "bluetooth_raw_address.h"
#include "bluetooth_hfp_hf_call.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothHfpHfObserver : public OHOS::IRemoteBroker {
public:

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothHfpHfObserver");

    virtual void OnConnectionStateChanged(const BluetoothRawAddress &device, int state, int cause) = 0;
    virtual void OnScoStateChanged(const BluetoothRawAddress &device,  int state) = 0;
    virtual void OnCallChanged(const BluetoothRawAddress &device, const BluetoothHfpHfCall &call) = 0;
    virtual void OnSignalStrengthChanged(const BluetoothRawAddress &device,  int signal) = 0;
    virtual void OnRegistrationStatusChanged(const BluetoothRawAddress &device,  int status) = 0;
    virtual void OnRoamingStatusChanged(const BluetoothRawAddress &device,  int status) = 0;
    virtual void OnOperatorSelectionChanged(const BluetoothRawAddress &device, const std::string &name) = 0;
    virtual void OnSubscriberNumberChanged(const BluetoothRawAddress &device, const std::string &number) = 0;
    virtual void OnVoiceRecognitionStatusChanged(const BluetoothRawAddress &device, int status) = 0;
    virtual void OnInBandRingToneChanged(const BluetoothRawAddress &device, int status) = 0;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_HFP_HF_OBSERVER_INTERFACE_H