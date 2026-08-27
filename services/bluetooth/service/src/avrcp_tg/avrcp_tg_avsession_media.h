/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AVRCP_TG_AVSESSION_MEDIA_H
#define AVRCP_TG_AVSESSION_MEDIA_H

#include "bluetooth_audio_manager/bluetooth_audio_manager_interface.h"
#include "bluetooth.h"
#include "base_def.h"
#include "service_util.h"
#include "gavdp/a2dp_service.h"

#include "avrcp_bip_service.h"
#include "../obex/obex_server_socket.h"
#include "../obex/obex_socket_observer.h"

namespace OHOS {
namespace bluetooth {
using namespace ::bluetooth::avrcp;

class MediaInterfaceAdapter : public MediaInterface {
public:
    MediaInterfaceAdapter() = default;
    virtual ~MediaInterfaceAdapter() = default;

    /**
     * @brief Set method.
     * Set a2dp service.
     */
    virtual void SetA2dpService(IProfileA2dp *a2dpService);
    virtual void SetAudioManager(IProfileAudioManager *audioManager);
    virtual void SendKeyEventByWearDetection(uint8_t key, KeyState state);
    virtual void SendCtrlCommandToAvSession(uint8_t key);
    virtual void PlaySilenceSound(void);
    virtual void NotifyA2dpPlayStateChange(void);
    virtual void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice);
};

class AvrcpMediaInterfaceImpl : public MediaInterfaceAdapter {
public:
    AvrcpMediaInterfaceImpl();
    ~AvrcpMediaInterfaceImpl() override = default;

    void Init();
    void CleanUp();

    /**
     * @brief callback method.
     * The key event such as pause，start Event.
     */
    void SendKeyEvent(uint8_t key, KeyState state) override;

    /**
     * @brief callback method.
     * Get song info.
     */
    void GetSongInfo(SongInfoCallback cb) override;

    /**
     * @brief callback method.
     * Get play status.
     */
    void GetPlayStatus(PlayStatusCallback cb) override;

    /**
     * @brief callback method.
     * Get now playing list.
     */
    void GetNowPlayingList(NowPlayingCallback cb) override;

    /**
     * @brief callback method.
     * Get media player list.
     */
    void GetMediaPlayerList(MediaListCallback cb) override;

    /**
     * @brief callback method.
     * Get folder intems.
     */
    void GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb) override;

    /**
     * @brief callback method.
     * Set browsed player.
     */
    void SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb) override;

    /**
     * @brief callback method.
     * Register update callback.
     */
    void RegisterUpdateCallback(MediaCallbacks *callback) override;

    /**
     * @brief callback method.
     * Unregister update Callback.
     */
    void UnregisterUpdateCallback(MediaCallbacks *callback) override;

    /**
     * @brief callback method.
     * Play item.
     */
    void PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId) override;

    /**
     * @brief callback method.
     * Set active device.
     */
    void SetActiveDevice(const STACK::RawAddress &address) override;

    /**
     * @brief callback method.
     * App Settings support.
     */
    void AppSettingsSupport(AppSettingsCallback appCb) override;

    /**
     * @brief callback method.
     * Set app settings mode such as order,repeat.
     */
    void SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode) override;

    void SetA2dpService(IProfileA2dp *a2dpService) override;

    void SetAudioManager(IProfileAudioManager *audioManager) override;

    /**
     * @brief Send avrcp command when wearing headset.
     *        No need to update media stack.
     */
    void SendKeyEventByWearDetection(uint8_t key, KeyState state) override;

    /**
     * 向avsession发送指令
     */
    void SendCtrlCommandToAvSession(uint8_t key) override;

    void PlaySilenceSound(void) override;

    void NotifyA2dpPlayStateChange(void) override;

    void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice) override;

private:
    struct impl;
    std::shared_ptr<impl> pimpl_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif
