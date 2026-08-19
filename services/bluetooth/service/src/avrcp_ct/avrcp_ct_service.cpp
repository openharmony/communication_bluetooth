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

#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_ct"
#endif

#include <limits>
#include "avrcp_ct_service.h"
#include "adapter_config.h"
#include "class_creator.h"
#include "profile_service_manager.h"
#include "log.h"
#include "avrcp_ct_define.h"
#include "adapter_manager.h"
#include "thread_util.h"
#include "service_util.h"
#include "a2dp_snk_service.h"

#include <algorithm>

namespace OHOS {
namespace bluetooth {

AvrcpCtService *AvrcpCtService::GetService()
{
    auto srevManager = IProfileManager::GetInstance();
    return static_cast<AvrcpCtService *>(srevManager->GetProfileService(PROFILE_NAME_AVRCP_CT));
}

AvrcpCtService::AvrcpCtService() : utility::Context(PROFILE_NAME_AVRCP_CT, "1.6.2")
{}

AvrcpCtService::~AvrcpCtService()
{}

utility::Context *AvrcpCtService::GetContext()
{
    return this;
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpCtService::RegisterObserver(IObserver *observer)
{}

void AvrcpCtService::UnregisterObserver(void)
{}

static void PassThroughResCallback(const BLUEDROID::RawAddress &bdAddr, int id, int keyState)
{
    HILOGD("passthrough response received as: key:%{public}d, state:%{public}d, address:%{public}s",
        id, keyState, bdAddr.ToLogString().c_str());
}

static void GroupNavigationResCallback(int id, int keyState)
{
    HILOGD("navigation response received as: key:%{public}d, state:%{public}d", id, keyState);
}

static void ConnectionStateCallback(bool rcConnect, bool btConnect, const BLUEDROID::RawAddress &bdAddr)
{
    HILOGI("callback from stack: rcConnect:%{public}d, btConnect:%{public}d", rcConnect, btConnect);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_CONNECT_EVT);
    event.dev_ = rawAddr.GetAddress();
    event.connectFlag_.remoteCtlConnected = rcConnect;
    event.connectFlag_.browsingConnected = btConnect;
    AvrcpCtService::GetService()->PostEvent(event);
}

static void GetRcFeaturesCallback(const BLUEDROID::RawAddress &bdAddr, int features)
{
    HILOGI("do nothing!");
}

static void SetAbsVolCmdCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t absVol, uint8_t label)
{
    HILOGI("SetAbsVolCmdCallback: absVol=%{public}d, label=%{public}d", absVol, label);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_SET_ABS_VOL_CMD, absVol);
    event.dev_ = rawAddr.GetAddress();
    event.eventThreeParameter_ = label;
    AvrcpCtService::GetService()->PostEvent(event);
}

static void RegisterNotificationAbsVolCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t label)
{
    HILOGI("RegisterNotificationAbsVolCallback: label=%{public}d", label);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_REGISTER_ABS_VOL_NOTIFICATION, label);
    event.dev_ = rawAddr.GetAddress();
    AvrcpCtService::GetService()->PostEvent(event);
}

static void SetPlayerApplicationSettingRspCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t accepted)
{
    HILOGI("do nothing!");
}

static void PlayerApplicationSettingCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t numAttr,
    btrc_player_app_attr_t *appAttrs, uint8_t numExtAttr, btrc_player_app_ext_attr_t *extAttrs)
{
    HILOGI("player_application_setting_callback");
}

static void PlayerApplicationSettingChangedCallback(
    const BLUEDROID::RawAddress &bdAddr, const btrc_player_settings_t &vals)
{
    HILOGI("PlayerApplicationSettingChangedCallback: num_attr=%{public}d", vals.num_attr);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    auto service = AvrcpCtService::GetService();
    if (!service) {
        return;
    }
    // 遍历变更的属性，对 REPEAT/SHUFFLE 分别投递消息（对齐双框架 onPlayerAppSettingChanged）
    for (uint8_t i = 0; i < vals.num_attr && i < BTRC_MAX_APP_SETTINGS; i++) {
        uint8_t attrId = vals.attr_ids[i];
        uint8_t attrVal = vals.attr_values[i];
        HILOGI("  attr[%{public}d]: id=%{public}d, val=%{public}d", i, attrId, attrVal);
        if (attrId != AVRCP_REPEAT_STATUS && attrId != AVRCP_SHUFFLE_STATUS) {
            continue;
        }
        AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_CURRENT_APPLICATION_SETTINGS);
        event.dev_ = rawAddr.GetAddress();
        event.appSettingAttr_ = attrId;
        event.appSettingVal_ = attrVal;
        service->PostEvent(event);
    }
}

