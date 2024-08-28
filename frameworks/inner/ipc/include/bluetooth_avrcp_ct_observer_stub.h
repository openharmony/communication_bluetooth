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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_host.h"
#include "i_bluetooth_avrcp_ct_observer.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothAvrcpCtObserverStub : public IRemoteStub<IBluetoothAvrcpCtObserver> {
public:
    BluetoothAvrcpCtObserverStub();
    virtual ~BluetoothAvrcpCtObserverStub();

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static ErrCode OnConnectionStateChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnPressButtonInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnReleaseButtonInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnSetBrowsedPlayerInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetCapabilitiesInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayerAppSettingAttributesInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayerAppSettingValuesInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayerAppSettingCurrentValueInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnSetPlayerAppSettingCurrentValueInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayerAppSettingAttributeTextInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayerAppSettingValueTextInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetElementAttributesInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetPlayStatusInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnPlayItemInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetTotalNumberOfItemsInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetItemAttributesInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnSetAbsoluteVolumeInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnPlaybackStatusChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnTrackChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnTrackReachedEndInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnTrackReachedStartInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnPlaybackPosChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnPlayerAppSettingChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnNowPlayingContentChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnAvailablePlayersChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnAddressedPlayerChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnUidChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnVolumeChangedInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetMediaPlayersInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    static ErrCode OnGetFolderItemsInner(
        BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);

    using BluetoothAvrcpCtObserverStubFunc =
        ErrCode (*)(BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, BluetoothAvrcpCtObserverStubFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(BluetoothAvrcpCtObserverStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_STUB_H