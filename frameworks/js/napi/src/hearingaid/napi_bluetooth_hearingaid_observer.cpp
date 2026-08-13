/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_hearingaid_observer"
#endif

#include "napi_bluetooth_hearingaid_observer.h"
#include "napi_bluetooth_event.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

NapiHearingAidObserver::NapiHearingAidObserver()
    : eventSubscribe_(STR_BT_HEARING_AID_CONNECTION_STATE_CHANGE, BT_MODULE_NAME)
{}

void NapiHearingAidObserver::OnConnectionStateChanged(const std::string& address, int32_t state, int32_t cause)
{
    HILOGI("OnConnectionStateChanged, state:%{public}d", state);
    auto nativeObject = std::make_shared<NapiNativeStateChangeParam>(address, state, cause);
    eventSubscribe_.PublishEvent(STR_BT_HEARING_AID_CONNECTION_STATE_CHANGE, nativeObject);
}

}  // namespace Bluetooth
}  // namespace OHOS
