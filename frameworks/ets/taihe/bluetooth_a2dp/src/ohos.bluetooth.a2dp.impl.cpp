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

using namespace taihe;
using namespace ohos::bluetooth::a2dp;

namespace {
// To be implemented.

class A2dpSourceProfileImpl {
public:
    A2dpSourceProfileImpl()
    {
        // Don't forget to implement the constructor.
    }

    void OnConnectionStateChange(
        callback_view<void(::ohos::bluetooth::baseProfile::StateChangeParam const&)> callback)
    {
    }
};

A2dpSourceProfile CreateA2dpSrcProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<A2dpSourceProfileImpl, A2dpSourceProfile>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateA2dpSrcProfile(CreateA2dpSrcProfile);
// NOLINTEND
