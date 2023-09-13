/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "bluetooth_load_system_ability.h"

#include "bluetooth_a2dp_snk.h"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_avrcp_ct.h"
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_hfp_ag.h"
#include "bluetooth_hfp_hf.h"
#include "bluetooth_hid_host.h"
#include "bluetooth_opp.h"
#include "bluetooth_pan.h"
#include "log.h"
#include "system_ability_definition.h"

namespace OHOS {
class IRemoteObject;
}
namespace OHOS {
namespace Bluetooth {

BluetootLoadSystemAbility::BluetootLoadSystemAbility()
{}

BluetootLoadSystemAbility::~BluetootLoadSystemAbility()
{}

void BluetootLoadSystemAbility::SubScribeBluetoothSystemAbility()
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    if (subscribeBluetoothSystemAbilityState != STATE_NONE) {
        return;
    }
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("[BLUETOOTH_LOAD_SYSTEM] failed to get samgrProxy");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, this);
    if (ret != ERR_OK) {
        HILOGE("[BLUETOOTH_LOAD_SYSTEM] subscribe systemAbilityId: bluetooth service failed!");
        return;
    }
    subscribeBluetoothSystemAbilityState = STATE_SUBSCRIBING;
}

BluetootLoadSystemAbility &BluetootLoadSystemAbility::GetInstance()
{
    static BluetootLoadSystemAbility bluetoothLoadSystemAbility;
    return bluetoothLoadSystemAbility;
}

bool BluetootLoadSystemAbility::HasSubscribedBluetoothSystemAbility()
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    HILOGI("[BLUETOOTH_LOAD_SYSTEM] subscribedState=%{public}d", subscribeBluetoothSystemAbilityState);
    if (subscribeBluetoothSystemAbilityState != STATE_SUBSCRIBED) {
        return false;
    }
    return true;
}

void BluetootLoadSystemAbility::RegisterNotifyMsg(const uint32_t &profileId)
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    profileIdList_.push_back(profileId);
}

void BluetootLoadSystemAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    HILOGI("[BLUETOOTH_LOAD_SYSTEM]OnAddSystemAbility systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case BLUETOOTH_HOST_SYS_ABILITY_ID: {
            NotifyMsgToProfile(NOTIFY_MSG_INIT);
            subscribeBluetoothSystemAbilityState = STATE_SUBSCRIBED;
            break;
        }
        default:
            HILOGE("[BLUETOOTH_LOAD_SYSTEM]unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    return;
}

void BluetootLoadSystemAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    return;
}

void BluetootLoadSystemAbility::NotifyMsgToProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    for (auto it = profileIdList_.begin(); it != profileIdList_.end(); ++it) {
        NotifyProfile(notifyMsg, *it);
    }
}

void BluetootLoadSystemAbility::NotifyProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId)
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    switch (profileId) {
        case PROFILE_ID_A2DP_SRC:
        case PROFILE_ID_A2DP_SINK:
        case PROFILE_ID_HFP_AG:
        case PROFILE_ID_HFP_HF:
        case PROFILE_ID_AVRCP_TG:
        case PROFILE_ID_AVRCP_CT:
            NotifyAudioProfile(notifyMsg, profileId);
            break;
        case PROFILE_ID_MAP_MCE:
        case PROFILE_ID_MAP_MSE:
        case PROFILE_ID_HID_HOST_SERVER:
        case PROFILE_ID_OPP_SERVER:
        case PROFILE_ID_PAN_SERVER:
            NotifyTransferProfile(notifyMsg, profileId);
            break;
        case PROFILE_ID_HOST:
            NotifyHostInit(notifyMsg);
            break;
        default:
            break;
    }
}

void BluetootLoadSystemAbility::NotifyHostInit(NOTIFY_PROFILE_MSG notifyMsg)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    if (host == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        host->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyA2dpSrcProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    A2dpSource *profile = A2dpSource::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyA2dpSinkProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    A2dpSink *profile = A2dpSink::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyHfpAgProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyHfpHfProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyAvrcpTgProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    AvrcpTarget *profile = AvrcpTarget::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyAvrcpCtProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    AvrcpController *profile = AvrcpController::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyAudioProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId)
{
    switch (profileId) {
        case PROFILE_ID_A2DP_SRC:
            NotifyA2dpSrcProfile(notifyMsg);
            break;
        case PROFILE_ID_A2DP_SINK:
            NotifyA2dpSinkProfile(notifyMsg);
            break;
        case PROFILE_ID_HFP_AG:
            NotifyHfpAgProfile(notifyMsg);
            break;
        case PROFILE_ID_HFP_HF:
            NotifyHfpHfProfile(notifyMsg);
            break;
        case PROFILE_ID_AVRCP_TG:
            NotifyAvrcpTgProfile(notifyMsg);
            break;
        case PROFILE_ID_AVRCP_CT:
            NotifyAvrcpCtProfile(notifyMsg);
            break;
        default:
            break;
    }
}

void BluetootLoadSystemAbility::NotifyOppProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    Opp *profile = Opp::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyHidHostProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    HidHost *profile = HidHost::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyPanProfile(NOTIFY_PROFILE_MSG notifyMsg)
{
    Pan *profile = Pan::GetProfile();
    if (profile == nullptr) {
        return;
    }
    if (notifyMsg == NOTIFY_MSG_INIT) {
        profile->Init();
        return;
    }
}

void BluetootLoadSystemAbility::NotifyTransferProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId)
{
    switch (profileId) {
        case PROFILE_ID_OPP_SERVER:
            NotifyOppProfile(notifyMsg);
        break;

        case PROFILE_ID_HID_HOST_SERVER:
            NotifyHidHostProfile(notifyMsg);
            break;
        case PROFILE_ID_PAN_SERVER:
            NotifyPanProfile(notifyMsg);
            break;
        default:
            break;
    }
}

}  // namespace Bluetooth
}  // namespace OHOS