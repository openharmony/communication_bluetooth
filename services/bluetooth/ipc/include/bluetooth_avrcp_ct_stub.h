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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_CT_STUB_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_CT_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_bluetooth_host.h"
#include "i_bluetooth_avrcp_ct.h"
#include "bluetooth_avrcp_ct_observer_proxy.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothAvrcpCtStub : public IRemoteStub<IBluetoothAvrcpCt> {
public:
    BluetoothAvrcpCtStub();
    virtual ~BluetoothAvrcpCtStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using AvrcpCtStubFunc = std::function<int32_t(BluetoothAvrcpCtStub *, MessageParcel &, MessageParcel &)>;
    using AvrcpCtStubFuncPerm = std::pair<AvrcpCtStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t PressButtonnner(MessageParcel &data, MessageParcel &reply);
    int32_t ReleaseButtonInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetUnitInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetSubUnitInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetSupportedCompaniesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetSupportedEventsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayerAppSettingAttributesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayerAppSettingValuesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayerAppSettingCurrentValueInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetPlayerAppSettingCurrentValueInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayerAppSettingAttributeTextInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayerAppSettingValueTextInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetElementAttributesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayStatusInner(MessageParcel &data, MessageParcel &reply);
    int32_t PlayItemInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetFolderItemsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetTotalNumberOfItemsInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAbsoluteVolumeInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableNotificationInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableNotificationInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetItemAttributesInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetBrowsedPlayerInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetMeidaPlayerListInner(MessageParcel &data, MessageParcel &reply);

    bool IsInvalidAttributesSize(int32_t attributesSize);
    bool IsInvalidDeviceStatesSize(int32_t statesSize);

private:
    static const std::map<uint32_t, AvrcpCtStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothAvrcpCtStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_CT_STUB_H