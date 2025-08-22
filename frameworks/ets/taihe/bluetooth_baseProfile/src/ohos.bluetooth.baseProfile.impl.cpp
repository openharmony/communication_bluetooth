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

#include "ohos.bluetooth.baseProfile.proj.hpp"
#include "ohos.bluetooth.baseProfile.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

using namespace taihe;
using namespace ohos::bluetooth::baseProfile;

namespace {
// To be implemented.

class BaseProfileImpl {
public:
    BaseProfileImpl()
    {
        // Don't forget to implement the constructor.
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {}

    void Off(::taihe::string_view type, ::taihe::optional_view<::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {}

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(string_view deviceId)
    {
        return ohos::bluetooth::constant::ProfileConnectionState(
            ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_DISCONNECTED);
    }

    array<string> GetConnectedDevices()
    {
        return {};
    }
};
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
// NOLINTEND
