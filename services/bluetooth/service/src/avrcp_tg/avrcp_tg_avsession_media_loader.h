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

#ifndef AVRCP_TG_AVSESSION_MEDIA_LOADER_H
#define AVRCP_TG_AVSESSION_MEDIA_LOADER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "avrcp_tg_avsession_media.h"
#include "btcommon/dynamic_library_loader.h"
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {
class AvrcpTgAvsessionMediaLoader : public MediaInterface {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtavrcp_media.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateAvrcpMediaInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyAvrcpMediaInterface";
    // Unload the dynamic library after 5min (300000ms) Calling the UnloadMediaInterfaceLib interface.
    const uint32_t DEFAULT_UNLOAD_TIMER_MS = 300000;  // 5min

    AvrcpTgAvsessionMediaLoader();
    ~AvrcpTgAvsessionMediaLoader() = default;

    void SendKeyEvent(uint8_t key, KeyState state) override;
    void GetSongInfo(SongInfoCallback cb) override;
    void GetPlayStatus(PlayStatusCallback cb) override;
    void GetNowPlayingList(NowPlayingCallback cb) override;
    void GetMediaPlayerList(MediaListCallback cb) override;
    void GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb) override;
    void SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb) override;
    void RegisterUpdateCallback(MediaCallbacks *callback) override;
    void UnregisterUpdateCallback(MediaCallbacks *callback) override;
    void PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId) override;
    void SetActiveDevice(const STACK::RawAddress &address) override;
    void AppSettingsSupport(AppSettingsCallback appCb) override;
    void SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode) override;

    /**
     * @brief Load the dynamic library, may cost 20ms ~ 50ms。
     *
     * @return void
     */
    void LoadMediaInterfaceLib(void);

    /**
     * @brief When the headset is worn or taken off, the avrcp command is sent without updating the sound stack.
     *
     * @param key command key.
     * @param state pushed or released.
     * @return void
     */
    void SendKeyEventByWearDetection(uint8_t key, KeyState state) override;

    void SendCtrlCommandToAvSession(uint8_t key) override;

    void NotifyA2dpPlayStateChange(void) override;

    bool IsLibraryLoaded(void);

    void StartSocketListener(void);

    void SetBipObserverImpl(const std::shared_ptr<MediaInterfaceAdapter> avrcpMediaInterfaceImpl);

    int32_t GetBipPsm(void);

    void DisableBipService(void);
private:
    void PlaySilenceSound(void) override;
    void ProcessPushedToPlayHelper(bool isPushed);
    bool NeedDropPlayAfterScoEnd(bool isPushed);
    void SavePauseTimeToHelper();
    bool NeedDropPause(bool isPushed);
    bool IsNeededDropKeyEvent(uint8_t key, KeyState state);
    void ReportDropPlayPauseChr(int event);

private:
    MediaCallbacks *mediaCallbacks_ = nullptr;
    CxxDynamicLibraryLoader<MediaInterfaceAdapter> loader_;
    int32_t bipPsm_ = 0;
    ServiceInterface *avrcpServiceInterface_ {nullptr};
    std::shared_ptr<IObexSocketObserver> socketObserver_ ;
    std::shared_ptr<ObexServerSocket> obexServerSocket_;
};

class BipSocketObserver : public IObexSocketObserver {
public:
    explicit BipSocketObserver();
    void SetImpl(const std::shared_ptr<MediaInterfaceAdapter> &avrcpMediaInterfaceImpl);
    void OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice) override;
private:
    std::shared_ptr<MediaInterfaceAdapter> impl_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_TG_AVSESSION_MEDIA_LOADER_H