static void TrackChangedCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t numAttr, btrc_element_attr_val_t *pAttrs)
{
    HILOGI("TrackChangedCallback: numAttr=%{public}d", numAttr);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_TRACK_CHANGED, numAttr);
    event.dev_ = rawAddr.GetAddress();

    auto track = std::make_shared<AvrcpCtMediaItem>();
    if (pAttrs != nullptr && numAttr > 0) {
        std::vector<uint32_t> attrIds;
        std::vector<std::string> attrVals;
        for (int i = 0; i < numAttr; i++) {
            HILOGI("TrackChangedCallback i %{public}d, attr_id %{public}d, textPtr %{public}s", i, pAttrs[i].attr_id, pAttrs[i].text);
            attrIds.push_back(static_cast<uint32_t>(pAttrs[i].attr_id));
            const char* textPtr = reinterpret_cast<const char*>(pAttrs[i].text);
            attrVals.push_back(textPtr != nullptr ? std::string(textPtr) : std::string());
        }
        track->CreateAvrcpAttribute(attrIds, attrVals);
    }
    track->device_ = rawAddr;

    auto service = AvrcpCtService::GetService();
    if (service && track->HasCoverArt() && !track->GetCoverArtHandle().empty()) {
        std::string uuid = service->GetUuidForHandle(rawAddr.GetAddress(), track->GetCoverArtHandle());
        if (!uuid.empty()) {
            track->SetCoverArtUuid(uuid);
        }
    }

    event.track_ = track;
    if (service) {
        service->PostEvent(event);
    }
}

static void PlayPositionChangedCallback(const BLUEDROID::RawAddress &bdAddr, uint32_t songLen, uint32_t songPos)
{
    HILOGI("PlayPositionChangedCallback: songPos=%{public}u", songPos);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_PLAY_POS_CHANGED, songPos);
    event.dev_ = rawAddr.GetAddress();
    AvrcpCtService::GetService()->PostEvent(event);
}

static void PlayStatusChangedCallback(const BLUEDROID::RawAddress &bdAddr, btrc_play_status_t playStatus)
{
    HILOGI("PlayStatusChangedCallback: playStatus=%{public}d", playStatus);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_PLAY_STATUS_CHANGED, playStatus);
    event.dev_ = rawAddr.GetAddress();
    AvrcpCtService::GetService()->PostEvent(event);
}

static void GetFolderItemsCallback(
    const BLUEDROID::RawAddress &bdAddr, btrc_status_t status, const btrc_folder_items_t *folderItems, uint8_t count)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, GetFolderItemsCallback ignored");
}

static void ChangePathCallback(const BLUEDROID::RawAddress &bdAddr, uint32_t count)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, ChangePathCallback ignored");
}

static void SetBrowsedPlayerCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t numItems, uint8_t depth)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, SetBrowsedPlayerCallback ignored");
}

static void SetAddressedPlayerCallback(const BLUEDROID::RawAddress &bdAddr, uint8_t status)
{
    HILOGI("SetAddressedPlayerCallback: status=%{public}d", status);
}

static void AddressedPlayerChangedCallback(const BLUEDROID::RawAddress &bdAddr, uint16_t id)
{
    HILOGI("AddressedPlayerChangedCallback: id=%{public}d", id);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED, id);
    event.dev_ = rawAddr.GetAddress();
    AvrcpCtService::GetService()->PostEvent(event);
}

static void NowPlayingContentsChangedCallback(const BLUEDROID::RawAddress &bdAddr)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, NowPlayingContentsChangedCallback ignored");
}

static void AvailablePlayerChangedCallback(const BLUEDROID::RawAddress &bdAddr)
{}

