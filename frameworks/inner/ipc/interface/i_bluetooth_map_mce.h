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

#ifndef OHOS_BLUETOOTH_STANDARD_MAP_MCE_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_MAP_MCE_INTERFACE_H

#include "../parcel/bluetooth_map_mce_parameter.h"
#include "../parcel/bluetooth_raw_address.h"
#include "iremote_broker.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_map_mce_observer.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothMapMce : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothMapMce");

    virtual void RegisterObserver(const sptr<IBluetoothMapMceObserver> &observer) = 0;
    virtual void DeregisterObserver(const sptr<IBluetoothMapMceObserver> &observer) = 0;
    virtual int Connect(const BluetoothRawAddress &device) = 0;
    virtual int Disconnect(const BluetoothRawAddress &device) = 0;
    virtual int IsConnected(const BluetoothRawAddress &device) = 0;
    virtual void GetConnectDevices(std::vector<BluetoothRawAddress> &devices) = 0;
    virtual void GetDevicesByStates(const std::vector<int32_t> &statusList, std::vector<BluetoothRawAddress> &devices) = 0;
    virtual int GetConnectionState(const BluetoothRawAddress &device) = 0;
    virtual int SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy) = 0;
    virtual int GetConnectionStrategy(const BluetoothRawAddress &device) = 0;
    virtual int GetUnreadMessages(const BluetoothRawAddress &device, int32_t msgType, int32_t max) = 0;
    virtual int GetSupportedFeatures(const BluetoothRawAddress &device) = 0;
    virtual int SendMessage(const BluetoothRawAddress &device, const BluetoothIProfileSendMessageParameters &msg) = 0;
    virtual int SetNotificationFilter(const BluetoothRawAddress &device, int32_t mask) = 0;
    virtual int GetMessagesListing(const BluetoothRawAddress &device,
        const BluetoothIProfileGetMessagesListingParameters &para) = 0;
    virtual int GetMessage(const BluetoothRawAddress &device, int32_t msgType,
        const std::u16string &msgHandle, const BluetoothIProfileGetMessageParameters &para) = 0;
    virtual int UpdateInbox(const BluetoothRawAddress &device, int32_t msgType) = 0;
    virtual int GetConversationListing(const BluetoothRawAddress &device,
        const BluetoothIProfileGetConversationListingParameters &para) = 0;
    virtual int SetMessageStatus(const BluetoothRawAddress &device, int32_t msgType, const std::u16string &msgHandle,
        int32_t statusIndicator, int32_t statusValue, const std::string &extendedData) = 0;
    virtual int SetOwnerStatus(const BluetoothRawAddress &device,
        const BluetoothIProfileSetOwnerStatusParameters &para) = 0;
    virtual int GetOwnerStatus(const BluetoothRawAddress &device, const std::string &conversationId) = 0;
    virtual BluetoothIProfileMasInstanceInfoList GetMasInstanceInfo(const BluetoothRawAddress &device) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_MAP_MCE_INTERFACE_H
