/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer AVRCP headers (avrcp/avrcp.h).
 * Keeps the global bluetooth::avrcp namespace used by the service layer
 * compilable; the real implementation is dlopened at runtime.
 */

#ifndef AVRCP_AVRCP_H
#define AVRCP_AVRCP_H

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "bt_types.h"

/*
 * Forward declarations of service layer types referenced by the media
 * interface below; they live in the global OHOS namespace. They must be
 * declared here (outside bluetooth::avrcp) so that unqualified OHOS::
 * references inside bluetooth::avrcp resolve to the global namespace
 * instead of an ambiguous nested one.
 */
namespace OHOS {
namespace bluetooth {
class IProfileA2dp;
class IProfileAudioManager;
struct ObexSocketDevice;
}  // namespace bluetooth
}  // namespace OHOS

namespace bluetooth {
namespace avrcp {

/* Plain enum (not enum class) so that PlayState values convert implicitly to
 * the integer members of PlayStatus, as the service layer relies on. */
enum PlayState : uint8_t {
    STOPPED = 0,
    PLAYING = 1,
    PAUSED = 2,
    FWD_SEEK = 3,
    REV_SEEK = 4,
    ERROR = 0xFF,
};

enum class KeyState : uint8_t {
    PUSHED = 0,
    RELEASED = 1,
};

struct PlayStatus {
    int32_t state;
    uint32_t position;
    uint32_t duration;
};

enum class Attribute : uint8_t {
    TITLE = 0x01,
    ARTIST_NAME = 0x02,
    ALBUM_NAME = 0x03,
    TRACK_NUMBER = 0x04,
    TOTAL_NUMBER_OF_TRACKS = 0x05,
    GENRE = 0x06,
    PLAYING_TIME = 0x07,
    DEFAULT_COVER_ART = 0x08,
};

class AttributeEntry {
public:
    Attribute attribute;
    std::string value;

    AttributeEntry(Attribute attr, std::string val) : attribute(attr), value(std::move(val)) {}

    bool operator<(const AttributeEntry &rhs) const
    {
        return attribute < rhs.attribute;
    }
};

class SongInfo {
public:
    std::string mediaId;
    std::string title;
    std::string artist;
    std::string album;
    std::string genre;
    int trackNum = -1;
    int numTracks = -1;
    int duration = -1;
    std::set<AttributeEntry> attributes;
};

class MediaPlayerInfo {
public:
    uint16_t id;
    std::string name;
    bool browsingSupported;
};

class ListItem {
public:
    int dummy = 0;
};

class MediaCallbacks {
public:
    virtual ~MediaCallbacks() = default;
    virtual void SendMediaUpdate(bool trackChanged, bool playStateChanged, bool queueChanged) = 0;
    virtual void SendFolderUpdate(bool availablePlayersChanged, bool addressedPlayerChanged, bool uidsChanged) {}
    virtual void SendPlayStatusUpdate() {}
    virtual void SendQueueUpdate() {}
    virtual void SendNowPlayingUpdate() {}
    virtual void SendAddressPlayerUpdate() {}
    virtual void SendAvailablePlayerUpdate() {}
    virtual void SendAddressedPlayerUpdate() {}
};

using SongInfoCallback = std::function<void(SongInfo)>;
using PlayStatusCallback = std::function<void(PlayStatus)>;
using NowPlayingCallback = std::function<void(std::string, std::vector<SongInfo>)>;
using MediaListCallback = std::function<void(uint16_t, std::vector<MediaPlayerInfo>)>;
using FolderItemsCallback = std::function<void(std::vector<ListItem>)>;
using SetBrowsedPlayerCallback = std::function<void(bool, std::string, uint16_t)>;
using AppSettingsCallback = std::function<void(uint8_t, uint8_t)>;
using VolumeChangedCb = std::function<void(const RawAddress &, int32_t)>;

class MediaInterface {
public:
    virtual ~MediaInterface() = default;
    /* Nested alias so the service layer can spell
     * AvrcpMediaInterfaceImpl::PlayStatusCallback. */
    using PlayStatusCallback = std::function<void(PlayStatus)>;
    virtual void SendKeyEvent(uint8_t key, KeyState state) {}
    virtual void GetSongInfo(SongInfoCallback cb) {}
    virtual void GetPlayStatus(PlayStatusCallback cb) {}
    virtual void GetNowPlayingList(NowPlayingCallback cb) {}
    virtual void GetMediaPlayerList(MediaListCallback cb) {}
    virtual void GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb) {}
    virtual void SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb) {}
    virtual void RegisterUpdateCallback(MediaCallbacks *callback) {}
    virtual void UnregisterUpdateCallback(MediaCallbacks *callback) {}
    virtual void PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId) {}
    virtual void SetActiveDevice(const RawAddress &address) {}
    virtual void AppSettingsSupport(AppSettingsCallback appCb) {}
    virtual void SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode) {}
    virtual void SetA2dpService(OHOS::bluetooth::IProfileA2dp *a2dpService) {}
    virtual void SetAudioManager(OHOS::bluetooth::IProfileAudioManager *audioManager) {}
    virtual void SendKeyEventByWearDetection(uint8_t key, KeyState state) {}
    virtual void SendCtrlCommandToAvSession(uint8_t key) {}
    virtual void PlaySilenceSound() {}
    virtual void NotifyA2dpPlayStateChange() {}
    virtual void HandlerConnectEvt(std::shared_ptr<OHOS::bluetooth::ObexSocketDevice> &socketDevice) {}
};

class VolumeInterface {
public:
    virtual ~VolumeInterface() = default;
    virtual void DeviceConnected(const RawAddress &bdaddr) {}
    virtual void DeviceConnected(const RawAddress &bdaddr, VolumeChangedCb cb) {}
    virtual void DeviceEmplaceSetVolume(const RawAddress &bdaddr, VolumeChangedCb cb) {}
    virtual void DeviceDisconnected(const RawAddress &bdaddr) {}
    virtual void SetVolume(int8_t volume) {}
    virtual void setVolumeOfDevice(int8_t volume, const RawAddress &bdaddr) {}
};

class ServiceInterface {
public:
    virtual ~ServiceInterface() = default;
    virtual void Init(MediaInterface *mediaInterface, VolumeInterface *volumeInterface) {}
    virtual void Cleanup() {}
    virtual void ConnectDevice(const RawAddress &bdaddr) {}
    virtual void DisconnectDevice(const RawAddress &bdaddr) {}
    virtual void SetBipClientStatus(const RawAddress &bdaddr, bool connected) {}
    virtual void RegisterBipServer(int32_t psm) {}
    virtual void UnregisterBipServer() {}
};

}  // namespace avrcp
}  // namespace bluetooth

/*
 * The service layer uses PlayState/PlayStatus unqualified inside the
 * OHOS::bluetooth namespace (bluetooth_audio_manager), bridge them here.
 */
namespace OHOS {
namespace bluetooth {
using ::bluetooth::avrcp::PlayState;
using ::bluetooth::avrcp::PlayStatus;
using ::bluetooth::avrcp::KeyState;
using ::bluetooth::avrcp::ServiceInterface;
using ::bluetooth::avrcp::MediaInterface;
using ::bluetooth::avrcp::VolumeInterface;
}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_AVRCP_H
