/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_profile_service.h"

#include "a2dp_snk_service.h"
#include "bluetooth_def.h"
#include "hfp_hf_service.h"
#include "raw_address.h"

#include <vector>

namespace OHOS {
namespace bluetooth {

void CarProfileService::Connect(ProfileType profile, const std::string &addr)
{
    RawAddress device(addr);
    if (profile == ProfileType::HFP_HF) {
        auto *svc = HfpHfService::GetService();
        if (svc != nullptr) {
            svc->Connect(device);
        }
    } else if (profile == ProfileType::A2DP_SINK) {
        auto *svc = A2dpSnkService::GetService();
        if (svc != nullptr) {
            svc->Connect(device);
        }
    }
}

void CarProfileService::Disconnect(ProfileType profile, const std::string &addr)
{
    RawAddress device(addr);
    if (profile == ProfileType::HFP_HF) {
        auto *svc = HfpHfService::GetService();
        if (svc != nullptr) {
            svc->Disconnect(device);
        }
    } else if (profile == ProfileType::A2DP_SINK) {
        auto *svc = A2dpSnkService::GetService();
        if (svc != nullptr) {
            svc->Disconnect(device);
        }
    }
}

void CarProfileService::NotifyA2dpConnected()
{
    auto *svc = A2dpSnkService::GetService();
    if (svc != nullptr) {
        svc->NotifyA2dpConnected(true);
    }
}

void CarProfileService::NotifyPlaybackDisallowed()
{
    auto *svc = A2dpSnkService::GetService();
    if (svc != nullptr) {
        svc->NotifyPlaybackDisallowed();
    }
}

bool CarProfileService::HasHfpConnectingOrConnected()
{
    auto *svc = HfpHfService::GetService();
    if (svc == nullptr) {
        return false;
    }
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTING),
                               static_cast<int>(BTConnectState::CONNECTED)};
    return !svc->GetDevicesByStates(states).empty();
}

int CarProfileService::GetHfpDeviceState(const std::string &addr)
{
    auto *svc = HfpHfService::GetService();
    if (svc == nullptr) {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    return svc->GetDeviceState(RawAddress(addr));
}

int CarProfileService::GetA2dpDeviceState(const std::string &addr)
{
    auto *svc = A2dpSnkService::GetService();
    if (svc == nullptr) {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    return svc->GetDeviceState(RawAddress(addr));
}

}  // namespace bluetooth
}  // namespace OHOS
