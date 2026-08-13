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
#define LOG_TAG "bt_service_profile_manager"
#endif

#include "profile_service_manager.h"

#include <algorithm>

#include "bt_def.h"
#include "log.h"

#include "adapter_config.h"
#include "adapter_manager.h"
#include "class_creator.h"
#include "profile_info.h"
#include "profile_list.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
class ProfileServicesContextCallback : public utility::IContextCallback {
public:
    explicit ProfileServicesContextCallback(ProfileServiceManager &psm) : psm_(psm){};
    ~ProfileServicesContextCallback() override = default;

    void OnEnable(const std::string &name, bool ret) override
    {
        HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
        psm_.OnEnable(name, ret);
    }

    void OnDisable(const std::string &name, bool ret) override
    {
        HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
        psm_.OnDisable(name, ret);
    }

private:
    ProfileServiceManager &psm_;
};

// ProfileServiceManager
enum ServiceStateID {
    TURNING_ON = BTStateID::STATE_TURNING_ON,
    TURN_ON = BTStateID::STATE_TURN_ON,
    TURNING_OFF = BTStateID::STATE_TURNING_OFF,
    TURN_OFF = BTStateID::STATE_TURN_OFF,
    WAIT_TURN_ON,
};
struct ProfileServiceManager::impl {
    impl() = default;
    ProfilesList<IProfile *> startedProfiles_ = {};
    ProfilesList<ServiceStateID> profilesState_ = {};
    std::unique_ptr<ProfileServicesContextCallback> contextCallback_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(impl);
};

IProfileManager *IProfileManager::GetInstance()
{
    return ProfileServiceManager::GetInstance();
}

ProfileServiceManager *ProfileServiceManager::GetInstance()
{
    static ProfileServiceManager instance;
    return &instance;
}

void ProfileServiceManager::Initialize()
{
    GetInstance()->Start();
}

void ProfileServiceManager::Uninitialize()
{
    GetInstance()->Stop();
}

ProfileServiceManager::ProfileServiceManager() : pimpl(std::make_unique<ProfileServiceManager::impl>())
{
    // context callback create
    pimpl->contextCallback_ = std::make_unique<ProfileServicesContextCallback>(*this);
}

ProfileServiceManager::~ProfileServiceManager()
{}

void ProfileServiceManager::Start() const
{
    if (IAdapterManager::GetInstance()->GetClassicAdapterInterface()) {
        CreateClassicProfileServices();
    }

    if (IAdapterManager::GetInstance()->GetBleAdapterInterface()) {
        CreateBleProfileServices();
    }
}

void ProfileServiceManager::CreateClassicProfileServices() const
{
    for (auto &sp : GET_CONFIG_PROFILES(BTTransport::ADAPTER_BREDR)) {
        IProfile *profile = ClassCreator<IProfile>::NewInstance(sp.name_);

        if (profile != nullptr) {
            HILOGD("%{public}s", sp.name_.c_str());
            profile->GetContext()->Initialize();
            profile->GetContext()->RegisterCallback(*(pimpl->contextCallback_));
            pimpl->startedProfiles_.SetProfile(BTTransport::ADAPTER_BREDR, sp.name_, profile);
            pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BREDR, sp.name_, ServiceStateID::TURN_OFF);
        } else {
            HILOGE("%{public}s is not registered!!!", sp.name_.c_str());
        }
    }
}

void ProfileServiceManager::CreateBleProfileServices() const
{
    for (auto &sp : GET_CONFIG_PROFILES(BTTransport::ADAPTER_BLE)) {
        // BREDR started the profile
        IProfile *bredrProfile = nullptr;
        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, sp.name_, bredrProfile)) {
            // BREDR create the profile
            HILOGI("classic %{public}s", sp.name_.c_str());
            pimpl->startedProfiles_.SetProfile(BTTransport::ADAPTER_BLE, sp.name_, bredrProfile);
            // init profilesState_
            pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BLE, sp.name_, ServiceStateID::TURN_OFF);
        } else {
            // the profile is not create
            IProfile *newProfile = ClassCreator<IProfile>::NewInstance(sp.name_);
            if (newProfile != nullptr) {
                HILOGI("%{public}s", sp.name_.c_str());
                newProfile->GetContext()->Initialize();
                newProfile->GetContext()->RegisterCallback(*(pimpl->contextCallback_));
                pimpl->startedProfiles_.SetProfile(BTTransport::ADAPTER_BLE, sp.name_, newProfile);
                pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BLE, sp.name_, ServiceStateID::TURN_OFF);
            } else {
                HILOGE("%{public}s is not registered!!!", sp.name_.c_str());
            }
        }
    }
}