static void GetCoverArtPsmCallback(const BLUEDROID::RawAddress &bdAddr, const uint16_t psm)
{
    HILOGI("GetCoverArtPsmCallback: bdAddr=%{public}s, psm=%{public}d",
           ServiceUtil::AddrFromBluedroid(bdAddr).GetAddress().c_str(), psm);
    RawAddress rawAddr = ServiceUtil::AddrFromBluedroid(bdAddr);
    AvrcpCtMessage event(AVRCP_CT_GET_COVER_ART_PSM_EVT, psm);
    event.dev_ = rawAddr.GetAddress();
    AvrcpCtService::GetService()->PostEvent(event);
}

static btrc_ctrl_callbacks_t g_btAvrcpCallbacks = {
    sizeof(g_btAvrcpCallbacks),
    PassThroughResCallback,
    GroupNavigationResCallback,
    ConnectionStateCallback,
    GetRcFeaturesCallback,
    SetPlayerApplicationSettingRspCallback,
    PlayerApplicationSettingCallback,
    PlayerApplicationSettingChangedCallback,
    SetAbsVolCmdCallback,
    RegisterNotificationAbsVolCallback,
    TrackChangedCallback,
    PlayPositionChangedCallback,
    PlayStatusChangedCallback,
    GetFolderItemsCallback,
    ChangePathCallback,
    SetBrowsedPlayerCallback,
    SetAddressedPlayerCallback,
    AddressedPlayerChangedCallback,
    NowPlayingContentsChangedCallback,
    AvailablePlayerChangedCallback,
    GetCoverArtPsmCallback
};

/******************************************************************
 * ENABLE / DISABLE                                               *
 ******************************************************************/

void AvrcpCtService::Enable(void)
{
    DoInAvrcpCtThread(std::bind(&AvrcpCtService::StartUp, this));
}

void AvrcpCtService::Disable(void)
{
    DoInAvrcpCtThread(std::bind(&AvrcpCtService::ShutDown, this));
}
bool AvrcpCtService::IsEnabled(void)
{
    return true;
}

void AvrcpCtService::StartUp()
{
    HILOGI("enter");
    if (isStarted_) {
        GetContext()->OnEnable(PROFILE_NAME_AVRCP_CT, true);
        HILOGW("AvrcpCtService has already been started before.");
        return;
    }
    bt_interface_t *btInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (!btInterface) {
        HILOGE("btInterface is nullptr");
#ifdef BT_USE_OPEN_STACK
        GetContext()->OnEnable(PROFILE_NAME_AVRCP_CT, true);
#endif
        return;
    }

    btAvrcpInterface_ = (btrc_ctrl_interface_t *)btInterface->get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);
    if (!btAvrcpInterface_) {
#ifdef BT_USE_OPEN_STACK
        HILOGW("AVRCP CT profile unavailable on open stack, skip stack init");
        GetContext()->OnEnable(PROFILE_NAME_AVRCP_CT, true);
#else
        HILOGE("btAvrcpInterface is nullptr");
#endif
        return;
    }

    bt_status_t status = btAvrcpInterface_->init(&g_btAvrcpCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed to initialize Bluetooth Avrcp Controller, status: %{public}d", status);
        btAvrcpInterface_ = nullptr;
        return;
    }

    GetContext()->OnEnable(PROFILE_NAME_AVRCP_CT, true);
    isStarted_ = true;

    // 对齐双框架 new AvrcpCoverArtManager(this, new ImageDownloadCallback())：
    // 回调通过构造函数注入，全局唯一，按 deviceAddr 路由。
    coverArtManager_ = std::make_shared<AvrcpCtCoverArtManager>(
        [this](const std::string &deviceAddr, const std::string &imageUuid,
               const std::vector<uint8_t> &imageData) {
            OnCoverArtDownloaded(deviceAddr, imageUuid, imageData);
        });
}

