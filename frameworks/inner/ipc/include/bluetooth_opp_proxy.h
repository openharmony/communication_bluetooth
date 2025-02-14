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
#ifndef OHOS_BLUETOOTH_BLUETOOTHOPPPROXY_H
#define OHOS_BLUETOOTH_BLUETOOTHOPPPROXY_H

#include <iremote_proxy.h>
#include "i_bluetooth_opp.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothOppProxy : public IRemoteProxy<IBluetoothOpp> {
public:
    explicit BluetoothOppProxy(const sptr<IRemoteObject>& remote) : IRemoteProxy<IBluetoothOpp>(remote)
    {}
    virtual ~BluetoothOppProxy()
    {}

    int32_t SendFile(std::string &device,
        std::vector<BluetoothIOppTransferFileHolder> fileHolders, bool& result) override;
    int32_t SetIncomingFileConfirmation(bool accept, int fd) override;
    int32_t GetCurrentTransferInformation(BluetoothIOppTransferInformation &transferInformation) override;
    int32_t CancelTransfer(bool &result) override;
    int32_t SetLastReceivedFileUri(const std::string &uri) override;
    void RegisterObserver(const sptr<IBluetoothOppObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothOppObserver> &observer) override;
    int32_t GetDeviceState(const BluetoothRawAddress &device, int& result) override;
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRawAddress>& result) override;

private:
    static inline BrokerDelegator<BluetoothOppProxy> delegator_;
    bool WriteParcelableStringVector(const std::vector<std::string> &parcelableVector, Parcel &reply);
    bool WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply);
    bool WriteParcelableFileHolderVector(const std::vector<BluetoothIOppTransferFileHolder> &fileHolders,
        MessageParcel &reply);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_BLUETOOTHOPPPROXY_H