void ProfileServiceManager::Stop() const
{
    for (auto &sp : GET_SUPPORT_PROFILES()) {
        IProfile *profile = nullptr;
        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, sp.name_, profile)) {
            profile->GetContext()->Uninitialize();
            pimpl->startedProfiles_.SetProfile(BTTransport::ADAPTER_BREDR, sp.name_, nullptr);
            delete profile;
        } else if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BLE, sp.name_, profile)) {
            profile->GetContext()->Uninitialize();
            pimpl->startedProfiles_.SetProfile(BTTransport::ADAPTER_BLE, sp.name_, nullptr);
            delete profile;
        } else {
            // Nothing to do
        }
    }

    pimpl->startedProfiles_.Clear();
    pimpl->profilesState_.Clear();
}

IProfile *ProfileServiceManager::GetProfileService(const std::string &name) const
{
    IProfile *profile = nullptr;

    if (pimpl->startedProfiles_.Find(name, profile)) {
        return profile;
    } else {
        return nullptr;
    }
}

bool ProfileServiceManager::Enable(const BTTransport transport) const
{
    HILOGI("transport is %{public}d", transport);

    if (IsAllEnabled(transport)) {
        HILOGI("OK");
        AdapterManager::GetInstance()->OnProfileServicesEnableComplete(transport, true);
    } else {
        EnableProfiles(transport);
        if (IsAllEnabled(transport)) {
            HILOGI("OK");
            AdapterManager::GetInstance()->OnProfileServicesEnableComplete(transport, true);
        }
    }
    return true;
}

void ProfileServiceManager::OnAllEnabled(const BTTransport transport) const
{
    HILOGI("%{public}d", transport);

    if (pimpl->startedProfiles_.IsEmpty(transport)) {
        HILOGI("empty %{public}d", transport);
        return;
    }

    auto *profiles = pimpl->profilesState_.GetProfiles(transport);
    if (profiles == nullptr) {
        return;
    }
    for (auto it : *profiles) {
        pimpl->profilesState_.SetProfile(transport, it.first, ServiceStateID::TURN_ON);
    }
}

bool ProfileServiceManager::IsAllEnabled(const BTTransport transport) const
{
    if (pimpl->startedProfiles_.IsEmpty(transport)) {
        HILOGI("empty %{public}d", transport);
        return true;
    }
    if (pimpl->profilesState_.GetProfiles(transport) == nullptr) {
        HILOGE("GetProfiles() return nullptr");
        return false;
    }
    const bool any = std::any_of(pimpl->profilesState_.GetProfiles(transport)->begin(),
        pimpl->profilesState_.GetProfiles(transport)->end(),
        [](const auto &temp) -> bool { return temp.second != ServiceStateID::TURN_ON; });
    if (any) {
        HILOGI("false %{public}d", transport);
        return false;
    }
    HILOGI("true %{public}d", transport);
    return true;
}

bool ProfileServiceManager::NeedWaitOtherProfileComplete(const std::string &name) const
{
    if (name == PROFILE_NAME_AVRCP_TG) {
        HILOGI("avrcp wait a2dp service complete...");
        return true;
    }

    if (name == PROFILE_NAME_PBAP_PSE) {
        HILOGI("pbap pse wait socket service complete...");
        return true;
    }

    if (name == PROFILE_NAME_MAP_MSE) {
        HILOGI("map mse wait socket service complete...");
        return true;
    }

    if (name == PROFILE_NAME_OPP) {
        HILOGI("opp wait socket service complete...");
        return true;
    }
    return false;
}

