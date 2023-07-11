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

#ifndef BLUETOOTH_LOAD_SYSTEM_ABILITY_H
#define BLUETOOTH_LOAD_SYSTEM_ABILITY_H

#include <string>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Bluetooth {

enum SUBSCRIBED_SYSTEM_ABILITY_STATE {
    STATE_NONE,
    STATE_SUBSCRIBING,
    STATE_SUBSCRIBED,
};

enum NOTIFY_PROFILE_MSG { NOTIFY_MSG_INIT, NOTIFY_MSG_UNINIT };

class BluetootLoadSystemAbility : public SystemAbilityStatusChangeStub {
public:
    /**
     * @brief Get the Instance of the BluetootLoadSystemAbility.
     *
     * @return Returns the Instance of the BluetootLoadSystemAbility.
     */
    static BluetootLoadSystemAbility &GetInstance();
    /**
     * @brief When an external process accesses bluetooth profile, the profile has not yet obtained the proxy. At this
     * time, it is necessary to subscribe to the status of the start and stop of the Bluetooth process, and then
     * register the id of the profile, so that it can notify the profile initialization or uninitialization in time
     * after receiving the start and stop of the Bluetooth.
     *
     * @param profileId profile identification.
     */
    void RegisterNotifyMsg(const uint32_t &profileId);
    /**
     * @brief Used to query whether the process currently calling the profile interface has subscribed to the start and
     * stop of the Bluetooth process.
     *
     * @return Returns true if has subscribed;
     *         Returns false If it is the first time to access the interface of this profile or is subscribing.
     */
    bool HasSubscribedBluetoothSystemAbility();
    /**
     * @brief If the process accesses the profile interface of the Bluetooth process for the first time, call this
     * interface to subscribe to the start and stop of the Bluetooth process.
     *
     */
    void SubScribeBluetoothSystemAbility();

    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    /**
     * @brief After receiving the Bluetooth start and stop notification, notify each profile to initialize or
     * uninitialize in time.
     *
     * @param profileId profile identification.
     */
    void NotifyMsgToProfile(NOTIFY_PROFILE_MSG notifyMsg);
    /**
     * @brief Notify each registered profile.
     *
     * @param notifyMsg NOTIFY_MSG_INIT:the Bluetooth process wakes up and notifies profile initialization;
     * NOTIFY_MSG_UNINIT:after the Bluetooth process is turned off, notify each profile to uninitialize.
     * @param profileId profile identification.
     */
    void NotifyProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId);

    void NotifyA2dpSrcProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyA2dpSinkProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyHfpAgProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyHfpHfProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyAvrcpTgProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyAvrcpCtProfile(NOTIFY_PROFILE_MSG notifyMsg);

    void NotifyMapClientProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyMapServerProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyPbapClientProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyPbapServerProfile(NOTIFY_PROFILE_MSG notifyMsg);

    void NotifyOppProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyHidHostProfile(NOTIFY_PROFILE_MSG notifyMsg);
    void NotifyPanProfile(NOTIFY_PROFILE_MSG notifyMsg);

    void NotifyAudioProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId);
    void NotifyMessageProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId);
    void NotifyTransferProfile(NOTIFY_PROFILE_MSG notifyMsg, const uint32_t &profileId);
    void NotifyHostInit();

    /**
     * @brief Construct a new BluetootLoadSystemAbility object.
     */
    BluetootLoadSystemAbility();

    /**
     * @brief Destroy the BluetootLoadSystemAbility object.
     */
    ~BluetootLoadSystemAbility();

    std::recursive_mutex mutex_{};
    std::vector<uint32_t> profileIdList_{};
    SUBSCRIBED_SYSTEM_ABILITY_STATE subscribeBluetoothSystemAbilityState = STATE_NONE;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HOST_LOAD_CALLBACK_H