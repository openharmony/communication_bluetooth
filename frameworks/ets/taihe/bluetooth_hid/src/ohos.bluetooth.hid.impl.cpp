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

#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "taihe_bluetooth_hid_host_observer.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::hid;

class HidHostProfileImpl {
public:
    HidHostProfileImpl()
    {
        std::shared_ptr<TaiheBluetoothHidHostObserver> observer_ =
            std::make_shared<TaiheBluetoothHidHostObserver>();
        HidHost *profile = HidHost::GetProfile();
        profile->RegisterObserver(observer_);
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
    }
    
    void Off(::taihe::string_view type, ::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }
private:
    std::shared_ptr<TaiheBluetoothHidHostObserver> observer_ = nullptr;
};

HidHostProfile CreateHidHostProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<HidHostProfileImpl, ::ohos::bluetooth::hid::HidHostProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHidHostProfile(OHOS::Bluetooth::CreateHidHostProfile);
// NOLINTEND