void ProfileServiceManager::EnableProfiles(const BTTransport transport) const
{
    auto *profiles = pimpl->profilesState_.GetProfiles(transport);
    if (profiles == nullptr) {
        return;
    }
    for (auto it : *profiles) {
        std::string name = it.first;
        BTTransport otherTransport =
            (transport == BTTransport::ADAPTER_BREDR) ? BTTransport::ADAPTER_BLE : BTTransport::ADAPTER_BREDR;
        ServiceStateID otherTransportState = ServiceStateID::TURN_OFF;

        if (pimpl->profilesState_.Find(otherTransport, name, otherTransportState)) {
            switch (otherTransportState) {
                case ServiceStateID::TURN_ON:
                    HILOGI("TURN_ON otherTransport %{public}d %{public}s", otherTransport, name.c_str());
                    pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURN_ON);
                    break;
                case ServiceStateID::TURNING_OFF:
                    HILOGI("TURNING_OFF otherTransport %{public}d %{public}s", otherTransport, name.c_str());
                    pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::WAIT_TURN_ON);
                    break;
                case ServiceStateID::TURNING_ON:
                    HILOGI("TURNING_ON otherTransport %{public}d %{public}s", otherTransport, name.c_str());
                    pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURNING_ON);
                    break;
                default:
                    break;
            }
        }

        if (pimpl->profilesState_.Get(transport, name) == ServiceStateID::TURN_OFF) {
            pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURNING_ON);
            // Initialize the avrcp service only after the a2dp service is initialized.
            // Otherwise, the avct_cb resource allocated by the avrcp service will be cleared by the a2dp service.
            if (NeedWaitOtherProfileComplete(name)) {
                continue;
            }
            HILOGI("transport %{public}d %{public}s enable", transport, name.c_str());
            IProfile *profile = nullptr;
            if (pimpl->startedProfiles_.Find(transport, name, profile)) {
                profile->GetContext()->Enable();
            } else {
                HILOGI("startedProfiles_ is not find");
            }
        }
    }
}

void ProfileServiceManager::OnEnable(const std::string &name, bool ret) const
{
    HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
    DoInAdapterManagerThread([this, name, ret] {this->EnableCompleteProcess(name, ret);});
}

void ProfileServiceManager::CheckEnableOtherProfile(const std::string &enabledProfile) const
{
    // when a2dp initialized, init avrcp
    if (enabledProfile == PROFILE_NAME_A2DP_SRC) {
        HILOGI("init avrcpTg");
        IProfile *profile = nullptr;
        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, PROFILE_NAME_AVRCP_TG, profile)) {
            profile->GetContext()->Enable();
        }
    }

    // pbap map opp wait socket
    if (enabledProfile == PROFILE_NAME_SPP) {
        HILOGI("init map mse");
        IProfile *profile = nullptr;
        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, PROFILE_NAME_PBAP_PSE, profile)) {
            profile->GetContext()->Enable();
        }

        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, PROFILE_NAME_MAP_MSE, profile)) {
            profile->GetContext()->Enable();
        }

        if (pimpl->startedProfiles_.Find(BTTransport::ADAPTER_BREDR, PROFILE_NAME_OPP, profile)) {
            profile->GetContext()->Enable();
        }
    }
}

void ProfileServiceManager::EnableCompleteProcess(const std::string &name, bool ret) const
{
    ServiceStateID newState = ret ? ServiceStateID::TURN_ON : ServiceStateID::TURN_OFF;
    std::string profileName = name;
    CheckEnableOtherProfile(profileName);
    ServiceStateID state = ServiceStateID::TURN_OFF;
    if ((pimpl->profilesState_.Find(BTTransport::ADAPTER_BREDR, profileName, state)) &&
        (state == ServiceStateID::TURNING_ON)) {
        HILOGI("BREDR %{public}s complete ret %{public}d", profileName.c_str(), ret);
        pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BREDR, profileName, newState);
        if (!IsProfilesTurning(BTTransport::ADAPTER_BREDR)) {
            EnableCompleteNotify(BTTransport::ADAPTER_BREDR);
        }
    }

    if ((pimpl->profilesState_.Find(BTTransport::ADAPTER_BLE, profileName, state)) &&
        (state == ServiceStateID::TURNING_ON)) {
        HILOGI("BLE %{public}s complete ret %{public}d", profileName.c_str(), ret);
        pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BLE, profileName, newState);
        if (!IsProfilesTurning(BTTransport::ADAPTER_BLE)) {
            EnableCompleteNotify(BTTransport::ADAPTER_BLE);
        }
    }
}