void AvrcpCtService::ShutDown()
{
    HILOGI("enter");
    if (!isStarted_) {
        GetContext()->OnDisable(PROFILE_NAME_AVRCP_CT, true);
        HILOGW("AvrcpCtService has already been shutdown before.");
        return;
    }

    isShuttingDown_ = true;
    bool isNeedDisconnected = false;
    std::list<std::string> disconnectAddress;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
            if ((it->second != nullptr) && (it->second->GetDeviceState() > AVRCP_CT_STATE_DISCONNECTED)) {
                disconnectAddress.push_back(it->first);
                isNeedDisconnected = true;
            }
        }
    }
    for (std::string address : disconnectAddress) {
        Disconnect(RawAddress(address));
        HILOGI("disconnect deviceAddr=%{public}s", address.c_str());
    }
    if (!isNeedDisconnected) {
        ShutDownDone(true);
    }
}

void AvrcpCtService::ShutDownDone(bool isAllDisconnected)
{
    HILOGI("ShutDownDone %{public}d", isAllDisconnected);
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        if (!isAllDisconnected) {
            for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
                if ((it->second != nullptr) && (it->second->GetDeviceState() > AVRCP_CT_STATE_DISCONNECTED)) {
                    HILOGW("not all device disconnected.");
                    return;
                }
            }
        }
        for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
            if (it->second != nullptr) {
                it->second->CleanupAvSession();
            }
        }
        stateMachines_.clear();
    }
    isStarted_ = false;
    isShuttingDown_ = false;
    GetContext()->OnDisable(PROFILE_NAME_AVRCP_CT, true);
    if (coverArtManager_) {
        coverArtManager_->Cleanup();
        coverArtManager_ = nullptr;
    }
    if (btAvrcpInterface_ != nullptr) {
        btAvrcpInterface_->cleanup();
        btAvrcpInterface_ = nullptr;
    }
    HILOGI("AvrcpCtService shutdown");
}

void AvrcpCtService::PostEvent(const AvrcpCtMessage &event)
{
    HILOGI("PostEvent event_Id=[%{public}d]", event.what_);
    DoInAvrcpCtThread(std::bind(&AvrcpCtService::ProcessEvent, this, event));
}

void AvrcpCtService::ProcessEvent(const AvrcpCtMessage &event)
{
    HILOGI("Address=[%{public}s], event_Id=[%{public}d]", GET_ENCRYPT_STR_ADDR(event.dev_), event.what_);

    if (event.what_ == AVRCP_CT_CONNECT_EVT) {
        ProcessConnectEvent(event);
        return;
    }

    if (event.what_ == AVRCP_CT_DISCONNECT_EVT) {
        AvrcpCtMessage msg(STACK_DISCONNECT);
        auto it = stateMachines_.find(event.dev_);
        if (it != stateMachines_.end() && it->second != nullptr) {
            it->second->ProcessMessage(msg);
            if (event.dev_ == GetActiveDevice()) {
                SetActiveDevice("");
            }
        }
        return;
    }

    if (!event.dev_.empty()) {
        auto it = stateMachines_.find(event.dev_);
        if (it != stateMachines_.end() && it->second != nullptr) {
            it->second->ProcessMessage(event);
        } else {
            HILOGW("No state machine for device: %{public}s", GET_ENCRYPT_STR_ADDR(event.dev_));
        }
    }
}

void AvrcpCtService::ProcessConnectEvent(const AvrcpCtMessage &event)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(event.dev_);
    if (it == stateMachines_.end() || it->second == nullptr) {
        stateMachines_[event.dev_] = std::make_shared<AvrcpCtStateMachine>(event.dev_);
        stateMachines_[event.dev_]->Init();
    }

    if (event.connectFlag_.remoteCtlConnected) {
        AvrcpCtMessage msg(STACK_CONNECT);
        stateMachines_[event.dev_]->ProcessMessage(msg);
        if (GetActiveDevice() == "") {
            SetActiveDevice(event.dev_);
        }
    } else {
        AvrcpCtMessage msg(STACK_DISCONNECT);
        stateMachines_[event.dev_]->ProcessMessage(msg);
        if (event.dev_ == GetActiveDevice()) {
            SetActiveDevice("");
        }
    }
}

std::string AvrcpCtService::GetActiveDevice() const
{
    std::lock_guard<BtRecursiveMutex> lk(activeDevMutex_);
    return activeDevice_;
}

