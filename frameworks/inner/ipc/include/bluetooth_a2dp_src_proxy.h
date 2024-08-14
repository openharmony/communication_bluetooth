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

#ifndef OHOS_BLUETOOTH_STANDARD_A2DP_SRC_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_A2DP_SRC_PROXY_H

#include "iremote_proxy.h"
#include "i_bluetooth_a2dp_src.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
class BluetoothA2dpSrcProxy : public IRemoteProxy<IBluetoothA2dpSrc> {
public:
    explicit BluetoothA2dpSrcProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBluetoothA2dpSrc>(impl)
    {}
    ~BluetoothA2dpSrcProxy()
    {}

    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;
    void RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer) override;
    int GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs) override;
    int GetDeviceState(const RawAddress &device, int &state) override;
    int GetPlayingState(const RawAddress &device, int &state) override;
    int SetConnectStrategy(const RawAddress &device, int32_t strategy) override;
    int GetConnectStrategy(const RawAddress &device, int &strategy) override;
    int SetActiveSinkDevice(const RawAddress &device) override;
    RawAddress GetActiveSinkDevice() override;
    BluetoothA2dpCodecStatus GetCodecStatus(const RawAddress &device) override;
    int GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info) override;
    int SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info) override;
    void SwitchOptionalCodecs(const RawAddress &device, bool isEnable) override;
    int GetOptionalCodecsSupportState(const RawAddress &device) override;
    int StartPlaying(const RawAddress &device) override;
    int SuspendPlaying(const RawAddress &device) override;
    int StopPlaying(const RawAddress &device) override;
    int WriteFrame(const uint8_t *data, uint32_t size) override;
    int GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                          uint32_t &timeStamp) override;
    int OffloadStartPlaying(const RawAddress &device, const std::vector<int32_t> &sessionId) override;
    int OffloadStopPlaying(const RawAddress &device, const std::vector<int32_t> &sessionId) override;
    int A2dpOffloadSessionPathRequest(const RawAddress &device,
        const std::vector<BluetoothA2dpStreamInfo> &info) override;
    BluetoothA2dpOffloadCodecStatus GetOffloadCodecStatus(const RawAddress &device) override;
    int EnableAutoPlay(const RawAddress &device) override;
    int DisableAutoPlay(const RawAddress &device, const int duration) override;
    int GetAutoPlayDisabledDuration(const RawAddress &device, int &duration) override;
    void GetVirtualDeviceList(std::vector<std::string> &device) override;
    void UpdateVirtualDevice(int32_t action, const std::string &address) override;

private:
    static inline BrokerDelegator<BluetoothA2dpSrcProxy> delegator_;
    /**
     * @brief Write the serializable data
     * @param parcelableVector The communication data of IPC
     * @param reply Serializable data
     * @return true: Write the serializable data successfully; otherwise is not.
     */
    bool WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply);

    /**
     * @brief control offload play or stop action
     * @param device the remote connected sink device
     * @param sessionsId audio control streams sessions id
     * @param control audio control action request playing or stop playing.
     * @return true: control remote sink device successfully; otherwise is not.
     */
    int OffloadPlayingControl(const RawAddress &device, const std::vector<int32_t> &sessionsId, int32_t control);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_A2DP_SRC_PROXY_H