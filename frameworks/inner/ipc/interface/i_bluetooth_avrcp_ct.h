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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_CT_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_CT_INTERFACE_H

#include <cstdint>
#include <vector>

#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"
#include "i_bluetooth_avrcp_ct_observer.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
class IBluetoothAvrcpCt : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothAvrcpCt");

    virtual void RegisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer) = 0;
    virtual void UnregisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer) = 0;

    virtual std::vector<RawAddress> GetConnectedDevices() = 0;
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;
    virtual int32_t Connect(const RawAddress &device) = 0;
    virtual int32_t Disconnect(const RawAddress &device) = 0;
    virtual int32_t PressButton(const RawAddress &device, int32_t button) = 0;
    virtual int32_t ReleaseButton(const RawAddress &device, int32_t button) = 0;
    virtual int32_t GetUnitInfo(const RawAddress &device) = 0;
    virtual int32_t GetSubUnitInfo(const RawAddress &device) = 0;
    virtual int32_t GetSupportedCompanies(const RawAddress &device) = 0;
    virtual int32_t GetSupportedEvents(const RawAddress &device) = 0;
    virtual int32_t GetPlayerAppSettingAttributes(const RawAddress &device) = 0;
    virtual int32_t GetPlayerAppSettingValues(const RawAddress &device, int32_t attribute) = 0;
    virtual int32_t GetPlayerAppSettingCurrentValue(const RawAddress &device,
        const std::vector<int32_t> &attributes) = 0;
    virtual int32_t SetPlayerAppSettingCurrentValue(const RawAddress &device,
        const std::vector<int32_t> &attributes, const std::vector<int32_t> &values) = 0;
    virtual int32_t GetPlayerAppSettingAttributeText(const RawAddress &device,
        const std::vector<int32_t> &attributes) = 0;
    virtual int32_t GetPlayerAppSettingValueText(const RawAddress &device, int32_t attributes,
        const std::vector<int32_t> &values) = 0;
    virtual int32_t GetElementAttributes(const RawAddress &device,
        const std::vector<int32_t> &attributes) = 0;
    virtual int32_t GetPlayStatus(const RawAddress &device) = 0;
    virtual int32_t PlayItem(const RawAddress &device, int32_t scope, int64_t uid, int32_t uidCounter) = 0;
    virtual int32_t GetFolderItems(const RawAddress &device, int32_t startItem, int32_t endItem,
        const std::vector<int32_t> &attributes) = 0;
    virtual int32_t GetTotalNumberOfItems(const RawAddress &device, int32_t scope) = 0;
    virtual int32_t SetAbsoluteVolume(const RawAddress &device, int32_t volume) = 0;
    virtual int32_t EnableNotification(const RawAddress &device,
        const std::vector<int32_t> &events, int32_t interval) = 0;
    virtual int32_t DisableNotification(const RawAddress &device,
        const std::vector<int32_t> &events) = 0;
    virtual int32_t GetItemAttributes(const RawAddress &device, int64_t uid, int32_t uidCounter,
        const std::vector<int32_t> &attributes) = 0;
    virtual int32_t SetBrowsedPlayer(const RawAddress &device, int32_t playerId) = 0;
    virtual int32_t GetMeidaPlayerList(const RawAddress &device, int32_t startItem, int32_t endItem) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_CT_INTERFACE_H