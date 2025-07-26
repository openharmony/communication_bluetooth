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

#include "ohos.bluetooth.a2dp.proj.hpp"
#include "ohos.bluetooth.a2dp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_a2dp_src.h"
#include "taihe_bluetooth_a2dp_src_observer.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::a2dp;

class A2dpSourceProfileImpl {
public:
    A2dpSourceProfileImpl()
    {
        observer_ = std::make_shared<TaiheA2dpSourceObserver>();
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            A2dpSource *profile = A2dpSource::GetProfile();
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
    std::shared_ptr<TaiheA2dpSourceObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

A2dpSourceProfile CreateA2dpSrcProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<A2dpSourceProfileImpl, ohos::bluetooth::a2dp::A2dpSourceProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateA2dpSrcProfile(OHOS::Bluetooth::CreateA2dpSrcProfile);
// NOLINTEND
