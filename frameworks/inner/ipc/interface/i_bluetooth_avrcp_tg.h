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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_TG_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_TG_INTERFACE_H

#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"
#include "i_bluetooth_avrcp_tg_observer.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothAvrcpTg : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothAvrcpTg");

    virtual void RegisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer) = 0;
    virtual void UnregisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer) = 0;
    virtual void SetActiveDevice(const BluetoothRawAddress &addr) = 0;
    virtual int32_t Connect(const BluetoothRawAddress &addr) = 0;
    virtual int32_t Disconnect(const BluetoothRawAddress &addr) = 0;
    virtual std::vector<BluetoothRawAddress> GetConnectedDevices() = 0;
    virtual std::vector<BluetoothRawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;
    virtual int32_t GetDeviceState(const BluetoothRawAddress &addr) = 0;
    virtual void NotifyPlaybackStatusChanged(int32_t playStatus, int32_t playbackPos) = 0;
    virtual void NotifyTrackChanged(int64_t uid, int32_t playbackPos) = 0;
    virtual void NotifyTrackReachedEnd(int32_t playbackPos) = 0;
    virtual void NotifyTrackReachedStart(int32_t playbackPos) = 0;
    virtual void NotifyPlaybackPosChanged(int32_t playbackPos) = 0;
    virtual void NotifyPlayerAppSettingChanged(const std::vector<int32_t> &attributes,
                                               const std::vector<int32_t> &values) = 0;
    virtual void NotifyNowPlayingContentChanged() = 0;
    virtual void NotifyAvailablePlayersChanged() = 0;
    virtual void NotifyAddressedPlayerChanged(int32_t playerId, int32_t uidCounter) = 0;
    virtual void NotifyUidChanged(int32_t uidCounter) = 0;
    virtual void NotifyVolumeChanged(int32_t volume) = 0;
    virtual int32_t SetDeviceAbsoluteVolume(const BluetoothRawAddress &addr, int32_t volumeLevel) = 0;
    virtual int32_t SetDeviceAbsVolumeAbility(const BluetoothRawAddress &addr, int32_t ability) = 0;
    virtual int32_t GetDeviceAbsVolumeAbility(const BluetoothRawAddress &addr, int32_t &ability) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_TG_INTERFACE_H