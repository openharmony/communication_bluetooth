/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ohos.bluetooth.hfp.proj.hpp"
#include "ohos.bluetooth.hfp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_bluetooth_hfp_ag_observer.h"

namespace OHOS {
namespace Bluetooth {
    
using namespace taihe;
using namespace ohos::bluetooth::hfp;

class HandsFreeAudioGatewayProfileImpl {
public:
    HandsFreeAudioGatewayProfileImpl()
    {
        std::shared_ptr<TaiheHandsFreeAudioGatewayObserver> observer_ =
            std::make_shared<TaiheHandsFreeAudioGatewayObserver>();
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }
    
    void Off(::taihe::string_view type, ::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(string_view deviceId)
    {
        return {ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_DISCONNECTED};
    }

    array<string> GetConnectedDevices()
    {
        return {};
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(string_view deviceId)
    {
        return {ohos::bluetooth::baseProfile::ConnectionStrategy::key_t::CONNECTION_STRATEGY_UNSUPPORTED};
    }

    void SetConnectionStrategySync(string_view deviceId, ohos::bluetooth::baseProfile::ConnectionStrategy strategy) {}
private:
    std::shared_ptr<TaiheHandsFreeAudioGatewayObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

HandsFreeAudioGatewayProfile CreateHfpAgProfile()
{
    return make_holder<HandsFreeAudioGatewayProfileImpl, ohos::bluetooth::hfp::HandsFreeAudioGatewayProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHfpAgProfile(OHOS::Bluetooth::CreateHfpAgProfile);
// NOLINTEND
