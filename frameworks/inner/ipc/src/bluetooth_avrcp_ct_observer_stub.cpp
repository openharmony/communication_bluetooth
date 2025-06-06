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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_avrcp_ct_observer_stub"
#endif

#include "bluetooth_avrcp_ct_observer_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "string_ex.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

const uint32_t AVRCP_COMPANY_ID_COUNT_MAX = 0xFF;
const uint32_t AVRCP_CAPABILITY_COUNT_MAX = 0xFF;
const uint32_t AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX = 0xFF;
const uint32_t AVRCP_PLAYER_APP_SETTING_VALUE_NUM_MAX = 0xFF;
const uint32_t AVRCP_ELEMENT_ATTRIBUTE_NUM_MAX = 0xFF;
const uint32_t AVRCP_FOLDER_DEPTH_MAX = 0xFF;
const uint32_t AVRCP_MEDIA_PLAYER_NUM_MAX = 0xFF;
const uint32_t AVRCP_FOLDER_ITEMS_NUM_MAX = 0xFFFF;

BluetoothAvrcpCtObserverStub::BluetoothAvrcpCtObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_CONNECTION_STATE_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PRESS_BUTTON)] =
        BluetoothAvrcpCtObserverStub::OnPressButtonInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_RELEASE_BUTTON)] =
        BluetoothAvrcpCtObserverStub::OnReleaseButtonInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_BROWSED_PLAYER)] =
        BluetoothAvrcpCtObserverStub::OnSetBrowsedPlayerInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_CAPABILITIES)] =
        BluetoothAvrcpCtObserverStub::OnGetCapabilitiesInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingAttributesInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingValuesInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingCurrentValueInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE)] =
        BluetoothAvrcpCtObserverStub::OnSetPlayerAppSettingCurrentValueInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingAttributeTextInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingValueTextInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ELEMENT_ATTRIBUTRES)] =
        BluetoothAvrcpCtObserverStub::OnGetElementAttributesInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_PLAY_STATUS)] =
        BluetoothAvrcpCtObserverStub::OnGetPlayStatusInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_ITEM)] =
        BluetoothAvrcpCtObserverStub::OnPlayItemInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS)] =
        BluetoothAvrcpCtObserverStub::OnGetTotalNumberOfItemsInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_ITEM_ATTRIBUTES)] =
        BluetoothAvrcpCtObserverStub::OnGetItemAttributesInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_SET_ABSOLUTE_VOLUME)] =
        BluetoothAvrcpCtObserverStub::OnSetAbsoluteVolumeInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAYBACK_STATUS_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnPlaybackStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnTrackChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_REACHED_END)] =
        BluetoothAvrcpCtObserverStub::OnTrackReachedEndInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_TRACK_REACHED_START)] =
        BluetoothAvrcpCtObserverStub::OnTrackReachedStartInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAYBACK_POS_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnPlaybackPosChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_PLAY_APP_SETTING_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnPlayerAppSettingChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_NOW_PLAYING_CONTENT_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnNowPlayingContentChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_AVAILABLE_PLAYER_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnAvailablePlayersChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_ADDRESSED_PLAYER_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnAddressedPlayerChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_UID_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnUidChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_VOLUME_CHANGED)] =
        BluetoothAvrcpCtObserverStub::OnVolumeChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_MEDIA_PLAYERS)] =
        BluetoothAvrcpCtObserverStub::OnGetMediaPlayersInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothAvrcpCtObserverInterfaceCode::AVRCP_CT_GET_FOLDER_ITEMS)] =
        BluetoothAvrcpCtObserverStub::OnGetFolderItemsInner;
}

