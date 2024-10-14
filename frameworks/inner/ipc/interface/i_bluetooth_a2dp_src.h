/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_A2DP_SRC_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_A2DP_SRC_INTERFACE_H

#include <string>

#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"
#include "i_bluetooth_a2dp_src_observer.h"
#include "../parcel/bluetooth_raw_address.h"
#include "../parcel/bluetooth_a2dp_a2dpCodecStatus.h"
#include "../parcel/bluetooth_a2dp_a2dpCodecInfo.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
class IBluetoothA2dpSrc : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothA2dpSrc");

    virtual int Connect(const RawAddress &device) = 0;
    virtual int Disconnect(const RawAddress &device) = 0;
    virtual void RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) = 0;
    virtual void DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) = 0;
    virtual int GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs) = 0;
    virtual int GetDeviceState(const RawAddress &device, int &state) = 0;
    virtual int GetPlayingState(const RawAddress &device, int &state) = 0;
    virtual int SetConnectStrategy(const RawAddress &device, int32_t strategy) = 0;
    virtual int GetConnectStrategy(const RawAddress &device, int &strategy) = 0;
    virtual int SetActiveSinkDevice(const RawAddress &device) = 0;
    virtual RawAddress GetActiveSinkDevice() = 0;
    virtual BluetoothA2dpCodecStatus GetCodecStatus(const RawAddress &device) = 0;
    virtual int GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info) = 0;
    virtual int SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info) = 0;
    virtual void SwitchOptionalCodecs(const RawAddress &device, bool isEnable) = 0;
    virtual int GetOptionalCodecsSupportState(const RawAddress &device) = 0;
    virtual int StartPlaying(const RawAddress &device) = 0;
    virtual int SuspendPlaying(const RawAddress &device) = 0;
    virtual int StopPlaying(const RawAddress &device) = 0;
    virtual int WriteFrame(const uint8_t *data, uint32_t size) = 0;
    virtual int GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                                  uint32_t &timeStamp) = 0;
    virtual int OffloadStartPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId) = 0;
    virtual int OffloadStopPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId) = 0;
    virtual int A2dpOffloadSessionPathRequest(const RawAddress &device,
        const std::vector<BluetoothA2dpStreamInfo> &info) = 0;
    virtual BluetoothA2dpOffloadCodecStatus GetOffloadCodecStatus(const RawAddress &device) = 0;
    virtual int EnableAutoPlay(const RawAddress &device) = 0;
    virtual int DisableAutoPlay(const RawAddress &device, const int duration) = 0;
    virtual int GetAutoPlayDisabledDuration(const RawAddress &device, int &duration) = 0;
    virtual void GetVirtualDeviceList(std::vector<std::string> &devices) = 0;
    virtual void UpdateVirtualDevice(int32_t action, const std::string &address) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_A2DP_INTERFACE_H