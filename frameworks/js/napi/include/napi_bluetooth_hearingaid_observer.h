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

#ifndef NAPI_BLUETOOTH_HEARINGAID_OBSERVER_H
#define NAPI_BLUETOOTH_HEARINGAID_OBSERVER_H

#include <shared_mutex>
#include "bluetooth_hearingaid.h"
#include "napi_event_subscribe_module.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

const char * const STR_BT_HEARING_AID_CONNECTION_STATE_CHANGE = "connectionStateChange";

class NapiHearingAidObserver : public HearingAidObserver {
public:
    NapiHearingAidObserver();
    ~NapiHearingAidObserver() override = default;
    void OnConnectionStateChanged(const std::string& address, int32_t state, int32_t cause) override;
    NapiEventSubscribeModule eventSubscribe_;
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif