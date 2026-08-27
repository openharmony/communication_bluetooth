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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg_media_loader"
#endif

#include "avrcp_tg_avsession_media_loader.h"
#include "avrc_defs.h"
#include "bluetooth_audio_manager.h"
#include "common_util.h"
#include "gavdp/a2dp_service.h"
#include "refuse_play_helper.h"
#include "hfp_ag_service.h"
#include "hfp_ag_system_interface.h"
#include "bt_chr_dft_exception.h"
#include "common/bluetooth_hw_interface.h"
#include "hw_interop.h"

#include "adapter_manager.h"
#include "log.h"
#include "thread_util.h"
#include "avrcp_bip_service.h"

namespace OHOS {
namespace bluetooth {
// libbtavrcp_media.z.so dynamic loader

constexpr uint64_t DELAY_PAUSE_TIME_MS = 100;
constexpr int32_t SOCKET_CHANNEL_AUTO_STATIC_NO_SDP = -2;
const std::string BIP_SERVICE_NAME = "BIP";
const std::string EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
AvrcpTgAvsessionMediaLoader::AvrcpTgAvsessionMediaLoader()
    : loader_(DEFAULT_LIB_NAME, DEFAULT_UNLOAD_TIMER_MS, DEFAULT_LIB_CREATE_FUNC_NAME, DEFAULT_LIB_DESTROY_FUNC_NAME)
{}

void AvrcpTgAvsessionMediaLoader::LoadMediaInterfaceLib(void)
{
    loader_.OpenLib();
    auto avrcpMediaInterfaceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(avrcpMediaInterfaceImpl, "Get avrcp media interface failed");

    if (mediaCallbacks_) {
        avrcpMediaInterfaceImpl->RegisterUpdateCallback(mediaCallbacks_);
    }

    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    IProfileAudioManager *audioManager = &BluetoothAudioManager::GetInstance();
    avrcpMediaInterfaceImpl->SetAudioManager(audioManager);
    avrcpMediaInterfaceImpl->SetA2dpService(a2dpService);
    DoInBipThread([this, avrcpMediaInterfaceImpl]() {
        SetBipObserverImpl(avrcpMediaInterfaceImpl);
    });
}

bool AvrcpTgAvsessionMediaLoader::IsLibraryLoaded(void)
{
    return loader_.IsLibraryLoaded();
}

#define CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(func, ...) \
do { \
    auto avrcpMediaInterfaceImpl = loader_.GetLibInstance(); \
    CHECK_AND_RETURN_LOG(avrcpMediaInterfaceImpl, "failed"); \
    avrcpMediaInterfaceImpl->func(__VA_ARGS__); \
} while (0)

void AvrcpTgAvsessionMediaLoader::SendKeyEvent(uint8_t key, KeyState state)
{
    HILOGI("key = %{public}x. state = %{public}d.", key, state);
    if (IsNeededDropKeyEvent(key, state)) {
        return;
    }
    if (state == KeyState::PUSHED) {
        // key down do nothing
        return;
    }
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp service nullptr");
    std::string activeDevice = service->GetActiveSinkDevice().GetAddress();
    if (key == AVRC_ID_PLAY && RefusePlayHelper::GetInstance()->IsRefusePlayAfterConnect(activeDevice)) {
        HILOGI("device: %{public}s refuse auto play", GET_ENCRYPT_STR_ADDR(activeDevice));
        PlaySilenceSound();
        return;
    }
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SendKeyEvent, key, state);
}

void AvrcpTgAvsessionMediaLoader::SendKeyEventByWearDetection(uint8_t key, KeyState state)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SendKeyEventByWearDetection, key, state);
}

void AvrcpTgAvsessionMediaLoader::GetSongInfo(SongInfoCallback cb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(GetSongInfo, cb);
}

void AvrcpTgAvsessionMediaLoader::GetPlayStatus(PlayStatusCallback cb)
{
    auto avrcpMediaInterfaceImpl = loader_.GetLibInstance();
    if (!avrcpMediaInterfaceImpl) {
        HILOGE("avrcpMediaInterfaceImpl is nullptr, reply PlayStatus error");
        PlayStatus status;
        status.state = ERROR;
        cb.Run(status);
        return;
    }
    avrcpMediaInterfaceImpl->GetPlayStatus(cb);
}

void AvrcpTgAvsessionMediaLoader::GetNowPlayingList(NowPlayingCallback cb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(GetNowPlayingList, cb);
}

void AvrcpTgAvsessionMediaLoader::GetMediaPlayerList(MediaListCallback cb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(GetMediaPlayerList, cb);
}

void AvrcpTgAvsessionMediaLoader::GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(GetFolderItems, playerId, mediaId, folderCb);
}

void AvrcpTgAvsessionMediaLoader::SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SetBrowsedPlayer, playerId, browseCb);
}

void AvrcpTgAvsessionMediaLoader::RegisterUpdateCallback(MediaCallbacks *callback)
{
    // 暂时缓存协议栈注册的callback，等avsessionMedia实际加载的时候再注册
    mediaCallbacks_ = callback;

    auto avrcpMediaInterfaceImpl = loader_.GetLibInstance();
    CHECK_AND_RETURN_LOG(avrcpMediaInterfaceImpl, "cache callback");

    avrcpMediaInterfaceImpl->RegisterUpdateCallback(callback);
}

void AvrcpTgAvsessionMediaLoader::UnregisterUpdateCallback(MediaCallbacks *callback)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(UnregisterUpdateCallback, callback);
}

void AvrcpTgAvsessionMediaLoader::PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(PlayItem, playerId, nowPlaying, mediaId);
}
void AvrcpTgAvsessionMediaLoader::SetActiveDevice(const STACK::RawAddress &address)
{
    HfpAgService *hfp = HfpAgService::GetService();
    if (hfp != nullptr && hfp->IsAudioConnected(hfp->GetActiveDevice())) {
        HILOGW("ignore set active device[%{public}s] during sco", address.ToStringForLogging().c_str());
        BtChrA2dpAudioExcpEvent(address.ToString(), CHR_A2DP_ERR_TYPE_SET_A2DP_DEV, CHR_SUB_ERRCODE_CASE2);
        return;
    }
    if (RefusePlayHelper::GetInstance()->NeedIgnoreSetActive()) {
        HILOGW("ignore set active device[%{public}s] after sco", address.ToStringForLogging().c_str());
        BtChrA2dpAudioExcpEvent(address.ToString(), CHR_A2DP_ERR_TYPE_SET_A2DP_DEV, CHR_SUB_ERRCODE_CASE1);
        return;
    }
    HILOGI("avrcp tg set active device %{public}s", address.ToStringForLogging().c_str());
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SetActiveDevice, address);
}

void AvrcpTgAvsessionMediaLoader::AppSettingsSupport(AppSettingsCallback appCb)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(AppSettingsSupport, appCb);
}

void AvrcpTgAvsessionMediaLoader::SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SetAppSettingsMode, shuffleMode, repeatMode);
}

void AvrcpTgAvsessionMediaLoader::SendCtrlCommandToAvSession(uint8_t key)
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(SendCtrlCommandToAvSession, key);
}

void AvrcpTgAvsessionMediaLoader::PlaySilenceSound(void)
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp service nullptr");
    std::string activeDevice = service->GetActiveSinkDevice().GetAddress();
    int state = A2DP_NOT_PLAYING;
    service->GetPlayingState(service->GetActiveSinkDevice(), state);
    HILOGI("A2DP playing state=%{public}d", state);
    if (state == A2DP_IS_PLAYING) {
        return;
    }
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(PlaySilenceSound);
}

void AvrcpTgAvsessionMediaLoader::ProcessPushedToPlayHelper(bool isPushed)
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp service nullptr");
    std::string activeDevice = service->GetActiveSinkDevice().GetAddress();
    RefusePlayHelper::GetInstance()->ProcessPushed(activeDevice, isPushed);
}

bool AvrcpTgAvsessionMediaLoader::NeedDropPlayAfterScoEnd(bool isPushed)
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "a2dp service nullptr");
    RawAddress activeDevice = service->GetActiveSinkDevice();
    auto *bthwif = BluetoothHwInterface::GetInstance();
    CHECK_AND_RETURN_LOG_RET(bthwif != nullptr, false, "bthwif nullptr");
    if (bthwif->InteropMatch(INTEROP_NOT_DROP_PLAY, activeDevice)) {
        return false;
    }
    std::string deviceAddr = activeDevice.GetAddress();
    return RefusePlayHelper::GetInstance()->NeedDropDueScoEnd(deviceAddr, isPushed);
}

void AvrcpTgAvsessionMediaLoader::SavePauseTimeToHelper()
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp service nullptr");
    std::string activeDevice = service->GetActiveSinkDevice().GetAddress();
    RefusePlayHelper::GetInstance()->SavePauseTime(activeDevice);
}

bool AvrcpTgAvsessionMediaLoader::NeedDropPause(bool isPushed)
{
    // this logic is the same as play
    return NeedDropPlayAfterScoEnd(isPushed);
}

