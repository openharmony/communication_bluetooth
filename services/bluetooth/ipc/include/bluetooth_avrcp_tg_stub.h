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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_TG_STUB_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_TG_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_avrcp_tg.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothAvrcpTgStub : public IRemoteStub<IBluetoothAvrcpTg> {
public:
    BluetoothAvrcpTgStub();
    virtual ~BluetoothAvrcpTgStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using AvrcpTgStubFunc = std::function<int32_t(BluetoothAvrcpTgStub *, MessageParcel &, MessageParcel &)>;
    using AvrcpTgStubFuncPerm = std::pair<AvrcpTgStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetActiveDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyPlaybackStatusChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyTrackChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyTrackReachedEndInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyTrackReachedStartInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyPlaybackPosChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyPlayerAppSettingChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyNowPlayingContentChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyAvailablePlayersChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyAddressedPlayerChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyUidChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyVolumeChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDeviceAbsoluteVolumeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceAbsVolumeAbilityInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDeviceAbsVolumeAbilityInner(MessageParcel &data, MessageParcel &reply);

    bool IsInvalidAttributesSize(int32_t attributesSize);
    bool IsInvalidDeviceStatesSize(int32_t statesSize);

private:
    static const std::map<uint32_t, AvrcpTgStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothAvrcpTgStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_TG_STUB_H