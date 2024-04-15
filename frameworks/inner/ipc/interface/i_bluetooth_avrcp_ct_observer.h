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

#ifndef OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_INTERFACE_H

#include <cstdint>
#include <vector>
#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"
#include "../parcel/bluetooth_avrcp_meItem.h"
#include "../parcel/bluetooth_avrcp_mpItem.h"
#include "../common/raw_address.h"
#include "../common/avrcp_media.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
class IBluetoothAvrcpCtObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothAvrcpCtObserver");

    virtual void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause) = 0;
    virtual void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) = 0;
    virtual void OnReleaseButton(const RawAddress &rawAddr, uint8_t button,  int result) = 0;
    virtual void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter,
        uint32_t numberOfItems, const std::vector<std::string> &folderNames, int result, int detail) = 0;
    virtual void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result) = 0;
    virtual void OnGetPlayerAppSettingAttributes(
        const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) = 0;
    virtual void OnGetPlayerAppSettingValues(const RawAddress &rawAddr,
        int attribute, const std::vector<uint8_t> &values, int result) = 0;
    virtual void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> attributes,
        const std::vector<uint8_t> &values, int result) = 0;
    virtual void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) = 0;
    virtual void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr,
        const std::vector<uint8_t> attribtues, const std::vector<std::string> &attributeName, int result) = 0;
    virtual void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr,
        const std::vector<uint8_t> &values, const std::vector<std::string> &valueName, int result) = 0;
    virtual void OnGetElementAttributes(const RawAddress &rawAddr,
        const std::vector<uint32_t> &attribtues, const std::vector<std::string> &valueName, int result) = 0;
    virtual void OnGetPlayStatus(const RawAddress &rawAddr,
        uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result) = 0;
    virtual void OnPlayItem(const RawAddress &rawAddr, int status, int result) = 0;
    virtual void OnGetTotalNumberOfItems(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        int result, int detail) = 0;
    virtual void OnGetItemAttributes(const RawAddress &rawAddr,
        const std::vector<uint32_t> &attribtues, const std::vector<std::string> &values, int result, int detail) = 0;
    virtual void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) = 0;
    virtual void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) = 0;
    virtual void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) = 0;
    virtual void OnTrackReachedEnd(const RawAddress &rawAddr, int result) = 0;
    virtual void OnTrackReachedStart(const RawAddress &rawAddr, int result) = 0;
    virtual void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) = 0;
    virtual void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result) = 0;
    virtual void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) = 0;
    virtual void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) = 0;
    virtual void OnAddressedPlayerChanged(
        const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) = 0;
    virtual void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) = 0;
    virtual void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) = 0;
    virtual void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter,
        std::vector<BluetoothAvrcpMpItem> &items, int result, int detail) = 0;
    virtual void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter,
        std::vector<BluetoothAvrcpMeItem> &items, int result, int detail) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_AVRCP_CT_OBSERVER_INTERFACE_H