bool AvrcpTgAvsessionMediaLoader::IsNeededDropKeyEvent(uint8_t key, KeyState state)
{
    if (key == AVRC_ID_REWIND || key == AVRC_ID_FAST_FOR) {
        return false;
    }
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "hfp service nullptr");
    bool isPushed = (state == KeyState::PUSHED) ? true : false;
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    if (!systemInterface.IsCallIdle() || service->IsAudioConnected(service->GetActiveDevice())) {
        HILOGI("Dropped during headset in call, key:%{public}d.", key);
        ReportDropPlayPauseChr(key);
        ProcessPushedToPlayHelper(isPushed);
        return true;
    }

    if (key == AVRC_ID_PLAY && NeedDropPlayAfterScoEnd(isPushed)) {
        HILOGI("play after sco end, drop play");
        CHECK_AND_RETURN_LOG_RET(isPushed == false, true, "");
        PlaySilenceSound();
        return true;
    }
    if (key == AVRC_ID_PAUSE) {
        if (!RefusePlayHelper::GetInstance()->IsPauseAllowedByWhiteList() && NeedDropPause(isPushed)) {
            HILOGI("pause after sco end, drop pause");
            return true;
        }
        if (isPushed) {
            SavePauseTimeToHelper();
        }
    }
    if (key == AVRC_ID_STOP && NeedDropPause(isPushed)) {
        HILOGI("stop after sco end, drop stop");
        return true;
    }
    return false;
}

void AvrcpTgAvsessionMediaLoader::ReportDropPlayPauseChr(int event)
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(service != nullptr, "a2dp service nullptr");
    std::string activeDevice = service->GetActiveSinkDevice().GetAddress();
    switch (event) {
        case AVRC_ID_PLAY:
            BtChrA2dpAudioExcpEvent(activeDevice, CHR_AVRCP_PLAY_INTERCEPT, CHR_SUB_ERRCODE_CASE1);
            break;
        case AVRC_ID_PAUSE:
            BtChrA2dpAudioExcpEvent(activeDevice, CHR_AVRCP_PAUSE_INTERCEPT, CHR_SUB_ERRCODE_CASE1);
            break;
        default:
            HILOGI("not play/pause");
            break;
    }
}

void AvrcpTgAvsessionMediaLoader::NotifyA2dpPlayStateChange()
{
    CALL_FUNC_IF_AVSESSION_MEDIA_LIB_LOADED(NotifyA2dpPlayStateChange);
}

void AvrcpTgAvsessionMediaLoader::StartSocketListener(void)
{
    CHECK_AND_RETURN_LOG(!obexServerSocket_, "obexServerSocket_ is already create.");
    socketObserver_ = std::make_shared<BipSocketObserver>();
    obexServerSocket_ = std::make_shared<ObexServerSocket>(BIP_SERVICE_NAME,
        SOCKET_CHANNEL_AUTO_STATIC_NO_SDP, SOCKET_CHANNEL_AUTO_STATIC_NO_SDP, EMPTY_UUID, socketObserver_);
    int securityFlags = 0;
    obexServerSocket_->Startup(securityFlags);
    bipPsm_ = obexServerSocket_->GetL2capPsm();
}

void AvrcpTgAvsessionMediaLoader::SetBipObserverImpl(
    const std::shared_ptr<MediaInterfaceAdapter> avrcpMediaInterfaceImpl)
{
    // socketObserver_ 只在 StartSocketListener 中初始化，若 LoadMediaInterfaceLib 先于其执行
    // （Init 中 StartSocketListener 尚未在 BIP 线程排队完成），对空指针 static_pointer_cast 会崩溃。
    // 本函数运行在 BIP 线程上，StartSocketListener 幂等（obexServerSocket_ 已存在则直接返回），
    // 因此这里兜底先确保 listener 已启动，既防崩溃又保证 BIP observer 正确设置。
    if (socketObserver_ == nullptr) {
        HILOGW("socketObserver_ is nullptr, start socket listener first");
        StartSocketListener();
    }
    auto observer = std::static_pointer_cast<BipSocketObserver>(socketObserver_);
    observer->SetImpl(avrcpMediaInterfaceImpl);
}

int32_t AvrcpTgAvsessionMediaLoader::GetBipPsm(void)
{
    return bipPsm_;
}

void AvrcpTgAvsessionMediaLoader::DisableBipService(void)
{
    HILOGI("Disable bip service");
    bipPsm_ = 0;
    if (avrcpServiceInterface_ != nullptr) {
        avrcpServiceInterface_->UnregisterBipServer();
    }
    if (obexServerSocket_ != nullptr) {
        obexServerSocket_->Shutdown();
        obexServerSocket_ = nullptr;
    }
}

BipSocketObserver::BipSocketObserver()
{
    HILOGI("BipSocketObserver create");
}

void BipSocketObserver::SetImpl(const std::shared_ptr<MediaInterfaceAdapter> &avrcpMediaInterfaceImpl)
{
    impl_ = avrcpMediaInterfaceImpl;
}

void BipSocketObserver::OnConnect(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    if (socketDevice == nullptr) {
        HILOGE("socketDevice is null");
        return;
    }
    CHECK_AND_RETURN_LOG(impl_, "Get avrcp media interface failed");
    impl_->HandlerConnectEvt(socketDevice);
}
}  // namespace bluetooth
}  // namespace OHOS