bool ProfileServiceManager::IsProfilesTurning(const BTTransport transport) const
{
    if (pimpl->startedProfiles_.IsEmpty(transport)) {
        HILOGI("empty %{public}d", transport);
        return false;
    }
    if (pimpl->profilesState_.GetProfiles(transport) == nullptr) {
        HILOGE("GetProfiles() return nullptr");
        return false;
    }
    const bool any = std::any_of(pimpl->profilesState_.GetProfiles(transport)->begin(),
        pimpl->profilesState_.GetProfiles(transport)->end(),
        [](const auto &temp) -> bool {
            return temp.second != ServiceStateID::TURN_ON && temp.second != ServiceStateID::TURN_OFF;
        });
    if (any) {
        HILOGD("true %{public}d", transport);
        return true;
    }
    HILOGI("false %{public}d", transport);
    return false;
}

void ProfileServiceManager::EnableCompleteNotify(const BTTransport transport) const
{
    if (pimpl->profilesState_.GetProfiles(transport) == nullptr) {
        HILOGE("GetProfiles() return nullptr");
        return;
    }
    int turnOnProfileCount = std::count_if(pimpl->profilesState_.GetProfiles(transport)->begin(),
        pimpl->profilesState_.GetProfiles(transport)->end(),
        [](const auto &temp) -> bool { return temp.second == ServiceStateID::TURN_ON; });
    if (turnOnProfileCount == pimpl->profilesState_.Size(transport)) {
        HILOGI("OK transport %{public}d turnOnProfileCount %{public}d", transport, turnOnProfileCount);
        AdapterManager::GetInstance()->OnProfileServicesEnableComplete(transport, true);
    } else {
        HILOGI("NG transport %{public}d turnOnProfileCount %{public}d", transport, turnOnProfileCount);
        AdapterManager::GetInstance()->OnProfileServicesEnableComplete(transport, false);
    }
}

bool ProfileServiceManager::Disable(const BTTransport transport) const
{
    HILOGI("transport is %{public}d", transport);

    if (IsAllDisabled(transport)) {
        HILOGI("OK");
        AdapterManager::GetInstance()->OnProfileServicesDisableComplete(transport, true);
    } else {
        DisableProfiles(transport);
        if (IsAllDisabled(transport)) {
            HILOGI("OK");
            AdapterManager::GetInstance()->OnProfileServicesDisableComplete(transport, true);
        }
    }

    return true;
}

void ProfileServiceManager::OnAllDisabled(const BTTransport transport) const
{
    HILOGI("%{public}d", transport);

    if (pimpl->startedProfiles_.IsEmpty(transport)) {
        HILOGI("empty %{public}d", transport);
        return;
    }

    auto *profiles = pimpl->profilesState_.GetProfiles(transport);
    if (profiles == nullptr) {
        return;
    }
    for (auto it : *profiles) {
        pimpl->profilesState_.SetProfile(transport, it.first, ServiceStateID::TURN_OFF);
    }
}

bool ProfileServiceManager::IsAllDisabled(const BTTransport transport) const
{
    if (pimpl->startedProfiles_.IsEmpty(transport)) {
        HILOGI("empty %{public}d", transport);
        return true;
    }
    if (pimpl->profilesState_.GetProfiles(transport) == nullptr) {
        HILOGE("GetProfiles() return nullptr");
        return false;
    }
    const bool any = std::any_of(pimpl->profilesState_.GetProfiles(transport)->begin(),
        pimpl->profilesState_.GetProfiles(transport)->end(),
        [](const auto &temp) -> bool { return temp.second != ServiceStateID::TURN_OFF; });
    if (any) {
        HILOGI("false %{public}d", transport);
        return false;
    }
    HILOGI("true %{public}d", transport);
    return true;
}