BluetoothAvrcpCtObserverStub::~BluetoothAvrcpCtObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothAvrcpCtObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothAvrcpCtObserverStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d",
        code, option.GetFlags());
    if (BluetoothAvrcpCtObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }

    HILOGW("BluetoothAvrcpCtObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothAvrcpCtObserverStub::OnConnectionStateChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int state = data.ReadInt32();
    int cause = data.ReadInt32();

    stub->OnConnectionStateChanged(RawAddress(addr), state, cause);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnPressButtonInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int button = data.ReadInt32();
    int result = data.ReadInt32();

    stub->OnPressButton(RawAddress(addr), button, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnReleaseButtonInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int button = data.ReadInt32();
    int result = data.ReadInt32();

    stub->OnReleaseButton(RawAddress(addr), button, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnSetBrowsedPlayerInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int uidCounter = data.ReadInt32();
    uint32_t numberOfItems = data.ReadUint32();
    std::vector<std::string> folderNames {};
    int32_t namesSize = data.ReadInt32();
    if (static_cast<uint32_t>(namesSize) > AVRCP_FOLDER_DEPTH_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < namesSize; i++) {
        std::string name = data.ReadString();
        folderNames.push_back(name);
    }
    int result = data.ReadInt32();
    int detail = data.ReadInt32();

    stub->OnSetBrowsedPlayer(RawAddress(addr), uidCounter, numberOfItems, folderNames, result, detail);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetCapabilitiesInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint32_t> companies {};
    int32_t companiesSize = data.ReadInt32();
    if (static_cast<uint32_t>(companiesSize) > AVRCP_COMPANY_ID_COUNT_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < companiesSize; i++) {
        uint32_t company = data.ReadUint32();
        companies.push_back(company);
    }

    std::vector<uint8_t> events {};
    int32_t eventsSize = data.ReadInt32();
    if (static_cast<uint32_t>(eventsSize) > AVRCP_CAPABILITY_COUNT_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < eventsSize; i++) {
        uint8_t event = static_cast<uint8_t>(data.ReadInt32());
        events.push_back(event);
    }

    int result = data.ReadInt32();

    stub->OnGetCapabilities(RawAddress(addr), companies, events, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingAttributesInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();

    std::vector<uint8_t> attributes {};
    int32_t attributesSize = data.ReadInt32();
    if (static_cast<uint32_t>(attributesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < attributesSize; i++) {
        uint8_t attrbute = static_cast<uint8_t>(data.ReadInt32());
        attributes.push_back(attrbute);
    }

    int result = data.ReadInt32();

    stub->OnGetPlayerAppSettingAttributes(RawAddress(addr), attributes, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingValuesInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int attrbute = data.ReadInt32();

    std::vector<uint8_t> values {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_VALUE_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        uint8_t value = static_cast<uint8_t>(data.ReadInt32());
        values.push_back(value);
    }
    int result = data.ReadInt32();

    stub->OnGetPlayerAppSettingValues(RawAddress(addr), attrbute, values, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingCurrentValueInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint8_t> attrbutes {};
    int32_t attrbutesSize = data.ReadInt32();
    if (static_cast<uint32_t>(attrbutesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < attrbutesSize; i++) {
        uint8_t attrbute = static_cast<uint8_t>(data.ReadInt32());
        attrbutes.push_back(attrbute);
    }

    std::vector<uint8_t> values {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        uint8_t value = static_cast<uint8_t>(data.ReadInt32());
        values.push_back(value);
    }
    int result = data.ReadInt32();

    stub->OnGetPlayerAppSettingCurrentValue(RawAddress(addr), attrbutes, values, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnSetPlayerAppSettingCurrentValueInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int result = data.ReadInt32();

    stub->OnSetPlayerAppSettingCurrentValue(RawAddress(addr), result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingAttributeTextInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint8_t> attrbutes {};
    int32_t attrbutesSize = data.ReadInt32();
    if (static_cast<uint32_t>(attrbutesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < attrbutesSize; i++) {
        uint8_t attrbute = static_cast<uint8_t>(data.ReadInt32());
        attrbutes.push_back(attrbute);
    }

    std::vector<std::string> attributeNames {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        std::string value = data.ReadString();
        attributeNames.push_back(value);
    }

    int result = data.ReadInt32();

    stub->OnGetPlayerAppSettingAttributeText(RawAddress(addr), attrbutes, attributeNames, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayerAppSettingValueTextInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint8_t> values {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_VALUE_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        uint8_t attrbute = static_cast<uint8_t>(data.ReadInt32());
        values.push_back(attrbute);
    }

    std::vector<std::string> valueNames {};
    int32_t valueNamesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valueNamesSize) > AVRCP_PLAYER_APP_SETTING_VALUE_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valueNamesSize; i++) {
        std::string value = data.ReadString();
        valueNames.push_back(value);
    }

    int result = data.ReadInt32();

    stub->OnGetPlayerAppSettingValueText(RawAddress(addr), values, valueNames, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetElementAttributesInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint32_t> attribtues {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_ELEMENT_ATTRIBUTE_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        uint32_t attrbute = data.ReadUint32();
        attribtues.push_back(attrbute);
    }

    std::vector<std::string> valueNames {};
    int32_t valueNamesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valueNamesSize) > AVRCP_ELEMENT_ATTRIBUTE_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valueNamesSize; i++) {
        std::string value = data.ReadString();
        valueNames.push_back(value);
    }

    int result = data.ReadInt32();

    stub->OnGetElementAttributes(RawAddress(addr), attribtues, valueNames, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetPlayStatusInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint32_t songLength = data.ReadUint32();
    uint32_t songPosition = data.ReadUint32();
    uint8_t playStatus = static_cast<uint8_t>(data.ReadInt32());

    int result = data.ReadInt32();

    stub->OnGetPlayStatus(RawAddress(addr), songLength, songPosition, playStatus, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnPlayItemInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int status = data.ReadInt32();
    int result = data.ReadInt32();

    stub->OnPlayItem(RawAddress(addr), status, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetTotalNumberOfItemsInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint16_t uidCounter = static_cast<uint16_t>(data.ReadInt32());
    uint32_t numOfItems = data.ReadUint32();
    int result = data.ReadInt32();
    int detail = data.ReadInt32();

    stub->OnGetTotalNumberOfItems(RawAddress(addr), uidCounter, numOfItems, result, detail);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetItemAttributesInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint32_t> attribtues {};
    int32_t attribtuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(attribtuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < attribtuesSize; i++) {
        uint32_t attrbute = data.ReadUint32();
        attribtues.push_back(attrbute);
    }

    std::vector<std::string> valueNames {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        std::string value = data.ReadString();
        valueNames.push_back(value);
    }

    int result = data.ReadInt32();
    int detail = data.ReadInt32();

    stub->OnGetItemAttributes(RawAddress(addr), attribtues, valueNames, result, detail);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnSetAbsoluteVolumeInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint8_t volume = static_cast<uint8_t>(data.ReadInt32());
    int result = data.ReadInt32();

    stub->OnSetAbsoluteVolume(RawAddress(addr), volume, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnPlaybackStatusChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint8_t playStatus = static_cast<uint8_t>(data.ReadInt32());
    int result = data.ReadInt32();

    stub->OnPlaybackStatusChanged(RawAddress(addr), playStatus, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnTrackChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint64_t uid = data.ReadUint64();
    int result = data.ReadInt32();

    stub->OnTrackChanged(RawAddress(addr), uid, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnTrackReachedEndInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int result = data.ReadInt32();

    stub->OnTrackReachedEnd(RawAddress(addr), result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnTrackReachedStartInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int result = data.ReadInt32();

    stub->OnTrackReachedStart(RawAddress(addr), result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnPlaybackPosChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint32_t playbackPos = data.ReadUint32();
    int result = data.ReadInt32();

    stub->OnPlaybackPosChanged(RawAddress(addr), playbackPos, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnPlayerAppSettingChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::vector<uint8_t> attribtues {};
    int32_t attribtuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(attribtuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < attribtuesSize; i++) {
        int32_t attrbute = data.ReadInt32();
        attribtues.push_back(attrbute);
    }

    std::vector<uint8_t> values {};
    int32_t valuesSize = data.ReadInt32();
    if (static_cast<uint32_t>(valuesSize) > AVRCP_PLAYER_APP_SETTING_ATTRIBUTES_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < valuesSize; i++) {
        int32_t attrbute = data.ReadInt32();
        values.push_back(attrbute);
    }

    int result = data.ReadInt32();

    stub->OnPlayerAppSettingChanged(RawAddress(addr), attribtues, values, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnNowPlayingContentChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int result = data.ReadInt32();

    stub->OnNowPlayingContentChanged(RawAddress(addr), result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnAvailablePlayersChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int result = data.ReadInt32();

    stub->OnAvailablePlayersChanged(RawAddress(addr), result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnAddressedPlayerChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint16_t playerId = static_cast<uint16_t>(data.ReadInt32());
    uint16_t uidCounter = static_cast<uint16_t>(data.ReadInt32());
    int result = data.ReadInt32();
    stub->OnAddressedPlayerChanged(RawAddress(addr), playerId, uidCounter, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnUidChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint16_t uidCounter = static_cast<uint16_t>(data.ReadInt32());
    int result = data.ReadInt32();
    stub->OnUidChanged(RawAddress(addr), uidCounter, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnVolumeChangedInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint8_t volume = static_cast<uint8_t>(data.ReadInt32());
    int result = data.ReadInt32();
    stub->OnVolumeChanged(RawAddress(addr), volume, result);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetMediaPlayersInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint16_t uidCounter = static_cast<uint16_t>(data.ReadInt32());

    std::vector<BluetoothAvrcpMpItem> items {};
    int32_t itemsSize = data.ReadInt32();
    if (static_cast<uint32_t>(itemsSize) > AVRCP_MEDIA_PLAYER_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < itemsSize; i++) {
        std::shared_ptr<BluetoothAvrcpMpItem> item(data.ReadParcelable<BluetoothAvrcpMpItem>());
        if (!item) {
            return TRANSACTION_ERR;
        }
        items.push_back(*item);
    }
    int result = data.ReadInt32();
    int detail = data.ReadInt32();
    stub->OnGetMediaPlayers(RawAddress(addr), uidCounter, items, result, detail);
    return NO_ERROR;
}

ErrCode BluetoothAvrcpCtObserverStub::OnGetFolderItemsInner(
    BluetoothAvrcpCtObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    uint8_t uidCounter = static_cast<uint8_t>(data.ReadInt32());

    std::vector<BluetoothAvrcpMeItem> items {};
    int32_t itemsSize = data.ReadInt32();
    if (static_cast<uint32_t>(itemsSize) > AVRCP_FOLDER_ITEMS_NUM_MAX) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < itemsSize; i++) {
        std::shared_ptr<BluetoothAvrcpMeItem> item(data.ReadParcelable<BluetoothAvrcpMeItem>());
        if (!item) {
            return TRANSACTION_ERR;
        }
        items.push_back(*item);
    }
    int result = data.ReadInt32();
    int detail = data.ReadInt32();
    stub->OnGetFolderItems(RawAddress(addr), uidCounter, items, result, detail);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