bool AvrcpCtService::FindSmAndSendMsg(const std::string &dev, int msg)
{
    auto it = stateMachines_.find(dev);
    if (it != stateMachines_.end() && it->second != nullptr) {
        AvrcpCtMessage message(msg);
        it->second->ProcessMessage(message);
        return true;
    }

    HILOGE("Invalid address:%{public}s", GET_ENCRYPT_STR_ADDR(dev));
    return false;
}

void AvrcpCtService::RemoveStateMachine(const std::string &dev)
{
    HILOGI("RemoveStateMachine: addr=%{public}s", GET_ENCRYPT_STR_ADDR(dev));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    stateMachines_.erase(dev);
}

bool AvrcpCtService::SetActiveDevice(const std::string &dev)
{
    std::string currentActiveDev = GetActiveDevice();
    if ((dev == "" && currentActiveDev == "") || (dev != "" && dev == currentActiveDev)) {
        return true;
    }

    A2dpSnkService *a2dpSinkService = A2dpSnkService::GetService();
    if (!a2dpSinkService) {
        HILOGE("a2dpSinkService is nullptr");
        return false;
    }

    if (!currentActiveDev.empty() && dev != currentActiveDev) {
        FindSmAndSendMsg(currentActiveDev, DEVICE_STATE_INACTIVE);
    }

    {
        std::lock_guard<BtRecursiveMutex> lk(activeDevMutex_);
        if (a2dpSinkService->SetActiveSrcDevice(RawAddress(dev)) == RET_NO_ERROR) {
            activeDevice_ = dev;
            if (!dev.empty()) {
                FindSmAndSendMsg(dev, DEVICE_STATE_ACTIVE);
            }
            return true;
        }
    }
    return false;
}

bool AvrcpCtService::SendPassThroughCommand(const RawAddress &rawAddr, uint8_t keyCode, uint8_t keyState)
{
    CHECK_AND_RETURN_LOG_RET(btAvrcpInterface_, false, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    bt_status_t status = btAvrcpInterface_->send_pass_through_cmd(btRawAddr, keyCode, keyState);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed sending passthrough cmd, status:%{public}d", status);
        return false;
    }
    return true;
}

void AvrcpCtService::SendAbsVolumeResponse(const RawAddress &rawAddr, int absVol, int label)
{
    HILOGI("SendAbsVolumeResponse: addr=%{public}s, absVol=%{public}d, label=%{public}d",
           GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()), absVol, label);
    CHECK_AND_RETURN_LOG(btAvrcpInterface_, "bluetoothAvrcpInterface is nullptr");
    if ((absVol < 0 || absVol > std::numeric_limits<uint8_t>::max()) ||
        (label < 0 || label > std::numeric_limits<uint8_t>::max())) {
        HILOGE("Invalid absVol or label value");
        return;
    }
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    btAvrcpInterface_->set_volume_rsp(btRawAddr, static_cast<uint8_t>(absVol), static_cast<uint8_t>(label));
}

void AvrcpCtService::SendRegisterAbsVolResponse(
    const RawAddress &rawAddr, int rspType, int absVol, int label)
{
    HILOGI("SendRegisterAbsVolResponse: addr=%{public}s, rspType=%{public}d, absVol=%{public}d, label=%{public}d",
           GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()), rspType, absVol, label);
    CHECK_AND_RETURN_LOG(btAvrcpInterface_, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    btAvrcpInterface_->register_abs_vol_rsp(
        btRawAddr, static_cast<btrc_notification_type_t>(rspType),
        static_cast<uint8_t>(absVol), static_cast<uint8_t>(label));
}