void ProfileServiceManager::DisableProfiles(const BTTransport transport) const
{
    auto *profiles = pimpl->profilesState_.GetProfiles(transport);
    if (profiles == nullptr) {
        return;
    }
    for (auto it : *profiles) {
        ServiceStateID otherTransportState = ServiceStateID::TURN_OFF;
        BTTransport otherTransport =
            (transport == BTTransport::ADAPTER_BREDR) ? BTTransport::ADAPTER_BLE : BTTransport::ADAPTER_BREDR;
        std::string name = it.first;

        if (pimpl->profilesState_.Find(otherTransport, name, otherTransportState)) {
            switch (otherTransportState) {
                case ServiceStateID::TURN_ON:
                case ServiceStateID::TURNING_ON:
                    HILOGI("%{public}d otherTransport %{public}d %{public}s",
                        otherTransportState,
                        otherTransport,
                        name.c_str());
                    pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURN_OFF);
                    break;
                default:
                    break;
            }
        }

        if (pimpl->profilesState_.Get(transport, name) == ServiceStateID::TURN_ON) {
            pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURNING_OFF);
            HILOGI("transport %{public}d %{public}s disable", transport, name.c_str());
            IProfile *profile = nullptr;
            if (pimpl->startedProfiles_.Find(transport, name, profile)) {
                profile->GetContext()->Disable();
            }
        }
    }
}

void ProfileServiceManager::OnDisable(const std::string &name, bool ret) const
{
    HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
    DoInAdapterManagerThread([this, name, ret] {this->DisableCompleteProcess(name, ret);});
}

void ProfileServiceManager::DisableCompleteProcess(const std::string &name, bool ret) const
{
    std::string profileName = name;

    ServiceStateID state = ServiceStateID::TURN_OFF;
    if ((pimpl->profilesState_.Find(BTTransport::ADAPTER_BREDR, profileName, state)) &&
        (state == ServiceStateID::TURNING_OFF)) {
        HILOGI("BREDR %{public}s complete ret %{public}d", profileName.c_str(), ret);
        pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BREDR, profileName, ServiceStateID::TURN_OFF);
        if (!IsProfilesTurning(BTTransport::ADAPTER_BREDR)) {
            DisableCompleteNotify(BTTransport::ADAPTER_BREDR);
        }
        CheckWaitEnableProfiles(profileName, BTTransport::ADAPTER_BLE);
    }
    if ((pimpl->profilesState_.Find(BTTransport::ADAPTER_BLE, profileName, state)) &&
        (state == ServiceStateID::TURNING_OFF)) {
        HILOGI("BLE %{public}s complete ret %{public}d", profileName.c_str(), ret);
        pimpl->profilesState_.SetProfile(BTTransport::ADAPTER_BLE, profileName, ServiceStateID::TURN_OFF);
        if (!IsProfilesTurning(BTTransport::ADAPTER_BLE)) {
            DisableCompleteNotify(BTTransport::ADAPTER_BLE);
        }
        CheckWaitEnableProfiles(profileName, BTTransport::ADAPTER_BREDR);
    }
}

void ProfileServiceManager::DisableCompleteNotify(const BTTransport transport) const
{
    if (pimpl->profilesState_.GetProfiles(transport) == nullptr) {
        HILOGE("GetProfiles() return nullptr");
        return;
    }
    int turnOffProfileCount = std::count_if(pimpl->profilesState_.GetProfiles(transport)->begin(),
        pimpl->profilesState_.GetProfiles(transport)->end(),
        [](const auto &temp) -> bool { return temp.second == ServiceStateID::TURN_OFF; });
    if (turnOffProfileCount == pimpl->profilesState_.Size(transport)) {
        HILOGI("OK transport %{public}d turnOffProfileCount %{public}d",
            static_cast<int>(transport), turnOffProfileCount);
        AdapterManager::GetInstance()->OnProfileServicesDisableComplete(transport, true);
    } else {
        HILOGI("NG transport %{public}d turnOffProfileCount %{public}d",
            static_cast<int>(transport), turnOffProfileCount);
        AdapterManager::GetInstance()->OnProfileServicesDisableComplete(transport, false);
    }
}

void ProfileServiceManager::CheckWaitEnableProfiles(const std::string &name, const BTTransport transport) const
{
    ServiceStateID state = ServiceStateID::TURN_OFF;
    if ((pimpl->profilesState_.Find(transport, name, state)) && (state == ServiceStateID::WAIT_TURN_ON)) {
        HILOGI("%{public}s ::WAIT_TURN_ON", name.c_str());
        IProfile *profile = nullptr;
        if (pimpl->startedProfiles_.Find(transport, name, profile)) {
            pimpl->profilesState_.SetProfile(transport, name, ServiceStateID::TURNING_ON);
            profile->GetContext()->Enable();
        }
    }
}

void ProfileServiceManager::GetProfileServicesSupportedUuids(std::vector<std::string> &uuids) const
{
    for (auto &sp : GET_SUPPORT_PROFILES()) {
        ServiceStateID state = ServiceStateID::TURN_OFF;
        if (pimpl->profilesState_.Find(BTTransport::ADAPTER_BREDR, sp.name_, state) &&
            (state == ServiceStateID::TURN_ON) && (sp.uuid_ != "") &&
            (std::find(uuids.begin(), uuids.end(), sp.uuid_) == uuids.end())) {
            uuids.push_back(sp.uuid_);
        }
        if (pimpl->profilesState_.Find(BTTransport::ADAPTER_BLE, sp.name_, state) &&
            (state == ServiceStateID::TURN_ON) && (sp.uuid_ != "") &&
            (std::find(uuids.begin(), uuids.end(), sp.uuid_) == uuids.end())) {
            uuids.push_back(sp.uuid_);
        }
    }
}

std::vector<uint32_t> ProfileServiceManager::GetProfileServicesList() const
{
    std::vector<uint32_t> profileServicesList;
    for (auto &sp : GET_SUPPORT_PROFILES()) {
        if (pimpl->startedProfiles_.Contains(sp.name_)) {
            HILOGI("%{public}s", sp.name_.c_str());
            profileServicesList.push_back(sp.id_);
        }
    }
    return profileServicesList;
}

BTConnectState ProfileServiceManager::GetProfileServiceConnectState(const uint32_t profileID) const
{
    std::string profileName = SupportProfilesInfo::IdToName(profileID);
    IProfile *profile = nullptr;
    if (!pimpl->startedProfiles_.Find(profileName, profile)) {
        return BTConnectState::DISCONNECTED;
    }

    unsigned int profileStateMask = static_cast<unsigned int>(profile->GetConnectState());
    HILOGI("profileStateMask is %{public}d", profileStateMask);
    if (profileStateMask & PROFILE_STATE_CONNECTED) {
        return BTConnectState::CONNECTED;
    } else if (profileStateMask & PROFILE_STATE_CONNECTING) {
        return BTConnectState::CONNECTING;
    } else if (profileStateMask & PROFILE_STATE_DISCONNECTING) {
        return BTConnectState::DISCONNECTING;
    } else {
        return BTConnectState::DISCONNECTED;
    }
}

BTConnectState ProfileServiceManager::GetProfileServicesConnectState() const
{
    unsigned int stateMask = 0;

    for (auto &sp : GET_SUPPORT_PROFILES()) {
        IProfile *profile = nullptr;
        if (pimpl->startedProfiles_.Find(sp.name_, profile)) {
            if (sp.name_ != PROFILE_NAME_GATT_CLIENT && sp.name_ != PROFILE_NAME_GATT_SERVER &&
                sp.name_ != PROFILE_NAME_SPP) {
                stateMask |= (unsigned int)profile->GetConnectState();
            }
        }
    }

    HILOGI("stateMask: %{public}d", stateMask);
    if (stateMask & PROFILE_STATE_CONNECTED) {
        return BTConnectState::CONNECTED;
    } else if (stateMask & PROFILE_STATE_CONNECTING) {
        return BTConnectState::CONNECTING;
    } else if (stateMask & PROFILE_STATE_DISCONNECTING) {
        return BTConnectState::DISCONNECTING;
    } else {
        return BTConnectState::DISCONNECTED;
    }
}

void ProfileServiceManager::SetProfileService(const BTTransport transport, const std::string &name)
{
    IProfile *profile = ClassCreator<IProfile>::NewInstance(name);
    pimpl->startedProfiles_.SetProfile(transport, name, profile);
}
}  // namespace bluetooth
}  // namespace OHOS