void AvrcpCtService::RequestCurrentMetadata(const RawAddress &rawAddr)
{
    HILOGI("RequestCurrentMetadata: addr=%{public}s", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
    CHECK_AND_RETURN_LOG(btAvrcpInterface_, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    btAvrcpInterface_->get_current_metadata_cmd(btRawAddr);
}

void AvrcpCtService::RequestPlaybackState(const RawAddress &rawAddr)
{
    HILOGI("RequestPlaybackState: addr=%{public}s", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
    CHECK_AND_RETURN_LOG(btAvrcpInterface_, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    btAvrcpInterface_->get_playback_state_cmd(btRawAddr);
}

void AvrcpCtService::RequestRemotePlay(const RawAddress &rawAddr)
{
    HILOGI("RequestRemotePlay: addr=%{public}s", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
    AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_PLAY);
    msg.dev_ = rawAddr.GetAddress();
    PostEvent(msg);
}

void AvrcpCtService::RequestRemotePause(const RawAddress &rawAddr)
{
    HILOGI("RequestRemotePause: addr=%{public}s", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
    AvrcpCtMessage msg(AVRCP_MSG_PASSTHRU, PASS_THRU_CMD_ID_PAUSE);
    msg.dev_ = rawAddr.GetAddress();
    PostEvent(msg);
}

std::string AvrcpCtService::GetUuidForHandle(const std::string &deviceAddr, const std::string &handle)
{
    if (coverArtManager_) {
        return coverArtManager_->GetUuidForHandle(deviceAddr, handle);
    }
    return "";
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

std::vector<RawAddress> AvrcpCtService::GetConnectedDevices(void)
{
    std::vector<RawAddress> result;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second && it->second->GetDeviceState() == AVRCP_CT_STATE_CONNECTED) {
            result.push_back(RawAddress(it->first));
        }
    }
    return result;
}

std::vector<bluetooth::RawAddress> AvrcpCtService::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<bluetooth::RawAddress> result;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (const auto &[addr, sm] : stateMachines_) {
        if (!sm) {
            continue;
        }
        int deviceState = sm->GetDeviceState();
        if (std::find(states.begin(), states.end(), deviceState) != states.end()) {
            result.emplace_back(RawAddress(addr));
        }
    }
    return result;
}

int AvrcpCtService::GetMaxConnectNum(void)
{
    return 1;
}

int AvrcpCtService::GetDeviceState(const RawAddress &rawAddr)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(rawAddr.GetAddress());
    if (it != stateMachines_.end() && it->second != nullptr) {
        return it->second->GetDeviceState();
    }
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

int AvrcpCtService::Connect(const RawAddress &rawAddr)
{
    HILOGI("Connect: addr=%{public}s", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    auto it = stateMachines_.find(rawAddr.GetAddress());
    if (it != stateMachines_.end() && it->second != nullptr) {
        int state = it->second->GetDeviceState();
        if (state == AVRCP_CT_STATE_CONNECTED || state == AVRCP_CT_STATE_CONNECTING) {
            return RET_NO_ERROR;
        }
    }
    return RET_BAD_STATUS;
}

// avrcp内部stack发送断开消息，上层不发断开命令
int AvrcpCtService::Disconnect(const RawAddress &rawAddr)
{
    int result = RET_NO_ERROR;

    return result;
}

int AvrcpCtService::GetConnectState(void)
{
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second && it->second->GetDeviceState() != AVRCP_CT_STATE_DISCONNECTED) {
            return PROFILE_STATE_CONNECTED;
        }
    }
    return PROFILE_STATE_DISCONNECTED;
}

/******************************************************************
 * BUTTON OPERATION                                               *
 ******************************************************************/

int AvrcpCtService::PressButton(const RawAddress &rawAddr, uint8_t button)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::ReleaseButton(const RawAddress &rawAddr, uint8_t button)
{
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * UNIT INFO / SUB UNIT INFO                                      *
 ******************************************************************/

int AvrcpCtService::GetUnitInfo(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetSubUnitInfo(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * Media Player Selection                                         *
 ******************************************************************/

int AvrcpCtService::SetAddressedPlayer(const RawAddress &rawAddr, uint16_t playerId)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::SetBrowsedPlayer(const RawAddress &rawAddr, uint16_t playerId)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, SetBrowsedPlayer ignored");
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * Capabilities                                                   *
 ******************************************************************/

int AvrcpCtService::GetSupportedCompanies(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetSupportedEvents(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * PLAYER APPLICATION SETTINGS                                    *
 ******************************************************************/

int AvrcpCtService::GetPlayerAppSettingAttributes(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetPlayerAppSettingValues(const RawAddress &rawAddr, uint8_t attribute)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::SetPlayerAppSettingCurrentValue(
    const RawAddress &rawAddr, const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values)
{
    CHECK_AND_RETURN_LOG_RET(btAvrcpInterface_, false, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    std::vector<uint8_t> attributesCopy(attributes.begin(), attributes.end());
    std::vector<uint8_t> valuesCopy(values.begin(), values.end());
    bt_status_t status = btAvrcpInterface_->set_player_app_setting_cmd(
        btRawAddr, static_cast<uint8_t>(attributesCopy.size()), attributesCopy.data(), valuesCopy.data());
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed sending setRepeatMode cmd, status:%{public}d", status);
        return RET_BAD_STATUS;
    }
    return RET_NO_ERROR;
}

int AvrcpCtService::GetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetPlayerAppSettingValueText(
    const RawAddress &rawAddr, uint8_t attributeId, const std::vector<uint8_t> &values)
{
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * MEDIA INFORMATION                                              *
 ******************************************************************/

int AvrcpCtService::GetElementAttributes(
    const RawAddress &rawAddr, uint64_t identifier, const std::vector<uint32_t> &attributes)
{
    int result = RET_BAD_STATUS;
    return result;
}

/******************************************************************
 * PLAY                                                           *
 ******************************************************************/

int AvrcpCtService::GetPlayStatus(const RawAddress &rawAddr)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::PlayItem(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::AddToNowPlaying(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter)
{
    int result = RET_BAD_STATUS;
    return result;
}

/******************************************************************
 * CONTINUING RESPONSE / ABORT CONTINUING RESPONSE                *
 ******************************************************************/

int AvrcpCtService::RequestContinuingResponse(const RawAddress &rawAddr, uint8_t pduId)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::AbortContinuingResponse(const RawAddress &rawAddr, uint8_t pduId)
{
    int result = RET_BAD_STATUS;

    return result;
}

/******************************************************************
 * OPERATE THE VIRTUAL FILE SYSTEM                                *
 ******************************************************************/

int AvrcpCtService::ChangePath(const RawAddress &rawAddr, uint16_t uidCounter, uint8_t direction, uint64_t folderUid)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, ChangePath ignored");
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::GetFolderItems(const RawAddress &rawAddr, uint8_t scope, uint32_t startItem, uint32_t endItem,
    const std::vector<uint32_t> &attributes)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, GetFolderItems ignored");
    return RET_BAD_STATUS;
}

int AvrcpCtService::GetItemAttributes(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter,
    const std::vector<uint32_t> &attributes)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, GetItemAttributes ignored");
    return RET_BAD_STATUS;
}

int AvrcpCtService::GetTotalNumberOfItems(const RawAddress &rawAddr, uint8_t scope)
{
    HILOGI("[BIP_NOT_SUPPORT] Browse channel not supported, GetTotalNumberOfItems ignored");
    return RET_BAD_STATUS;
}

/******************************************************************
 * ABSOLUTE VOLUME                                                *
 ******************************************************************/

int AvrcpCtService::SetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume)
{
    HILOGI("SetAbsoluteVolume: addr=%{public}s, volume=%{public}d",
           GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()), volume);
    CHECK_AND_RETURN_LOG_RET(btAvrcpInterface_, RET_BAD_STATUS, "bluetoothAvrcpInterface is nullptr");
    BLUEDROID::RawAddress btRawAddr = ServiceUtil::AddrToBluedroid(rawAddr);
    bt_status_t status = btAvrcpInterface_->set_volume_rsp(btRawAddr, volume, 0);
    return (status == BT_STATUS_SUCCESS) ? RET_NO_ERROR : RET_BAD_STATUS;
}

/******************************************************************
 * NOTIFICATION                                                   *
 ******************************************************************/

int AvrcpCtService::EnableNotification(const RawAddress &rawAddr, const std::vector<uint8_t> &events, uint8_t interval)
{
    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpCtService::DisableNotification(const RawAddress &rawAddr, const std::vector<uint8_t> &events)
{
    int result = RET_BAD_STATUS;

    return result;
}

void AvrcpCtService::GetCurrentMetadataIfNoCoverArt(const std::string &device)
{
    // 仅当当前曲目还没有封面数据时才重新请求元数据，避免无谓请求
    std::shared_ptr<AvrcpCtStateMachine> sm;
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(device);
        if (it == stateMachines_.end() || it->second == nullptr) {
            return;
        }
        sm = it->second;
    }
    auto track = sm->GetCurrentTrack();
    if (track == nullptr || track->GetCoverArtData().empty()) {
        HILOGI("Current track has no cover art, re-request metadata: %{public}s", GET_ENCRYPT_STR_ADDR(device));
        RequestCurrentMetadata(RawAddress(device));
    }
}

void AvrcpCtService::OnCoverArtDownloaded(const std::string &deviceAddr, const std::string &imageUuid,
                                          const std::vector<uint8_t> &imageData)
{
    HILOGI("Cover art downloaded: device=%{public}s, uuid=%{public}s, size=%{public}zu",
           GET_ENCRYPT_STR_ADDR(deviceAddr), imageUuid.c_str(), imageData.size());

    // 对齐双框架 ImageDownloadCallback：找不到 StateMachine 时 removeImage 清理，防止图片泄漏
    {
        std::lock_guard<BtRecursiveMutex> lk(mutex_);
        auto it = stateMachines_.find(deviceAddr);
        if (it == stateMachines_.end() || it->second == nullptr) {
            HILOGE("No state machine for device %{public}s, removing downloaded image",
                   GET_ENCRYPT_STR_ADDR(deviceAddr));
            if (coverArtManager_) {
                coverArtManager_->RemoveImage(deviceAddr, imageUuid);
            }
            return;
        }
    }

    AvrcpCtMessage event(AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED);
    event.dev_ = deviceAddr;
    event.uuid_ = imageUuid;
    PostEvent(event);
}

std::vector<RawAddress> AvrcpCtService::GetDevicesMatchingConnectionStates(void)
{
    std::vector<RawAddress> result;
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto it = stateMachines_.begin(); it != stateMachines_.end(); ++it) {
        if (it->second && it->second->GetDeviceState() == AVRCP_CT_STATE_CONNECTING) {
            result.push_back(RawAddress(it->first));
        }
    }
    return result;
}

int AvrcpCtService::GetConnectionState(const std::string &device)
{
    auto it = stateMachines_.find(device);
    if (it != stateMachines_.end()) {
        return it->second->GetDeviceState();
    }
    return AVRCP_CT_STATE_DISCONNECTED;
}

void AvrcpCtService::GetRcPsm(const std::string &dev, int psm)
{
    auto it = stateMachines_.find(dev);
    if (it != stateMachines_.end() && it->second != nullptr) {
        AvrcpCtMessage message(AVRCP_CT_MSG_PROCESS_RECEIVED_COVER_ART_PSM, psm);
        it->second->ProcessMessage(message);
        return;
    }

    HILOGE("Invalid address:%{public}s", GET_ENCRYPT_STR_ADDR(dev));
    return;
}

void AvrcpCtService::HandleAddressedPlayerChanged(const std::string &dev, int id)
{
    auto it = stateMachines_.find(dev);
    if (it != stateMachines_.end() && it->second != nullptr) {
        AvrcpCtMessage message(AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED, id);
        it->second->ProcessMessage(message);
        return;
    }

    HILOGE("Invalid address:%{public}s", GET_ENCRYPT_STR_ADDR(dev));
    return;
}

void AvrcpCtService::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    HILOGI("NotifyAudioVolumeEvent: streamType=%{public}d, volume=%{public}d", streamType, volume);
    // 对齐双框架 processAbsoluteVolumeChangeNotification: 向所有已连接 SM 投递 VOLUME_CHANGED_NOTIFICATION
    std::lock_guard<BtRecursiveMutex> lk(mutex_);
    for (auto &pair : stateMachines_) {
        if (pair.second != nullptr) {
            AvrcpCtMessage msg(AVRCP_CT_MSG_PROCESS_VOLUME_CHANGED_NOTIFICATION, volume);
            msg.dev_ = pair.first;
            PostEvent(msg);
        }
    }
}

REGISTER_CLASS_CREATOR(AvrcpCtService);
}  // namespace bluetooth
}  // namespace OHOS